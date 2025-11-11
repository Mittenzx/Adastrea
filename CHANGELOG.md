# Changelog

All notable changes to the Adastrea project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added - Comprehensive Project Roadmap (2025-11-10)
- **Project Roadmap**: Created comprehensive ROADMAP.md with detailed development plan
  - Added complete project vision and core pillars
  - Documented 6 development phases from Foundation to Release & Beyond
  - Included detailed feature roadmap for all major systems
  - Added technical roadmap with engine updates and optimization plans
  - Documented community and modding ecosystem plans
  - Created system maturity matrix showing current state of all systems
  - Added risk management section with mitigation strategies
  - Included contribution guidelines for all contributor types
  - Timeline spans from Q4 2025 (completed) through 2027 (release and beyond)
  - Updated README.md to reference the new roadmap in multiple sections
  - Added roadmap link to Documentation section

**Roadmap Highlights**:
- **Phase 1-2 (Complete)**: Foundation and core systems including all 8 major game systems
- **Phase 3 (In Progress)**: Advanced systems - Combat, Navigation, Quest, Exploration
- **Phase 4 (Q1-Q2 2026)**: Gameplay & Polish - Player progression, save system, UI/UX
- **Phase 5 (Q3-Q4 2026)**: Content & Beta - Substantial content creation, procedural generation
- **Phase 6 (2027+)**: Release & Beyond - Public launch, ongoing support, expansions

**System Maturity**:
- 8 core systems at 75-95% completion
- 3 advanced systems at 10-60% completion (in progress)
- 3 future systems in planning phase

### Changed - Unreal Engine Version Update (2025-11-07)
- **Engine Version Update**: Updated project from Unreal Engine 5.5 to 5.6.1
  - Updated `Adastrea.uproject` EngineAssociation from "5.5" to "5.6"
  - Updated validation scripts:
    - `SetupCheck.py` - updated required_version to "5.6"
    - `SetupCheck.sh` - updated REQUIRED_UE_VERSION to "5.6"
  - Updated documentation files:
    - `README.md` - Prerequisites and version compatibility references
    - `CONTRIBUTING.md` - Prerequisites section
    - `ARCHITECTURE.md` - System dependencies
    - `BUILD_TROUBLESHOOTING.md` - All UE version references and documentation links
    - `TESTING_AUTOMATION_GUIDE.md` - Prerequisites
    - `GENERATOR_SCRIPTS_README.md` - FAQ section
    - `YAML_IMPORT_GUIDE.md` - Requirements
    - `CONTENT_CREATION_QUICKSTART.md` - Prerequisites
    - `SCRIPTS_README.md` - Version checks and requirements
    - `Docs/EmptyMapSetup.md` - Overview and prerequisites
    - `Docs/TestMode_INDEX.md` - Prerequisites
    - `Plugins/README.md` - Documentation links
    - `GuideGenerator.py` - API reference template
- **Build Compatibility**: No code changes required - UE 5.6.1 maintains backward compatibility with existing C++ code and Build.cs configurations

**Migration Notes:**
- When pulling these changes, regenerate project files in Unreal Engine 5.6.1:
  1. Right-click `Adastrea.uproject` and select "Switch Unreal Engine version..." to 5.6
  2. Generate Visual Studio project files
  3. Rebuild in "Development Editor" configuration
- All existing Data Assets, Blueprints, and content are compatible with 5.6.1
- No API changes affect current codebase

### Added - Test Mode Level Documentation (2025-11-05)
- **Test Mode Level Specification**: Created comprehensive documentation for Test Mode level
  - Added `Content/Maps/TestMode.umap.txt` with complete level specification
  - Added `Content/Blueprints/BP_TestModeSetup_Guide.txt` with Blueprint setup instructions
  - Added `Docs/TestMode_QuickReference.md` for quick reference
  - Updated README.md to reference Test Mode documentation
- **Test Mode Features**:
  - Simple floor for spatial reference
  - Player start position at (0, 0, 100)
  - Spaceship spawn point at (500, 0, 200) with tag "SpaceshipSpawnPoint"
  - Basic lighting setup (Directional Light, Sky Light, Sky Atmosphere)
  - Game Mode configuration for automatic spaceship spawning
- **Documentation Purpose**: Provides step-by-step instructions for designers to create a minimal test level in Unreal Editor for rapid gameplay testing

