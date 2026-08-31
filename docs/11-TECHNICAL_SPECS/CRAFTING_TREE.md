# Crafting & Building Tree

**Trade Simulator MVP → Post-MVP | Last Updated: 2026-08-31**

This spec defines the full **crafting / building tree** for Adastrea: the chain from
raw materials (and how they are obtained) → refined materials → components &
electronics → ship parts → weapons → station construction parts → modules. It is
the design companion to the machine-readable
`Content/Data/CraftingTree.json`.

**Grounded in existing systems:**
- `EMaterialCategory` (`Source/Adastrea/Public/Materials/MaterialDataAsset.h`):
  Mineral, RefinedMineral, Component, Electronics, ConstructionPart, ShipPart,
  Food, Research, Organic, Synthetic, Other.
- `ETradeItemCategory` (`Source/Adastrea/Public/Trading/TradeItemDataAsset.h`):
  RawMaterials, RefinedGoods, Components, Technology, Food, Luxury, Contraband,
  Military, Medical, Data, Other.
- Station modules (`Source/Adastrea/Public/Stations/*.h`): Barracks, CargoBay,
  Corridor, DockingBay, DockingPort, Fabrication, FuelDepot, Habitation,
  Marketplace, Processing, Reactor, ScienceLab, ShieldGenerator, SolarArray,
  Turret; groups Docking, Power, Storage, Processing, Defence, Habitation,
  Public, Connection.
- Existing trade ItemIDs (see `Content/DataAssets/Trading/Items/DA_TradeItem_*.uasset`).

---

## 1. Design Principles

1. **Acyclic, tiered, monotonic.** A recipe's output is always at a tier ≥ every
   ingredient's tier; no recipe produces itself, directly or transitively.
2. **Four production depths, three build depths.** Tiers 1–2 are *raw/refined*
   (extraction + processing). Tiers 3 establish *intermediate goods*. Tiers 4–5
   are *functional assemblies*. Tier 6 is *station modules/construction* — the
   tree stops here (no whole-station blueprints).
3. **Every ingredient is producible.** Nothing in the tree is a dead leaf that is
   also a required input without a producer.
4. **Module tags match the real station modules.** `Processing` (the refinery
   module) refines; `Fabrication` machine-assembles; `Reactor`/`SolarArray`
   generate power-tier goods; `ScienceLab` produces advanced tech/research;
   `FuelDepot` produces fuel. Extraction (Tier 1) happens aboard ships / at
   mining outposts, not inside a module.
5. **Category inherits the existing trade-enum** so crafted items drop straight
   into the economy/market system.

---

## 2. Tier Map

| Tier | Name                | Made By / Where                          | Representative outputs |
|------|---------------------|------------------------------------------|------------------------|
| **1** | Raw materials       | Extraction (ships / mining stations)     | Ores, ice, gas, biomass, salvage |
| **2** | Refined materials   | `Processing`, `SolarArray`, `FuelDepot`  | Alloys, wafers, wiring, fuel, food staples, power cells |
| **3** | Components & electronics | `Fabrication`, `Reactor`, `ScienceLab`, `Processing` | Electronics, chips, frames, plating, computers, medical, luxury |
| **4** | Functional assemblies | `Fabrication`, `ScienceLab`            | Ship parts, weapons, shields, AI cores |
| **5** | Station construction parts | `Fabrication`, `Reactor`, `ScienceLab` | Construction materials, hull frame, power core, life support, module shell |
| **6** | Station modules & assembly | `Fabrication`                        | Corridor, cargo bay, docking, reactor, turret, shield module, etc. |

> Note: the tree deliberately **stops at station modules/construction parts** —
> there are no whole-station "blueprint" recipes. Station *modules* map to the
> in-engine `ASpaceStationModule` classes; the player assembles a station from those
> modules in the game, but a full-station assembly recipe is out of scope.

### Research / progression axis

Independent of the construction tiers above, the tree has a **research system** built
around **specialized labs**: each type of science/research lab unlocks a **different
breakthrough domain**. A domain's breakthroughs are real, consumable items produced
inside that lab, and they gate the upgraded (Mk2/Mk3) versions of that domain's
equipment.

