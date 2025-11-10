# Adastrea - Directory Structure Summary

This document provides a comprehensive overview of the Adastrea project's directory structure, file organization, and the purpose of each major component.

## 📁 Root Directory

The root directory contains project configuration files, documentation, and organizational materials:

| File/Directory | Purpose |
|----------------|---------|
| `Adastrea.uproject` | Unreal Engine project file - main project definition |
| `README.md` | Primary project documentation and overview |
| `LICENSE` | MIT license information |
| `.gitignore` | Git version control exclusions |
| `.editorconfig` | Editor configuration for consistent coding style |
| `.vsconfig` | Visual Studio configuration |
| `.markdownlint.json` | Markdown linting rules |
| `shadertoolsconfig.json` | Shader tools configuration |
| `validator_config.json` | Asset validator configuration |

### Setup and Validation Scripts
| File | Purpose |
|------|---------|
| `SetupCheck.py` | Python setup validation script |
| `SetupCheck.sh` | Shell setup validation script |
| `AdastreaAssetValidator.py` | Python asset validation tool |
| `move-and-update-includes.sh` | Script to organize and update C++ includes |

### Documentation Files (Root)
| File | Purpose |
|------|---------|
| `ARCHITECTURE.md` | Overall project architecture and design patterns |
| `CODE_STYLE.md` | C++ and Blueprint coding standards |
| `CONTRIBUTING.md` | Guidelines for contributing to the project |
| `CHANGELOG.md` | Version history and release notes |
| `CHEATSHEET.md` | Quick reference for common tasks |
| `QUICKSTART.md` | Getting started guide |
| `TODO_Mittenzx.md` | Developer todo list |
| `WHATS_STILL_NEEDED.md` | Planned features and improvements |

### Implementation Summaries (Root)
| File | Purpose |
|------|---------|
| `IMPLEMENTATION_SUMMARY.md` | General implementation overview |
| `AUDIT_SUMMARY.md` | Code audit findings |
| `CODE_REVIEW_SUMMARY.md` | Code review results |
| `PERFORMANCE_IMPROVEMENTS.md` | Performance optimization notes |
| `MITTENZX_MK1_IMPLEMENTATION.md` | Mittenzx Mk1 ship implementation details |

### Specialized Guides (Root)
| File | Purpose |
|------|---------|
| `BUILD_TROUBLESHOOTING.md` | Solutions for common build issues |
| `BLUEPRINT_CREATION_GUIDE.md` | Blueprint asset creation workflow |
| `CONTENT_CREATION_QUICKSTART.md` | Quick start for content creators |
| `ENHANCED_INPUT_GUIDE.md` | Enhanced Input system documentation |
| `EXAMPLE_DATA_ASSETS.md` | Examples of data assets |
| `OPTIMIZATION_QUICK_REFERENCE.md` | Performance optimization tips |
| `SCRIPTS_README.md` | Documentation for project scripts |

### Editor-Specific Documentation (Root)
| File | Purpose |
|------|---------|
| `ADASTREA_EDITOR_ANALYSIS.md` | Editor customization analysis |
| `ADASTREA_EDITOR_NEXT_STEPS.md` | Future editor improvements |
| `ADASTREA_EDITOR_SUMMARY.md` | Editor features summary |

---

## 📂 Source/

Contains all C++ source code organized by game system.

### Source/Adastrea/
**Main game module** - Core game systems and functionality.

| Subdirectory | Contents | Purpose |
|--------------|----------|---------|
| `AI/` | AI logic classes | NPC behavior, faction AI, personnel AI systems |
| `Characters/` | Character classes | Player and NPC character implementations |
| `Factions/` | Faction system | Faction data assets, relationships, diplomacy |
| `Materials/` | Material system | Raw materials, resources, material management |
| `Planets/` | Planet classes | Planet generation, homeworlds, celestial bodies |
| `Player/` | Player systems | Player controller, pawn, input handling |
| `Ships/` | Spaceship classes | Ship data assets, ship systems, controls |
| `Stations/` | Space station classes | Modular stations, station management |
| `Trading/` | Trading system | Economy, contracts, supply/demand |
| `UI/` | User interface | HUD, menus, widgets |
| `Public/` | Public headers | Blueprint-exposed headers organized by system |

**Key Files:**
- `Adastrea.Build.cs` - Unreal Build Tool configuration
- `Adastrea.h/cpp` - Module initialization
- `AdastreaGameMode.h/cpp` - Game mode implementation
- `AdastreaLog.cpp` - Logging categories
- `SpaceSectorMap.h/cpp` - Galaxy map and sector navigation

### Source/StationEditor/
**In-game station editor** - Runtime module for player-facing station construction.

