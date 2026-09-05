"""
Phase 6 runtime verification: exercises UCraftingTreeLoader recipe loading +
the craft pipeline (consume ingredients from cargo, produce output item).

IronOre x2 + CopperOre x1 -> SteelAlloy x1  (ProducedIn: Processing).

Checks:
  - LoadRecipes() loads > 0 recipes; GetRecipesForFacility("Processing") includes SteelAlloy.
  - CanCraftRecipe false with empty cargo, true with ingredients present.
  - CraftRecipe consumes 2 IronOre + 1 CopperOre, adds 1 SteelAlloy.
Writes JSON to Saved/_phase6_verify.json.
"""
import json
import os
import unreal

OUT = os.path.join(unreal.Paths.project_dir(), "Saved", "_phase6_verify.json")
result = {"ok": False, "checks": {}, "error": ""}
try:
    loader = unreal.new_object(unreal.CraftingTreeLoader)
    n_items = loader.load_crafting_tree()
    n_recipes = loader.load_recipes()

    proc_recipes = loader.get_recipes_for_facility("Processing")
    recipe_ids = [r.get_editor_property("recipe_id") for r in proc_recipes]
    has_steel = any("SteelAlloy" in rid for rid in recipe_ids)

    # Cargo with ingredients (IronOre x2, CopperOre x1) + capacity.
    cargo = unreal.new_object(unreal.CargoComponent)
    cargo.set_editor_property("cargo_capacity", 1000.0)
    iron = unreal.new_object(unreal.TradeItemDataAsset); iron.set_editor_property("item_id", "IronOre"); iron.set_editor_property("volume_per_unit", 1.0)
    copper = unreal.new_object(unreal.TradeItemDataAsset); copper.set_editor_property("item_id", "CopperOre"); copper.set_editor_property("volume_per_unit", 1.0)
    cargo.add_cargo(iron, 2)
    cargo.add_cargo(copper, 1)

    # Find SteelAlloy recipe + resolve output item.
    steel_recipe = None
    for r in proc_recipes:
        if "SteelAlloy" in r.get_editor_property("recipe_id"):
            steel_recipe = r
            break
    can_before = unreal.CraftingTreeLoader.can_craft_recipe(steel_recipe, cargo) if steel_recipe else None

    # Empty a fresh cargo and check can_craft false.
    cargo2 = unreal.new_object(unreal.CargoComponent); cargo2.set_editor_property("cargo_capacity", 1000.0)
    can_empty = unreal.CraftingTreeLoader.can_craft_recipe(steel_recipe, cargo2) if steel_recipe else None

    # Craft into cargo.
    if steel_recipe:
        crafted = loader.craft_recipe(steel_recipe, cargo)
    else:
        crafted = None

    qty_iron_after = cargo.get_item_quantity_by_id("IronOre")
    qty_copper_after = cargo.get_item_quantity_by_id("CopperOre")
    qty_steel_after = cargo.get_item_quantity_by_id("SteelAlloy")

    result["checks"] = {
        "items loaded": n_items,
        "recipes loaded": n_recipes,
        "processing recipes": len(proc_recipes),
        "steel among processing recipes": bool(has_steel),
        "can craft (empty cargo)": bool(can_empty),
        "can craft (with ingredients)": bool(can_before),
        "crafted": bool(crafted),
        "iron after (expect 0)": qty_iron_after,
        "copper after (expect 0)": qty_copper_after,
        "steel after (expect 1)": qty_steel_after,
    }
    result["ok"] = (
        n_recipes > 0 and has_steel
        and can_empty is False and can_before is True
        and crafted is True and qty_iron_after == 0
        and qty_copper_after == 0 and qty_steel_after == 1
    )
except Exception as ex:  # noqa: BLE001
    result["error"] = "exception: " + repr(ex)

with open(OUT, "w", encoding="utf-8") as fh:
    json.dump(result, fh, indent=2, default=str)
print("phase6_verify ->", OUT, "ok=", result["ok"], "err=", result["error"])
print(json.dumps(result.get("checks", {}), indent=2))