| Lab (`ASpaceStationModule`) | Domain breakthrough (rl2 / rl3) | Unlocks Mk2/Mk3 of |
|-----------------------------|-------------------------------|--------------------|
| `PhysicsLab` (PhysicsLabModule) | `PropulsionResearch` / `AdvancedPropulsionResearch` | Engines, reactors, thrusters, fuel tanks |
| `MaterialsLab` (MaterialsLabModule) | `MaterialsResearch` / `NanoMaterialsResearch` | Hulls, plating, cargo pods, life-support, ship components |
| `ElectronicsLab` (ElectronicsLabModule) | `ComputingResearch` / `QuantumComputingResearch` | Electronics, chips, boards, computers, sensors, processors |
| `WeaponsLab` (WeaponsLabModule) | `DefenceResearch` / `AdvancedDefenceResearch` | Turrets, cannons, missiles, shields, grav-generation |
| `BiologyLab` (BiologyLabModule) | `BioResearch` / `AdvancedBioResearch` | Refined medicine, supplies, vaccines |
| `ScienceLab` (ScienceLabModule) | `ResearchData` (generic base) | — (base tech line) |

Each lab module is itself a craftable **Tier-6 station module**, built in
`Fabrication`. A research **breakthrough** is produced **inside** its own lab
(e.g. PropulsionResearch from a `PhysicsLab`), and is consumed (required) by the
Mk2/Mk3 recipes of that domain. Research **levels**: rl2 breakthroughs unlock Mk2
versions; rl3 breakthroughs unlock Mk3 versions. Every recipe carries a
`ResearchLevel` (1–4) and, when upgraded, a `ResearchRequired` item at a level ≤
its own.

**Cross-lab coupling is intentional**: every rl3 breakthrough also requires
high-tier computing (`QuantumProcessor_Mk2`, unlocked by the ElectronicsLab), so a
player chasing late-game tech must stand up more than one lab type. The resulting
loop: gather mats → build base gear → construct a specialized lab → produce its
breakthrough → craft Mk2/Mk3 gear in that domain (then branch to other labs for the
rest).

---

## 3. Acquisition — Where Raw Materials Come From

Raw extraction is a **ship/outpost activity**, not a station module. It feeds the
`Processing` chain.

| Resource (ItemID)        | Acquisition method              | Notes |
|--------------------------|---------------------------------|-------|
| `IronOre`                | **Mining** (mining-laser / drill) | Abundant; primary iron source. Matches `DA_TradeItem_IronOre`. |
| `CopperOre`              | **Mining**                       | Conductor metal. `DA_TradeItem_CopperOre`. |
| `TitaniumOre`            | **Mining** (richer belts)        | Stronger; `DA_TradeItem_TitaniumOre`. |
| `Platinum`               | **Mining** (rare PGM deposits)   | High value catalyst. `DA_TradeItem_Platinum`. |
| `RareEarthElements`      | **Mining** (lanthanide-rich)     | Electronics feedstock. `DA_TradeItem_RareEarthElements`. |
| `Silicon` *(new)*        | **Mining** (silicate asteroids)  | Base for wafers/solar cells. |
| `RawOre`                 | **Mining** (generic)             | Bulk low-grade fallback. `DA_TradeItem_RawOre` template. |
| `WaterIce`               | **Ice collection** (ice harvester) | `DA_TradeItem_WaterIce`. |
| `Helium3`                | **Gas harvesting** (gas-giant skim) | NOTE: canonical `Helium3`; maps to `DA_TradeItem_Helium-3` (existing asset uses a hyphen that violates the ItemID regex). |
| `OrganicBiomass` *(new)* | **Farming** (hydroponic/algae at agricultural stations) | Organic feedstock for food & composites. |
| `SalvagedComponents` *(new)* | **Salvage** (derelicts / debris fields) | Recovered components used *directly* as components. |

> The ship classes that perform these are the asset-side **Miner** (drill +
> mining-laser), plus the **Mining Station** hull; refer to
> `SM_Ship_Miner_01_hardpoints.json` / `SM_Station_Mining_01_hardpoints.json`
> (HP_Underslung drill + HP_Forward mining-laser).

---

## 4. The Tree (ASCII)

