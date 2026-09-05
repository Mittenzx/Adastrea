#!/usr/bin/env python3
"""
Adastrea Station Builder — layout validator + example generator.

Validates a station layout (JSON) against the rules in STATION_BUILDER.md:
  - every module is a craftable station module (from CraftingTree.json)
  - exactly one core
  - all modules connected to the core (via connection faces on the grid)
  - power balance >= 0 (needs at least one power module)
  - at least one docking module
  - all modules within plot bounds

Also emits a small builder metadata table (grid size / power / group) derived from
the crafting tree, and an example layout.

Run:
  python docs/11-TECHNICAL_SPECS/generate_station_builder.py
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TREE = os.path.join(ROOT, "Content", "Data", "CraftingTree.json")
OUT_META = os.path.join(ROOT, "Content", "Data", "StationModuleBuilderData.json")
OUT_EXAMPLE = os.path.join(ROOT, "Content", "Data", "ExampleStationLayout.json")

# Builder-only metadata: grid size (in grid cells), connection faces, power.
# Power mirrors the runtime module classes (negative = generates).
MODULE_META = {
    "CorridorModule":        {"size": (1, 1, 1), "power": -5,  "group": "Connection"},
    "CargoBayModule":        {"size": (2, 2, 1), "power": 20,  "group": "Storage"},
    "DockingBayModule":      {"size": (3, 2, 1), "power": 30,  "group": "Docking"},
    "DockingPortModule":     {"size": (1, 1, 1), "power": 10,  "group": "Docking"},
    "MarketplaceModule":     {"size": (2, 2, 1), "power": 40,  "group": "Public"},
    "HabitationModule":      {"size": (2, 2, 1), "power": 25,  "group": "Habitation"},
    "BarracksModule":        {"size": (2, 2, 1), "power": 30,  "group": "Habitation"},
    "ReactorModule":         {"size": (2, 2, 1), "power": -200, "group": "Power"},
    "SolarArrayModule":      {"size": (3, 1, 1), "power": -50, "group": "Power"},
    "ProcessingModule":      {"size": (2, 2, 1), "power": 40,  "group": "Processing"},
    "FabricationModule":     {"size": (2, 2, 1), "power": 50,  "group": "Processing"},
    "ScienceLabModule":      {"size": (2, 2, 1), "power": 40,  "group": "Processing"},
    "FuelDepotModule":       {"size": (2, 2, 1), "power": 30,  "group": "Storage"},
    "ShieldGeneratorModule": {"size": (2, 2, 1), "power": 60,  "group": "Defence"},
    "TurretModule":          {"size": (1, 1, 1), "power": 40,  "group": "Defence"},
    "PhysicsLabModule":      {"size": (2, 2, 1), "power": 40,  "group": "Research"},
    "MaterialsLabModule":    {"size": (2, 2, 1), "power": 40,  "group": "Research"},
    "ElectronicsLabModule":  {"size": (2, 2, 1), "power": 45,  "group": "Research"},
    "WeaponsLabModule":      {"size": (2, 2, 1), "power": 45,  "group": "Research"},
    "BiologyLabModule":      {"size": (2, 2, 1), "power": 40,  "group": "Research"},
}


def load_tree():
    with open(TREE) as f:
        return json.load(f)


def craftable_modules():
    tree = load_tree()
    return {r["OutputItem"] for r in tree["Recipes"] if r["OutputItem"].endswith("Module")}


def check_station_layout(layout, meta=None):
    """Validate a station layout dict. Returns (ok, errors[])."""
    meta = meta or MODULE_META
    errors = []
    modules = layout.get("Modules", [])
    if not modules:
        return False, ["no modules"]
    craftable = craftable_modules()

    # exactly one core
    cores = [m for m in modules if m.get("IsCore")]
    if len(cores) != 1:
        errors.append(f"must have exactly one core (found {len(cores)})")

    # every module is craftable + has metadata
    for m in modules:
        iid = m["ItemID"]
        if iid not in craftable:
            errors.append(f"{m['ModuleID']}: '{iid}' is not a craftable station module")
        if iid not in meta:
            errors.append(f"{m['ModuleID']}: '{iid}' missing builder metadata")

    # grid bounds
    plot = layout.get("PlotSize", [1000, 1000, 1000])
    spacing = layout.get("GridSpacing", 100)
    for m in modules:
        gx, gy, gz = m["GridPos"]
        if not (0 <= gx * spacing < plot[0] and 0 <= gy * spacing < plot[1] and 0 <= gz * spacing < plot[2]):
            errors.append(f"{m['ModuleID']}: out of plot bounds")

    # connectivity (BFS from core over adjacent grid cells)
    if cores:
        core = cores[0]
        pos_to_id = {tuple(m["GridPos"]): m["ModuleID"] for m in modules}
        # adjacency: two modules connect if their grid footprints touch on a face
        def footprint(m):
            size = meta.get(m["ItemID"], {}).get("size", (1, 1, 1))
            x, y, z = m["GridPos"]
            return [(x + dx, y + dy, z + dz) for dx in range(size[0]) for dy in range(size[1]) for dz in range(size[2])]
        cells = {}
        for m in modules:
            for c in footprint(m):
                cells.setdefault(c, []).append(m["ModuleID"])
        # BFS over modules that share a face (adjacent cells)
        from collections import deque
        start = core["ModuleID"]
        seen = {start}
        dq = deque([start])
        while dq:
            cur = dq.popleft()
            cur_m = next(m for m in modules if m["ModuleID"] == cur)
            cur_cells = set(footprint(cur_m))
            for other in modules:
                if other["ModuleID"] in seen:
                    continue
                o_cells = set(footprint(other))
                # share a face if any cell is adjacent (manhattan dist 1) to a cur cell
                if any((abs(a[0]-b[0]) + abs(a[1]-b[1]) + abs(a[2]-b[2])) == 1
                       for a in cur_cells for b in o_cells):
                    seen.add(other["ModuleID"])
                    dq.append(other["ModuleID"])
        for m in modules:
            if m["ModuleID"] not in seen:
                errors.append(f"{m['ModuleID']}: disconnected from core")

    # power balance (negative power = generates, positive = consumes)
    gen = sum(-m for m in (meta.get(x["ItemID"], {}).get("power", 0) for x in modules) if m < 0)
    cons = sum(m for m in (meta.get(x["ItemID"], {}).get("power", 0) for x in modules) if m > 0)
    if gen == 0:
        errors.append("no power-generating module (needs Reactor or SolarArray)")
    if cons > gen:
        errors.append(f"power deficit: consume {cons} > generate {gen}")

    # docking
    if not any(meta.get(m["ItemID"], {}).get("group") == "Docking" for m in modules):
        errors.append("no docking module (station unreachable by ships)")

    return (len(errors) == 0), errors


def build_meta():
    tree = load_tree()
    items = tree.get("Items", {})
    recipes = {r["OutputItem"]: r for r in tree["Recipes"]}
    economy = tree.get("Economy", {})
    meta = {}
    for iid, m in MODULE_META.items():
        st = items.get(iid, {})
        ec = economy.get(iid, {})
        meta[iid] = {
            "size": list(m["size"]),
            "power": m["power"],
            "group": m["group"],
            "cost": ec.get("OutputValue", st.get("BaseValue", 0)),
            "BaseValue": st.get("BaseValue", 0),
            "WeightKg": st.get("WeightKg", 0),
            "recipe": recipes.get(iid, {}).get("Ingredients", []),
        }
    return meta


def build_cost_summary(layout, meta):
    """Return the total build cost (sum of module OutputValue) and per-group counts."""
    meta = meta or MODULE_META
    modules = layout.get("Modules", [])
    total = 0
    groups = {}
    for m in modules:
        md = meta.get(m["ItemID"], {})
        total += md.get("cost", md.get("BaseValue", 0))
        g = md.get("group", "?")
        groups[g] = groups.get(g, 0) + 1
    return {"total_cost": total, "group_counts": groups}


def check_station_layout(layout, meta=None, strict_overlap=True):
    """Validate a station layout dict. Returns (ok, errors[]).\n
    strict_overlap: if True (default), overlapping/off-grid module footprints are
    hard errors. Set False to only warn (returns a third 'warnings' list)."""
    meta = meta or MODULE_META
    errors = []
    warnings = []
    modules = layout.get("Modules", [])
    if not modules:
        return False, ["no modules"]
    craftable = craftable_modules()

    # exactly one core
    cores = [m for m in modules if m.get("IsCore")]
    if len(cores) != 1:
        errors.append(f"must have exactly one core (found {len(cores)})")

    # every module is craftable + has metadata
    for m in modules:
        iid = m["ItemID"]
        if iid not in craftable:
            errors.append(f"{m['ModuleID']}: '{iid}' is not a craftable station module")
        if iid not in meta:
            errors.append(f"{m['ModuleID']}: '{iid}' missing builder metadata")

    # grid bounds + footprint overlap
    plot = layout.get("PlotSize", [1000, 1000, 1000])
    spacing = layout.get("GridSpacing", 100)

    def footprint(m):
        size = meta.get(m["ItemID"], {}).get("size", (1, 1, 1))
        x, y, z = m["GridPos"]
        return set((x + dx, y + dy, z + dz) for dx in range(size[0]) for dy in range(size[1]) for dz in range(size[2]))

    seen_cells = {}
    for m in modules:
        gx, gy, gz = m["GridPos"]
        if not (0 <= gx * spacing < plot[0] and 0 <= gy * spacing < plot[1] and 0 <= gz * spacing < plot[2]):
            errors.append(f"{m['ModuleID']}: out of plot bounds")
        fp = footprint(m)
        for c in fp:
            if c in seen_cells:
                msg = f"overlap: {m['ModuleID']} and {seen_cells[c]} share cell {c}"
                if strict_overlap:
                    errors.append(msg)
                else:
                    warnings.append(msg)
            else:
                seen_cells[c] = m["ModuleID"]

    # connectivity (BFS from core over adjacent grid cells)
    if cores:
        core = cores[0]
        from collections import deque
        start = core["ModuleID"]
        seen = {start}
        dq = deque([start])
        while dq:
            cur = dq.popleft()
            cur_m = next(m for m in modules if m["ModuleID"] == cur)
            cur_cells = footprint(cur_m)
            for other in modules:
                if other["ModuleID"] in seen:
                    continue
                o_cells = footprint(other)
                if any((abs(a[0]-b[0]) + abs(a[1]-b[1]) + abs(a[2]-b[2])) == 1
                       for a in cur_cells for b in o_cells):
                    seen.add(other["ModuleID"])
                    dq.append(other["ModuleID"])
        for m in modules:
            if m["ModuleID"] not in seen:
                errors.append(f"{m['ModuleID']}: disconnected from core")

    # power balance (negative power = generates, positive = consumes)
    gen = sum(-m for m in (meta.get(x["ItemID"], {}).get("power", 0) for x in modules) if m < 0)
    cons = sum(m for m in (meta.get(x["ItemID"], {}).get("power", 0) for x in modules) if m > 0)
    if gen == 0:
        errors.append("no power-generating module (needs Reactor or SolarArray)")
    if cons > gen:
        errors.append(f"power deficit: consume {cons} > generate {gen}")

    # docking
    if not any(meta.get(m["ItemID"], {}).get("group") == "Docking" for m in modules):
        errors.append("no docking module (station unreachable by ships)")

    # ---- production-chain warnings (industry / research modules need inputs) ----
    has_storage = any(meta.get(m["ItemID"], {}).get("group") in ("Storage",) for m in modules)
    has_industry = any(meta.get(m["ItemID"], {}).get("group") in
                       ("Processing", "Research", "Defence") for m in modules)
    if has_industry and not has_storage:
        warnings.append("industry/research present but no storage (CargoBay/FuelDepot) — inputs can't be held")
    if has_industry and gen == 0:
        warnings.append("industry/research present but no power source")

    return (len(errors) == 0), errors, warnings


def example_layout():
    # A small valid, NON-overlapping trade hub connected to the core:
    #   corridor(core)@[6,4] touches cargo[7,4] (E) and reactor[4,4] (W)
    #   cargo touches dock[6,6] (N); reactor touches market[4,6]; market touches dock.
    # Footprints verified disjoint; power: reactor -200 >= 95 consumed; has docking.
    return {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "Title": "Adastrea Station Layout",
        "SchemaVersion": "1.0.0",
        "StationName": "Example Trade Hub",
        "PlotSize": [1000, 1000, 1000],
        "GridSpacing": 100,
        "Modules": [
            {"ModuleID": "M1", "ItemID": "CorridorModule", "GridPos": [6, 4, 0], "Rotation": 0, "IsCore": True},
            {"ModuleID": "M2", "ItemID": "CargoBayModule", "GridPos": [7, 4, 0], "Rotation": 0, "IsCore": False},
            {"ModuleID": "M3", "ItemID": "DockingBayModule", "GridPos": [6, 6, 0], "Rotation": 0, "IsCore": False},
            {"ModuleID": "M4", "ItemID": "ReactorModule", "GridPos": [4, 4, 0], "Rotation": 0, "IsCore": False},
            {"ModuleID": "M5", "ItemID": "MarketplaceModule", "GridPos": [4, 6, 0], "Rotation": 0, "IsCore": False},
        ],
    }


def main():
    meta = build_meta()
    os.makedirs(os.path.dirname(OUT_META), exist_ok=True)
    with open(OUT_META, "w", encoding="utf-8") as f:
        json.dump({"SchemaVersion": "1.0.0", "Modules": meta}, f, indent=2)
        f.write("\n")

    layout = example_layout()
    with open(OUT_EXAMPLE, "w", encoding="utf-8") as f:
        json.dump(layout, f, indent=2)
        f.write("\n")

    ok, errors, warnings = check_station_layout(layout, meta)
    cost = build_cost_summary(layout, meta)
    print(f"Wrote {OUT_META}")
    print(f"Wrote {OUT_EXAMPLE}")
    print(f"Example layout valid: {ok}  (build cost {cost['total_cost']:,} cr, {len(layout['Modules'])} modules)")
    for e in errors:
        print("  [error] ", e)
    for w in warnings:
        print("  [warn]  ", w)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
