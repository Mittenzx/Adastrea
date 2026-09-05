#!/usr/bin/env python3
"""
Station Module Catalog Generator (Phase 3)

Derives the full station module catalog (all 27 craftable modules: 15 core + 12
research labs) from the authoritative crafting tree. The crafting tree is the
single source of truth for module IDs, ingredients, tier, and cost-driven
BuildCost (OutputValue / IngredientCost). This generator re-emits that data in
the module-catalog schema (FStationModuleEntry fields) so the catalog can never
drift from the crafting tree.

Emits:
  - Content/DataAssets/Stations/DA_StationModuleCatalog.uasset.txt  (text mirror)
  - Content/Data/StationModuleCatalog.json                          (runtime-import spec)
  - Tools/tmp/build_costs.csv                                      (audit)

Consumed at runtime by UStationModuleCatalog::LoadCatalogFromJson (C++ loader)
which populates ModuleEntries, mirroring UCraftingTreeLoader.

Usage:
    python Tools/generate_station_module_catalog.py
"""

import csv
import json
import sys
from pathlib import Path

PROJECT = Path(__file__).resolve().parent.parent
CRAFTING = PROJECT / "Content" / "Data" / "CraftingTree.json"
MIRROR = PROJECT / "Content" / "DataAssets" / "Stations" / "DA_StationModuleCatalog.uasset.txt"
CATALOG_JSON = PROJECT / "Content" / "Data" / "StationModuleCatalog.json"
TMP = PROJECT / "Tools" / "tmp"

# The 27 craftable station modules → their C++ class (native, no BP shim needed)
# and their functional group. IDs match exactly the crafting tree OutputItems.
MODULE_MAP = {
    # name (craft item)            C++ class path                           group
    "CorridorModule":        ("A" + "CorridorModule",        "/Script/Adastrea.CorridorModule",        "Connection"),
    "CargoBayModule":        ("A" + "CargoBayModule",        "/Script/Adastrea.CargoBayModule",        "Storage"),
    "DockingBayModule":      ("A" + "DockingBayModule",      "/Script/Adastrea.DockingBayModule",      "Docking"),
    "DockingPortModule":     ("A" + "DockingPortModule",     "/Script/Adastrea.DockingPortModule",     "Docking"),
    "HabitationModule":      ("A" + "HabitationModule",      "/Script/Adastrea.HabitationModule",      "Habitation"),
    "BarracksModule":        ("A" + "BarracksModule",        "/Script/Adastrea.BarracksModule",        "Habitation"),
    "MarketplaceModule":     ("A" + "MarketplaceModule",     "/Script/Adastrea.MarketplaceModule",     "Public"),
    "ScienceLabModule":      ("A" + "ScienceLabModule",      "/Script/Adastrea.ScienceLabModule",      "Processing"),
    "ReactorModule":         ("A" + "ReactorModule",         "/Script/Adastrea.ReactorModule",         "Power"),
    "SolarArrayModule":      ("A" + "SolarArrayModule",      "/Script/Adastrea.SolarArrayModule",      "Power"),
    "ProcessingModule":      ("A" + "ProcessingModule",      "/Script/Adastrea.ProcessingModule",      "Processing"),
    "FabricationModule":     ("A" + "FabricationModule",     "/Script/Adastrea.FabricationModule",     "Processing"),
    "FuelDepotModule":       ("A" + "FuelDepotModule",       "/Script/Adastrea.FuelDepotModule",       "Storage"),
    "TurretModule":          ("A" + "TurretModule",          "/Script/Adastrea.TurretModule",          "Defence"),
    "ShieldGeneratorModule": ("A" + "ShieldGeneratorModule", "/Script/Adastrea.ShieldGeneratorModule", "Defence"),
    # 12 research labs (T6 base + T7 niche). Groups = Processing (research/fabrication).
    "PhysicsLabModule":        ("A" + "PhysicsLabModule",        "/Script/Adastrea.PhysicsLabModule",        "Processing"),
    "MaterialsLabModule":      ("A" + "MaterialsLabModule",      "/Script/Adastrea.MaterialsLabModule",      "Processing"),
    "ElectronicsLabModule":    ("A" + "ElectronicsLabModule",    "/Script/Adastrea.ElectronicsLabModule",    "Processing"),
    "WeaponsLabModule":        ("A" + "WeaponsLabModule",        "/Script/Adastrea.WeaponsLabModule",        "Processing"),
    "BiologyLabModule":        ("A" + "BiologyLabModule",        "/Script/Adastrea.BiologyLabModule",        "Processing"),
    "ProjectileWeaponsLab":    ("A" + "ProjectileWeaponsLab",    "/Script/Adastrea.ProjectileWeaponsLab",    "Processing"),
    "BeamWeaponsLab":          ("A" + "BeamWeaponsLab",          "/Script/Adastrea.BeamWeaponsLab",          "Processing"),
    "IonPropulsionLab":        ("A" + "IonPropulsionLab",        "/Script/Adastrea.IonPropulsionLab",        "Processing"),
    "GravMaterialsLab":        ("A" + "GravMaterialsLab",        "/Script/Adastrea.GravMaterialsLab",        "Processing"),
    "EncryptionLab":           ("A" + "EncryptionLab",           "/Script/Adastrea.EncryptionLab",           "Processing"),
    "OptronicsLab":            ("A" + "OptronicsLab",            "/Script/Adastrea.OptronicsLab",            "Processing"),
    "CyberneticsLab":          ("A" + "CyberneticsLab",          "/Script/Adastrea.CyberneticsLab",          "Processing"),
}