```
TIER 1  RAW EXTRACTION
  ores ─ IronOre ─ CopperOre ─ TitaniumOre ─ Platinum ─ RareEarth ─ Silicon ─ RawOre
  ice  ── WaterIce
  gas  ── Helium3
  bio  ── OrganicBiomass
  salvage ─ SalVagedComponents
        │
TIER 2  REFINED (Processing / SolarArray / FuelDepot)
  Iron+Copper ──────────────► SteelAlloy
  Copper ────────────────────► CopperWiring
  Silicon ───────────────────► SiliconWafer
  Titanium+RareEarth ────────► TitaniumAlloy
  Platinum+RareEarth ────────► PlatinumCatalyst
  WaterIce ──────────────────► Water
  Helium3+Water ─────────────► BasicFuel
  SiliconWafer ──────────────► PowerCells (SolarArray)
  Biomass+Water ─────────────► SyntheticProtein ─────────────────► FoodRations
        │
TIER 3  COMPONENTS & ELECTRONICS (Fabrication / Reactor / ScienceLab / Processing)
  Wiring+Wafer ──────────────► Electronics
  Wafer+Catalyst ────────────► Microchips
  Steel ─────────────────────► StructuralFrame
  Titanium+Biomass ──────────► CompositePlating
  Steel+Wiring ──────────────► ServoActuator
  Electronics+Chips ─────────► BasicComputer ────────────────────► ControlUnit
  PowerCells+Electronics ────► PowerDistributionUnit
  PowerCells+Catalyst ───────► AdvancedPowerCells (Reactor)
  Helium+Catalyst ───────────► PlasmaFuel (Reactor)
  Chips+Catalyst+RareEarth ──► QuantumProcessor
  Electronics+Wafer+Cells ───► AdvancedSensors
  Protein+Electronics+Water ─► MedicalSupplies
  Water+Biomass ─────────────► LuxuryBeverages
        │
TIER 4  FUNCTIONAL ASSEMBLIES (Fabrication / ScienceLab)
  Frame+Plating+Electronics ─► ShipComponents
  Plating+Titanium ──────────► HullPlating
  Steel+ShipComp+Wiring ─────► ShipEngine
  Steel+PowerDist ───────────► ShipThruster
  AdvCells+ShipComp+Frame ───► ShipReactor
  AdvSensors+AdvCells+Plating► ShipShieldGenerator
  Steel+ShipComp+Control ────► TurretWeapon ──(+AdvCells+AdvSensors)─► EnergyCannon
  Cannon+QuantumProc ────────► ProhibitedWeapons (ScienceLab)
  QuantumProc+Chips ─────────► AICores (ScienceLab)
  AdvSensors+Computer ───────► ResearchData (ScienceLab)
  Platinum+RareEarth ────────► Jewelry
        │
TIER 5  STATION CONSTRUCTION PARTS (Fabrication / Reactor / ScienceLab)
  Frame+Steel+Plating ───────► ConstructionMaterials
  Constr+titanium ───────────► StationHullFrame
  AdvCells+Control+Quantum ──► PowerCore (Reactor)
  Electronics+Frame+Computer ─► LifeSupportUnit
  HullFrame+Plating ─────────► ModuleShell
  AICores+Quantum+Research ──► AdvancedAICore (ScienceLab)
        │
TIER 6  STATION MODULES (Fabrication)
  Shell+Frame        ► CorridorModule
  Shell+Constr       ► CargoBayModule
  Shell+Control+Frame► DockingBayModule
  Shell+Control      ► DockingPortModule
  Shell+LifeSupport  ► HabitationModule
  Shell+LifeSupport+Frame ► BarracksModule
  Shell+Computers+LifeSupport ► MarketplaceModule
  Shell+Quantum+Sensors ► ScienceLabModule
  PowerCore+Shell    ► ReactorModule
  Cells+Wafer+Shell  ► SolarArrayModule
  Shell+Steel+Control► ProcessingModule
  Shell+Control+Constr ► FabricationModule
  Shell+Steel+Cells  ► FuelDepotModule
  TurretWeapon+Shell ► TurretModule
  ShieldGen+Shell+AdvCells ► ShieldGeneratorModule
```

---

## 5. Master Recipe Table

Reference tables from the canonical JSON. **Ingredient quantities are unit counts.**

### Tier 2 — Refined materials / power / food

