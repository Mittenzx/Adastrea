# AdAstra - Open World Space Flight Game

## Project Description
AdAstra is an open-world space flight game built on Unreal Engine. Players can explore vast galaxies, engage in thrilling space battles, and discover new planets. The game aims to provide an immersive experience with stunning visuals and dynamic gameplay.

## Quick Start Instructions
1. **Clone the repository**:
   ```bash
   git clone https://github.com/Mittenzx/adastrea.git
   cd adastrea
   ```

2. **Install Unreal Engine**:
   Make sure you have Unreal Engine installed. You can download it from the [Epic Games website](https://www.unrealengine.com/).

3. **Open the project**:
   Open the project by double-clicking the `.uproject` file in the cloned repository.

4. **Build the project**:
   If necessary, build the project by selecting "Build" from the "File" menu in Unreal Engine.

5. **Run the game**:
   Click the "Play" button in the Unreal Engine editor to start the game.

## Folder Structure
```
adastrea/
├── Assets/                 # Game assets and documentation
│   └── FactionSetupGuide.md # Faction system documentation
├── Blueprints/             # Game blueprints
├── Config/                 # Configuration files
├── Content/                # Game content (levels, materials)
├── Source/                 # Source code
│   ├── AdAstra/            # Main game module
│   ├── AdAstraEditor/      # Editor module
│   └── Adastrea/           # Game systems
│       ├── Characters/     # Character classes
│       ├── Factions/       # Faction system (Data Assets)
│       ├── Planets/        # Planet actors
│       ├── Ships/          # Spaceship actors and components
│       ├── Stations/       # Space station actors
│       └── UI/             # User interface widgets
└── README.md               # Project documentation
```

## Features

### Spaceship Data Asset System
A comprehensive spaceship specification system that allows designers to create detailed ship configurations as Blueprint Data Assets without writing C++ code.

**Key Components:**
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
The game includes a flexible faction system that allows designers to create and manage various galactic factions. Each faction has unique attributes, colors, relationships, traits, and diplomacy options.

**Key Components:**
- `UFactionDataAsset` - C++ Data Asset class for defining factions
- `FFactionTrait` - Struct for faction traits with gameplay modifiers
- `FFactionRelationship` - Struct for inter-faction diplomacy
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

**Documentation:**
- **Complete Guide**: [Assets/BlueprintFactionAssetGuide.md](Assets/BlueprintFactionAssetGuide.md) - Step-by-step faction creation
- **Faction Templates**: [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md) - 10 pre-made factions with full specs
- **Blueprint Workflows**: [Assets/BlueprintWorkflowTemplates.md](Assets/BlueprintWorkflowTemplates.md) - UI implementation examples
- **Playtesting Guide**: [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md) - Verification and QA checklist
- **Iteration Guide**: [Assets/IterationGuide.md](Assets/IterationGuide.md) - Expanding faction behaviors and systems
- **Designer Onboarding**: [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md) - Best practices and quick start

**Quick Usage:**
1. Create a new FactionDataAsset in the Content Browser
2. Configure faction properties (name, colors, attributes, traits, relationships)
3. Assign the faction to any Space Station via the Faction property
4. Use Blueprint-callable functions to query traits and diplomacy in gameplay

## Contributing
Contributions are welcome! Please submit a pull request or create an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
