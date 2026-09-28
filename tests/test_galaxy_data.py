"""Galaxy layout (Content/Data/Universe/Galaxy.json) read by UGalaxySubsystem for the system/universe maps."""
import json
import os

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GALAXY_PATH = os.path.join(ROOT, "Content", "Data", "Universe", "Galaxy.json")
CONTENT = os.path.join(ROOT, "Content")


@pytest.fixture(scope="module")
def galaxy():
    with open(GALAXY_PATH, encoding="utf-8") as f:
        return json.load(f)


def _sectors(galaxy):
    for system in galaxy["systems"]:
        for sector in system.get("sectors", []):
            yield system, sector


def test_has_systems_and_valid_start(galaxy):
    ids = [s["id"] for s in galaxy["systems"]]
    assert ids, "no systems"
    assert galaxy["startSystem"] in ids


def test_ids_are_unique_across_galaxy(galaxy):
    system_ids = [s["id"] for s in galaxy["systems"]]
    sector_ids = [sec["id"] for _, sec in _sectors(galaxy)]
    assert len(system_ids) == len(set(system_ids))
    assert len(sector_ids) == len(set(sector_ids))
    for i in system_ids + sector_ids:
        assert i and i == i.strip() and " " not in i, f"bad id {i!r}"


def test_systems_have_map_fields(galaxy):
    for s in galaxy["systems"]:
        assert s.get("name"), s["id"]
        assert len(s["position"]) == 2, s["id"]
        color = s["starColor"]
        assert len(color) == 3 and all(0.0 <= c <= 1.0 for c in color), s["id"]


def test_jump_links_resolve(galaxy):
    ids = {s["id"] for s in galaxy["systems"]}
    for s in galaxy["systems"]:
        for link in s.get("jumpLinks", []):
            assert link in ids, f"{s['id']} -> unknown system {link}"
            assert link != s["id"], f"{s['id']} links to itself"


def test_start_system_is_reachable_from_every_linked_system(galaxy):
    # Lanes are two-way at runtime; systems with no lanes at all are "uncharted" and allowed.
    adj = {s["id"]: set(s.get("jumpLinks", [])) for s in galaxy["systems"]}
    for a, links in list(adj.items()):
        for b in links:
            adj[b].add(a)
    seen, todo = set(), [galaxy["startSystem"]]
    while todo:
        cur = todo.pop()
        if cur not in seen:
            seen.add(cur)
            todo.extend(adj[cur])
    stranded = [sid for sid, links in adj.items() if links and sid not in seen]
    assert not stranded, f"lane network split off from the start system: {stranded}"


def test_gates_stay_inside_their_system(galaxy):
    for system, sector in _sectors(galaxy):
        local = {sec["id"] for sec in system.get("sectors", [])}
        for gate in sector.get("gates", []):
            assert gate in local, f"{sector['id']} gate -> {gate} is not in system {system['id']} (use jumpLinks)"
            assert gate != sector["id"]


def test_sector_map_placement(galaxy):
    for _, sector in _sectors(galaxy):
        assert 0.1 <= sector["orbitRadius"] <= 1.0, sector["id"]
        assert -360 <= sector["orbitAngle"] <= 360, sector["id"]


def test_sector_levels_exist(galaxy):
    for _, sector in _sectors(galaxy):
        level = sector.get("level", "")
        if not level:
            continue
        assert level.startswith("/Game/"), f"{sector['id']}: level must be a /Game/ package path"
        umap = os.path.join(CONTENT, level[len("/Game/"):].split(".")[0] + ".umap")
        assert os.path.isfile(umap), f"{sector['id']}: {level} has no .umap"


def test_each_level_is_used_by_one_sector(galaxy):
    levels = [sec["level"] for _, sec in _sectors(galaxy) if sec.get("level")]
    assert len(levels) == len(set(levels))