# Tech level mapping. Stations are assembled from these modules via the station
# editor (PlayerTechLevel 1..10). Base core modules (T6 recipes) unlock early so a
# player can start building a station; the 5 research lab modules (T6) sit one step
# higher; the niche/contract T7 labs are highest-tier. Aligned to the crafting tree:
#   - core T6 modules          -> TL2
#   - research lab T6 modules  -> TL3
#   - niche / contract T7      -> TL4
TECH_TIER = {6: 2, 7: 4}  # fallback for any tier otherwise present
RESEARCH_TL = 3  # the 5 base research labs (T6)


def tech_level(item_id, tier):
    if item_id.endswith("LabModule"):
        return RESEARCH_TL
    return TECH_TIER.get(tier, 1)


def load_recipes():
    if not CRAFTING.exists():
        print("E: crafting tree not found", CRAFTING)
        sys.exit(1)
    data = json.loads(CRAFTING.read_text(encoding="utf-8"))
    recipes = {r["OutputItem"]: r for r in data["Recipes"]}
    return data, recipes


def human_name(item_id):
    # CargoBayModule -> "Cargo Bay"
    words = []
    for ch in item_id:
        if ch.isupper() and words:
            words.append(" ")
        words.append(ch)
    s = "".join(words).replace("Module", "").strip()
    return s


def main():
    data, recipes = load_recipes()
    economy = data.get("Economy", {})
    items = data.get("Items", {})

    TMP.mkdir(parents=True, exist_ok=True)
    entries = []
    csv_rows = []
    missing = []

    for item_id, (cpp, class_path, group_str) in MODULE_MAP.items():
        recipe = recipes.get(item_id)
        eco = economy.get(item_id, {})
        if not recipe:
            missing.append(item_id)
            continue

        tier = recipe.get("Tier", 6)
        build_cost = eco.get("OutputValue", eco.get("IngredientCost", 0))
        ingredient_cost = eco.get("IngredientCost", 0)
        # Materials map: ingredient item -> qty
        materials = {i["ItemID"]: int(i["Qty"]) for i in recipe.get("Ingredients", [])}

        entry = {
            "item_id": item_id,
            "cpp_class": cpp,
            "class_path": class_path,
            "display_name": human_name(item_id),
            "description": items.get(item_id, {}).get("Description", "Prefabricated station module."),
            "group": group_str,
            "required_tech_level": tech_level(item_id, tier),
            "build_cost_credits": int(round(build_cost)),
            "build_time_seconds": 60.0 * max(1, int(round(ingredient_cost / 15000.0))),
            "materials": materials,
            "recipe_tier": tier,
        }
        entries.append(entry)
        csv_rows.append({
            "item_id": item_id, "tier": tier, "tech_level": entry["required_tech_level"],
            "build_cost": entry["build_cost_credits"], "ingredient_cost": ingredient_cost,
            "build_time": entry["build_time_seconds"], "n_ingredients": len(materials),
        })

    # --- write text mirror (house .uasset.txt style) ---
    lines = [
        "# DA_StationModuleCatalog",
        "# Station Module Catalog Configuration",
        "# Source: CraftingTree.json (authoritative) via generate_station_module_catalog.py",
        "",
        f"ModuleCount: {len(entries)}",
        "",
    ]
    for e in sorted(entries, key=lambda x: x["item_id"]):
        mats = ", ".join(f"{k}x{v}" for k, v in sorted(e["materials"].items()))
        lines += [
            f"Module: {e['item_id']}",
            f"  Class: {e['class_path']}",
            f"  DisplayName: {e['display_name']}",
            f"  Group: {e['group']}",
            f"  RequiredTechLevel: {e['required_tech_level']}",
            f"  BuildCost.Credits: {e['build_cost_credits']}",
            f"  BuildTime: {e['build_time_seconds']}",
            f"  Materials: {mats if mats else '(none)'}",
            f"  Description: {e['description']}",
            "",
        ]
    MIRROR.write_text("\n".join(lines), encoding="utf-8")

    # --- editor/runtime import spec (mirrors CraftingTree.json location) ---
    CATALOG_JSON.write_text(
        json.dumps({"entries": entries}, indent=2), encoding="utf-8")
    with open(TMP / "build_costs.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(csv_rows[0].keys()))
        w.writeheader()
        w.writerows(csv_rows)

    print(f"Wrote {len(entries)} module catalog entries (+{len(missing)} missing):")
    for m in missing:
        print(f"  ! no craft tree recipe for {m}")
    for e in sorted(entries, key=lambda x: x["required_tech_level"]):
        print(f"  TL{e['required_tech_level']} {e['item_id']:<26} {e['build_cost_credits']:>8} cr  {e['group']}")
    return 0 if not missing else 1


if __name__ == "__main__":
    sys.exit(main())