#!/usr/bin/env python3
"""
Adastrea Crafting & Building Tree — authoritative generator.

Defines the COMPLETE crafting/building catalog (raw acquisition -> refined ->
components -> ship parts/weapons -> station parts -> modules -> station assembly)
and emits the canonical machine-readable file Content/Data/CraftingTree.json.

This generator IS the single source of truth for the tree's recipe catalog.
The JSON it emits is what the game reads and what tests/ the diagram generate
against.

Run:
    python docs/11-TECHNICAL_SPECS/generate_crafting_tree.py
    pytest tests/test_crafting_tree.py -v

The generator validates the tree before writing:
  - unique RecipeID / unique OutputItem
  - every ItemID matches ^[A-Za-z][A-Za-z0-9_]*$
  - canonical ProducedIn tags
  - acquisition (Tier1) recipes have no ingredients; crafted do
  - every ingredient is producible by some recipe
  - tier monotonicity (ingredient tier <= output tier)
  - acyclic recipe graph
"""
import json
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(ROOT, "Content", "Data", "CraftingTree.json")

ITEM_ID_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
PRODUCED_IN_TAGS = {
    "ShipMining", "GasHarvesting", "IceCollection", "OrganicFarming", "Salvage",
    "Processing", "Fabrication", "Reactor", "SolarArray", "ScienceLab", "FuelDepot",
}

TIER_LABELS = {
    1: "Raw Acquisition", 2: "Refined Materials", 3: "Components & Electronics",
    4: "Ship Parts / Weapons", 5: "Station Construction Parts",
    6: "Station Modules (Assembly)", 7: "Station Assembly",
}


# --------------------------------------------------------------------------
# Recipe catalog.  Each entry:
#   (output, tier, produced_in, category, [ (ingredient, qty), ... ])
# Acquisition recipes (Tier 1) have an empty ingredient list.
# --------------------------------------------------------------------------
R = []
def add(output, tier, produced_in, category, ingredients=()):
    R.append({
        "OutputItem": output,
        "Tier": tier,
        "ProducedIn": produced_in,
        "Category": category,
        "Ingredients": [{"ItemID": i, "Qty": q} for i, q in ingredients],
    })


# =========================== TIER 1 : RAW ACQUISITION ==========================
# --- ShipMining (metals / minerals / semiconductors / rare earths) ---
METALS_ORE = [
    ("IronOre", "RawMaterials"), ("CopperOre", "RawMaterials"),
    ("TitaniumOre", "RawMaterials"), ("NickelOre", "RawMaterials"),
    ("ZincOre", "RawMaterials"), ("ChromiumOre", "RawMaterials"),
    ("AluminiumOre", "RawMaterials"), ("ManganeseOre", "RawMaterials"),
    ("GoldOre", "RawMaterials"), ("SilverOre", "RawMaterials"),
    ("Platinum", "RawMaterials"), ("Palladium", "RawMaterials"),
    ("CobaltOre", "RawMaterials"), ("TungstenOre", "RawMaterials"),
    ("UraniumOre", "RawMaterials"), ("LithiumOre", "RawMaterials"),
]
for ore, cat in METALS_ORE:
    add(ore, 1, "ShipMining", cat)

MINERALS = [
    ("Silicon", "RawMaterials"), ("CarbonCrystal", "RawMaterials"),
    ("RareEarthElements", "RawMaterials"), ("PreciousStones", "RawMaterials"),
    ("RawOre", "RawMaterials"),
]
for m, cat in MINERALS:
    add(m, 1, "ShipMining", cat)

# --- Gas harvesting ---
GASES = [
    ("Helium3", "RawMaterials"), ("Hydrogen", "RawMaterials"),
    ("MethaneGas", "RawMaterials"), ("NitrogenGas", "RawMaterials"),
    ("NobleGas", "RawMaterials"),
]
for g, cat in GASES:
    add(g, 1, "GasHarvesting", cat)

