#!/usr/bin/env python3
"""
Phase 6 — Crafting-in-engine data integrity tests.

Validates the crafting-tree recipe data that Fabrication/Processing/loader read,
so the new in-engine craft system can't be fed inconsistent data:
  - The 'Recipes' array is present and every OutputItem/ingredient is in 'Items'.
  - Every non-acquisition recipe has non-empty ingredients.
  - Recipe counts per ProducedIn facility are sane (Fabrication has many, etc.).
  - ProducedIn tags are from the known facility whitelist (match the loader).

Usage:
    pytest tests/test_crafting_recipes.py -v
    python tests/test_crafting_recipes.py      # Standalone mode
"""

import json
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
CRAFTING_PATH = PROJECT_ROOT / "Content" / "Data" / "CraftingTree.json"

# Facilities the in-engine loader/CraftingTreeLoader can dispatch to.
KNOWN_FACILITIES = {
    "ShipMining", "GasHarvesting", "IceCollection", "OrganicFarming", "Salvage",
    "Processing", "Fabrication", "Reactor", "SolarArray", "ScienceLab", "FuelDepot",
    "PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab", "BiologyLab",
    "ProjectileWeaponsLab", "BeamWeaponsLab", "IonPropulsionLab", "GravMaterialsLab",
    "EncryptionLab", "OptronicsLab", "CyberneticsLab", "Contract:Researchers",
}


def load_tree():
    with open(CRAFTING_PATH, "r", encoding="utf-8") as f:
        return json.load(f)


class TestCraftingRecipes:
    """Data integrity for the crafting-tree recipes consumed by the craft system."""

    def test_recipes_and_items_present(self):
        tree = load_tree()
        assert "Recipes" in tree and len(tree["Recipes"]) > 0
        assert "Items" in tree and len(tree["Items"]) > 0

    def test_every_recipe_has_fields(self):
        for r in load_tree()["Recipes"]:
            assert r["RecipeID"], "recipe missing RecipeID"
            assert r["OutputItem"], f"{r['RecipeID']} missing OutputItem"
            assert r["ProducedIn"], f"{r['RecipeID']} missing ProducedIn"
            assert isinstance(r.get("OutputQty", 0), int) and r.get("OutputQty", 0) >= 1, \
                f"{r['RecipeID']} bad OutputQty"
            assert isinstance(r.get("Ingredients", None), list), f"{r['RecipeID']} missing Ingredients"

    def test_all_outputs_and_ingredients_are_known_items(self):
        tree = load_tree()
        items = set(tree["Items"].keys())
        for r in tree["Recipes"]:
            assert r["OutputItem"] in items, f"{r['RecipeID']} OutputItem {r['OutputItem']} not in Items"
            for ing in r["Ingredients"]:
                assert ing["ItemID"] in items, \
                    f"{r['RecipeID']} ingredient {ing['ItemID']} not in Items"
                assert ing["Qty"] > 0, f"{r['RecipeID']} ingredient {ing['ItemID']} qty<=0"

    def test_non_acquisition_has_ingredients(self):
        for r in load_tree()["Recipes"]:
            if not r.get("Acquisition", False):
                assert r["Ingredients"], f"{r['RecipeID']} is non-acquisition but has no ingredients"

    def test_produced_in_tags_valid(self):
        for r in load_tree()["Recipes"]:
            assert r["ProducedIn"] in KNOWN_FACILITIES, \
                f"{r['RecipeID']} ProducedIn '{r['ProducedIn']}' not in facility whitelist"

    def test_fabrication_and_processing_have_recipes(self):
        tree = load_tree()
        fab = [r for r in tree["Recipes"] if r["ProducedIn"] == "Fabrication"]
        proc = [r for r in tree["Recipes"] if r["ProducedIn"] == "Processing"]
        assert len(fab) > 0, "no Fabrication recipes (loader GetRecipesForFacility would be empty)"
        assert len(proc) > 0, "no Processing recipes"
        assert len(fab) > len(proc), "Fabrication should have the most recipes"


if __name__ == "__main__":
    failed = 0
    for name in dir(TestCraftingRecipes):
        if name.startswith("test_") and callable(getattr(TestCraftingRecipes, name)):
            try:
                getattr(TestCraftingRecipes(), name)()
                print(f"PASS {name}")
            except AssertionError as ex:
                failed += 1
                print(f"FAIL {name}: {ex}")
    sys.exit(1 if failed else 0)