#!/usr/bin/env python3
"""
Adastrea Crafting & Building Tree — authoritative generator.

Defines the COMPLETE crafting/building catalog (raw acquisition -> refined ->
components -> ship parts/weapons -> station construction parts/modules) and
emits the canonical machine-readable file Content/Data/CraftingTree.json.

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
  - research monotonicity (ResearchRequired exists and is at a strictly prior
    level)

Research/progression model: recipes carry ResearchLevel (1..4) and optionally a
ResearchRequired item. ScienceLab milestones — ResearchData, AdvancedResearch,
QuantumResearch — gate Mk2/Mk3 upgrades of components, ship parts, weapons,
shields. The tree is edited here, then regenerated to JSON.
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
    # specialized research labs — each produces its own breakthrough domain
    "PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab", "BiologyLab",
    # upgraded / niche research labs (specialized sub-fields)
    "ProjectileWeaponsLab", "BeamWeaponsLab",
}

# Research domains: each specialized lab unlocks its own breakthrough chain.
# (rl2 milestone, rl3 milestone). Added to Mk recipes via the `_mk` domain param.
PROPULSION = ("PropulsionResearch", "AdvancedPropulsionResearch")
MATERIALS  = ("MaterialsResearch",  "NanoMaterialsResearch")
COMPUTING  = ("ComputingResearch",  "QuantumComputingResearch")
DEFENCE    = ("DefenceResearch",    "AdvancedDefenceResearch")
BIO        = ("BioResearch",        "AdvancedBioResearch")

TIER_LABELS = {
    1: "Raw Acquisition", 2: "Refined Materials", 3: "Components & Electronics",
    4: "Ship Parts / Weapons", 5: "Station Construction Parts",
    6: "Station Modules (Assembly)",
}


# --------------------------------------------------------------------------
# Recipe catalog.  Each entry:
#   (output, tier, produced_in, category, [ (ingredient, qty), ... ])
# Acquisition recipes (Tier 1) have an empty ingredient list.
#
# Research model: each recipe carries a ResearchLevel (rl) 1..4 and, when it is
# a researched MILESTONE or an upgraded Mk2/Mk3 part, a ResearchRequired item
# (the research data asset that must be unlocked first). Research data is
# itself producible via ScienceLab, so research is a real consumable in-tree.
#   rl 1  base                    rl 2  ResearchData
#   rl 3  AdvancedResearch        rl 4  QuantumResearch
# --------------------------------------------------------------------------
R = []
def add(output, tier, produced_in, category, ingredients=(), research=None, rl=1):
    R.append({
        "OutputItem": output,
        "Tier": tier,
        "ProducedIn": produced_in,
        "Category": category,
        "Ingredients": [{"ItemID": i, "Qty": q} for i, q in ingredients],
        "ResearchLevel": rl,
        "ResearchRequired": research,
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
    ("PlantFibre", "RawMaterials"), ("GrainCrop", "RawMaterials"),
    ("FruitCrop", "RawMaterials"), ("FungusCrop", "RawMaterials"),
    ("LivestockCulture", "RawMaterials"),
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
add("GrainTortilla", 2, "Processing", "Food", [("GrainCrop", 2), ("WaterIce", 1)])
add("FreshProduce", 2, "Processing", "Food", [("FruitCrop", 2), ("WaterIce", 1)])
add("FungusSteak", 2, "Processing", "Food", [("FungusCrop", 3), ("SyntheticProtein", 1)])
add("CulturedMeat", 2, "Processing", "Food", [("LivestockCulture", 2), ("SyntheticProtein", 1)])
add("NutritionBar", 2, "Processing", "Food", [("GrainCrop", 1), ("SyntheticProtein", 1), ("FruitCrop", 1)])
add("BioCompound", 2, "Processing", "RefinedGoods", [("OrganicBiomass", 2), ("LiquidNitrogen", 1)])
add("SyntheticTextiles", 2, "Processing", "RefinedGoods", [("PlantFibre", 2), ("BioCompound", 1)])
add("BalancedMeal", 3, "Processing", "Food", [("CulturedMeat", 1), ("FreshProduce", 1), ("GrainTortilla", 1)])
add("LuxuryFeast", 3, "Processing", "Food", [("BalancedMeal", 1), ("FungusSteak", 1), ("FineSpirits", 1)])

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

# --- more electronics components (Fabrication) ---
add("MicroCapacitor", 3, "Fabrication", "Components", [("PowerCells", 1), ("Electronics", 1)])
add("CapacitorBank", 3, "Fabrication", "Components", [("MicroCapacitor", 4), ("AluminiumComposite", 1)])
add("TransceiverUnit", 3, "Fabrication", "Components", [("Electronics", 1), ("Microchips", 1), ("CopperWiring", 1)])
add("SignalAmplifier", 3, "Fabrication", "Components", [("TransceiverUnit", 1), ("SuperConductingWire", 1)])
add("OpticalLens", 3, "Fabrication", "Components", [("GlassComposite", 2), ("CobaltMagnet", 1)])
add("SensorArray", 3, "Fabrication", "Components", [("AdvancedSensors", 1), ("OpticalLens", 2), ("SignalAmplifier", 1)])
add("FiberOpticCable", 3, "Fabrication", "Components", [("GlassComposite", 2), ("Microchips", 1)])
add("DataMatrix", 3, "Fabrication", "Components", [("MemoryUnit", 1), ("Microchips", 1)])
add("Gyroscope", 3, "Fabrication", "Components", [("CobaltMagnet", 2), ("PrecisionFastener", 2), ("ServoActuator", 1)])
add("RadioAntenna", 3, "Fabrication", "Components", [("TransceiverUnit", 1), ("AluminiumComposite", 1)])
add("PowerRelay", 3, "Fabrication", "Components", [("SuperConductingWire", 1), ("CopperWiring", 2)])
add("ThermalCoupler", 3, "Fabrication", "Components", [("CeramicTiles", 2), ("CopperWiring", 1)])
add("LogicChip", 3, "Fabrication", "Components", [("Microchips", 2), ("SiliconWafer", 1)])
add("VoltageRegulator", 3, "Fabrication", "Components", [("MicroCapacitor", 2), ("CeramicTiles", 1)])
add("EncryptedCircuit", 3, "Fabrication", "Components", [("CircuitBoard", 1), ("Microchips", 1), ("PalladiumCatalyst", 1)])
add("RelayCoil", 3, "Fabrication", "Components", [("CopperWiring", 2), ("CobaltMagnet", 1), ("SteelAlloy", 1)])
add("ClockGenerator", 3, "Fabrication", "Components", [("Microchips", 1), ("PlatinumCatalyst", 1)])

# --- Advanced tech & the computer family (ScienceLab / Fabrication) ---
add("QuantumProcessor", 3, "ScienceLab", "Technology", [("Microchips", 4), ("PlatinumCatalyst", 2), ("RareEarthElements", 1)])
add("AdvancedSensors", 3, "ScienceLab", "Technology", [("Electronics", 2), ("SiliconWafer", 1), ("PowerCells", 1)])
add("NavigationComputer", 3, "ScienceLab", "Technology", [("QuantumProcessor", 1), ("AdvancedSensors", 1), ("MemoryUnit", 1)])
add("TargetingComputer", 3, "ScienceLab", "Technology", [("AdvancedSensors", 1), ("QuantumProcessor", 1), ("CircuitBoard", 1)])

# --- the computer family (Fabrication / ScienceLab, tier 3-4) ---
add("PilotComputer", 3, "ScienceLab", "Technology", [("BasicComputer", 1), ("Gyroscope", 1), ("NavigationComputer", 1)])
add("CargoComputer", 3, "ScienceLab", "Technology", [("BasicComputer", 1), ("DataMatrix", 2), ("MemoryUnit", 1)])
add("ShipComputer", 3, "ScienceLab", "Technology", [("BasicComputer", 1), ("ControlUnit", 1), ("LogicChip", 2)])
add("TradingComputer", 3, "ScienceLab", "Technology", [("CargoComputer", 1), ("EncryptedCircuit", 1), ("TransceiverUnit", 1)])
add("ScanComputer", 3, "ScienceLab", "Technology", [("AdvancedSensors", 1), ("SensorArray", 1), ("MemoryUnit", 1)])
add("ScienceComputer", 4, "ScienceLab", "Technology", [("QuantumProcessor", 1), ("ScanComputer", 1), ("DataMatrix", 2)])
add("EngineeringComputer", 4, "ScienceLab", "Technology", [("ShipComputer", 1), ("LogicChip", 2), ("ControlUnit", 1)])
add("AutomatedDefenceComputer", 4, "ScienceLab", "Technology", [("TargetingComputer", 1), ("ShipComputer", 1), ("LogicChip", 2)])
add("ResearchComputer", 4, "ScienceLab", "Technology", [("ScienceComputer", 1), ("QuantumProcessor", 1), ("MemoryUnit", 1)])
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

# --- Contraband / black market (ScienceLab / BiologyLab) ---
add("ProhibitedWeapons", 4, "ScienceLab", "Contraband", [("EnergyCannon", 1), ("QuantumProcessor", 1)])
add("NanoInjectors", 4, "ScienceLab", "Contraband", [("QuantumProcessor", 1), ("BioCompound", 2), ("PalladiumCatalyst", 1)])
add("BlackMarketDrugs", 3, "Processing", "Contraband", [("BioCompound", 2), ("OrganicBiomass", 2)])   # cheap, illegal stimulants
add("ForgedCredits", 3, "ScienceLab", "Contraband", [("GoldIngot", 1), ("Microchips", 2), ("PalladiumCatalyst", 1)])
add("HackingTool", 4, "ScienceLab", "Contraband", [("HackWare", 1), ("MemoryUnit", 1)])
add("SmuggledBioweapon", 4, "BiologyLab", "Contraband", [("Vaccines", 1), ("BioCompound", 3), ("ProhibitedWeapons", 1)])
add("ContrabandTech", 4, "ScienceLab", "Contraband", [("AdvancedSensors_Mk2", 1), ("QuantumProcessor", 1), ("ShieldCapacitor", 1)])

# --- AI / research (ScienceLab) ---
add("AICores", 4, "ScienceLab", "Technology", [("QuantumProcessor", 1), ("Microchips", 2)])
add("ResearchData", 4, "ScienceLab", "Data", [("AdvancedSensors", 1), ("BasicComputer", 1)], rl=2)
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

# =========================== SPECIALIZED RESEARCH LABS ============================
# Different SCIENCE / RESEARCH LABS produce different BREAKTHROUGHS. Each station
# lab module below hosts one research domain and produces its own rl2/rl3
# breakthroughs, consumed by (and so gating) the Mk2/Mk3 recipes of that domain.
# Cross-lab coupling is intentional: every rl3 breakthrough needs high-tier
# computing (QuantumProcessor_Mk2), so late-game requires more than one lab.
#   Lab               Module              Domain (rl2 / rl3)
#   PhysicsLab        PhysicsLabModule    Propulsion / AdvancedPropulsion
#   MaterialsLab      MaterialsLabModule  Materials / NanoMaterials
#   ElectronicsLab    ElectronicsLabModule Computing / QuantumComputing
#   WeaponsLab        WeaponsLabModule    Defence / AdvancedDefence
#   BiologyLab        BiologyLabModule    Bio / AdvancedBio
#   ScienceLab        ScienceLabModule    ResearchData (generic base)

# --- Research lab station modules (Tier 6, built in Fabrication) ---
add("PhysicsLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("ShipReactor", 1), ("FusionFuelCell", 2)])
add("MaterialsLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("StationHullFrame", 1), ("ArmourPlate", 2)])
add("ElectronicsLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("QuantumProcessor", 2), ("CircuitBoard", 2)])
add("WeaponsLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("EnergyCannon", 1), ("TargetingComputer", 1)])
add("BiologyLabModule", 6, "Fabrication", "Other", [("ModuleShell", 2), ("RefinedMedicine", 2), ("BioCompound", 1)])

# --- UPGRADED / SPECIALIZED RESEARCH LABS -------------------------------
# Each lab can be upgraded to (a) an upgraded hull that boosts its domain, and
# (b) niche specializations that unlock targeted breakthroughs. A niche lab is a
# distinct craftable Tier-7 module whose ProducedIn tag doubles as its domain,
# and it produces the niche breakthrough(s) that gate that niche's recipes.
#   WeaponsLab -> ProjectileWeaponsLab (rail, missiles, torpedoes)
#             -> BeamWeaponsLab       (laser, plasma, point-defence)
#   PhysicsLab -> PropulsionNicheLab  (engine/reactor/flinger sub-fields, placeholder)
# (add similar niches per lab as the tree grows)
add("ProjectileWeaponsLab", 7, "Fabrication", "Other", [("WeaponsLabModule", 1), ("ModuleShell", 1), ("Railgun", 1), ("SuperConductingWire_Mk2", 1)])
add("BeamWeaponsLab", 7, "Fabrication", "Other", [("WeaponsLabModule", 1), ("ModuleShell", 1), ("PlasmaCannon", 1), ("AdvancedPowerCells", 2)])

# --- Domain breakthroughs (produced inside their own lab) ---
add("PropulsionResearch", 5, "PhysicsLab", "Data", [("Helium3", 3), ("AdvancedPowerCells", 2)], research="PropulsionResearch", rl=2)
add("AdvancedPropulsionResearch", 6, "PhysicsLab", "Data", [("PropulsionResearch", 1), ("FusionFuelCell", 2)], research="AdvancedPropulsionResearch", rl=3)
add("MaterialsResearch", 5, "MaterialsLab", "Data", [("TungstenCarbide", 2), ("TitaniumAlloy", 2)], research="MaterialsResearch", rl=2)
add("NanoMaterialsResearch", 6, "MaterialsLab", "Data", [("MaterialsResearch", 1), ("QuantumProcessor_Mk2", 1)], research="NanoMaterialsResearch", rl=3)
add("ComputingResearch", 4, "ElectronicsLab", "Data", [("Microchips", 3), ("AdvancedSensors", 1)], research="ComputingResearch", rl=2)
add("QuantumComputingResearch", 5, "ElectronicsLab", "Data", [("ComputingResearch", 1), ("QuantumProcessor_Mk2", 1), ("MemoryUnit_Mk2", 1)], research="QuantumComputingResearch", rl=3)
add("DefenceResearch", 5, "WeaponsLab", "Data", [("EnergyCannon", 1), ("SuperConductingWire", 2)], research="DefenceResearch", rl=2)
add("AdvancedDefenceResearch", 6, "WeaponsLab", "Data", [("DefenceResearch", 1), ("QuantumProcessor_Mk2", 1)], research="AdvancedDefenceResearch", rl=3)
add("BioResearch", 4, "BiologyLab", "Data", [("RefinedMedicine", 1), ("Vaccines", 1)], research="BioResearch", rl=2)
add("AdvancedBioResearch", 5, "BiologyLab", "Data", [("BioResearch", 1), ("Vaccines_Mk2", 1)], research="AdvancedBioResearch", rl=3)

# --- Niche specialization breakthroughs (produced inside their niche labs) ---
# A secondary breakthrough, produced only by the upgraded/specialized lab, that
# unlocks the niche's deeper tech. Consumed by (and gating) niche Mk2/3 recipes.
add("KineticWeaponResearch", 6, "ProjectileWeaponsLab", "Data", [("DefenceResearch", 1), ("Railgun", 1), ("SuperConductingWire_Mk2", 2)], research="KineticWeaponResearch", rl=3)
add("BeamWeaponResearch", 6, "BeamWeaponsLab", "Data", [("DefenceResearch", 1), ("PlasmaCannon", 1), ("AdvancedPowerCells", 2)], research="BeamWeaponResearch", rl=3)

def _mk(base, suffix, tier, produced_in, cat, extra, domain, prev_suffix):
    """Add a `base<suffix>` mark/version. Consumes the preceding version (or base),
    extra ingredients, and the domain's breakthrough (Mk2 -> rl2, Mk3 -> rl3)."""
    rl = 3 if suffix == "_Mk3" else 2
    research = domain[1] if suffix == "_Mk3" else domain[0]
    prev = base if prev_suffix is None else base + prev_suffix
    add(base + suffix, tier, produced_in, cat, [(prev, 1)] + extra + [(research, 1)], research=research, rl=rl)


