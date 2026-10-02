"""Create a trade item, an asteroid material and an asteroid type for every raw resource the galaxy lists.

Run inside the editor (remote execution). Reads Content/Data/Universe/Galaxy.json (sector resources and
POI resources) and Content/Data/CraftingTree.json (names, descriptions, value, rarity), then for each raw
item makes, when missing:
  /Game/DataAssets/Trading/Items/DA_TradeItem_<Item>   ItemID TradeItem_<Item>
  /Game/Materials/Asteroids/MI_Asteroid_<Item>         tinted instance of M_Asteroid_Master
  /Game/DataAssets/Mining/DA_Asteroid_<Item>           rock type yielding that trade item
Existing assets are left alone (the four hand-made types: Iron, Copper, Titanium, Ice). One AAsteroidField
actor then makes any kind of field by listing these ids in its Ores: rock, ice, wreck debris (salvage)
or gas pockets. Placeholder looks: gas pockets and wreck debris use the rock meshes until the art
to-dos land (progress board: Art > Space props).

Usage (editor Python): exec this file. Pass --dry to only log what would be made.
See docs/11-TECHNICAL_SPECS/GALAXY_PLAN.md, build order step 2.
"""
import json
import os
import sys

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DRY = "--dry" in sys.argv

ITEMS_DIR = "/Game/DataAssets/Trading/Items"
MAT_DIR = "/Game/Materials/Asteroids"
TYPE_DIR = "/Game/DataAssets/Mining"

# Existing assets whose names don't follow DA_TradeItem_<CraftingId>.
TRADE_ITEM_ALIASES = {"Helium3": "DA_TradeItem_Helium-3"}
EXISTING_TYPES = {"IronOre": "DA_Asteroid_Iron", "CopperOre": "DA_Asteroid_Copper",
                  "TitaniumOre": "DA_Asteroid_Titanium", "WaterIce": "DA_Asteroid_Ice"}

# The crafting tree rates every raw item Common / 30, so value comes from the galaxy plan instead:
# the more dangerous the space an item comes from, the more it's worth. (price, spawn weight)
# Iron ore already sells for 50, so common items match it.
TIERS = {
    "common": (50, 3.0),   # Core: iron, copper, nickel, aluminium, silicon, ice, bulk gases, scrap
    "mid": (110, 1.5),     # Free Belts / Marches: titanium, chromium, cobalt, lithium, zinc, ...
    "rare": (260, 0.6),    # Pale Expanse / Graveyard: platinum group, gold, rare earths, uranium, He-3
}
TIER_OF = {
    "NickelOre": "common", "AluminiumOre": "common", "Silicon": "common", "Hydrogen": "common",
    "MethaneGas": "common", "NitrogenGas": "common", "ScrapMetal": "common",
    "ZincOre": "mid", "ManganeseOre": "mid", "ChromiumOre": "mid", "CobaltOre": "mid", "TungstenOre": "mid",
    "LithiumOre": "mid", "NobleGas": "mid", "SalvagedComponents": "mid", "DerelictHullPlate": "mid",
    "GoldOre": "rare", "SilverOre": "rare", "Platinum": "rare", "Palladium": "rare", "RareEarthElements": "rare",
    "UraniumOre": "rare", "CarbonCrystal": "rare", "PreciousStones": "rare", "Helium3": "rare",
}

