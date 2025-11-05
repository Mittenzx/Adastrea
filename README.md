# Adastrea - Open World Space Flight Game

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.x-blue.svg)](https://www.unrealengine.com/)

> An open-world space flight game featuring exploration, combat, faction diplomacy, crew management, and dynamic trading economies.

## Table of Contents

- [About](#about)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Core Features](#core-features)
  - [Spaceship System](#spaceship-data-asset-system)
  - [Space Station System](#space-station-editor-system)
  - [Faction System](#faction-system)
  - [Personnel Management](#personnel-management-system)
  - [AI System](#faction-and-personnel-ai-system)
  - [Trading System](#trading-system)
  - [Material System](#material-system)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## About

**Adastrea** is an open-world space flight game built on Unreal Engine. Players can explore vast galaxies, engage in thrilling space battles, discover new planets, manage space stations, build crew relationships, and participate in a dynamic galactic economy.

### Key Highlights

- 🚀 **Data-Driven Design**: All game content uses Blueprint Data Assets
- 🎮 **Designer-Friendly**: Minimal C++ knowledge required
- 🏗️ **Modular Architecture**: Systems work independently and integrate seamlessly
- 📚 **Comprehensive Documentation**: Guides, templates, and workflows for everything
- 🔧 **Blueprint First**: Full Blueprint exposure for all systems

## Quick Start

### Prerequisites

- **Unreal Engine 5.5** (check the .uproject file for exact version)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++ development
- **Git** for version control

### Robust Setup Validation

Before building the project, it's **highly recommended** to run our automated setup validation scripts to ensure your development environment is properly configured. These scripts check for all prerequisites, validate project structure, and can optionally perform static analysis.

#### Option 1: Shell Script (Linux/Mac/WSL)

```bash
# Basic validation
./SetupCheck.sh

# Validation with build folder cleanup
./SetupCheck.sh --clean

# Validation with static analysis (requires cppcheck)
./SetupCheck.sh --analyze

# Display help
./SetupCheck.sh --help
```

#### Option 2: Python Script (Cross-Platform)

```bash
# Basic validation (works on Windows, Linux, Mac)
python SetupCheck.py

# Validation with static analysis (requires cppcheck)
python SetupCheck.py --analyze

# Verbose output with detailed information
python SetupCheck.py --verbose

# Display help
python SetupCheck.py --help
```

The validation scripts check for:
- ✓ Unreal Engine version compatibility (5.5)
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
   ./SetupCheck.sh
   
   # Or cross-platform Python
   python SetupCheck.py
   ```
   
   This validates your environment and identifies any missing prerequisites before building.

3. **Generate Project Files**:
   - Right-click on the `.uproject` file
   - Select "Generate Visual Studio project files"

4. **Build the project**:
   - Open the generated `.sln` file in Visual Studio
   - Build in "Development Editor" configuration
   - Or open the `.uproject` file directly in Unreal Engine

5. **Verify Installation**:
   - Launch the editor
   - Check that all example Data Assets load correctly
   - Run the game with the Play button
   - (Optional) Run [AdastreaAssetValidator.py](AdastreaAssetValidator.py) in the Unreal Editor Python console for asset validation

### First Steps

1. **For Designers**: Read [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)
2. **For Programmers**: Read [CONTRIBUTING.md](CONTRIBUTING.md) and [ARCHITECTURE.md](ARCHITECTURE.md)
3. **For Testing**: Create a Test Mode level - see [Docs/TestMode_INDEX.md](Docs/TestMode_INDEX.md) for complete guide
4. **Quick Reference**: See individual system guides in the Assets/ folder

## Project Structure
```
Adastrea/
├── .editorconfig           # Code formatting configuration
├── .gitignore              # Git ignore patterns
├── ARCHITECTURE.md         # Architecture and design documentation
├── CHANGELOG.md            # Version history and changes
├── CONTRIBUTING.md         # Contribution guidelines
├── LICENSE                 # MIT License
├── README.md               # This file
│
├── Assets/                 # Documentation and YAML templates
│   ├── *Guide.md          # Comprehensive system guides
│   ├── *Templates.md      # Reference documentation
│   ├── *Workflow.md       # Designer workflows
│   ├── *SUMMARY.md        # Technical summaries
│   ├── FactionAITemplates/    # YAML templates for faction AI
│   ├── PersonnelAITemplates/  # YAML templates for personnel AI
│   ├── PersonnelTemplates/    # YAML templates for crew roles
│   ├── SpaceshipTemplates/    # YAML templates for ships
│   └── TradingTemplates/      # YAML templates for trading
│
├── Blueprints/             # Blueprint documentation
│
├── Config/                 # Unreal Engine configuration
│
├── Content/                # Game content (UE assets)
│   ├── Blueprints/        # Blueprint classes
│   ├── Homeworlds/        # Homeworld assets
│   ├── Maps/              # Level maps
│   ├── Planets/           # Planet assets
│   ├── SpaceStations/     # Station assets and modules
│   └── UI/                # UI widgets
│
└── Source/                 # C++ source code
    ├── AdastreaEditor/    # Editor customizations module
    └── Adastrea/          # Core game systems
        ├── AI/            # AI logic classes
        ├── Characters/    # Character and personnel
        ├── Factions/      # Faction system
        ├── Materials/     # Material system
        ├── Planets/       # Planet classes
        ├── Player/        # Player-specific systems
        ├── Ships/         # Spaceship classes
        ├── Stations/      # Space station classes
        ├── Trading/       # Trading economy
        └── UI/            # UI widgets
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
- **YAML Templates**: [Assets/SpaceshipTemplates/](Assets/SpaceshipTemplates/) - Copy-paste ready ship configurations

**Quick Usage:**
1. Create a new SpaceshipDataAsset in the Content Browser
2. Use YAML templates from `Assets/SpaceshipTemplates/` as reference
3. Configure all stat categories (core, combat, mobility, utility, operational, advanced, lore)
4. Test with Blueprint-callable helper functions
5. Integrate into your spaceship Blueprint actors

### Space Station Editor System
A modular space station construction system that enables designers to build, customize, and manage space stations through Blueprint-friendly C++ classes.

**Key Components:**
- `ASpaceStation` - Core station actor with module management (add, remove, move modules)
- `ASpaceStationModule` - Base class for station modules with type classification
- `UStationEditorWidget` - Widget class providing UI hooks for station editing
- `EStationModuleGroup` - Module categorization enum (Docking, Power, Storage, Defence, etc.)

**For detailed implementation guide, see:** [Content/Blueprints/STATION_EDITOR_README.md](Content/Blueprints/STATION_EDITOR_README.md)

**Quick Usage:**
1. Create Blueprint extending `ASpaceStation` for your custom station
2. Create module Blueprints extending `ASpaceStationModule` (e.g., docking bays, trade hubs)
3. Create Widget Blueprint extending `UStationEditorWidget` for the editor UI
4. Use Blueprint-callable functions to add, remove, and position modules

### Faction System
The game includes a flexible faction system that allows designers to create and manage various galactic factions. Each faction has unique attributes, colors, relationships, traits, and diplomacy options. The system includes homeworld-based faction relationships for diverse player starting experiences.

**Key Components:**
- `UFactionDataAsset` - C++ Data Asset class for defining factions
- `UHomeworldDataAsset` - C++ Data Asset class for player starting locations with initial faction relationships
- `FFactionTrait` - Struct for faction traits with gameplay modifiers
- `FFactionRelationship` - Struct for inter-faction diplomacy
- `FFactionRelationEntry` - Struct for homeworld faction starting reputations
- `ASpaceStation::Faction` - Reference property to assign factions to stations
- `ASpaceStationModule::ModuleFaction` - Module-level faction assignment
- 10 pre-designed faction templates (Solaris Union, Ironclad Consortium, etc.)

**Advanced Features:**
- **Trait System**: Factions can have multiple traits with gameplay modifiers
  - Blueprint-callable functions: `HasTrait()`, `GetTraitModifier()`, `GetTraits()`
  - Categories: Military, Economic, Scientific, Diplomatic, Special
  - Support for runtime trait addition/removal (see IterationGuide.md)
  
- **Diplomacy System**: Define relationships between factions
  - Alliance and war status tracking
  - Relationship values (-100 to 100)
  - Trade modifiers based on relationships
  - Blueprint-callable functions: `IsAlliedWith()`, `IsAtWarWith()`, `GetRelationship()`

- **Station Integration**: Factions integrate with station and module systems
  - Tech level restrictions on modules
  - Faction-specific traits affect gameplay
  - Blueprint-callable hooks on stations: `HasFactionTrait()`, `IsAlliedWithFaction()`

- **Homeworld System**: Define player starting locations with unique faction relationships
  - Create diverse starting experiences for players
  - Initialize player faction reputation based on chosen homeworld
  - Blueprint-callable functions: `GetStartingReputation()`, `HasRelationshipWith()`
  - Store player reputation as TMap for efficient runtime queries

**Documentation:**
- **Complete Guide**: [Assets/BlueprintFactionAssetGuide.md](Assets/BlueprintFactionAssetGuide.md) - Step-by-step faction creation
- **Faction Templates**: [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md) - 10 pre-made factions with full specs
- **Homeworld System**: [Assets/HomeworldSystemGuide.md](Assets/HomeworldSystemGuide.md) - Homeworld setup and faction relationships
- **Blueprint Workflows**: [Assets/BlueprintWorkflowTemplates.md](Assets/BlueprintWorkflowTemplates.md) - UI implementation examples
- **Homeworld Blueprints**: [Blueprints/HomeworldBlueprintTemplates.md](Blueprints/HomeworldBlueprintTemplates.md) - Blueprint logic for reputation system
- **Playtesting Guide**: [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md) - Verification and QA checklist
- **Iteration Guide**: [Assets/IterationGuide.md](Assets/IterationGuide.md) - Expanding faction behaviors and systems
- **Designer Onboarding**: [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md) - Best practices and quick start

**Quick Usage:**

*Creating Factions:*
1. Create a new FactionDataAsset in the Content Browser
2. Configure faction properties (name, colors, attributes, traits, relationships)
3. Assign the faction to any Space Station via the Faction property
4. Use Blueprint-callable functions to query traits and diplomacy in gameplay

### Faction and Personnel AI System
The game includes a complete AI framework for data-driven faction and personnel behaviors, supporting peaceful/exploration early-game priorities and personality-driven decision making.

**Key Components:**
- `UNPCLogicBase` - Base class for all NPC AI logic with BlueprintNativeEvent hooks
- `UFactionLogic` - Faction-level strategic AI (macro decisions: expansion, trade, diplomacy)
- `UPersonnelLogic` - Individual crew AI (micro decisions: tasks, social, training)
- Early Game Priorities - Configurable peaceful/exploration focus for factions
- 8 Personality Dispositions - Cautious, Bold, Analytical, Empathetic, Pragmatic, Curious, Disciplined, Creative
- Truces and Relationships - Diplomatic systems for peaceful gameplay
- Task Priorities - Dynamic daily scheduling for personnel based on personality

**Advanced Features:**
- **Blueprint-Friendly Extension**: All AI logic uses BlueprintNativeEvent pattern
  - Override any behavior in Blueprints without C++ knowledge
  - Call parent implementation when needed
  - Mix C++ performance with Blueprint flexibility

- **Peaceful Early-Game AI**: Factions support exploration-focused gameplay
  - Configurable priority system (0-10 scale for each activity)
  - Automatic transition from early to mid/late game
  - Truce system for maintaining peace
  - Trade and diplomacy emphasis over conflict

- **Personality-Driven Personnel**: 8 dispositions affect all decisions
  - Curious: Exploration and research focus
  - Disciplined: Protocol adherence, by-the-book
  - Empathetic: People-focused, supportive
  - Pragmatic: Efficiency and results
  - Bold: Risk-taking, decisive
  - Cautious: Safety-first, methodical
  - Analytical: Logic and calculation
  - Creative: Innovation and unconventional solutions

- **Social and Relationship Systems**: Dynamic crew interactions
  - Friendship and rivalry development
  - Personality compatibility affects collaboration
  - Morale impacts based on social dynamics
  - Command hierarchy with manager/report relationships

**YAML Templates:**
- `FactionAI/PeacefulExplorers.yaml` - Exploration-focused peaceful faction
- `FactionAI/TradeCoalition.yaml` - Commerce-focused merchant faction
- `PersonnelAI/CuriousExplorer.yaml` - Inquisitive science officer
- `PersonnelAI/DisciplinedSecurity.yaml` - By-the-book security officer
- `PersonnelAI/EmpatheticMedic.yaml` - Caring medical officer

**Documentation:**
- **Faction AI Guide**: [Assets/FactionAISetupGuide.md](Assets/FactionAISetupGuide.md) - Complete faction AI setup
- **Personnel AI Guide**: [Assets/PersonnelAISetupGuide.md](Assets/PersonnelAISetupGuide.md) - Complete personnel AI setup
- **Designer Workflow**: [Assets/AIDesignerWorkflow.md](Assets/AIDesignerWorkflow.md) - No C++ workflow for designers
- **System Integration**: [Assets/AISystemIntegration.md](Assets/AISystemIntegration.md) - Connecting AI to game systems
- **YAML Templates**: [Assets/FactionAITemplates/](Assets/FactionAITemplates/) and [Assets/PersonnelAITemplates/](Assets/PersonnelAITemplates/)

**Quick Usage:**

*Creating Faction AI:*
1. Create a FactionDataAsset with traits and relationships
2. Create a Blueprint based on UFactionLogic
3. Assign FactionDataAsset and configure Early Game Priorities
4. Override BlueprintNativeEvents to customize behavior (optional)
5. Integrate with ships/stations via component or reference

*Creating Personnel AI:*
1. Create a PersonnelDataAsset with skills, traits, and relationships
2. Create a Blueprint based on UPersonnelLogic
3. Assign PersonnelDataAsset and set Disposition
4. Override BlueprintNativeEvents for custom personality (optional)
5. Add to character actors and update in Tick

*For Designers (No C++ Required):*
- Use YAML templates as reference for values
- Configure all behavior through Data Assets
- Customize decisions in Blueprint visual scripting
- Test and iterate without recompiling
- All AI hooks are BlueprintNativeEvents

### Personnel Management System
The game includes a comprehensive personnel/crew management system for creating and managing crew members, staff, and character progression.

**Key Components:**
- `UPersonnelDataAsset` - C++ Data Asset class for defining personnel with 31 role types
- `FPersonnelTrait` - Struct for personnel traits with gameplay modifiers
- `FPersonnelRelationship` - Struct for interpersonal relationships
- `FPersonnelSkill` - Struct for skill tracking with levels and experience
- `FPerformanceMetric` - Struct for tracking effectiveness over time
- 7 ready-to-use YAML templates for common roles

**Advanced Features:**
- **Role System**: 31 distinct roles from Captain to Janitor
  - Command: Captain, XO, Station Manager
  - Flight: Pilot, Navigator
  - Technical: Engineer, AI Operator, Researcher
  - Combat: Gunnery Chief, Gunner, Security Officer
  - Medical: Medical Officer, Science Officer
  - Support: Quartermaster, Chef, Bartender, Janitor
  - Specialized: Diplomat, Trader, Salvage Specialist, Mining Specialist, Smuggler
  - General: Crew Member, Station Staff, Entertainment, Civilian, Ambassador

- **Skill System**: Multi-level skill tracking with experience points
  - Blueprint-callable functions: `GetSkillByName()`, `GetSkillLevel()`, `HasSpecialty()`
  - Individual skill levels (1-10) and experience points
  - Support for skill progression and career development

- **Status Tracking**: Comprehensive personnel condition monitoring
  - Health, Morale, Fatigue, Loyalty, Reputation
  - Blueprint-callable functions: `IsInGoodCondition()`, `NeedsRest()`, `IsLoyal()`, `HasLowMorale()`
  - Real-time status affects performance

- **Relationship System**: Track interpersonal connections
  - Friend, Rival, Mentor, Professional, Family relationships
  - Relationship strength (-100 to 100)
  - Blueprint-callable functions: `IsFriendsWith()`, `IsRivalWith()`, `GetRelationshipStrength()`

- **Performance Metrics**: Track effectiveness and career history
  - Multiple customizable performance metrics
  - Trend tracking (improving/declining)
  - Past assignments with ratings
  - Blueprint-callable functions: `GetPerformanceMetric()`, `GetAveragePerformance()`, `IsPerformingWell()`

- **Command Hierarchy**: Build organizational structures
  - Manager/Direct Report relationships
  - Department assignment
  - Role-based queries: `IsInLeadershipRole()`, `IsInCombatRole()`, `GetDirectReportCount()`

- **Employment System**: Contract and compensation management
  - Salary tracking
  - Contract duration and remaining time
  - Employment history

- **Trait System**: Unique characteristics with gameplay modifiers
  - Blueprint-callable functions: `HasTrait()`, `GetTraitModifier()`, `GetTraits()`
  - Support for stacking modifiers
  - Personality types and descriptions

**YAML Templates:**
- `Captain.yaml` - Ship commander template with leadership stats
- `Pilot.yaml` - Flight operations specialist
- `Gunner.yaml` - Weapons operator template
- `Engineer.yaml` - Technical specialist template
- `MedicalOfficer.yaml` - Healthcare provider template
- `StationManager.yaml` - Station operations manager
- `CrewMember_Generic.yaml` - General crew member template

**Documentation:**
- **Complete Guide**: [Assets/PersonnelSystemGuide.md](Assets/PersonnelSystemGuide.md) - Full system documentation
- **Role Templates**: [Assets/PersonnelTemplates.md](Assets/PersonnelTemplates.md) - All 31 roles with stats and examples
- **Designer Workflow**: [Assets/PersonnelDesignerWorkflow.md](Assets/PersonnelDesignerWorkflow.md) - Step-by-step creation workflows
- **YAML Templates**: [Assets/PersonnelTemplates/](Assets/PersonnelTemplates/) - 7 ready-to-use templates

**Quick Usage:**
1. Create a new PersonnelDataAsset in the Content Browser
2. Configure personnel properties (role, skills, status, relationships, traits)
3. Use YAML templates as reference for typical values and structure
4. Assign to ships/stations via CurrentAssignment property
5. Use Blueprint-callable functions to query personnel data in gameplay
6. Build command hierarchies using Manager ID and Direct Reports
7. Create relationships between personnel for team dynamics

## Documentation

### Quick Start Guides
- [Designer Onboarding](Assets/DesignerOnboarding.md) - Start here if you're new to the project
- [Architecture Overview](ARCHITECTURE.md) - Technical overview of project structure
- [Contributing Guidelines](CONTRIBUTING.md) - How to contribute to the project

### System Documentation

#### Core Systems
- [Spaceship System Guide](Assets/SpaceshipDataAssetGuide.md)
- [Space Station System](Content/Blueprints/STATION_EDITOR_README.md)
- [Faction System Guide](Assets/BlueprintFactionAssetGuide.md)
- [Personnel System Guide](Assets/PersonnelSystemGuide.md)

#### Advanced Systems
- [AI System Overview](Assets/AI_QUICK_START.md)
  - [Faction AI Setup](Assets/FactionAISetupGuide.md)
  - [Personnel AI Setup](Assets/PersonnelAISetupGuide.md)
  - [AI Designer Workflow](Assets/AIDesignerWorkflow.md)
- [Trading System Guide](Assets/TradingSystemGuide.md)
- [Material System Guide](Assets/MaterialSystemGuide.md)
- [Homeworld System Guide](Assets/HomeworldSystemGuide.md)

#### Templates and References
- [Spaceship Templates](Assets/SpaceshipTemplates.md) - 6 pre-built ship designs
- [Faction Templates](Assets/FactionSetupGuide.md) - 10 pre-made factions
- [Personnel Templates](Assets/PersonnelTemplates.md) - 31 role templates
- [Blueprint Workflows](Assets/BlueprintWorkflowTemplates.md)
- [Trading API Reference](Assets/TradingAPIReference.md)

#### Integration Guides
- [AI System Integration](Assets/AISystemIntegration.md)
- [Material-Station Integration](Assets/MaterialStationIntegration.md)
- [Trading Blueprint Guide](Blueprints/TradingSystemBlueprintGuide.md)

#### Workflows and Best Practices
- [Spaceship Designer Workflow](Assets/SpaceshipDesignerWorkflow.md)
- [Personnel Designer Workflow](Assets/PersonnelDesignerWorkflow.md)
- [Trading Designer Workflow](Assets/TradingDesignerWorkflow.md)
- [Playtesting Checklist](Assets/PlaytestingChecklist.md)
- [Iteration Guide](Assets/IterationGuide.md)

### YAML Templates

All systems include ready-to-use YAML templates in their respective folders:
- `Assets/SpaceshipTemplates/` - Ship configurations
- `Assets/FactionAITemplates/` - Faction AI behaviors
- `Assets/PersonnelAITemplates/` - Personnel AI personalities
- `Assets/PersonnelTemplates/` - Crew member profiles
- `Assets/TradingTemplates/` - Trade items and markets

## Contributing

We welcome contributions! Here's how to get started:

1. **Read the Guidelines**: See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed instructions
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

See [CHANGELOG.md](CHANGELOG.md) for detailed version history and release notes.

## Support

- **Documentation Issues**: Open an issue with the "documentation" label
- **Bug Reports**: Use the bug report template
- **Feature Requests**: Use the feature request template
- **General Questions**: Start a discussion in the repository

---

**Built with ❤️ using Unreal Engine**
