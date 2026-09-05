# Station Modules — Build Plan

Grounded inventory + phased roadmap for creating **all 27 craftable station modules**
(15 core T6 + 12 research-lab T7). Complements the crafting tree (`CRAFTING_TREE.md`,
`Content/Data/CraftingTree.json`) which already stops at Station Modules — this doc
covers turning those recipes into in-engine, asset-backed, playable modules.

**Lanes:** `main` = C++ gameplay / HUD / editor; `assets` = Blender / plugin meshes;
`crafting` = recipe & economy data. This doc is the cross-lane contract.

## Current state (all three halves)

| Module | Group | C++ class | Gameplay | 3D mesh |
|---|---|---|---|---|
| DockingBay | Docking | ✅ `DockingBayModule` (237 ln) | ✅ real | ❌ none |
| DockingPort | Docking | ✅ `DockingPortModule` (235 ln) | ✅ real | ❌ none |
| Reactor | Power | ⚠️ `ReactorModule` 10-ln stub | ❌ | ◐ Mining_01_Reactor |
| SolarArray | Power | ⚠️ 10-ln stub | ❌ | ❌ |
| CargoBay | Storage | ⚠️ 10-ln stub | ❌ | ◐ Mining_01_Cargo |
| FuelDepot | Storage | ⚠️ 10-ln stub | ❌ | ❌ |
| Fabrication | Processing | ⚠️ 10-ln stub | ❌ | ❌ |
| Processing | Processing | ⚠️ 10-ln stub | ❌ | ◐ Mining drill/laser |
| ShieldGenerator | Defence | ⚠️ 10-ln stub | ❌ | ❌ |
| Turret | Defence | ⚠️ 10-ln stub | ❌ | ❌ |
| Barracks | Habitation | ⚠️ 10-ln stub | ❌ | ❌ |
| Habitation | Habitation | ⚠️ 10-ln stub | ❌ | ✅ Habitation_01 + AsteroidHab |
| Marketplace | Public | ◐ `MarketplaceModule` 15-ln | ◐ | ❌ |
| Corridor | Connection | ⚠️ 10-ln stub | ❌ | ❌ |
| ScienceLab | Other | ⚠️ 10-ln stub | ❌ | ❌ |
| **+ 12 research labs** (Physics, Materials, Electronics, Weapons, Biology + 7 niche: ProjectileWeapons, BeamWeapons, IonPropulsion, GravMaterials, Encryption, Optronics, Cybernetics) | — | ❌ no class at all | ❌ | ❌ |

**Summary:** 2/27 fully built. 12 empty C++ stubs (header only — no power / storage /
processing / defence / crew behavior). 12 research labs exist only as recipes. Only 3
station visual sets on disk (`Habitation_01`, `Mining_01`, `AsteroidHab_01`); only
`Hab` has a texture set.

**Working infrastructure already present (reuse, don't redo):**
- `ASpaceStation` (`SpaceStation.cpp` 694 ln): `Modules[]`, `AddModule/AddModuleAtLocation/
  RemoveModule/MoveModule`, group/count aggregation, `GetTotalPowerConsumption` /
  `GetTotalPowerGeneration` / `GetPowerBalance` / `HasDockingCapability` / `HasMarketplace` /
  `HasCargoStorage`.
- `ASpaceStationModule` (`SpaceStationModule.cpp` 306 ln): base class with `ModuleType`,
  `ModulePower`, `EStationModuleGroup`.
- `DA_StationModuleCatalog`: `ModuleEntries[]` with `ModuleClass` / `ModuleGroup` /
  `Power` / `BuildCost` / `BuildTime` / `Materials` / `PreviewMesh` / `RequiredTechLevel`.
- `CraftingTreeLoader` + `CraftingTree.json` 315 recipes; every T6/T7 module recipe exists.

## Phased roadmap

### Phase 1 — Real gameplay for the 12 stub modules (C++, main)
Grow each stub (`Source/Adastrea/Private/Stations/<Module>.cpp`) from ~10 lines to real
`ASpaceStationModule` behavior:
- `PowerConsumption` / `PowerGeneration` (Reactor, SolarArray).
- `StorageCapacity` (CargoBay, FuelDepot).
- `ProcessingRate` + recipe slots (Fabrication, Processing).
- `ShieldStrength` / `Firepower` (ShieldGenerator, Turret).
- `CrewCapacity` (Barracks, Habitation).
- ScienceLab → research-output hook.
Wire all into the existing `GetPowerBalance()` / `HasCargoStorage()` / group aggregates.
Pure C++ — fully testable, no assets needed. **Unblocks every downstream phase.**

### Phase 2 — The module mesh kit (Blender / plugin, assets)
A **unified station module visual language**: one shared "module carcass/shell" all
modules snap onto (single coherent silhouette, no stacked-box look), plus a distinct
identity mesh per group: Docking (bay/port), Power (reactor + solar array), Storage
(cargo + fuel depot), Processing (fabricator + refinery), Defence (shield gen + turret),
Habitation, Market, Corridor connector. Follow the proven split-part pattern
`SM_Station_<Group>_01_<Part>` + `T_Station_<Group>_*` tex set per module (only Hab has
textures today). Research labs reuse the ScienceLab/Processing shell + `SM_Int_*` interior
kit visuals.

### Phase 3 — Catalog + Blueprint wiring (main, editor data)
Every module → new `ModuleEntries` row in `DA_StationModuleCatalog` with real
`BuildCost` / `BuildTime` / `Power` / `RequiredTechLevel` **driven from its crafting
recipe**, a `PreviewMesh`, and a `BP_StationModule_<Type>` variant (only CargoBay,
DockingBay, Market BPs exist today). Closes the loop so the station-editor placement UI
sees every module.

### Phase 4 — Station construction gameplay (main)
Make `AddModule` / `MoveModule` / `IsValidPlacement` + power aggregation *playable*:
place a module only if you hold its crafted piece in cargo + have enough power; modules
consume/generate power; docking & market only function when those modules exist.

### Phase 5 — Research-lab modules (main + assets)
Create C++ classes + meshes + catalog rows for the 12 T7 labs (base → niche upgrade
shape already defined in the crafting data); each lab is itself a craftable station module,
so the Phase 1–3 plumbing extends cleanly.

### Phase 6 — Crafting in-engine (main + crafting)
Fabrication / Processing actually consume `CraftingTree.json` recipes to build modules &
ships — the tree stops being market filler and becomes the real build pipeline.

## Priority
**1 → 2 → 3 first.** Phase 1 makes every module meaningful as code; Phase 2 gives visuals;
Phase 3 registers them. That is the vertical slice where each module is "created." Research
labs (5) and construction gameplay (4) layer on top.

## Scope guard (user, explicit)
- **No whole-station "blueprint" recipes.** Stay at modules / construction parts (T6) +
  research labs (T7). A whole-station assembly recipe has no engine counterpart.
- Research = **domain-specialized labs**, not one generic ladder.
- Keep the acyclic / cost-driven economy invariants (see `game-data-crafting-trees` skill);
  any `BuildCost` in the catalog must derive from the recipe's cost-driven `BaseValue`.