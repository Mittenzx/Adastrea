"""Check Content/Data/Universe/Galaxy.json against the galaxy plan's rules.

Errors (exit code 1):
  - a resource or POI resource that isn't a crafting-tree item id
  - a system in a region that doesn't exist; a region type that isn't one of the six
  - a security, hazard or richness value that isn't one of the allowed words
  - a sector nobody can reach by gate or lane from the start system's first sector
    (only for systems that have a jump lane; uncharted systems are listed separately)
Warnings:
  - raw (tier 1) crafting items no sector yields yet
  - sectors with no resources or no POIs
  - a sector's level that doesn't exist on disk

Usage: python Tools/validate_galaxy.py [--quiet]
See docs/11-TECHNICAL_SPECS/GALAXY_PLAN.md.
"""
from __future__ import annotations

import json
import sys
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GALAXY = ROOT / "Content/Data/Universe/Galaxy.json"
CRAFTING = ROOT / "Content/Data/CraftingTree.json"

REGION_TYPES = {"Heartland", "March", "Frontier", "Ruin", "Wilds", "Haven"}
SECURITY = {"High", "Medium", "Low", "None"}
HAZARD = {"None", "Mild", "Severe", "Extreme"}
RICHNESS = {"Low", "Medium", "High"}
HAZARD_TYPES = {"RadiationStorm", "Nebula", "GravityTide", "Debris", "Minefield", "Heat", "GasStorm",
                "Distance", "WarMachines"}


def main() -> int:
    quiet = "--quiet" in sys.argv
    galaxy = json.loads(GALAXY.read_text(encoding="utf-8"))
    crafting = json.loads(CRAFTING.read_text(encoding="utf-8"))
    items = set(crafting["Items"])
    raw = {r["OutputItem"] for r in crafting["Recipes"] if r["Tier"] == 1}

    errors: list[str] = []
    warnings: list[str] = []

    regions = {r["id"]: r for r in galaxy.get("regions", [])}
    for r in regions.values():
        if r.get("type") not in REGION_TYPES:
            errors.append(f"region {r['id']}: type '{r.get('type')}' is not one of {sorted(REGION_TYPES)}")

    sectors: dict[str, dict] = {}
    system_of: dict[str, str] = {}
    lanes: dict[str, set[str]] = {}
    yielded: set[str] = set()
    for sys_ in galaxy["systems"]:
        sid = sys_["id"]
        lanes.setdefault(sid, set()).update(sys_.get("jumpLinks", []))
        for link in sys_.get("jumpLinks", []):
            lanes.setdefault(link, set()).add(sid)
        if sys_.get("region") and sys_["region"] not in regions:
            errors.append(f"system {sid}: unknown region '{sys_['region']}'")
        if not sys_.get("region"):
            warnings.append(f"system {sid}: no region")
        for sec in sys_.get("sectors", []):
            sectors[sec["id"]] = sec
            system_of[sec["id"]] = sid
            where = f"sector {sec['id']}"
            if sec.get("security", "None") not in SECURITY:
                errors.append(f"{where}: security '{sec.get('security')}'")
            if sec.get("hazard", "None") not in HAZARD:
                errors.append(f"{where}: hazard '{sec.get('hazard')}'")
            for h in sec.get("hazards", []):
                if h.get("severity") not in HAZARD:
                    errors.append(f"{where}: hazard {h.get('type')} severity '{h.get('severity')}'")
                if h.get("type") not in HAZARD_TYPES:
                    warnings.append(f"{where}: hazard type '{h.get('type')}' is new (known: {sorted(HAZARD_TYPES)})")
            for res in sec.get("resources", []):
                if res.get("item") not in items:
                    errors.append(f"{where}: resource '{res.get('item')}' is not a crafting item")
                if res.get("richness") not in RICHNESS:
                    errors.append(f"{where}: resource {res.get('item')} richness '{res.get('richness')}'")
                yielded.add(res.get("item"))
            for poi in sec.get("pois", []):
                for item in poi.get("resources", []):
                    if item not in items:
                        errors.append(f"{where}: POI {poi.get('type')} resource '{item}' is not a crafting item")
            if not sec.get("resources"):
                warnings.append(f"{where}: no resources")
            if not sec.get("pois"):
                warnings.append(f"{where}: no POIs")
            level = sec.get("level", "")
            if level and level.startswith("/Game/"):
                path = ROOT / "Content" / (level[len("/Game/"):].split(".")[0] + ".umap")
                if not path.exists():
                    warnings.append(f"{where}: level {level} not found on disk")

    # Reachability over gates (same system) and lane gates (between systems).
    adj: dict[str, set[str]] = {s: set() for s in sectors}
    for sid, sec in sectors.items():
        for g in sec.get("gates", []) + sec.get("laneGates", []):
            if g in sectors:
                adj[sid].add(g)
                adj[g].add(sid)
    start_system = galaxy.get("startSystem")
    start = next((s for s in sectors if system_of[s] == start_system), None)
    seen: set[str] = set()
    if start:
        queue = deque([start])
        seen.add(start)
        while queue:
            for nxt in adj[queue.popleft()]:
                if nxt not in seen:
                    seen.add(nxt)
                    queue.append(nxt)
    uncharted = []
    for sid in sectors:
        if sid in seen:
            continue
        if lanes.get(system_of[sid]):
            errors.append(f"sector {sid}: can't be reached by gate from {start}")
        else:
            uncharted.append(sid)

    missing = sorted(raw - yielded)

    if not quiet:
        print(f"Galaxy: {len(regions)} regions, {len(galaxy['systems'])} systems, {len(sectors)} sectors, "
              f"{sum(1 for s in sectors.values() if s.get('level'))} with a level")
        if uncharted:
            print(f"Uncharted (no lane yet, not checked for gates): {', '.join(uncharted)}")
        if missing:
            print(f"Raw items no sector yields yet ({len(missing)}): {', '.join(missing)}")
        for w in warnings:
            print("WARN ", w)
    for e in errors:
        print("ERROR", e)
    print("OK" if not errors else f"{len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
