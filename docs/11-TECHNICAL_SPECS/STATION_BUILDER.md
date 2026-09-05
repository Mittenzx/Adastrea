# Station Builder — In-Game Station Design & Layout (X4-style)

**Trade Simulator MVP → Post-MVP | Last Updated: 2026-09-02 | Audience: main/gameplay**

This spec designs the **in-game space-station builder**: a plan-mode editor where the
player designs a station's layout by placing modules on a connection grid — the same
feel as *X4: Foundations*' station planner. It builds directly on the existing
`ASpaceStation` / `ASpaceStationModule` system and the crafting tree's station-module
set, and reuses the ship module-builder contract's conventions where they apply.

---

## 1. Goals & non-goals

**Goals**
- Let the player **design a station layout** in a dedicated plan view: place, rotate,
  connect, and remove modules on a grid.
- **Validate the design live** (power balance, connectivity, docking access,
  production-chain completeness) so a broken station can't be built.
- **Persist the layout** as a data asset / save entry, then **spawn the real station**
  from it (reusing `ASpaceStation` + `ASpaceStationModule`).
- Tie the builder to the **crafting tree**: modules are *built* from crafted parts
  (the Tier-6 station modules already in `CraftingTree.json`), so building a station
  is the payoff of the crafting economy.

**Non-goals (v1)**
- No free-form 3D placement — modules snap to a **connection grid** (X4-style).
- No in-flight editing — the builder is a **plan mode** entered from a station/plot.
- No terrain/planetary constraints — stations are built in open space.

---

## 2. Core interaction model

The builder is a **plan mode** (a separate game state / UI layer), entered by:
- selecting an **empty build plot** in space, or
- opening an **existing station** for expansion.

### 2.1 The build plot
A build plot is a bounded volume (e.g. a 1 km cube) anchored at a point in space.
It has a **connection grid** — a 3D lattice of **connection nodes** at fixed spacing
(e.g. 100 m apart). Modules snap to nodes.

```
Build plot (top-down, X4-style):
   ┌────┬────┬────┬────┬────┐
   │    │    │    │    │    │   ● = connection node (grid point)
   ├────┼────┼────┼────┼────┤
   │    │    │    │    │    │
   ├────┼────┼────┼────┼────┤
   │    │    │    │    │    │
   └────┴────┴────┴────┴────┘
```

### 2.2 Module placement
- **Select** a module from the palette → it becomes a **ghost** that follows the
  cursor, snapping to the nearest free node.
- **Rotate** (R / Q-E) to orient the module's connection faces.
- **Place** (click) → the module is added to the plan, connecting to any adjacent
  module's connection face.
- **Remove** (right-click / delete) → module is removed; its children (modules that
  only connect through it) are flagged as **disconnected**.
- **Undo/Redo** (Ctrl-Z / Ctrl-Y) for the whole session.

### 2.3 Connection faces
Each module has **connection faces** (N/S/E/W/Up/Down) — the sides where it can
attach to another module. A module is **connected** if at least one face touches a
neighbouring module's face. The **core** (first module placed) anchors the station.

```
Module connection faces (top-down):
        N
      ┌───┐
   W  │   │  E
      └───┘
        S
```

---

## 3. Module catalog (from the crafting tree)

The builder's palette is exactly the **craftable station modules** in
`Content/Data/CraftingTree.json` (Tier 6). Each maps to an `ASpaceStationModule`
subclass and an `EStationModuleGroup`. The builder reads the catalog from the tree
(plus a small builder-only metadata table for grid size / connection faces / power).

| Module (ItemID) | Class | Group | Grid size | Power | Role |
|-----------------|-------|-------|-----------|-------|------|
| `CorridorModule` | `ACorridorModule` | Connection | 1×1 | -5 | Connects modules |
| `CargoBayModule` | `ACargoBayModule` | Storage | 2×2 | -20 | Storage |
| `DockingBayModule` | `ADockingBayModule` | Docking | 3×2 | -30 | Ship docking |
| `DockingPortModule` | `ADockingPortModule` | Docking | 1×1 | -10 | Single dock |
| `MarketplaceModule` | `AMarketplaceModule` | Public | 2×2 | -40 | Trading |
| `HabitationModule` | `AHabitationModule` | Habitation | 2×2 | -25 | Crew quarters |
| `BarracksModule` | `ABarracksModule` | Habitation | 2×2 | -30 | NPC housing |
| `ReactorModule` | `AReactorModule` | Power | 2×2 | **+200** | Power gen |
| `SolarArrayModule` | `ASolarArrayModule` | Power | 3×1 | **+50** | Power gen |
| `ProcessingModule` | `AProcessingModule` | Processing | 2×2 | -40 | Refining |
| `FabricationModule` | `AFabricationModule` | Processing | 2×2 | -50 | Crafting |
| `ScienceLabModule` | `AScienceLabModule` | Processing | 2×2 | -40 | Research |
| `FuelDepotModule` | `AFuelDepotModule` | Storage | 2×2 | -30 | Fuel |
| `ShieldGeneratorModule` | `AShieldGeneratorModule` | Defence | 2×2 | -60 | Shields |
| `TurretModule` | `ATurretModule` | Defence | 1×1 | -40 | Defence |
| `PhysicsLabModule` | `APhysicsLabModule` | Research | 2×2 | -40 | Propulsion R&D |
| `MaterialsLabModule` | `AMaterialsLabModule` | Research | 2×2 | -40 | Materials R&D |
| `ElectronicsLabModule` | `AElectronicsLabModule` | Research | 2×2 | -45 | Computing R&D |
| `WeaponsLabModule` | `AWeaponsLabModule` | Research | 2×2 | -45 | Weapons R&D |
| `BiologyLabModule` | `ABiologyLabModule` | Research | 2×2 | -40 | Bio R&D |

