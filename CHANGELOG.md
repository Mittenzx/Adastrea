# Changelog

All notable changes to the AdAstra project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- MIT License file
- Contributing guidelines (CONTRIBUTING.md)
- Comprehensive changelog (this file)
- Improved .gitignore with proper Unreal Engine patterns
- Architecture documentation

### Changed
- Enhanced README with better organization and quick start guide
- Improved code documentation and consistency

### Fixed
- .gitignore now properly tracks .uproject, .uplugin, and .uasset files
- Naming consistency issues

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

AdAstra is an open-world space flight game focused on:
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