**Setup Instructions:**
1. Follow the guide in `Docs/TestMode_QuickReference.md` for quick setup
2. See `Content/Maps/TestMode.umap.txt` for detailed level specification
3. See `Content/Blueprints/BP_TestModeSetup_Guide.txt` for Blueprint creation

### Changed - Name Normalization (2025-10-18)
- **Module Renaming**: Renamed `AdAstreaEditor` module to `AdastreaEditor` to match canonical project name
  - Renamed directory: `Source/AdAstreaEditor/` → `Source/AdastreaEditor/`
  - Renamed files: `AdAstreaEditor.Build.cs`, `AdAstreaEditor.cpp`, `AdAstreaEditor.h` → `AdastreaEditor.*`
  - Updated class names: `FAdAstreaEditorModule` → `FAdastreaEditorModule`
  - Updated module name in `Adastrea.uproject`: `"AdAstreaEditor"` → `"AdastreaEditor"`
  - Updated `AdastreaEditor.Target.cs` to reference correct module name
- **GameMode Renaming**: Renamed GameMode class to match canonical name
  - Renamed files: `AdAstraGameMode.cpp`, `AdAstraGameMode.h` → `AdastreaGameMode.*`
  - Updated class name: `AAdAstraGameMode` → `AAdastreaGameMode`
  - Updated generated header include: `AdAstraGameMode.generated.h` → `AdastreaGameMode.generated.h`
- **Documentation Updates**: Normalized all textual references from "AdAstra" and "AdAstrea" to canonical "Adastrea"
  - Updated README.md project title and references
  - Updated ARCHITECTURE.md documentation
  - Updated CHANGELOG.md references
  - Updated CONTRIBUTING.md, CODE_STYLE.md, QUICKSTART.md, CHEATSHEET.md
  - Updated BLUEPRINT_CREATION_GUIDE.md, CONTENT_CREATION_QUICKSTART.md
  - Updated AUDIT_SUMMARY.md
  - Updated Config/README.md
  - Updated Assets documentation files (MaterialSystemGuide, TradingAPIReference, etc.)
- **Project Description**: Updated .uproject description from "AdAstra" to "Adastrea"
- **Removed Empty Directory**: Removed empty `Source/AdAstrea/` directory (contained only .keep file)

**Rebuild Instructions:**
After pulling these changes, you must regenerate project files and rebuild:
1. Right-click `Adastrea.uproject` and select "Generate Visual Studio project files"
2. Rebuild the solution in "Development Editor" configuration
3. The editor will now load the renamed `AdastreaEditor` module

**Verification Steps:**
- Verify the editor launches without module loading errors
- Check that all editor customizations work as expected
- Confirm no references to "AdAstrea" or "AdAstra" remain in code/docs

**Rollback Instructions:**
If issues arise, revert the changes with:
```bash
git revert HEAD
git push origin copilot/normalize-adastrea-names
```
Then regenerate project files and rebuild.

### Added - Repository Infrastructure (2025-10-18)
- **MIT License**: Added proper LICENSE file
- **Contributing Guidelines**: Comprehensive CONTRIBUTING.md with coding standards, workflow, and guidelines
- **Changelog**: This file with semantic versioning and Keep a Changelog format
- **Architecture Documentation**: ARCHITECTURE.md explaining project structure and design patterns
- **Quick Start Guide**: QUICKSTART.md for absolute beginners (15-minute setup)
- **Code Style Guide**: CODE_STYLE.md with C++, Blueprint, and asset naming conventions
- **Developer Cheat Sheet**: CHEATSHEET.md with quick reference for common tasks
- **Configuration Guide**: Config/README.md explaining Unreal Engine configuration
- **Content Organization**: Content/CONTENT_ORGANIZATION.md with best practices
- **Editor Config**: .editorconfig for consistent code formatting

### Changed - Documentation Improvements
- **Enhanced README**: Added table of contents, badges, better organization
- **Better Structure**: Reorganized project structure documentation with visual hierarchy
- **Documentation Section**: Comprehensive guide section with links to all system docs
- **Contributing Section**: Clear paths for different contributor types
- **Support Information**: Added version history, support channels, and community links

### Changed - Source Code Improvements
- **Enhanced C++ Documentation**: Added comprehensive header comments to key classes
- **Improved Spaceship Classes**: Better documentation for ASpaceship and ASpaceshipInterior
- **Enhanced Station Classes**: Detailed docs for ASpaceStation and ASpaceStationModule
- **Enum Improvements**: Added DisplayName metadata to all enums for better UX
- **Consistent Style**: All code follows Unreal Engine best practices