# --- Ice / water collection ---
add("WaterIce", 1, "IceCollection", "RawMaterials")

# --- Farming / organic ---
ORGANICS = [
    ("OrganicBiomass", "RawMaterials"), ("AlgaeBiomass", "RawMaterials"),
    ("PlantFibre", "RawMaterials"),
]
for o, cat in ORGANICS:
    add(o, 1, "OrganicFarming", cat)

# --- Salvage ---
SALVAGE = [
    ("SalvagedComponents", "Components"), ("ScrapMetal", "RawMaterials"),
    ("DerelictHullPlate", "RawMaterials"),
]
for s, cat in SALVAGE:
    add(s, 1, "Salvage", cat)

# =========================== TIER 2 : REFINED MATERIALS ==========================
# --- Metal alloys / refined metals (Processing) ---
add("SteelAlloy", 2, "Processing", "RefinedGoods", [("IronOre", 2), ("CopperOre", 1)])
add("CopperWiring", 2, "Processing", "RefinedGoods", [("CopperOre", 2)])
add("NickelSteel", 2, "Processing", "RefinedGoods", [("NickelOre", 2), ("IronOre", 1)])
add("ZincPlate", 2, "Processing", "RefinedGoods", [("ZincOre", 2)])
add("ChromiumPlating", 2, "Processing", "RefinedGoods", [("ChromiumOre", 2)])
add("AluminiumComposite", 2, "Processing", "RefinedGoods", [("AluminiumOre", 2), ("ManganeseOre", 1)])
add("GoldIngot", 2, "Processing", "RefinedGoods", [("GoldOre", 2)])
add("SilverIngot", 2, "Processing", "RefinedGoods", [("SilverOre", 2)])
add("TitaniumAlloy", 2, "Processing", "RefinedGoods", [("TitaniumOre", 2), ("RareEarthElements", 1)])
add("TungstenCarbide", 2, "Processing", "RefinedGoods", [("TungstenOre", 2), ("CarbonCrystal", 1)])
add("PlatinumCatalyst", 2, "Processing", "RefinedGoods", [("Platinum", 1), ("RareEarthElements", 1)])
add("PalladiumCatalyst", 2, "Processing", "RefinedGoods", [("Palladium", 1), ("Platinum", 1)])
add("CobaltMagnet", 2, "Processing", "RefinedGoods", [("CobaltOre", 2), ("RareEarthElements", 1)])
add("EnrichedUranium", 2, "Processing", "RefinedGoods", [("UraniumOre", 2), ("CarbonCrystal", 1)])
add("LithiumCompound", 2, "Processing", "RefinedGoods", [("LithiumOre", 2)])

# --- Wafers / advanced materials (Processing) ---
add("SiliconWafer", 2, "Processing", "RefinedGoods", [("Silicon", 3)])
add("CarbonFibre", 2, "Processing", "RefinedGoods", [("CarbonCrystal", 2)])
add("GlassComposite", 2, "Processing", "RefinedGoods", [("Silicon", 1), ("CopperOre", 1)])
add("CeramicTiles", 2, "Processing", "RefinedGoods", [("Silicon", 1), ("RareEarthElements", 1)])

# --- Processed chemicals / gases (Processing) ---
add("LiquidHydrogen", 2, "Processing", "RefinedGoods", [("Hydrogen", 2)])
add("CompressedMethane", 2, "Processing", "RefinedGoods", [("MethaneGas", 2)])
add("LiquidNitrogen", 2, "Processing", "RefinedGoods", [("NitrogenGas", 2)])
add("HydrogenFuel", 2, "FuelDepot", "RefinedGoods", [("Hydrogen", 2), ("WaterIce", 1)])

# --- Power cells (SolarArray) / advanced power (Reactor) ---
add("PowerCells", 2, "SolarArray", "RefinedGoods", [("SiliconWafer", 2)])
add("AdvancedPowerCells", 3, "Reactor", "Technology", [("PowerCells", 2), ("PlatinumCatalyst", 1)])
add("FusionFuelCell", 3, "Reactor", "RefinedGoods", [("Helium3", 3), ("PlatinumCatalyst", 1), ("LiquidHydrogen", 1)])

