# Facility & Industry Plan — What Each Recipe Needs to Run

**Trade Simulator MVP → Post-MVP | Last Updated: 2026-08-31**

Every recipe in the crafting/build tree is produced *somewhere*. This doc turns the
`ProducedIn` tags of `Content/Data/CraftingTree.json` (295 recipes) into a concrete
**industry / facility plan**, and — critically — into the **list of space-station
modules** we must build so the player can actually run that industry at a station.

The tree has **4 classes of producer**: (1) *star-bound* activities that happen on
ships / rigs (mining, gas, ice, salvage, farming), (2) *station modules* that already
exist in-engine, (3) *station modules we must ADD* to cover the rest of the tree, and
(4) *research labs* that gate tech. The plan below resolves all of them.

---

## 1. Facility inventory (mapped from the tree)

| `ProducedIn` tag | Facility type | Where it lives | Station module needed? |
|------------------|---------------|----------------|------------------------|
| `ShipMining` | Mining rig / asteroid miner | **Star-bound** (ships & mining stations) | ⚠️ *add* `MiningModule` / refit a Mining Station |
| `GasHarvesting` | Gas harvester / gas-cloud rig | **Star-bound** | ⚠️ *add* `GasHarvesterModule` |
| `IceCollection` | Ice harvester / icy-asteroid collector | **Star-bound** | ⚠️ *add* `IceHarvesterModule` |
| `OrganicFarming` | Hydroponic farm | Star-bound or station | ⚠️ *add* `FarmingModule` (hydroponics) |
| `Salvage` | Salvage yard / derelict processor | Star-bound or station | ⚠️ *add* `SalvageModule` |
| `Processing` | Refinery / material processor | **Station module — EXISTS** | `ProcessingModule` ✅ |
| `Fabrication` | Precision fabricator / assembly plant | **Station module — EXISTS** | `FabricationModule` ✅ |
| `ScienceLab` | Research & development lab | **Station module — EXISTS** | `ScienceLabModule` ✅ |
| `Reactor` | Power / energy reactor | **Station module — EXISTS** | `ReactorModule` ✅ |
| `SolarArray` | Solar power collector | **Station module — EXISTS** | `SolarArrayModule` ✅ |
| `FuelDepot` | Fuel storage & processing | **Station module — EXISTS** | `FuelDepotModule` ✅ |
| `PhysicsLab` | Propulsion R&D | Research lab (NEW) | ⚠️ `PhysicsLabModule` |
| `MaterialsLab` | Materials R&D | Research lab (NEW) | ⚠️ `MaterialsLabModule` |
| `ElectronicsLab` | Electronics R&D | Research lab (NEW) | ⚠️ `ElectronicsLabModule` |
| `WeaponsLab` | Weapons R&D | Research lab (NEW) | ⚠️ `WeaponsLabModule` |
| `BiologyLab` | Bio / medical R&D | Research lab (NEW) | ⚠️ `BiologyLabModule` |

**Summary**: 6 production modules already exist; **10 new facility modules** are
indicated by the tree (5 resource/extraction + 5 research labs).

---

## 2. Industry value chain (facility → what it produces)

```
           STAR-BOUND  (ships / mining stations / rigs)
  ─────────────────────────────────────────────────────
  ShipMining ─► ores, platinum, rare earths, silicon, carbon crystal
  GasHarvesting ─► helium-3, hydrogen, methane, nitrogen, noble gas
  IceCollection ─► water ice
  OrganicFarming ─► biomass, algae, grain, fruit, fungus, livestock, plant fibre
  Salvage ─► salvaged components, scrap metal, derelict hull plate
        │
        ▼
  ┌─ ProcessingModule ─(refine)─► alloys, wafers, wiring, food, chemicals
  │          └─────────────► SyntheticProtein, FoodRations, CulturedMeat …
  │          └─────────────► SteelAlloy, TitaniumAlloy, SiliconWafer …
  ├─ SolarArrayModule ─(power)──► PowerCells
  ├─ ReactorModule ─(energy)──► AdvancedPowerCells, FusionFuelCell, PlasmaFuel
  │          └─────────► PowerCore
  ├─ FuelDepotModule ─► HydrogenFuel
  │
  ▼
  ┌─ FabricationModule ─(assemble)─► Components, Electronics, chips, computers
  │          └─────────────► ShipComponents, weapons, shields, station parts, modules
  └─ ScienceLabModule ─(research)─► QuantumProcessor, AICores, research data
        │
        ▼
  Research LABs (NEW modules)
  ────────────────────────────
  PhysicsLab      ─► PropulsionResearch  → Mk2/3 engines & reactors
  MaterialsLab    ─► MaterialsResearch   → Mk2/3 hulls & plating
  ElectronicsLab  ─► ComputingResearch   → Mk2/3 chips & computers
  WeaponsLab      ─► DefenceResearch     → Mk2/3 weapons & shields
  BiologyLab      ─► BioResearch         → Mk2/3 medicine & vaccines
```

