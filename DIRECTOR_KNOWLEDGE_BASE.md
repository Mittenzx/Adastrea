# Adastrea Director Knowledge Base

**Last Updated**: November 14, 2025  
**Version**: 1.1  
**Purpose**: Comprehensive knowledge base optimized for Adastrea-Director AI tool ingestion

---

## Overview

This document serves as the primary knowledge base entry point for the **Adastrea-Director** AI tool. It provides a structured overview of the Adastrea project, its systems, documentation, and development workflows to enable the Director to understand and assist with game development.

**Key Resources:**
- [ADASTREA_DIRECTOR_ANALYSIS.md](ADASTREA_DIRECTOR_ANALYSIS.md) - Analysis of external Director tool
- [UE_PLUGIN_FEASIBILITY_REPORT.md](UE_PLUGIN_FEASIBILITY_REPORT.md) - Plugin conversion feasibility study
- [PLUGIN_DEVELOPMENT_GUIDE.md](PLUGIN_DEVELOPMENT_GUIDE.md) - Plugin implementation roadmap

> **📢 Plugin Development:** A native Unreal Engine plugin version of Adastrea-Director is planned for Q1-Q3 2026, providing seamless AI assistance within the editor. See the [Plugin Development Guide](PLUGIN_DEVELOPMENT_GUIDE.md) for details.

---

## Table of Contents

