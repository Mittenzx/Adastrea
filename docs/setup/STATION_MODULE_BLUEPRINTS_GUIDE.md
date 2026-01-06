# Station Module Blueprints Creation Guide

**Purpose**: Step-by-step guide for creating all BP_SpaceStationModule_* blueprints in Unreal Editor

**Last Updated**: 2026-01-06  
**MVP Status**: Critical for Trade Simulator MVP (Docking, Marketplace, Cargo)

---

## Overview

This guide shows how to create Blueprint assets for all station module types. Each module inherits from a C++ parent class that provides the core functionality, and the Blueprint configures visual appearance and properties.

**Important**: Blueprint assets (.uasset files) can only be created in Unreal Engine Editor, not through code or scripts.

---

## Prerequisites

- Unreal Engine 5.6 installed
- Adastrea project opened in Unreal Editor
- Basic familiarity with Blueprint creation
- C++ code compiled successfully

---

## Module Types Reference

All modules inherit from `ASpaceStationModule` and have these base properties:

| Property | Type | Description |
|----------|------|-------------|
| `ModuleType` | String | Display name (auto-set by C++ parent) |
| `ModulePower` | Float | Power consumption (+) or generation (-) |
| `ModuleGroup` | Enum | Category for organization |
| `ModuleFaction` | Asset Reference | Faction owner (optional) |

### Module Groups (EStationModuleGroup)

- **Docking**: Ship docking facilities (MVP Critical)
- **Storage**: Cargo and material storage (MVP Critical)
- **Public**: Markets, trading areas (MVP Critical)
- **Habitation**: Living quarters (MVP Critical)
- **Connection**: Corridors, passageways (MVP Critical)
- **Power**: Reactors, generators (POST-MVP)
- **Processing**: Manufacturing, refinement (POST-MVP)
- **Defence**: Weapons, shields (POST-MVP: Combat system)

---

## Complete Module Reference

### MVP-Critical Modules

These modules are essential for the Trade Simulator MVP:

#### 1. Docking Bay Module
- **C++ Parent Class**: `ADockingBayModule`
- **Blueprint Name**: `BP_SpaceStationModule_DockingBay`
- **Module Type**: "Docking Bay"
- **Power Consumption**: 50 units
- **Module Group**: Docking
- **Purpose**: Large docking facility for ships and freighters
- **Description**: Provides docking facilities with hangar doors, tractor beams, and docking assistance systems
- **MVP Usage**: Primary docking point for player ship trading

#### 2. Docking Port Module
- **C++ Parent Class**: `ADockingPortModule`
- **Blueprint Name**: `BP_SpaceStationModule_DockingPort`
- **Module Type**: "Docking Port"
- **Power Consumption**: 10 units
- **Module Group**: Docking
- **Purpose**: Compact docking point for smaller vessels
- **Description**: Smaller, more efficient docking point for shuttles and light ships
- **MVP Usage**: Alternative docking for smaller player ships

#### 3. Marketplace Module
- **C++ Parent Class**: `AMarketplaceModule`
- **Blueprint Name**: `BP_SpaceStationModule_Marketplace`
- **Module Type**: "Marketplace"
- **Power Consumption**: 40 units
- **Module Group**: Public
- **Purpose**: Trade marketplace for buying/selling goods
- **Description**: Commercial facility with vendor stalls, trading terminals, and auction systems
- **MVP Usage**: Core trading interface location

#### 4. Cargo Bay Module
- **C++ Parent Class**: `ACargoBayModule`
- **Blueprint Name**: `BP_CargoBayModule` *(Already exists)*
- **Module Type**: "Cargo Bay"
- **Power Consumption**: 5 units
- **Module Group**: Storage
- **Purpose**: Storage for goods and materials
- **Description**: Warehouse space for cargo storage
- **MVP Usage**: Station inventory for traded goods

#### 5. Habitation Module
- **C++ Parent Class**: `AHabitationModule`
- **Blueprint Name**: `BP_SpaceStationModule_Habitation`
- **Module Type**: "Habitation"
- **Power Consumption**: 30 units
- **Module Group**: Habitation
- **Purpose**: Living quarters for station inhabitants
- **Description**: Residential facility with life support, private quarters, and communal areas
- **MVP Usage**: Basic station population/functionality

#### 6. Corridor Module
- **C++ Parent Class**: `ACorridorModule`
- **Blueprint Name**: `BP_SpaceStationModule_Corridor`
- **Module Type**: "Corridor"
- **Power Consumption**: 2 units
- **Module Group**: Connection
- **Purpose**: Passageway connecting station sections
- **Description**: Connecting passage with basic lighting and environmental systems
- **MVP Usage**: Station navigation between modules

---

### POST-MVP Modules

These modules are for future expansion after MVP validation:

#### 7. Reactor Module
- **C++ Parent Class**: `AReactorModule`
- **Blueprint Name**: `BP_SpaceStationModule_Reactor`
- **Module Type**: "Reactor"
- **Power Generation**: -500 units (negative = generates)
- **Module Group**: Power
- **Purpose**: Primary power generation
- **POST-MVP**: Advanced station management

#### 8. Solar Array Module
- **C++ Parent Class**: `ASolarArrayModule`
- **Blueprint Name**: `BP_SpaceStationModule_SolarArray`
- **Module Type**: "Solar Array"
- **Power Generation**: -100 units
- **Module Group**: Power
- **Purpose**: Solar power generation
- **POST-MVP**: Advanced station management

#### 9. Fuel Depot Module
- **C++ Parent Class**: `AFuelDepotModule`
- **Blueprint Name**: `BP_SpaceStationModule_FuelDepot`
- **Module Type**: "Fuel Depot"
- **Power Consumption**: 15 units
- **Module Group**: Storage
- **Purpose**: Fuel storage and refueling services
- **POST-MVP**: Advanced trading/services

#### 10. Barracks Module
- **C++ Parent Class**: `ABarracksModule`
- **Blueprint Name**: `BP_SpaceStationModule_Barracks`
- **Module Type**: "Barracks"
- **Power Consumption**: 20 units
- **Module Group**: Habitation
- **Purpose**: Military living quarters
- **POST-MVP**: Crew/personnel system

#### 11. Fabrication Module
- **C++ Parent Class**: `AFabricationModule`
- **Blueprint Name**: `BP_SpaceStationModule_Fabrication`
- **Module Type**: "Fabrication"
- **Power Consumption**: 150 units
- **Module Group**: Processing
- **Purpose**: Manufacturing and assembly
- **POST-MVP**: Crafting system

#### 12. Processing Module
- **C++ Parent Class**: `AProcessingModule`
- **Blueprint Name**: `BP_SpaceStationModule_Processing`
- **Module Type**: "Processing"
- **Power Consumption**: 100 units
- **Module Group**: Processing
- **Purpose**: Material refinement and processing
- **POST-MVP**: Resource processing

#### 13. Science Lab Module
- **C++ Parent Class**: `AScienceLabModule`
- **Blueprint Name**: `BP_SpaceStationModule_ScienceLab`
- **Module Type**: "Science Lab"
- **Power Consumption**: 75 units
- **Module Group**: Processing
- **Purpose**: Research and development
- **POST-MVP**: Research system

#### 14. Shield Generator Module
- **C++ Parent Class**: `AShieldGeneratorModule`
- **Blueprint Name**: `BP_SpaceStationModule_ShieldGenerator`
- **Module Type**: "Shield Generator"
- **Power Consumption**: 200 units
- **Module Group**: Defence
- **Purpose**: Station defensive shields
- **POST-MVP**: Combat system

#### 15. Turret Module
- **C++ Parent Class**: `ATurretModule`
- **Blueprint Name**: `BP_SpaceStationModule_Turret`
- **Module Type**: "Turret"
- **Power Consumption**: 25 units
- **Module Group**: Defence
- **Purpose**: Station defensive weapons
- **POST-MVP**: Combat system

---

## Step-by-Step Blueprint Creation

### General Process (For All Modules)

Follow these steps for each module type:

#### Step 1: Navigate to Module Folder

1. Open Unreal Engine Editor with Adastrea project
2. In Content Browser, navigate to: `Content/Blueprints/Stations/Modules/`
3. If folder doesn't exist, create it:
   - Right-click on `Stations` folder
   - Select "New Folder"
   - Name it "Modules"

#### Step 2: Create New Blueprint

1. Right-click in the `Modules` folder
2. Select "Blueprint Class"
3. In the "Pick Parent Class" dialog:
   - Click "All Classes" to expand
   - Search for the C++ parent class name (e.g., `DockingBayModule`)
   - Select the class
   - Click "Select"

#### Step 3: Name the Blueprint

1. Name the blueprint following the convention:
   - Format: `BP_SpaceStationModule_[ModuleName]`
   - Examples:
     - `BP_SpaceStationModule_DockingBay`
     - `BP_SpaceStationModule_Marketplace`
   - Exception: `BP_CargoBayModule` (existing module, created before naming convention established)

#### Step 4: Configure Blueprint

1. Double-click the new blueprint to open it
2. The C++ parent automatically sets:
   - `ModuleType` (display name)
   - `ModulePower` (power consumption/generation)
   - `ModuleGroup` (category)
3. Add visual components:
   - **Mesh**: Select `MeshComponent` in Components panel
   - In Details panel, set `Static Mesh` to your module mesh
   - Adjust transform (location, rotation, scale) as needed

#### Step 5: Add Visual Polish (Optional)