# --- Bio / food (Processing) ---
add("AlgaePaste", 2, "Processing", "Food", [("AlgaeBiomass", 2)])
add("SyntheticProtein", 2, "Processing", "Food", [("OrganicBiomass", 2), ("WaterIce", 1)])
add("FoodRations", 2, "Processing", "Food", [("SyntheticProtein", 1), ("WaterIce", 1)])
add("MRE_Rations", 2, "Processing", "Food", [("FoodRations", 1), ("AlgaePaste", 1)])
add("BioCompound", 2, "Processing", "RefinedGoods", [("OrganicBiomass", 2), ("LiquidNitrogen", 1)])
add("SyntheticTextiles", 2, "Processing", "RefinedGoods", [("PlantFibre", 2), ("BioCompound", 1)])

# =========================== TIER 3 : COMPONENTS & ELECTRONICS ==========================
# --- Mechanical components (Fabrication) ---
add("StructuralFrame", 3, "Fabrication", "Components", [("SteelAlloy", 3)])
add("CompositePlating", 3, "Fabrication", "Components", [("TitaniumAlloy", 2), ("OrganicBiomass", 1)])
add("ServoActuator", 3, "Fabrication", "Components", [("SteelAlloy", 1), ("CopperWiring", 2)])
add("GearAssembly", 3, "Fabrication", "Components", [("NickelSteel", 2), ("ZincPlate", 1)])
add("HydraulicPiston", 3, "Fabrication", "Components", [("SteelAlloy", 2), ("TungstenCarbide", 1)])
add("PrecisionFastener", 3, "Fabrication", "Components", [("NickelSteel", 1), ("SteelAlloy", 1)])
add("ReinforcedGirder", 3, "Fabrication", "Components", [("StructuralFrame", 1), ("NickelSteel", 2)])
add("ArmourPlate", 3, "Fabrication", "Components", [("ChromiumPlating", 2), ("TungstenCarbide", 1)])
add("SuperConductingWire", 3, "Fabrication", "Components", [("CopperWiring", 2), ("CobaltMagnet", 1)])

# --- Electronics / computing (Fabrication) ---
add("Electronics", 3, "Fabrication", "Components", [("CopperWiring", 2), ("SiliconWafer", 2)])
add("Microchips", 3, "Fabrication", "Components", [("SiliconWafer", 3), ("PlatinumCatalyst", 1)])
add("CircuitBoard", 3, "Fabrication", "Components", [("Electronics", 2), ("Microchips", 1), ("GlassComposite", 1)])
add("MemoryUnit", 3, "Fabrication", "Technology", [("Microchips", 2), ("SiliconWafer", 1)])
add("ControlUnit", 3, "Fabrication", "Components", [("BasicComputer", 1), ("CopperWiring", 2)])
add("PowerDistributionUnit", 3, "Fabrication", "Components", [("PowerCells", 2), ("Electronics", 1)])
add("BasicComputer", 3, "Fabrication", "Technology", [("Electronics", 2), ("Microchips", 1)])

# --- Advanced tech (ScienceLab) ---
add("QuantumProcessor", 3, "ScienceLab", "Technology", [("Microchips", 4), ("PlatinumCatalyst", 2), ("RareEarthElements", 1)])
add("AdvancedSensors", 3, "ScienceLab", "Technology", [("Electronics", 2), ("SiliconWafer", 1), ("PowerCells", 1)])
add("NavigationComputer", 3, "ScienceLab", "Technology", [("QuantumProcessor", 1), ("AdvancedSensors", 1), ("MemoryUnit", 1)])
add("TargetingComputer", 3, "ScienceLab", "Technology", [("AdvancedSensors", 1), ("QuantumProcessor", 1), ("CircuitBoard", 1)])
add("RefinedMedicine", 3, "Processing", "Medical", [("BioCompound", 2), ("SyntheticProtein", 1), ("LiquidNitrogen", 1)])
add("MedicalSupplies", 3, "Processing", "Medical", [("RefinedMedicine", 1), ("WaterIce", 1), ("SyntheticTextiles", 1)])
add("Vaccines", 3, "ScienceLab", "Medical", [("RefinedMedicine", 2), ("QuantumProcessor", 1)])

