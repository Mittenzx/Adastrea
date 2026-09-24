# Adastrea — Project Knowledge Base

> **Purpose**: One authoritative, current snapshot of the Adastrea project — architecture, systems, pipelines, tooling, and how the pieces fit together. Written from direct repo inspection on 2026-09-13 (git `546baf32`). If a file referenced below differs from what you see, **the repo is the source of truth** — update this document.

---

## 1. What Is Adastrea

Adastrea is an **open-world space trade sim** built on **Unreal Engine 5.8** (SM5 shading, no Lumen/VSM, iGPU-friendly renderer settings) as a solo/agent-assisted project by Mittenzx. The current thrust is an X4-style trade-sim MVP: fly ships, dock at stations, buy/sell cargo, walk interior spaces on foot.

**Repo**: `https://github.com/Mittenzx/Adastrea` (MIT license) — local checkout `C:\Users\akuma\Adastrea`.

**At a glance (2026-09-13):**
- 128 pytest tests passing (0 failures)
- ~190 C++ source files across Ships / Stations / Trading / Player / UI / Input
- 73 scripts in `Tools/` (assets, renders, UE importers, QA)
- 53 ship meshes + 31 interior meshes imported into the `AdastreaShips` plugin
- 48 materials in `Content/Materials/` + 12 interior materials in the plugin
- One playable map: `TestLevel` (space sector with 2 stations)
- 128 UI/HUD stays C++-driven (`AAdastreaHUD::DrawHUD()` canvas)

---

## 2. Repository Layout

```
Adastrea/
├── Adastrea.uproject          # UE project (engine assoc. UUID, plugins list)
├── Source/Adastrea/           # Game C++ module (Public=headers, Private=impl)
│   └── (Ships, Stations, Trading, Player, UI, Input, Materials, Performance, Planets, Interfaces)
├── Plugins/
│   ├── AdastreaShips/         # Imported 3D kit (meshes, interiors, materials)
│   └── vc-ue-extensions/      # VS/UE editor extensions
├── Content/                   # UE assets: Blueprints, DataAssets, Maps, Materials, Textures, UI
├── Assets/FBX/generated/      # Blender exports: ships, interiors, textures, obj/, previews/
│   ├── hand_author/           # Hand-authored flagship ships (frigate, cutlass)
│   └── generated/material_map.json   # M_* material → T_* texture-set mapping
├── Tools/                     # 73 Python scripts (generator, renderers, QA, UE importers)
├── tests/                     # 128 pytest tests
├── docs/                      # Design docs, tech specs, phase plans, archive
├── wiki/                      # Git-wiki mirror (Core-Systems, Developer-Guides...)
├── ref_ships/ ref_textures/   # Free reference assets (CC0/CC-BY, gitignored)
├── AGENT_BOARD.md             # Cross-agent async coordination board (main ↔ assets)
├── ue_mcp.py                  # CLI helper for the built-in UE MCP server (port 8000)
└── build_with_ue_tools.bat    # UBT-based build script (Development/Win64)
```

---

## 3. Engine & Toolchain

| Item | Value |
|---|---|
| Engine | UE 5.8 (`C:\Program Files\Epic Games\UE_5.8`) |
| Graphics profile | `DefaultGraphicsPerformance=Scalable`, SM5, no Lumen/VSM |
| Screen % | `r.ScreenPercentage` 75 (integrated graphics budget) |
| Build | `build_with_ue_tools.bat Development Win64` (UBT via dotnet + VS2022 toolchain) |
| Editor launch | `UnrealEditor.exe Adastrea.uproject` |
| MCP server | Built-in UE MCP on `http://127.0.0.1:8000/mcp` (`.mcp.json`), JSON-RPC 2.0 over HTTP |
| Python | `python` (3.11) for tests; Blender 5.2 headless for asset gen |
| **Build gotcha** | Close the editor **before** `Build.bat`, or the build falsely reports Failed |
| **Live Coding** | Editor open + source changed → press `Ctrl+Alt+F11` to recompile (or rebuild via build script) |

