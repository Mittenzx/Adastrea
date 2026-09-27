"""Level station layouts: resolve, validate and encode as Station Editor blueprints.

Content/Data/LevelStationLayouts.json describes each level station as a core plus
modules attached face-to-face, the way a player builds in the Station Editor. This
module turns that into grid positions using the editor's own placement rules
(Source/StationEditor/Private/StationEditorManager.cpp):

- Each module is a box: its grid footprint in cells, X and Y swapped by a
  quarter-turn yaw (GetModuleHalfExtents); a core is its archetype's square
  footprint (GetPlacedModuleHalfExtents).
- Attaching B to A's face puts B at A + dir * ceil((halfA + halfB) / cell) * cell,
  half-sizes taken along dir (FindAttachPosition).
- No two boxes may overlap (CheckCollision), and every module must share a face
  with an earlier one, touching or up to one cell apart, with a connection face
  pointing each way (IsAdjacentToExistingModule).

Layouts are horizontal: modules attach N/S/E/W only.

It also checks the station's power balance and that each docking bay keeps at
least two usable berths (ADockingBayModule's BerthClearance rule).

Run:  python Tools/station_layouts.py           # validate and print blueprints
"""
from __future__ import annotations

import json
import math
import os
import re
import sys
from dataclasses import dataclass

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LAYOUTS_PATH = os.path.join(ROOT, "Content", "Data", "LevelStationLayouts.json")
BUILDER_DATA_PATH = os.path.join(ROOT, "Content", "Data", "StationModuleBuilderData.json")
STATIONS_SRC = os.path.join(ROOT, "Source", "Adastrea", "Private", "Stations")

# StationEditorManager.cpp: PlacementDistanceTolerance
TOLERANCE = 1.0

DIRECTIONS = {
    "N": (1, 0, 0),
    "S": (-1, 0, 0),
    "E": (0, 1, 0),
    "W": (0, -1, 0),
    "Up": (0, 0, 1),
    "Down": (0, 0, -1),
}

# DockingBayModule.cpp: default berths (local space) and the clearance each needs.
DOCK_BERTHS = [(0.0, -1400.0), (0.0, 1400.0), (1600.0, 0.0), (-1600.0, 0.0)]
DOCK_BERTH_CLEARANCE = 750.0
MIN_USABLE_BERTHS = 2


@dataclass
class Placed:
    id: str
    item: str
    pos: tuple
    rot: int
    half: tuple  # footprint half-size (cm) after rotation
    faces: list  # empty = all faces
    is_core: bool = False


def load_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def module_power(item: str, builder_data: dict) -> float:
    """Runtime ModulePower from the module's C++ constructor (what the editor sums)."""
    cpp = os.path.join(STATIONS_SRC, f"{item}.cpp")
    if os.path.exists(cpp):
        with open(cpp, encoding="utf-8") as f:
            m = re.search(r"ModulePower\s*=\s*(-?[\d.]+)f?\s*;", f.read())
        if m:
            return float(m.group(1))
    return float(builder_data.get(item, {}).get("power", 0))


def rotate(vec, yaw_degrees):
    rad = math.radians(yaw_degrees)
    c, s = round(math.cos(rad), 6), round(math.sin(rad), 6)
    x, y = vec[0], vec[1]
    return (x * c - y * s, x * s + y * c) + tuple(vec[2:])


def faces_toward(p: Placed, direction) -> bool:
    """DoesModuleFaceDirection: some face within 60 degrees of direction."""
    if not p.faces:
        return True
    for face in p.faces:
        world = rotate(DIRECTIONS[face], p.rot)
        if sum(a * b for a, b in zip(world, direction)) > 0.5:
            return True
    return False


def dist(a, b):
    return math.dist(a, b)


def half_extents(size_cells, yaw, cell):
    half = [c * cell * 0.5 for c in size_cells]
    if abs(math.sin(math.radians(yaw))) > 0.5:
        half[0], half[1] = half[1], half[0]
    return tuple(half)


def overlaps(a: Placed, b: Placed) -> bool:
    return all(abs(pb - pa) < ha + hb - TOLERANCE for pa, pb, ha, hb in zip(a.pos, b.pos, a.half, b.half))


def shared_face(a: Placed, b: Placed, max_gap: float):
    """Cardinal direction from a to b if their boxes share a face, else None."""
    separated = None
    for axis in range(3):
        gap = abs(b.pos[axis] - a.pos[axis]) - (a.half[axis] + b.half[axis])
        if gap < -TOLERANCE:
            continue
        if separated is not None or gap > max_gap + TOLERANCE:
            return None
        separated = axis
    if separated is None:
        return None
    direction = [0, 0, 0]
    direction[separated] = 1 if b.pos[separated] > a.pos[separated] else -1
    return tuple(direction)


