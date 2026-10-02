# Project Roadmap

**Current status and open work | Updated: September 28, 2026 (main as of PR #520, plus `feat/station-cores`)**

---

## What Adastrea is right now

An **open-world space trade sim** built on **Unreal Engine 5.8**. Mittenzx develops it solo,
with several Claude Code sessions working the codebase in parallel. The playable loop: fly a ship,
dock at a station, trade or mine, and walk the ship's or station's interior on foot. The sector
has AI traders and miners working it at the same time.

This page is the single source of truth for project status. The root `ROADMAP.md` and
`docs/03-CURRENT_STATUS.md` only point here now. Live, day-to-day coordination between agent
sessions happens in `AGENT_BOARD.md`.

> Older versions of this roadmap described a "12-week MVP sprint, Week 12 of 12, March 2026
> deadline." That framing no longer applies. The project moved to open-ended multi-agent
> development and has gone well past that scope. Nothing below is a date commitment.

---

## ✅ What's built and working

**Ships**
- Data-driven ship stats (`USpaceshipDataAsset`) across a 17-ship DataAsset roster
- Distinct hulls per class, including Cruiser and CommandXL (#486, #488). The ship mesh now faces
  actor forward (#503)
- Data-driven interior family selection (`EShipInteriorFamily`). The 5 core classes (Fighter,
  Corvette, Cruiser, Destroyer, Freighter) are wired to a ship DataAsset and interior family
- Walkable ship interiors: floor/wall collision, entry/exit triggers, per-family companion parts,
  avatar torch (F), 1st/3rd-person camera toggle (T). Interiors are walked in a pocket below the
  ship, so docked ships don't overlap station collision (#493)
- Capital bridges: Battleship and CommandXL bridge interiors (#493). The CommandXL bridge has
  exterior-grade geometry and a unique baked PBR set (#499)
- X4-style flight model (flight assist, throttle, boost, travel mode)
- Fighter redesigned as a Newtonian X-frame strike fighter (no wings or tail)

**Stations**
- In-game Station Editor (X4-style plan mode, `G`): place, rotate and connect modules on a grid.
  It works standalone (spawns a fresh station if none is nearby)
- 3D plan camera: opening the editor blends from the ship to an orbit camera framed on the
  station (RMB orbit, MMB/WASD pan, wheel zoom, Q/E turn, F reframe) and back on close. The UI is
  a light native overlay (`UStationEditorWidgetCpp`): stats, close, construction queue and a
  group-coloured module palette. The old two-panel `WBP_StationEditor` layout is retired
- Building is paid from the player's trader credits. New games start with 150,000 credits so a
  first outpost is affordable (#515, #516)
- Unique core per station archetype (`AStationCoreModule`: TradeHub, Agricultural, Industrial,
  Research, Luxury, BlackMarket), each with its own hull and connection collars. Cores can't be
  built, removed, moved or rotated in the editor
- Level stations are built from `Content/Data/LevelStationLayouts.json` using the editor's own
  attach rules (`Tools/station_layouts.py` validates, `Tools/build_level_stations.py` places).
  Collision/adjacency/attach spacing uses rotated footprint boxes
- Footprint-aware collision/adjacency and face-restricted connections (#485), including the
  tier-4 lab footprints
- Docking-access and production-chain validation warnings. Station designs serialize to a
  shareable string and back
- Station Editor correctness pass (#501): undo/redo, cancel and refunds charge and return exactly
  what they should, a construction queue, a power gate, and no split stations. Editor hotkeys
  (R/Shift+R, Esc, Ctrl+Z/Y, Delete, Shift+Click). Manager notifications show in the editor
  widget. Covered by `Adastrea.StationEditor.*` automation tests
- **Dedicated meshes for all 27 station module types** (#493), with UCX collision, a per-class
  mesh table in `SpaceStationModule.cpp` that falls back to the 4 shared shells, domain material
  instances, and a separate `TurretHead` on the turret
- Docking: a range gate, a HUD dock prompt, and a station services menu on docking (#486)
- Walkable station interior with Walk / Maintenance / Habitation rooms (#490)
- Lab/turret/shield module sockets import at identity rotation, and their UCX collision hulls
  now import (#508, #510)
- Player-built docking bays get native docking points (#514)

**Economy, mining and AI**
- Trading loop: buy/sell against station marketplaces, with `UPlayerTraderComponent` for credits
  and cargo
- Mining loop (#487): asteroid DataAssets and types, harvestable asteroids, `UMiningLaserComponent`,
  `AAsteroidField`, a mining ship, sell-all, and the `MiningTest` map. The player Fighter carries a
  starter mining laser (#497)
- Mining HUD (#496): target / laser / hold panel
- AI traders (`AAIPilotController`, `AAIShipPopulator`, #490) and AI miners that mine until full,
  then sell at the best-paying station (`AAIMinerController`, #491)
- TestLevel has an asteroid belt plus AI miners and traders working it (#497)
- Crafting tree: 68 recipes defined (`CraftingTree.json`) that feed station-module build costs

**Save/load**
- Save v2 (#509): ship class and DataAsset, trader credits, cargo (including mined ore), docked
  station and bay, and player-built stations/modules. Console: `adastrea.SaveGame` /
  `adastrea.LoadGame`. v1 saves still load
- F5 / F9 quicksave and quickload, with feedback on the canvas HUD (#515)

**Audio**
- Event-ID audio API: `UAudioCatalogDataAsset` + `UAudioCatalogSubsystem`, with gameplay moments
  hooked to catalog events (EventSFX) and pause-menu UI sounds routed through
  `UAudioEventLibrary` (#517, #519, #520)
- Per-ship engine voice (`UShipEngineAudioComponent`), size-mapped across the ship roster (#518)
- Mix subsystem (`UAudioMixSubsystem`): interior muffle, menu duck, ambience beds near stations,
  and pause-menu volume sliders (#520)
- Sounds are generated by a script in `Tools/audio/` and imported with
  `Tools/import_audio_assets.py`; `/Game/Audio` is always cooked

**HUD, targeting and debug**
- Neon cockpit flight HUD (#505): heading tape, boresight and flight-path marker, shield/hull
  bars, speed/throttle, restyled dock prompt and mining panel. The old telemetry panel is still
  there behind `bCyberpunkFlightHUD = false`
- Targeting of ships and asteroids, not just stations, with cycle keys: `]` / `[` next/previous,
  `Y` nearest, `Z` clear (#504)
- Fleet monitor (#502): a debug list of every ship and station in the level, with pilot,
  cargo, objective, docking and market details. Opened with
  `adastrea.FleetMonitor [expand|collapse]` (F10 now opens the pause menu)
- `DebugToggleInterior` console command for scripted interior walkthroughs (#493)
- Player spawn is pushed clear of station hulls (#500)

**Art and presentation**
- Interior texture library (16 sets) and the `M_IntSurface_Oriented` master with ship-kit and
  station material instances (#493). This replaces the "hull material as wallpaper" look, and
  `MI_Interior_Eng` fixes the flat-grey engineering bay
- Translucent bridge viewport glass (#493, #495) and emissive interior lights
- Deep-space PostProcessVolume and a regenerated starfield. `SM_StarDome_Dense` was rebuilt at
  960 tris (under its 1,000 budget) and reimported (#493)
- Unique-UV baked hull sets for the Corvette (prototype) and Battleship (#493), now flown on
  `BP_Ship_Corvette` and `BP_Battleship` via `ASpaceship::HullMaterialOverride` (#510)

**Engine and tooling**
- UE 5.8, SM5 (no Lumen/VSM, iGPU-friendly)
- 592 pytest tests (`python -m pytest -q -p no:randomly`); 591 pass, see known issues
- Procedural Blender asset pipeline (`Tools/generate_adastrea_assets.py`, station/interior
  generators) plus a licensed BlenderKit sourcing pipeline for hero/accent props
- Multi-agent coordination protocol (`AGENT_BOARD.md`)
- Win64 packaging (#513): `Tools\package_win64.bat` runs BuildCookRun, and the packaged
  `Adastrea.exe` boots into TestLevel with the ship, markets and AI miners working. See
  [09-SETUP_GUIDES/PACKAGING.md](09-SETUP_GUIDES/PACKAGING.md)

---

## 🔨 Open work

- **`CraftingManager` is unbuilt.** The recipe tree exists, but no C++ system runs crafting.
- **Packaged build needs a proper play-through.** It builds and boots, but visuals and
  performance haven't been checked interactively, and the HUD shows object names instead of
  station/ship names. See the known-issues list in
  [PACKAGING.md](09-SETUP_GUIDES/PACKAGING.md#known-issues--remaining-blockers)
- **Combat is being rebuilt (lean, X4-style).** Milestone 1 is done: fixed forward guns (hold LMB),
  bolts with hit flashes, shields that soak damage and recharge, armor, a lead pip and hit marker,
  and shield/hull bars on the locked target. Test it in `/Game/Maps/CombatTest` (training-target
  dummies respawn 5 s after being destroyed; `LockTarget <name>` locks from the console).
  Milestone 2 is done too: hostile fighters (`AHostileFighterController`) make attack runs on the
  player, firing on the lead point, then break off and come round again. In CombatTest a director
  sends waves of 2 and repairs the player 5 s after they're disabled (console: `HostileWaves 0/1`,
  `SpawnHostiles N`, `ClearHostiles`). Milestone 3: ships are never blown up. At zero hull every
  ship (player, hostiles, traders, dummies) is disabled and becomes a wreck: engines, guns, lights
  and windows go dark, a scorch overlay goes on the hull, and it drifts to a stop while tumbling.
  AI pilots abandon wrecks; `ClearWrecks` removes them. Milestone 4: station turrets
  (`ATurretModule`) pick the nearest live hostile within 200 m, slew their heads onto the lead
  point and fire alternate barrels. Teams (`CombatTeam`): the player, traders and turrets are
  Civil and never hit each other; hostiles are Hostile; dummies are Neutral. CombatTest has a
  4-turret station 120 m from the spawn. For the time being station shields are
  incomprehensibly strong: stations and their modules take no combat damage (bolts just stop).
  Disabled player (`UPlayerRescueSubsystem`): a tow drone from the nearest friendly station
  clamps onto the wreck and hauls it in. The pilot ejects in an escape pod to that station, or to
  a nearer trader in flight, which ferries them there. The ship is repaired free and re-boarded,
  then docks normally. Sector security and patrols (`UDistressSubsystem`, `APatrolController`):
  a Civil ship shot by a hostile raises a distress call. The sector's security (the `security`
  field in Galaxy.json) decides the answer: High sends up to 3 patrols at once, Medium 2 after
  15 s, Low 1 after 40 s, None nobody. Roster ships with the Patrol role fly between stations,
  attack hostiles within 250 m, and answer calls. Test with `adastrea.Security 0-3`,
  `SpawnPatrols [N]` and `Tools/pie_distress_test.py`. Hostiles and patrols share their attack
  runs (`ACombatPilotController`). What combat is for (goal-driven pirates, stolen cargo,
  boarding, capture) is designed in `11-TECHNICAL_SPECS/PIRACY_AND_LAW.md`; step 1 of its build
  order is done. Next: its step 2 (pirate organisations with needs), plus combat audio, bolt and
  impact-flash look, turrets on capital ships. The old `Combat/` module archived in `e155151d`
  is reference only.

---

## ⚠️ Known issues and polish backlog

- Two interior-contract pytest tests are order-sensitive (use `-p no:randomly`)
- `test_event_sfx_hooks.py::TestHookPlacement::test_editor_widget_hooks` fails: it still checks
  `ModuleListItemWidget.cpp`, which was removed with the old Station Editor panels. The UI hover
  hooks need to move to (and the test to check) `UStationEditorWidgetCpp`
- Orphaned DataAssets whose C++ classes were removed (`DA_Weapon_*`, `DA_Quest_*`, etc.) make the
  cooker warn. They should be deleted in the editor
- Docking-type variety (S/M/L/XL bays, X4-style) needs new module classes and content. It's on
  hold pending direction, since it affects ship-side sizing

---

## 🚫 Deferred systems (code exists, not wired into the live loop)

Large parts of the codebase are implemented but deliberately disabled. This is a real backlog,
not dead code:

| System | Status |
|--------|--------|
| Combat | Being rebuilt: guns, shields, damage, hostile AI fighters, wrecks, station turrets, player rescue, and patrols answering distress calls by sector security (see Open work) |
| Navigation/Autopilot | Complete, disabled |
| Quest System | Complete, disabled |
| Faction Diplomacy | Complete, disabled |
| Personnel/Crew | Complete, disabled |
| Advanced AI | Complete, disabled (the live AI traders/miners are separate, lightweight controllers) |
| Exploration/Scanning | Partial |
| Way Network | Complete, disabled |
| Full Save/Load | Live loop covered by save v2 (#509); not re-checked against every deferred system |
| Multiplayer | Planned only |
| Crafting (build execution) | Recipe tree done. C++ `CraftingManager` not built |

Choosing which of these turns "fly, dock, trade, mine, walk around" into a full game, rather than
building all of them, is a product decision, not an engineering one.

---

## 🔧 Engine and infrastructure

| Item | Current |
|------|---------|
| Unreal Engine | 5.8 (`C:\Program Files\Epic Games\UE_5.8`) |
| Build | `build_with_ue_tools.bat Development Win64` (UBT via dotnet + VS2022) |
| MCP | Built-in UE MCP server on `http://127.0.0.1:8000/mcp`, used by agent sessions for live editor inspection and edits |
| CI/CD | GitHub Actions (dead workflows removed in #494) |
| Tests | 592 pytest tests (`python -m pytest -q -p no:randomly`) plus UE automation tests (`Adastrea.StationEditor.*`) |
| Packaging | Win64 Development package builds and boots (`Tools\package_win64.bat`, see [PACKAGING.md](09-SETUP_GUIDES/PACKAGING.md)) |

---

## 📞 Where to find current status

- **This page**: the project status summary. It's updated when the shape of the project changes,
  not on every commit
- **Live, day to day**: `AGENT_BOARD.md`, the append-only log between the agent sessions working
  this repo
- **Deep technical snapshot**: [00-KNOWLEDGE_BASE.md](00-KNOWLEDGE_BASE.md)

---

*Back to [INDEX.md](INDEX.md) | Next: [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md)*