**MCP access pattern** (`ue_mcp.py` helper): `POST /mcp` → `initialize` (capture `Mcp-Session-Id` header) → `tools/call` with `toolset_name` + `tool_name` + `arguments`. Key toolsets: `EditorToolset.EditorAppToolset` (StartPIE/StopPIE/IsPIERunning/SearchCVars), `EditorToolset.LogsToolset` (GetLogEntries), `editor_toolset.toolsets.object.ObjectTools` (get/set_properties), `...blueprint.BlueprintTools` (get_default_object), `...asset.AssetTools` (find_assets/save_assets), `...material.MaterialTools`.

---

## 4. Gameplay Systems (C++)

### 4.1 Ships (`Source/Adastrea/{Public,Private}/Ships/`)
- **`ASpaceship`** (pawn base): hull mesh, movement (`UFloatingPawnMovement`), camera spring-arm, cargo + trader components, docking. X4-style flight model (flight assist, throttle, boost, travel mode, mouse-position look). **Runtime input mapping context** in C++ so controls work without content-side IMC (legacy bindings + Enhanced Input actions).
- **`ASpaceshipAvatar`**: the on-foot pawn. Capsule ~42×96, flying movement (gravity 0). WASD move, mouse look, Shift sprint, C crouch, **E interact** (`IWorldInteractable` / `UPlayerInteractableComponent`), **V sit down** (return to cockpit). Held inside the interior by a Tick-time clamp using `Interior->GetLocalHalfExtents()`.
- **`ASpaceshipInterior`**: walkable room actor. `ConfigureInterior(mesh, family)` loads the shell mesh (explicit arg → `DefaultInteriorMesh` → C++ fallback `SM_Int_Fighter_Cabin`), **normalizes scale to TargetRadius 650**, fits the walk volume + seat/exit trigger to mesh bounds, mounts per-family companion parts (driven by the explicit `EShipInteriorFamily` param — string-matching the shell mesh's name is now only a legacy fallback), assigns `M_Int_*` materials by slot name (consolidated into one static slot→path table; `M_Interior_Eng`/`M_Interior_Hab` now mapped instead of silently unhandled). Ambient fill + 2 warm fixture point-lights on reveal (no-Lumen plan).
- **`USpaceshipDataAsset`**: per-ship tuning (hull, cargo, mobility, combat, lore) + **Avatar Spawn** override (`bUseCustomAvatarSpawn`, `AvatarSpawnOffset` ±Yaw) used by `EnterShipInterior`. 18 authored ship data assets in `Content/DataAssets/Ships/` (deduped from 26 — half were exact-duplicate ships under a second naming convention).
- **Per-ship interior wiring**: `BP_Ship_*` Blueprints carry `interiorShellMesh` (soft ref) + `interiorFamily` (`EShipInteriorFamily`), and all 5 now have a `shipDataAsset` assigned (previously all `None`). Fighter → `SM_Int_Fighter_Cabin` (new v2 cabin: pilot seat + console + canopy, plus open walk space, replacing the old Cockpit/EmptyRoom pair). Corvette/Cruiser/Destroyer → `SM_Int_CommandBridge_Shell` (`CommandBridge` family). Freighter → `SM_Int_Freighter_CrewQuarters_Shell` (`CrewQuarters` family).
- **`SetRuntimeInputEnabled(bool)`**: toggles the flight IMC when entering/exiting the interior (currently a logging stub; full impl lands with the input refactor).

### 4.2 Player (`Source/Adastrea/{Public,Private}/Player/`)
- **`AAdastreaPlayerController`**: born in C++ (blueprint controller inputs retired). HUD widget creation, targeting (Tab + click), sector map (M), trade UI (B/S), ship-select (P), station info (N), **V = toggle interior** (`EnterShipInterior` / `ExitShipInterior`), docking flows.
- **`AAdastreaGameMode`**: forces the C++ player controller in **`InitGame`** (before login) — a `BeginPlay`-only override is too late and PIE fails with "Couldn't spawn player controller of class NULL". `BeginPlay` keeps the same assignment as defense-in-depth plus ship auto-spawn.
- Interactables: `IWorldInteractable` interface + `UPlayerInteractableComponent` (E-to-interact prompt, HUD wiring).

### 4.3 Stations (`Source/Adastrea/{Public,Private}/Stations/`)
Modular space stations (`ASpaceStation`) composed of interchangeable modules: **31 module classes** (DockingBay, CargoBay, Marketplace, Habitation, Reactor, Processing, Fabrication, 12+ research labs incl. Physics/Biology/Electronics, Barracks, FuelDepot, ShieldGenerator, Corridor, ...). Each module is a data-driven `ASpaceStationModule` subclass; market data lives in `UMarketDataAsset`.

### 4.4 Trading & Economy (`Source/Adastrea/{Public,Private}/Trading/`)
- `UTradeItemDataAsset` — 20+ trade items in `Content/DataAssets/Trading/Items/`
- `UMarketDataAsset` — markets with buy/sell markup + inventory
- `UPlayerTraderComponent` — credits, buy/sell against the nearest station's marketplace
- `UEconomyManager` — optional economy simulation subsystem
- HUD-driven trade screen (buy/sell toggle, quantity 1×/5×)

### 4.5 UI (`Source/Adastrea/{Public,Private}/UI/`)
- **`AAdastreaHUD`** (classic `AHUD::DrawHUD()` canvas — the *only* reliably-rendering HUD path in in-editor PIE): flight reticle, coords/speed/throttle/credits/cargo readout, off-screen target arrow, X4-style sector map, trade/ship-select/inspect screens, docking prompts.
- **`UAdastreaHUDWidget`** (UMG): self-building runtime text panel; used when no designer BP class is set.
- Widget BPs: ship status, inventory, trading, station management, main menu.

### 4.6 Input
- Legacy `BindKey` (WASDETabM... etc.) + Enhanced Input runtime actions created in C++ on each pawn (ship `MoveAction`/`LookAction`/..., avatar runtime `IA_AvatarLook_Runtime` + `IMC_AvatarRuntime`).
- Content-side `IMC_Spaceship.uasset` exists but the authoritative bindings are in C++ (`CreateInputMappingContext`/`SetupPlayerInputComponent`).

---

## 5. Content Asset Pipeline (Blender → UE)

### 5.1 Generator — `Tools/generate_adastrea_assets.py` (3,088 lines, 36 `build_*` fns)
Procedurally authors the whole 3D kit:
- **Ships**: fighter, freighter, corvette, gunship, miner + size-class carcass systems (cargo/warship/corvette/mining), modular parts (engine/cargo/weapon/sensor/reactor/habitat/drill), nav lights, hardpoints.
- **Interiors**: cockpit, **empty room** (fighter's current interior), crew quarters (hab, split into colored zone FBX), corridor, airlock, engineering bay, alien hold, command bridge.
- **Combat props**: laser/plasma bolts, missile, debris.
- **Textures**: `gen_texture_set()` produces 2K PBR sets `T_*_D/N/R/M/AO/E.png`. **`WEATHERING = False`** at top — ships are clean/shiny by user preference (flag re-enables grime).
- Emits `Assets/FBX/generated/material_map.json` (M_* → T_* mapping, 43 entries).
- **Hand-authored line**: `Tools/hand_ships.py` (`frigate`, `cutlass` → `Assets/FBX/hand_author/*.fbx`) — the premium ships the user picks for flagships.

**Known limitations (documented in skills/docs):**
- Interior textures reuse the ship-hull recipe (recolored) — a known visual weakness; undersized for furniture-scale detail.
- Procedural canopies were rejected by the user; **hand-build canopies/bridges in Blender GUI themselves**.
- `M_Interior_Eng` (engineering bay) has **no texture mapping** — renders flat grey.

### 5.2 Renderers (preview / QA loop)
**Consolidated 2026-09-13**: this used to be 18 render/preview scripts — most were one earlier session's fix for a rendering bug (headless EEVEE writing nothing, backface culling, crushed accent colors, garbled merged-OBJ UVs) that a *later* session re-solved with a new file instead of editing the old one. Removed 7 with zero remaining callers, each fully superseded by a later entry in this same table: `render_blender.py`/`render_wb.py` (early EEVEE/Workbench attempts -> superseded by `render_studio.py`), `render_ships.py` (EEVEE textured exterior -> superseded by `render_studio.py`), `render_pbr.py` (its per-part-texture logic was ported *into* `render_studio.py`, per that file's own docstring), `render_photoreal.py` (Cycles interior demo track, no callers, omitted from this table since before the consolidation), `render_assembled.py`/`render_objs.py` (matplotlib no-texture silhouette viewers, superseded by the textured software-rasterizer path). Recoverable from git history if a "photoreal demo" pass is wanted again later. The 11 below are what's left — each does a genuinely distinct job; before adding a 12th, check whether one of these can just take a flag instead.

| Script | Purpose |
|---|---|
| `render_studio.py` | Whole-ship/station studio shots (PARTS_ORDER + MOUNT_FRAC parity) — the current default exterior renderer; a deterministic numpy/PIL software rasterizer (not a Blender render engine), so no headless-EEVEE/Cycles quirks |
| `render_scene.py` | Cycles photoreal exterior (real `bpy` render, floor + HDRI-ish + 3-point rig) — kept because `render_station_modules.py` subprocess-calls it; not otherwise the default (prefer `render_studio.py` unless you specifically need Cycles lighting) |
| `render_station_modules.py` | Driver: runs `render_scene.py` once per `SM_Station_*.fbx`, copies results into `station_modules/` — does not reimplement rendering |
| `render_material_preview.py` | Close-up flat tiling plane (Cycles) — the honest way to judge a texture set on its own |
| `render_int_inside.py` | **Fixed (2026-09-13)**: camera inside rooms, multi-zone FBX composited, per-zone solid materials preserved + self-lit, `Standard` view transform + exposure 0.6. Run `blender -b --python Tools/render_int_inside.py -- <name>` |
| `render_interiors.py` | External-top-down rasterizer (for overall layout, not furniture legibility) |
| `hand_render.py` | Hand-authored ship renders (`Assets/FBX/hand_author/*`, a different source than the generator) |
| `render_hardpoints.py` | Visualizes hardpoint sockets as glowing spheres (teal=occupied, amber=free) — data/QA viz, not a look-dev tool |
| `preview_build.py` | Module-builder: assembles a ship from chosen module variants (reuses `generate_adastrea_assets.py`'s part builders via `importlib`), renders it, exports the ready-to-place FBX |
| `preview_codes.py` | Deterministic color-coded rasterizer for a `preview_build.py` output (module type -> accent color) — exists because headless EEVEE/Cycles crushed accent colors to grey; not merged into `preview_build.py` itself, run as a second pass |
| `module_picker.py` | Renders each module *variant* as a standalone color-coded thumbnail, tiled into a picker-sheet grid (builder UI icons) — similar rasterizer to `preview_codes.py` but for the catalog, not a single build; a candidate to share code with `preview_codes.py` if touched again |

### 5.3 Import into UE
- FBX → `.obj` via `Tools/fbx2obj.py`; OBJ → plugin meshes via `Tools/ue_import_*_one-shots.py` (import inside the editor via `-ExecutePythonScript` or MCP).
- **Import target**: `/AdastreaShips/Meshes/...` (NOT `/Game/Assets/...`) — `BP_Ship_*` already reference the plugin meshes; importing elsewhere creates unused duplicates.
- **Materials for ships**: C++ `ApplyShipHullMaterial()` maps hull by ship class and calls `SetMaterial(0, M_*_Hull)` at BeginPlay. Re-importing/recompiling the shared `M_*_Hull` asset auto-updates ships (no BP edit).
- **Interior materials**: `ApplyInteriorMaterials()` re-points exported slot names (e.g. `M_Int_Shell`) to the plugin's 12 `M_Int_*` materials. The exported meshes ship with a placeholder grid material on every slot — slot **name** is the mapping key.

### 5.3b Sourced kitbash assets (BlenderKit) — alternative to procedural generation
- `Tools/blenderkit_fetch.py` — headless (stdlib-only, no bpy) search/download against the public BlenderKit REST API. Reads `BLENDERKIT_API_KEY` from a gitignored `.env` (template: `.env.template`). **Only ever fetches `royalty_free`/`cc_zero` licensed assets** — refuses anything else. Downloads raw `.blend`/`.gltf` sources to `Assets/BlenderKit/library/` (gitignored, re-fetchable) and records attribution (name, author, license, source URL) in the tracked `Assets/BlenderKit/manifest.json`.
- `Tools/blenderkit_import.py` — headless Blender script; appends the full object hierarchy from a downloaded source file (preserving parent-Empty transforms via `CLEAR_KEEP_TRANSFORM`, since kitbash packs commonly bake real-world size onto a parent, not the mesh), then exports via the *exact* same FBX settings as `generate_adastrea_assets.py`'s `export_fbx()`. Output lands in `Assets/FBX/generated/kitbash/` (kept separate from procedural output so sourced-vs-generated is obvious at a glance); `Tools/fbx2obj.py` now globs both dirs so it flows into the same QA gate as everything else.
- **Critical unit gotcha (verified empirically, don't skip this if extending the tool)**: this pipeline's FBX export round-trips **1 raw Blender unit = 1 cm** in the final QA/UE space (confirmed by exporting a 1×1×1-unit test cube through the real path and measuring it post-`fbx2obj.py`: comes out exactly 1×1×1) — despite `scale_length` nominally meaning meters-per-unit. BlenderKit assets are authored in real-world meters, so `blenderkit_import.py` applies a **100x default `--scale`** correction. Without it, a real 10cm panel silently exports as a 0.1cm sliver and fails QA's `bounds` check (this exact failure is what caught the bug during setup — `SM_Prop_SciFiControlPanel` initially QA'd as `size 0.1 x 0.1 x 0.1 cm`, fixed by the scale default).
- Usage: `python Tools/blenderkit_fetch.py search "sci-fi console"` → `... fetch "sci-fi console" --index 0` → `blender -b --python Tools/blenderkit_import.py -- <path from fetch output> SM_Prop_Console`.

### 5.4 QA gates
- `pytest` — **128 passing** (run `python -m pytest`).
- `Tools/qa_assets.py` — numeric per-asset checks (tri budget, degenerate faces, bounds cm, symmetry, UV presence, non-empty). `SM_StarDome_Dense` was retopo'd 2026-09-23 to 960 tris (stars live in `T_Starfield.png`; `Tools/gen_dense_dome.py`), so its old tri-budget failure is gone. Remaining known failures are the Nav_NavRed/NavGreen light pairs (intentionally asymmetric), the sourced `SM_Int_Corvette_Bridge_Console` (3 degenerate faces) and `SM_Station_Mining_01_Sensor`. Interior kit parts (`SM_Int_*`) are exempt from the X-mirror check (rooms are front/back asymmetric by contract).
- Vision-verify renders with `vision_analyze` before claiming visual quality.

---

## 6. Testing

| File | Covers |
|---|---|
| `test_all_interiors.py` | Interior meshes exist in plugin path |
| `test_interiors_walk_scales.py` | Floor altitude, walk extents, exit trigger, companion parts, entry point contracts (order-randomization-sensitive) |
| `test_trading_integration.py` | Buy/sell flows |
| `test_station_module_catalog.py`, `test_schema_validator.py` | Station modules + data schemas |
| `test_crafting_tree.py`, `test_crafting_recipes.py` | Crafting/build-tree data |
| `test_procedural_generators.py`, `test_blueprint_generator.py` | Generator smoke tests |
| `test_ue_build_tools.py`, `test_unreal_connection.py`, `TestUnrealMCP.py` | UE/MCP harness checks (require editor) |

Run one file: `python -m pytest tests/<file>.py`. **Randomization note**: a couple of `test_interiors_walk_scales` tests are order-sensitive — run with `-p no:randomly` if they fail in isolation.

---

## 7. Editor / Runtime Workflows

### 7.1 Launch & PIE
1. Launch: `UnrealEditor.exe C:\Users\akuma\Adastrea\Adastrea.uproject` (background; ~1 min on this iGPU box).
2. Handle dialogs: "Restore Packages" → **Skip Restore** (clean load).
3. Start PIE via MCP: `python ue_mcp.py call EditorToolset.EditorAppToolset.StartPIE '{"options": {"bSimulate": false, "playMode": "PlayMode_InViewPort", "warmupSeconds": 2}}'`
4. Check log: `SHIP POSSESSED: BP_Ship_Fighter_C_0 by controller AdastreaPlayerController_0` confirms clean spawn.
5. **V** toggles interior; **E** interacts; **P** ship-select; **M** map.

### 7.2 Editing Blueprint properties (MCP)
`ObjectTools.get/set_properties` on the **CDO path**, e.g.
`/Game/Blueprints/Ships/BP_Ship_Fighter.Default__BP_Ship_Fighter_C`.
- `set_properties` takes `values` as a **JSON string** (`{"prop": {"refPath": "..."}}`).
- Then `AssetTools.save_assets` with the asset path; verify by readback + on-disk timestamp.
- Component-level graph edits (`:ShipMesh` etc.) are **unreliable outside the GUI** — prefer C++ wiring for materials.

### 7.3 If PIE fails
| Symptom | Cause / fix |
|---|---|
| "Couldn't spawn player controller of class NULL" | GameMode override run too late — the C++ force lives in `InitGame()` now (committed). Rebuild. |
| "SpawnActor ... collision" | Backdrop/dome covers all; set `AlwaysSpawn` on the pawn CDO. |
| "ship doesn't move but throttle works" | Drive via `AddMovementInput`, not `Velocity` writes (UFloatingPawnMovement overrides). |
| Editor dialogs block automation | Modal dialogs capture input; use MCP for actions, `computer_use` foreground only when needed. |

---

## 8. Multi-Agent Development

- **Two standing roles** (AGENT_BOARD.md): `main` (HUD/controls/gameplay/input) and `assets` (Blender/generator/plugin 3D). A craft/build-tree third agent has been added as a cron job in some sessions.
- **Coordination protocol**: append dated `FROM → TO` posts to `AGENT_BOARD.md`; `Tools/watch_agent_board.py` surfaces non-main posts; check for posts addressed to you before big tasks.
- **Git hygiene**: close the editor before C++ builds; never blanket `git checkout -- Content/` while the editor has files open (uasset lock errors); stash only what you intend; commit + push frequently so work is never lost.
- **Hands-off**: don't edit another role's in-flight files (check `git status` first).

---

## 9. Known Issues & Gotchas (living list)

1. **Interior textures are the ship-hull recipe reused** — reads "wallpaper" on furniture at scale; needs bespoke interior language (priority item).
2. ~~**`SM_StarDome_Dense`** tri budget exceed~~ — fixed 2026-09-23 (960-tri UV sphere, same UVs/name; needs a straight reimport in UE).
3. **`M_Interior_Eng`** has no texture mapping → engineering bay flat grey.
4. ~~`SetRuntimeInputEnabled` is a logging stub~~ — fixed 2026-09-15: it now really adds/removes the ship's runtime input mapping context.
5. **Bridge viewport zone** doesn't read as glass in renders (material polish item).
6. ~~Test-order randomization can make 2 interior contract tests fail when run alone~~ — misdiagnosed. `tests/test_interiors_walk_scales.py` has no shared/mutable state between tests (every test reads C++ source fresh); the real cause was 5 tests asserting against the pre-Enhanced-Input-rewrite avatar/interior confinement code (`InteriorFloorAltitude`, `GetLocalHalfExtents`, the manual per-tick clamp) after that code was replaced by real wall/floor collision + swept movement. Fixed 2026-09-15 by rewriting the stale assertions to match the current contract; confirmed passing both together and each in full isolation (pytest-randomly isn't even installed in this env).
7. **README** still states UE 5.6 / older phase status — docs lag; the repo is current.
8. **Interior preview tooling** (`render_int_inside.py`) is fixed and the correct way to judge rooms — don't trust `render_interiors.py` for furniture legibility.

---

## 10. Where To Find Things Fast

| Question | Where |
|---|---|
| Ship stats/classes | `docs/11-TECHNICAL_SPECS/SHIP_MOBILITY_STATS.md`, `Content/Data/ShipClasses.json`, `USpaceshipDataAsset` |
| Material↔texture map | `Assets/FBX/generated/material_map.json` + `docs/11-TECHNICAL_SPECS/UE_MATERIAL_REFERENCE.md` |
| What buttons do | C++ `SetupPlayerInputComponent` (ship/avatar/controller), not content IMC |
| Interior parts list per family | `SpaceshipInterior::MountInteriorParts` |
| Economy design | `docs/11-TECHNICAL_SPECS/ECONOMY_SYSTEM.md`, `TRADING_SYSTEM.md` |
| Station modules | `Source/Adastrea/Private/Stations/` + `docs/11-TECHNICAL_SPECS/STATION_MODULES_PLAN.md` |
| Crafting | `docs/11-TECHNICAL_SPECS/CRAFTING_TREE.md` + generators |
| Reference ship analysis | `docs/11-TECHNICAL_SPECS/REFERENCE_SHIP_ANALYSIS.md` |
| How agents coordinate | `AGENT_BOARD.md` |
| PIE/debug recipes | `docs/11-TECHNICAL_SPECS/` + `unreal-engine-mcp` skill (Hermes) |

---

*Keep this file updated when the repo changes materially (new system, removed system, engine change). It is a map, not a substitute for reading the code.*