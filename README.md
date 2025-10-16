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
The game includes a flexible faction system that allows designers to create and manage various galactic factions. Each faction has unique attributes, colors, and relationships.

**Key Components:**
- `UFactionDataAsset` - C++ Data Asset class for defining factions
- `ASpaceStation::Faction` - Reference property to assign factions to stations
- 10 pre-designed faction templates (Solaris Union, Ironclad Consortium, etc.)

**For detailed setup instructions, see:** [Assets/FactionSetupGuide.md](Assets/FactionSetupGuide.md)

**Quick Usage:**
1. Create a new FactionDataAsset in the Content Browser
2. Configure faction properties (name, colors, attributes, etc.)
3. Assign the faction to any Space Station via the Faction property

## Contributing
Contributions are welcome! Please submit a pull request or create an issue for any suggestions or improvements.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
