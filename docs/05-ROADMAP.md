# Project Roadmap

**Trade Simulator MVP → Post-MVP | Updated: September 16, 2026**

---

## What Adastrea actually is right now

An **open-world space trade sim** built on **Unreal Engine 5.8**, developed solo/agent-assisted by
Mittenzx with multiple Claude Code sessions working the codebase in parallel. The playable thrust:
fly a ship, dock at a station, buy/sell cargo, and walk the ship's interior on foot. This is a living
document — the authoritative moment-to-moment status is the cross-agent squad log
(`AGENT_BOARD.md` + the shared "Squad Status" quest-log artifact the team keeps updated), not a
fixed week-by-week schedule. This page is the slower-moving summary.

> Superseded framing, for the record: earlier versions of this roadmap described a literal
> "12-week MVP sprint, Week 12 of 12, March 2026 deadline." That sprint framing is gone — the
> project moved to open-ended multi-agent development and has shipped well past that scope
> (a full Station Editor, walkable ship interiors, a crafting tree, procedural + sourced asset
> pipelines). Nothing below is a date commitment; it's a status snapshot.

---

## ✅ What's actually built and working

**Ships**
- Data-driven ship stats (`USpaceshipDataAsset`) across 18 deduped ship DataAssets (down from 26
  near-duplicates)
- Data-driven interior family selection (`EShipInteriorFamily`) replacing string-matched mesh
  detection; all 5 core ship classes (Fighter, Corvette, Cruiser, Destroyer, Freighter) wired to a
  `shipDataAsset` + interior family
- Walkable ship interiors: floor/wall collision, entry/exit triggers, per-family companion-part
  mounting, avatar torch (F), 1st/3rd-person camera toggle (T)
- X4-style flight model (flight assist, throttle, boost, travel mode)

**Stations**
- In-game Station Editor (X4-style plan mode): place/rotate/connect modules on a grid, standalone
  (spawns a fresh station if none nearby), reachable without prior Blueprint setup
- Footprint-aware collision/adjacency (real per-module grid size, not one uniform radius),
  face-restricted connections (e.g. `SolarArrayModule` needs an actual facing connection)
- Docking-access and production-chain validation warnings
- Blueprint save/load: a built station design serializes to a shareable string and back
- Honest module upgrades (`UpgradeModule()` now applies a real stat bonus instead of charging
  credits for nothing)
- 4 shared shell geometries (Standard/Large/ConnectorThin/SolarArray) built and wired to all 20
  module types with footprint data

**Economy / crafting**
- Trading loop: buy/sell against station marketplaces, `UPlayerTraderComponent` credits/cargo
- Crafting tree: 68 recipes defined (`CraftingTree.json`) feeding the station-module build costs

**Space presentation**
- Deep-space PostProcessVolume (was `SkyAtmosphere`, wrong for vacuum) tuned for low ambient
  exposure; regenerated starfield with a Milky Way band and dimmer stars; star dome follows the
  camera instead of parallaxing oddly

**Engine/tooling**
- UE 5.8, SM5 (no Lumen/VSM, iGPU-friendly)
- 128 pytest tests passing
- Procedural Blender asset pipeline (`Tools/generate_adastrea_assets.py`) plus a licensed
  BlenderKit sourcing pipeline (`Tools/blenderkit_fetch.py` / `blenderkit_import.py`) for
  hero/accent props
- Multi-agent coordination protocol (`AGENT_BOARD.md`) for the several Claude Code sessions
  routinely working this repo in parallel

---

## 🔨 Active work

**Biggest open gap — station module art**: 27 module types exist in data/code; almost none have
dedicated meshes yet (most render one of 4 generic shells). In progress, in dock→trade priority
order: 3 remaining per-family texture sets (Connector/Utility/Defence — Module and Habitation
families are already done) → hero accent props for DockingBay/DockingPort/CargoBay/Habitation →
Marketplace → Reactor/SolarArray/FuelDepot → research labs. Turret/ShieldGenerator dressing is
intentionally last — combat is a deferred system (see below). The 7 tier-4 advanced labs
(`ProjectileWeaponsLab`, `BeamWeaponsLab`, etc.) don't even have placement/grid data yet in
`StationModuleBuilderData.json` — that's a design task blocking any art work on them.

**Ship roster completion**: Cruiser is still wearing the Gunship's hull, CommandXL has neither an
exterior nor interior yet, Battleship needs an interior. Pattern established: art team builds the
mesh, whoever's on ship-systems wires the Blueprint property once it exists.

**Station Editor polish**: wire the notification system (built, `OnNotificationAdded`/
`GetNotifications()`) into the actual `WBP_StationEditor` widget UI; docking-type variety
(S/M/L/XL bays, X4-style) needs new module classes/content — on hold pending direction since it
crosses into ship-side sizing.

---

## ⚠️ Known issues & polish backlog

- Interior textures reuse the ship-hull material recipe — reads as "wallpaper" at furniture scale;
  needs its own bespoke interior texture language
- ~~`SM_StarDome_Dense` exceeds its tri budget (16,128 vs 1,000)~~ — fixed 2026-09-23: rebuilt as a 960-tri UV sphere (same UVs/name), pending UE reimport
- `M_Interior_Eng` has no texture mapping — engineering bay renders flat grey
- Bridge viewport zone doesn't read as glass in renders
- Two interior-contract pytest tests are order-sensitive (`-p no:randomly` needed in isolation)
- Root `README.md` had stale UE-5.6 / old-MVP-phase framing — being corrected alongside this doc

---

## 🚫 Deferred systems (code exists, not wired into the live loop)

Large chunks of the codebase are implemented but intentionally disabled — this is a real backlog,
not dead code, and represents the "next era" once ships/stations/interiors feel finished:

| System | Status |
|--------|--------|
| Combat | Complete, disabled |
| Navigation/Autopilot | Complete, disabled |
| Quest System | Complete, disabled |
| Faction Diplomacy | Complete, disabled |
| Personnel/Crew | Complete, disabled |
| Advanced AI | Complete, disabled |
| Exploration/Scanning | Partial |
| Way Network | Complete, disabled |
| Full Save/Load | Partial |
| Multiplayer | Planned only |
| Crafting (build execution) | Recipe tree done; C++ `CraftingManager` interface still not built |

Deciding which of these actually turns "fly, dock, trade, walk around" into a full game — rather
than building all of them — is an open product question, not an engineering one.

---

## 🔧 Engine & Infrastructure

| Item | Current |
|------|---------|
| Unreal Engine | 5.8 (`C:\Program Files\Epic Games\UE_5.8`) |
| Build | `build_with_ue_tools.bat Development Win64` (UBT via dotnet + VS2022) |
| MCP | Built-in UE MCP server on `http://127.0.0.1:8000/mcp`, used by agent sessions for
  live editor inspection/edits |
| CI/CD | GitHub Actions |
| Tests | 128 pytest tests, `python -m pytest` |

---

## 📞 Where to find current status

- **Live, minute-to-minute**: `AGENT_BOARD.md` (append-only async log between the agent sessions
  working this repo) and the shared Squad Status quest-log artifact
- **Deep technical snapshot**: `docs/00-KNOWLEDGE_BASE.md`
- **This page**: a slower-moving summary, updated when the shape of the project changes materially
  rather than on every commit

---

*Back to [INDEX.md](INDEX.md) | Next: [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md)*
