# 🚀 Adastrea - Open-World Space Flight Game

**Built on Unreal Engine 5.8** | **MIT License** | **Active Development**

## 📊 Current Status

**🎯 Current Focus**: X4-style trade sim — fly, dock, trade, and walk ship interiors on foot; multiple Claude Code sessions developing in parallel  
**🔥 Status**: **ACTIVE DEVELOPMENT**  
**🔄 Latest Update**: See [docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md) for the current technical snapshot, or `AGENT_BOARD.md` for live cross-agent status  

---

## 🚀 Quick Navigation

**👉 New to Adastrea?** → **[docs/INDEX.md](docs/INDEX.md)** - Start here! ⭐

| For... | Go to... |
|--------|----------|
| 🎮 **Want to understand the project** | [docs/02-MVP_OVERVIEW.md](docs/02-MVP_OVERVIEW.md) |
| 💻 **Want to contribute code** | [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md) |
| 📚 **Looking for documentation** | [docs/INDEX.md](docs/INDEX.md) |
| 🎯 **Understanding the MVP** | [docs/02-MVP_OVERVIEW.md](docs/02-MVP_OVERVIEW.md) |
| 🏗️ **Architecture questions** | [docs/04-ARCHITECTURE.md](docs/04-ARCHITECTURE.md) |
| 📝 **Recent changes** | `git log` (no maintained root CHANGELOG.md currently exists) |
| ⚡ **Quick reference (MCP)** | [docs/MVP_QUICK_REFERENCE_CONSOLIDATED.md](docs/MVP_QUICK_REFERENCE_CONSOLIDATED.md) |

**Historical Context:**
- **January 2026**: Repository modernization (documentation consolidation)
- **December 2025**: [Critical review and MVP pivot](docs/14-ARCHIVE/archive_old/critical-review-2025/README.md) (archived)
- **October-December 2025**: Initial development (infrastructure-first approach)

---

