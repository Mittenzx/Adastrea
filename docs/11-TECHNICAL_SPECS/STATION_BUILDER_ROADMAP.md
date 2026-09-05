# Station Builder — Improvement Roadmap

**Status:** Living plan | **Last Updated:** 2026-09-05 | **Owner:** crafting/design

A prioritized roadmap for evolving `STATION_BUILDER.md` from a design + HTML prototype
into a fully playable in-game construction system. Each item is scoped to be
independently shippable; items are ordered by value/effort and dependency.

---

## Current state (the baseline)

- **Design spec** — `STATION_BUILDER.md` (plan mode, module catalog, validation rules,
  `StationLayout` JSON contract, UI/UX flow, crafting-tree integration).
- **Logic** — `generate_station_builder.py`: layout **validator**
  (`check_station_layout()`: connectivity BFS, power balance, docking access, single
  core, bounds) + builder metadata + example layout. Unit-tested (16/16 pass).
- **Prototype** — `station_builder_prototype.html`: interactive X4-style builder
  (palette, connection grid, ghost placement, live validation, PLAN MODE HUD),
  frame-driven for deterministic capture; demo video rendered.
- **Not yet built** — no in-UE implementation; no `UStationLayoutDataAsset`; no
  builder controller; production-chain / warnings missing.

---

## Roadmap (prioritized)

### Phase 1 — Grow the design model (cheap, unblocks everything)
| # | Item | What | Why | Effort |
|---|------|------|-----|--------|
| 1.1 | **More module metadata** | Add grid size, connection faces (N/S/E/W/Up/Down), build cost, and a `rotation` table for ALL craftable modules (currently only 20 in the metadata). Pull automatically from the crafting tree. | The palette and validation need complete per-module data. | S |
| 1.2 | **Production-chain validation** | If a `Fabrication`/`Processing`/lab module is placed, warn when inputs (storage) or power supply are missing (import-aware). | "Smart" validation catches half-built industry stations. | M |
| 1.3 | **Module build-cost display** | Show each module's crafting cost (from `CraftingTree.json` `Economy`) in the palette + total station cost in the HUD. | Ties builder to the crafting economy (already prototyped as "4,800 cr"). | S |
| 1.4 | **Overlapping/occupied rule** | Model module **footprints + occupancy** so two modules can't occupy the same cells (BFS already uses footprints; formalize the no-overlap check). | Prevents broken overlapping layouts. | S |
| 1.5 | **Connection-face orientation** | Validate that modules connect only via allowed faces (a module with no face toward its neighbour is invalid), not just cell adjacency. | Matches the spec's "connection faces" concept. | M |

### Phase 2 — Persistence & data (make it a real asset)
| # | Item | What | Why | Effort |
|---|------|------|-----|--------|
| 2.1 | **`UStationLayoutDataAsset`** | C++ data-asset class (mirror `UTradeItemDataAsset` style) that loads a `StationLayout` JSON. | Authorable, saveable layouts. | M (needs UE build to verify) |
| 2.2 | **Layout → station spawner** | `ASpaceStation::BuildFromLayout(layout)`: reads the JSON, spawns each module at `GridPos*Spacing`, rotates, registers, recomputes aggregates. Re-enable the deferred module-management API (`AddModule`, `AddModuleAtLocation`, `MoveModule`, `RemoveModule`). | Turns a design into a real station. | M |
| 2.3 | **Save/load design** | Persist the live plan to the save game (module list, positions, rotation) and restore on reload. | Builds don't vanish. | M |

### Phase 3 — In-engine builder UI (the actual gameplay)
| # | Item | What | Why | Effort |
|---|------|------|-----|--------|
| 3.1 | **Builder controller** | `AStationBuilderComponent` (or controller actor) owning plan-mode state: palette, ghost placement, rotation, commit. | The builder needs a runtime owner. | L |
| 3.2 | **Input scheme** | Mouse: click palette → place, R/Q-E rotate, Del/right-click remove, Ctrl-Z/Y undo/redo. Match the prototype's key/UX conventions. | Core interaction feel. | M |
| 3.3 | **3D module meshes in builder** | Show real module meshes (from `SM_Modules_catalog` / FBX pipeline) on the grid instead of colored tiles. | Visually matches the actual game. | M (depends on assets agent) |
| 3.4 | **Build-commit flow** | "Build Station" consumes crafted modules (or ingredients) from cargo/station storage per the tree's recipes; blocks on material deficit. | Payoff = the crafting economy. | M |
| 3.5 | **Orbiting/3D camera** | Rotate/pan/zoom the plan view (X4-style), with togglable grid + a top-down snap. | Readability of large stations. | M |

### Phase 4 — Deeper systems (post-MVP flavor)
| # | Item | What | Why | Effort |
|---|------|------|-----|--------|
| 4.1 | **Research-gated module access** | Some modules (e.g. niche labs like `BeamWeaponsLab`) require their research breakthrough before they appear in the palette. | Ties the builder to the research tree. | M |
| 4.2 | **Module upgrade in place** | Upgrade an existing module (e.g. Cargo → Cargo Mk2) without removing/re-placing, consuming the upgrade recipe. | Sweeter progression. | M |
| 4.3 | **Multiple build plots / station groups** | Let a "station" be a cluster of connected plots (spoke + sub-stations), with shared power grid. | Bigger, X4-grade construction. | L |
| 4.4 | **Copy / template / blueprint sharing** | Save a validated layout as a shareable blueprint string; paste to replicate on another plot. | Replayability + community. | S–M |
| 4.5 | **Cost & crew budget** | Track total build cost vs. player credits, and crew required vs. berths (from Barracks/Habitation). | Economic/crew constraints. | S |
| 4.6 | **Connection-line routing visualization** | Draw explicit module-to-module connection lines in the plan view (prototype already does). | Clearer connectivity debugging. | S |

---

## Suggested build order (first 3 iterations)

1. **Iteration A** — Phase 1 breadth: module metadata 1.1, cost display 1.3,
   no-overlap 1.4, connection-face validation 1.5. All pure data/logic, testable now
   in `generate_station_builder.py` (no UE build needed). Add pytest coverage.
2. **Iteration B** — production-chain warning 1.2, blueprint string 4.4, save/load
   scaffold 2.3. Still data/logic + JSON; keeps momentum while waiting on a UE build.
3. **Iteration C** — begin UE integration 2.1/2.2/3.1 once a build is available:
   `UStationLayoutDataAsset` + `BuildFromLayout` + builder controller. This is the
   first item that needs a compiling UE build.

## Definition of done per item
- Data/logic items: implemented + covered by a pytest (mirrors
  `test_station_builder_layout_valid`), diagrams/docs updated, committed + pushed.
- UE items: compiles, exercises PIE, documented, committed + pushed.

## Related
- Design spec: `STATION_BUILDER.md`
- Validator + example: `Content/Data/StationModuleBuilderData.json`,
  `Content/Data/ExampleStationLayout.json`, `generate_station_builder.py`
- Prototype + demo: `station_builder_prototype.html`,
  `media/station_builder_demo.mp4`
- Crafting tree / research tree: `CRAFTING_TREE.md`, `RESEARCH_TREE.md`