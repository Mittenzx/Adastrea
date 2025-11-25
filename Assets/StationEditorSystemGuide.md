# Station Editor System Guide

## Overview

The Station Editor System provides a comprehensive C++ framework for building and editing space stations in Adastrea. It offers a designer-friendly interface with full Blueprint exposure for customization.

## Core Classes

### UStationModuleCatalog (Data Asset)

Registry of all available module types that can be placed in a station.

**Location:** `Source/StationEditor/StationModuleCatalog.h/cpp`

**Key Features:**
- Stores `FStationModuleEntry` structs describing available modules
- Query functions with filtering by group and tech level
- Build cost tracking via `FStationBuildCost` struct

**Usage:**
```cpp
// Create a catalog Blueprint Data Asset
// Add entries for each module type:
// - ModuleClass: The module class to spawn
// - DisplayName: User-facing name
// - Description: Module functionality
// - BuildCost: Credits, materials, time
// - PreviewMesh: Visual preview mesh
// - RequiredTechLevel: Tech level required (1-10)
// - ModuleGroup: Category (Docking, Power, etc.)
```

### UStationEditorManager (UObject)

Core manager class that handles all station editing operations.

**Location:** `Source/StationEditor/StationEditorManager.h/cpp`

**Key Features:**
- Editing lifecycle: BeginEditing, EndEditing, Save, Cancel
- Module operations: PlaceModule, RemoveModule, MoveModule, RotateModule
- Validation: collision detection, tech level, funds checking
- Resource tracking: power consumption/generation balance
- Preview system integration
- Session-based undo on cancel

**Blueprint Events:**
- `OnEditingStateChanged` - Fires when entering/exiting edit mode
- `OnModulePlaced` - Fires when a module is successfully placed
- `OnModuleRemoved` - Fires when a module is removed
- `OnPowerBalanceChanged` - Fires when power balance changes

**Configuration Properties:**
- `ModuleCatalog` - Reference to the module catalog
- `PlayerTechLevel` - Current player's tech level (1-10)
- `PlayerCredits` - Available credits for building
- `bSnapToGrid` - Enable/disable grid snapping
- `bCheckCollisions` - Enable/disable collision checking
- `CollisionRadius` - Configurable collision detection radius

### UStationGridSystem (Actor Component)

Grid-based snapping system for module placement.

**Location:** `Source/StationEditor/StationGridSystem.h/cpp`

**Key Features:**
- Configurable grid size (default 100 units)
- 3D position snapping
- Rotation snapping with configurable angle increment
- Grid coordinate conversion
- Connection point validation (future expansion)

**Configuration Properties:**
- `GridSize` - Size of each grid cell in world units
- `bGridEnabled` - Enable/disable grid snapping
- `GridOrigin` - Offset for grid alignment
- `bSnapRotation` - Enable/disable rotation snapping
- `RotationSnapDegrees` - Rotation snap increment (e.g., 90°)

### AStationBuildPreview (Actor)

Preview actor showing module placement position with validity indication.

**Location:** `Source/StationEditor/StationBuildPreview.h/cpp`

**Key Features:**
- Dynamic mesh display based on selected module
- Color-coded validity (green = valid, red = invalid)
- Show/hide control
- Material customization

**Configuration Properties:**
- `ValidPlacementMaterial` - Material for valid placement
- `InvalidPlacementMaterial` - Material for invalid placement
- `ValidColor` / `InvalidColor` - Color overrides
- `PreviewOpacity` - Transparency level

## Data Structures

### FStationBuildCost

```cpp
USTRUCT(BlueprintType)
struct FStationBuildCost
{
    int32 Credits;              // Base credit cost
    TMap<FName, int32> Materials; // Resource requirements
    float BuildTime;            // Build time in seconds
};
```

### FStationModuleEntry

```cpp
USTRUCT(BlueprintType)
struct FStationModuleEntry
{
    TSubclassOf<ASpaceStationModule> ModuleClass;
    FText DisplayName;
    FText Description;
    FStationBuildCost BuildCost;
    TSoftObjectPtr<UStaticMesh> PreviewMesh;
    int32 RequiredTechLevel;      // 1-10
    EStationModuleGroup ModuleGroup;
};
```