def resolve_station(name: str, spec: dict, layouts: dict, builder_data: dict) -> list:
    cell = float(layouts["GridSpacing"])
    core_cells = layouts["Cores"][spec["core"]]
    core_half = half_extents((core_cells, core_cells, core_cells), 0, cell)
    placed = {"core": Placed("core", spec["core"], (0.0, 0.0, 0.0), 0, core_half, [], True)}
    order = [placed["core"]]
    for entry in spec["modules"]:
        item = entry["item"]
        data = builder_data.get(item)
        if data is None:
            raise ValueError(f"{name}: unknown module {item}")
        parent = placed.get(entry["on"])
        if parent is None:
            raise ValueError(f"{name}: {entry['id']} attaches to unknown/later module {entry['on']}")
        if entry["dir"] not in ("N", "S", "E", "W"):
            raise ValueError(f"{name}: {entry['id']} must attach N/S/E/W")
        rot = int(entry.get("rot", 0)) % 360
        half = half_extents(data["size"], rot, cell)
        faces = [] if data.get("faces", "all") == "all" else list(data["faces"])
        direction = DIRECTIONS[entry["dir"]]
        reach = sum((ph + h) * abs(d) for ph, h, d in zip(parent.half, half, direction))
        offset = math.ceil(reach / cell - 1e-6) * cell
        pos = tuple(round((pc + d * offset) / cell) * cell for pc, d in zip(parent.pos, direction))
        p = Placed(entry["id"], item, pos, rot, half, faces)
        if entry["id"] in placed:
            raise ValueError(f"{name}: duplicate module id {entry['id']}")
        placed[entry["id"]] = p
        order.append(p)
    return order


def validate_station(name: str, modules: list, builder_data: dict, cell: float) -> list:
    errors = []
    for i, m in enumerate(modules):
        for other in modules[:i]:
            if overlaps(m, other):
                errors.append(f"{name}: {m.id} overlaps {other.id}")
        if i == 0:
            continue
        connected = False
        for other in modules[:i]:
            direction = shared_face(m, other, cell)
            if direction is None:
                continue
            if faces_toward(m, direction) and faces_toward(other, tuple(-x for x in direction)):
                connected = True
                break
        if not connected:
            errors.append(f"{name}: {m.id} doesn't connect to an earlier module")

    balance = -sum(module_power(m.item, builder_data) for m in modules if not m.is_core)
    if balance < 0:
        errors.append(f"{name}: power deficit {balance:.0f}")

    docks = [m for m in modules if m.item == "DockingBayModule"]
    if not docks:
        errors.append(f"{name}: no docking bay")
    for dock in docks:
        usable = 0
        for berth in DOCK_BERTHS:
            b = rotate(berth, dock.rot)
            world = (dock.pos[0] + b[0], dock.pos[1] + b[1], dock.pos[2])
            if all(dist(world, o.pos) >= DOCK_BERTH_CLEARANCE for o in modules if o is not dock):
                usable += 1
        if usable < MIN_USABLE_BERTHS:
            errors.append(f"{name}: {dock.id} has only {usable} usable berths")
    if not any(m.item == "MarketplaceModule" for m in modules):
        errors.append(f"{name}: no marketplace")
    return errors


def to_blueprint(modules: list, cell: float) -> str:
    """ASpaceStation::ExportBlueprintString format; the core is listed first."""
    parts = [f"1.0.0;1000,1000,1000;{int(cell)}"]
    for i, m in enumerate(modules):
        g = ",".join(str(int(round(v / cell))) for v in m.pos)
        parts.append(f"M{i + 1}:{m.item}:{g}:{m.rot}:{1 if m.is_core else 0}")
    return ";".join(parts)


def load_all():
    """Returns ({station label: [Placed...]}, grid spacing, errors)."""
    layouts = load_json(LAYOUTS_PATH)
    builder_data = load_json(BUILDER_DATA_PATH)["Modules"]
    cell = float(layouts["GridSpacing"])
    stations, errors = {}, []
    for name, spec in layouts["Stations"].items():
        modules = resolve_station(name, spec, layouts, builder_data)
        errors += validate_station(name, modules, builder_data, cell)
        stations[name] = modules
    return stations, cell, errors


def main():
    stations, cell, errors = load_all()
    for name, modules in stations.items():
        print(f"{name}: {to_blueprint(modules, cell)}")
    for e in errors:
        print("ERROR:", e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
