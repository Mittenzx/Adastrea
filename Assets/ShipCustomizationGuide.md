# Ship Customization System Guide

> **Complete guide to implementing and using the ship customization system in Adastrea**

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Core Classes](#core-classes)
- [Creating Modules](#creating-modules)
- [Hardpoint System](#hardpoint-system)
- [Customization UI](#customization-ui)
- [Blueprint Integration](#blueprint-integration)
- [Examples](#examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

---

## Overview

The ship customization system allows players to modify and upgrade their spaceships with various modules and equipment. The system provides:

- Modular hardpoint system
- Stat modification framework
- Power and weight management
- Visual customization
- Loadout presets
- Integration with inventory and trading

### System Architecture

```
UShipCustomizationDataAsset (Module Definitions)
         ↓
UShipCustomizationWidget (UI & Logic)
         ↓
ASpaceship (Installation & Application)
```

---

## Quick Start

### 1. Create a Module Data Asset

1. In Content Browser, right-click → **Miscellaneous > Data Asset**
2. Select **ShipCustomizationDataAsset**
3. Name it `DA_Module_PulseLaser`
4. Configure:
   - Customization Name: "Pulse Laser Mk.I"
   - Type: Weapon Module
   - Required Hardpoint: Small
   - Power Consumption: 50.0
   - Mass: 15.0
   - Base Value: 3000

### 2. Add Ship Customization UI

1. Create Widget Blueprint → **ShipCustomizationWidget**
2. Name it `WBP_ShipCustomization`
3. Design hardpoint layout
4. Add module selection panel
5. Create stats preview area

### 3. Initialize Customization

```
On Open Customization Menu
  → Create Widget (WBP_ShipCustomization)
  → Initialize Customization (Current Spaceship)
  → Add to Viewport
```

### 4. Install a Module

```
On Install Button Clicked
  → Get Selected Module
  → Get Selected Hardpoint
  → Install Module (Module, Hardpoint Index)
  → If Success:
    → Play Install Animation
    → Update Stats Display
  → Else:
    → Show Error Message
```

---

## Core Classes

### UShipCustomizationDataAsset

**Location**: `Source/Adastrea/UI/ShipCustomizationDataAsset.h`

Defines a module that can be installed on ships.

**Properties**:

```cpp
// Basic Information
FText CustomizationName         // Display name
FText Description              // Detailed description
UTexture2D* Icon              // UI icon

// Classification
EShipCustomizationType CustomizationType  // Type of module
EShipHardpointType RequiredHardpoint     // Required slot type

// Requirements
float PowerConsumption         // Power draw
float Mass                    // Module weight
int32 MinimumTechLevel        // Tech requirement
TArray<FString> RestrictedToShipClasses  // Ship class restrictions (empty = all ships)

// Economy
int32 BaseValue               // Value in credits

// Stats
TMap<FName, float> StatModifiers     // Stat modifications

// Visual
UStaticMesh* ModuleMesh              // 3D mesh
UMaterialInterface* MaterialOverride  // Custom material

// Advanced
TMap<FName, FString> CustomProperties  // Extensible data
```

**Functions**:

```cpp
// Get stat modifier value
float GetStatModifier(FName StatName, float DefaultValue) const;

// Get custom property
FString GetCustomProperty(FName PropertyName, const FString& DefaultValue) const;

// Check ship compatibility
bool IsCompatibleWithShip(const FString& ShipClassName) const;
```

### Customization Types

```cpp
enum class EShipCustomizationType
{
    WeaponModule,      // Weapons (lasers, missiles, etc.)
    EngineModule,      // Engine upgrades
    ShieldModule,      // Shield generators
    ArmorPlating,      // Hull armor
    CargoExpansion,    // Cargo capacity
    SensorArray,       // Sensors and scanners
    LifeSupport,       // Life support systems
    PowerCore,         // Power generation
    Cosmetic          // Visual customization
};
```

### Hardpoint Types

```cpp
enum class EShipHardpointType
{
    Small,     // Small weapon/utility slot
    Medium,    // Medium weapon slot
    Large,     // Large weapon slot
    Utility,   // Utility equipment slot
    Internal   // Internal system slot
};
```

---

## Creating Modules

### Weapon Module Template

```yaml
CustomizationName: "Pulse Laser Mk.I"
Description: "Standard energy weapon with good accuracy and moderate damage"
CustomizationType: WeaponModule
RequiredHardpoint: Small
PowerConsumption: 50.0
Mass: 15.0
BaseValue: 3000
MinimumTechLevel: 1

StatModifiers:
  - DPS: 100.0
  - Range: 2000.0
  - Accuracy: 0.85
  - FireRate: 5.0

CustomProperties:
  - DamageType: "Energy"
  - ProjectileSpeed: "1500"
  - HeatGeneration: "25"
```

### Engine Module Template

```yaml
CustomizationName: "Afterburner Module"
Description: "Provides short-duration speed boost at the cost of increased fuel consumption"
CustomizationType: EngineModule
RequiredHardpoint: Internal
PowerConsumption: 75.0
Mass: 50.0
BaseValue: 8000
MinimumTechLevel: 3

StatModifiers:
  - MaxSpeed: 200.0
  - Acceleration: 50.0
  - BoostSpeed: 500.0
  - FuelConsumption: -25.0

CustomProperties:
  - BoostDuration: "5.0"
  - BoostCooldown: "15.0"
```

### Shield Module Template

```yaml
CustomizationName: "Mk.II Shield Generator"
Description: "Advanced shield generator with improved capacity and recharge rate"
CustomizationType: ShieldModule
RequiredHardpoint: Internal
PowerConsumption: 100.0
Mass: 80.0
BaseValue: 12000
MinimumTechLevel: 4

StatModifiers:
  - ShieldCapacity: 500.0
  - ShieldRecharge: 50.0
  - ShieldDelay: -2.0

CustomProperties:
  - ShieldType: "Energy"
  - ResistanceBonus: "10"
```

### Cargo Module Template

```yaml
CustomizationName: "Expanded Cargo Bay"
Description: "Increases cargo capacity for trade and resource hauling"
CustomizationType: CargoExpansion
RequiredHardpoint: Internal
PowerConsumption: 10.0
Mass: 100.0
BaseValue: 5000
MinimumTechLevel: 2

StatModifiers:
  - CargoCapacity: 500.0
  - MaxWeight: 1000.0
```

### Cosmetic Module Template

```yaml
CustomizationName: "Racing Stripes"
Description: "Stylish paint job that makes your ship look faster"
CustomizationType: Cosmetic
RequiredHardpoint: Internal
PowerConsumption: 0.0
Mass: 0.0
BaseValue: 1000
MinimumTechLevel: 1

CustomProperties:
  - ColorPrimary: "FF0000"
  - ColorSecondary: "FFFFFF"
  - Pattern: "Stripes"
```

---

## Hardpoint System

### Defining Hardpoints

Hardpoints are defined per ship and determine what can be installed.

**Structure**:
```cpp
struct FShipHardpoint
{
    EShipHardpointType HardpointType;      // Slot type
    FText HardpointName;                   // Display name
    UShipCustomizationDataAsset* InstalledModule;  // Current module
};
```

### Hardpoint Configuration

**Fighter Ship Example**:
```
Hardpoints:
  - Small Weapon 1 (Small)
  - Small Weapon 2 (Small)
  - Utility Slot (Utility)
  - Engine Module (Internal)
  - Shield Generator (Internal)
```

**Heavy Gunship Example**:
```
Hardpoints:
  - Small Weapon 1 (Small)
  - Small Weapon 2 (Small)
  - Medium Weapon 1 (Medium)
  - Medium Weapon 2 (Medium)
  - Large Weapon (Large)
  - Utility Slot 1 (Utility)
  - Utility Slot 2 (Utility)
  - Engine Module (Internal)
  - Shield Generator (Internal)
  - Armor Plating (Internal)
```

**Cargo Hauler Example**:
```
Hardpoints:
  - Small Weapon (Small)
  - Utility Slot (Utility)
  - Engine Module (Internal)
  - Shield Generator (Internal)
  - Cargo Bay 1 (Internal)
  - Cargo Bay 2 (Internal)
  - Cargo Bay 3 (Internal)
```

---

## Customization UI

### UShipCustomizationWidget

**Location**: `Source/Adastrea/UI/ShipCustomizationWidget.h`

Manages ship customization interface.

**Key Properties**:

```cpp
ASpaceship* Spaceship;                 // Ship being customized
TArray<FShipHardpoint> Hardpoints;    // All hardpoint slots
float MaxPowerCapacity;               // Maximum power
bool bIsPreviewActive;                // Preview mode active
```

**Key Functions**:

#### Initialize Customization

```cpp
void InitializeCustomization(ASpaceship* InSpaceship);
```

Sets up customization for a specific ship.

#### Module Installation

```cpp
bool InstallModule(UShipCustomizationDataAsset* Module, int32 HardpointIndex);
bool RemoveModule(int32 HardpointIndex);
```

Install or remove modules from hardpoints.

#### Preview System

```cpp
void PreviewModuleStats(UShipCustomizationDataAsset* Module, int32 HardpointIndex);
void ClearStatsPreview();
```

Preview stat changes before committing.

#### Stat Queries

```cpp
TArray<FShipHardpoint> GetHardpoints() const;
TArray<FInstalledModule> GetInstalledModules() const;
float GetTotalPowerConsumption() const;
float GetTotalModuleMass() const;
bool CanInstallModule(UShipCustomizationDataAsset* Module, int32 HardpointIndex) const;
```

#### Apply Changes

```cpp
void ApplyCustomizations();    // Apply all changes to ship
void ResetCustomizations();    // Remove all modules
```

---

## Blueprint Integration

### Opening Customization Menu

```
In Space Station Blueprint:
  
  On Player Interact (Customization Terminal)
    → Get Player Spaceship
    → Create Customization Widget
    → Initialize Customization (Player Ship)
    → Add to Viewport
    → Set Input Mode (UI Only)
    → Show Mouse Cursor
```

### Module Selection

```
In Customization Widget:
  
  On Module Button Clicked (Module)
    → Set Selected Module = Module
    → Get Hovered Hardpoint
    → Preview Module Stats (Module, Hardpoint)
    → Highlight Compatible Hardpoints
```

### Installation Process

```
On Install Confirmed
  → Check Can Install Module
  → If Compatible:
    → If Player Has Module in Inventory:
      → Remove from Inventory
      → Install Module
      → Play Success Animation
      → Update Stats Display
      → Deduct Installation Cost
    → Else:
      → Show "Module Not in Inventory"
  → Else:
    → Show Incompatibility Reason:
      - Wrong hardpoint type
      - Insufficient power
      - Tech level too low
      - Faction restricted
```

### Power Management

```
Display Power Usage:
  → Total Used = Get Total Power Consumption
  → Total Available = Max Power Capacity
  → Power Bar.Percent = Total Used / Total Available
  → If Total Used > Total Available:
    → Power Bar.Color = Red
    → Disable Apply Button
  → Else:
    → Power Bar.Color = Green
```

### Stat Preview

```
On Module Hovered
  → Clear Stats Preview
  → Calculate New Stats:
    → Current Stats = Get Ship Stats
    → Preview Stats = Current + Module.StatModifiers
  → Display Comparison:
    → For Each Stat:
      → Show: "Stat Name: Current → Preview"
      → Color: Green if improved, Red if worse
```

---

## Examples

### Example 1: Basic Loadout Editor

**Features**:
- View all hardpoints
- Install/remove modules
- See power usage
- Save/load loadouts

**UI Layout**:
```
Left Panel: Hardpoint List
  - Hardpoint Name
  - Current Module
  - Install/Remove Buttons

Center: Ship Preview (3D or 2D)
  - Visual representation
  - Hardpoint markers

Right Panel: Module Selection
  - Available modules
  - Filter by type
  - Sort by stats

Bottom: Stats Bar
  - Power usage
  - Weight
  - Total stats
```

### Example 2: Comparison Tool

**Features**:
- Compare current vs. new module
- Side-by-side stat comparison
- Trade-off analysis

**Implementation**:
```
Comparison Panel:
  
  Current Module     vs     New Module
  ----------------         ---------------
  Name: Laser Mk.I         Name: Laser Mk.II
  DPS: 100                 DPS: 150 (+50)
  Range: 2000              Range: 1800 (-200)
  Power: 50                Power: 75 (+25)
  
  Overall: +30% DPS, -10% Range, +50% Power
```

### Example 3: Preset System

**Features**:
- Save loadout presets
- Quick loadout switching
- Preset sharing

**Implementation**:
```
Preset Management:
  
  Create Preset:
    → Get All Installed Modules
    → Save to Preset Data Asset
    → Name Preset
  
  Load Preset:
    → Check Has All Modules
    → Remove Current Modules
    → Install Preset Modules
    → Apply Changes
  
  Share Preset:
    → Export Preset to JSON
    → Generate Share Code
```

### Example 4: Visual Customization

**Features**:
- Paint colors
- Decals
- Visual effects
- Ship lights

**Implementation**:
```
Visual Customization:
  
  Color Picker:
    → Select Primary Color
    → Select Secondary Color
    → Select Accent Color
    → Preview on Ship Model
  
  Decal System:
    → Select Decal Type
    → Position on Ship
    → Scale and Rotate
    → Apply to Material
```

---

## Best Practices

### Module Design

1. **Balanced Trade-offs**:
   - Powerful modules = higher power/weight
   - Specialized modules = restricted use
   - Universal modules = moderate stats

2. **Meaningful Choices**:
   - Each module should have a purpose
   - Avoid strictly better options
   - Create build diversity

3. **Tier Progression**:
   - Mk.I: Basic, accessible
   - Mk.II: Improved, mid-game
   - Mk.III: Advanced, late-game
   - Unique: Special, quest rewards

### UI/UX

1. **Clear Information**:
   - Show stat changes clearly
   - Indicate incompatibilities
   - Display requirements upfront

2. **Intuitive Layout**:
   - Group related hardpoints
   - Color-code hardpoint types
   - Use consistent iconography

3. **Quick Actions**:
   - Drag-and-drop installation
   - Quick swap modules
   - One-click presets

### Performance

1. **Efficient Updates**:
   ```
   Only recalculate stats when modules change
   Cache computed values
   Batch visual updates
   ```

2. **Lazy Loading**:
   ```
   Load module meshes on demand
   Unload unused assets
   Use LODs for previews
   ```

---

## Troubleshooting

### Module Won't Install

**Symptom**: Can't install module in hardpoint

**Solutions**:
1. Check hardpoint type:
   ```
   Module.RequiredHardpoint must match Hardpoint.HardpointType
   ```

2. Check power capacity:
   ```
   Total power consumption <= Max power capacity
   ```

3. Check tech level:
   ```
   Player tech level >= Module.MinimumTechLevel
   ```

4. Check ship class restrictions:
   ```
   If RestrictedToShipClasses not empty, must match
   Note: Uses internal UE class names (e.g., 'BP_Ship_Fighter_C')
   ```

### Stats Not Updating

**Symptom**: Ship stats don't reflect installed modules

**Solutions**:
1. Call ApplyCustomizations:
   ```
   After installing modules, call ApplyCustomizations()
   ```

2. Verify stat modifiers:
   ```
   Check StatModifiers map has correct keys
   Ensure values are being applied to ship
   ```

3. Refresh ship systems:
   ```
   May need to reinitialize ship components
   ```

### Power Overload

**Symptom**: Total power exceeds capacity

**Solutions**:
1. Display clear warning:
   ```
   "Power Overload! 1200/1000 W"
   Disable Apply button
   ```

2. Suggest solutions:
   ```
   - Remove power-hungry modules
   - Install better power core
   - Use lower-tier modules
   ```

---

## YAML Import Templates

See `Assets/ShipCustomizationTemplates/` for examples:

- `WeaponModules.yaml` - Weapon definitions
- `EngineModules.yaml` - Engine upgrades
- `DefensiveModules.yaml` - Shields and armor
- `UtilityModules.yaml` - Sensors, cargo, etc.
- `CosmeticModules.yaml` - Visual customization

---

## Additional Resources

- **Spaceship System**: See `Assets/SpaceshipSetupGuide.md`
- **Inventory System**: See `Assets/InventorySystemGuide.md`
- **Trading System**: See `Assets/TradingSystemGuide.md`
- **Combat System**: See `Assets/CombatSystemGuide.md`

---

**Last Updated**: November 11, 2025  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