1. Add lights, particle effects, materials
2. Configure collision if needed
3. Add sound effects (docking clamps, machinery)
4. Add any module-specific visual indicators

#### Step 6: Save and Test

1. Click "Compile" button (top toolbar)
2. Click "Save" button
3. Test by:
   - Dragging into a level
   - Checking properties display correctly
   - Verifying mesh appears

---

## Quick Creation Checklist

Use this checklist to create all MVP-critical modules:

### MVP Modules (Create These First)

- [ ] **BP_SpaceStationModule_DockingBay**
  - Parent: `ADockingBayModule`
  - Add large hangar bay mesh
  - Add docking lights/indicators
  - Power: 50 units

- [ ] **BP_SpaceStationModule_DockingPort**
  - Parent: `ADockingPortModule`
  - Add small docking port mesh
  - Add docking clamp visuals
  - Power: 10 units

- [ ] **BP_SpaceStationModule_Marketplace**
  - Parent: `AMarketplaceModule`
  - Add marketplace interior mesh
  - Add vendor stall visuals
  - Power: 40 units

- [ ] **BP_SpaceStationModule_Habitation**
  - Parent: `AHabitationModule`
  - Add residential section mesh
  - Add window/lighting
  - Power: 30 units

- [ ] **BP_SpaceStationModule_Corridor**
  - Parent: `ACorridorModule`
  - Add corridor tube mesh
  - Add lighting strips
  - Power: 2 units

### Verify Existing Module

- [ ] **BP_CargoBayModule** *(Already exists)*
  - Verify parent is `ACargoBayModule`
  - Check mesh is set
  - Confirm power: 5 units

---

## Common Issues and Solutions

### Issue: Can't Find Parent Class

**Problem**: C++ class doesn't appear in "Pick Parent Class" dialog

**Solutions**:
1. Ensure C++ code is compiled (`Build` → `Build Solution` in IDE)
2. Close and reopen Unreal Editor after compilation
3. Check class is marked `Blueprintable` in C++ header
4. Verify module is included in `.Build.cs` file

### Issue: Properties Not Showing

**Problem**: Module properties (Power, Type, Group) don't appear

**Solution**: These are set automatically by C++ parent constructor. Check:
1. Blueprint parent class is correct
2. C++ constructor sets values correctly
3. Recompile C++ if you made changes

### Issue: Mesh Not Appearing

**Problem**: Module has no visual representation in level

**Solutions**:
1. Check `MeshComponent` has a `Static Mesh` assigned
2. Verify mesh asset exists in Content Browser
3. Check mesh scale isn't too small
4. Ensure mesh isn't hidden in Component details

### Issue: Module Power Incorrect

**Problem**: Power consumption shows wrong value

**Solution**: Power is set in C++ constructor and cannot be changed in Blueprint. If value is wrong:
1. Fix in C++ parent class constructor
2. Recompile C++ code
3. Re-open Blueprint (may need to recreate)

---

## Testing Your Modules

### Basic Functionality Test

1. **Place in Level**:
   - Drag module blueprint into level
   - Verify it appears at correct location
   - Check mesh renders correctly

2. **Check Properties**:
   - Select module in level
   - In Details panel, verify:
     - `ModuleType` shows correct name
     - `ModulePower` shows correct value
     - `ModuleGroup` shows correct category

3. **Attach to Station** (if BP_SpaceStation exists):
   - Place a `BP_SpaceStation` in level
   - Test attaching modules
   - Verify power consumption updates

### Integration Test (When Trading System Ready)

1. **Docking Test**:
   - Approach station with player ship
   - Trigger docking sequence
   - Verify docking UI appears

2. **Trading Test**:
   - Dock at station with Marketplace module
   - Open trading interface
   - Verify market prices display

3. **Cargo Test**:
   - Complete a trade
   - Verify station Cargo Bay updates inventory

---

## Blueprint Naming Conventions

**Format**: `BP_SpaceStationModule_[ModuleName]`

**Correct Examples**:
- ✅ `BP_SpaceStationModule_DockingBay`
- ✅ `BP_SpaceStationModule_Marketplace`
- ✅ `BP_SpaceStationModule_Habitation`

**Exception**:
- ⚠️ `BP_CargoBayModule` (existing module, created before naming convention was established)

**Incorrect Examples**:
- ❌ `DockingBayModule` (missing prefix)
- ❌ `BP_DockingBay` (not specific enough)
- ❌ `BP_Module_DockingBay` (wrong format)
- ❌ `BP_Station_DockingBay` (unclear type)

---

## File Organization

### Recommended Structure

