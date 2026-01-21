# Modular Ship System Documentation

## Overview

The Modular Ship System allows ships in Adastrea to have swappable components including engines, weapons, shields, and other modules. This system provides:

- **Designer-Friendly**: All module types configured via Data Assets
- **Flexible**: Supports any module category through extensible interface
- **Complete**: Includes connection points, power/mass tracking, and UI framework
- **Blueprint-Ready**: Full Blueprint integration for customization

## Architecture

### Core Components

1. **IShipModule Interface** (`IShipModule.h`)
   - Defines the contract for all ship modules
   - Provides default implementations
   - Handles installation, removal, and compatibility checks

2. **FShipModuleSlot Struct** (`IShipModule.h`)
   - Represents a connection point on a ship
   - Stores position, rotation, and compatibility requirements
   - Tracks occupancy status

3. **UShipModuleComponent** (`ShipModuleComponent.h`)
   - Base component class implementing IShipModule
   - Manages visual mesh attachment
   - Handles enable/disable state

4. **UShipModuleDataAsset** (`ShipModuleDataAsset.h`)
   - Base Data Asset for module configuration
   - Stores common properties (mass, power, cost, etc.)
   - Extensible for specific module types

5. **UShipCustomizationComponent** (`ShipCustomizationComponent.h`)
   - Central management component for all modules
   - Handles installation/removal
   - Tracks power and mass usage
   - Validates module compatibility

### Module Types

#### Engine Modules
- **Data Asset**: `UEngineModuleDataAsset`
- **Component**: `UEngineModuleComponent`
- **Features**:
  - Thrust and speed contribution
  - Heat management
  - Fuel consumption tracking
  - Boost and travel mode support
  - Visual and audio effects

#### Shield Modules
- **Data Asset**: `UShieldModuleDataAsset`
- **Component**: *(To be implemented)*
- **Features**:
  - Damage resistance by type
  - Adaptive shields
  - Shield boosting
  - Phase shift capability

#### Weapon Modules
- **Existing**: `UWeaponComponent` with `UWeaponDataAsset`
- **Integration**: Can be adapted to implement `IShipModule`

### UI Framework

**UModularShipCustomizationWidget** provides:
- Slot visualization
- Module selection
- Installation/removal operations
- Power and mass tracking display
- Blueprint-extensible events

## Usage Guide

### Creating a New Module Type

1. **Create Data Asset Class**:
```cpp
UCLASS(BlueprintType)
class UMyModuleDataAsset : public UShipModuleDataAsset
{
    GENERATED_BODY()
    
    // Add module-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="My Module")
    float SpecialValue;
};
```

2. **Create Component Class**:
```cpp
UCLASS()
class UMyModuleComponent : public UShipModuleComponent
{
    GENERATED_BODY()
    
    // Override IShipModule methods as needed
    virtual EShipModuleCategory GetModuleCategory_Implementation() const override
    {
        return EShipModuleCategory::Special;
    }
};
```

3. **Create Blueprint Data Assets**:
   - Right-click in Content Browser
   - Create Blueprint Data Asset based on `UMyModuleDataAsset`
   - Configure properties

### Adding Modules to a Ship

1. **Add Customization Component** to ship:
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UShipCustomizationComponent* CustomizationComponent;
```

2. **Define Module Slots** in constructor or Blueprint:
```cpp
FShipModuleSlot EngineSlot;
EngineSlot.SlotID = FName("MainEngine");
EngineSlot.SlotName = FText::FromString("Main Engine");
EngineSlot.AcceptedCategory = EShipModuleCategory::Engine;
EngineSlot.MaxSize = EShipModuleSize::Large;
EngineSlot.SlotLocation = FVector(-500, 0, 0);
CustomizationComponent->ModuleSlots.Add(EngineSlot);
```

3. **Install Modules** via code or UI:
```cpp
// Create module component
UEngineModuleComponent* Engine = NewObject<UEngineModuleComponent>(Ship);
Engine->InitializeWithData(EngineDataAsset);

// Install module
CustomizationComponent->InstallModule(Engine, FName("MainEngine"));
```

### Creating a Customization UI

1. **Create Blueprint Widget** based on `UModularShipCustomizationWidget`

2. **Call Initialize** with ship's customization component:
```cpp
CustomizationWidget->Initialize(Ship->CustomizationComponent);
```

3. **Implement Blueprint Events**:
   - `OnSlotSelected` - Highlight selected slot
   - `OnModuleSelected` - Show module details
   - `OnModuleInstalled` - Success feedback
   - `OnInstallationFailed` - Error message

4. **Use Helper Functions**:
```cpp
// Get power usage for progress bar
FText PowerText = GetPowerUsageText();

