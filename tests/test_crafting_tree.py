#!/usr/bin/env python3
"""
Crafting & Building Tree Tests

Validates Content/Data/CraftingTree.json for structural and logical soundness:
unique IDs, ItemID regex, canonical production tags, acyclicity, tier
monotonicity, and full ingredient producibility.

Usage:
    pytest tests/test_crafting_tree.py -v
    python tests/test_crafting_tree.py      # Standalone mode
"""

import json
import re
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
CRAFTING_PATH = PROJECT_ROOT / "Content" / "Data" / "CraftingTree.json"
ITEM_ID_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
PRODUCED_IN_TAGS = {
    "ShipMining", "GasHarvesting", "IceCollection", "OrganicFarming", "Salvage",
    "Processing", "Fabrication", "Reactor", "SolarArray", "ScienceLab", "FuelDepot",
    "PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab", "BiologyLab",
}


def load_tree():
    """Load and parse the crafting tree JSON."""
    with open(CRAFTING_PATH, "r") as f:
        return json.load(f)


class TestCraftingTree:
    """Structural and logical validation of the crafting tree data."""

    def test_json_parses(self):
        """CraftingTree.json is valid JSON with a Recipe list."""
        data = load_tree()
        assert "Recipes" in data and isinstance(data["Recipes"], list)
        assert str(data["SchemaVersion"]).startswith("1.")

    def test_recipe_ids_unique(self):
        """Every recipe has a unique RecipeID and OutputItem."""
        data = load_tree()
        recipes = data["Recipes"]
        recipe_ids = [r["RecipeID"] for r in recipes]
        outputs = [r["OutputItem"] for r in recipes]
        assert len(recipe_ids) == len(set(recipe_ids))
        assert len(outputs) == len(set(outputs))

    def test_itemids_match_convention(self):
        """All RecipeID, OutputItem, and ingredient ItemIDs match the regex."""
        data = load_tree()
        recipes = data["Recipes"]
        assert recipes, "Tree is empty"
        for r in recipes:
            assert ITEM_ID_RE.match(r["RecipeID"]), r["RecipeID"]
            assert ITEM_ID_RE.match(r["OutputItem"]), r["OutputItem"]
            for ing in r["Ingredients"]:
                assert ITEM_ID_RE.match(ing["ItemID"]), ing["ItemID"]

    def test_produced_in_tags_canonical(self):
        """Only known production module tags are used."""
        data = load_tree()
        for r in data["Recipes"]:
            assert r["ProducedIn"] in PRODUCED_IN_TAGS, \
                f"{r['RecipeID']}: unknown tag {r['ProducedIn']}"

    def test_acquisition_has_no_ingredients(self):
        """Raw extraction recipes have no ingredients; crafted ones do."""
        data = load_tree()
        for r in data["Recipes"]:
            if r.get("Acquisition"):
                assert not r["Ingredients"], f"{r['RecipeID']}: extraction w/ ingredients"
            else:
                assert r["Ingredients"], f"{r['RecipeID']}: crafted w/o ingredients"

    def test_all_ingredients_producible(self):
        """Every ingredient is produced by some recipe (acyclic source check)."""
        data = load_tree()
        produced = {r["OutputItem"] for r in data["Recipes"]}
        for r in data["Recipes"]:
            for ing in r["Ingredients"]:
                assert ing["ItemID"] in produced, \
                    f"{r['RecipeID']}: {ing['ItemID']} has no producer"

    def test_tier_monotonic(self):
        """No ingredient at a strictly higher tier than its output."""
        data = load_tree()
        tiers = {r["OutputItem"]: r["Tier"] for r in data["Recipes"]}
        for r in data["Recipes"]:
            for ing in r["Ingredients"]:
                if ing["ItemID"] in tiers:
                    assert tiers[ing["ItemID"]] <= r["Tier"], \
                        f"{r['RecipeID']}: {ing['ItemID']} tier {tiers[ing['ItemID']]} > {r['Tier']}"

    def test_no_circular_recipes(self):
        """The recipe graph is acyclic."""
        data = load_tree()
        graph = {r["OutputItem"]: {i["ItemID"] for i in r["Ingredients"]} for r in data["Recipes"]}
        WHITE, GRAY, BLACK = 0, 1, 2
        color = {}
        stack = []
        cycles = []

        def visit(node):
            color[node] = GRAY
            stack.append(node)
            for dep in graph.get(node, ()):
                if dep not in graph:
                    continue  # raw material leaf
                if color.get(dep, WHITE) == GRAY:
                    start = stack.index(dep)
                    cycles.append(stack[start:] + [dep])
                elif color.get(dep, WHITE) == WHITE:
                    visit(dep)
            color[node] = BLACK
            stack.pop()

        for node in graph:
            if color.get(node, WHITE) == WHITE:
                visit(node)
        assert not cycles, f"Circular recipes: {cycles}"

    def test_research_required_is_valid(self):
        """ResearchRequired (if set) names an existing item at an unlocked research level."""
        data = load_tree()
        recipes = data["Recipes"]
        items = {r["OutputItem"] for r in recipes}
        levels = {r["OutputItem"]: r["ResearchLevel"] for r in recipes}
        for r in recipes:
            req = r.get("ResearchRequired")
            if not req:
                continue
            assert req in items, f"{r['OutputItem']}: research '{req}' not produced"
            assert levels[req] <= r["ResearchLevel"], \
                f"{r['OutputItem']}: research '{req}' at lvl {levels[req]} > own {r['ResearchLevel']}"

    def test_all_have_research_level(self):
        """Every recipe exposes a numeric ResearchLevel (1..4)."""
        data = load_tree()
        for r in data["Recipes"]:
            assert isinstance(r.get("ResearchLevel"), int)
            assert 1 <= r["ResearchLevel"] <= 4, r["OutputItem"]

    def test_items_cover_all_recipe_outputs(self):
        """Every recipe OutputItem has an entry in the Items stats map."""
        data = load_tree()
        recipes = data["Recipes"]
        items = data["Items"]
        assert isinstance(items, dict)
        for r in recipes:
            assert r["OutputItem"] in items, f"{r['OutputItem']} missing ItemStats"

    def test_item_stats_valid(self):
        """ItemStats fields are well-formed: name/desc present, weight & volume > 0,
        valid storage/rarity enums."""
        data = load_tree()
        items = data["Items"]
        storage = {"Solid", "Liquid", "Gas", "Refrigerated", "Hazardous", "Other"}
        rarity = {"Common", "Uncommon", "Rare", "VeryRare", "Legendary"}
        for iid, st in items.items():
            assert st["ItemName"], f"{iid}: empty name"
            assert st["Description"], f"{iid}: empty description"
            assert st["WeightKg"] > 0, f"{iid}: bad weight"
            assert st["VolumeM3"] > 0, f"{iid}: bad volume"
            assert st["StorageType"] in storage, f"{iid}: bad storage {st['StorageType']}"
            assert st["Rarity"] in rarity, f"{iid}: bad rarity {st['Rarity']}"
            assert st["BaseValue"] >= 0, f"{iid}: negative value"
            assert st["MaterialCategory"], f"{iid}: missing material category"

    def test_research_tree_valid(self):
        """ResearchTree.json is present, parses, every lab research has its nodes,
        and every Unlock is a real recipe OutputItem in the crafting tree."""
        research_path = Path(__file__).parent.parent / "Content" / "Data" / "ResearchTree.json"
        assert research_path.is_file(), "ResearchTree.json missing"
        with open(research_path) as f:
            rtree = json.load(f)
        assert rtree.get("Branches"), "no branches"
        assert rtree.get("ResearchNodes"), "no research nodes"
        crafting = load_tree()
        outputs = {r["OutputItem"] for r in crafting["Recipes"]}
        research_nodes = {n["ResearchID"] for n in rtree["ResearchNodes"]}
        # every research node's unlocks are real craftable items
        for n in rtree["ResearchNodes"]:
            for unlock in n["Unlocks"]:
                assert unlock in outputs, \
                    f"{n['ResearchID']} unlocks {unlock} which is not a crafted item"
        # every lab in Branches has matching nodes
        for b in rtree["Branches"]:
            lab_nodes = [n for n in rtree["ResearchNodes"] if n["ProducedIn"] == b["Lab"]]
            assert lab_nodes, f"branch {b['Domain']} has no nodes"
            assert b["ResearchLevel2"] and b["ResearchLevel3"], \
                f"branch {b['Domain']} missing rl2/rl3"


if __name__ == "__main__":
    try:
        import pytest
        print("Running with pytest...")
        import sys
        sys.exit(pytest.main([__file__, "-v"]))
    except ImportError:
        failures = 0
        print("Standalone mode (pytest not available)")
        for name in sorted(dir(TestCraftingTree)):
            if not name.startswith("test_"):
                continue
            method = getattr(TestCraftingTree(), name)
            try:
                method()
                print(f"  ✓ {name}")
            except AssertionError as e:
                print(f"  ✗ {name}: {e}")
                failures += 1
        print(f"\n{'✅ ALL PASSED' if failures == 0 else f'{failures} FAILED'}")
        import sys
        sys.exit(1 if failures else 0)