# --- Luxury goods (Fabrication / Processing) ---
add("FineSpirits", 3, "Processing", "Luxury", [("WaterIce", 2), ("AlgaePaste", 1)])
add("LuxuryBeverages", 3, "Processing", "Luxury", [("WaterIce", 2), ("OrganicBiomass", 1)])
add("Jewelry", 4, "Fabrication", "Luxury", [("GoldIngot", 1), ("SilverIngot", 1), ("PreciousStones", 1)])
add("FineSilk", 3, "Processing", "Luxury", [("SyntheticTextiles", 2), ("PlantFibre", 1)])
add("LuxuryFurniture", 4, "Fabrication", "Luxury", [("CeramicTiles", 2), ("AluminiumComposite", 1), ("FineSilk", 1)])

# =========================== TIER 4 : SHIP PARTS / WEAPONS ==========================
# --- Ship sub-assemblies (Fabrication) ---
add("ShipComponents", 4, "Fabrication", "Components", [("StructuralFrame", 2), ("CompositePlating", 1), ("Electronics", 1)])
add("HullPlating", 4, "Fabrication", "Military", [("CompositePlating", 3), ("TitaniumAlloy", 1)])
add("ArmourHull", 4, "Fabrication", "Military", [("ArmourPlate", 4), ("CarbonFibre", 1)])
add("Thruster", 4, "Fabrication", "Military", [("SteelAlloy", 2), ("PowerDistributionUnit", 1), ("TungstenCarbide", 1)])
add("ShipEngine", 4, "Fabrication", "Military", [("Thruster", 1), ("ShipComponents", 1), ("AdvancedPowerCells", 1)])
add("ShipReactor", 4, "Fabrication", "Military", [("AdvancedPowerCells", 2), ("ShipComponents", 2), ("StructuralFrame", 1), ("FusionFuelCell", 1)])
add("LifeSupportPod", 4, "Fabrication", "Military", [("BasicComputer", 1), ("RefinedMedicine", 1), ("CompositePlating", 1)])
add("FuelTank", 4, "Fabrication", "Military", [("SteelAlloy", 2), ("ReinforcedGirder", 1), ("CeramicTiles", 1)])
add("CargoPod", 4, "Fabrication", "Military", [("StructuralFrame", 2), ("SteelAlloy", 1), ("ZincPlate", 1)])
add("GrapplerBoom", 4, "Fabrication", "Military", [("HydraulicPiston", 2), ("ReinforcedGirder", 1), ("ServoActuator", 1)])
add("ShipShieldGenerator", 4, "Fabrication", "Military", [("AdvancedSensors", 1), ("AdvancedPowerCells", 2), ("CompositePlating", 2)])