| Output            | Produced In   | Category      | Ingredients (qty) |
|-------------------|---------------|---------------|-------------------|
| `SteelAlloy`      | Processing    | RefinedGoods  | IronOre×2, CopperOre×1 |
| `Water`           | Processing    | Food          | WaterIce×1 → ×2 |
| `BasicFuel`       | FuelDepot     | RefinedGoods  | Helium3×2, Water×1 → ×2 |
| `SiliconWafer`    | Processing    | RefinedGoods  | Silicon×3 → ×3 |
| `CopperWiring`    | Processing    | RefinedGoods  | CopperOre×2 → ×2 |
| `TitaniumAlloy`   | Processing    | RefinedGoods  | TitaniumOre×2, RareEarthElements×1 |
| `PlatinumCatalyst`| Processing    | RefinedGoods  | Platinum×1, RareEarthElements×1 |
| `PowerCells`      | SolarArray    | RefinedGoods  | SiliconWafer×2 → ×2 |
| `AdvancedPowerCells`| Reactor    | Technology    | PowerCells×2, PlatinumCatalyst×1 |
| `PlasmaFuel`      | Reactor       | RefinedGoods  | Helium3×3, PlatinumCatalyst×1 |
| `SyntheticProtein`| Processing   | Food          | OrganicBiomass×2, Water×1 → ×2 |
| `FoodRations`     | Processing    | Food          | SyntheticProtein×1, Water×1 |

### Tier 3 — Components, electronics, medical, luxury

| Output            | Produced In   | Category      | Ingredients (qty) |
|-------------------|---------------|---------------|-------------------|
| `Electronics`     | Fabrication   | Components    | CopperWiring×2, SiliconWafer×2 |
| `Microchips`      | Fabrication   | Components    | SiliconWafer×3, PlatinumCatalyst×1 |
| `StructuralFrame` | Fabrication   | Components    | SteelAlloy×3 |
| `CompositePlating`| Fabrication   | Components    | TitaniumAlloy×2, OrganicBiomass×1 |
| `ServoActuator`   | Fabrication   | Components    | SteelAlloy×1, CopperWiring×2 |
| `BasicComputer`   | Fabrication   | Technology    | Electronics×2, Microchips×1 |
| `ControlUnit`     | Fabrication   | Components    | BasicComputer×1, CopperWiring×2 |
| `PowerDistributionUnit` | Fabrication | Components | PowerCells×2, Electronics×1 |
| `QuantumProcessor`| ScienceLab    | Technology    | Microchips×4, PlatinumCatalyst×2, RareEarthElements×1 |
| `AdvancedSensors` | ScienceLab    | Technology    | Electronics×2, SiliconWafer×1, PowerCells×1 |
| `MedicalSupplies` | Processing    | Medical       | SyntheticProtein×1, Electronics×1, Water×1 |
| `LuxuryBeverages` | Processing    | Luxury        | Water×2, OrganicBiomass×1 |

### Tier 4 — Ship parts, weapons, shields, research

| Output            | Produced In   | Category      | Ingredients (qty) |
|-------------------|---------------|---------------|-------------------|
| `ShipComponents`  | Fabrication   | Components    | StructuralFrame×2, CompositePlating×1, Electronics×1 |
| `HullPlating`     | Fabrication   | Military      | CompositePlating×3, TitaniumAlloy×1 |
| `ShipEngine`      | Fabrication   | Military      | SteelAlloy×2, ShipComponents×1, CopperWiring×1 |
| `ShipThruster`    | Fabrication   | Military      | SteelAlloy×2, PowerDistributionUnit×1 |
| `ShipReactor`     | Fabrication   | Military      | AdvancedPowerCells×2, ShipComponents×2, StructuralFrame×1 |
| `ShipShieldGenerator` | Fabrication | Military    | AdvancedSensors×1, AdvancedPowerCells×2, CompositePlating×2 |
| `TurretWeapon`    | Fabrication   | Military      | SteelAlloy×3, ShipComponents×1, ControlUnit×1 |
| `EnergyCannon`    | Fabrication   | Military      | TurretWeapon×1, AdvancedPowerCells×2, AdvancedSensors×1 |
| `ProhibitedWeapons`| ScienceLab   | Contraband    | EnergyCannon×1, QuantumProcessor×1 |
| `AICores`         | ScienceLab    | Technology    | QuantumProcessor×1, Microchips×2 |
| `AdvancedAICore`  | ScienceLab    | Technology    | AICores×1, QuantumProcessor×2, ResearchData×1 |
| `ResearchData`    | ScienceLab    | Data          | AdvancedSensors×1, BasicComputer×1 |
| `Jewelry`         | Fabrication   | Luxury        | Platinum×2, RareEarthElements×1 |