# Per item look: (base colour A, base colour B, vein colour, metallic, roughness, vein glow, hardness)
LOOKS = {
    "NickelOre":          ((0.22, 0.21, 0.18), (0.10, 0.10, 0.09), (0.75, 0.72, 0.55), 0.6, 0.6, 2.0, 1.2),
    "AluminiumOre":       ((0.30, 0.30, 0.32), (0.16, 0.16, 0.18), (0.85, 0.88, 0.95), 0.5, 0.5, 2.0, 0.9),
    "ZincOre":            ((0.20, 0.22, 0.24), (0.10, 0.11, 0.12), (0.55, 0.75, 0.80), 0.5, 0.6, 2.0, 1.0),
    "ManganeseOre":       ((0.14, 0.10, 0.12), (0.07, 0.05, 0.06), (0.85, 0.40, 0.60), 0.4, 0.7, 2.5, 1.3),
    "ChromiumOre":        ((0.18, 0.20, 0.20), (0.08, 0.09, 0.09), (0.40, 1.00, 0.60), 0.7, 0.4, 3.0, 1.6),
    "CobaltOre":          ((0.10, 0.12, 0.20), (0.05, 0.06, 0.10), (0.20, 0.40, 1.00), 0.6, 0.5, 3.0, 1.7),
    "TungstenOre":        ((0.16, 0.16, 0.16), (0.07, 0.07, 0.07), (0.90, 0.85, 0.75), 0.8, 0.4, 2.0, 2.5),
    "LithiumOre":         ((0.25, 0.22, 0.24), (0.12, 0.10, 0.12), (1.00, 0.45, 0.70), 0.3, 0.6, 3.0, 0.9),
    "Silicon":            ((0.24, 0.22, 0.20), (0.12, 0.11, 0.10), (0.60, 0.70, 0.85), 0.2, 0.5, 1.5, 1.0),
    "GoldOre":            ((0.18, 0.14, 0.08), (0.08, 0.06, 0.04), (1.00, 0.78, 0.25), 0.9, 0.3, 3.0, 1.4),
    "SilverOre":          ((0.20, 0.20, 0.22), (0.09, 0.09, 0.10), (0.92, 0.94, 1.00), 0.9, 0.3, 2.5, 1.4),
    "Platinum":           ((0.22, 0.22, 0.23), (0.10, 0.10, 0.11), (0.85, 0.90, 1.00), 0.9, 0.25, 3.5, 2.2),
    "Palladium":          ((0.21, 0.20, 0.19), (0.10, 0.09, 0.09), (1.00, 0.92, 0.80), 0.9, 0.3, 3.5, 2.2),
    "RareEarthElements":  ((0.16, 0.14, 0.12), (0.08, 0.07, 0.06), (0.70, 1.00, 0.30), 0.4, 0.6, 3.5, 1.8),
    "UraniumOre":         ((0.12, 0.13, 0.10), (0.06, 0.07, 0.05), (0.45, 1.00, 0.20), 0.4, 0.6, 6.0, 2.0),
    "CarbonCrystal":      ((0.05, 0.05, 0.06), (0.02, 0.02, 0.03), (0.70, 0.85, 1.00), 0.2, 0.15, 4.0, 2.5),
    "PreciousStones":     ((0.14, 0.12, 0.14), (0.07, 0.06, 0.07), (0.90, 0.20, 0.50), 0.1, 0.2, 4.0, 2.4),
    # Gas pockets (placeholder look: glowing, smooth).
    "Hydrogen":           ((0.30, 0.40, 0.55), (0.18, 0.25, 0.40), (0.60, 0.80, 1.00), 0.0, 0.1, 4.0, 0.3),
    "MethaneGas":         ((0.40, 0.35, 0.20), (0.25, 0.22, 0.12), (1.00, 0.75, 0.30), 0.0, 0.1, 4.0, 0.3),
    "NitrogenGas":        ((0.30, 0.35, 0.45), (0.18, 0.20, 0.30), (0.55, 0.60, 1.00), 0.0, 0.1, 4.0, 0.3),
    "NobleGas":           ((0.40, 0.25, 0.45), (0.25, 0.15, 0.30), (0.95, 0.40, 1.00), 0.0, 0.1, 5.0, 0.3),
    "Helium3":            ((0.45, 0.40, 0.30), (0.30, 0.25, 0.18), (1.00, 0.95, 0.60), 0.0, 0.1, 6.0, 0.4),
    # Wreck debris (placeholder look: dark hull metal, dull glow).
    "ScrapMetal":         ((0.12, 0.12, 0.13), (0.05, 0.05, 0.06), (0.70, 0.45, 0.30), 0.8, 0.6, 1.0, 1.2),
    "SalvagedComponents": ((0.13, 0.13, 0.14), (0.06, 0.06, 0.07), (0.30, 0.80, 1.00), 0.8, 0.5, 2.0, 1.0),
    "DerelictHullPlate":  ((0.10, 0.10, 0.10), (0.04, 0.04, 0.05), (0.80, 0.30, 0.20), 0.9, 0.5, 1.0, 1.8),
}
GAS = {"Hydrogen", "MethaneGas", "NitrogenGas", "NobleGas", "Helium3"}
SALVAGE = {"ScrapMetal", "SalvagedComponents", "DerelictHullPlate"}


def display_name(item_id, name):
    if item_id in GAS:
        return f"{name} Pocket"
    if item_id in SALVAGE:
        return {"ScrapMetal": "Scrap Debris", "SalvagedComponents": "Wreck Section",
                "DerelictHullPlate": "Derelict Hull Plate"}[item_id]
    return f"{name.replace(' Ore', '')}-Rich Asteroid"


def wanted_items():
    with open(os.path.join(ROOT, "Content/Data/Universe/Galaxy.json"), encoding="utf-8") as f:
        galaxy = json.load(f)
    ids = set()
    for system in galaxy["systems"]:
        for sector in system.get("sectors", []):
            ids.update(r["item"] for r in sector.get("resources", []))
            for poi in sector.get("pois", []):
                ids.update(poi.get("resources", []))
    return ids


def lc(rgb):
    return unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0)


def rock_base(rgb, vein):
    """Darker, and tinted toward the vein colour: the field lighting blows plain greys out to white
    (the hand-made iron/copper/ice rocks use dark, saturated bases)."""
    return tuple(c * 0.6 + v * 0.08 for c, v in zip(rgb, vein))


def saturate(rgb, amount=1.8):
    """Push a colour away from its grey: pale vein colours glow white and bloom over the whole rock."""
    grey = sum(rgb) / 3.0
    return tuple(min(1.0, max(0.0, grey + (c - grey) * amount)) for c in rgb)


