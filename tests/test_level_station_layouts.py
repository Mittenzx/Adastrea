"""Level station layouts (Content/Data/LevelStationLayouts.json) obey the Station Editor's rules."""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "Tools"))

import station_layouts  # noqa: E402

CORE_CPP = os.path.join(ROOT, "Source", "Adastrea", "Private", "Stations", "StationCoreModule.cpp")


def test_layouts_are_valid():
    _, _, errors = station_layouts.load_all()
    assert errors == []


def test_every_station_has_a_unique_core():
    layouts = station_layouts.load_json(station_layouts.LAYOUTS_PATH)
    cores = [spec["core"] for spec in layouts["Stations"].values()]
    assert len(cores) == len(set(cores))


def test_core_footprints_match_cpp():
    with open(CORE_CPP, encoding="utf-8") as f:
        src = f.read()
    cpp = {
        m.group(1): int(m.group(2))
        for m in re.finditer(r"A(StationCore_\w+)::A\1\(\)\s*\{\s*InitCore\([^,]+,\s*(\d+)\)", src)
    }
    layouts = station_layouts.load_json(station_layouts.LAYOUTS_PATH)
    assert cpp == layouts["Cores"]


def test_blueprint_lists_core_first():
    stations, cell, _ = station_layouts.load_all()
    for modules in stations.values():
        entries = station_layouts.to_blueprint(modules, cell).split(";")[3:]
        assert entries[0].split(":")[1].startswith("StationCore_")
        assert entries[0].endswith(":1")
        assert all(e.endswith(":0") for e in entries[1:])


def test_overlap_is_caught():
    layouts = station_layouts.load_json(station_layouts.LAYOUTS_PATH)
    builder = station_layouts.load_json(station_layouts.BUILDER_DATA_PATH)["Modules"]
    spec = {"core": "StationCore_Research", "modules": [
        {"id": "A", "item": "MarketplaceModule", "on": "core", "dir": "E"},
        {"id": "B", "item": "DockingBayModule", "on": "core", "dir": "N"},
        {"id": "C", "item": "CargoBayModule", "on": "A", "dir": "W"},
    ]}
    modules = station_layouts.resolve_station("T", spec, layouts, builder)
    errors = station_layouts.validate_station("T", modules, builder, 400.0)
    assert any("overlaps" in e for e in errors)