### Fixed - Critical Issues
- **Gitignore**: Now properly tracks .uproject, .uplugin, and .uasset files (was incorrectly ignoring them)
- **Naming Consistency**: Clarified Adastrea naming throughout project (note: fully normalized in later version)
- **Module Organization**: Documented purpose of editor module

### Documentation Structure
```
New/Enhanced Files:
├── LICENSE                          # MIT License
├── CONTRIBUTING.md                  # Contribution guidelines (7.5 KB)
├── CHANGELOG.md                     # This file (detailed version history)
├── ARCHITECTURE.md                  # Architecture overview (12.7 KB)
├── QUICKSTART.md                    # Beginner's setup guide (7.9 KB)
├── CODE_STYLE.md                    # Style guidelines (11.6 KB)
├── CHEATSHEET.md                    # Developer quick reference (8.2 KB)
├── .editorconfig                    # Code formatting config
├── Config/README.md                 # Configuration guide (4.5 KB)
└── Content/CONTENT_ORGANIZATION.md  # Asset organization (9.6 KB)

Updated Files:
├── README.md                        # Enhanced with TOC, badges, better organization
└── .gitignore                       # Fixed to properly track Unreal assets
```

## [0.9.0] - 2025-10-18

### Added - Trading System
- **Trade Item Data Assets**: Define tradeable commodities with supply/demand
- **Market Data Assets**: Configure trading hubs with price modifiers
- **Trade Contract System**: Quest-like delivery missions with rewards
- **AI Trader Component**: Autonomous trading behavior for NPCs
- **Trade Transaction System**: Complete trading workflow implementation
- Complete trading documentation suite:
  - TradingSystemGuide.md - Full system overview
  - TradingAPIReference.md - Complete API documentation
  - TradingDesignerWorkflow.md - Designer workflows
  - TradingBlueprintGuide.md - Blueprint integration
  - TradingBlueprintExamples.md - Example implementations
- 10+ YAML templates for trade items, markets, and contracts

### Added - Personnel Management System
- **Personnel Data Assets**: 31 distinct crew roles with full stat tracking
- **Skill System**: Multi-level skills with experience progression
- **Status Tracking**: Health, morale, fatigue, loyalty monitoring
- **Relationship System**: Interpersonal connections (friends, rivals, mentors)
- **Performance Metrics**: Career tracking and effectiveness monitoring
- **Command Hierarchy**: Manager/report organizational structures
- **Employment System**: Contract and compensation management
- **Trait System**: Unique characteristics with gameplay modifiers
- Complete personnel documentation:
  - PersonnelSystemGuide.md - Full system overview
  - PersonnelTemplates.md - All 31 roles documented
  - PersonnelDesignerWorkflow.md - Creation workflows
- 7 YAML templates for common roles (Captain, Pilot, Engineer, etc.)

### Added - AI System
- **NPC Logic Base**: Foundation for all AI behaviors
- **Faction Logic**: Strategic AI for faction-level decisions
- **Personnel Logic**: Individual crew AI with personality-driven behavior
- **8 Personality Dispositions**: Cautious, Bold, Analytical, Empathetic, Pragmatic, Curious, Disciplined, Creative
- **Early Game Priorities**: Peaceful/exploration-focused gameplay support
- **Truce System**: Diplomatic peace mechanics
- **Social Interactions**: Dynamic crew relationships
- **Task Priorities**: Personality-driven daily scheduling
- Complete AI documentation:
  - FactionAISetupGuide.md - Faction AI setup
  - PersonnelAISetupGuide.md - Personnel AI setup
  - AIDesignerWorkflow.md - No-code designer workflows
  - AISystemIntegration.md - System integration guide
  - AI_QUICK_START.md - Quick reference guide
  - AI_IMPLEMENTATION_SUMMARY.md - Technical overview
- YAML templates for peaceful explorers, trade coalitions, and various crew types

### Added - Material System
- **Material Data Assets**: Define raw materials, refined goods, and exotic resources
- **Material Categories**: Organized by type (metals, gases, organics, energy, rare)
- **Quality Tiers**: Common, Uncommon, Rare, Legendary
- **Station Integration**: Materials used in station module construction
- Complete material documentation:
  - MaterialSystemGuide.md - Full system overview
  - MaterialQuickStart.md - Quick start guide
  - MaterialStationIntegration.md - Integration with stations
  - MATERIAL_SYSTEM_SUMMARY.md - Technical summary
