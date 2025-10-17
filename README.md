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
│       ├── Player/         # Player-related systems (Homeworld)
│       ├── Ships/          # Spaceship actors and components
│       ├── Stations/       # Space station actors
│       └── UI/             # User interface widgets
└── README.md               # Project documentation
```

## Features

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

*Creating Homeworlds:*
1. Create a new HomeworldDataAsset in the Content Browser (e.g., `Content/Homeworlds/`)
2. Configure homeworld name, description, and ID
3. Add Faction Relations entries with starting reputation values for each faction (-100 to 100)
4. In character creation Blueprint, call `InitializeFactionReputationFromHomeworld()`
5. Store resulting FactionReputationMap (TMap<UFactionDataAsset*, int32>) on player character

## Contributing
Contributions are welcome! Please submit a pull request or create an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