// Check if module can be installed
FText Reason;
bool bCanInstall = CanInstallSelectedModule(Reason);
```

## Module Categories

- **Engine**: Propulsion systems (thrust, speed, acceleration)
- **Weapon**: Weapon systems (already implemented)
- **Shield**: Shield generators (protection, regeneration)
- **Utility**: Sensors, scanners, mining equipment
- **Hull**: Armor plating, structural modifications
- **PowerPlant**: Power generation systems
- **Storage**: Cargo holds, ammunition storage
- **Special**: Unique equipment (cloaking, jump drives, etc.)

## Module Sizes

- **Small**: Light equipment, fighters
- **Medium**: Standard equipment, most ships
- **Large**: Heavy equipment, larger vessels
- **Capital**: Massive equipment, capital ships only

## Power and Mass System

Each module has:
- **Power Requirement**: How much power it consumes (or generates if negative)
- **Mass**: Weight in kilograms

Ships have:
- **Max Power Capacity**: Total available power
- **Max Mass Capacity**: Maximum weight ship can carry

Modules cannot be installed if they would exceed either capacity.

## Integration with Existing Systems

### Ship Movement
Engine modules contribute to ship's:
- Maximum speed
- Acceleration rate
- Turn rate
- Boost capabilities

Total performance = sum of all installed engines.

### Combat System
Weapon modules already exist and can be adapted to:
- Implement `IShipModule` interface
- Attach to weapon slots
- Use modular data assets

### Visual System
- Modules can specify `ModuleMesh` for visual representation
- Meshes auto-attach to slot positions
- Support for custom materials

## Best Practices

1. **Always validate before installation**:
```cpp
FText Reason;
if (!CanInstallModule(Module, SlotID, Reason))
{
    // Handle error
}
```

2. **Use Data Assets for configuration**:
   - Never hardcode module stats
   - Create variants through Blueprint Data Assets
   - Easy for designers to balance

3. **Provide feedback to players**:
   - Visual confirmation of installation
   - Clear error messages
   - Power/mass warnings

4. **Test performance**:
   - Profile with many modules
   - Consider caching calculations
   - Use object pooling for frequently swapped modules

## Example Scenarios

### Fighter Ship
```
Slots:
- 2x Small Weapon Slots
- 1x Medium Engine Slot
- 1x Small Shield Slot
- 1x Small Utility Slot

Power: 500
Mass: 5,000 kg
```

### Freighter Ship
```
Slots:
- 1x Small Weapon Slot
- 2x Large Engine Slots
- 1x Medium Shield Slot
- 3x Large Storage Slots
- 2x Medium Utility Slots

Power: 2,000
Mass: 50,000 kg
```

### Capital Ship
```
Slots:
- 4x Large Weapon Slots
- 8x Medium Weapon Slots
- 3x Capital Engine Slots
- 2x Large Shield Slots
- 2x Large PowerPlant Slots
- 4x Medium Utility Slots

Power: 10,000
Mass: 500,000 kg
```

## Future Enhancements

- Save/load loadout system
- Module damage and repair
- Module upgrade paths
- Module crafting/modification
- Visual customization (colors, decals)
- Performance tuning interface
- Module sets/bonuses

## Troubleshooting

**Module won't install:**
- Check category compatibility
- Verify size fits slot
- Confirm sufficient power/mass
- Check module is not already installed

**Visual mesh not appearing:**
- Verify `ModuleMesh` is set in Data Asset
- Check slot position/rotation
- Ensure module is enabled

**Performance issues:**
- Reduce number of module slots
- Cache expensive calculations
- Disable visual effects on distant ships
- Use LOD for module meshes

## Related Files

- `Source/Adastrea/Public/Interfaces/IShipModule.h`
- `Source/Adastrea/Public/Ships/ShipModuleComponent.h`
- `Source/Adastrea/Public/Ships/ShipModuleDataAsset.h`
- `Source/Adastrea/Public/Ships/ShipCustomizationComponent.h`
- `Source/Adastrea/Public/Ships/EngineModuleComponent.h`
- `Source/Adastrea/Public/Ships/EngineModuleDataAsset.h`
- `Source/Adastrea/Public/Ships/ShieldModuleDataAsset.h`
- `Source/Adastrea/Public/UI/ModularShipCustomizationWidget.h`
