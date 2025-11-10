# Space Station Editor System

## Overview
This system provides C++ classes and Blueprint-ready hooks for creating a modular space station editor. The implementation focuses on core logic in C++ while allowing full UI customization through Blueprints.

## Created Files

### C++ Classes

#### 1. **ASpaceStation** (`Source/Adastrea/Stations/SpaceStation.h/cpp`)
Main station actor that manages all attached modules.

**Key Features:**
- `TArray<ASpaceStationModule*> Modules` - Stores all attached modules
- `AddModule(Module)` - Attaches a module to the station (simple version)
- `AddModuleAtLocation(Module, Location)` - Attaches a module at a specific location
- `RemoveModule(Module)` - Detaches a module from the station
- `MoveModule(Module, NewLocation)` - Repositions an attached module
- `GetModules()` - Returns all attached modules
- `GetModulesByType(Type)` - Filters modules by their type string
- `GetModuleCount()` - Returns the number of attached modules

#### 2. **ASpaceStationModule** (`Source/Adastrea/Stations/SpaceStationModule.h/cpp`)
Base class for all station modules (docking bays, trade hubs, etc.).

**Key Properties:**
- `FString ModuleType` - Identifies the module type (e.g., "Docking", "Commerce")
- `float ModulePower` - Power consumption or generation
- `EStationModuleGroup ModuleGroup` - Module category enum (Docking, Power, Storage, etc.)

#### 3. **UStationEditorWidget** (`Source/StationEditor/UI/StationEditorWidget.h/cpp`)
Widget class providing UI hooks for the station editor.

**Note:** This is part of the **StationEditor** runtime module, which contains all player-facing in-game station editing features.

**Key Features:**
- `CurrentStation` - Reference to the station being edited
- `AddModule(ModuleClass, Location)` - Spawns and adds a new module
- `RemoveModule(Module)` - Removes a module from the station
- `MoveModule(Module, NewLocation)` - Moves an existing module
- `GetAllModules()` - Returns all modules for UI population
- `SetStation(Station)` - Sets which station to edit
- `IsValidPlacement(Location)` - Validates module placement

### Documentation

#### **BP_StationEditor_WorkflowNotes.txt**
Comprehensive guide for implementing the Blueprint UI:
- Step-by-step Blueprint creation instructions
- Sample event graph workflows
- UI layout recommendations
- Implementation checklist
- Advanced feature suggestions

## Usage

### For C++ Developers
All classes are ready to use. Simply include the headers in your code:
```cpp
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "UI/StationEditorWidget.h"  // From StationEditor module
```

**Module Dependency:** The StationEditor module depends on the Adastrea module. Make sure your Build.cs file includes both modules if you need to reference station editor functionality.

### For Blueprint Developers
1. Create a Blueprint extending `ASpaceStation` (e.g., `BP_SpaceStation`)
2. Create Blueprint variants extending `ASpaceStationModule` for different module types
3. Create a Widget Blueprint extending `UStationEditorWidget` (e.g., `WBP_StationEditor`)
4. Design your UI using UMG
5. Call the exposed C++ functions from Blueprint nodes
6. Refer to `BP_StationEditor_WorkflowNotes.txt` for detailed workflows

## Example Blueprint Workflow

### Adding a Module:
1. User clicks "Add Docking Bay" button
2. Button event calls `AddModule` with `BP_StationModule_DockingBay` class and placement location
3. New module is spawned and attached to the station
4. UI updates to show the new module

### Removing a Module:
1. User selects a module from the list
2. User clicks "Remove" button  
3. Button event calls `RemoveModule` with selected module
4. Module is detached from the station
5. UI updates to reflect the change

## Blueprint Callable Functions

All functions marked with `UFUNCTION(BlueprintCallable)` are available as Blueprint nodes:

**From ASpaceStation:**
- Add Module (simple version)
- Add Module At Location
- Remove Module
- Move Module
- Get Modules
- Get Modules By Type
- Get Module Count

**From UStationEditorWidget:**
- Add Module (spawns and adds)
- Remove Module
- Move Module
- Get All Modules
- Set Station
- Is Valid Placement

## Next Steps

1. ✅ C++ foundation classes created
2. ✅ Trait and diplomacy systems implemented
3. ✅ Faction integration with stations and modules
4. ✅ Comprehensive documentation created
5. ⏸ Create Blueprint variants (BP_SpaceStation, module BPs)
6. ⏸ Design Widget Blueprint UI (WBP_StationEditor)
7. ⏸ Implement event graphs for add/remove/move operations
8. ⏸ Add visual feedback (selection highlights, placement preview)
9. ⏸ Test in-editor and in-game

## Additional Documentation

**For detailed Blueprint implementation:**
- [Assets/BlueprintWorkflowTemplates.md](../Assets/BlueprintWorkflowTemplates.md) - Complete UI workflows and sample graphs
- [Assets/BlueprintFactionAssetGuide.md](../Assets/BlueprintFactionAssetGuide.md) - Faction creation and integration
- [Assets/PlaytestingChecklist.md](../Assets/PlaytestingChecklist.md) - Testing and verification guide
- [Assets/IterationGuide.md](../Assets/IterationGuide.md) - Future expansion plans
- [Assets/DesignerOnboarding.md](../Assets/DesignerOnboarding.md) - Team onboarding and best practices

## Notes

- All C++ functions include null checks and validation
- Modules are attached to the station using Unreal's attachment system
- The system is designed to be extended - add custom properties and functions as needed
- No full UI is implemented - this provides the C++ hooks for Blueprint implementation
- Follow existing Unreal Engine patterns and conventions

## Integration with Existing Systems

This editor system can work alongside:
- Existing spaceship systems (`ASpaceship`, `ASpaceshipInterior`)
- Planet and sector systems
- Any gameplay systems that need to interact with stations

## Support

Refer to:
- `BP_StationEditor_WorkflowNotes.txt` - Detailed Blueprint implementation guide
- Unreal Engine documentation for UMG and Blueprint
- Existing code examples in `Source/Adastrea/Ships/` for similar patterns
