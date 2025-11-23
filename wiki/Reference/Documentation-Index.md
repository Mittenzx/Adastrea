# Documentation Index

Complete index of all documentation in the Adastrea project.

> **Note**: This is a wiki-friendly version. For the complete technical index, see [DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md) in the repository root.

## Quick Navigation

- [Getting Started](#getting-started)
- [Core Systems](#core-systems)
- [Developer Guides](#developer-guides)
- [Blueprint Development](#blueprint-development)
- [Automation & Testing](#automation--testing)
- [Reference Materials](#reference-materials)

---

## Getting Started

**In This Wiki**:
- [Quick Start Guide](../Getting-Started/Quick-Start.md) - 10-minute setup
- [Installation Guide](../Getting-Started/Installation.md) - Detailed installation
- [Designer Onboarding](../Getting-Started/Designer-Onboarding.md) - For non-programmers
- [Content Creation](../Getting-Started/Content-Creation.md) - Creating game content
- [First Steps](../Getting-Started/First-Steps.md) - Your first hour

**In Repository**:
- `README.md` - Project overview
- `QUICKSTART.md` - Fast setup guide
- `CONTENT_CREATION_QUICKSTART.md` - Content creation basics
- `Assets/DesignerOnboarding.md` - Original onboarding guide

---

## Core Systems

**In This Wiki**:
- [Core Systems Overview](../Core-Systems/README.md)
- [Spaceship System](../Core-Systems/Spaceship-System.md)
- [Faction System](../Core-Systems/Faction-System.md)
- [Personnel System](../Core-Systems/Personnel-System.md)
- [Trading System](../Core-Systems/Trading-System.md)
- [Way System](../Core-Systems/Way-System.md)
- [Station System](../Core-Systems/Station-System.md)
- [Combat System](../Core-Systems/Combat-System.md)
- [AI System](../Core-Systems/AI-System.md)

**Complete System Documentation** (in repository `Assets/` folder):

### Spaceship System
- `Assets/SpaceshipDataAssetGuide.md` - Complete guide
- `Assets/SpaceshipDesignerWorkflow.md` - Workflow
- `Assets/SpaceshipTemplates.md` - Templates reference
- `Assets/SpaceshipTemplates/*.yaml` - YAML configs

### Faction System  
- `Assets/BlueprintFactionAssetGuide.md` - Blueprint guide
- `Assets/FactionSetupGuide.md` - Setup guide
- `Assets/FactionSystemQuickReference.md` - Quick reference
- `Assets/FactionAITemplates/*.yaml` - AI configs

### Personnel System
- `Assets/PersonnelSystemGuide.md` - Complete guide
- `Assets/PersonnelDesignerWorkflow.md` - Workflow
- `Assets/PersonnelTemplates.md` - Role templates
- `Assets/PersonnelTemplates/*.yaml` - Personnel configs
- `Assets/PersonnelAITemplates/*.yaml` - AI configs

### Trading System
- `Assets/TradingSystemGuide.md` - Complete guide
- `Assets/TradingDesignerWorkflow.md` - Workflow
- `Assets/TradingAPIReference.md` - API docs
- `Assets/TradingBlueprintExamples.md` - Examples
- `Assets/TradingTemplates/*.yaml` - Trade configs

### Way System (Guilds)
- `Assets/WAY_SYSTEM_INDEX.md` - System index
- `Assets/WaySystemGuide.md` - Complete guide
- `Assets/WaySystemOverview.md` - Overview
- `Assets/WaySystemQuickReference.md` - Quick reference
- `Assets/WaySystemAPIReference.md` - API docs
- `Assets/WayCreationGuide.md` - Creation guide
- `Assets/WayNetworkGuide.md` - Network guide
- `Assets/WayNetworkQuickStart.md` - Quick start
- `Assets/WayNetworkWorkflow.md` - Workflow
- `Assets/WayNetworkTemplates/*.yaml` - Network configs

### Additional Systems
- `Assets/CombatSystemGuide.md` - Combat
- `Assets/MaterialSystemGuide.md` - Materials
- `Assets/NavigationSystemGuide.md` - Navigation
- `Assets/QuestSystemGuide.md` - Quests
- `Assets/ExplorationSystemGuide.md` - Exploration
- `Assets/HUDSystemGuide.md` - HUD/UI
- `Assets/AudioSystemGuide.md` - Audio
- `Assets/SaveSystemGuide.md` - Save/Load
- `Assets/TutorialSystemGuide.md` - Tutorials
- `Assets/HomeworldSystemGuide.md` - Homeworlds
- `Assets/InventorySystemGuide.md` - Inventory
- `Assets/PlayerProgressionGuide.md` - Progression

---

## Developer Guides

**In This Wiki**:
- [Architecture Overview](../Developer-Guides/Architecture.md)
- [Contributing Guide](../Developer-Guides/Contributing.md)
- [Code Style Guide](../Developer-Guides/Code-Style.md)
- [Build & Setup](../Developer-Guides/Build-Setup.md)
- [Performance Guide](../Developer-Guides/Performance.md)
- [Roadmap](../Developer-Guides/Roadmap.md)

**In Repository**:
- `ARCHITECTURE.md` - System architecture
- `CONTRIBUTING.md` - Contribution guidelines
- `CODE_STYLE.md` - Coding standards
- `ROADMAP.md` - Development roadmap
- `PERFORMANCE_IMPROVEMENTS.md` - Optimization
- `EDITOR_IMPLEMENTATION_GUIDE.md` - Editor tools
- `ENHANCED_INPUT_GUIDE.md` - Input system

---

## Blueprint Development

**In This Wiki**:
- [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)
- [Blueprint Requirements List](../Blueprint-Development/Blueprint-Requirements.md)
- [Blueprint Quick Reference](../Blueprint-Development/Blueprint-Quick-Reference.md)
- [Blueprint Creation Guide](../Blueprint-Development/Blueprint-Creation.md)
- [Blueprint Workflows](../Blueprint-Development/Blueprint-Workflows.md)

**In Repository**:
- `BLUEPRINT_GUIDE_FOR_BEGINNERS.md` - Complete beginner guide (1146 lines)
- `BLUEPRINT_REQUIREMENTS_LIST.md` - All required Blueprints (882 lines)
- `BLUEPRINT_QUICK_START_SUMMARY.md` - One-page summary
- `BLUEPRINT_ROADMAP_VISUAL.md` - Visual progression
- `BLUEPRINT_CHECKLIST.md` - Creation checklist
- `BLUEPRINT_QUICK_REFERENCE.md` - Quick reference card
- `BLUEPRINT_CREATION_GUIDE.md` - Advanced guide
- `Assets/BlueprintWorkflowTemplates.md` - Workflow templates

---

## Automation & Testing

**In This Wiki**:
- [Testing Guide](../Automation-Testing/Testing-Guide.md)
- [Automation Scripts](../Automation-Testing/Automation-Scripts.md)
- [Procedural Generation](../Automation-Testing/Procedural-Generation.md)
- [Scene Population](../Automation-Testing/Scene-Population.md)
- [Cloud Build](../Automation-Testing/Cloud-Build.md)

**In Repository**:
- `TESTING_AUTOMATION_GUIDE.md` - Complete testing guide
- `TESTING_QUICK_REFERENCE.md` - Testing quick ref
- `PROCEDURAL_GENERATION_GUIDE.md` - Procedural content
- `PROCEDURAL_GENERATION_QUICKSTART.md` - Quick start
- `PROCEDURAL_GENERATION_EXAMPLES.md` - Examples
- `SCENE_POPULATION_GUIDE.md` - Scene automation
- `SCENE_POPULATION_QUICK_REFERENCE.md` - Quick ref
- `CLOUD_BUILD_SERVICES.md` - Cloud CI/CD
- `CLOUD_BUILD_QUICK_START.md` - Cloud setup

**Scripts** (in repository root):
- `AutomationRunner.py` - Test orchestrator
- `SmokeTest.py` - Smoke testing
- `ScreenshotTester.py` - Screenshot tests
- `ProceduralGenerators.py` - Content generation
- `ScenePopulator.py` - Scene population
- `InteriorLayoutPopulator.py` - Interior layouts
- `YAMLtoDataAsset.py` - YAML import
- `AdastreaAssetValidator.py` - Asset validation

---

## Reference Materials

**In This Wiki**:
- [API Reference](API-Reference.md)
- [YAML Templates](YAML-Templates.md)
- [Quick References](Quick-References.md)
- [Cheat Sheet](Cheat-Sheet.md)

**Quick References** (in repository):
- `CHEATSHEET.md` - Common commands
- `DATA_MANAGEMENT_QUICK_REFERENCE.md` - Data workflows
- `OPTIMIZATION_QUICK_REFERENCE.md` - Performance tips
- `TESTING_QUICK_REFERENCE.md` - Testing workflows
- `Assets/FactionSystemQuickReference.md` - Factions
- `Assets/CombatQuickReference.md` - Combat
- `Assets/HUD_QUICK_REFERENCE.md` - HUD/UI
- `Assets/WaySystemQuickReference.md` - Ways
- `Assets/SectorSystemQuickReference.md` - Sectors
- `Docs/TestMode_QuickReference.md` - Test mode

**Templates** (in repository `Assets/` folder):
- `Assets/SpaceshipTemplates/` - Ship configs
- `Assets/FactionAITemplates/` - Faction AI
- `Assets/PersonnelTemplates/` - Personnel
- `Assets/PersonnelAITemplates/` - Personnel AI
- `Assets/TradingTemplates/` - Trading
- `Assets/WayNetworkTemplates/` - Way networks
- `Assets/CombatTemplates/` - Combat
- `Assets/InventoryTemplates/` - Inventory
- `Assets/SectorTemplates/` - Sector governance

---

## AI Tool Integration

Documentation for AI-powered development:
- `DIRECTOR_KNOWLEDGE_BASE.md` - AI tool knowledge base
- `DIRECTOR_PLUGIN_OVERVIEW.md` - Plugin overview
- `UE_PLUGIN_FEASIBILITY_REPORT.md` - Feasibility study
- `PLUGIN_DEVELOPMENT_GUIDE.md` - Plugin development
- `.github/copilot-instructions.md` - GitHub Copilot config
- `.github/copilot-knowledge.md` - Copilot knowledge base

---

## Project Management

- `ROADMAP.md` - Development roadmap
- `ROADMAP_UPDATE_GUIDELINES.md` - Roadmap updates
- `CHANGELOG.md` - Version history
- `LICENSE` - MIT License
- `CONTRIBUTING.md` - How to contribute

---

## Additional Documentation

### Content Directory
- `Content/CONTENT_ORGANIZATION.md` - Content structure
- `Content/Blueprints/STATION_EDITOR_README.md` - Station editor
- `Content/Docs/Blueprint_ModManager_UI.md` - Mod manager UI

### Plugins Directory
- `Plugins/README.md` - Plugins overview
- `Plugins/AdastreaDirector/README.md` - Director plugin

### Configuration Files
- `automation_config.json` - Test automation
- `validator_config.json` - Validation rules
- `smoke_test_config.json` - Smoke tests
- `screenshot_test_config.json` - Screenshot tests

---

## Documentation Statistics

- **Total Markdown Files**: 210+ files
- **Root Level**: 40+ files
- **Assets Directory**: 48+ files
- **Wiki Pages**: Growing collection
- **Template Files**: 50+ YAML configs

---

## How to Use This Index

### For Designers
1. Start with [Getting Started](#getting-started)
2. Review [Core Systems](#core-systems) for your area
3. Use [Reference Materials](#reference-materials) while working

### For Developers
1. Review [Developer Guides](#developer-guides)
2. Check [Architecture](../Developer-Guides/Architecture.md)
3. Follow [Code Style](../Developer-Guides/Code-Style.md)

### For Contributors
1. Read [Contributing Guide](../Developer-Guides/Contributing.md)
2. Check relevant system documentation
3. Follow [Blueprint Guidelines](../Blueprint-Development/Blueprint-Beginners-Guide.md)

---

## Documentation Best Practices

When looking for documentation:
1. **Check this index** for the right document
2. **Use wiki search** for specific topics
3. **Browse by category** for related docs
4. **Check YAML templates** for examples
5. **Read quick references** for fast lookups

When adding documentation:
1. **Follow naming conventions** (`[System]Guide.md`, etc.)
2. **Update this index** with new files
3. **Cross-reference** related docs
4. **Include examples** and templates
5. **Keep README.md updated** for major additions

---

## Getting Help

- **Can't find a document?** Use GitHub repository search
- **Need more detail?** Check the repository `Assets/` folder
- **Have questions?** Open a [discussion](https://github.com/Mittenzx/Adastrea/discussions)
- **Found an error?** Open an [issue](https://github.com/Mittenzx/Adastrea/issues)

---

**This index covers the complete Adastrea documentation ecosystem.**

*For the complete technical index, see [DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md)*

*Last Updated: November 23, 2025*