# --- Weapons (Fabrication) ---
add("TurretWeapon", 4, "Fabrication", "Military", [("SteelAlloy", 3), ("ShipComponents", 1), ("ControlUnit", 1)])
add("EnergyCannon", 4, "Fabrication", "Military", [("TurretWeapon", 1), ("AdvancedPowerCells", 2), ("AdvancedSensors", 1)])
add("TriLaser", 4, "Fabrication", "Military", [("EnergyCannon", 1), ("SuperConductingWire", 2), ("Microchips", 1)])
add("MissileLauncher", 4, "Fabrication", "Military", [("TurretWeapon", 1), ("ShipComponents", 1), ("GearAssembly", 2), ("FusionFuelCell", 1)])
add("Railgun", 4, "Fabrication", "Military", [("TungstenCarbide", 3), ("SuperConductingWire", 3), ("ShipComponents", 1)])
add("PlasmaCannon", 4, "Fabrication", "Military", [("EnergyCannon", 1), ("SuperConductingWire", 2), ("AdvancedPowerCells", 2)])
add("TorpedoLauncher", 4, "Fabrication", "Military", [("MissileLauncher", 1), ("ShipComponents", 1), ("CobaltMagnet", 1)])
add("ShieldCapacitor", 4, "Fabrication", "Military", [("AdvancedPowerCells", 3), ("CobaltMagnet", 2), ("SuperConductingWire", 2)])
add("PointDefenceLaser", 4, "Fabrication", "Military", [("TriLaser", 1), ("TargetingComputer", 1), ("PowerDistributionUnit", 1)])

# --- Contraband / special (ScienceLab) ---
add("ProhibitedWeapons", 4, "ScienceLab", "Contraband", [("EnergyCannon", 1), ("QuantumProcessor", 1)])
add("NanoInjectors", 4, "ScienceLab", "Contraband", [("QuantumProcessor", 1), ("BioCompound", 2), ("PalladiumCatalyst", 1)])

# --- AI / research (ScienceLab) ---
add("AICores", 4, "ScienceLab", "Technology", [("QuantumProcessor", 1), ("Microchips", 2)])
add("ResearchData", 4, "ScienceLab", "Data", [("AdvancedSensors", 1), ("BasicComputer", 1)])
add("AdvancedAICore", 5, "ScienceLab", "Technology", [("AICores", 1), ("QuantumProcessor", 2), ("ResearchData", 1)])
add("HackWare", 4, "ScienceLab", "Data", [("AICores", 1), ("NavigationComputer", 1), ("MemoryUnit", 1)])

# =========================== TIER 5 : STATION CONSTRUCTION PARTS ==========================
add("ConstructionMaterials", 5, "Fabrication", "Components", [("StructuralFrame", 2), ("SteelAlloy", 2), ("CompositePlating", 1)])
add("StationHullFrame", 5, "Fabrication", "Components", [("ConstructionMaterials", 2), ("TitaniumAlloy", 2)])
add("PowerCore", 5, "Reactor", "Technology", [("AdvancedPowerCells", 3), ("ControlUnit", 1), ("QuantumProcessor", 1)])
add("LifeSupportUnit", 5, "Fabrication", "Components", [("Electronics", 2), ("StructuralFrame", 1), ("BasicComputer", 1)])
add("ModuleShell", 5, "Fabrication", "Components", [("StationHullFrame", 1), ("CompositePlating", 1)])
add("StructuralGirder", 5, "Fabrication", "Components", [("ReinforcedGirder", 2), ("NickelSteel", 2)])
add("HullSegment", 5, "Fabrication", "Components", [("StationHullFrame", 1), ("ArmourPlate", 2)])
add("DockingMechanism", 5, "Fabrication", "Components", [("HydraulicPiston", 2), ("ServoActuator", 2), ("SteelAlloy", 2)])
add("HabitationPod", 5, "Fabrication", "Components", [("ModuleShell", 1), ("LifeSupportUnit", 1), ("CeramicTiles", 1)])
add("CorridorSegment", 5, "Fabrication", "Components", [("ModuleShell", 1), ("StructuralGirder", 1)])
add("AirTank", 5, "Fabrication", "Components", [("SteelAlloy", 1), ("CeramicTiles", 1), ("LiquidNitrogen", 1)])
add("GravitationGenerator", 5, "Reactor", "Technology", [("CobaltMagnet", 3), ("SuperConductingWire", 3), ("AdvancedPowerCells", 1)])
add("ShieldEmitter", 5, "Fabrication", "Components", [("ShipShieldGenerator", 1), ("AdvancedPowerCells", 2), ("CarbonFibre", 1)])