### EModulePlacementResult

```cpp
UENUM(BlueprintType)
enum class EModulePlacementResult : uint8
{
    Success,
    InvalidModule,
    InvalidPosition,
    CollisionDetected,
    InsufficientPower,
    InsufficientTech,
    InsufficientFunds,
    NoStation,
    NotEditing
};
```

## Integration Guide

### Basic Setup

1. **Create a Module Catalog:**
   - Create a Blueprint Data Asset based on `UStationModuleCatalog`
   - Add entries for each available module type
   - Configure costs, tech levels, and descriptions

2. **Initialize the Editor Manager:**
   ```cpp
   UStationEditorManager* EditorManager = NewObject<UStationEditorManager>();
   EditorManager->ModuleCatalog = MyCatalog;
   EditorManager->PlayerTechLevel = PlayerState->GetTechLevel();
   EditorManager->PlayerCredits = PlayerState->GetCredits();
   ```

3. **Begin Editing a Station:**
   ```cpp
   if (EditorManager->BeginEditing(MyStation))
   {
       // Editing started successfully
   }
   ```

4. **Place Modules:**
   ```cpp
   ASpaceStationModule* NewModule = EditorManager->PlaceModule(
       AReactorModule::StaticClass(),
       CursorLocation,
       FRotator::ZeroRotator
   );
   ```

5. **Save or Cancel:**
   ```cpp
   // To save changes:
   EditorManager->Save();
   
   // To cancel and revert:
   EditorManager->Cancel();
   ```

### Blueprint Integration

All manager functions use `BlueprintNativeEvent`, allowing Blueprint override:

1. Create a Blueprint class based on `UStationEditorManager`
2. Override `PlaceModule_Implementation` for custom placement logic
3. Override `CanPlaceModule_Implementation` for custom validation
4. Bind to events for UI updates

### Preview System Usage

```cpp
// Show preview for a module class
EditorManager->ShowPreview(AReactorModule::StaticClass());

// Update preview position (call during cursor movement)
EditorManager->UpdatePreview(CursorWorldPosition, CursorRotation);

// Hide preview
EditorManager->HidePreview();
```

## Integration with Existing Systems

### ASpaceStation

The editor manager integrates with the existing `ASpaceStation` class:
- Uses `AddModuleAtLocation()` for placement
- Uses `RemoveModule()` for removal
- Uses `MoveModule()` for repositioning
- Reads `Modules` array for collision detection

### ASpaceStationModule

Works with the existing module system:
- Reads `ModulePower` for power balance calculations
- Reads `ModuleGroup` for catalog filtering
- Compatible with all existing module subclasses

### Faction System

- Station faction affects tech level availability
- Can be integrated with `UFactionDataAsset::TechnologyLevel`

## Best Practices

1. **Always check placement before placing:**
   ```cpp
   if (EditorManager->CanPlaceModule(ModuleClass, Position, Rotation) == EModulePlacementResult::Success)
   {
       EditorManager->PlaceModule(ModuleClass, Position, Rotation);
   }
   ```

2. **Use events for UI updates:**
   ```cpp
   EditorManager->OnPowerBalanceChanged.AddDynamic(this, &UMyWidget::UpdatePowerDisplay);
   ```

3. **Handle edit session lifecycle:**
   - Always call `Save()` or `Cancel()` to properly end sessions
   - `Cancel()` automatically reverts all changes

4. **Customize collision detection:**
   - Adjust `CollisionRadius` based on your module sizes
   - Override `CanPlaceModule` for complex validation

## Technical Notes

- All classes use `UCLASS(BlueprintType)` for Blueprint accessibility
- Log category: `LogAdastreaStations`
- Grid system is created automatically when editing begins
- Preview actor is spawned/reused across editing sessions

## See Also

- `ASpaceStation` - Core station actor
- `ASpaceStationModule` - Base module class
- `UStationEditorWidget` - UI widget (in `Source/StationEditor/UI/`)
- `EStationModuleGroup` - Module categories enum