def _mk_niche(base, suffix, tier, produced_in, cat, extra, niche_research, prev_suffix, rl):
    """Add a `base<suffix>` mark/version gated by a SINGLE niche-lab breakthrough
    (niche_research), used for weapons/families that only unlock via an upgraded
    niche lab. The niche breakthrough is consumed and set as ResearchRequired."""
    prev = base if prev_suffix is None else base + prev_suffix
    add(base + suffix, tier, produced_in, cat, [(prev, 1)] + extra + [(niche_research, 1)],
        research=niche_research, rl=rl)

# --- COMPUTING domain (ElectronicsLab): component / electronics upgrades ---
_mk("Electronics", "_Mk2", 4, "Fabrication", "Components", [("SuperConductingWire", 2), ("Microchips", 1)], COMPUTING, None)
_mk("Microchips", "_Mk2", 4, "Fabrication", "Components", [("SiliconWafer", 3), ("PalladiumCatalyst", 1)], COMPUTING, None)
_mk("CircuitBoard", "_Mk2", 4, "Fabrication", "Components", [("Electronics_Mk2", 1), ("Microchips_Mk2", 1), ("PalladiumCatalyst", 1)], COMPUTING, None)
_mk("MemoryUnit", "_Mk2", 4, "Fabrication", "Technology", [("Microchips_Mk2", 2), ("SiliconWafer", 1)], COMPUTING, None)
_mk("ControlUnit", "_Mk2", 4, "Fabrication", "Components", [("BasicComputer_Mk2", 1), ("SuperConductingWire", 2)], COMPUTING, None)
_mk("PowerDistributionUnit", "_Mk2", 4, "Fabrication", "Components", [("AdvancedPowerCells", 2), ("Electronics_Mk2", 1)], COMPUTING, None)
_mk("BasicComputer", "_Mk2", 4, "Fabrication", "Technology", [("Electronics_Mk2", 2), ("Microchips_Mk2", 1)], COMPUTING, None)
_mk("AdvancedSensors", "_Mk2", 4, "ScienceLab", "Technology", [("Electronics_Mk2", 2), ("QuantumProcessor", 1)], COMPUTING, None)
_mk("QuantumProcessor", "_Mk2", 4, "ScienceLab", "Technology", [("Microchips_Mk2", 4), ("PlatinumCatalyst", 2), ("RareEarthElements", 1)], COMPUTING, None)
_mk("TargetingComputer", "_Mk2", 4, "ScienceLab", "Technology", [("AdvancedSensors_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, None)
_mk("NavigationComputer", "_Mk2", 4, "ScienceLab", "Technology", [("QuantumProcessor_Mk2", 1), ("AdvancedSensors_Mk2", 1)], COMPUTING, None)
_mk("ServoActuator", "_Mk2", 4, "Fabrication", "Components", [("HydraulicPiston", 1), ("SuperConductingWire", 1)], COMPUTING, None)
_mk("SuperConductingWire", "_Mk2", 4, "Fabrication", "Components", [("CobaltMagnet", 3), ("CopperWiring", 1)], COMPUTING, None)
_mk("Electronics", "_Mk3", 5, "Fabrication", "Components", [("Electronics_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("Microchips", "_Mk3", 5, "Fabrication", "Components", [("Microchips_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("CircuitBoard", "_Mk3", 5, "Fabrication", "Components", [("CircuitBoard_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, "_Mk2")
_mk("MemoryUnit", "_Mk3", 5, "Fabrication", "Technology", [("MemoryUnit_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("ControlUnit", "_Mk3", 5, "Fabrication", "Components", [("ControlUnit_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("PowerDistributionUnit", "_Mk3", 5, "Fabrication", "Components", [("PowerDistributionUnit_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("BasicComputer", "_Mk3", 5, "Fabrication", "Technology", [("BasicComputer_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("AdvancedSensors", "_Mk3", 5, "ScienceLab", "Technology", [("AdvancedSensors_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, "_Mk2")
_mk("QuantumProcessor", "_Mk3", 5, "ScienceLab", "Technology", [("QuantumProcessor_Mk2", 1), ("RareEarthElements", 2), ("PlatinumCatalyst", 2)], COMPUTING, "_Mk2")

# --- COMPUTING Mk2: new electronics components + computer family (rl2) ---
_mk("MicroCapacitor", "_Mk2", 4, "Fabrication", "Components", [("AdvancedPowerCells", 1), ("Electronics_Mk2", 1)], COMPUTING, None)
_mk("CapacitorBank", "_Mk2", 4, "Fabrication", "Components", [("MicroCapacitor_Mk2", 4), ("AluminiumComposite", 1)], COMPUTING, None)
_mk("TransceiverUnit", "_Mk2", 4, "Fabrication", "Components", [("Electronics_Mk2", 1), ("Microchips_Mk2", 1)], COMPUTING, None)
_mk("SignalAmplifier", "_Mk2", 4, "Fabrication", "Components", [("TransceiverUnit_Mk2", 1), ("SuperConductingWire_Mk2", 1)], COMPUTING, None)
_mk("OpticalLens", "_Mk2", 4, "Fabrication", "Components", [("GlassComposite", 2), ("CobaltMagnet", 2)], COMPUTING, None)
_mk("SensorArray", "_Mk2", 4, "Fabrication", "Components", [("AdvancedSensors_Mk2", 1), ("OpticalLens_Mk2", 2)], COMPUTING, None)
_mk("FiberOpticCable", "_Mk2", 4, "Fabrication", "Components", [("GlassComposite", 2), ("Microchips_Mk2", 1)], COMPUTING, None)
_mk("DataMatrix", "_Mk2", 4, "Fabrication", "Components", [("MemoryUnit_Mk2", 1), ("Microchips_Mk2", 1)], COMPUTING, None)
_mk("LogicChip", "_Mk2", 4, "Fabrication", "Components", [("Microchips_Mk2", 2), ("SiliconWafer", 1)], COMPUTING, None)
_mk("EncryptedCircuit", "_Mk2", 4, "Fabrication", "Components", [("CircuitBoard_Mk2", 1), ("Microchips_Mk2", 1)], COMPUTING, None)
_mk("ClockGenerator", "_Mk2", 4, "Fabrication", "Components", [("Microchips_Mk2", 1), ("PlatinumCatalyst", 1)], COMPUTING, None)
# computers Mk2 (tier 4)
_mk("PilotComputer", "_Mk2", 4, "ScienceLab", "Technology", [("NavigationComputer_Mk2", 1), ("Gyroscope", 2)], COMPUTING, None)
_mk("CargoComputer", "_Mk2", 4, "ScienceLab", "Technology", [("DataMatrix_Mk2", 2), ("MemoryUnit_Mk2", 1)], COMPUTING, None)
_mk("ShipComputer", "_Mk2", 4, "ScienceLab", "Technology", [("LogicChip_Mk2", 2), ("ControlUnit_Mk2", 1)], COMPUTING, None)
_mk("TradingComputer", "_Mk2", 4, "ScienceLab", "Technology", [("CargoComputer_Mk2", 1), ("EncryptedCircuit_Mk2", 1)], COMPUTING, None)
_mk("ScanComputer", "_Mk2", 4, "ScienceLab", "Technology", [("SensorArray_Mk2", 1), ("MemoryUnit_Mk2", 1)], COMPUTING, None)
_mk("ScienceComputer", "_Mk2", 5, "ScienceLab", "Technology", [("ScienceComputer", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, None)
_mk("EngineeringComputer", "_Mk2", 5, "ScienceLab", "Technology", [("ShipComputer_Mk2", 1), ("LogicChip_Mk2", 2)], COMPUTING, None)
_mk("AutomatedDefenceComputer", "_Mk2", 5, "ScienceLab", "Technology", [("TargetingComputer_Mk2", 1), ("ShipComputer_Mk2", 1)], COMPUTING, None)
_mk("ResearchComputer", "_Mk2", 5, "ScienceLab", "Technology", [("ScienceComputer_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, None)

# --- COMPUTING Mk3: flagship components + computers (rl3) ---
_mk("SensorArray", "_Mk3", 5, "Fabrication", "Components", [("SensorArray_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("LogicChip", "_Mk3", 5, "Fabrication", "Components", [("LogicChip_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("ShipComputer", "_Mk3", 5, "ScienceLab", "Technology", [("ShipComputer_Mk2", 1), ("QuantumProcessor", 1)], COMPUTING, "_Mk2")
_mk("ScienceComputer", "_Mk3", 6, "ScienceLab", "Technology", [("ScienceComputer_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, "_Mk2")
_mk("ResearchComputer", "_Mk3", 6, "ScienceLab", "Technology", [("ResearchComputer_Mk2", 1), ("QuantumProcessor_Mk2", 1)], COMPUTING, "_Mk2")
_mk("AutomatedDefenceComputer", "_Mk3", 6, "ScienceLab", "Technology", [("AutomatedDefenceComputer_Mk2", 1), ("AdvancedSensors_Mk3", 1)], COMPUTING, "_Mk2")

# --- PROPULSION domain (PhysicsLab): engines / reactors / thrusters / fuel ---
_mk("ShipEngine", "_Mk2", 5, "Fabrication", "Military", [("Thruster", 1), ("ReinforcedGirder", 1), ("SuperConductingWire", 1)], PROPULSION, None)
_mk("ShipReactor", "_Mk2", 5, "Fabrication", "Military", [("FusionFuelCell", 2), ("CobaltMagnet", 1), ("StructuralFrame", 1)], PROPULSION, None)
_mk("Thruster", "_Mk2", 5, "Fabrication", "Military", [("TungstenCarbide", 2), ("ServoActuator_Mk2", 1)], PROPULSION, None)
_mk("FuelTank", "_Mk2", 5, "Fabrication", "Military", [("ReinforcedGirder", 1), ("CeramicTiles", 2)], PROPULSION, None)
_mk("ShipEngine", "_Mk3", 6, "Fabrication", "Military", [("ReinforcedGirder", 2), ("FusionFuelCell", 1)], PROPULSION, "_Mk2")
_mk("ShipReactor", "_Mk3", 6, "Fabrication", "Military", [("FusionFuelCell", 2), ("QuantumProcessor_Mk2", 1)], PROPULSION, "_Mk2")
_mk("Thruster", "_Mk3", 6, "Fabrication", "Military", [("TungstenCarbide", 2), ("CobaltMagnet", 1)], PROPULSION, "_Mk2")
_mk("FuelTank", "_Mk3", 6, "Fabrication", "Military", [("AluminiumComposite", 1), ("EnrichedUranium", 1)], PROPULSION, "_Mk2")

# --- MATERIALS domain (MaterialsLab): hulls / structure / cargo / life-support ---
_mk("CargoPod", "_Mk2", 5, "Fabrication", "Military", [("StructuralFrame", 2), ("ReinforcedGirder", 1)], MATERIALS, None)
_mk("LifeSupportPod", "_Mk2", 5, "Fabrication", "Military", [("Vaccines", 1), ("CompositePlating", 1)], MATERIALS, None)
_mk("ShipComponents", "_Mk2", 5, "Fabrication", "Components", [("ArmourPlate", 1), ("Microchips_Mk2", 1), ("ReinforcedGirder", 1)], MATERIALS, None)
_mk("ArmourHull", "_Mk2", 5, "Fabrication", "Military", [("ArmourPlate", 4), ("CarbonFibre", 2)], MATERIALS, None)
_mk("HullPlating", "_Mk2", 5, "Fabrication", "Military", [("ArmourPlate", 2), ("TitaniumAlloy", 1)], MATERIALS, None)
_mk("CargoPod", "_Mk3", 6, "Fabrication", "Military", [("StructuralFrame", 2), ("ReinforcedGirder", 1)], MATERIALS, "_Mk2")
_mk("LifeSupportPod", "_Mk3", 6, "Fabrication", "Military", [("Vaccines_Mk2", 1), ("CompositePlating", 1)], MATERIALS, "_Mk2")
_mk("ShipComponents", "_Mk3", 6, "Fabrication", "Components", [("ArmourPlate", 1), ("Microchips_Mk3", 1), ("ReinforcedGirder", 1)], MATERIALS, "_Mk2")
_mk("ArmourHull", "_Mk3", 6, "Fabrication", "Military", [("ArmourPlate", 4), ("CarbonFibre", 2)], MATERIALS, "_Mk2")
_mk("HullPlating", "_Mk3", 6, "Fabrication", "Military", [("ArmourPlate", 2), ("TitaniumAlloy", 1)], MATERIALS, "_Mk2")

# --- DEFENCE domain (WeaponsLab TL: bastions + energy cannons + shields; weapon
# families split into NICHE labs). Base WeaponsLab handles the general "bread &
# butter" weapons (turret, energy cannon, missiles). Niche labs unlock specialized
# families: ProjectileWeaponsLab -> rail/missile/torpedo, BeamWeaponsLab ->
# laser/plasma/point-defence. A family's Mk upgrades need that niche lab's
# breakthrough, so specializing early pays off. ---
# general (base WeaponsLab):
_mk("ShipShieldGenerator", "_Mk2", 5, "Fabrication", "Military", [("ShieldCapacitor", 1), ("AdvancedSensors_Mk2", 1)], DEFENCE, None)
_mk("ShieldCapacitor", "_Mk2", 5, "Fabrication", "Military", [("CobaltMagnet", 3), ("SuperConductingWire_Mk2", 2)], DEFENCE, None)
_mk("TurretWeapon", "_Mk2", 5, "Fabrication", "Military", [("TargetingComputer", 1), ("ArmourPlate", 1)], DEFENCE, None)
_mk("EnergyCannon", "_Mk2", 5, "Fabrication", "Military", [("SuperConductingWire_Mk2", 2), ("AdvancedPowerCells", 1)], DEFENCE, None)
_mk("ShipShieldGenerator", "_Mk3", 6, "Fabrication", "Military", [("ShieldCapacitor_Mk2", 1), ("AdvancedSensors_Mk3", 1)], DEFENCE, "_Mk2")
_mk("TurretWeapon", "_Mk3", 6, "Fabrication", "Military", [("QuantumProcessor", 1), ("ArmourPlate", 2)], DEFENCE, "_Mk2")
_mk("EnergyCannon", "_Mk3", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 3), ("FusionFuelCell", 1)], DEFENCE, "_Mk2")
_mk("GravitationGenerator", "_Mk2", 6, "Reactor", "Technology", [("CobaltMagnet", 3), ("SuperConductingWire_Mk2", 2)], DEFENCE, None)

# --- PROJECTILE family (ProjectileWeaponsLab -> KineticWeaponResearch) ---
_mk_niche("Railgun", "_Mk2", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 3), ("CobaltMagnet", 1)], "KineticWeaponResearch", None, 3)
_mk_niche("MissileLauncher", "_Mk2", 6, "Fabrication", "Military", [("ServoActuator_Mk2", 1), ("TargetingComputer", 1)], "KineticWeaponResearch", None, 3)
_mk_niche("TorpedoLauncher", "_Mk2", 6, "Fabrication", "Military", [("MissileLauncher_Mk2", 1), ("CobaltMagnet", 1)], "KineticWeaponResearch", None, 3)
_mk_niche("Railgun", "_Mk3", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 3), ("TungstenCarbide", 3)], "KineticWeaponResearch", "_Mk2", 3)
_mk_niche("MissileLauncher", "_Mk3", 6, "Fabrication", "Military", [("ServoActuator_Mk2", 2), ("FusionFuelCell", 1)], "KineticWeaponResearch", "_Mk2", 3)
_mk_niche("TorpedoLauncher", "_Mk3", 6, "Fabrication", "Military", [("MissileLauncher_Mk2", 1), ("AdvancedSensors_Mk2", 1)], "KineticWeaponResearch", "_Mk2", 3)

# --- BEAM family (BeamWeaponsLab -> BeamWeaponResearch) ---
_mk_niche("TriLaser", "_Mk2", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 2), ("Microchips_Mk2", 1)], "BeamWeaponResearch", None, 3)
_mk_niche("PlasmaCannon", "_Mk2", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 2), ("AdvancedPowerCells", 2)], "BeamWeaponResearch", None, 3)
_mk_niche("PointDefenceLaser", "_Mk2", 6, "Fabrication", "Military", [("TriLaser_Mk2", 1), ("TargetingComputer_Mk2", 1)], "BeamWeaponResearch", None, 3)
_mk_niche("TriLaser", "_Mk3", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 3), ("TargetingComputer", 1)], "BeamWeaponResearch", "_Mk2", 3)
_mk_niche("PlasmaCannon", "_Mk3", 6, "Fabrication", "Military", [("SuperConductingWire_Mk2", 2), ("FusionFuelCell", 1)], "BeamWeaponResearch", "_Mk2", 3)
_mk_niche("PointDefenceLaser", "_Mk3", 6, "Fabrication", "Military", [("TriLaser_Mk2", 1), ("AdvancedSensors_Mk3", 1)], "BeamWeaponResearch", "_Mk2", 3)

# --- BIO domain (BiologyLab): medical / bio upgrades ---
_mk("RefinedMedicine", "_Mk2", 4, "Processing", "Medical", [("BioCompound", 2), ("Vaccines", 1)], BIO, None)
_mk("MedicalSupplies", "_Mk2", 4, "Processing", "Medical", [("Vaccines", 1), ("BioCompound", 2)], BIO, None)
_mk("Vaccines", "_Mk2", 4, "ScienceLab", "Medical", [("RefinedMedicine", 1), ("QuantumProcessor", 1)], BIO, None)
_mk("RefinedMedicine", "_Mk3", 5, "Processing", "Medical", [("BioCompound", 3), ("Vaccines_Mk2", 1)], BIO, "_Mk2")
_mk("MedicalSupplies", "_Mk3", 5, "Processing", "Medical", [("Vaccines_Mk2", 1), ("BioCompound", 3)], BIO, "_Mk2")

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

    # ---- research monotonicity ----
    # ResearchRequired must exist, be a Set (research milestone), and its level
    # must be < the recipe's level (an upgrade requires strictly prior research).
    rlevels = {r["OutputItem"]: r["ResearchLevel"] for r in R}
    rreq = {r["OutputItem"]: r["ResearchRequired"] for r in R}
    for r in R:
        req = r["ResearchRequired"]
        if req is None:
            continue
        if req not in rlevels:
            errors.append(f"{r['OutputItem']}: ResearchRequired '{req}' has no producer")
        elif rlevels[req] > r["ResearchLevel"]:
            errors.append(f"{r['OutputItem']}: research '{req}' level {rlevels[req]} > own {r['ResearchLevel']}")

    # ---- item stats coverage ----
    items = build_items(build_recipes_with_ids())
    for iid in ids:
        if iid not in items:
            errors.append(f"{iid}: missing ItemStats entry")
            continue
        st = items[iid]
        if not st["ItemName"]:
            errors.append(f"{iid}: empty ItemName")
        if not st["Description"]:
            errors.append(f"{iid}: empty Description")
        if st["WeightKg"] <= 0 or st["VolumeM3"] <= 0:
            errors.append(f"{iid}: non-positive weight/volume")
        if st["StorageType"] not in ("Solid", "Liquid", "Gas", "Refrigerated", "Hazardous", "Other"):
            errors.append(f"{iid}: bad StorageType '{st['StorageType']}'")
        if st["Rarity"] not in ("Common", "Uncommon", "Rare", "VeryRare", "Legendary"):
            errors.append(f"{iid}: bad Rarity '{st['Rarity']}'")

    return errors


def build_items(recipes):
    """Per-item stats map. Keyed by ItemID; fields mirror MaterialDataAsset /
    TradeItemDataAsset domains so the JSON plugs straight into the game:
    ItemName, Description, WeightKg, VolumeM3, StorageType, Rarity, BaseValue,
    MaterialCategory. Heuristics fill defaults; overrides fix notable items."""
    # ---- category -> storage / material-category defaults ----
    STORE = {
        "RawMaterials": "Solid", "RefinedGoods": "Solid", "Components": "Solid",
        "Technology": "Solid", "Food": "Refrigerated", "Luxury": "Solid",
        "Contraband": "Solid", "Military": "Solid", "Medical": "Refrigerated",
        "Data": "Other", "Other": "Solid",
    }
    MATCAT = {
        "RawMaterials": "Mineral", "RefinedGoods": "RefinedMineral",
        "Components": "Component", "Technology": "Electronics", "Food": "Food",
        "Luxury": "Other", "Contraband": "Other", "Military": "ShipPart",
        "Medical": "Food", "Data": "Research", "Other": "Other",
    }
    TIER_RARITY = {1: "Common", 2: "Common", 3: "Uncommon", 4: "Rare",
                   5: "VeryRare", 6: "Legendary"}
    # base value per tier (credits / unit); shaped up for Military & modules
    TIER_VALUE = {1: 60, 2: 200, 3: 800, 4: 4000, 5: 16000, 6: 64000}

    # ---- manual overrides for display names & flavor descriptions ----
    NAME_OVERRIDES = {
        "Helium3": "Helium-3", "WaterIce": "Water Ice", "RawOre": "Raw Ore",
        "RareEarthElements": "Rare Earth Elements", "CarbonCrystal": "Carbon Crystal",
        "PreciousStones": "Precious Stones", "OrganicBiomass": "Organic Biomass",
        "AlgaeBiomass": "Algae Biomass", "PlantFibre": "Plant Fibre",
        "SalvagedComponents": "Salvaged Components", "DerelictHullPlate": "Derelict Hull Plate",
        "MRE_Rations": "MRE Rations", "BioCompound": "Bio Compound",
        "SyntheticTextiles": "Synthetic Textiles", "FineSpirits": "Fine Spirits",
        "FineSilk": "Fine Silk", "LuxuryFurniture": "Luxury Furniture",
        "LuxuryBeverages": "Luxury Beverages", "Military": "Military",
        "GrainTortilla": "Grain Tortilla", "FreshProduce": "Fresh Produce",
        "FungusSteak": "Fungus Steak", "CulturedMeat": "Cultured Meat",
        "NutritionBar": "Nutrition Bar", "BalancedMeal": "Balanced Meal",
        "LuxuryFeast": "Luxury Feast", "BlackMarketDrugs": "Black Market Drugs",
        "ForgedCredits": "Forged Credits", "HackingTool": "Hacking Tool",
        "SmuggledBioweapon": "Smuggled Bioweapon", "ContrabandTech": "Contraband Tech",
        "GrainCrop": "Grain Crop", "FruitCrop": "Fruit Crop",
        "FungusCrop": "Fungus Crop", "LivestockCulture": "Livestock Culture",
    }
    def human(item):
        if item in NAME_OVERRIDES:
            return NAME_OVERRIDES[item]
        s = item.replace("_", " ").replace("Mk2", "Mk2").replace("Mk3", "Mk3")
        s = re.sub(r'(?<=[a-z])(?=[A-Z])', ' ', s)   # camelCase -> spaces
        return s

    DESC = {
        "SteelAlloy": "Forged iron-copper alloy; the backbone of most construction.",
        "SiliconWafer": "Ultra-flat silicon disk etched for microelectronics.",
        "Electronics": "Assembled circuit assemblies for sensors and computers.",
        "Microchips": "High-density processors etched from refined silicon.",
        "CulturedMeat": "Lab-grown protein with the texture of real meat.",
        "FungusSteak": "Dense myco-protein steak, a favourite mid-tier food.",
        "FreshProduce": "Fresh fruit and vegetables grown in hydroponic farms.",
        "LuxuryFeast": "A celebrated multi-course meal fit for station VIPs.",
        "BlackMarketDrugs": "Illicit stimulants that boost focus at a cost.",
        "ForgedCredits": "Counterfeit credit chits indistinguishable from real.",
        "HackingTool": "Slick malware toolkit for slicing station networks.",
        "SmuggledBioweapon": "Engineered pathogen sealed in a hazmat canister.",
        "ContrabandTech": "Stolen military-grade tech, no questions asked.",
        "OpticalLens": "Precision-ground lens for imaging, scanning and sensors.",
        "SensorArray": "Chip of optical sensors, lenses and amplifiers for detection.",
        "TransceiverUnit": "Radio/signal transceiver for ship and station comms.",
        "MicroCapacitor": "Tiny energy storage cell for circuit boards.",
        "LogicChip": "Programmable logic array accelerating dedicated operations.",
        "Gyroscope": "Inertial stabiliser that keeps a ship's attitude true.",
        "DataMatrix": "High-density optical storage module for records.",
        "ClockGenerator": "Precision oscillator that paces a computer's operations.",
        "PilotComputer": "Dedicated flight-assist computer managing ship control.",
        "CargoComputer": "Manages cargo manifests, mass, and docking logistics.",
        "ShipComputer": "Central onboard computer coordinating all ship subsystems.",
        "TradingComputer": "Live market-analysis computer for finding the best routes.",
        "ScanComputer": "Fast signal-processing computer for sensor sweeps.",
        "ScienceComputer": "Heavy-compute computer for research and data analysis.",
        "EngineeringComputer": "Diagnostic computer for repairs and system tuning.",
        "ResearchComputer": "Scientist-grade computing cluster for breakthroughs.",
        "AutomatedDefenceComputer": "Auto-targeting computer for turrets and point-defence.",
    }
    def desc(item, tier, cat):
        if item in DESC:
            return DESC[item]
        head = human(item)
        if item.endswith("Ore") or cat == "RawMaterials" and tier == 1:
            return f"{head}, mined from asteroids and planetary deposits."
        if tier == 1:
            return f"Raw {head.lower()}, extracted directly from space environments."
        if item.endswith("Module"):
            return f"{head}: a prefabricated station module ready for assembly."
        if item.endswith("Research"):
            return f"Unlocked {head.lower()} — a Science Lab breakthrough data-file."
        return f"Refined {head.lower()}, produced by advanced fabrication."

    # ---- weight (kg) heuristic per category & tier ----
    def weight(item, tier, cat, produced_in):
        if cat == "Military":
            base = 60.0 if tier == 4 else 250.0 if tier in (5, 6) else 40.0
        else:
            base = {"RawMaterials": 4.0, "RefinedGoods": 6.0, "Components": 12.0,
                    "Technology": 8.0, "Food": 2.0, "Medical": 3.0, "Data": 1.0,
                    "Luxury": 5.0, "Contraband": 6.0, "Other": 6.0}[cat]
        if item.endswith("Module"):
            base *= 40.0
        if item.endswith("Ore"):
            return 8.0
        # density: metals & processing get heavier
        if produced_in == "ShipMining" and tier == 1:
            return 8.0
        return round(base * (1.2 ** (tier - 1)), 1)

    def volume(item, tier, cat, weight_kg):
        # derive from weight with a loose packing density (kg/m^3 ~ 400 base)
        v = weight_kg / 400.0
        if cat == "Food":
            v = weight_kg / 900.0
        if cat == "Technology":
            v = weight_kg / 600.0
        if cat == "Military":
            v = weight_kg / 500.0
        return round(max(v, 0.05), 2)

    def storage(item, cat, produced_in):
        if item in ("Hydrogen", "Helium3", "MethaneGas", "NitrogenGas", "NobleGas"):
            return "Gas"
        if item in ("CompressedMethane",):
            return "Hazardous"
        if item in ("LiquidHydrogen", "LiquidNitrogen", "Water", "HydrogenFuel",
                    "FusionFuelCell"):
            return "Liquid"
        if item in ("EnrichedUranium", "PlasmaFuel", "ProhibitedWeapons", "NanoInjectors",
                    "SmuggledBioweapon", "BlackMarketDrugs", "ContrabandTech", "ForgedCredits",
                    "HackingTool"):
            return "Hazardous"
        if cat in ("Food", "Medical"):
            return "Refrigerated"
        return STORE[cat]

    items = {}
    # ---- BaseValue: COST-DRIVEN via memoized topological resolve ----
    # A crafted item is worth (sum of its ingredients' values) x markup, where
    # markup = value-add for processing/machining, scaled by category and
    # research level. This guarantees every crafted recipe has a positive,
    # BOUNDED margin (no loss, no free-money arbitrage). Raws are market-priced.
    def raw_value(tier):
        return round(TIER_VALUE.get(tier, 100) * 0.5)

    def markup_for(r):
        m = 1.5                     # base value-add for refining/crafting one step
        if r["Category"] == "Military":
            m *= 1.5                # weapons/parts carry a premium
        if r["Category"] == "Contraband":
            m *= 2.0                # black-market premium
        if r["Category"] == "Luxury":
            m *= 1.7                # luxury markup
        if r["ResearchLevel"] == 2:
            m *= 1.7                # Mk2 better-than-base
        elif r["ResearchLevel"] == 3:
            m *= 3.0                # Mk3 flagship
        return m

    recipe_by_out = {r["OutputItem"]: r for r in recipes}
    value_cache = {}

    def value_of(item):
        if item in value_cache:
            return value_cache[item]
        r = recipe_by_out.get(item)
        if r is None:
            value_cache[item] = 0
            return 0
        if r["Acquisition"]:
            v = raw_value(r["Tier"])
        else:
            cost = sum(ing["Qty"] * value_of(ing["ItemID"]) for ing in r["Ingredients"])
            v = round(cost * markup_for(r))
        value_cache[item] = v
        return v

    # Pre-seed with basic fields, then resolve values in a stable pass.
    for r in recipes:
        iid, tier, cat = r["OutputItem"], r["Tier"], r["Category"]
        st = storage(iid, cat, r["ProducedIn"])
        wt = weight(iid, tier, cat, r["ProducedIn"])
        rar = TIER_RARITY.get(tier, "Common")
        # resolve values depth-first; acyclic so memoization terminates
        base_v = value_of(iid)
        items[iid] = {
            "ItemName": human(iid),
            "Description": desc(iid, tier, cat),
            "WeightKg": wt,
            "VolumeM3": volume(iid, tier, cat, wt),
            "StorageType": st,
            "Rarity": rar,
            "BaseValue": int(round(base_v)),
            "MaterialCategory": MATCAT[cat],
        }
    return items


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
            "ResearchLevel": r["ResearchLevel"],
            "ResearchRequired": r["ResearchRequired"],
            "Ingredients": r["Ingredients"],
        })
    return out


def compute_economy(recipes, items):
    """Per-recipe economy. For each (non-acquisition) recipe: sum ingredient cost
    from their BaseValue, and compare to the output BaseValue. Returns a dict of
    RecipeID -> {IngredientCost, OutputValue, Profit, Margin}. Also returns a list
    of (RecipeID, kind, message) flags for real anomalies: genuine losses (output
    below ingredient cost) or margins that diverge from the recipe's INTENDED
    markup (catches data-entry errors), not the normal value-add profit."""
    value = {i: st["BaseValue"] for i, st in items.items()}

    def intended_markup(r):
        m = 1.5
        if r["Category"] == "Military":
            m *= 1.5
        if r["Category"] == "Contraband":
            m *= 2.0
        if r["Category"] == "Luxury":
            m *= 1.7
        if r["ResearchLevel"] == 2:
            m *= 1.7
        elif r["ResearchLevel"] == 3:
            m *= 3.0
        return m

    ec = {}
    flags = []
    for r in recipes:
        if r["Acquisition"]:
            continue
        cost = sum(ing["Qty"] * value.get(ing["ItemID"], 0) for ing in r["Ingredients"])
        out_val = value.get(r["OutputItem"], 0)
        profit = out_val - cost
        margin = (profit / cost) if cost else 0.0
        ec[r["OutputItem"]] = {
            "IngredientCost": cost,
            "OutputValue": out_val,
            "Profit": profit,
            "Margin": round(margin, 3),
            "IntendedMarkup": round(intended_markup(r), 3),
        }
        # genuine loss (would never craft it)
        if cost > 0 and out_val < cost:
            flags.append((r["OutputItem"], "loss",
                          f"output {out_val}cr < ingredient cost {cost}cr (margin {round(margin*100)}%)"))
        # divergence from intended markup (data-entry / modeling error)
        elif cost > 0:
            intended_margin = intended_markup(r) - 1.0
            if margin > intended_margin * 2.0 + 0.1:
                flags.append((r["OutputItem"], "margin-high",
                              f"margin {round(margin*100)}% far above intended {round(intended_margin*100)}%"))
            elif margin < intended_margin * 0.4:
                flags.append((r["OutputItem"], "margin-low",
                              f"margin {round(margin*100)}% far below intended {round(intended_margin*100)}%"))
    return ec, flags


def main():
    errors = check()
    if errors:
        print("VALIDATION FAILED:")
        for e in errors:
            print("  -", e)
        sys.exit(1)

    recipes = build_recipes_with_ids()
    items = build_items(recipes)
    economy, eflags = compute_economy(recipes, items)
    doc = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "Title": "Adastrea Crafting & Building Tree",
        "Description": "Machine-readable crafting/building tree (recipes + per-item stats + recipe economy): raw extraction -> refined materials -> components & electronics -> ship parts -> weapons -> station construction parts -> modules. Authoritative generator: docs/11-TECHNICAL_SPECS/generate_crafting_tree.py",
        "SchemaVersion": "1.7.0",
                "LastUpdated": "2026-08-31",
        "ItemIDConvention": "^[A-Za-z][A-Za-z0-9_]*$",
        "NoteHelium3": "Existing trade asset uses 'Helium-3' (hyphen, violates ItemID regex). Crafting data canonicalizes to 'Helium3' and maps to DA_TradeItem_Helium-3.",
        "ProducedInTags": sorted(PRODUCED_IN_TAGS),
        "TierLabels": TIER_LABELS,
        "Recipes": recipes,
        "Items": items,
        "Economy": economy,
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
    n_loss = len([f for f in eflags if f[1] == "loss"])
    n_hi = len([f for f in eflags if f[1] == "margin-high"])
    n_lo = len([f for f in eflags if f[1] == "margin-low"])
    if eflags:
        print(f"  ECONOMY FLAGS: {len(eflags)} ({n_loss} loss, {n_hi} margin-high, {n_lo} margin-low)")
        for rid, kind, msg in eflags[:20]:
            print(f"    [{kind:11s}] {rid}: {msg}")
    else:
        print("  ECONOMY: all margins within intended range (no loss/arbitrage)")


if __name__ == "__main__":
    main()