# =========================== TIER 6 : STATION MODULES ==========================
add("CorridorModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("CorridorSegment", 1)])
add("CargoBayModule", 6, "Fabrication", "Other", [("ModuleShell", 3), ("ConstructionMaterials", 1), ("CargoPod", 1)])
add("DockingBayModule", 6, "Fabrication", "Other", [("ModuleShell", 3), ("ControlUnit", 1), ("DockingMechanism", 1)])
add("DockingPortModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("DockingMechanism", 1)])
add("HabitationModule", 6, "Fabrication", "Other", [("ModuleShell", 3), ("LifeSupportUnit", 2), ("HabitationPod", 1)])
add("BarracksModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("LifeSupportUnit", 1), ("HabitationPod", 1)])
add("MarketplaceModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("BasicComputer", 3), ("LifeSupportUnit", 1)])
add("ScienceLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("QuantumProcessor", 1), ("AdvancedSensors", 1)])
add("ReactorModule", 6, "Fabrication", "Other", [("PowerCore", 2), ("ModuleShell", 2)])
add("SolarArrayModule", 6, "Fabrication", "Other", [("PowerCells", 4), ("SiliconWafer", 3), ("ModuleShell", 1)])
add("ProcessingModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("SteelAlloy", 2), ("ControlUnit", 1)])
add("FabricationModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("ControlUnit", 2), ("ConstructionMaterials", 1)])
add("FuelDepotModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("SteelAlloy", 2), ("FuelTank", 1)])
add("TurretModule", 6, "Fabrication", "Other", [("TurretWeapon", 2), ("ModuleShell", 2)])
add("ShieldGeneratorModule", 6, "Fabrication", "Other", [("ShieldEmitter", 1), ("ModuleShell", 2), ("AdvancedPowerCells", 2)])

# =========================== TIER 7 : STATION ASSEMBLY ==========================
add("TradeStation", 7, "Fabrication", "Other", [("DockingBayModule", 1), ("MarketplaceModule", 1), ("CargoBayModule", 1)])
add("IndustrialStation", 7, "Fabrication", "Other", [("ProcessingModule", 2), ("FabricationModule", 1), ("ReactorModule", 1)])
add("MiningStation", 7, "Fabrication", "Other", [("ProcessingModule", 1), ("CargoBayModule", 1), ("PowerCore", 1)])
add("ResearchStation", 7, "Fabrication", "Other", [("ScienceLabModule", 2), ("SolarArrayModule", 1), ("AdvancedAICore", 1)])
add("DefenceStation", 7, "Fabrication", "Other", [("TurretModule", 2), ("ShieldGeneratorModule", 1), ("DockingPortModule", 1)])
add("ColonyStation", 7, "Fabrication", "Other", [("HabitationModule", 2), ("BarracksModule", 1), ("LifeSupportUnit", 2)])

# =========================== Derived helper tables ==========================
def cat(tier):
    pass


def check():
    """Structural validation — mirrors tests/test_crafting_tree.py."""
    from collections import OrderedDict
    errors = []
    ids = [r["OutputItem"] for r in R]
    rid = [rid_out for rid_out in [None]]  # placeholder
    recipe_ids = []
    for idx, r in enumerate(R):
        recipe_ids.append(f"RCP_{idx+1:03d}_{r['OutputItem']}")
    # uniqueness
    if len(set(ids)) != len(ids):
        dup = [i for i in set(ids) if ids.count(i) > 1]
        errors.append(f"Duplicate OutputItem: {dup}")
    # regex
    for r in R:
        if not ITEM_ID_RE.match(r["OutputItem"]):
            errors.append(f"Bad ItemID: {r['OutputItem']}")
        for ing in r["Ingredients"]:
            if not ITEM_ID_RE.match(ing["ItemID"]):
                errors.append(f"Bad ingredient ItemID: {ing['ItemID']} in {r['OutputItem']}")
    # produced in canonical
    for r in R:
        if r["ProducedIn"] not in PRODUCED_IN_TAGS:
            errors.append(f"Bad ProducedIn: {r['ProducedIn']} for {r['OutputItem']}")
    # acquisition invariants
    for r in R:
        if r["Tier"] == 1:
            if r["Ingredients"]:
                errors.append(f"Tier1 with ingredients: {r['OutputItem']}")
        else:
            if not r["Ingredients"]:
                errors.append(f"Crafted w/o ingredients: {r['OutputItem']}")
    # every ingredient producible
    produced = set(ids)
    for r in R:
        for ing in r["Ingredients"]:
            if ing["ItemID"] not in produced:
                errors.append(f"{r['OutputItem']}: ingredient {ing['ItemID']} has no producer")
    # tier monotonic
    tiers = {r["OutputItem"]: r["Tier"] for r in R}
    for r in R:
        for ing in r["Ingredients"]:
            if ing["ItemID"] in tiers and tiers[ing["ItemID"]] > r["Tier"]:
                errors.append(f"{r['OutputItem']}: ingredient {ing['ItemID']} tier {tiers[ing['ItemID']]} > {r['Tier']}")
    # acyclic
    graph = {r["OutputItem"]: {i["ItemID"] for i in r["Ingredients"] if i["ItemID"] in produced} for r in R}
    color = {}
    stack = []
    cycles = []

    def visit(node):
        color.setdefault(node, 0)
        color[node] = 1
        stack.append(node)
        for dep in graph.get(node, ()):
            if dep not in graph:
                continue
            if color.get(dep, 0) == 1:
                start = stack.index(dep)
                cycles.append(stack[start:] + [dep])
            elif color.get(dep, 0) == 0:
                visit(dep)
        color[node] = 2
        stack.pop()

    for n in graph:
        if color.get(n, 0) == 0:
            visit(n)
    if cycles:
        errors.append(f"Circular recipes: {cycles}")

    return errors


def build_recipes_with_ids():
    out = []
    for idx, r in enumerate(R):
        out.append({
            "RecipeID": f"RCP_{idx+1:03d}_{r['OutputItem']}",
            "OutputItem": r["OutputItem"],
            "OutputQty": 1,
            "Tier": r["Tier"],
            "ProducedIn": r["ProducedIn"],
            "Category": r["Category"],
            "Acquisition": r["Tier"] == 1,
            "Ingredients": r["Ingredients"],
        })
    return out


def main():
    errors = check()
    if errors:
        print("VALIDATION FAILED:")
        for e in errors:
            print("  -", e)
        sys.exit(1)

    recipes = build_recipes_with_ids()
    doc = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "Title": "Adastrea Crafting & Building Tree",
        "Description": "Machine-readable crafting/building tree: raw extraction -> refined materials -> components & electronics -> ship parts -> weapons -> station construction parts -> modules -> station assembly. Authoritative generator: docs/11-TECHNICAL_SPECS/generate_crafting_tree.py",
        "SchemaVersion": "1.1.0",
        "LastUpdated": "2026-08-31",
        "ItemIDConvention": "^[A-Za-z][A-Za-z0-9_]*$",
        "NoteHelium3": "Existing trade asset uses 'Helium-3' (hyphen, violates ItemID regex). Crafting data canonicalizes to 'Helium3' and maps to DA_TradeItem_Helium-3.",
        "ProducedInTags": sorted(PRODUCED_IN_TAGS),
        "TierLabels": TIER_LABELS,
        "Recipes": recipes,
    }
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        json.dump(doc, f, indent=2)
        f.write("\n")
    counts = defaultdict(int)
    for r in recipes:
        counts[r["Tier"]] += 1
    print(f"VALID: {len(recipes)} recipes written to {OUT}")
    print("  per-tier:", ", ".join(f"T{t}={counts[t]}" for t in sorted(counts)))


if __name__ == "__main__":
    main()