### Tier 5 — Station construction parts

| Output            | Produced In   | Category      | Ingredients (qty) |
|-------------------|---------------|---------------|-------------------|
| `ConstructionMaterials` | Fabrication | Components | StructuralFrame×2, SteelAlloy×2, CompositePlating×1 |
| `StationHullFrame`      | Fabrication | Components | ConstructionMaterials×2, TitaniumAlloy×2 |
| `PowerCore`             | Reactor      | Technology  | AdvancedPowerCells×3, ControlUnit×1, QuantumProcessor×1 |
| `LifeSupportUnit`       | Fabrication | Components  | Electronics×2, StructuralFrame×1, BasicComputer×1 |
| `ModuleShell`           | Fabrication | Components  | StationHullFrame×1, CompositePlating×1 |

### Tier 6 — Station modules (assembly)

| Output                | Produced In   | Category | Ingredients (qty) |
|-----------------------|---------------|----------|-------------------|
| `CorridorModule`      | Fabrication   | Other    | ModuleShell×2, StructuralFrame×1 |
| `CargoBayModule`      | Fabrication   | Other    | ModuleShell×3, ConstructionMaterials×1 |
| `DockingBayModule`    | Fabrication   | Other    | ModuleShell×3, ControlUnit×1, StructuralFrame×2 |
| `DockingPortModule`   | Fabrication   | Other    | ModuleShell×2, ControlUnit×1 |
| `HabitationModule`    | Fabrication   | Other    | ModuleShell×3, LifeSupportUnit×2 |
| `BarracksModule`      | Fabrication   | Other    | ModuleShell×2, LifeSupportUnit×1, StructuralFrame×1 |
| `MarketplaceModule`   | Fabrication   | Other    | ModuleShell×2, BasicComputer×3, LifeSupportUnit×1 |
| `ScienceLabModule`    | Fabrication   | Other    | ModuleShell×2, QuantumProcessor×1, AdvancedSensors×1 |
| `ReactorModule`       | Fabrication   | Other    | PowerCore×2, ModuleShell×2 |
| `SolarArrayModule`    | Fabrication   | Other    | PowerCells×4, SiliconWafer×3, ModuleShell×1 |
| `ProcessingModule`    | Fabrication   | Other    | ModuleShell×2, SteelAlloy×2, ControlUnit×1 |
| `FabricationModule`   | Fabrication   | Other    | ModuleShell×2, ControlUnit×2, ConstructionMaterials×1 |
| `FuelDepotModule`     | Fabrication   | Other    | ModuleShell×2, SteelAlloy×2, PowerCells×1 |
| `TurretModule`        | Fabrication   | Other    | TurretWeapon×2, ModuleShell×2 |
| `ShieldGeneratorModule`| Fabrication  | Other    | ShipShieldGenerator×1, ModuleShell×2, AdvancedPowerCells×2 |

---

## 6. Item ID Glossary & Notes

- **Existing trade ItemIDs reused** (map to existing Data Assets): `IronOre`,
  `CopperOre`, `TitaniumOre`, `Platinum`, `RareEarthElements`, `RawOre`,
  `WaterIce`, `Water`, `BasicFuel`, `SteelAlloy`, `Electronics`, `PowerCells`,
  `ShipComponents`, `ConstructionMaterials`, `QuantumProcessor`,
  `AdvancedSensors`, `AICores`, `BasicComputer`, `SyntheticProtein`,
  `FoodRations`, `MedicalSupplies`, `LuxuryBeverages`, `Jewelry`,
  `AdvancedAICore`, `ProhibitedWeapons`.
- **New ItemIDs proposed** (all match `^[A-Za-z][A-Za-z0-9_]*$`):
  `Silicon`, `Helium3`, `OrganicBiomass`, `SalvagedComponents`, `CopperWiring`,
  `SiliconWafer`, `TitaniumAlloy`, `PlatinumCatalyst`, `AdvancedPowerCells`,
  `PlasmaFuel`, `Microchips`, `StructuralFrame`, `CompositePlating`,
  `ServoActuator`, `ControlUnit`, `PowerDistributionUnit`, `HullPlating`,
  `ShipEngine`, `ShipThruster`, `ShipReactor`, `ShipShieldGenerator`,
  `TurretWeapon`, `EnergyCannon`, `ResearchData`, `StationHullFrame`,
  `PowerCore`, `LifeSupportUnit`, `ModuleShell`, and the 15 module assembly
  outputs.
