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

### Material Data Asset System
A comprehensive material and resource management system that defines all physical items, resources, and commodities in the game. Materials integrate with station modules, storage systems, and the economy.

**Key Components:**
- `UMaterialDataAsset` - C++ Data Asset class for defining materials
- `EMaterialCategory` - Material classification (Mineral, Component, Electronics, ShipPart, Food, etc.)
- `EStorageType` - Storage requirements (Solid, Liquid, Gas, Refrigerated, Hazardous)
- `EMaterialRarity` - Rarity levels (Common to Legendary)
- Material stats: value, mass, tags, module compatibility

**Material Categories:**
- **Minerals**: Raw ores (iron ore, titanium ore, crystalline quartz)
- **Refined Minerals**: Processed materials (iron ingot, titanium ingot, steel alloy)
- **Components**: Manufactured parts (actuators, power cells, cooling systems)
- **Electronics**: Tech components (circuit boards, quantum processors, sensor arrays)
- **Construction Parts**: Building materials (hull plates, support beams, pressure seals)
- **Ship Parts**: Spacecraft components (reactor cores, thruster assemblies, shield generators)
- **Food**: Consumables (protein rations, hydroponic vegetables, nutrient paste)
- **Research**: Scientific items (research data, alien artifacts, experimental compounds)
- **Organic**: Biological materials (bio samples, medical supplies, genetic material)
- **Synthetic**: Manufactured substances (nanomaterials, synthetic fuel, polymers)

**Advanced Features:**
- **Tag System**: General gameplay tags for filtering and queries
  - Blueprint-callable functions: `HasTag()`, `GetTags()`
  - Categories: Metal, RawMaterial, Electronics, Construction, Energy, Food, Research, etc.
  
- **Module Integration**: Materials specify compatible station modules
  - Module tags define which modules can process materials
  - Blueprint-callable functions: `CanBeProcessedByModule()`, `GetModuleTags()`
  - Supports crafting chains and resource processing

- **Storage Management**: Automatic volume calculations and special requirements
  - Storage type determines handling needs
  - Blueprint-callable functions: `GetStorageVolume()`, `RequiresSpecialStorage()`
  - Integration with station storage modules

**Documentation:**
- **Complete Guide**: [Assets/MaterialSystemGuide.md](Assets/MaterialSystemGuide.md) - Full designer documentation
- **Material Templates**: [Assets/MaterialTemplates.yaml](Assets/MaterialTemplates.yaml) - 25+ example materials with all categories
- **Integration Notes**: Module connectivity, storage systems, and crafting chains

**Quick Usage:**
1. Create a new MaterialDataAsset in the Content Browser
2. Configure material properties (name, category, storage type, stats)
3. Add gameplay tags and module tags for integration
4. Use Blueprint-callable functions to query material properties in gameplay
5. Connect materials to station modules via module tags

## Contributing
Contributions are welcome! Please submit a pull request or create an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