# Adastrea - Open World Space Flight Game

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-blue.svg)](https://www.unrealengine.com/)
[![Build Status](https://github.com/Mittenzx/Adastrea/actions/workflows/ue-build-example.yml/badge.svg)](https://github.com/Mittenzx/Adastrea/actions/workflows/ue-build-example.yml)

> **Current Focus**: X4-style trade sim — fly ships, dock at stations, buy/sell cargo, walk interior spaces on foot. See [docs/05-ROADMAP.md](docs/05-ROADMAP.md) for current status.

---

> **Note**: The sections immediately below describe a December 2025–March 2026 "12-week MVP sprint"
> that has since been superseded by open-ended multi-agent development — see
> [docs/05-ROADMAP.md](docs/05-ROADMAP.md) and [docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md)
> for what's actually true today. Left in place as historical context rather than rewritten in full.

## 🚨 PROJECT UPDATE (December 29, 2025) — historical

**Project Status at the time**: **TRADE SIMULATOR MVP - WEEK 1 OF 12**

After comprehensive critical review (Dec 24, 2025), Adastrea is focused on a **Trade Simulator MVP** to validate gameplay before expanding scope.

**Current Week**: Week 1 (Dec 29 - Jan 4) - Hardcoded prototype foundation

### Recent Progress (Dec 24-29, 2025)

- ✅ **Critical Review Complete**: Identified architecture-first issues
- ✅ **Phase 2.1 Analysis**: 186 functions → 64-75 MVP-critical
- ✅ **Trading Cleanup**: Removed 12 non-MVP functions
- ✅ **All Roadmaps Updated**: 9 documents aligned with MVP focus
- 🔄 **Week 1 In Progress**: Hardcoded prototype development starting

### What Happened

- ✅ **Code**: 85% complete (33K lines, 22 systems)
- ❌ **Playable Content**: 0% (no gameplay to test)
- 📊 **Assessment**: Architecture-first approach led to over-engineering without validation

### What's Changing

**NEW FOCUS**: Build playable trading game in 12 weeks
**Core Loop**: Buy low → Fly → Sell high → Upgrade ship → Repeat

**Scope Discipline**:
- ✅ **Building**: Trading system, basic flight, stations, economy
- 🚫 **Deferred**: Combat, exploration, quests, crew management (until post-MVP)

**Validation Gates**:
- Week 4: 60% "had fun" rating or pivot
- Week 12: 75% "had fun", 50% "would buy" → Ready for funding

### Critical Documents

**Historical context** (see the note above — this list documented the Dec 2025 pivot, not current reading order):
1. 📊 [Critical Review Summary](docs/13-CRITICAL_REVIEW_SUMMARY.md) - Honest assessment, why we're pivoting
2. 📋 [Action Plan](docs/14-ARCHIVE/archive_old/critical-review-2025/CRITICAL_REVIEW_ACTION_PLAN.md) - 12-week recovery roadmap (archived)
3. 🎯 [MVP Instructions](.github/instructions/trade-simulator-mvp.instructions.md) - What to build
4. 🚫 [Anti-Patterns](.github/instructions/anti-patterns.instructions.md) - Lessons learned (18 mistakes to avoid)
5. 📁 [MVP Documentation](docs/14-ARCHIVE/archive_old/critical-review-2025/README.md) - Original phase details (archived)

**Key Principle**: One polished gameplay loop beats ten half-finished systems.

---

## 📖 Documentation Navigation

**New to Adastrea?** Start with the current, maintained docs:
- 📁 **[docs/INDEX.md](docs/INDEX.md)** - Master documentation index
- ⭐ **[docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md)** - Authoritative, repo-current technical snapshot
- 📊 **[docs/05-ROADMAP.md](docs/05-ROADMAP.md)** - Current status and priorities

Plus a wiki mirror covering core systems and architecture:
- 🚀 [Quick Start Guide](wiki/Getting-Started/Quick-Start.md) - Get running in 10 minutes
- 🎨 [Designer Onboarding](wiki/Getting-Started/Designer-Onboarding.md) - For non-programmers
- 🎮 [Core Systems Overview](wiki/Core-Systems/README.md) - Game systems
- 💻 [Developer Guides](wiki/Developer-Guides/Architecture.md) - Architecture and contributing

> The many individual per-system guides this section used to link to (under `Assets/`, plus a
> `docs/development/`, `docs/setup/`, `docs/systems/` folder structure) were consolidated into
> `docs/` and the rest archived to `docs/14-ARCHIVE/` during a documentation reorg. A handful of
> `Assets/*.md` guides are still live and linked from their relevant system sections below; the
> "Endless Development System" hourly-cycle automation described in an earlier version of this
> README is not currently active.

## Table of Contents

- [About](#about)
- [Quick Start](#quick-start)
- [Fast-Forward Content Creation](#fast-forward-content-creation) ⭐ **NEW**
- [Project Roadmap](#project-roadmap)
- [Project Structure](#project-structure)
- [Core Features](#core-features)
  - [Spaceship System](#spaceship-data-asset-system)
  - [Space Station System](#space-station-editor-system)
  - [Faction System](#faction-system)
  - [Way System](#way-system)
  - [Personnel Management](#personnel-management-system)
  - [AI System](#faction-and-personnel-ai-system)
  - [Trading System](#trading-system)
  - [Material System](#material-system)
  - [Map System](#map-system)
- [Scene and Asset Population](#scene-and-asset-population)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## About

**Adastrea** is an open-world space flight game built on Unreal Engine 5.8. Players can explore vast galaxies, engage in thrilling space battles, discover new planets, manage space stations, build crew relationships, and participate in a dynamic galactic economy. The project features 22 major game systems with over 33,000 lines of C++ code and comprehensive Blueprint integration.

### Key Highlights

- 🚀 **Data-Driven Design**: All game content uses Blueprint Data Assets
- 🎮 **Designer-Friendly**: Minimal C++ knowledge required
- 🏗️ **Modular Architecture**: Systems work independently and integrate seamlessly
- 🤖 **Automated Scene Population**: Quickly populate levels with actors and interior layouts
- 📚 **Comprehensive Documentation**: Guides, templates, and workflows for everything
- 🔧 **Blueprint First**: Full Blueprint exposure for all systems

## Quick Start

### Prerequisites

- **Unreal Engine 5.8** (check the .uproject file for exact version)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++ development
- **Git** for version control

### 🆕 Build with UE Build Tools Only

You can now build Adastrea using only the Unreal Engine build tools (~500MB) instead of the full engine (~50GB):

**Windows (SetEnv workaround for UE 5.6):**
```batch
REM Download build tools
setup_ue_build_tools.bat

REM Build the project
build_with_ue_tools.bat Development Win64
```

**Linux/Mac:**
```bash
# Download build tools
./setup_ue_build_tools.sh

# Build the project
./build_with_ue_tools.sh Development Linux
```

> **🚨 Windows Users:** If experiencing MSBuild SetEnv task failures (49KB+ include paths), this was the workaround used for large UE 5.6 projects (unverified whether it's still needed on 5.8). See [Windows Build Scripts Reference](docs/14-ARCHIVE/reference/WINDOWS_BUILD_SCRIPTS.md) (archived).

See **[BUILD_WITH_UE_TOOLS.md](docs/09-SETUP_GUIDES/BUILD_WITH_UE_TOOLS.md)** for complete instructions.

**Perfect for:**
- ✓ CI/CD pipelines
- ✓ Automated testing
- ✓ Build verification
- ✓ Code compilation without full editor
- ✓ **Windows: Bypassing MSBuild SetEnv limitations**

**Note:** To actually run and edit the project, you still need the full Unreal Engine 5.8 installation.

### Robust Setup Validation

Before building the project, it's **highly recommended** to run our automated setup validation scripts to ensure your development environment is properly configured. These scripts check for all prerequisites, validate project structure, and can optionally perform static analysis.

#### Option 1: Shell Script (Linux/Mac/WSL)

```bash
# Basic validation
./Tools/SetupCheck.sh

# Validation with build folder cleanup
./Tools/SetupCheck.sh --clean

# Validation with static analysis (requires cppcheck)
./Tools/SetupCheck.sh --analyze

# Display help
./Tools/SetupCheck.sh --help
```

#### Option 2: Python Script (Cross-Platform)

```bash
# Basic validation (works on Windows, Linux, Mac)
python Tools/SetupCheck.py

# Validation with static analysis (requires cppcheck)
python Tools/SetupCheck.py --analyze

# Verbose output with detailed information
python Tools/SetupCheck.py --verbose

# Display help
python Tools/SetupCheck.py --help
```

The validation scripts check for:
- ✓ Unreal Engine version compatibility (5.8)
- ✓ C++ compiler availability (MSVC, GCC, Clang)
- ✓ Required project folders (Source, Config, Content, Assets)
- ✓ Required project files (.uproject, module headers)
- ✓ Essential documentation files
- ✓ Optional: Static code analysis with cppcheck

**Note**: These scripts automate many of the manual checks from [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md). While the scripts handle environment validation, the playtesting checklist is still valuable for in-editor verification and quality assurance.

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Mittenzx/Adastrea.git
   cd Adastrea
   ```

2. **Run Setup Validation** (Recommended):
   ```bash
   # Linux/Mac/WSL
   ./Tools/SetupCheck.sh

   # Or cross-platform Python
   python Tools/SetupCheck.py
   ```

   This validates your environment and identifies any missing prerequisites before building.

3. **Generate Project Files**:
   - Right-click on the `.uproject` file
   - Select "Generate Visual Studio project files"
   - **Having issues?** See [PROJECT_GENERATION_QUICK_FIX.md](docs/09-SETUP_GUIDES/PROJECT_GENERATION_QUICK_FIX.md) for immediate solutions
   - **Need details?** See [VISUAL_STUDIO_PROJECT_GENERATION.md](docs/09-SETUP_GUIDES/VISUAL_STUDIO_PROJECT_GENERATION.md) for comprehensive troubleshooting

4. **Build the project**:
   - Open the generated `.sln` file in Visual Studio
   - Build in "Development Editor" configuration
   - Or open the `.uproject` file directly in Unreal Engine

5. **Verify Installation**:
   - Launch the editor
   - Check that all example Data Assets load correctly
   - Run the game with the Play button
   - (Optional) run asset validation via `Tools/qa_assets.py`

### Testing and Automation

> **Verified during this pass**: `AutomationRunner.py`, `AdastreaAssetValidator.py`, and
> `test_epic_connection.sh` (all referenced in an earlier version of this section) no longer
> exist at the repo root, and no `CONTAINER_DIAGNOSTICS.md`/`CLOUD_BUILD_*.md`/
> `TESTING_AUTOMATION_GUIDE.md` file exists anywhere in the repo (not even archived) — the
> Docker/Epic-container build path this section described appears to have been dropped entirely.

The project's real, current test suite is **128 pytest tests** (`python -m pytest`, see
[docs/00-KNOWLEDGE_BASE.md §6](docs/00-KNOWLEDGE_BASE.md)) plus `Tools/qa_assets.py` for
per-asset QA checks (tri budgets, degenerate faces, bounds). CI runs via GitHub Actions
(`.github/workflows/`) — no Docker/Epic Games container step is currently configured.

### Procedural Content Generation

Generate randomized game content quickly with the procedural generation system:

```bash
# Generate everything at once (star systems, NPCs, quests, ships)
python ProceduralGenerators.py --type all

# Generate specific content
python ProceduralGenerators.py --type galaxy --count 10    # Star systems
python ProceduralGenerators.py --type npc --count 25       # Crew members
python ProceduralGenerators.py --type quest --count 20     # Missions
python ProceduralGenerators.py --type ship --count 8       # Ship variants

# Interactive menu
python ProceduralGenerators.py
```

**What Gets Generated:**
- 🌌 **Galaxy Systems** - Complete star systems with planets, asteroids, stations, and factions
- 👥 **NPCs** - Crew members with realistic names, skills, backstories, and personality traits
- 📜 **Quests** - Missions with objectives, rewards, difficulty scaling, and special conditions
- 🚀 **Ship Variants** - Randomized ship designs with class-appropriate stats and unique names

All generated content is saved as YAML files compatible with the existing Data Asset system and can be imported directly into Unreal Engine.

The `PROCEDURAL_GENERATION_docs/QUICKSTART.md` and `PROCEDURAL_GENERATION_GUIDE.md` guides this
section used to link to no longer exist — the commands above are runnable as-is against the
current `ProceduralGenerators.py`.

### First Steps

> Most of the 11 linked guides this list originally pointed to (`FAST_FORWARD_QUICK_START.md`,
> `CPP_FOCUSED_SETUP.md`, `ESSENTIAL_BLUEPRINTS_ONLY.md`, `BLUEPRINT_QUICK_START_PIE.md`,
> `BLUEPRINT_REQUIREMENTS_LIST.md`, `BLUEPRINT_LIST_FOR_PIE.md`, `Docs/TestMode_INDEX.md`,
> `TESTING_AUTOMATION_GUIDE.md`, `PROCEDURAL_GENERATION_docs/QUICKSTART.md`) no longer exist.
> Trimmed to what's real:

1. **For Designers**: Read [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)
2. **For Programmers**: Read [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md) and [ARCHITECTURE.md](ARCHITECTURE.md)
3. **For the current technical picture**: [docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md)
4. **For Procedural Content**: `python ProceduralGenerators.py` (root) or the `Tools/` pipeline scripts
5. **Quick Reference**: See the still-live per-system guides listed in [Documentation](#documentation) below

## Project Roadmap

📍 **Want to see where the project is headed?** See **[docs/05-ROADMAP.md](docs/05-ROADMAP.md)** for
current status, active work, and known gaps — kept in sync with the live development happening in
this repo (the root-level `ROADMAP.md` and `docs/03-CURRENT_STATUS.md` are retired and just point
there).

## Content Generation

The project's real content pipeline (Blender-based procedural generation plus a licensed
BlenderKit sourcing path for hero props) lives in **`Tools/`** — see
**[docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md#5-content-asset-pipeline-blender--ue)**
for the current, accurate description of `generate_adastrea_assets.py`,
`blenderkit_fetch.py`/`blenderkit_import.py`, the render/QA scripts, and the UE import workflow.
`ProceduralGenerators.py` (galaxy systems, NPCs, quests, ship variants) is also still present at
the repo root.

> An earlier version of this section described a `MasterContentGenerator.py` one-shot generator
> and linked to `FAST_FORWARD_QUICK_START.md` / `FREE_CONTENT_GENERATION_GUIDE.md` /
> `FREE_TOOLS_COMPARISON.md`. None of those exist anymore — the content pipeline has since moved
> to the `Tools/` scripts referenced above.

## Project Structure

> The tree below reflects the actual current repo layout (see
> [docs/00-KNOWLEDGE_BASE.md §2](docs/00-KNOWLEDGE_BASE.md) for the maintained version). An
> earlier version of this section described a `Python Scripts/` folder and several root-level
> guide files that no longer exist — content generation and automation now live under `Tools/`.

```
Adastrea/
├── Adastrea.uproject          # UE project file
├── README.md                  # Project overview
├── ROADMAP.md                 # Retired: points to docs/05-ROADMAP.md
├── ARCHITECTURE.md            # MVP architecture
├── LICENSE                    # MIT License
├── AGENT_BOARD.md             # Cross-agent async coordination board
│
├── docs/                      # Current documentation (start at docs/INDEX.md)
│   ├── 00-KNOWLEDGE_BASE.md   # Authoritative technical snapshot
│   ├── 05-ROADMAP.md          # Current status and priorities
│   ├── 11-TECHNICAL_SPECS/    # Per-system technical specs
│   └── 14-ARCHIVE/            # Archived legacy documentation (500+ files)
│
├── Source/Adastrea/           # Core game C++ module
│   ├── Public/ Private/       # Ships, Stations, Trading, Player, UI, Input, Materials, Planets
│
├── Plugins/
│   ├── AdastreaShips/         # Imported 3D kit (meshes, interiors, materials)
│   └── vc-ue-extensions/      # VS/UE editor extensions
│
├── Content/                   # UE assets: Blueprints, DataAssets, Maps, Materials, Textures, UI
├── Assets/FBX/generated/      # Blender exports: ships, interiors, textures
├── Tools/                     # ~60 Python scripts: asset generation, renderers, QA, UE importers
├── tests/                     # pytest test suite
└── wiki/                      # Git-wiki mirror (Core-Systems, Developer-Guides...)
```

## Core Features

### Spaceship Data Asset System
A comprehensive spaceship specification system that allows designers to create detailed ship configurations as Blueprint Data Assets without writing C++ code. The base `ASpaceship` class includes FloatingPawnMovement for 3D space flight capabilities.

**Key Components:**
- `ASpaceship` - Base ship actor with FloatingPawnMovement component for space flight
- `USpaceshipDataAsset` - Complete ship specification data asset with all stat categories
- Core Stats: Hull strength, cargo capacity, crew requirements, modular points
- Combat Stats: Armor, shields, weapons, point defense systems
- Mobility Stats: Speed, acceleration, maneuverability, jump range, fuel
- Utility Stats: Sensors, stealth, repair, science, medical systems
- Operational Stats: Power, life support, maintenance, hangars, drones, AI
- Advanced Stats: Diplomacy, boarding, customization slots, electronic warfare, mining
- Lore/Flavor: Manufacturer, year built, rarity tier, colors, lore notes

**Ship Templates Available:**
- **Pathfinder Scout** - Fast reconnaissance vessel with exceptional sensors
- **Warhammer Gunship** - Heavy combat ship with devastating firepower
- **Merchant Trader** - Large cargo hauler optimized for trade routes
- **Starliner Luxury** - Premium passenger vessel with diplomatic facilities
- **Excavator Mining** - Industrial mining barge with resource extraction tools
- **Sovereign Command** - Capital flagship with command and carrier operations

**Blueprint-Callable Functions:**
- `GetCombatRating()` - Calculate overall combat effectiveness (0-100)
- `GetMobilityRating()` - Calculate overall mobility (0-100)
- `GetUtilityRating()` - Calculate overall utility (0-100)
- `IsSuitableForRole(Role)` - Check if ship fits a specific role
- `GetSizeCategory()` - Get ship size classification
- `GetOperationalCost()` - Calculate daily maintenance cost

**Documentation:**
- **Complete Guide**: [Assets/SpaceshipDataAssetGuide.md](Assets/SpaceshipDataAssetGuide.md) - Full ship creation walkthrough
- **Templates Reference**: [Assets/SpaceshipTemplates.md](Assets/SpaceshipTemplates.md) - All ship templates with stats
- **Workflow Guide**: [Assets/SpaceshipDesignerWorkflow.md](Assets/SpaceshipDesignerWorkflow.md) - Best practices and workflows
- **Control Guide**: [Assets/ShipControlGuide.md](Assets/ShipControlGuide.md) - Ship control system from interiors
- **YAML Templates**: [Assets/SpaceshipTemplates/](Assets/SpaceshipTemplates/) - Copy-paste ready ship configurations

**Quick Usage:**
1. Create a new SpaceshipDataAsset in the Content Browser
2. Use YAML templates from `Assets/SpaceshipTemplates/` as reference
3. Configure all stat categories (core, combat, mobility, utility, operational, advanced, lore)
4. Test with Blueprint-callable helper functions
5. Integrate into your spaceship Blueprint actors

### Space Station Editor System
A modular space station construction system that enables players to build, customize, and manage space stations in-game. The system uses Blueprint-friendly C++ classes and is provided through the **StationEditor** runtime module.

**Key Components:**
- `ASpaceStation` - Core station actor with module management (add, remove, move modules) [Adastrea module]
- `ASpaceStationModule` - Base class for station modules with type classification [Adastrea module]
- `UStationEditorManager` - Core C++ manager handling placement, validation, undo/redo, construction queue [StationEditor module]
- `UStationModuleCatalog` - Data Asset defining available modules, costs, and requirements [StationEditor module]
- `UStationEditorWidget` - Player-facing UI widget for station editing [StationEditor module]
- `AStationBuildPreview` - Visual preview actor showing placement validity [StationEditor module]
- `UStationGridSystem` - Grid snapping and alignment system [StationEditor module]
- `EStationModuleGroup` - Module categorization enum (Docking, Power, Storage, Defence, etc.) [Adastrea module]

**Module Organization:**
- **StationEditor module** (Runtime): In-game player-facing station editor UI with full editing capabilities
- **Adastrea module** (Runtime): Core station and module actors
- **PlayerMods module** (Runtime): Player modification system

**Documentation:**
- **PIE Testing Guide**: [Assets/StationEditorPIETestingGuide.md](Assets/StationEditorPIETestingGuide.md) - ⭐ **START HERE!** Complete implementation guide for testing in PIE with key binding setup
- **Migration Guide**: [Assets/StationEditorMigrationGuide.md](Assets/StationEditorMigrationGuide.md) - 🆕 **Blueprint to C++** Migration guide for new C++ implementation
- **System API Reference**: [Assets/StationEditorSystemGuide.md](Assets/StationEditorSystemGuide.md) - Complete C++ API documentation
- **Management Guide**: [Assets/StationManagementGuide.md](Assets/StationManagementGuide.md) - Station management UI patterns
- **Blueprint Integration**: [Content/Blueprints/STATION_EDITOR_README.md](Content/Blueprints/STATION_EDITOR_README.md) - Blueprint implementation details

**Quick Usage:**
1. Create Blueprint extending `ASpaceStation` for your custom station
2. Create module Blueprints extending `ASpaceStationModule` (e.g., docking bays, trade hubs)
3. Create `DA_StationModuleCatalog` Data Asset with available modules
4. Create Widget Blueprint extending `UStationEditorWidget` for the in-game editor UI
5. In your Player Controller Blueprint:
   - Set `StationEditorWidgetClass` to your widget (e.g., `WBP_StationEditor`)
   - Set `ModuleCatalog` to your catalog data asset
6. Bind a key (e.g., M) to call `ToggleStationEditor()` - the C++ implementation handles everything automatically!
7. **New in 2025**: All editor opening logic is now in C++ - see [Migration Guide](Assets/StationEditorMigrationGuide.md)

### Way System
The Way System provides specialized guilds (Ways) and micro-alliances (Way Networks) based on shared values (Precepts). Ways are small focused organizations (50-1000 members) that handle specific industries and form networks where reputation with one member affects the entire alliance.

**Key Components:**
- `UWayDataAsset` - Specialized guild definition with industry, quality, and Precepts
- `UWayNetworkDataAsset` - Micro-alliance of 2-5 Ways with shared values
- `UVerseSubsystem` - Global network management and reputation tracking
- `UVerse` - Player component tracking Way reputation and completed Feats
- Reputation Spillover - Actions with one network member affect all members
- Network Bonuses - Reputation multipliers for aligned accomplishments

**Advanced Features:**
- **Industry Specialization**: 15 industry types (Mining, Manufacturing, Exploration, etc.)
  - Each Way focuses on specific production/consumption
  - Supply chain relationships between Ways
  - Quality reputation tiers (Basic to Legendary)

- **Way Networks**: Micro-alliances of 2-5 Ways
  - Blueprint-callable functions: `RegisterNetwork()`, `GetNetworksForWay()`, `GetNetworkVerseScore()`
  - Shared Precepts (values) unite network members
  - Configurable spillover (20-70% typical)
  - Network bonuses (1.5-2.5x multipliers)
  - Minimum reputation thresholds for benefits

- **Precepts and Feats**: Value-driven reputation system
  - Precepts: Core values (Craftsmanship, Discovery, Honor, etc.)
  - Feats: Legendary accomplishments that grant reputation
  - Alignment-based rewards (matching Precepts earn more rep)
  - Automatic network spillover with `RecordFeatWithNetworkEffects()`

- **Sector Councils**: Practical governance (not political)
  - Ways participate in sector decision-making
  - Tax rates and budget allocation
  - Weighted voting based on influence
  - Players can join councils through Way leadership

- **Economic Integration**: Supply chains and trading
  - Ways produce and consume resources
  - Trade agreements and pricing
  - Network member trading bonuses
  - Joint contracts for high-reputation players

**Example Networks:**
- **Craftsmen's Compact** - Engineering excellence (Engineers, Ship-Builders, Refiners)
- **Frontier Alliance** - Exploration and discovery (Scouts, Cartographers, Pioneers)
- **Honor Circle** - Warrior traditions (Warriors, Duelists, Protectors, Bounty Hunters)
- **Merchant Coalition** - Trade and commerce (Traders, Brokers, Transport Union)
- **Scholars' Syndicate** - Knowledge and research (Researchers, Archivists, Scientists)

**Documentation:**
- **Blueprint Examples**: [Blueprints/WayNetworkBlueprintExamples.md](Blueprints/WayNetworkBlueprintExamples.md) - Copy-paste examples
- **YAML Templates**: [Assets/WayNetworkTemplates/](Assets/WayNetworkTemplates/) - Ready-to-use network configs

> The dedicated Way/Way-Network prose guides this section used to link to (`WaySystemGuide.md`,
> `WayCreationGuide.md`, `WayNetworkGuide.md`, etc.) have been archived; `UWayDataAsset` /
> `UWayNetworkDataAsset` still exist in source (`Source/Adastrea/Public/Player/Verse.h`).

**Quick Usage:**

*Creating a Way:*
1. Create a new WayDataAsset in the Content Browser
2. Configure identity (name, ID, description)
3. Set specialization (industry, quality tier, member count)
4. Define Core Precepts (3-5 values with importance levels)
5. Add produced/consumed resources for supply chains
6. Set organizational attributes (tech, military, economic, influence)

*Creating a Network:*
1. Create a new WayNetworkDataAsset in the Content Browser
2. Add 2-5 member Ways with influence and commitment levels
3. Define Shared Precepts (copy from member Ways)
4. Configure spillover percentage (30-50% typical)
5. Set network alignment bonus (1.5-2.0x typical)
6. Set minimum reputation threshold (20-40 typical)
7. Register network with VerseSubsystem on game start

*Using in Gameplay:*
- Complete Feats aligned with Way Precepts to earn reputation
- Reach reputation thresholds to access network bonuses
- Gain spillover reputation with entire network automatically
- Access network-exclusive missions, discounts, and rewards
- Participate in sector council decisions as Way leader

### Faction and Personnel AI System — design documentation, not current code

> **Verified during this pass**: none of `UNPCLogicBase`, `UFactionLogic`, `UPersonnelLogic`, or
> `UPersonnelDataAsset` exist anywhere under `Source/` (checked directly, no matches). This
> section (and Personnel Management below) describes a designed system that isn't implemented in
> the current codebase — not just missing docs for something that exists. `docs/05-ROADMAP.md`
> lists "Personnel/Crew" and "Faction Diplomacy" as large deferred systems; treat the detailed
> class/feature descriptions that used to live here as design intent for that future work, not a
> description of what's buildable today. YAML templates for reference values are still present at
> [Assets/PersonnelAITemplates/](Assets/PersonnelAITemplates/) and
> [Assets/PersonnelTemplates/](Assets/PersonnelTemplates/) (`Assets/FactionAITemplates/` is gone).

### Personnel Management System

See the note above — this section previously documented `UPersonnelDataAsset` and a 31-role
personnel system that does not currently exist in `Source/`. Treated as design intent for the
deferred Personnel/Crew system rather than rewritten with invented current behavior.

### Additional Core Systems

Combat, Navigation/Autopilot, Quest, Save, Tutorial, Audio, and Player Progression systems are
present in the codebase but currently **deferred/disabled** per [docs/05-ROADMAP.md](docs/05-ROADMAP.md)
(Combat/Navigation/Quest/Faction Diplomacy/Advanced AI/Way Network are "complete, disabled";
Save/Load and Exploration are "partial"). The per-system prose guides this section used to link
to (`CombatSystemGuide.md`, `NavigationSystemGuide.md`, `QuestSystemGuide.md`, `SaveSystemGuide.md`,
`TutorialSystemGuide.md`, `AudioSystemGuide.md`, `PlayerProgressionGuide.md`) have been archived;
see `docs/06-SYSTEM_REFERENCE.md` for the current active/deferred system breakdown.

**Enhanced Input System** - Centralized input configuration with 30+ actions organized into categories (Movement, Combat, Navigation, System, Camera). See [Assets/EnhancedInputImplementation.md](Assets/EnhancedInputImplementation.md) and [Assets/MousewheelThrottleSetupGuide.md](Assets/MousewheelThrottleSetupGuide.md) for detailed mousewheel throttle control setup.

**UI Systems** - HUD (`AAdastreaHUD`, the reliably-rendering canvas path in PIE), Ship Status/Inventory/Ship Customization/Trading/Station Management widgets, all verified present in `Source/Adastrea/Public/{Player,UI}/`. The dedicated `ShipStatusScreenSetup.md` guide this bullet used to link to has been archived.

**Performance Tools** - referenced `LODManagerComponent`/`PerformanceProfiler` and their guide
(`PerformanceOptimizationGuide.md`) could not be verified in current `Source/` during this pass —
flagging rather than asserting either way.

### Map System

A comprehensive navigation system for displaying the universe and individual sectors with exploration tracking.

**Key Components:**
- **UniverseMapWidget** - Galaxy-level view showing all sectors organized in a 3D grid
- **SectorMapWidget** - Detailed view of individual sectors with object listings
- **Sector Discovery** - Track explored vs unexplored regions
- **Player Tracking** - Automatically locate and display player's current sector
- **Grid Organization** - Sectors organized based on world position (200km per cell)
- **Exploration Progress** - Statistics showing percentage of universe discovered

**Features:**
- Full Blueprint exposure with BlueprintNativeEvent pattern
- Automatic sector discovery as player explores
- Neighboring sector finding and distance calculations
- Random spawn position generation within sectors
- Integration with quest markers and faction territories
- Customizable visibility for undiscovered sectors

**Usage:**
Create Widget Blueprints based on `UniverseMapWidget` and `SectorMapWidget` classes, design the UI layout, and bind to player controller for map toggle (M key recommended).

> The `Map System Guide`/`Map System Quick Start` docs this section used to link to
> (`Assets/MapSystemGuide.md`, `Assets/MapSystemQuickStart.md`) have been archived; the
> component descriptions above are verified against current source
> (`Source/Adastrea/Public/UI/UniverseMapWidget.h` / `SectorMapWidget.h`).

## Documentation

> This section previously listed ~80 links across Plugin/Tool, Project Planning, Research,
> Visual Design, Developer, Build/CI, Quick Start, Blueprint, and System Documentation
> subsections. Auditing them during this pass found the large majority dead (root-level and
> `Assets/*.md` files that were archived to `docs/14-ARCHIVE/` in a later documentation reorg).
> Consolidated to what's actually current, plus the real subset of `Assets/*.md` guides that are
> still live.

**Start here**: [docs/INDEX.md](docs/INDEX.md) → [docs/00-KNOWLEDGE_BASE.md](docs/00-KNOWLEDGE_BASE.md)
(authoritative technical snapshot) → [docs/05-ROADMAP.md](docs/05-ROADMAP.md) (current status/priorities).

**Setup**: [Build with UE Tools](docs/09-SETUP_GUIDES/BUILD_WITH_UE_TOOLS.md) ·
[Visual Studio Project Generation](docs/09-SETUP_GUIDES/VISUAL_STUDIO_PROJECT_GENERATION.md) ·
[Project Generation Quick Fix](docs/09-SETUP_GUIDES/PROJECT_GENERATION_QUICK_FIX.md)

**Contributing**: [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md) ·
[Unreal Directive Best Practices](.github/instructions/unreal-directive-best-practices.md) ·
[Custom Agents](.github/agents/README.md)

**Still-live per-system guides** (the rest were archived — see the note above):
- Spaceship: [Data Asset Guide](Assets/SpaceshipDataAssetGuide.md) ·
  [Templates](Assets/SpaceshipTemplates.md) ·
  [Designer Workflow](Assets/SpaceshipDesignerWorkflow.md) ·
  [Control Guide](Assets/ShipControlGuide.md) ·
  [X4 Flight Controls](Assets/X4FlightControlsGuide.md) ·
  [Mousewheel Throttle Setup](Assets/MousewheelThrottleSetupGuide.md)
- Station Editor: [PIE Testing Guide](Assets/StationEditorPIETestingGuide.md) ·
  [Migration Guide](Assets/StationEditorMigrationGuide.md) ·
  [System Guide](Assets/StationEditorSystemGuide.md) ·
  [Management Guide](Assets/StationManagementGuide.md) ·
  [Blueprint README](Content/Blueprints/STATION_EDITOR_README.md)
- Trading: [System Guide](Assets/TradingSystemGuide.md) ·
  [API Reference](Assets/TradingAPIReference.md) ·
  [Designer Workflow](Assets/TradingDesignerWorkflow.md)
- Way Network (design docs for a deferred system): [Blueprint Examples](Blueprints/WayNetworkBlueprintExamples.md)
- General: [Designer Onboarding](Assets/DesignerOnboarding.md) ·
  [Playtesting Checklist](Assets/PlaytestingChecklist.md) ·
  [Iteration Guide](Assets/IterationGuide.md) ·
  [Blueprint Workflow Templates](Assets/BlueprintWorkflowTemplates.md)

**Editor Development**: *The `AdastreaEditor` C++ module has been removed, but `AdastreaEditor.Target.cs` is included for building the project in Unreal Editor. Future editor customizations can be added as a new module if needed.*

### YAML Templates

Ready-to-use YAML templates that still exist:
- `Assets/SpaceshipTemplates/` - Ship configurations
- `Assets/PersonnelAITemplates/` - Personnel AI personalities
- `Assets/PersonnelTemplates/` - Crew member profiles
- `Assets/TradingTemplates/` - Trade items and markets

(`Assets/FactionAITemplates/` referenced in an earlier version of this section no longer exists.)

## Contributing

We welcome contributions! Here's how to get started:

1. **Read the Guidelines**: See [docs/08-CONTRIBUTING.md](docs/08-CONTRIBUTING.md) for detailed instructions
2. **Check Issues**: Look for issues labeled "good first issue" or "help wanted"
3. **Fork & Clone**: Fork the repository and create a feature branch
4. **Make Changes**: Follow our coding standards and best practices
5. **Test Thoroughly**: Ensure your changes work in-editor
6. **Submit PR**: Open a pull request with a clear description

### Ways to Contribute
- 🐛 **Bug Reports**: Found a bug? Open an issue with reproduction steps
- ✨ **New Features**: Have an idea? Discuss it in an issue first
- 📚 **Documentation**: Improve guides, fix typos, add examples
- 🎨 **Assets**: Create example ships, factions, or other content
- 🧪 **Testing**: Help test features and report issues

### Code of Conduct
Be respectful, constructive, and welcoming. We're all here to build something great together!

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Version History

No maintained root changelog currently exists — see `git log` for detailed version history.

## Support

- **Documentation Issues**: Open an issue with the "documentation" label
- **Bug Reports**: Use the bug report template
- **Feature Requests**: Use the feature request template
- **General Questions**: Start a discussion in the repository

---

**Built with ❤️ using Unreal Engine**