> The 5 research-lab modules and the niche labs (`ProjectileWeaponsLab`,
> `BeamWeaponsLab`, `IonPropulsionLab`, etc.) are all craftable in the tree and can
> be placed in the builder. The builder metadata table is the single place that
> defines grid size / connection faces / power for the builder (the runtime module
> classes already carry `ModulePower`).

---

## 4. Layout rules & validation

The builder validates the design **live** and blocks committing an invalid station.
Validation rules (each returns pass/fail + a message):

### 4.1 Connectivity
- Every placed module must be **connected** to the core (directly or transitively
  through other modules). A module with no connection face touching a neighbour is
  **disconnected** → invalid.
- **Face-aware:** two modules connect only when they are cell-adjacent **and** each
  exposes a compatible connection face on the shared side (module A's `N` face meets
  module B's `S` face, etc.). Rotation re-orients a module's faces, so a directional
  module (e.g. a `SolarArrayModule` that only connects through its `W` hub face) will
  *disconnect* if rotated so that face no longer points at its neighbour.
- The **core** is the first module placed; it anchors the station.

### 4.2 Power balance
- `Σ power_generation ≥ Σ power_consumption` (net ≥ 0). Uses each module's
  `ModulePower` (negative = generates, positive = consumes).
- If the station has no power module, it's **invalid** (can't run).

### 4.3 Docking access
- A station must have **at least one** `DockingBayModule` or `DockingPortModule`
  to be reachable by ships (matches `ASpaceStation::HasDockingCapability()`).
- Docking modules must be **connected** to the core (they are, by rule 4.1).

### 4.4 Production-chain completeness (optional, "smart" validation)
- If the player places a `FabricationModule` or `ProcessingModule`, the builder can
  warn if the station lacks the **input storage** (CargoBay) or **power** to run it.
- This is a *warning*, not a hard block (the player may import inputs).

### 4.5 Grid bounds
- All modules must be within the build plot's bounds.

---

## 5. Data contract — station layout

A station layout is a **JSON document** that fully describes a design. It's what the
builder saves and what `ASpaceStation` reads to spawn the real station.

