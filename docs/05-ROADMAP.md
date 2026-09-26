# Project Roadmap

**Current status and open work | Updated: September 26, 2026 (main as of PR #505)**

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
- Data-driven ship stats (`USpaceshipDataAsset`) across 18 deduped ship DataAssets
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

**Stations**
- In-game Station Editor (X4-style plan mode): place, rotate and connect modules on a grid. It
  works standalone (spawns a fresh station if none is nearby)
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

**HUD, targeting and debug**
- Neon cockpit flight HUD (#505): heading tape, boresight and flight-path marker, shield/hull
  bars, speed/throttle, restyled dock prompt and mining panel. The old telemetry panel is still
  there behind `bCyberpunkFlightHUD = false`
- Targeting of ships and asteroids, not just stations, with cycle keys: `]` / `[` next/previous,
  `Y` nearest, `Z` clear (#504)
- F10 fleet monitor (#502): a debug list of every ship and station in the level, with pilot,
  cargo, objective, docking and market details. Also available as
  `adastrea.FleetMonitor [expand|collapse]`
- `DebugToggleInterior` console command for scripted interior walkthroughs (#493)
- Player spawn is pushed clear of station hulls (#500)

**Art and presentation**
- Interior texture library (16 sets) and the `M_IntSurface_Oriented` master with ship-kit and
  station material instances (#493). This replaces the "hull material as wallpaper" look, and
  `MI_Interior_Eng` fixes the flat-grey engineering bay
- Translucent bridge viewport glass (#493, #495) and emissive interior lights
- Deep-space PostProcessVolume and a regenerated starfield. `SM_StarDome_Dense` was rebuilt at
  960 tris (under its 1,000 budget) and reimported (#493)
- Unique-UV baked hull sets for the Corvette (prototype) and Battleship (#493). They're built but
  not yet assigned on the ship Blueprints (see below)

**Engine and tooling**
- UE 5.8, SM5 (no Lumen/VSM, iGPU-friendly)
- 129 pytest tests passing (`python -m pytest -q -p no:randomly`)
- Procedural Blender asset pipeline (`Tools/generate_adastrea_assets.py`, station/interior
  generators) plus a licensed BlenderKit sourcing pipeline for hero/accent props
- Multi-agent coordination protocol (`AGENT_BOARD.md`)

---

## 🔨 Open work

- **Modules-B sockets import with roll 180.** The lab/turret/shield `SOCKET_*` empties from
  `Tools/generate_station_lab_defence_modules.py` come into UE rolled 180°. The turret doesn't use
  socket rotation, so this doesn't block anything yet, but it will once something mounts on those
  sockets. A fix at the source is in progress.
- **Unique baked hulls are built but not on the ship Blueprints.** The Corvette and Battleship
  unique-UV hull meshes and materials exist, but the Corvette BP and `BP_Battleship` still use the
  shared hulls.
- **Save/load doesn't cover the newer systems.** `UAdastreaSaveGame` stores credits, location,
  generic inventory, ships, quests and market prices. It doesn't store mining cargo, player-built
  stations or other state from the current live loop.
- **`CraftingManager` is unbuilt.** The recipe tree exists, but no C++ system runs crafting.
- **No packaged build yet.** Everything so far has been verified in the editor/PIE only.
- **What to build next is an open product decision.** Combat is the most likely candidate, since
  targeting, the defence HUD and turret modules are already in place.

---

## ⚠️ Known issues and polish backlog

- Shields have no runtime pool. The HUD shield bar shows the DataAsset's rated `ShieldStrength`
  as full
- Two interior-contract pytest tests are order-sensitive (use `-p no:randomly`)
- Docking-type variety (S/M/L/XL bays, X4-style) needs new module classes and content. It's on
  hold pending direction, since it affects ship-side sizing

---

## 🚫 Deferred systems (code exists, not wired into the live loop)

Large parts of the codebase are implemented but deliberately disabled. This is a real backlog,
not dead code:

| System | Status |
|--------|--------|
| Combat | Complete, disabled (likely next, see above) |
| Navigation/Autopilot | Complete, disabled |
| Quest System | Complete, disabled |
| Faction Diplomacy | Complete, disabled |
| Personnel/Crew | Complete, disabled |
| Advanced AI | Complete, disabled (the live AI traders/miners are separate, lightweight controllers) |
| Exploration/Scanning | Partial |
| Way Network | Complete, disabled |
| Full Save/Load | Partial (doesn't cover mining cargo or player-built stations) |
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
| Tests | 129 pytest tests (`python -m pytest -q -p no:randomly`) plus UE automation tests (`Adastrea.StationEditor.*`) |
| Packaging | Not done yet |

---

## 📞 Where to find current status

- **This page**: the project status summary. It's updated when the shape of the project changes,
  not on every commit
- **Live, day to day**: `AGENT_BOARD.md`, the append-only log between the agent sessions working
  this repo
- **Deep technical snapshot**: [00-KNOWLEDGE_BASE.md](00-KNOWLEDGE_BASE.md)

---

*Back to [INDEX.md](INDEX.md) | Next: [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md)*
