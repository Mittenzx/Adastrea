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
import os
import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
CRAFTING_PATH = PROJECT_ROOT / "Content" / "Data" / "CraftingTree.json"
ITEM_ID_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
PRODUCED_IN_TAGS = {
    "ShipMining", "GasHarvesting", "IceCollection", "OrganicFarming", "Salvage",
    "Processing", "Fabrication", "Reactor", "SolarArray", "ScienceLab", "FuelDepot",
    "PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab", "BiologyLab",
    "ProjectileWeaponsLab", "BeamWeaponsLab",
    "IonPropulsionLab", "GravMaterialsLab", "EncryptionLab", "OptronicsLab", "CyberneticsLab",
    "Contract:Researchers",
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
        main_labs = {"PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab", "BiologyLab"}
        for b in rtree["Branches"]:
            lab_nodes = [n for n in rtree["ResearchNodes"] if n["ProducedIn"] == b["Lab"]]
            assert lab_nodes, f"branch {b['Domain']} has no nodes"
            # Main domain labs have both an rl2 and an rl3 milestone. Niche /
            # specialized / contracted branches legitimately have a single
            # breakthrough (usually rl3) gating their niche or contract tech.
            if b["Lab"] in main_labs:
                assert b["ResearchLevel2"] and b["ResearchLevel3"], \
                    f"branch {b['Domain']} missing rl2/rl3"
            else:
                assert b["ResearchLevel3"], f"niche/contract branch {b['Domain']} missing rl3"

    def test_economy_is_coherent(self):
        """Every crafted recipe's output value is >= ingredient cost (no loss),
        every Economy entry present, and values positive."""
        data = load_tree()
        economy = data.get("Economy")
        assert economy, "no Economy map"
        for item, e in economy.items():
            assert e["OutputValue"] >= e["IngredientCost"], \
                f"{item}: output {e['OutputValue']} < cost {e['IngredientCost']} (loss)"
            assert e["IngredientCost"] > 0, f"{item}: zero ingredient cost"

    def test_contract_research_model(self):
        """Expertise-level-3 (contract) research has a valid provider and is produced
        in the Contract:Researchers channel; expertise 1/2 items have no provider."""
        data = load_tree()
        providers = {"ScholarsSyndicate", "EngineersCollective", "FrontierAlliance",
                     "HonorCircle", "MerchantCoalition", "TradersGuild"}
        contract = [r for r in data["Recipes"] if r.get("ExpertiseLevel") == 3]
        assert contract, "no contract research"
        for r in contract:
            assert r["ProducedIn"] == "Contract:Researchers", \
                f"{r['OutputItem']}: contract research must use Contract:Researchers"
            assert r["ResearchProvider"] in providers, \
                f"{r['OutputItem']}: bad provider {r.get('ResearchProvider')}"
        # only contract research may carry a provider
        for r in data["Recipes"]:
            if r["ExpertiseLevel"] != 3:
                assert not r.get("ResearchProvider"), \
                    f"{r['OutputItem']}: provider without expertise 3"

    def test_station_builder_layout_valid(self):
        """The example station layout validates under the station-builder rules,
        and known-bad layouts (disconnected, no power, no dock, no core) fail."""
        base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        sys.path.insert(0, os.path.join(base, "docs", "11-TECHNICAL_SPECS"))
        try:
            import generate_station_builder as gsb
        finally:
            sys.path.pop(0)
        # valid example
        layout = gsb.example_layout()
        ok, errs, warns = gsb.check_station_layout(layout, gsb.build_meta())
        assert ok, f"example layout should be valid: {errs}"
        # disconnected: move marketplace far away
        bad = json.loads(json.dumps(layout))
        bad["Modules"][4]["GridPos"] = [8, 8, 8]
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert not ok
        assert any("disconnected" in e for e in errs)
        # no power: drop the reactor
        bad = json.loads(json.dumps(layout))
        bad["Modules"] = [m for m in bad["Modules"] if m["ItemID"] != "ReactorModule"]
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert not ok
        assert any("power" in e for e in errs)
        # no dock
        bad = json.loads(json.dumps(layout))
        bad["Modules"] = [m for m in bad["Modules"] if m["ItemID"] != "DockingBayModule"]
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert not ok
        assert any("docking" in e for e in errs)
        # two cores
        bad = json.loads(json.dumps(layout))
        bad["Modules"][1]["IsCore"] = True
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert not ok
        assert any("core" in e for e in errs)
        # footprint overlap: stack cargo on the core
        bad = json.loads(json.dumps(layout))
        core_pos = next(m["GridPos"] for m in bad["Modules"] if m["IsCore"])
        bad["Modules"][1]["GridPos"] = list(core_pos)  # same corner as corridor core
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert not ok
        assert any("overlap" in e for e in errs)
        # cost summary present
        cost = gsb.build_cost_summary(layout, gsb.build_meta())
        assert cost["total_cost"] > 0
        assert sum(cost["group_counts"].values()) == len(layout["Modules"])
        # no industry present -> the production-chain warn should NOT fire on the base layout
        ok, errs, warns = gsb.check_station_layout(layout, gsb.build_meta())
        assert not any(("industry" in w or "storage" in w) for w in warns), f"unexpected warn: {warns}"
        # add a FabricationModule (industry) but drop storage -> warning fires
        bad = json.loads(json.dumps(layout))
        bad["Modules"] = [m for m in bad["Modules"] if m["ItemID"] != "CargoBayModule"]
        bad["Modules"].append({"ModuleID": "M9", "ItemID": "FabricationModule",
                               "GridPos": [9, 4, 0], "Rotation": 0, "IsCore": False})
        ok, errs, warns = gsb.check_station_layout(bad, gsb.build_meta())
        assert any("storage" in w for w in warns) or any("industry" in w for w in warns)


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