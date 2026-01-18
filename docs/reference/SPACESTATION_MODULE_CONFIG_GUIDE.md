# SpaceStation Module Configuration Guide

**Created**: January 10, 2026  
**Issue Resolved**: Module dropdown empty in Class Defaults  
**Related Classes**: `ASpaceStation`, `ASpaceStationModule`

> **💡 Alternative Method Available**: This guide covers the **DefaultModuleClasses** runtime spawning method. For a visual, design-time approach where modules are visible in the editor, see **Child Actor Components** method in [TRADING_AND_DOCKING_COMPLETE_GUIDE.md](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md#step-2-add-components). Both methods are valid!

---

## Problem Overview

When trying to add modules to a SpaceStation in Class Defaults, the dropdown shows no results. This happens because the original `Modules` array uses actor pointers (`TArray<ASpaceStationModule*>`), which require existing instances in the level.

## Solution

Added `DefaultModuleClasses` property to configure which module types should spawn automatically.

---

## How to Configure Modules

### Step 1: Open SpaceStation Blueprint

1. Navigate to your SpaceStation Blueprint (e.g., `BP_SpaceStation`)
2. Open the Blueprint editor
3. Click **Class Defaults** button in toolbar

### Step 2: Add Module Classes

1. Find **Station | Configuration** category
2. Locate **Default Module Classes** array property
3. Click **+** button to add a new entry
4. Click the dropdown and select a module class:
   - `BP_DockingBayModule` - Large docking facilities
   - `BP_DockingPortModule` - Small docking ports
   - `BP_ReactorModule` - Power generation
   - `BP_CargoBayModule` - Storage
   - `BP_HabitationModule` - Living quarters
   - `BP_ShieldGeneratorModule` - Defense
   - *(and any other custom module Blueprint you've created)*

### Step 3: Configure Multiple Modules

Repeat Step 2 to add as many modules as needed. For example:
- Entry 0: `BP_DockingBayModule`
- Entry 1: `BP_DockingPortModule`
- Entry 2: `BP_ReactorModule`
- Entry 3: `BP_CargoBayModule`

### Step 4: Save and Test

1. **Save** the Blueprint
2. **Compile** the Blueprint
3. Place station in level
4. **Play** in editor
5. Modules spawn automatically at BeginPlay

---

## Understanding the Two Arrays

### 1. DefaultModuleClasses (Editor Configuration)

**Type**: `TArray<TSubclassOf<ASpaceStationModule>>`  
**Access**: `EditAnywhere` (configure in Class Defaults)  
**Purpose**: Define which module types to spawn  
**Location**: **Station | Configuration** category

**When to Use**:
- Setting up station module composition
- Creating station variants
- Defining default loadout

### 2. Modules (Runtime Tracking)

**Type**: `TArray<ASpaceStationModule*>`  
**Access**: `VisibleAnywhere` (read-only)  
**Purpose**: Track currently attached modules  
**Location**: **Station** category

**When to Use**:
- Querying attached modules at runtime
- Iterating through active modules
- Checking module count
- Finding modules by type

**Important**: Do NOT edit `Modules` in Class Defaults. It's populated automatically.

---

## Module Spawning Behavior

### Automatic Spawning

When station's `BeginPlay()` executes:

1. Checks if `DefaultModuleClasses` has entries
2. For each module class in array:
   - Spawns instance at station location
   - Attaches to station actor
   - Adds to `Modules` runtime array
3. Logs spawn count to output log

### Spawn Parameters

- **Location**: Station's current location
- **Rotation**: Station's current rotation
- **Collision**: Always spawn (ignore collisions)
- **Owner**: Station actor

### Log Output Example

```
LogAdastreaStations: SpaceStation::BeginPlay - Spawned default module: BP_DockingBayModule_C_0 for station BP_SpaceStation_C_0
LogAdastreaStations: SpaceStation::BeginPlay - Spawned default module: BP_ReactorModule_C_0 for station BP_SpaceStation_C_0
LogAdastreaStations: SpaceStation::BeginPlay - Station BP_SpaceStation_C_0 initialized with 2 default modules
```

---

## Common Use Cases

### Trading Station Setup

For MVP Trade Simulator, typical configuration:

```
DefaultModuleClasses:
  [0] BP_DockingBayModule      (player docking)
  [1] BP_DockingPortModule      (NPC ships)
  [2] BP_CargoBayModule         (storage)
  [3] BP_MarketplaceModule      (trading interface)
  [4] BP_HabitationModule       (station population)
```

### Military Station Setup

```
DefaultModuleClasses:
  [0] BP_DockingBayModule
  [1] BP_ShieldGeneratorModule
  [2] BP_TurretModule
  [3] BP_BarracksModule
  [4] BP_ReactorModule
```

### Mining Station Setup

```
DefaultModuleClasses:
  [0] BP_DockingPortModule
  [1] BP_ProcessingModule
  [2] BP_CargoBayModule
  [3] BP_FabricationModule
```

---

## Dynamic Module Management

### Adding Modules at Runtime

While `DefaultModuleClasses` handles initialization, you can still add modules dynamically:

**C++ Code**:
```cpp
// Spawn a new module
ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
    ModuleClass, 
    SpawnLocation, 
    SpawnRotation
);

// Add to station
Station->AddModule(NewModule);
```

**Blueprint**:
```
Event: Player Builds Module
├─ Spawn Actor from Class (Module Class)
├─ Get Station Reference
└─ Call AddModule (Station, Spawned Module)
```

### Removing Modules

**C++ Code**:
```cpp
Station->RemoveModule(ModuleToRemove);
```

**Blueprint**:
```
Event: Destroy Module
├─ Get Station Reference
└─ Call RemoveModule (Station, Module)
```

---

## Troubleshooting

### Problem: Dropdown Still Empty

**Cause**: No module Blueprint classes exist in project

**Solution**: Create module Blueprints first:
1. Right-click in Content Browser
2. **Blueprint Class** → Search for "SpaceStationModule"
3. Select parent class (e.g., `DockingBayModule`)
4. Name it (e.g., `BP_DockingBayModule`)
5. Now it appears in dropdown

### Problem: Modules Not Spawning

**Check These**:
- [ ] `DefaultModuleClasses` array has entries
- [ ] Module classes are valid (not null)
- [ ] Check Output Log for spawn errors
- [ ] BeginPlay is executing (check log)

**Output Log Search**:
```
Search for: "SpaceStation::BeginPlay"
Look for: "Spawned default module" or "Failed to spawn"
```

### Problem: Modules Spawning at Wrong Location

**Cause**: Default spawn location is station's location/rotation

**Solution**: 
- Adjust module positions in `AddModule()` implementation
- Or use `AddModuleAtLocation()` with relative offsets
- Consider adding module offset properties to module classes

### Problem: Duplicate Modules

**Cause**: Modules added both in `DefaultModuleClasses` AND manually placed in level

**Solution**: Choose one method:
- **Option A**: Use only `DefaultModuleClasses` (recommended)
- **Option B**: Place modules in level manually, leave `DefaultModuleClasses` empty

---

## Best Practices

### 1. Use DefaultModuleClasses for Templates

Configure `DefaultModuleClasses` in **Blueprint parent class** to create reusable station templates:
- `BP_TradingStation` → Trading modules
- `BP_MilitaryStation` → Defense modules
- `BP_MiningStation` → Processing modules

### 2. Module Order Matters

Modules spawn in array order. Consider:
- Critical modules first (docking, power)
- Optional modules last
- This helps with debugging (essential systems fail early)

### 3. One Configuration Method

Don't mix methods:
- ✅ **Good**: Configure in `DefaultModuleClasses` only
- ❌ **Bad**: Some in array, some manually placed, causes confusion

### 4. Test Spawning Early

After adding modules to array:
1. Save Blueprint
2. PIE (Play In Editor)
3. Check Output Log immediately
4. Verify spawn messages

### 5. Use Meaningful Names

When creating module Blueprints, use clear names:
- ✅ `BP_DockingBay_Large` (descriptive)
- ✅ `BP_Reactor_Nuclear` (specific)
- ❌ `BP_Module1` (unclear)

---

## Integration with Station Editor

The `DefaultModuleClasses` system works alongside the Station Editor:

1. **Initial Setup**: `DefaultModuleClasses` spawns base modules
2. **Runtime Editing**: Station Editor allows adding/removing modules
3. **Persistence**: Both systems update the same `Modules` array

**Station Editor functions**:
- `BeginEditing()` - Enters edit mode
- `PlaceModule()` - Adds module to station
- `RemoveModule()` - Removes module
- `SaveChanges()` - Persists modifications

---

## Migration Guide

### From Old System (Manual Placement)

**Before** (placing modules in level):
```
1. Place BP_SpaceStation in level
2. Place BP_DockingBayModule in level
3. Manually attach module to station
4. Repeat for each module
```

**After** (using DefaultModuleClasses):
```
1. Open BP_SpaceStation Class Defaults
2. Add module classes to DefaultModuleClasses array
3. Place station in level
4. Modules spawn automatically
```

**Benefits**:
- ✅ Faster setup (configure once, use many times)
- ✅ Consistent configuration across instances
- ✅ Easy to create station variants
- ✅ No manual attachment needed

---

## Technical Implementation Details

### Header Declaration

```cpp
// Source/Adastrea/Public/Stations/SpaceStation.h

/**
 * Default module classes to spawn when station is created
 */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station|Configuration")
TArray<TSubclassOf<ASpaceStationModule>> DefaultModuleClasses;

/**
 * Array of currently attached modules (runtime tracking)
 */
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station")
TArray<ASpaceStationModule*> Modules;
```

### BeginPlay Implementation

```cpp
// Source/Adastrea/Private/Stations/SpaceStation.cpp

void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn default modules configured in Class Defaults
    if (DefaultModuleClasses.Num() > 0)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = 
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            for (TSubclassOf<ASpaceStationModule> ModuleClass : DefaultModuleClasses)
            {
                if (ModuleClass)
                {
                    ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
                        ModuleClass, 
                        GetActorLocation(), 
                        GetActorRotation(),
                        SpawnParams
                    );
                    
                    if (NewModule)
                    {
                        AddModule(NewModule);
                    }
                }
            }
        }
    }
}
```

### Key Design Decisions

1. **TSubclassOf vs Actor Pointer**:
   - `TSubclassOf` allows class selection in editor
   - Actor pointers require existing instances
   - Solution: Separate arrays for different purposes

2. **BeginPlay vs Constructor**:
   - BeginPlay chosen because World is guaranteed to exist
   - Allows proper actor spawning
   - Constructor has no World context

3. **Automatic vs Manual**:
   - Automatic spawning reduces setup time
   - Manual placement still supported via AddModule()
   - Both methods update same runtime array

---

## Related Documentation

- **Class Reference**: `Source/Adastrea/Public/Stations/SpaceStation.h`
- **Module Base**: `Source/Adastrea/Public/Stations/SpaceStationModule.h`
- **Station Editor**: `Assets/StationEditorSystemGuide.md`
- **Module Types**: `Source/Adastrea/Public/Stations/StationModuleTypes.h`
- **Blueprint Workflow**: `Assets/BlueprintWorkflowTemplates.md`

---

## FAQ

**Q: Can I mix DefaultModuleClasses with manual placement?**  
A: Yes, but not recommended. Choose one method for consistency.

**Q: How do I prevent spawning duplicate modules?**  
A: Leave `DefaultModuleClasses` empty if placing modules manually.

**Q: Can I change DefaultModuleClasses at runtime?**  
A: Array is `BlueprintReadOnly`, so no. Use `AddModule()` for runtime changes.

**Q: Do modules inherit station's transform?**  
A: Yes, modules spawn at station location/rotation and are attached.

**Q: How do I add custom positioning?**  
A: Override BeginPlay in Blueprint and call `AddModuleAtLocation()` with offsets.

**Q: Can I create module variants?**  
A: Yes! Create multiple Blueprint classes based on same C++ parent:
- `BP_DockingBay_Small`
- `BP_DockingBay_Large`
- `BP_DockingBay_Capital`

**Q: Does this work with procedural generation?**  
A: Yes. Procedural code can set `DefaultModuleClasses` before spawning station.

---

## Support

If you encounter issues:

1. **Check Output Log** for spawn errors
2. **Verify module classes** exist and are valid
3. **Test with single module** first
4. **Search log for**: `LogAdastreaStations: SpaceStation::BeginPlay`
5. **Report issues** with log output and Blueprint configuration

---

**Last Updated**: January 10, 2026  
**Adastrea Version**: Alpha 1.0.0  
**Author**: GitHub Copilot Agent