Contains the player-accessible station editor UI and gameplay systems that allow players to build and modify space stations during gameplay. This is a runtime module available in both editor and packaged builds.

**Key Components:**
- `StationEditor.Build.cs` - Module build configuration
- `StationEditor.h/cpp` - Module initialization
- `UI/StationEditorWidget.h/cpp` - Station editor UI widget

### Source/PlayerMods/
**Player modification system** - Modding support and extensibility.

Separate module for player-created modifications and extensions to keep them isolated from core game code.

---

## 📂 Content/

Unreal Engine content browser assets (Blueprints, materials, textures, etc.).

| Subdirectory | Contents | Purpose |
|--------------|----------|---------|
| `Blueprints/` | Blueprint classes | Visual scripting implementations of game systems |
| `Docs/` | Content documentation | Markdown files specific to content organization |
| `Homeworlds/` | Homeworld assets | Faction starting locations and planetary systems |
| `Maps/` | Level maps | Game levels, test maps, scene setups |
| `Planets/` | Planet assets | Planet meshes, materials, data assets |
| `SpaceStations/` | Station assets | Station modules, blueprints, prefabs |
| `UI/` | UI widgets | User interface blueprints and materials |

**Key File:**
- `CONTENT_ORGANIZATION.md` - Content folder structure guidelines

---

## 📂 Assets/

**Design documentation and templates** - Designer-friendly guides, workflows, and YAML templates.

### System Guides
| File | Purpose |
|------|---------|
| `DesignerOnboarding.md` | Comprehensive onboarding for content designers |
| `IterationGuide.md` | Workflow for iterative development |
| `PlaytestingChecklist.md` | Testing and validation checklist |

### Spaceship System
| File | Purpose |
|------|---------|
| `SPACESHIP_SYSTEM_SUMMARY.md` | Overview of spaceship architecture |
| `SpaceshipDataAssetGuide.md` | Creating ship data assets |
| `SpaceshipDesignerWorkflow.md` | Ship design workflow |
| `SpaceshipInteriorGuide.md` | Interior design and layout |
| `SpaceshipTemplates.md` | Template documentation |
| `ShipControlGuide.md` | Ship control systems |
| `ShipControlImplementationSummary.md` | Control implementation details |
| `MittenzxMk1SetupGuide.md` | Specific ship setup example |
| `SpaceshipTemplates/` | YAML templates for ships |

### Faction System
| File | Purpose |
|------|---------|
| `FactionSetupGuide.md` | Creating and configuring factions |
| `FactionSystemQuickReference.md` | Quick reference for faction features |
| `BlueprintFactionAssetGuide.md` | Blueprint faction workflows |

### Personnel System
| File | Purpose |
|------|---------|
| `PERSONNEL_IMPLEMENTATION_SUMMARY.md` | Personnel system architecture |
| `PersonnelSystemGuide.md` | Personnel management overview |
| `PersonnelDesignerWorkflow.md` | Creating and managing crew |
| `PersonnelTemplates.md` | Template documentation |
| `PersonnelTemplates/` | YAML templates for personnel |

### AI System
| File | Purpose |
|------|---------|
| `AI_IMPLEMENTATION_SUMMARY.md` | AI architecture overview |
| `AI_QUICK_START.md` | Quick start for AI setup |
| `AISystemIntegration.md` | AI system integration guide |
| `AIDesignerWorkflow.md` | Designer workflow for AI |
| `FactionAISetupGuide.md` | Faction-level AI configuration |
| `PersonnelAISetupGuide.md` | Personnel AI behavior setup |
| `FactionAITemplates/` | YAML templates for faction AI |
| `PersonnelAITemplates/` | YAML templates for personnel AI |

### Trading System
| File | Purpose |
|------|---------|
| `TRADING_IMPLEMENTATION_SUMMARY.md` | Trading system architecture |
| `TradingSystemGuide.md` | Complete trading system guide |
| `TradingDesignerWorkflow.md` | Designer workflow for trading |
| `TradingAPIReference.md` | API documentation |
| `TradingBlueprintExamples.md` | Blueprint usage examples |
| `TradingTemplates/` | YAML templates for trading items |

### Material System
| File | Purpose |
|------|---------|
| `MATERIAL_SYSTEM_SUMMARY.md` | Material system overview |
| `MaterialSystemGuide.md` | Complete material guide |
| `MaterialQuickStart.md` | Quick start guide |
| `MaterialStationIntegration.md` | Station integration |
| `MaterialTemplates.yaml` | YAML templates |

### Homeworld System
| File | Purpose |
|------|---------|
| `HOMEWORLD_IMPLEMENTATION_SUMMARY.md` | Homeworld system architecture |
| `HomeworldSystemGuide.md` | Homeworld creation and setup |