```
Content/
└── Blueprints/
    └── Stations/
        ├── BP_SpaceStation.uasset (main station)
        ├── BP_StationModule.uasset (generic base)
        └── Modules/
            ├── BP_SpaceStationModule_DockingBay.uasset
            ├── BP_SpaceStationModule_DockingPort.uasset
            ├── BP_SpaceStationModule_Marketplace.uasset
            ├── BP_CargoBayModule.uasset (existing, pre-convention)
            ├── BP_SpaceStationModule_Habitation.uasset
            ├── BP_SpaceStationModule_Corridor.uasset
            ├── BP_SpaceStationModule_Reactor.uasset
            ├── BP_SpaceStationModule_SolarArray.uasset
            └── [... all other modules ...]
```

---

## Visual Mesh Recommendations

Since you're creating the Blueprint assets without actual 3D models yet, here's what to use:

### Placeholder Meshes (For Testing)

Use Unreal Engine starter content or basic shapes:

- **Docking Bay**: Large cube or hangar shape
- **Docking Port**: Small cylinder or ring
- **Marketplace**: Medium cube with open front
- **Cargo Bay**: Large box/container shape
- **Habitation**: Cylinder or tube shape
- **Corridor**: Long, thin box or tube
- **Reactor**: Sphere with glow
- **Solar Array**: Flat rectangular panel

### Material Suggestions

Use different materials to distinguish module types:

- **Docking**: Blue/cyan lights (active systems)
- **Storage**: Gray/industrial (utilitarian)
- **Public**: Warm colors (welcoming)
- **Habitation**: Soft lighting (comfortable)
- **Power**: Yellow/orange (energy)
- **Defence**: Red (warning)

---

## Advanced Configuration

### Adding Module-Specific Properties

If a module needs custom properties beyond the base class:

1. **Option A**: Add in Blueprint
   - Add variables in Blueprint editor
   - Set defaults in Class Defaults
   - Expose to level designers

2. **Option B**: Extend in C++ (Recommended for complex logic)
   - Add properties to C++ parent class
   - Mark with `UPROPERTY(EditAnywhere, BlueprintReadOnly)`
   - Recompile and Blueprint automatically gets new properties

### Example: Adding Docking Capacity

If `BP_SpaceStationModule_DockingBay` needs to track ship capacity:

**C++ Approach** (in `DockingBayModule.h`):
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking")
int32 MaxDockedShips = 4;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
int32 CurrentDockedShips = 0;
```

**Blueprint Approach**:
1. Open `BP_SpaceStationModule_DockingBay`
2. Add variable `MaxDockedShips` (Integer)
3. Set default value to 4
4. Add variable `CurrentDockedShips` (Integer)
5. Set default value to 0

---

## Integration with Trading System

### Docking Interface

When player approaches station with Docking Bay or Docking Port:

1. **Detection**: Station detects nearby ship
2. **Prompt**: UI shows "Press [F] to Dock"
3. **Docking**: Triggers docking sequence
4. **Trading UI**: Opens marketplace interface

### Marketplace Interface

When docked at station with Marketplace module:

1. **Access**: Player can access trading UI
2. **Inventory**: Shows station's Cargo Bay contents
3. **Prices**: Based on station market data
4. **Transactions**: Updates both player and station cargo

### Cargo Management

Cargo Bay module provides storage:

1. **Capacity**: Defines max storage volume
2. **Contents**: Tracks stored goods
3. **Trading**: Source/destination for transactions

---

## Next Steps

After creating all blueprints:

1. **Create BP_SpaceStation** (if not exists)
   - Combine modules into complete station
   - Configure module attachment points
   - Set up station-level properties

2. **Add to Levels**
   - Place stations in trading routes
   - Configure different station types
   - Test docking from various angles

3. **Implement Docking System**
   - Create docking detection
   - Build docking UI
   - Connect to trading interface

4. **Test Trade Flow**
   - Full loop: Approach → Dock → Trade → Undock
   - Verify all systems integrate
   - Polish interactions

---

## Related Documentation

- **C++ Reference**: `Source/Adastrea/Public/Stations/SpaceStationModule.h`
- **Module Types**: `Source/Adastrea/Public/Stations/StationModuleTypes.h`
- **Station System**: `ARCHITECTURE.md` (Stations section)
- **Trading System**: `.github/instructions/trade-simulator-mvp.instructions.md`
- **Blueprint Standards**: `.github/instructions/blueprint-documentation.instructions.md`

---

## Support

If you encounter issues:

1. Check "Common Issues" section above
2. Verify C++ code compiled successfully
3. Review parent class documentation in header files
4. Check console log for errors
5. Open GitHub issue with tag `blueprint` or `stations`

---

**Remember**: Start with MVP-critical modules (Docking, Marketplace, Habitation) before creating POST-MVP modules. Focus on functionality over visual polish initially.

---

**Last Updated**: 2026-01-06  
**Created by**: GitHub Copilot Agent  
**Status**: Ready for implementation