def apply_look(mi, look):
    base_a, base_b, vein, metallic, rough, glow, _hardness = look
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_vector_parameter_value(mi, "BaseColorA", lc(rock_base(base_a, vein)))
    mel.set_material_instance_vector_parameter_value(mi, "BaseColorB", lc(rock_base(base_b, vein)))
    mel.set_material_instance_vector_parameter_value(mi, "VeinColor", lc(saturate(vein)))
    # Shiny metal mirrors the field lighting and reads white; rocks stay dull like the hand-made
    # iron rock (metallic 0.3, roughness 0.9) and the ore shows through the glowing veins.
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", min(metallic, 0.35))
    mel.set_material_instance_scalar_parameter_value(mi, "Roughness", max(rough, 0.7))
    mel.set_material_instance_scalar_parameter_value(mi, "VeinEmissive", min(glow, 1.5))
    mel.update_material_instance(mi)


def main():
    with open(os.path.join(ROOT, "Content/Data/CraftingTree.json"), encoding="utf-8") as f:
        crafting = json.load(f)
    raw = {r["OutputItem"] for r in crafting["Recipes"] if r["Tier"] == 1}
    items = crafting["Items"]
    eal = unreal.EditorAssetLibrary
    mel = unreal.MaterialEditingLibrary
    made, skipped = [], []

    # Everything the galaxy lists, plus every item the plan already has a look for (future sectors).
    for item_id in sorted((wanted_items() | set(LOOKS)) & raw):
        info = items[item_id]
        name = info["ItemName"]

        # Trade item.
        ti_name = TRADE_ITEM_ALIASES.get(item_id, f"DA_TradeItem_{item_id}")
        ti_path = f"{ITEMS_DIR}/{ti_name}"
        if not eal.does_asset_exist(ti_path):
            if DRY:
                made.append(ti_path)
            else:
                ti = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    ti_name, ITEMS_DIR, unreal.TradeItemDataAsset, unreal.DataAssetFactory())
                ti.set_editor_property("item_name", name)
                ti.set_editor_property("description", info.get("Description", ""))
                ti.set_editor_property("item_id", f"TradeItem_{item_id}")
                ti.set_editor_property("category", unreal.TradeItemCategory.RAW_MATERIALS)
                ti.set_editor_property("base_price", float(TIERS[TIER_OF.get(item_id, "common")][0]))
                ti.set_editor_property("volume_per_unit", 1.0)
                ti.set_editor_property("mass_per_unit", 1.0)
                ti.set_editor_property("standard_lot_size", 100)
                ti.set_editor_property("typical_market_stock", 1000)
                ti.set_editor_property("replenishment_rate", 100)
                eal.save_loaded_asset(ti)
                made.append(ti_path)

        if item_id in EXISTING_TYPES:
            skipped.append(item_id)
            continue
        look = LOOKS.get(item_id)
        if not look:
            unreal.log_warning(f"ue_make_resource_types: no look for {item_id}, skipped")
            continue
        vein, hardness = look[2], look[6]

        # Material instance. The look is re-applied every run, so LOOKS can be tuned and re-run.
        mi_path = f"{MAT_DIR}/MI_Asteroid_{item_id}"
        template_mi = f"{MAT_DIR}/MI_Asteroid_Ice" if item_id in GAS else f"{MAT_DIR}/MI_Asteroid_Iron"
        if DRY:
            if not eal.does_asset_exist(mi_path):
                made.append(mi_path)
        else:
            if eal.does_asset_exist(mi_path):
                mi = unreal.load_asset(mi_path)
            else:
                mi = eal.duplicate_asset(template_mi, mi_path)
                made.append(mi_path)
            apply_look(mi, look)
            eal.save_loaded_asset(mi)

        # Asteroid type.
        type_path = f"{TYPE_DIR}/DA_Asteroid_{item_id}"
        if not eal.does_asset_exist(type_path):
            if DRY:
                made.append(type_path)
            else:
                at = eal.duplicate_asset(f"{TYPE_DIR}/DA_Asteroid_Iron", type_path)
                at.set_editor_property("display_name", display_name(item_id, name))
                at.set_editor_property("ore_item", unreal.load_asset(ti_path))
                at.set_editor_property("material", unreal.load_asset(mi_path))
                at.set_editor_property("ore_tint", lc(vein))
                at.set_editor_property("hardness", hardness)
                at.set_editor_property("rarity", TIERS[TIER_OF.get(item_id, "common")][1])
                at.set_editor_property("ore_yield_per_second", 3.0 if item_id in GAS else 2.0 / hardness)
                at.set_editor_property("ore_units_at_unit_scale", 150.0 if item_id in GAS else 120.0)
                eal.save_loaded_asset(at)
                made.append(type_path)

    for path in made:
        unreal.log(f"ue_make_resource_types: {'would make' if DRY else 'made'} {path}")
    unreal.log(f"ue_make_resource_types: {len(made)} {'to make' if DRY else 'made'}, "
               f"hand-made types kept: {', '.join(skipped)}")


main()
