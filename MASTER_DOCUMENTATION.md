# Adastrea - Master Documentation

**Version**: 1.0.0-alpha  
**Last Updated**: December 9, 2025  
**Purpose**: Comprehensive single-file documentation consolidating all project documentation

---

## 📖 Navigation

**Quick Links:**
- [Current Project Status](#current-project-status) - ⚠️ Read this first!
- [Quick Start](#quick-start) - Get started in 10 minutes
- [System Index](#system-documentation-index) - Find specific system docs
- [Developer Guides](#developer-guides) - Architecture and coding standards
- [Complete File Index](#complete-documentation-index) - All 150+ markdown files

---

## Table of Contents

### I. Project Overview
1. [About Adastrea](#1-about-adastrea)
2. [Current Project Status](#2-current-project-status) ⚠️ **Important**
3. [Quick Start](#3-quick-start)
4. [Architecture Overview](#4-architecture-overview)

### II. Core Documentation  
5. [System Documentation Index](#5-system-documentation-index)
6. [Developer Guides](#6-developer-guides)
7. [Blueprint Documentation](#7-blueprint-documentation)
8. [Coding Standards](#8-coding-standards)

### III. Development Workflows
9. [Designer Workflows](#9-designer-workflows)
10. [Automation & Scripts](#10-automation--scripts)
11. [Testing & Quality Assurance](#11-testing--quality-assurance)
12. [Build & CI/CD](#12-build--cicd)

### IV. System Guides (Summary)
13. [Spaceship System](#13-spaceship-system)
14. [Space Station System](#14-space-station-system)
15. [Faction & Way Systems](#15-faction--way-systems)
16. [AI System](#16-ai-system)
17. [Trading & Economy](#17-trading--economy)
18. [Combat & Exploration](#18-combat--exploration)
19. [UI & Player Systems](#19-ui--player-systems)

### V. Reference Materials
20. [Complete Documentation Index](#20-complete-documentation-index)
21. [YAML Templates Reference](#21-yaml-templates-reference)
22. [Troubleshooting](#22-troubleshooting)

### VI. Appendices
23. [Archived Documentation](#23-archived-documentation)
24. [Version History](#24-version-history)
25. [Contributing](#25-contributing)

---

## 1. About Adastrea

**Adastrea** is an open-world space flight game built on Unreal Engine 5.6. Players can explore vast galaxies, engage in space battles, discover planets, manage space stations, build crew relationships, and participate in a dynamic galactic economy.

### Project Highlights

- 🚀 **Data-Driven Design**: All game content uses Blueprint Data Assets
- 🎮 **Designer-Friendly**: Minimal C++ knowledge required  
- 🏗️ **Modular Architecture**: 22 independent, integrated systems
- 📚 **33,000+ Lines of C++ Code**: Robust core implementation
- 🔧 **Blueprint First**: Full Blueprint exposure for all systems
- 🤖 **Automated Content Generation**: Python scripts for rapid prototyping

### Technology Stack

- **Engine**: Unreal Engine 5.6
- **Languages**: C++ (primary), Blueprint (visual scripting), Python (automation)
- **Platforms**: Windows, Mac, Linux
- **Build System**: Unreal Build Tool (UBT)
- **Version Control**: Git

### Key Features

- **Spaceship System** - 6 stat categories, modular design
- **Space Station System** - Modular construction with player editing
- **Faction System** - Complex diplomacy and relationships
- **Way System** - Specialized guilds with reputation networks
- **Personnel System** - 31 role types with skills and relationships
- **AI System** - Personality-driven NPCs and strategic faction AI
- **Trading System** - Dynamic supply/demand economy
- **Combat System** - 6 weapon types, directional shields, boarding
- **22 Total Systems** - All fully implemented in C++

---

## 2. Current Project Status

**Last Analysis**: November 29, 2025

### ⚠️ IMPORTANT: Realistic Status Assessment

**The project is in an interesting state where code is nearly complete but editor content is minimal.**

#### ✅ What IS Complete (Coding)

- **209 C++ files** (102 .cpp, 107 .h) - Well-structured
- **22 Major game systems** - Fully implemented in C++
- **Clean compilation** - No errors
- **Comprehensive Blueprint exposure** - All systems have UFUNCTION/UPROPERTY
- **Automation scripts** - Python tools for content generation
- **Documentation** - 150+ markdown files with guides and templates

#### ❌ What IS NOT Complete (Editor Implementation)

- **Blueprint Data Assets** - C++ classes exist, but Data Assets not created in editor
- **Actor Blueprints** - Templates documented, but actual Blueprints not created  
- **Maps/Levels** - Only 2 basic maps exist (TestLevel.umap, MainMenu.umap)
- **UI Widgets** - Widget base classes exist, but widgets not designed
- **Test Content** - Testing framework exists, but test scenarios not set up
- **Material Instances** - Material system coded, but materials not created
- **Audio Assets** - Audio system coded, but sounds not imported

### The Reality Gap

**Code Status**: 85-90% complete  
**Editor Content Status**: 10-15% complete  
**Overall Playability**: ~20% complete

**What this means:**
- ✅ You can read the code and understand how everything works
- ✅ You can compile the project successfully
- ❌ You cannot actually play the game yet without creating editor content
- ❌ Most systems need their Data Assets and Blueprints created
- ⚠️ Testing is possible once content is created

### What's Needed to Make It Playable

1. **Create Blueprint Data Assets** (~100+ assets)
   - Ships, factions, personnel, trading items, etc.
   - Use YAML templates in `Assets/` directories as reference

2. **Create Actor Blueprints** (~50+ blueprints)
   - Spaceships, stations, modules, NPCs, etc.
   - Follow guides in `BLUEPRINT_REQUIREMENTS_LIST.md`

3. **Create Game Maps** (~10+ maps)
   - Test levels, sectors, station interiors
   - Follow `Content/Maps/TestMode.umap.txt` specifications

4. **Create UI Widgets** (~30+ widgets)
   - HUD, menus, inventory, trading interface
   - Extend C++ widget base classes

5. **Import Audio/Visual Assets** (~100+ assets)
   - Sound effects, music, textures, models
   - Configure using system guides

6. **Configure Input Mappings** (1 Data Asset)
   - Create `DA_InputConfig` in editor
   - Assign all 30+ input actions

### Why the Disconnect?

The project has been developed with a **code-first approach**, focusing on:
- Robust C++ architecture
- Complete system design
- Comprehensive documentation
- Blueprint-friendly APIs

However, **editor content creation** has been minimal because:
- Focus was on establishing solid foundation
- C++ systems needed to be finalized first
- Content creation is time-consuming
- Documentation was prioritized for designers

### Next Steps for Developers

If you want to make this game playable:

**Week 1-2: Core Systems**
- Create ship Data Assets using `Assets/SpaceshipTemplates/`
- Create BP_Spaceship actor blueprint
- Create TestMode.umap level
- Set up input configuration

**Week 3-4: Basic Gameplay**
- Create faction Data Assets
- Create station blueprints
- Add NPC ships
- Implement basic combat

**Week 5-8: Build Out Content**
- Create all remaining Data Assets
- Build sector maps
- Design UI widgets
- Import audio/visual assets

**Week 9-12: Polish & Test**
- Create test scenarios
- Run automation tests
- Balance gameplay
- Fix bugs

See `BLUEPRINT_REQUIREMENTS_LIST.md` for complete blueprint creation guide.

---

## 3. Quick Start

### Prerequisites

- **Unreal Engine 5.6** (check .uproject for exact version)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac)
- **Git** for version control
- **Python 3.9+** for automation scripts

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Mittenzx/Adastrea.git
   cd Adastrea
   ```

2. **Run Setup Validation** (Recommended):
   ```bash
   # Linux/Mac/WSL
   ./SetupCheck.sh
   
   # Or cross-platform Python
   python SetupCheck.py
   ```

3. **Generate Project Files**:
   - Right-click on `.uproject` file
   - Select "Generate Visual Studio project files"

4. **Build the project**:
   - Open `.sln` file in Visual Studio
   - Build in "Development Editor" configuration
   - Or open `.uproject` directly in Unreal Engine

5. **Verify Installation**:
   - Launch the editor
   - Open TestLevel.umap
   - Press Play button
   - Note: No playable content exists yet, but compilation should succeed

### First Steps

**For Designers** (Want to create content):
1. Read `Assets/DesignerOnboarding.md`
2. Review `BLUEPRINT_GUIDE_FOR_BEGINNERS.md`
3. Start with `BLUEPRINT_REQUIREMENTS_LIST.md`
4. Use YAML templates in `Assets/` folders as reference

**For Programmers** (Want to understand code):
1. Read `ARCHITECTURE.md`
2. Review `CONTRIBUTING.md`
3. Check `CODE_STYLE.md`
4. Explore C++ source in `Source/Adastrea/`

**For Testers** (Want to set up testing):
1. Create TestMode.umap level (see `Content/Maps/TestMode.umap.txt`)
2. Create basic ship Blueprint (see `BLUEPRINT_QUICK_START_PIE.md`)
3. Review `TESTING_AUTOMATION_GUIDE.md`

---

## 4. Architecture Overview

### System Design Philosophy

Adastrea follows these core principles:

1. **Data-Driven Design**
   - All game content defined through Blueprint Data Assets
   - No hardcoded values in C++
   - Easy to create variants and test iterations

2. **Modular Architecture**
   - 22 independent game systems
   - Minimal coupling between systems
   - Each system can be developed/tested separately

3. **Blueprint-First Exposure**
   - All C++ classes fully exposed to Blueprints
   - BlueprintNativeEvent for extensibility
   - Designer-friendly APIs

4. **Performance-Conscious**
   - Follows Unreal Directive optimization patterns
   - Object pooling for frequently spawned actors
   - LOD management for distant objects
   - Tick optimization

### Project Structure

```
Adastrea/
├── Source/Adastrea/          # C++ game systems (209 files)
│   ├── AI/                   # AI logic classes (4 files)
│   ├── Characters/           # Character and personnel
│   ├── Combat/               # Combat mechanics (20 files)
│   ├── Exploration/          # Scanner and discovery (14 files)
│   ├── Factions/             # Faction system
│   ├── Inventory/            # Inventory management
│   ├── Materials/            # Material system
│   ├── Navigation/           # Movement and pathfinding
│   ├── Planets/              # Planet generation
│   ├── Player/               # Player-specific systems (24 files)
│   ├── Quest/                # Quest and missions
│   ├── Save/                 # Save/load system
│   ├── Sector/               # Sector governance
│   ├── Ships/                # Spaceship classes (24 files)
│   ├── Stations/             # Space station classes (34 files)
│   ├── Trading/              # Trading economy (10 files)
│   ├── Tutorial/             # Tutorial system
│   ├── UI/                   # UI widgets (25 files)
│   └── Way/                  # Way (guild) system (10 files)
│
├── Content/                  # Editor content (mostly empty)
│   ├── Blueprints/          # Blueprint actors (to be created)
│   ├── DataAssets/          # Data Assets (to be created)
│   ├── Maps/                # Level maps (2 exist: TestLevel, MainMenu)
│   ├── UI/                  # UI widgets (to be created)
│   └── [other]/             # Placeholder directories
│
├── Assets/                   # Documentation & templates (97 MD files)
│   ├── *Guide.md            # System documentation
│   ├── *Templates.md        # Reference templates
│   ├── *Workflow.md         # Designer workflows
│   └── */                   # YAML template directories
│
├── Plugins/                  # Unreal Engine plugins
│   ├── AdastreaDirector/    # AI assistant plugin (30 MD files)
│   └── VisualStudioTools/   # Development utilities
│
└── [Root]/                   # Python scripts & config (40 MD files)
    ├── ProceduralGenerators.py    # Content generation
    ├── ScenePopulator.py          # Scene automation
    ├── YAMLtoDataAsset.py         # Data import
    └── [others]                   # Testing & utilities
```

### System Dependency Map

```
Player Controller
    ├── Spaceship System → Combat System
    │   ├── Ship Data Assets → Trading System
    │   └── Ship Movement → Navigation System
    │
    ├── Faction System → Way System
    │   ├── Faction AI → Personnel System
    │   └── Diplomacy → Sector System
    │
    ├── Station System → Material System
    │   ├── Module Management
    │   └── Construction
    │
    ├── UI System
    │   ├── HUD → Ship Status
    │   ├── Trading Interface → Inventory
    │   └── Map System → Exploration
    │
    └── Progression System
        ├── Quest System
        ├── Save System
        └── Tutorial System
```

For complete architecture details, see `ARCHITECTURE.md`.

---

## 5. System Documentation Index

### Complete System List (22 Systems)

| # | System | Purpose | Doc Location |
|---|--------|---------|--------------|
| 1 | **Spaceship** | Ship specifications and flight | `Assets/SpaceshipDataAssetGuide.md` |
| 2 | **Space Station** | Modular station construction | `Assets/StationEditorSystemGuide.md` |
| 3 | **Faction** | Factions, relationships, diplomacy | `Assets/FactionSetupGuide.md` |
| 4 | **Way** | Specialized guilds and networks | `Assets/WaySystemGuide.md` |
| 5 | **Sector** | Governance and councils | `Assets/SectorCreationGuide.md` |
| 6 | **Personnel** | Crew management (31 roles) | `Assets/PersonnelSystemGuide.md` |
| 7 | **AI** | Faction and personnel AI | `Assets/FactionAISetupGuide.md` |
| 8 | **Trading** | Dynamic economy and contracts | `Assets/TradingSystemGuide.md` |
| 9 | **Combat** | Weapons, shields, boarding | `Assets/CombatSystemGuide.md` |
| 10 | **Exploration** | Scanners and discovery | `Assets/ExplorationSystemGuide.md` |
| 11 | **Navigation** | Autopilot and pathfinding | `Assets/NavigationSystemGuide.md` |
| 12 | **Quest** | Missions and objectives | `Assets/QuestSystemGuide.md` |
| 13 | **Material** | Resources and crafting | `Assets/MaterialSystemGuide.md` |
| 14 | **Map** | Universe and sector maps | `Assets/MapSystemGuide.md` |
| 15 | **Homeworld** | Starting locations | `Assets/HomeworldSystemGuide.md` |
| 16 | **HUD/UI** | In-game interface | `Assets/HUDSystemGuide.md` |
| 17 | **Inventory** | Item management | `Assets/InventorySystemGuide.md` |
| 18 | **Save** | Save/load game state | `Assets/SaveSystemGuide.md` |
| 19 | **Tutorial** | Interactive tutorials | `Assets/TutorialSystemGuide.md` |
| 20 | **Audio** | Sound effects and music | `Assets/AudioSystemGuide.md` |
| 21 | **Player Progression** | Leveling and unlocks | `Assets/PlayerProgressionGuide.md` |
| 22 | **Performance** | LOD and profiling | `Assets/PerformanceOptimizationGuide.md` |

### Documentation Access Pattern

For any system, follow this pattern:

1. **Quick Reference**: `[SYSTEM]_QUICK_REFERENCE.md` (root or Docs/)
2. **Complete Guide**: `Assets/[System]Guide.md`
3. **Workflow**: `Assets/[System]Workflow.md`
4. **Templates**: `Assets/[System]Templates/` directory
5. **API Reference**: `Assets/[System]APIReference.md`

---

## 6. Developer Guides

### Essential Developer Documentation

| Document | Purpose | Lines | Priority |
|----------|---------|-------|----------|
| **ARCHITECTURE.md** | System architecture, design patterns | 800+ | ⭐⭐⭐ |
| **CONTRIBUTING.md** | Contribution guidelines | 500+ | ⭐⭐⭐ |
| **CODE_STYLE.md** | C++ and Blueprint standards | 600+ | ⭐⭐⭐ |
| **ROADMAP.md** | Development phases, milestones | 1070 | ⭐⭐ |
| **PERFORMANCE_IMPROVEMENTS.md** | Optimization guidelines | 400+ | ⭐⭐ |
| **UNREAL_DIRECTIVE_SUGGESTIONS.md** | UE5 best practices | 1397 | ⭐⭐ |

### Advanced Topics

- **Plugin Development**: `PLUGIN_DEVELOPMENT_GUIDE.md` (719 lines)
- **Editor Customization**: `EDITOR_IMPLEMENTATION_GUIDE.md` (2029 lines)
- **MCP Integration**: `MCP_REMOTE_PYTHON_PLAN.md` (1158 lines)
- **AI Tool Integration**: `DIRECTOR_KNOWLEDGE_BASE.md` (936 lines)

### Key Concepts

**Data Asset Pattern** - Primary design pattern:
```cpp
UCLASS(BlueprintType)
class ADASTREA_API USpaceshipDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;
    
    // ... stats, properties, functions
};
```

**BlueprintNativeEvent Pattern** - Extensibility:
```cpp
UFUNCTION(BlueprintNativeEvent, Category="AI")
void DecideAction(AActor* Target);

void UNPCLogic::DecideAction_Implementation(AActor* Target)
{
    // C++ default implementation
    // Can be overridden in Blueprints
}
```

**Module Organization**:
- `Adastrea` - Core game systems (runtime)
- `AdastreaEditor` - Editor customizations (editor-only)
- `StationEditor` - Runtime station editing (runtime)

---

## 7. Blueprint Documentation

### Blueprint Learning Path

| Document | Level | Lines | Purpose |
|----------|-------|-------|---------|
| **BLUEPRINT_GUIDE_FOR_BEGINNERS.md** | Beginner | 1124 | Complete tutorial, zero knowledge assumed |
| **BLUEPRINT_QUICK_START_SUMMARY.md** | Beginner | 300+ | One-page printable summary |
| **BLUEPRINT_ROADMAP_VISUAL.md** | Intermediate | 400+ | Visual progression map |
| **BLUEPRINT_REQUIREMENTS_LIST.md** | All | 885 | Complete list of required Blueprints |
| **BLUEPRINT_CHECKLIST.md** | All | 372 | Quick creation checklist |
| **BLUEPRINT_CREATION_GUIDE.md** | Advanced | 500+ | Advanced best practices |

### Required Blueprints

**High Priority** (~20 Blueprints):
- BP_SpaceGameMode
- BP_AdastreaPlayerController
- BP_Spaceship (and variants)
- BP_SpaceStation
- BP_SpaceStationModule (8+ types)
- Input Data Assets
- HUD widgets

**Medium Priority** (~30 Blueprints):
- Faction Data Assets (10+)
- Personnel Data Assets (20+)
- Trading items
- Quest templates

**Low Priority** (~50+ Blueprints):
- Planet assets
- Visual effects
- Audio configurations
- Tutorial scenarios

### Blueprint Best Practices

1. **Naming Convention**: `BP_SystemName_Purpose`
   - Good: `BP_Ship_Fighter`, `BP_Station_TradeHub`
   - Bad: `NewBlueprint1`, `MyShip`

2. **Organization**: Use categories and comments
   - Group related nodes with comment boxes
   - Use reroute nodes for clean wiring
   - Add tooltips to complex variables

3. **Performance**: Minimize Tick usage
   - Use timers for periodic updates
   - Cache expensive calculations
   - Use BlueprintPure for getters

4. **Data Assets**: Prefer Data Assets over hardcoding
   - Easy to create variants
   - Designer-friendly
   - Version control friendly

---

## 8. Coding Standards

### C++ Style Guidelines

**Naming Conventions**:
- Classes: `UClassName` (UObject-derived), `AClassName` (AActor-derived)
- Structs: `FStructName`
- Enums: `EEnumName`
- Member variables: `PascalCase`
- Booleans: `bIsEnabled`
- Functions: `PascalCase` (verb-noun: `GetHealth()`, `CalculateDamage()`)

**File Structure**:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "YourClassName.generated.h"

// Forward declarations
class UOtherClass;

/**
 * Brief description
 * 
 * Detailed explanation
 */
UCLASS(BlueprintType)
class ADASTREA_API UYourClassName : public UDataAsset
{
    GENERATED_BODY()

public:
    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic")
    FText DisplayName;

    // Functions
    UFUNCTION(BlueprintCallable, Category="System")
    void DoSomething();
};
```

**UPROPERTY Best Practices**:
- Use `EditAnywhere, BlueprintReadWrite` for runtime values
- Use `EditAnywhere, BlueprintReadOnly` for configuration
- Use `VisibleAnywhere, BlueprintReadOnly` for display-only
- Always specify `Category`
- Use `meta=(ClampMin, ClampMax)` for ranges

**UFUNCTION Best Practices**:
- Mark `BlueprintCallable` if Blueprints should call
- Mark `BlueprintPure` for getters with no side effects
- Use `BlueprintNativeEvent` for overridable functions
- Always specify `Category`
- Add Doxygen comments

**Security**:
- Never hardcode secrets
- Validate all inputs
- Check pointers with `IsValid()`
- Initialize all pointers to `nullptr`
- Use `meta=(ClampMin, ClampMax)` for numeric inputs

For complete standards, see `CODE_STYLE.md`.

---

## 9. Designer Workflows

### System Creation Workflows

Each system has a documented workflow:

| System | Workflow Doc | Time Est. |
|--------|--------------|-----------|
| **Spaceship** | `Assets/SpaceshipDesignerWorkflow.md` | 30-60 min |
| **Personnel** | `Assets/PersonnelDesignerWorkflow.md` | 20-40 min |
| **Trading** | `Assets/TradingDesignerWorkflow.md` | 15-30 min |
| **AI** | `Assets/AIDesignerWorkflow.md` | 30-60 min |
| **Way Network** | `Assets/WayNetworkWorkflow.md` | 20-40 min |

### General Workflow Pattern

1. **Read System Guide** - Understand the system
2. **Review Templates** - See example YAML files
3. **Create Data Asset** - In Unreal Editor Content Browser
4. **Configure Properties** - Fill in all required fields
5. **Test in Editor** - Verify it loads without errors
6. **Integrate with Blueprints** - Reference from actor Blueprints
7. **Test in PIE** - Play In Editor to verify functionality

### Common Workflow Example: Creating a Ship

1. Read `Assets/SpaceshipDataAssetGuide.md`
2. Review `Assets/SpaceshipTemplates.md` for ship examples
3. Open Unreal Editor
4. Content Browser → Right-click → Create Data Asset → SpaceshipDataAsset
5. Name it `DA_Ship_YourShipName`
6. Configure all 7 stat categories:
   - Core (hull, cargo, crew)
   - Combat (weapons, shields, armor)
   - Mobility (speed, maneuverability, jump)
   - Utility (sensors, stealth, repair)
   - Operational (power, life support, maintenance)
   - Advanced (diplomacy, boarding, customization)
   - Lore (manufacturer, year, colors, notes)
7. Save Data Asset
8. Create or open BP_Spaceship Blueprint
9. Add reference to your Data Asset
10. Test in PIE

---

## 10. Automation & Scripts

### Python Automation Tools

| Script | Purpose | Usage |
|--------|---------|-------|
| **ProceduralGenerators.py** | Generate content | `python ProceduralGenerators.py --type all` |
| **ScenePopulator.py** | Populate levels | `python ScenePopulator.py --config template.json` |
| **YAMLtoDataAsset.py** | Import YAML | `python YAMLtoDataAsset.py --input ship.yaml` |
| **AutomationRunner.py** | Run tests | `python AutomationRunner.py --all` |
| **SmokeTest.py** | Smoke testing | `python SmokeTest.py` |
| **ScreenshotTester.py** | Visual regression | `python ScreenshotTester.py` |
| **AdastreaAssetValidator.py** | Validate assets | Run in UE Python console |

### Procedural Generation

Generate randomized content quickly:

```bash
# Generate everything
python ProceduralGenerators.py --type all

# Generate specific content
python ProceduralGenerators.py --type galaxy --count 10
python ProceduralGenerators.py --type npc --count 25
python ProceduralGenerators.py --type quest --count 20
python ProceduralGenerators.py --type ship --count 8
```

**What Gets Generated:**
- 🌌 **Galaxy Systems** - Star systems with planets, asteroids, stations
- 👥 **NPCs** - Crew members with skills, traits, backstories
- 📜 **Quests** - Missions with objectives, rewards, difficulty
- �� **Ship Variants** - Randomized designs with unique names

All content saved as YAML files compatible with Data Asset system.

See `PROCEDURAL_GENERATION_GUIDE.md` (822 lines) for complete documentation.

---

## 11. Testing & Quality Assurance

### Testing Infrastructure

**Available Testing Tools:**
- ✅ **Smoke Tests** - Load all maps, detect missing assets
- ✅ **Screenshot Tests** - Visual regression testing
- ✅ **Automation Runner** - CLI test orchestration
- ✅ **Asset Validator** - Validate Data Assets
- ✅ **C++ Unit Tests** - Unreal Automation framework
- ✅ **Python Tests** - pytest for scripts

**Testing Documentation:**
- `TESTING_AUTOMATION_GUIDE.md` (500+ lines)
- `TESTING_QUICK_REFERENCE.md` (200+ lines)
- `Assets/PlaytestingChecklist.md` (740 lines)
- `Assets/PIE_TESTING_GUIDE.md` (300+ lines)

### Running Tests

```bash
# Run all automated tests
python AutomationRunner.py --all

# Run only smoke tests
python AutomationRunner.py --smoke-test

# Run screenshot tests
python AutomationRunner.py --screenshot-test

# Schedule overnight validation
python AutomationRunner.py --all --schedule "23:00"
```

### Test Status

**Framework**: ✅ Implemented  
**Test Content**: ❌ Needs creation  
**CI/CD Integration**: ⚠️ Partial (GitHub Actions active)

**What's Needed:**
1. Create TestMode.umap level (specification exists)
2. Create test Data Assets for each system
3. Write test scenarios
4. Configure CI/CD to run tests automatically

---

## 12. Build & CI/CD

### GitHub Actions Setup

**Current Status**: ✅ Active for project validation  
**Docker Builds**: ⚠️ Requires Epic Games organization membership

**Documentation:**
- `GITHUB_TOKEN_DOCKER_SETUP.md` - Complete setup guide
- `DOCKER_SETUP_QUICK_REFERENCE.md` - One-page reference
- `CONTAINER_DIAGNOSTICS.md` - Troubleshooting guide
- `CLOUD_BUILD_QUICK_START.md` - Cloud build services
- `CLOUD_BUILD_SERVICES.md` - Service comparisons

### Docker Build Requirements

To use Unreal Engine containers:
1. Link Epic account at epicgames.com/account/connections
2. Accept Epic Games GitHub organization invitation
3. Verify membership at github.com/EpicGames

**Troubleshooting:**
- Run `./test_epic_connection.sh` for diagnostics
- Trigger "Diagnose Container Access" workflow
- See `CONTAINER_DIAGNOSTICS.md` for solutions

### Cloud Build Services

Available options:
- **GitHub Actions + Docker** - Free for public repos (active)
- **Epic Games Horde** - Native UE5 build platform
- **Unreal Cloud Build** - Managed service
- **TensorWorks Admiral** - Enterprise CI/CD
- **AWS Game Tech** - Comprehensive cloud infrastructure

### Local Build

```bash
# Validate environment
./SetupCheck.sh

# Generate project files
# Right-click .uproject → Generate Visual Studio project files

# Build (Visual Studio)
# Open .sln → Build → Development Editor

# Or build via Unreal Engine
# Open .uproject directly → Wait for compilation
```

---

## 13. Spaceship System

**Purpose**: Complete spaceship specification system with 6 stat categories

**Key Components:**
- `ASpaceship` - Base ship actor with FloatingPawnMovement
- `USpaceshipDataAsset` - Ship specification Data Asset
- 6 ship templates (Pathfinder, Warhammer, Merchant, Starliner, Excavator, Sovereign)

**Documentation:**
- Complete Guide: `Assets/SpaceshipDataAssetGuide.md` (800+ lines)
- Templates: `Assets/SpaceshipTemplates.md` (600+ lines)
- Workflow: `Assets/SpaceshipDesignerWorkflow.md` (500+ lines)
- Interior Guide: `Assets/SpaceshipInteriorGuide.md` (1238 lines)
- Control Guide: `Assets/ShipControlGuide.md` (400+ lines)
- YAML Templates: `Assets/SpaceshipTemplates/` (6 files)

**Stat Categories:**
1. **Core** - Hull, cargo, crew, modular points
2. **Combat** - Armor, shields, weapons, point defense
3. **Mobility** - Speed, acceleration, maneuverability, jump range, fuel
4. **Utility** - Sensors, stealth, repair, science, medical
5. **Operational** - Power, life support, maintenance, hangars, drones, AI
6. **Advanced** - Diplomacy, boarding, customization, electronic warfare, mining
7. **Lore** - Manufacturer, year, rarity, colors, notes

**Blueprint Functions:**
- `GetCombatRating()` - Calculate combat effectiveness (0-100)
- `GetMobilityRating()` - Calculate mobility (0-100)
- `GetUtilityRating()` - Calculate utility (0-100)
- `IsSuitableForRole(Role)` - Check role compatibility
- `GetSizeCategory()` - Get ship size classification
- `GetOperationalCost()` - Calculate maintenance cost

**Quick Usage:**
1. Create SpaceshipDataAsset in Content Browser
2. Use YAML templates as reference
3. Configure all 7 stat categories
4. Test with Blueprint functions
5. Integrate into ship Blueprint

---

## 14. Space Station System

**Purpose**: Modular space station construction with player editing

**Key Components:**
- `ASpaceStation` - Core station actor with module management
- `ASpaceStationModule` - Base module class
- `UStationEditorManager` - C++ editor manager (StationEditor module)
- `UStationModuleCatalog` - Available modules catalog
- `UStationEditorWidget` - Player UI widget
- `EStationModuleGroup` - Module categorization (Docking, Power, Storage, etc.)

**Documentation:**
- PIE Testing Guide: `Assets/StationEditorPIETestingGuide.md` (1109 lines) ⭐ **START HERE**
- Migration Guide: `Assets/StationEditorMigrationGuide.md` (400+ lines)
- System API: `Assets/StationEditorSystemGuide.md` (600+ lines)
- Management: `Assets/StationManagementGuide.md` (400+ lines)
- Blueprint Integration: `Content/Blueprints/STATION_EDITOR_README.md`

**Module Groups:**
- Docking - Ship bays and airlocks
- Power - Reactors and generators
- Storage - Cargo and resource management
- Defence - Weapons and shields
- Production - Manufacturing and refining
- Habitation - Living quarters
- Research - Science labs
- Communications - Sensors and comms

**Quick Usage:**
1. Create Blueprint extending ASpaceStation
2. Create module Blueprints extending ASpaceStationModule
3. Create DA_StationModuleCatalog Data Asset
4. Create Widget Blueprint extending UStationEditorWidget
5. In Player Controller:
   - Set StationEditorWidgetClass
   - Set ModuleCatalog
6. Bind key (M) to ToggleStationEditor()
7. C++ handles everything automatically (new in 2025)

---

## 15. Faction & Way Systems

### Faction System

**Purpose**: Factions with relationships and diplomacy

**Key Components:**
- `UFactionDataAsset` - Faction definition
- `FFactionTrait` - Faction characteristics
- `FFactionRelationship` - Inter-faction relationships
- 10 pre-made faction templates

**Documentation:**
- Setup Guide: `Assets/FactionSetupGuide.md` (600+ lines)
- Quick Reference: `Assets/FactionSystemQuickReference.md`
- Blueprint Guide: `Assets/BlueprintFactionAssetGuide.md`
- AI Setup: `Assets/FactionAISetupGuide.md` (500+ lines)

### Way System (Guilds)

**Purpose**: Specialized guilds with reputation networks

**Key Components:**
- `UWayDataAsset` - Guild definition
- `UWayNetworkDataAsset` - Guild alliances (2-5 Ways)
- `UVerseSubsystem` - Global reputation management
- `UVerse` - Player reputation component
- 15 industry types
- Precepts and Feats system

**Documentation:**
- Index: `Assets/WAY_SYSTEM_INDEX.md`
- Complete Guide: `Assets/WaySystemGuide.md` (800+ lines)
- API Reference: `Assets/WaySystemAPIReference.md` (1011 lines)
- Network Guide: `Assets/WayNetworkGuide.md` (600+ lines)
- Quick Start: `Assets/WayNetworkQuickStart.md`
- Workflow: `Assets/WayNetworkWorkflow.md` (400+ lines)
- YAML Templates: `Assets/WayNetworkTemplates/` (5 networks)

**Example Networks:**
- Craftsmen's Compact - Engineering excellence
- Frontier Alliance - Exploration and discovery
- Honor Circle - Warrior traditions
- Merchant Coalition - Trade and commerce
- Scholars' Syndicate - Knowledge and research

**Key Features:**
- Reputation spillover (actions affect entire network)
- Network bonuses (multipliers for aligned actions)
- Sector councils (governance participation)
- Supply chain economics

---

## 16. AI System

**Purpose**: Personality-driven NPCs and strategic faction AI

**Key Components:**
- `UNPCLogicBase` - Base AI class with BlueprintNativeEvent
- `UFactionLogic` - Strategic faction-level AI
- `UPersonnelLogic` - Individual crew AI
- 8 personality dispositions
- Early-game peaceful priorities

**Documentation:**
- Quick Start: `Assets/AI_QUICK_START.md`
- Integration: `Assets/AISystemIntegration.md` (741 lines)
- Faction AI: `Assets/FactionAISetupGuide.md` (500+ lines)
- Personnel AI: `Assets/PersonnelAISetupGuide.md` (698 lines)
- Designer Workflow: `Assets/AIDesignerWorkflow.md` (400+ lines)
- YAML Templates: `Assets/FactionAITemplates/`, `Assets/PersonnelAITemplates/`

**Personality Dispositions:**
1. Curious - Exploration and research
2. Disciplined - Protocol adherence
3. Empathetic - People-focused, supportive
4. Pragmatic - Efficiency and results
5. Bold - Risk-taking, decisive
6. Cautious - Safety-first, methodical
7. Analytical - Logic and calculation
8. Creative - Innovation and unconventional

**Features:**
- BlueprintNativeEvent pattern (C++ + Blueprint flexibility)
- Peaceful early-game focus
- Relationship dynamics
- Personality compatibility
- Command hierarchy support

---

## 17. Trading & Economy

**Purpose**: Dynamic supply/demand trading with faction pricing

**Key Components:**
- `UTradingDataAsset` - Trade item definition
- `UMarketComponent` - Dynamic pricing
- `UContractComponent` - Quest integration
- Supply/demand simulation
- Faction relationship pricing

**Documentation:**
- Complete Guide: `Assets/TradingSystemGuide.md` (1217 lines)
- Designer Workflow: `Assets/TradingDesignerWorkflow.md` (883 lines)
- API Reference: `Assets/TradingAPIReference.md` (843 lines)
- Blueprint Examples: `Assets/TradingBlueprintExamples.md` (821 lines)
- Blueprint Guide: `Blueprints/TradingSystemBlueprintGuide.md`
- YAML Templates: `Assets/TradingTemplates/`

**Features:**
- Dynamic supply/demand
- Faction relationship-based pricing
- Contract quest system
- AI-driven autonomous trading
- Resource chains and production

---

## 18. Combat & Exploration

### Combat System

**Purpose**: Weapons, shields, directional damage, and boarding

**Key Components:**
- 6 weapon types (Laser, Kinetic, Missile, etc.)
- Directional shields (Front, Rear, Port, Starboard, Top, Bottom)
- Point defense systems
- 5-phase boarding mechanics

**Documentation:**
- Complete Guide: `Assets/CombatSystemGuide.md` (800+ lines)
- Quick Reference: `Assets/CombatQuickReference.md`
- VFX System: `Assets/CombatVFXSystemGuide.md` (988 lines)
- VFX Workflow: `Assets/CombatVFXWorkflow.md` (911 lines)

### Exploration System

**Purpose**: Scanners, sensors, and object discovery

**Key Components:**
- `UScannerComponent` - Ship scanner
- `AScannableObject` - Discoverable objects
- 7 scanner types (Basic, Advanced, Military, Mining, Science, Stealth, Quantum)
- 9 object categories

**Documentation:**
- Complete Guide: `Assets/ExplorationSystemGuide.md` (1091 lines)
- YAML Templates: `Assets/ExplorationTemplates/`

---

## 19. UI & Player Systems

### HUD/UI System

**Purpose**: In-game interface and widgets

**Key Components:**
- `UAdastreaHUDWidget` - Main HUD
- `UShipStatusWidget` - Ship status screen
- `UInventoryWidget` - Inventory management
- `UTradingInterfaceWidget` - Trading UI
- `UStationManagementWidget` - Station management

**Documentation:**
- Complete Guide: `Assets/HUDSystemGuide.md` (600+ lines)
- Quick Reference: `Assets/HUD_QUICK_REFERENCE.md`
- Implementation: `Docs/HUD_IMPLEMENTATION_SUMMARY.md`
- Usage: `Docs/HUD_USAGE_GUIDE.md`

### Player Progression

**Purpose**: Leveling, reputation, unlocks

**Key Components:**
- `UPlayerProgressionComponent` - XP and leveling
- `UPlayerReputationComponent` - Faction reputation
- `UPlayerUnlockComponent` - Achievement unlocks
- Ship upgrades and customization

**Documentation:**
- Complete Guide: `Assets/PlayerProgressionGuide.md` (708 lines)

### Inventory System

**Purpose**: Item management and storage

**Documentation:**
- Complete Guide: `Assets/InventorySystemGuide.md` (701 lines)
- YAML Templates: `Assets/InventoryTemplates/`

---

## 20. Complete Documentation Index

### Documentation Statistics

- **Total Markdown Files**: 150+
- **Total Documentation Lines**: 130,880+
- **Root Level**: 40 MD files
- **Assets/ Directory**: 97 MD files
- **Docs/ Directory**: 27 MD files
- **Plugins/ Directory**: 30 MD files
- **Other Directories**: 20+ MD files

### Quick Access by Category

**Essential** (Must Read):
- README.md (841 lines)
- ARCHITECTURE.md (800+ lines)
- CONTRIBUTING.md (500+ lines)
- CODE_STYLE.md (600+ lines)
- DOCUMENTATION_INDEX.md (627 lines)

**Quick Starts**:
- QUICKSTART.md
- BLUEPRINT_GUIDE_FOR_BEGINNERS.md (1124 lines)
- Assets/DesignerOnboarding.md (849 lines)
- PROCEDURAL_GENERATION_QUICKSTART.md

**System Guides** (all in Assets/):
- SpaceshipDataAssetGuide.md (800+ lines)
- StationEditorPIETestingGuide.md (1109 lines)
- TradingSystemGuide.md (1217 lines)
- [See section 5 for complete list]

**Developer Resources**:
- EDITOR_IMPLEMENTATION_GUIDE.md (2029 lines)
- UNREAL_DIRECTIVE_SUGGESTIONS.md (1397 lines)
- PLUGIN_DEVELOPMENT_GUIDE.md (719 lines)
- MCP_REMOTE_PYTHON_PLAN.md (1158 lines)

**Build & CI/CD**:
- GITHUB_TOKEN_DOCKER_SETUP.md
- DOCKER_SETUP_QUICK_REFERENCE.md
- CONTAINER_DIAGNOSTICS.md
- CLOUD_BUILD_SERVICES.md

**Testing**:
- TESTING_AUTOMATION_GUIDE.md (500+ lines)
- Assets/PlaytestingChecklist.md (740 lines)
- Assets/PIE_TESTING_GUIDE.md

For complete file index, see `DOCUMENTATION_INDEX.md`.

---

## 21. YAML Templates Reference

### Template Directories

| Directory | Templates | Purpose |
|-----------|-----------|---------|
| `Assets/SpaceshipTemplates/` | 6 files | Ship configurations |
| `Assets/FactionAITemplates/` | 3+ files | Faction AI behaviors |
| `Assets/PersonnelAITemplates/` | 5+ files | Personnel AI personalities |
| `Assets/PersonnelTemplates/` | 7 files | Crew member profiles |
| `Assets/TradingTemplates/` | 10+ files | Trade items and markets |
| `Assets/CombatTemplates/` | 5+ files | Combat configurations |
| `Assets/InventoryTemplates/` | 10+ files | Inventory items |
| `Assets/WayNetworkTemplates/` | 5 files | Guild networks |
| `Assets/SectorTemplates/` | 7 files | Sector councils |
| `Assets/ExplorationTemplates/` | 2 files | Scanners and objects |

### Using YAML Templates

1. **Browse Templates** - Find relevant YAML file in Assets/
2. **Copy Template** - Copy YAML content
3. **Modify Values** - Adjust for your needs
4. **Import (Optional)** - Use YAMLtoDataAsset.py
5. **Or Create in Editor** - Use YAML as reference for manual creation

### YAML Conversion Status

**Total YAML Files**: 161  
**Converted**: 0 (all need Data Asset creation in editor)  
**See**: `YAML_TO_ASSETS_CONVERSION_LIST.md` for complete inventory

---

## 22. Troubleshooting

### Common Issues

**Problem**: "Project won't compile"
- Run `SetupCheck.py` to validate environment
- Check Visual Studio 2022 is installed
- Verify UE 5.6 is installed
- Regenerate project files

**Problem**: "Can't find Data Assets"
- Data Assets haven't been created yet
- Create them in editor using YAML templates
- See `BLUEPRINT_REQUIREMENTS_LIST.md`

**Problem**: "PIE crashes or nothing spawns"
- TestMode.umap doesn't exist yet
- Use TestLevel.umap instead
- Create ship Blueprint first
- Configure input mappings

**Problem**: "Docker build fails"
- Check Epic Games org membership
- Run `./test_epic_connection.sh`
- See `CONTAINER_DIAGNOSTICS.md`

**Problem**: "Python scripts don't work"
- Ensure Python 3.9+ installed
- Install dependencies: `pip install -r requirements.txt`
- Enable Python plugin in UE
- Check script documentation

### Documentation Resources

- **Setup Issues**: `SetupCheck.py`, `SETUP_CHECKLIST.md`
- **Build Issues**: `build_errors.md`, `GITHUB_TOKEN_DOCKER_SETUP.md`
- **Editor Issues**: `PIE_FIX_CHECKLIST.md`, `QUICK_PIE_FIX.md`
- **Blueprint Issues**: `BLUEPRINT_GUIDE_FOR_BEGINNERS.md`
- **Testing Issues**: `TESTING_AUTOMATION_GUIDE.md`

### Getting Help

1. Check documentation index for relevant guides
2. Review system-specific troubleshooting sections
3. Search closed GitHub issues
4. Open new issue with reproduction steps
5. Include system info and error messages

---

## 23. Archived Documentation

### Archive Location

Migrated documentation is located in:
- `Docs/archive/migrated/` - Consolidated historical documents
- `Docs/archive/` - Other archived files

### What Gets Archived

- **Status Documents** - PROJECT_STATUS_AND_IMPROVEMENTS.md, IMPLEMENTATION_COMPLETE.md
- **Completion Summaries** - TASK_COMPLETION_SUMMARY.md, WIKI_IMPLEMENTATION_SUMMARY.md
- **Historical Guides** - Temporary implementation guides
- **Phase Summaries** - PHASE2_SUMMARY.md, PHASE3_COMPLETION_SUMMARY.md
- **Obsolete Docs** - ZZ_* prefixed files (see OBSOLETE_DOCUMENTS.md)

### Active Documentation

**Keep Active** (Not Archived):
- README.md - Project overview
- ARCHITECTURE.md - System architecture
- CONTRIBUTING.md - Contribution guidelines
- ROADMAP.md - Development plan
- CODE_STYLE.md - Coding standards
- DOCUMENTATION_INDEX.md - Documentation index
- All system guides in Assets/
- All workflow documents
- All templates and examples

### Finding Archived Docs

1. Check `OBSOLETE_DOCUMENTS.md` for ZZ_* files
2. Look in `Docs/archive/migrated/` for consolidated docs
3. Check git history for removed files
4. See "Superseded By" links in OBSOLETE_DOCUMENTS.md

---

## 24. Version History

### Version 1.0.0-alpha (Current)

**Code Status**: 85-90% complete  
**Content Status**: 10-15% complete  
**Phase**: Alpha - Core systems implemented

**Recent Updates** (November 2025):
- ✅ Audio category tracking fully implemented
- ✅ PlayerUnlockComponent requirements integrated
- ✅ Legacy documentation cleanup (9 ZZ_* files removed)
- ✅ Python testing framework added
- ✅ C++ automation tests added
- ✅ GitHub Actions CI/CD configured

**What's Complete**:
- 22 major game systems (C++)
- 209 C++ files
- 150+ documentation files
- Python automation scripts
- Testing framework
- Blueprint exposure

**What's Needed**:
- Blueprint Data Assets (~100+)
- Actor Blueprints (~50+)
- Game maps (~10+)
- UI widgets (~30+)
- Audio/visual assets (~100+)
- Input configuration

### Roadmap

See `ROADMAP.md` for complete development plan:

**Phase 1-3**: Core Systems (✅ COMPLETE)
**Phase 4**: Gameplay & Polish (⚠️ IN PROGRESS - 20%)
**Phase 5**: Content & Beta (📋 PLANNED - 2026)
**Phase 6**: Release Preparation (📋 PLANNED - 2027)

### Changelog

For detailed version history, see `CHANGELOG.md`.

---

## 25. Contributing

### How to Contribute

1. **Read Guidelines**: See `CONTRIBUTING.md`
2. **Check Issues**: Look for "good first issue" or "help wanted"
3. **Fork & Clone**: Fork repo and create feature branch
4. **Make Changes**: Follow coding standards
5. **Test**: Ensure changes work in editor
6. **Submit PR**: Open pull request with clear description

### Ways to Contribute

- 🐛 **Bug Reports** - Found a bug? Open an issue
- ✨ **New Features** - Have an idea? Discuss first
- 📚 **Documentation** - Improve guides, fix typos
- 🎨 **Assets** - Create ships, factions, content
- 🧪 **Testing** - Test features and report issues
- 🔧 **Editor Content** - Create Blueprints and Data Assets

### Contribution Priority

**HIGH PRIORITY** (Most Needed):
1. Create Blueprint Data Assets (ships, factions, personnel)
2. Create Actor Blueprints (gameplay actors)
3. Create game maps and levels
4. Design UI widgets
5. Write test scenarios

**MEDIUM PRIORITY**:
1. Improve documentation
2. Add examples and tutorials
3. Create YAML templates
4. Fix bugs
5. Optimize performance

**LOW PRIORITY**:
1. New C++ features (code is mostly complete)
2. Refactoring (unless fixing bugs)
3. Plugin development

### Code of Conduct

- Be respectful and constructive
- Welcome newcomers
- Focus on the work, not the person
- Assume good intentions
- Give credit where due

---

## Conclusion

This master documentation provides a comprehensive overview of the Adastrea project. For specific systems, workflows, or troubleshooting, refer to the individual documentation files indexed above.

**Key Takeaways:**

1. **Code is 85-90% complete** - C++ systems are implemented
2. **Content is 10-15% complete** - Editor assets need creation
3. **Documentation is comprehensive** - 150+ guides available
4. **Blueprint creation is the priority** - Use YAML templates as reference
5. **Testing framework exists** - Content creation needed

**Next Steps:**

1. Read `BLUEPRINT_REQUIREMENTS_LIST.md` for what to create
2. Use `Assets/DesignerOnboarding.md` for designer workflow
3. Follow system guides in `Assets/` for each system
4. Create Data Assets using YAML templates as reference
5. Test in PIE (Play In Editor) as you build

**Remember**: The project is code-complete but not content-complete. Focus on creating editor content (Blueprints, Data Assets, Maps, UI) to make the game playable.

---

**Documentation Version**: 1.0  
**Last Updated**: December 9, 2025  
**Maintained By**: Adastrea Development Team  
**License**: MIT

For the latest updates, see the repository on GitHub: https://github.com/Mittenzx/Adastrea
