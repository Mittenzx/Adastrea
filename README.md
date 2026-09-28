# Adastrea

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-blue.svg)](https://www.unrealengine.com/)
[![Code Quality](https://github.com/Mittenzx/Adastrea/actions/workflows/code-quality.yml/badge.svg)](https://github.com/Mittenzx/Adastrea/actions/workflows/code-quality.yml)

An open-world space trade sim in the spirit of X4, built on **Unreal Engine 5.8**. Fly a ship,
dock at stations, trade and mine, build your own outposts, and get up out of the pilot's seat to
walk around your ship or a station on foot. AI traders and miners work the same sector alongside
you.

Mittenzx develops Adastrea solo, with several Claude Code sessions working the codebase in
parallel. Development is open-ended rather than tied to a fixed milestone plan.

> **Current status lives in [docs/05-ROADMAP.md](docs/05-ROADMAP.md).** This README describes the
> project at a high level; the roadmap is where "what's done / what's next" is kept up to date.

---

## What you can do in it today

**Fly.** X4-style flight model with flight assist, throttle, boost and travel mode. A roster of 17
ships from the redesigned strike fighter up to capital hulls (Cruiser, Battleship, CommandXL), each
with its own data-driven stats (`USpaceshipDataAsset`) and engine voice.

**Dock, trade and mine.** Station marketplaces with buy/sell pricing, a station services menu on
docking, and a full mining loop: harvestable asteroid fields, a mining laser (the starter Fighter
carries one), a mining HUD and sell-all at stations.

**Build stations.** An in-game, X4-style Station Editor with a 3D orbit plan camera: place, rotate
and connect modules on a grid, with footprint-aware collision, directional connection faces,
docking and production-chain warnings, undo/redo, a construction queue and a power gate. Building
is paid from your trader credits. Every station has a unique, immovable core for its archetype
(Trade Hub, Agricultural, Industrial, Research, Luxury, Black Market), and all 27 module types have
dedicated meshes.

**Walk around.** Walkable ship interiors (per-class interior families, capital bridges with
viewport glass) and walkable station interiors, with a torch and a 1st/3rd-person camera toggle.

**Share the sector.** Lightweight AI trader and miner pilots that fly, mine until full, and sell at
the best-paying station.

**Save.** Save v2 stores your ship, credits, cargo, docking state and player-built stations. F5/F9
quicksave and quickload.

**Hear it.** An event-driven audio layer: an audio catalog addressed by event ID, per-ship engine
voices, and a mix subsystem (interior muffle, menu duck, ambience beds) with volume sliders in the
pause menu.

**HUD.** A neon cockpit flight HUD (heading tape, flight-path marker, shield/hull, throttle),
targeting of ships, stations and asteroids, a sector map, trade and ship-select screens. The HUD is
drawn in C++ on the canvas (`AAdastreaHUD`), which is the path that renders reliably in PIE.

### Not wired in yet

A lot of code exists for systems that are deliberately switched off while the core loop matures:
combat, navigation/autopilot, quests, faction diplomacy, personnel/crew, the Way network and
advanced AI. The crafting recipe tree exists but nothing executes it yet. See the
[roadmap](docs/05-ROADMAP.md#-deferred-systems-code-exists-not-wired-into-the-live-loop) for the
full list and what's likely next.

---

## Getting started

### Prerequisites

- **Unreal Engine 5.8**
- **Visual Studio 2022** with the C++ game development workload (Windows is the only platform
  currently built and tested)
- **Python 3.11** for the tests and tooling
- **Blender** (optional) if you want to run the asset generators in `Tools/`

### Build and run

```bash
git clone https://github.com/Mittenzx/Adastrea.git
```

1. Right-click `Adastrea.uproject` → **Generate Visual Studio project files**.
2. Build the `Development Editor` configuration from `Adastrea.sln`, or from the command line:

   ```bat
   build_with_ue_tools.bat Development Win64
   ```

   Close the editor before building from the command line, or the build falsely reports failure.
   With the editor open, use Live Coding (`Ctrl+Alt+F11`) instead.
3. Open `Adastrea.uproject`. The editor starts in `TestLevel` (the playable sector with stations,
   an asteroid belt and AI ships). Press **Play**.

Having trouble generating project files? See
[PROJECT_GENERATION_QUICK_FIX.md](docs/09-SETUP_GUIDES/PROJECT_GENERATION_QUICK_FIX.md) and
[VISUAL_STUDIO_PROJECT_GENERATION.md](docs/09-SETUP_GUIDES/VISUAL_STUDIO_PROJECT_GENERATION.md).

### Packaging

A Win64 package builds and boots into `TestLevel`:

```bat
Tools\package_win64.bat
```

Output goes to `Saved\Packaged\Windows\Adastrea.exe`. See
[docs/09-SETUP_GUIDES/PACKAGING.md](docs/09-SETUP_GUIDES/PACKAGING.md) for configurations,
always-cooked folders and known issues.

### Controls (main ones)

| Key | Action |
|-----|--------|
| W/A/S/D, mouse | Fly |
| R / F | Throttle up / down |
| E | Dock (in flight) · Interact (on foot) |
| V | Get up from / return to the pilot seat |
| Tab, click | Targeting |
| `]` / `[` / Y / Z | Next / previous / nearest / clear target |
| LMB | Fire mining laser |
| M | Sector map |
| G | Station Editor |
| B/S | Trade screen (while docked) |
| P | Ship select |
| N | Station info |
| F5 / F9 | Quicksave / quickload |
| Esc | Pause menu |

The authoritative bindings are in C++: `AAdastreaPlayerController::SetupInputComponent` and the
runtime mapping context built in `ASpaceship`.

Useful console commands: `adastrea.SaveGame`, `adastrea.LoadGame`,
`adastrea.FleetMonitor [expand|collapse]` (debug list of every ship and station),
`DebugToggleInterior`.

---

## Repository layout

```
Adastrea/
├── Adastrea.uproject
├── Source/
│   ├── Adastrea/          # Game module: Ships, Stations, Trading, Mining, AI, Audio, Player, UI, Input
│   ├── StationEditor/     # In-game Station Editor (manager, grid, catalog, native overlay UI)
│   ├── PlayerMods/        # Player modification components
│   └── AdastreaEditor/    # Editor-only helpers (mesh validation)
├── Plugins/AdastreaShips/ # Imported 3D kit: ship meshes, interiors, materials
├── Content/               # Maps, Blueprints, DataAssets, Materials, Audio, UI, Data/*.json
├── Tools/                 # Python: Blender asset generators, renderers, UE importers, QA, packaging
├── tests/                 # pytest suite
├── docs/                  # Documentation (start at docs/INDEX.md)
├── wiki/                  # Git-wiki mirror
└── AGENT_BOARD.md         # Coordination log between parallel agent sessions
```

Key maps in `Content/Maps/`: `TestLevel` (main playable sector), `MiningTest`, `ShipShowcase`.

---

## Development

### Tests

```bash
python -m pytest -q -p no:randomly
```

`-p no:randomly` matters: two interior-contract tests are order-sensitive. The C++ side also has
UE automation tests (`Adastrea.StationEditor.*`) runnable from the editor's Session Frontend.
CI runs code-quality and module-dependency checks through GitHub Actions.

### Asset pipeline

Most 3D content is generated procedurally in Blender by scripts in `Tools/`
(`generate_adastrea_assets.py` for the ship and interior kit, plus dedicated station module, core
and interior generators), with a licensed BlenderKit sourcing path for hero props. Exports land in
`Assets/FBX/generated/` and are imported into UE by the importer scripts.
[docs/00-KNOWLEDGE_BASE.md §5](docs/00-KNOWLEDGE_BASE.md#5-content-asset-pipeline-blender--ue)
describes the pipeline in detail. `Tools/qa_assets.py` checks triangle budgets, degenerate faces
and bounds.

### Working with Claude Code

The repo is set up for agent-assisted development:

- `.mcp.json` points at Unreal's built-in MCP server (`http://127.0.0.1:8000/mcp`) so sessions can
  inspect and edit the running editor. `ue_mcp.py` is a small CLI for the same server.
- `.claude/agents/` has `unreal-integrator` and `blender-artist` subagents; `.claude/skills/`
  holds project skills.
- Parallel sessions coordinate through `AGENT_BOARD.md`. Commits should be scoped to specific
  paths, since other sessions may have files staged at the same time.

See [CLAUDE_SETUP.md](CLAUDE_SETUP.md) for setup details.

---

## Documentation

| Start here | |
|---|---|
| [docs/INDEX.md](docs/INDEX.md) | Documentation index |
| [docs/05-ROADMAP.md](docs/05-ROADMAP.md) | Current status, open work, known issues, deferred systems |
| [docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md) | Technical deep dive: systems, pipelines, tooling |
| [docs/06-SYSTEM_REFERENCE.md](docs/06-SYSTEM_REFERENCE.md) | Per-system reference |
| [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md) | Contributing guide and coding standards |
| [docs/09-SETUP_GUIDES/](docs/09-SETUP_GUIDES/) | Build, project generation, packaging |

Older planning material (the December 2025 critical review, the 12-week MVP plan, per-system
guides from before the docs reorg) is kept in [docs/14-ARCHIVE/](docs/14-ARCHIVE/) for history. It
no longer describes the project.

---

## Contributing

Contributions are welcome. Read [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md), open an issue
to discuss anything larger than a bug fix, and verify gameplay changes in PIE, not just by
compiling.

## License

MIT. See [LICENSE](LICENSE).