This chain maps 1:1 onto the **5 crafting tiers** deep industry supports:
raw → refined → components → assemblies → modules.

---

## 3. Station module requirements (the decision list)

These are the **new** `ASpaceStationModule` subclasses the plan calls for, grouped by
the existing `EStationModuleGroup` enum, with a suggested power profile:

### A. Extraction & resource (group: **Processing** or new **Extraction**)
| New module | Purpose | Pow | Tree tag it serves |
|------------|---------|-----|--------------------|
| `MiningModule` | Run asteroid-mining rigs; accepts ore | -35 | `ShipMining` |
| `GasHarvesterModule` | Skim gas giants / clouds | -30 | `GasHarvesting` |
| `IceHarvesterModule` | Collect water ice | -25 | `IceCollection` |
| `FarmingModule` (Hydroponics) | Grow crops / livestock cultures | -30 | `OrganicFarming` |
| `SalvageModule` | Process derelicts into components | -25 | `Salvage` |

### B. Research labs (group: **Processing** or new **Research**)
| New module | Purpose | Pow | Breakthrough produced |
|------------|---------|-----|----------------------|
| `PhysicsLabModule` | Propulsion R&D | -40 | Propulsion / AdvancedPropulsion |
| `MaterialsLabModule` | Materials R&D | -40 | Materials / NanoMaterials |
| `ElectronicsLabModule` | Electronics R&D | -45 | Computing / QuantumComputing |
| `WeaponsLabModule` | Weapons R&D | -45 | Defence / AdvancedDefence |
| `BiologyLabModule` | Bio / medical R&D | -40 | Bio / AdvancedBio |

### B2. Niche / upgraded research labs (group: Research)
A **weapons specialisation** split — build the base `WeaponsLabModule`, then upgrade
to a niche lab to unlock a weapon family's Mk2/3:
| New module | Built from | Niche breakthrough | Unlocks |
|------------|-----------|--------------------|---------|
| `ProjectileWeaponsLab` | WeaponsLabModule + Railgun | KineticWeaponResearch | Railgun, Missile, Torpedo Mk2/3 |
| `BeamWeaponsLab` | WeaponsLabModule + PlasmaCannon | BeamWeaponResearch | TriLaser, PlasmaCannon, Point-Defence Mk2/3 |

The pattern generalises to the other labs (e.g. a propulsion sub-field) as the tree grows. Current set:
| New module | Built from | Niche breakthrough |
|------------|-----------|--------------------|
| `IonPropulsionLab` | PhysicsLabModule | Ion Propulsion |
| `GravMaterialsLab` | MaterialsLabModule | Gravity Materials |
| `EncryptionLab` | ElectronicsLabModule | Encryption |
| `OptronicsLab` | ElectronicsLabModule | Optronics |
| `CyberneticsLab` | BiologyLabModule | Cybernetics |