- ⚠️ **`Helium-3` (hyphen) → canonical `Helium3`.** The existing
  `DA_TradeItem_Helium-3` asset name violates the ItemID regex. Crafting data
  uses `Helium3`; if you rename the asset, rename to `DA_TradeItem_Helium3` and
  set its `ItemID` to `Helium3`.

---

## 6b. Item Stats

Every item in the tree also carries a **per-unit stats entry** in the ``Items``
map of `CraftingTree.json` (keyed by `ItemID`, one per recipe output). The fields
mirror the existing `UMaterialDataAsset` / `UTradeItemDataAsset` domains so the
data plugs straight into the game:

| Field             | Type      | Meaning |
|-------------------|-----------|---------|
| `ItemName`        | string    | Human-readable display name (e.g. `Ship Engine Mk3`) |
| `Description`     | string    | One-line flavor/functional description |
| `WeightKg`        | float     | Mass per 1 unit |
| `VolumeM3`        | float     | Stowage volume per 1 unit |
| `StorageType`     | enum      | `Solid` / `Liquid` / `Gas` / `Refrigerated` / `Hazardous` / `Other` (`EStorageType`) |
| `Rarity`          | enum      | `Common`…`Legendary` (`EMaterialRarity`) |
| `BaseValue`       | int       | Base price in credits per 1 unit (tier-, category- and research-scaled) |
| `MaterialCategory`| enum      | `Mineral`…`Other` (`EMaterialCategory`) |

Assignments are heuristic + override driven in the generator: weight/volume scale
with tier & category (ores ~8 kg, ship parts 60–250 kg, station modules heavier
still), gases/liquids/hazardous materials get the correct `StorageType`, and
rarity/base value escalate with tier and Mk level so late-game research items are
genuinely more valuable. The interactive diagram's node tooltip shows these stats
(name, star-rating by rarity, kg, m³, storage, credits) on hover.

---

## 7. Verification

The JSON (`Content/Data/CraftingTree.json`, **227 recipes across 6 tiers** — including the Mk2/Mk3 progression versions and the 5 specialized research-lab domains + breakthroughs; no whole-station assembly recipes) is validated by the
`check_crafting_tree()` logic in `generate_crafting_tree.py` plus `tests/test_crafting_tree.py`:
- ✅ All recipes have unique `RecipeID` and unique `OutputItem`.
- ✅ Every `ItemID` matches `^[A-Za-z][A-Za-z0-9_]*$`.
- ✅ No circular recipes (graph acyclic).
- ✅ Every ingredient is produced by some recipe (nothing is an unproducible leaf
  that is required), including Mk versions and research items.
- ✅ Tier monotonicity (ingredient tier ≤ output tier; equal-tier combination
  recipes are intentional, e.g. Electronics+Chips → BasicComputer).
- ✅ Research monotonicity (`ResearchRequired` names a produced item at a level ≤
  the recipe's own `ResearchLevel`).
- ✅ Only the canonical `ProducedIn` module tags are used.
- ✅ Every output item has a valid `Items` stats entry (name, description, positive
  weight & volume, and valid `StorageType`/`Rarity`) — 12 unit tests pass.

---

## 8. Next Steps / Backend

The JSON is the single source of truth for the crafting/building tree. A C++
`UCraftingTreeDataAsset` + `FCraftingRecipe` struct (mirroring
`UTradeItemDataAsset`/`FTradeItemData`) is **intentionally deferred**: that change
needs a full UE build to verify it compiles cleanly, which is not feasible from
this environment. The struct contract (RecipeID, OutputItem, OutputQty, Tier,
ProducedIn as FName, Ingredients as TArray of {FName ItemID, int32 Qty}, Category
mapped to `ETradeItemCategory`) is documented so the `main` gameplay agent can
drop it in and build without redesigning the data. See
`Content/Data/CraftingTree.json` for the exact field names to mirror.

---

*Part of the Adastrea [Technical Specs](..). Back to the [Economy System](ECONOMY_SYSTEM.md).*