### Blueprint Workflows
| File | Purpose |
|------|---------|
| `BlueprintWorkflowTemplates.md` | Common Blueprint patterns |

---

## 📂 Blueprints/

**Blueprint documentation** - Guides specific to Blueprint implementations.

| File | Purpose |
|------|---------|
| `HomeworldBlueprintTemplates.md` | Blueprint templates for homeworlds |
| `TradingSystemBlueprintGuide.md` | Trading system Blueprint usage |

---

## 📂 Docs/

**Technical documentation** - Setup guides and technical references.

| File | Purpose |
|------|---------|
| `BEST_PRACTICES_CLASS_SETUP.md` | C++ class setup best practices |
| `CLASS_SETUP_AUDIT_SUMMARY.md` | Class setup audit results |
| `EmptyMapSetup.md` | Creating empty test maps |
| `TestMode_INDEX.md` | Test mode documentation index |
| `TestMode_Layout_Diagram.txt` | Test mode layout visualization |
| `TestMode_QuickReference.md` | Quick reference for test modes |

---

## 📂 Config/

**Unreal Engine configuration files** - Engine and game settings.

| File | Purpose |
|------|---------|
| `DefaultEngine.ini` | Engine configuration (rendering, physics, etc.) |
| `DefaultGame.ini` | Game-specific settings |
| `DefaultInput.ini` | Input bindings and controls |
| `DefaultEditor.ini` | Editor preferences and settings |
| `README.md` | Configuration documentation |

---

## 📂 Plugins/

**Third-party plugins** - External Unreal Engine plugins.

Currently contains documentation for plugin management. Plugins can be added here for extended functionality.

---

## 📂 tests/

**Test files** - Unit tests and validation code.

Contains C++ test files for automated testing. Currently has a placeholder test file structure.

---

## 📂 .github/

**GitHub configuration** - CI/CD workflows and repository settings.

| Subdirectory | Purpose |
|--------------|---------|
| `.github/workflows/` | GitHub Actions CI/CD pipeline definitions |

---

## System Architecture Summary

The Adastrea project is organized around **data-driven, modular systems**:

### Core Systems
1. **Spaceship System** - Data asset-based ship definitions with comprehensive stats
2. **Faction System** - Diplomacy, relationships, and faction-specific behaviors
3. **Personnel System** - Crew management with 31 role types and AI personalities
4. **Space Station System** - Modular construction with faction integration
5. **Trading System** - Dynamic economy with supply/demand simulation
6. **AI System** - BlueprintNativeEvent-based AI for factions and personnel
7. **Material System** - Raw materials and resource management
8. **Homeworld System** - Faction starting locations and planetary systems

### Design Philosophy
- **Blueprint First**: All systems fully exposed to Blueprints
- **Designer-Friendly**: YAML templates and comprehensive documentation
- **Data Assets**: Configuration through Unreal's data asset system
- **Modular**: Independent systems with clear integration points
- **Well-Documented**: Extensive guides for both designers and programmers

### File Organization Pattern
```
System Documentation (Assets/):
  - [System]Guide.md - Complete overview
  - [System]DesignerWorkflow.md - Step-by-step workflow
  - [System]Templates.md - Template documentation
  - [System]Templates/ - YAML configuration files
  - [SYSTEM]_IMPLEMENTATION_SUMMARY.md - Technical details

C++ Implementation (Source/Adastrea/[System]/):
  - Data asset classes
  - System logic
  - Blueprint-callable functions
  - Public headers in Source/Adastrea/Public/[System]/

Blueprint Assets (Content/[System]/):
  - Blueprint implementations
  - Visual scripting
  - Designer-created content
```

---

## Quick Navigation

**For Designers:**
- Start with `Assets/DesignerOnboarding.md`
- Use YAML templates in `Assets/[System]Templates/`
- Follow workflows in `Assets/[System]DesignerWorkflow.md`

**For Programmers:**
- Read `ARCHITECTURE.md` for system design
- Follow `CODE_STYLE.md` for coding standards
- Check `CONTRIBUTING.md` for contribution guidelines
- Explore `Source/Adastrea/` for C++ implementations

**For Quick Start:**
- See `QUICKSTART.md` for initial setup
- Use `CHEATSHEET.md` for common tasks
- Run `SetupCheck.py` or `SetupCheck.sh` to validate setup

**For Content Creation:**
- Review `CONTENT_CREATION_QUICKSTART.md`
- Check `BLUEPRINT_CREATION_GUIDE.md`
- Explore `Assets/` for system-specific guides

---

**Last Updated:** 2025-11-07  
**Project:** Adastrea - Open World Space Flight Game  
**Engine:** Unreal Engine 5.x  
**License:** MIT