### B3. Contracted research (the most niche work is outsourced)
The **deepest, most niche** breakthroughs can't be researched in any in-house lab at
all — they require the **staff expertise + facilities of an external guild/company**.
These are produced in the `Contract:Researchers` channel by a designated provider, and
carry `ExpertiseLevel 3`:
| Contract breakthrough | Provider (guild/company) | `DA_Way_*` asset |
|----------------------|--------------------------|------------------|
| Antimatter Containment | Scholars' Syndicate | DA_ScholarsSyndicate |
| Wormhole Navigation | Frontier Alliance | DA_FrontierAlliance |
| Fusion Miniaturization | Engineers' Collective | DA_CraftsmenCompact |
| Shield Bypass | Honour Circle | DA_HonorCircle |
| Micro-Fabrication | Merchant Coalition | DA_MerchantCoalition |
| Deep-Space Survey | Traders' Guild | DA_Way_TradersGuild |

This reintroduces a **faction/guild layer** on top of the removed MVP reputation
system: to get these, the player must find/befriend the guild and use their contracted
research service — a natural hook for the DA_Way_* guilds that already exist as data
assets.

> Note: the 5 research-lab modules are *already* in the crafting tree as craftable
> Tier-6 outputs (e.g. `PhysicsLabModule`, `ElectronicsLabModule`...). Building them
> in-game is the requirement; they just need engine classes. The industry plan makes
> them first-class station modules.

### C. Existing modules confirmed sufficient for refinement & assembly
`ProcessingModule`, `FabricationModule`, `ScienceLabModule`, `ReactorModule`,
`SolarArrayModule`, `FuelDepotModule` — **all already exist in-engine.** No new
classes needed for those tiers.

---

## 4. Compact schema (the "what to build" summary)

```
EXISTING (build industry with these)            NEW (must add classes for)
──────────────────────────────                  ──────────────────────────
ProcessingModule            ✔                  MiningModule
FabricationModule           ✔                  GasHarvesterModule
ScienceLabModule            ✔                  IceHarvesterModule
ReactorModule               ✔                  FarmingModule
SolarArrayModule            ✔                  SalvageModule
FuelDepotModule             ✔                  PhysicsLabModule
                                               MaterialsLabModule
                                               ElectronicsLabModule
                                               WeaponsLabModule
                                               BiologyLabModule
```

**Decision:** the tree *requires* the 5 research-lab modules and the 5 resource
extraction modules to fully play out every recipe. Everything below the research
tier (refinery/fabrication/science) is already covered by existing classes.

---

## 5. Suggested `EStationModuleGroup` additions

If we want filtering to reflect the new industry, extend `EStationModuleGroup`:
```cpp
enum class EStationModuleGroup : uint8
{
    All, Docking, Power, Storage, Processing, Defence, Habitation,
    Public, Connection,
    Extraction,   // NEW: Mining, Gas, Ice, Salvage, Farming
    Research,     // NEW: the 5 lab modules
    Other
};
```

---

## 6. Build order / recommendation

1. **Research labs first** — they gate every Mk2/Mk3 upgrade and are already in the
   tree as Tier-6 craftables; adding their 5 classes unblocks the entire progression
   system. Group `Research`.
2. **Extraction modules next** — `MiningModule` (most-used: 21 recipes), then
   `FarmingModule` (7), `GasHarvesterModule` (5), `SalvageModule` (3),
   `IceHarvesterModule` (1). Group `Extraction`.
3. Wire each new module's `ModuleType` string exactly to the `ProducedIn` tag so the
   crafting lookup hits correctly (e.g. `TEXT("PhysicsLab")` for the Physics lab).

---

## 7. Data contract for `main`

Each new module should follow the existing `ASpaceStationModule` pattern:
```cpp
class ADASTREA_API APhysicsLabModule : public ASpaceStationModule
{
    GENERATED_BODY()
public:
    APhysicsLabModule() {
        ModuleType = TEXT("PhysicsLab");          // matches ProducedIn
        ModuleGroup = EStationModuleGroup::Research;
        ModulePower = -40.0f;
    }
};
```
The `ModuleType` string **must** equal the `ProducedIn` tag the crafting tree uses, so
`CanBeProcessedByModule` / lookup resolves correctly.

---

## 8. Related

- Crafting tree & recipes: `CRAFTING_TREE.md`, `Content/Data/CraftingTree.json`
- Research tree: `RESEARCH_TREE.md`, `Content/Data/ResearchTree.json`
- Existing station: `STATION_SYSTEM.md`, `Source/Adastrea/Public/Stations/*.h`