- YAML templates with 20+ material examples

## [0.8.0] - 2025-10-15

### Added - Spaceship System
- **Spaceship Data Assets**: Complete ship specification system
- **6 Ship Templates**: Pathfinder Scout, Warhammer Gunship, Merchant Trader, Starliner Luxury, Excavator Mining, Sovereign Command
- **Stat Categories**: Core, Combat, Mobility, Utility, Operational, Advanced, Lore
- **Blueprint Callable Functions**: Combat/Mobility/Utility ratings, role suitability checks, size categories, operational costs
- Comprehensive spaceship documentation:
  - SpaceshipDataAssetGuide.md - Complete creation guide
  - SpaceshipTemplates.md - All templates with full stats
  - SpaceshipDesignerWorkflow.md - Best practices and workflows
  - SPACESHIP_SYSTEM_SUMMARY.md - Technical overview
- YAML templates for all 6 ship types

### Added - Homeworld System
- **Homeworld Data Assets**: Player starting location system
- **Faction Relationship System**: Initial reputation values per homeworld
- **Starting Experience Diversity**: Different starting conditions for replay value
- **Blueprint Integration**: Reputation initialization and queries
- Complete homeworld documentation:
  - HomeworldSystemGuide.md - Full system guide
  - HomeworldBlueprintTemplates.md - Blueprint implementation examples
  - HOMEWORLD_IMPLEMENTATION_SUMMARY.md - Technical overview

## [0.7.0] - 2025-10-10

### Added - Faction System
- **Faction Data Assets**: Designer-friendly faction creation
- **10 Faction Templates**: Pre-designed factions (Solaris Union, Ironclad Consortium, etc.)
- **Trait System**: Gameplay modifiers with Blueprint-callable functions
- **Diplomacy System**: Alliance, war, and relationship tracking
- **Station Integration**: Faction assignment to stations and modules
- Comprehensive faction documentation:
  - BlueprintFactionAssetGuide.md - Step-by-step creation guide
  - FactionSetupGuide.md - 10 faction templates
  - BlueprintWorkflowTemplates.md - UI implementation examples
  - PlaytestingChecklist.md - QA verification guide
  - IterationGuide.md - Expanding faction behaviors
  - DesignerOnboarding.md - Best practices and quick start
  - IMPLEMENTATION_SUMMARY.md - Technical overview

### Added - Space Station System
- **Space Station Base Class**: Core station actor with module management
- **Space Station Module System**: Modular construction framework
- **Station Editor Widget**: UI hooks for visual station editing
- **Module Groups**: Docking, Power, Storage, Defence, Habitation, Production, Science, Command, Utility, Custom
- **Blueprint Integration**: Add, remove, move modules via Blueprints
- Documentation: STATION_EDITOR_README.md with implementation guide

## [0.6.0] - 2025-10-05

### Added - Core Systems
- **Character System**: Base character classes
- **Planet System**: Placeholder planet actors
- **Ship System**: Spaceship base classes with interior support
- **UI System**: Sector map widget and basic UI framework
- **Module Components**: Engine, weapon, and organism components for ships

### Project Infrastructure
- Initial project structure
- Source code organization by feature (AI, Characters, Factions, Planets, Ships, Stations, UI)
- Content directory structure (Blueprints, Maps, Planets, SpaceStations, UI)
- Basic configuration setup
- README with project overview

## Project Goals and Vision

Adastrea is an open-world space flight game focused on:
- **Exploration**: Vast galaxies to discover
- **Space Combat**: Thrilling tactical battles
- **Station Building**: Modular space station construction
- **Faction Dynamics**: Complex diplomatic relationships
- **Crew Management**: Personnel with skills, personalities, and relationships
- **Trading Economy**: Dynamic market-driven commerce
- **Designer-Friendly**: Blueprint-first design with minimal C++ knowledge required

### Technical Philosophy
- **Data-Driven Design**: Use Data Assets for all game content
- **Blueprint First**: Expose all functionality to Blueprints
- **Designer Empowerment**: Comprehensive documentation and YAML templates
- **Modularity**: System should work independently and integrate seamlessly
- **Performance**: Optimize for scalability with many entities
- **Maintainability**: Clean code, good documentation, clear architecture

---

## Version History Summary

- **0.9.x**: Trading, Personnel, AI, and Materials systems
- **0.8.x**: Spaceships and Homeworlds
- **0.7.x**: Factions and Space Stations
- **0.6.x**: Core game systems foundation

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
