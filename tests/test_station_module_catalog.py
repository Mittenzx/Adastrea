#!/usr/bin/env python3
"""
Station Module Catalog Tests

Validates the Phase 3 station module catalog derivation: all 27 craftable modules
are present, their build cost derives from the authoritative crafting tree (cost-
driven OutputValue), materials match the recipe ingredients, and tech levels /
groups are sensible. Guards the catalog against drifting from the crafting tree.

Usage:
    pytest tests/test_station_module_catalog.py -v
    python tests/test_station_module_catalog.py      # Standalone mode
"""

import json
import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
# Allow importing the generator from Tools/ without a package
sys.path.insert(0, str(PROJECT_ROOT / "Tools"))

import generate_station_module_catalog as gen  # noqa: E402

CRAFTING_PATH = PROJECT_ROOT / "Content" / "Data" / "CraftingTree.json"
MIRROR_PATH = PROJECT_ROOT / "Content" / "DataAssets" / "Stations" / "DA_StationModuleCatalog.uasset.txt"
TMP_JSON = PROJECT_ROOT / "Content" / "Data" / "StationModuleCatalog.json"

# The 27 expected craftable station modules (15 core + 12 research labs).
EXPECTED_MODULES = {
    "CorridorModule", "CargoBayModule", "DockingBayModule", "DockingPortModule",
    "HabitationModule", "BarracksModule", "MarketplaceModule", "ScienceLabModule",
    "ReactorModule", "SolarArrayModule", "ProcessingModule", "FabricationModule",
    "FuelDepotModule", "TurretModule", "ShieldGeneratorModule",
    "PhysicsLabModule", "MaterialsLabModule", "ElectronicsLabModule", "WeaponsLabModule",
    "BiologyLabModule", "ProjectileWeaponsLab", "BeamWeaponsLab", "IonPropulsionLab",
    "GravMaterialsLab", "EncryptionLab", "OptronicsLab", "CyberneticsLab",
}

# Valid EStationModuleGroup values (must match StationModuleTypes.h).
VALID_GROUPS = {
    "All", "Docking", "Power", "Storage", "Processing", "Defence",
    "Habitation", "Public", "Connection", "Other",
}


def load_crafting_tree():
    with open(CRAFTING_PATH, "r", encoding="utf-8") as f:
        return json.load(f)


def load_catalog_spec():
    with open(TMP_JSON, "r", encoding="utf-8") as f:
        return json.load(f)["entries"]


class TestStationModuleCatalog:
    """Validation of the derived station module catalog."""

    # ---- presence & completeness ----

    def test_all_27_modules_present(self):
        entries = load_catalog_spec()
        present = {e["item_id"] for e in entries}
        assert EXPECTED_MODULES == present, (
            f"catalog mismatch: missing {EXPECTED_MODULES - present}, "
            f"extra {present - EXPECTED_MODULES}")

    def test_module_count_matches_mirror(self):
        entries = load_catalog_spec()
        text = MIRROR_PATH.read_text(encoding="utf-8")
        m = re.search(r"ModuleCount:\s*(\d+)", text)
        assert m and int(m.group(1)) == len(entries)

    def test_mirror_lists_every_module(self):
        text = MIRROR_PATH.read_text(encoding="utf-8")
        for e in load_catalog_spec():
            assert f"Module: {e['item_id']}" in text, f"{e['item_id']} missing from mirror"

    # ---- cost derivation ----

    def test_build_cost_matches_recipe_output(self):
        tree = load_crafting_tree()
        economy = tree.get("Economy", {})
        for e in load_catalog_spec():
            eco = economy.get(e["item_id"])
            assert eco, f"no economy entry for {e['item_id']}"
            expected = int(round(eco.get("OutputValue", 0)))
            assert e["build_cost_credits"] == expected, (
                f"{e['item_id']}: build cost {e['build_cost_credits']} != recipe "
                f"OutputValue {expected}")

    def test_materials_match_recipe_ingredients(self):
        tree = load_crafting_tree()
        recipes = {r["OutputItem"]: r for r in tree["Recipes"]}
        for e in load_catalog_spec():
            recipe = recipes[e["item_id"]]
            expected = {i["ItemID"]: int(i["Qty"]) for i in recipe["Ingredients"]}
            assert e["materials"] == expected, (
                f"{e['item_id']}: materials {e['materials']} != ingredients {expected}")

    def test_all_costs_positive(self):
        for e in load_catalog_spec():
            assert e["build_cost_credits"] > 0, f"{e['item_id']} has non-positive cost"
            assert e["build_time_seconds"] > 0, f"{e['item_id']} has non-positive build time"

    # ---- tech level & group sanity ----

    def test_tech_levels_in_range(self):
        for e in load_catalog_spec():
            assert 1 <= e["required_tech_level"] <= 10, (
                f"{e['item_id']} tech level {e['required_tech_level']} out of range")

    def test_core_modules_lower_or_equal_tech_than_labs(self):
        entries = load_catalog_spec()
        # A research lab is any module ending in "Lab" or "LabModule"; the core
        # modules are the plain buildable station modules (Docking, Power, ...).
        lab = lambda e: e["item_id"].endswith("Lab") or e["item_id"].endswith("LabModule")
        core = [e for e in entries if not lab(e)]
        labs = [e for e in entries if lab(e)]
        assert core and labs
        # Every research lab must sit at an equal-or-higher tech level than the
        # most advanced core module (so labs are never the easy early unlocks).
        max_core = max(e["required_tech_level"] for e in core)
        assert all(l["required_tech_level"] >= max_core for l in labs)

    def test_groups_are_valid(self):
        for e in load_catalog_spec():
            assert e["group"] in VALID_GROUPS, f"{e['item_id']} group '{e['group']}' invalid"

    def test_class_paths_wellformed(self):
        for e in load_catalog_spec():
            assert e["class_path"].startswith("/Script/Adastrea."), (
                f"{e['item_id']} bad class path {e['class_path']}")
            assert e["cpp_class"].startswith("A"), f"{e['item_id']} bad cpp class name"


if __name__ == "__main__":
    import os
    failed = 0
    for name in dir(TestStationModuleCatalog):
        if name.startswith("test_") and callable(getattr(TestStationModuleCatalog, name)):
            try:
                getattr(TestStationModuleCatalog(), name)()
                print(f"PASS {name}")
            except AssertionError as ex:
                failed += 1
                print(f"FAIL {name}: {ex}")
    sys.exit(1 if failed else 0)