1. [Project Summary](#project-summary)
2. [Documentation Structure](#documentation-structure)
3. [Core Game Systems](#core-game-systems)
4. [Development Workflows](#development-workflows)
5. [Code Organization](#code-organization)
6. [Data Asset System](#data-asset-system)
7. [Automation & Tooling](#automation--tooling)
8. [Common Tasks & Queries](#common-tasks--queries)
9. [File Locations Reference](#file-locations-reference)

---

## Project Summary

### What is Adastrea?

**Adastrea** is an open-world space flight game built on **Unreal Engine 5.6** featuring:
- Space exploration and navigation
- Tactical spaceship combat
- Modular space station construction
- Complex faction diplomacy
- Dynamic crew management
- Emergent trading economies
- Guild network systems (Ways)

### Core Philosophy

- **Data-Driven Design**: All content uses Blueprint Data Assets
- **Designer-Friendly**: Minimal C++ knowledge required
- **Modular Architecture**: Independent systems with clean integration
- **Blueprint First**: Complete Blueprint API exposure
- **Comprehensive Documentation**: Guides, templates, and workflows for everything

### Technology Stack

- **Engine**: Unreal Engine 5.6
- **Primary Language**: C++ (core systems)
- **Scripting**: Blueprint (game logic, content)
- **Configuration**: YAML (templates, data imports)
- **Automation**: Python (tools, testing, validation)
- **Build System**: Unreal Build Tool (UBT)
- **Version Control**: Git

### Current Status

- **Version**: 1.0.0 (Alpha)
- **Phase**: Phase 3 (Advanced Systems) - Complete as of November 2025
- **Next Phase**: Phase 4 (Gameplay & Polish) - Q1-Q2 2026

---

## Documentation Structure

### Root-Level Documentation (39 files)

#### Core Entry Points
- **[README.md](README.md)** - Project overview, quick start, feature summary
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System architecture and design patterns
- **[ROADMAP.md](ROADMAP.md)** - Development phases and milestones
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines
- **[CODE_STYLE.md](CODE_STYLE.md)** - C++ and Blueprint coding standards

#### Quick References (9 files)
- **[CHEATSHEET.md](CHEATSHEET.md)** - Common commands and shortcuts
- **[QUICKSTART.md](QUICKSTART.md)** - Fast project setup
- **[CONTENT_CREATION_QUICKSTART.md](CONTENT_CREATION_QUICKSTART.md)** - Designer onboarding
- **[DATA_MANAGEMENT_QUICK_REFERENCE.md](DATA_MANAGEMENT_QUICK_REFERENCE.md)** - Data asset workflows
- **[OPTIMIZATION_QUICK_REFERENCE.md](OPTIMIZATION_QUICK_REFERENCE.md)** - Performance tips
- **[SCENE_POPULATION_QUICK_REFERENCE.md](SCENE_POPULATION_QUICK_REFERENCE.md)** - Level population
- **[TESTING_QUICK_REFERENCE.md](TESTING_QUICK_REFERENCE.md)** - Testing workflows
- **GENERATOR_QUICK_START.md** - Script automation
- **PROCEDURAL_GENERATION_QUICKSTART.md** - Procedural content

#### System Guides (8 files)
- **[BLUEPRINT_CREATION_GUIDE.md](BLUEPRINT_CREATION_GUIDE.md)** - Blueprint best practices
- **[COMBAT_SYSTEM_SUMMARY.md](COMBAT_SYSTEM_SUMMARY.md)** - Combat system overview
- **[EDITOR_IMPLEMENTATION_GUIDE.md](EDITOR_IMPLEMENTATION_GUIDE.md)** - Custom editor tools
- **[ENHANCED_INPUT_GUIDE.md](ENHANCED_INPUT_GUIDE.md)** - Input system setup
- **[PROCEDURAL_GENERATION_GUIDE.md](PROCEDURAL_GENERATION_GUIDE.md)** - Procedural systems
- **[SCENE_POPULATION_GUIDE.md](SCENE_POPULATION_GUIDE.md)** - Scene automation
- **[SCHEMA_GUIDE.md](SCHEMA_GUIDE.md)** - YAML schema validation
- **[YAML_IMPORT_GUIDE.md](YAML_IMPORT_GUIDE.md)** - YAML to Data Asset import

#### Automation & Scripts (4 files)
- **[GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md)** - Content generation
- **[SCRIPTS_README.md](SCRIPTS_README.md)** - Utility scripts
- **[TESTING_AUTOMATION_GUIDE.md](TESTING_AUTOMATION_GUIDE.md)** - Test automation
- **[GOOGLE_SHEETS_GUIDE.md](GOOGLE_SHEETS_GUIDE.md)** - Google Sheets integration

### Assets/ Directory (48 files)

The **Assets/** directory contains the most detailed system documentation:

#### System Guides (Primary Documentation)
- **SpaceshipDataAssetGuide.md** - Spaceship system (ratings, stats, YAML templates)
- **FactionSetupGuide.md** - Faction system (traits, diplomacy, relationships)
- **PersonnelSystemGuide.md** - Personnel system (roles, skills, AI)
- **TradingSystemGuide.md** - Trading system (markets, contracts, prices)
- **WaySystemGuide.md** - Way guild system (networks, alliances)
- **NavigationSystemGuide.md** - Navigation and movement
- **CombatSystemGuide.md** - Combat mechanics
- **QuestSystemGuide.md** - Quest and mission system
- **InventorySystemGuide.md** - Inventory management
- **MaterialSystemGuide.md** - Material and resource system
- **HomeworldSystemGuide.md** - Starting location system
- **HUDSystemGuide.md** - UI and HUD system
- **ShipControlGuide.md** - Ship control implementation

#### Designer Workflows (7 files)
- **DesignerOnboarding.md** - New designer guide
- **SpaceshipDesignerWorkflow.md** - Ship creation workflow
- **PersonnelDesignerWorkflow.md** - Crew creation workflow
- **TradingDesignerWorkflow.md** - Trading setup workflow
- **AIDesignerWorkflow.md** - AI configuration workflow
- **BlueprintWorkflowTemplates.md** - Blueprint templates
- **IterationGuide.md** - Iterative development

#### Quick References (7 files)
- **FactionSystemQuickReference.md** - Faction quick ref
- **CombatQuickReference.md** - Combat quick ref
- **HUD_QUICK_REFERENCE.md** - HUD quick ref
- **WaySystemQuickReference.md** - Way system quick ref
- **AI_QUICK_START.md** - AI quick start
- **MaterialQuickStart.md** - Material quick start
- **WayNetworkQuickStart.md** - Way network quick start

#### Templates (5 files)
- **SpaceshipTemplates.md** - Ship YAML templates
- **PersonnelTemplates.md** - Personnel YAML templates
- **PlaytestingChecklist.md** - QA checklist
- **WayNetworkTemplates/README.md** - Way network templates

#### API References (3 files)
- **TradingAPIReference.md** - Trading API
- **WaySystemAPIReference.md** - Way system API

#### Integration Guides (6 files)
- **AISystemIntegration.md** - AI system integration
- **FactionAISetupGuide.md** - Faction AI
- **PersonnelAISetupGuide.md** - Personnel AI
- **BlueprintFactionAssetGuide.md** - Faction Blueprints
- **TradingBlueprintExamples.md** - Trading Blueprints
- **MaterialStationIntegration.md** - Material system

### .github/ Directory (7 files)

GitHub-specific configuration:
- **.github/copilot-instructions.md** - AI assistant configuration (comprehensive!)
- **.github/agents/** - Custom agent definitions

### Other Documentation Directories

- **Docs/** (5 files) - Additional technical docs
- **Blueprints/** (3 files) - Blueprint-specific guides
- **Content/** (3 files) - Content organization guides

---

## Core Game Systems

### 1. Spaceship System

**Purpose**: Define and manage player and NPC spaceships

**Key Components**:
- `USpaceshipDataAsset` - C++ data asset defining ship specifications
- 6 stat categories: Core, Combat, Mobility, Utility, Operational, Advanced
- Rating system (1-100) for balanced ship comparison
- YAML template support for rapid ship creation

**Documentation**:
- Primary: `Assets/SpaceshipDataAssetGuide.md`
- Workflow: `Assets/SpaceshipDesignerWorkflow.md`
- Templates: `Assets/SpaceshipTemplates.md`
- Quick Ref: See Primary guide

**Common Tasks**:
- Create new ship: Use YAML template + YAMLtoDataAsset.py script
- Balance ships: Use rating calculation functions
- Add ship class: Extend EShipClass enum in C++

### 2. Faction System

**Purpose**: Manage political factions, diplomacy, and relationships

**Key Components**:
- `UFactionDataAsset` - Faction definition with traits
- `UFactionDiplomacyManager` - Global faction relationship manager
- `UPlayerReputationComponent` - Player standing with factions
- Trait system for gameplay modifiers
- Relationship scale: -100 (hostile) to +100 (allied)

**Documentation**:
- Primary: `Assets/FactionSetupGuide.md`
- Quick Ref: `Assets/FactionSystemQuickReference.md`
- AI Setup: `Assets/FactionAISetupGuide.md`
- Blueprint Guide: `Assets/BlueprintFactionAssetGuide.md`

**Common Tasks**:
- Create faction: YAML template with traits
- Set diplomacy: Configure initial relationships
- Add trait: Define new FFactionTrait in C++

### 3. Personnel System

**Purpose**: Crew member management with skills, traits, and relationships

**Key Components**:
- `UPersonnelDataAsset` - Crew member definition
- 31 distinct role types (Captain, Engineer, Pilot, etc.)
- Skill system with proficiency levels
- Social relationship simulation
- Performance tracking

**Documentation**:
- Primary: `Assets/PersonnelSystemGuide.md`
- Workflow: `Assets/PersonnelDesignerWorkflow.md`
- Templates: `Assets/PersonnelTemplates.md`
- AI Setup: `Assets/PersonnelAISetupGuide.md`

**Common Tasks**:
- Create crew: YAML template with role and skills
- Add role: Extend EPersonnelRole enum
- Configure AI: Use UPersonnelLogic base class

### 4. Trading System

**Purpose**: Dynamic economy with supply/demand and contracts

**Key Components**:
- Supply/demand simulation
- Faction-based pricing modifiers
- Contract quest system
- AI autonomous trading
- Market fluctuation

**Documentation**:
- Primary: `Assets/TradingSystemGuide.md`
- Workflow: `Assets/TradingDesignerWorkflow.md`
- API: `Assets/TradingAPIReference.md`
- Blueprints: `Assets/TradingBlueprintExamples.md`

**Common Tasks**:
- Create trade good: Define item and base price
- Set market: Configure station trading inventory
- Create contract: Use quest system integration

### 5. Way System

**Purpose**: Guild networks for specialized trading and social systems

**Key Components**:
- `UWayDataAsset` - Guild definition
- `UWayNetworkDataAsset` - Network alliances
- `UVerseSubsystem` - Global Way network manager
- Specialized trading paths
- Guild reputation and benefits

**Documentation**:
- Primary: `Assets/WaySystemGuide.md`
- Overview: `Assets/WaySystemOverview.md`
- Quick Start: `Assets/WayNetworkQuickStart.md`
- Quick Ref: `Assets/WaySystemQuickReference.md`
- API: `Assets/WaySystemAPIReference.md`
- Index: `Assets/WAY_SYSTEM_INDEX.md`

**Common Tasks**:
- Create Way: YAML template with benefits
- Create Network: Link multiple Ways
- Configure trading: Set specialized goods

### 6. Space Station System

**Purpose**: Modular station construction and management

**Key Components**:
- `ASpaceStation` - Station actor
- `ASpaceStationModule` - Individual module actors
- Faction integration
- Module groups for organization
- Blueprint-friendly APIs

**Documentation**:
- Editor: `Content/Blueprints/STATION_EDITOR_README.md`

**Common Tasks**:
- Build station: Use editor tools
- Add module: Place module actor
- Configure faction: Set station owner

### 7. AI System

**Purpose**: Autonomous behavior for factions and personnel

**Key Components**:
- `UNPCLogicBase` - Base class for AI with BlueprintNativeEvent hooks
- `UFactionLogic` - Strategic faction-level AI
- `UPersonnelLogic` - Individual crew AI with 8 personality dispositions
- Peaceful early-game priorities
- Social relationship simulation

**Documentation**:
- Primary: `Assets/AISystemIntegration.md`
- Faction AI: `Assets/FactionAISetupGuide.md`
- Personnel AI: `Assets/PersonnelAISetupGuide.md`
- Workflow: `Assets/AIDesignerWorkflow.md`
- Quick Start: `Assets/AI_QUICK_START.md`

**Common Tasks**:
- Create AI: Inherit from UNPCLogicBase
- Configure behavior: Override BlueprintNativeEvent functions
- Set personality: Configure disposition weights

### 8. Combat System

**Purpose**: Tactical space combat with diverse mechanics

**Key Components**:
- Weapon systems
- Shield systems
- Damage calculation
- Targeting systems

**Documentation**:
- Summary: `COMBAT_SYSTEM_SUMMARY.md`
- Guide: `Assets/CombatSystemGuide.md`
- Quick Ref: `Assets/CombatQuickReference.md`

### 9. Navigation System

**Purpose**: Ship movement and pathfinding

**Documentation**:
- Primary: `Assets/NavigationSystemGuide.md`

### 10. Quest System

**Purpose**: Missions and objectives

**Documentation**:
- Primary: `Assets/QuestSystemGuide.md`

### 11. Material System

**Purpose**: Resource gathering and crafting

**Key Components**:
- Material definitions
- Resource nodes
- Crafting recipes
- Station integration

**Documentation**:
- Guide: `Assets/MaterialSystemGuide.md`
- Quick Start: `Assets/MaterialQuickStart.md`
- Integration: `Assets/MaterialStationIntegration.md`

### 12. HUD/UI System

**Purpose**: User interface and player feedback

**Documentation**:
- Guide: `Assets/HUDSystemGuide.md`
- Quick Ref: `Assets/HUD_QUICK_REFERENCE.md`

### 13. Ship Control System

**Purpose**: Player ship control and input

**Documentation**:
- Guide: `Assets/ShipControlGuide.md`
- Summary: `Assets/ShipControlImplementationSummary.md`

### 14. Inventory System

**Purpose**: Item storage and management

**Documentation**:
- Primary: `Assets/InventorySystemGuide.md`

### 15. Homeworld System

**Purpose**: Player starting location

**Documentation**:
- Primary: `Assets/HomeworldSystemGuide.md`

### 16. Ship Customization System

**Purpose**: Ship modifications and upgrades

**Documentation**:
- Primary: `Assets/ShipCustomizationGuide.md`

### 17. Interior System

**Purpose**: Ship interior layouts

**Documentation**:
- Primary: `Assets/SpaceshipInteriorGuide.md`

---

## Development Workflows

### Creating New Content

#### 1. Create a New Spaceship
```
1. Start with YAML template (Assets/SpaceshipTemplates.md)
2. Fill in ship specifications
3. Run: python YAMLtoDataAsset.py --input MyShip.yaml
4. Validate in Unreal Editor
5. Test ship ratings and balance
```

#### 2. Create a New Faction
```
1. Use YAML template (Assets/FactionSetupGuide.md)
2. Define traits and starting relationships
3. Import via YAMLtoDataAsset.py
4. Configure diplomacy manager
5. Set up AI behavior (optional)
```

#### 3. Create New Personnel
```
1. Use YAML template (Assets/PersonnelTemplates.md)
2. Define role, skills, traits
3. Import via YAMLtoDataAsset.py
4. Configure relationships
5. Set up AI personality (optional)
```

#### 4. Set Up Trading
```
1. Define trade goods
2. Configure station inventory
3. Set price modifiers by faction
4. Create contracts (optional)
5. Test supply/demand simulation
```

### Code Development

#### Adding a New System
```
1. Create C++ classes in Source/Adastrea/[SystemName]/
2. Implement UDataAsset for configuration
3. Expose to Blueprint with UFUNCTION/UPROPERTY
4. Create YAML schema in Schemas/
5. Write comprehensive guide in Assets/
6. Create designer workflow document
7. Add quick reference
8. Create YAML templates
9. Update README.md
```

#### Blueprint Development
```
1. Follow BLUEPRINT_CREATION_GUIDE.md
2. Use BP_ prefix for Blueprints
3. Keep functions small (max 20-30 nodes)
4. Add comment boxes for organization
5. Follow left-to-right, top-to-bottom flow
```

### Testing & Validation

#### Validation Workflow
```
1. Run SetupCheck.py for environment validation
2. Run SchemaValidator.py for YAML validation
3. Run AdastreaAssetValidator.py for asset validation
4. Run SmokeTest.py for quick testing
5. Run AutomationRunner.py for comprehensive tests
6. Follow PlaytestingChecklist.md for QA
```

---

## Code Organization

### Source Code Structure

```
Source/
├── Adastrea.Target.cs           # Runtime build target
├── AdastreaEditor.Target.cs     # Editor build target
├── Adastrea/                    # Core game systems
│   ├── AI/                      # AI logic classes
│   ├── Characters/              # Character and personnel
│   ├── Factions/                # Faction system
│   ├── Materials/               # Material system
│   ├── Planets/                 # Planet classes
│   ├── Player/                  # Player-specific systems
│   ├── Ships/                   # Spaceship classes
│   ├── Stations/                # Space station classes
│   ├── Trading/                 # Trading economy
│   └── UI/                      # UI widgets
├── PlayerMods/                  # Player modding system
└── StationEditor/               # Station construction editor
```

### Content Organization

```
Content/
├── Blueprints/              # Blueprint classes
├── DataAssets/              # Data Assets organized by system
├── Maps/                    # Level maps
├── UI/                      # UI widgets
└── [System]/                # System-specific content
```

### Naming Conventions

**C++ Classes**:
- UCLASS from UObject: `UClassName` (e.g., `USpaceshipDataAsset`)
- ACLASS from AActor: `AClassName` (e.g., `ASpaceStation`)
- Structs: `FStructName` (e.g., `FFactionTrait`)
- Enums: `EEnumName` (e.g., `EPersonnelRole`)

**Blueprints**:
- Blueprints: `BP_SystemName_Purpose` (e.g., `BP_Ship_Fighter`)
- Widgets: `WBP_WidgetName` (e.g., `WBP_HUD_Main`)
- Data Assets: `DA_Type_Name` (e.g., `DA_Ship_Pathfinder`)

**Assets**:
- Material: `M_Name`
- Material Instance: `MI_Name`
- Texture: `T_Name_Suffix` (_D, _N, _R, etc.)
- Static Mesh: `SM_Name`
- Skeletal Mesh: `SK_Name`

---

## Data Asset System

### What are Data Assets?

Data Assets are the primary content creation mechanism in Adastrea. They are:
- **Designer-friendly**: No C++ knowledge required
- **Blueprint-based**: Create in Unreal Editor
- **YAML-compatible**: Can be generated from YAML templates
- **Version-control friendly**: Text-based and diff-friendly
- **Type-safe**: Validated by C++ class definitions

### Core Data Asset Types

1. **USpaceshipDataAsset** - Ship specifications
2. **UFactionDataAsset** - Faction definitions
3. **UPersonnelDataAsset** - Crew members
4. **UWayDataAsset** - Guild definitions
5. **UWayNetworkDataAsset** - Guild networks
6. **UHomeworldDataAsset** - Starting locations
7. **UQuestDataAsset** - Missions (future)

### YAML to Data Asset Workflow

```bash
# 1. Create YAML file from template
cp Assets/SpaceshipTemplates/fighter_template.yaml my_fighter.yaml

# 2. Edit YAML file with specifications

# 3. Validate YAML against schema
python SchemaValidator.py --yaml my_fighter.yaml --schema Schemas/spaceship.schema.json

# 4. Import to Data Asset
python YAMLtoDataAsset.py --input my_fighter.yaml --output DA_Ship_MyFighter

# 5. Verify in Unreal Editor
```

---

## Automation & Tooling

### Python Scripts

#### Content Generation
- **YAMLtoDataAsset.py** - Convert YAML to Data Assets
- **ProceduralGenerators.py** - Generate procedural content
- **GuideGenerator.py** - Generate documentation
- **TemplateGenerator.py** - Create templates
- **ScenePopulator.py** - Populate levels
- **InteriorLayoutPopulator.py** - Generate ship interiors

#### Validation & Testing
- **SetupCheck.py** - Environment validation
- **SchemaValidator.py** - YAML schema validation
- **AdastreaAssetValidator.py** - Asset validation
- **SmokeTest.py** - Quick smoke tests
- **AutomationRunner.py** - Comprehensive test runner
- **ScreenshotTester.py** - Visual regression testing

#### Integration
- **GoogleSheetsIntegration.py** - Google Sheets import/export
- **CSVIntegration.py** - CSV data import
- **EditorUtilities.py** - Editor automation

### Configuration Files

- **automation_config.json** - Test automation settings
- **validator_config.json** - Validation rules
- **smoke_test_config.json** - Smoke test configuration
- **screenshot_test_config.json** - Screenshot test settings

---

## Common Tasks & Queries

### "How do I create a new [X]?"

**Spaceship**: See `Assets/SpaceshipDesignerWorkflow.md`  
**Faction**: See `Assets/FactionSetupGuide.md`  
**Personnel**: See `Assets/PersonnelDesignerWorkflow.md`  
**Trading Setup**: See `Assets/TradingDesignerWorkflow.md`  
**AI Behavior**: See `Assets/AIDesignerWorkflow.md`  
**Way/Guild**: See `Assets/WayNetworkQuickStart.md`

### "Where is the documentation for [System]?"

All system documentation is in the **Assets/** directory with naming pattern:
- `[System]Guide.md` - Comprehensive guide
- `[System]Workflow.md` - Designer workflow
- `[System]QuickReference.md` - Quick reference
- `[System]Templates.md` - YAML templates

### "How do I validate my YAML/assets?"

```bash
# Validate YAML schema
python SchemaValidator.py --yaml my_data.yaml

# Validate assets
python AdastreaAssetValidator.py

# Full validation
python SetupCheck.py
```

### "How do I build the project?"

Build using Unreal Editor or IDE:
```bash
# Validate environment first
python SetupCheck.py

# Open the .uproject file in Unreal Engine Editor, or
# Open the generated .sln file in Visual Studio
# Build in Development Editor configuration
```

### "What coding standards should I follow?"

See **CODE_STYLE.md** for comprehensive C++ and Blueprint standards.

Also reference **.github/copilot-instructions.md** for AI assistant configuration with embedded standards.

### "How do I contribute?"

See **CONTRIBUTING.md** for contribution guidelines.

### "Where are the YAML templates?"

YAML templates are located in:
- `Assets/SpaceshipTemplates/` - Ship templates
- `Assets/PersonnelTemplates/` - Personnel templates
- `Assets/FactionAITemplates/` - Faction AI templates
- `Assets/PersonnelAITemplates/` - Personnel AI templates
- `Assets/CombatTemplates/` - Combat templates
- `Assets/TradingTemplates/` - Trading templates
- `Assets/InventoryTemplates/` - Inventory templates
- `Assets/ShipCustomizationTemplates/` - Ship customization templates
- `Assets/StationModuleTemplates/` - Station module templates
- `Assets/WayNetworkTemplates/` - Way network templates

### "What are the design patterns used?"

See **ARCHITECTURE.md** for comprehensive design patterns.

Key patterns:
1. **Data Asset Pattern** - Primary content creation
2. **BlueprintNativeEvent Pattern** - C++/Blueprint flexibility
3. **Trait System Pattern** - Reusable modifiers
4. **Relationship System Pattern** - Social mechanics

### "How do I set up the editor?"

See **EDITOR_IMPLEMENTATION_GUIDE.md** for custom editor tools.

### "How do I use procedural generation?"

See:
- **PROCEDURAL_GENERATION_GUIDE.md** - Comprehensive guide
- **PROCEDURAL_GENERATION_QUICKSTART.md** - Quick start
- **PROCEDURAL_GENERATION_EXAMPLES.md** - Examples

### "How do I populate a scene/level?"

See:
- **SCENE_POPULATION_GUIDE.md** - Comprehensive guide
- **SCENE_POPULATION_QUICK_REFERENCE.md** - Quick reference

### "How do I use Google Sheets integration?"

See:
- **GOOGLE_SHEETS_GUIDE.md** - Setup and usage
- **GOOGLE_SHEETS_TEMPLATES.md** - Template formats

---

## File Locations Reference

### Essential Documentation Entry Points

| Topic | Primary File | Location |
|-------|-------------|----------|
| Project Overview | README.md | Root |
| Architecture | ARCHITECTURE.md | Root |
| Development Roadmap | ROADMAP.md | Root |
| Coding Standards | CODE_STYLE.md | Root |
| Contributing | CONTRIBUTING.md | Root |
| AI Configuration | copilot-instructions.md | .github/ |
| Director Analysis | ADASTREA_DIRECTOR_ANALYSIS.md | Root |

### System Documentation (Assets/)

| System | Primary Guide | Quick Reference | Workflow |
|--------|--------------|-----------------|----------|
| Spaceship | SpaceshipDataAssetGuide.md | (in guide) | SpaceshipDesignerWorkflow.md |
| Faction | FactionSetupGuide.md | FactionSystemQuickReference.md | (in guide) |
| Personnel | PersonnelSystemGuide.md | (in guide) | PersonnelDesignerWorkflow.md |
| Trading | TradingSystemGuide.md | (in guide) | TradingDesignerWorkflow.md |
| Way System | WaySystemGuide.md | WaySystemQuickReference.md | WayNetworkWorkflow.md |
| AI | AISystemIntegration.md | AI_QUICK_START.md | AIDesignerWorkflow.md |
| Combat | CombatSystemGuide.md | CombatQuickReference.md | (in guide) |
| Navigation | NavigationSystemGuide.md | - | - |
| Quest | QuestSystemGuide.md | - | - |
| Material | MaterialSystemGuide.md | MaterialQuickStart.md | - |
| HUD/UI | HUDSystemGuide.md | HUD_QUICK_REFERENCE.md | - |
| Ship Control | ShipControlGuide.md | - | - |
| Inventory | InventorySystemGuide.md | - | - |
| Homeworld | HomeworldSystemGuide.md | - | - |
| Customization | ShipCustomizationGuide.md | - | - |
| Interior | SpaceshipInteriorGuide.md | - | - |

### Quick Start Documents

| Purpose | File | Location |
|---------|------|----------|
| Project Setup | QUICKSTART.md | Root |
| Content Creation | CONTENT_CREATION_QUICKSTART.md | Root |
| Generators | GENERATOR_QUICK_START.md | Root |
| Procedural Gen | PROCEDURAL_GENERATION_QUICKSTART.md | Root |
| Way Network | WayNetworkQuickStart.md | Assets/ |
| Material System | MaterialQuickStart.md | Assets/ |
| AI System | AI_QUICK_START.md | Assets/ |

### Reference Documents

| Topic | File | Location |
|-------|------|----------|
| Commands | CHEATSHEET.md | Root |
| Data Management | DATA_MANAGEMENT_QUICK_REFERENCE.md | Root |
| Optimization | OPTIMIZATION_QUICK_REFERENCE.md | Root |
| Testing | TESTING_QUICK_REFERENCE.md | Root |
| Scene Population | SCENE_POPULATION_QUICK_REFERENCE.md | Root |

### Setup

| Purpose | File | Location |
|---------|------|----------|
| Setup Validation | SetupCheck.py | Root |

### Automation & Scripts

| Purpose | File | Location |
|---------|------|----------|
| Generator Scripts | GENERATOR_SCRIPTS_README.md | Root |
| Utility Scripts | SCRIPTS_README.md | Root |
| Testing Automation | TESTING_AUTOMATION_GUIDE.md | Root |
| YAML Import | YAML_IMPORT_GUIDE.md | Root |
| Schema Validation | SCHEMA_GUIDE.md | Root |
| Google Sheets | GOOGLE_SHEETS_GUIDE.md | Root |

### Templates & Examples

| Type | Files | Location |
|------|-------|----------|
| Spaceship | SpaceshipTemplates.md + templates/ | Assets/ |
| Personnel | PersonnelTemplates.md + templates/ | Assets/ |
| Way Network | README.md | Assets/WayNetworkTemplates/ |
| Data Assets | EXAMPLE_DATA_ASSETS.md | Root |
| Procedural Gen | PROCEDURAL_GENERATION_EXAMPLES.md | Root |
| Google Sheets | GOOGLE_SHEETS_TEMPLATES.md | Root |

---

## Integration with Adastrea-Director

### Director-Specific Guidance

The **Adastrea-Director** tool should:

1. **Prioritize** this document (DIRECTOR_KNOWLEDGE_BASE.md) as the primary knowledge entry point
2. **Ingest** all documents referenced in this file for comprehensive understanding
3. **Use** the file locations reference to quickly navigate to specific documentation
4. **Understand** the data-driven, designer-friendly philosophy of the project
5. **Recognize** the modular system architecture and clean separation of concerns
6. **Follow** coding standards from CODE_STYLE.md and .github/copilot-instructions.md

### Recommended Ingestion Order

For optimal understanding, the Director should ingest documentation in this order:

1. **Foundation** (4 files)
   - DIRECTOR_KNOWLEDGE_BASE.md (this file)
   - README.md
   - ARCHITECTURE.md
   - ROADMAP.md

2. **Standards & Process** (3 files)
   - CODE_STYLE.md
   - CONTRIBUTING.md
   - .github/copilot-instructions.md

3. **Core System Guides** (All Assets/*.md files)
   - SpaceshipDataAssetGuide.md
   - FactionSetupGuide.md
   - PersonnelSystemGuide.md
   - TradingSystemGuide.md
   - WaySystemGuide.md
   - (and all other system guides)

4. **Workflows** (All *Workflow.md files)
5. **Quick References** (All *QuickReference.md files)
6. **Templates** (All *Templates.md files)
7. **Automation Guides** (All automation-related guides)
8. **Troubleshooting** (All troubleshooting guides)

### Common Director Queries

**"How is the project structured?"**  
→ See [Documentation Structure](#documentation-structure) and [Code Organization](#code-organization)

**"What systems are available?"**  
→ See [Core Game Systems](#core-game-systems)

**"How do I create [content type]?"**  
→ See [Development Workflows](#development-workflows) and specific system guides

**"Where is documentation for [feature]?"**  
→ See [File Locations Reference](#file-locations-reference)

**"What are the coding standards?"**  
→ See CODE_STYLE.md and .github/copilot-instructions.md

**"How do I validate/test my changes?"**  
→ See [Automation & Tooling](#automation--tooling) and TESTING_AUTOMATION_GUIDE.md

---

## Additional Resources

### External Links

- **Unreal Engine Documentation**: <https://docs.unrealengine.com/>
- **Unreal Engine Coding Standard**: <https://docs.unrealengine.com/5.6/en-US/epic-cplusplus-coding-standard-for-unreal-engine/>
- **Adastrea Repository**: <https://github.com/Mittenzx/Adastrea>
- **Adastrea-Director Repository**: <https://github.com/Mittenzx/Adastrea-Director>

### Related Analysis Documents

- **ADASTREA_DIRECTOR_ANALYSIS.md** - Comprehensive analysis of the Director tool

### Change Management

- **CHANGELOG.md** - Version history and changes
- **ROADMAP_UPDATE_GUIDELINES.md** - How to update the roadmap

---

## Document Maintenance

### Last Updated

**Date**: November 13, 2025  
**Updated By**: GitHub Copilot (SWE Agent)  
**Version**: 1.0

### Update Guidelines

This document should be updated when:
- New systems are added
- Documentation structure changes significantly
- File locations change
- New development workflows are established
- Integration patterns change

To update:
1. Modify relevant sections
2. Update version and date
3. Update CHANGELOG.md
4. Re-ingest with Adastrea-Director

---

**Status**: ✅ Ready for Adastrea-Director Ingestion

This knowledge base provides a comprehensive overview for the Adastrea-Director AI tool to understand the project structure, systems, workflows, and documentation organization. Use this as the primary entry point for all project queries and task planning.