### 5.1 Schema — `StationLayout.json`

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "Title": "Adastrea Station Layout",
  "SchemaVersion": "1.0.0",
  "StationName": "My Trade Hub",
  "PlotSize": [1000, 1000, 1000],
  "GridSpacing": 100,
  "Modules": [
    {
      "ModuleID": "M1",
      "ItemID": "CorridorModule",
      "GridPos": [0, 0, 0],
      "Rotation": 0,
      "IsCore": true
    },
    {
      "ModuleID": "M2",
      "ItemID": "CargoBayModule",
      "GridPos": [1, 0, 0],
      "Rotation": 0,
      "IsCore": false
    }
  ]
}
```

### 5.2 Fields
- **`GridPos`** — integer grid coordinates (x, y, z) in units of `GridSpacing`.
- **`Rotation`** — 0/90/180/270 (yaw in degrees) for the module's orientation.
- **`IsCore`** — exactly one module is the core (the station anchor).
- **`ItemID`** — must be a craftable station module from `CraftingTree.json`.

### 5.3 Spawning the station
`ASpaceStation` reads the layout and, for each module:
1. Looks up the `ASpaceStationModule` subclass by `ItemID`.
2. Spawns it at `GridPos * GridSpacing` (station-local), rotated by `Rotation`.
3. Registers it in `Modules` (via the existing `AddModule`/`AddModuleAtLocation`).
4. Recomputes power / docking / storage aggregates (existing `GetTotalPowerBalance`,
   `GetTotalDockingCapacity`, etc.).

---

## 6. UI / UX flow

1. **Enter builder** — select a build plot or an existing station → plan mode.
2. **Palette** (left panel) — grouped by `EStationModuleGroup` (Docking, Power,
   Storage, Processing, Defence, Habitation, Public, Connection, Research). Each
   entry shows the module's icon, grid size, power, and **crafting cost** (from the
   tree's `Economy`/ingredients).
3. **Viewport** (center) — the build plot with the connection grid; ghost module
   follows the cursor, snapping to nodes; rotation via R/Q-E.
4. **Validation panel** (right) — live pass/fail for connectivity, power, docking,
   bounds; green/red indicators.
5. **Stats bar** (top) — total power balance, docking capacity, storage, module count,
   and **total build cost** (sum of crafting costs).
6. **Commit** — "Build Station" consumes the crafting materials (from the player's
   cargo / station storage) and spawns the real station. If materials are short, the
   builder shows the deficit and blocks commit.

---

## 7. Integration with the crafting tree

- The **palette** is generated from `CraftingTree.json` (all `*Module` Tier-6 items).
- Each module's **build cost** = the sum of its recipe's ingredient values (from the
  tree's `Economy` map) — so a station is a real economic investment.
- Building a station **consumes** the crafted modules (or their ingredients) from the
  player's inventory, tying the builder to the crafting loop.
- The **research labs** placed in a station unlock the research tree's breakthroughs
  (the `ProducedIn` tags match the module classes), so a station with a
  `WeaponsLabModule` can run `DefenceResearch`, etc.

---

## 8. Implementation notes for `main`

- **Re-enable the deferred module-management API** on `ASpaceStation`:
  `AddModule`, `AddModuleAtLocation`, `RemoveModule`, `MoveModule`,
  `GetModulesByType` (currently commented out as POST-MVP).
- Add a **`UStationLayoutDataAsset`** (mirrors `UTradeItemDataAsset` style) that
  holds a `StationLayout` JSON, so layouts are authorable assets.
- Add a **`UStationBuilderComponent`** (or a `AStationBuilderController`) that owns
  the plan-mode state: palette, ghost placement, validation, commit.
- **Grid/connection metadata** lives in a small `StationModuleBuilderData` table
  (grid size, connection faces, power) keyed by `ItemID` — the single source for the
  builder, separate from the runtime module classes.
- **Validation** is pure data logic (no UE dependency) — implement it as a
  `check_station_layout()` function (Python, in the generator) so it's unit-testable,
  and mirror it in C++ for runtime.

---

## 9. Related

- **Improvement roadmap:** `STATION_BUILDER_ROADMAP.md`
- Crafting tree & module set: `CRAFTING_TREE.md`, `Content/Data/CraftingTree.json`
- Station system: `STATION_SYSTEM.md`, `Source/Adastrea/Public/Stations/*.h`
- Ship module-builder contract (reference pattern): `MODULE_BUILDER_DATA_CONTRACT.md`
- Facility/industry plan (which modules to build): `FACILITY_AND_INDUSTRY_PLAN.md`

## 10. Persistence, blueprints & in-UE integration

Beyond the design + validator, the builder's **data/logic layer** now supports the
full construction loop (implemented + unit-tested in `generate_station_builder.py`):

### 10.1 Blueprint string (4.4)
A validated layout serializes to a compact, shareable string and back:
```
1.0.0;1000,1000,1000;100;M1:CorridorModule:6,4,0:0:1;M2:CargoBayModule:7,4,0:0:0;...
```
Format: `SchemaVersion;PlotXYZ;GridSpacing;` then `ModuleID:ItemID:gx,gy,gz:rot:isCore`
per module. `layout_to_blueprint()` / `blueprint_to_layout()` round-trip exactly.
Use for copying designs, templates, and community sharing.

### 10.2 Build-cost + crew budget (3.4 / 4.5)
- `build_cost_summary()` — total build cost (sum of each module's crafted
  `OutputValue`) + per-group module counts.
- `crew_budget()` — berths provided (Habitation/Barracks) vs crew required by
  operational modules; positive margin = fully staffed.
- `check_build_materials()` — given the player's held modules/cargo, reports
  whether the design can be built and any material shortfall.

### 10.3 Research-gated modules (4.1)
Niche lab modules require their research breakthrough before appearing in the
palette (the `RESEARCH_GATE` map): `BeamWeaponsLab` ↦ `BeamWeaponResearch`, etc.
This ties construction to the research tree.

### 10.4 In-place upgrade (4.2)
A placed module can be upgraded to its Mk2 variant in place (`upgrade_module()`),
keeping its grid position/rotation/core, e.g. `CargoBayModule` → `CargoBayModule_Mk2`.

### 10.5 In-UE data assets (2.1 / 2.2, written; need a UE build to verify)
- `UStationLayoutDataAsset` (`Public/Stations/`): a `UDataAsset` holding a parsed
  layout — `StationName`, `PlotSize`, `GridSpacing`, `Modules[]`, cost/crew fields.
  Mirror of `UTradeItemDataAsset`.
- `UStationLayoutLoader` (`Private/Stations/`): reads a StationLayout JSON (e.g.
  `Content/Data/ExampleStationLayout.json`) into the asset, mirroring
  `UCraftingTreeLoader`'s JSON pattern.
- `ASpaceStation::BuildFromLayout(Layout)` is declared (with the module-management
  API `AddModule`/`AddModuleAtLocation`/`RemoveModule`/`GetModulesByType` re-enabled);
  their implementations already exist in `SpaceStation.cpp`. This is the hook that
  spawns the real station from a committed design.

> ⚠️ **Build status:** the C++ files compile-check cleanly (balanced braces/parens)
> but have NOT been verified with a full UE build in this environment. Verify with a
> real `Build.bat` before merging to a playable branch.
