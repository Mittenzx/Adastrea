"""
Phase 6 module-level verification (world-spawned): spawns a real AProcessingModule
into the editor world, then exercises GetCraftableRecipes / CanCraft / CraftItem
(the actual game-facing path).
"""
import json
import os
import unreal

OUT = os.path.join(unreal.Paths.project_dir(), "Saved", "_phase6_module_verify.json")
result = {"ok": False, "checks": {}, "error": ""}
try:
    editor = unreal.EditorLevelLibrary
    # Spawn the module into the current editor world so it has a real world/context.
    mod = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ProcessingModule, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))

    recs = mod.get_craftable_recipes()
    steel = any("SteelAlloy" in r.get_editor_property("recipe_id") for r in recs)

    cargo = unreal.new_object(unreal.CargoComponent)
    cargo.set_editor_property("cargo_capacity", 1000.0)
    iron = unreal.new_object(unreal.TradeItemDataAsset); iron.set_editor_property("item_id", "IronOre"); iron.set_editor_property("volume_per_unit", 1.0)
    copper = unreal.new_object(unreal.TradeItemDataAsset); copper.set_editor_property("item_id", "CopperOre"); copper.set_editor_property("volume_per_unit", 1.0)
    cargo.add_cargo(iron, 3)
    cargo.add_cargo(copper, 2)

    can = mod.can_craft("SteelAlloy", cargo)
    crafted = mod.craft_item("SteelAlloy", cargo)
    can_wrong_facility = mod.can_craft("ShipComponents", cargo)

    result["checks"] = {
        "craftable recipes": len(recs),
        "steel craftable in processing": bool(steel),
        "can craft SteelAlloy": bool(can),
        "crafted SteelAlloy": bool(crafted),
        "Fabrication recipe rejected in Processing": bool(can_wrong_facility),
        "iron after": cargo.get_item_quantity_by_id("IronOre"),
        "copper after": cargo.get_item_quantity_by_id("CopperOre"),
        "steel after": cargo.get_item_quantity_by_id("SteelAlloy"),
    }
    result["ok"] = (
        len(recs) > 0 and steel and can is True and crafted is True
        and can_wrong_facility is False
        and cargo.get_item_quantity_by_id("IronOre") == 1
        and cargo.get_item_quantity_by_id("CopperOre") == 1
        and cargo.get_item_quantity_by_id("SteelAlloy") == 1
    )
except Exception as ex:  # noqa: BLE001
    result["error"] = "exception: " + repr(ex)

with open(OUT, "w", encoding="utf-8") as fh:
    json.dump(result, fh, indent=2, default=str)
print("phase6_module_verify ->", OUT, "ok=", result["ok"], "err=", result["error"])
print(json.dumps(result.get("checks", {}), indent=2))