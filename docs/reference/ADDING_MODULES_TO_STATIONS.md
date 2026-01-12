# Adding Modules to Space Stations in Unreal Editor

**Quick Guide**: How to add modules to BP_SpaceStation and leverage aggregate functionality

---

## Overview

Space stations in Adastrea use a **modular architecture** where individual modules (docking bays, marketplaces, cargo storage, etc.) are attached to a core station actor. This guide explains:

1. How to add modules to a station in the editor
2. How modules contribute functionality to the parent station
3. How to query aggregate station capabilities from modules

---

## Understanding the Architecture

### Current Design

**Modules are separate actors** that attach to the station:
- Each module is an `ASpaceStationModule` (or subclass) actor
- Modules are **attached as children** to the `ASpaceStation` actor
- The station tracks all modules in a `Modules` array
- Modules contribute their functionality to the parent station

### Why This Approach?

✅ **Flexible**: Add/remove modules dynamically  
✅ **Modular**: Each module is self-contained  
✅ **Visual**: Modules are physically placed in 3D space  
✅ **Designer-Friendly**: Configure modules in Blueprints, no C++ needed

---

## Method 1: Add Modules in Editor (Recommended for MVP)

### Step-by-Step Instructions

#### 1. Open Your Station Blueprint

1. Navigate to `Content/Blueprints/Stations/`
2. Open `BP_SpaceStation` (or your custom station Blueprint)
3. Open in **Blueprint Editor**

#### 2. Configure Default Modules

1. In the Blueprint Editor, click **Class Defaults** button (top toolbar)
2. In the **Details** panel, find **Station → Configuration**
3. Locate the **Default Module Classes** array

![Class Defaults Location](../images/station-class-defaults.png)

#### 3. Add Module Classes to Array

1. Click the **+** button next to "Default Module Classes"
2. For each entry, select a module Blueprint class:
   - `BP_SpaceStationModule_DockingBay`
   - `BP_SpaceStationModule_Marketplace`
   - `BP_CargoBayModule`
   - `BP_SpaceStationModule_Habitation`
   - `BP_SpaceStationModule_Corridor`

**Example Configuration:**
```
Default Module Classes:
  [0] BP_SpaceStationModule_DockingBay
  [1] BP_SpaceStationModule_Marketplace
  [2] BP_CargoBayModule
  [3] BP_SpaceStationModule_Habitation
```

#### 4. Compile and Save

1. Click **Compile** button
2. Click **Save** button
3. Close Blueprint Editor

#### 5. Test in Level

1. Drag `BP_SpaceStation` into your level
2. Press **Play** or **Simulate**
3. Modules will automatically spawn and attach when the station begins play

### What Happens at Runtime

When the station's `BeginPlay` runs:
1. ✅ Each module class in `DefaultModuleClasses` is spawned
2. ✅ Modules are automatically attached to the station
3. ✅ Modules are added to the station's `Modules` array
4. ✅ Station aggregates functionality from all modules

---

## Method 2: Add Modules Dynamically (Runtime)

### Blueprint Event Graph

For advanced use cases where you need to add modules during gameplay:

```
Event: Add New Module
  ↓
[Spawn Actor from Class]
  - Class: BP_SpaceStationModule_DockingBay
  - Location: Station Location
  - Owner: This Station
  ↓
[Add Module] (Station function)
  - Module: Spawned Module
  ↓
[Attach to Actor]
  - Target: Spawned Module
  - Parent: This Station
  - Transform Rule: Keep Relative
```

### C++ Code Example

```cpp
// Spawn a new module
ASpaceStationModule* NewModule = GetWorld()->SpawnActor<ASpaceStationModule>(
    ModuleClass, 
    GetActorLocation(), 
    GetActorRotation()
);

// Add to station
if (NewModule)
{
    AddModule(NewModule);  // Adds to Modules array and attaches
}
```

---

## Method 3: Manual Placement (Legacy, Not Recommended)

You can manually place module actors in the level, but they won't automatically attach:

1. Drag module Blueprint into level
2. Position near station
3. In module's **Details → Transform**, click dropdown
4. Select **Attach to** → Choose your station actor
5. Repeat for each module

⚠️ **Not Recommended**: Use Method 1 (DefaultModuleClasses) instead for MVP.

---

## How Modules Add Functionality to Stations

### Aggregate Properties

The station **automatically aggregates** functionality from all attached modules:

#### Power Management

```cpp
// Get total power consumption/generation
float TotalPower = Station->GetTotalPowerConsumption();  // Sum of all modules
float PowerGen = Station->GetTotalPowerGeneration();     // Only generators
float Balance = Station->GetPowerBalance();              // Net power (gen - consumption)
```

**Example:**
- Module 1: Docking Bay (+50 power) = consumes power
- Module 2: Reactor (-500 power) = generates power
- Module 3: Marketplace (+40 power) = consumes power
- **Total Consumption**: 90 power
- **Total Generation**: 500 power
- **Power Balance**: +410 power (surplus)

#### Capability Queries

```cpp
// Check station capabilities (from modules)
bool CanDock = Station->HasDockingCapability();      // Has Docking group modules?
bool CanTrade = Station->HasMarketplace();           // Has Marketplace module?
bool HasStorage = Station->HasCargoStorage();        // Has Storage group modules?
```

**Usage in Blueprints:**
```
[Branch: Has Marketplace?]
  True → Open Trading UI
  False → Display "No marketplace at this station"
```

#### Module Counts

```cpp
// Count modules by group
int32 DockingCount = Station->GetModuleCountByGroup(EStationModuleGroup::Docking);
int32 StorageCount = Station->GetModuleCountByGroup(EStationModuleGroup::Storage);
int32 PublicCount = Station->GetModuleCountByGroup(EStationModuleGroup::Public);
```

**UI Example:**
```
Station Facilities:
  Docking Bays: 2
  Cargo Storage: 3
  Public Spaces: 1
  Power: +410 MW (Surplus)
```

#### Get Modules by Group

```cpp
// Get all modules of a specific type
TArray<ASpaceStationModule*> DockingModules = 
    Station->GetModulesByGroup(EStationModuleGroup::Docking);

// Iterate and use them
for (ASpaceStationModule* Module : DockingModules)
{
    // Do something with each docking module
}
```

---

## Blueprint Example: Station Status Widget

Here's how to display station capabilities in a UI widget:

### Blueprint Graph

```
Event: Update Station Info
  ↓
[Get Total Power Consumption] → [Set Text: Power Status]
  ↓
[Get Power Balance] → [Branch]
  Positive → Set Color: Green (Surplus)
  Negative → Set Color: Red (Deficit)
  ↓
[Has Docking Capability?] → [Set Visibility: Docking Icon]
  ↓
[Has Marketplace?] → [Set Visibility: Trade Icon]
  ↓
[Get Module Count By Group: Storage] → [Set Text: "Storage Bays: X"]
```

### Example UI Output

```
═══════════════════════════════════
  STATION STATUS: Trade Hub Alpha
═══════════════════════════════════
  Power: +410 MW ✓ (Surplus)
  
  Facilities:
    🚀 Docking: 2 bays
    📦 Storage: 3 cargo holds
    🏪 Marketplace: Available
    🏠 Habitation: 1 module
═══════════════════════════════════
```

---

## Module Groups Reference

Modules are categorized into groups that define their purpose:

| Group | Purpose | MVP Status | Example Modules |
|-------|---------|------------|-----------------|
| **Docking** | Ship docking facilities | ✅ MVP | DockingBay, DockingPort |
| **Storage** | Cargo storage | ✅ MVP | CargoBay, FuelDepot |
| **Public** | Trading and social spaces | ✅ MVP | Marketplace, Plaza |
| **Habitation** | Living quarters | ✅ MVP | Habitation, Barracks |
| **Connection** | Corridors and passages | ✅ MVP | Corridor |
| **Power** | Power generation | ⏸ Post-MVP | Reactor, SolarArray |
| **Processing** | Manufacturing | ⏸ Post-MVP | Fabrication, Refinery |
| **Defence** | Weapons and shields | ⏸ Post-MVP | Turret, ShieldGenerator |

---

## Common Issues and Solutions

### Issue: Modules Not Appearing

**Problem**: Added modules to DefaultModuleClasses but they don't spawn.

**Solutions:**
1. ✅ Verify Blueprint compiled successfully
2. ✅ Check module classes are valid (not null entries)
3. ✅ Ensure C++ code compiled (modules need C++ parents)
4. ✅ Check Output Log for spawn errors

### Issue: Modules Floating Separately

**Problem**: Modules spawn but aren't attached to station.

**Solutions:**
1. ✅ Check `AddModule()` is called in BeginPlay (should be automatic)
2. ✅ Verify modules are in the `Modules` array (check in Details panel at runtime)
3. ✅ Use `AddModuleAtLocation()` instead of manual spawning

### Issue: Can't Find Module Classes

**Problem**: Can't select module Blueprint in DefaultModuleClasses.

**Solutions:**
1. ✅ Create module Blueprints first (see `STATION_MODULE_BLUEPRINTS_GUIDE.md`)
2. ✅ Ensure modules inherit from `ASpaceStationModule` or subclass
3. ✅ Check modules are marked `Blueprintable` in C++
4. ✅ Restart editor if modules were just created

### Issue: Aggregate Functions Not Working

**Problem**: `HasMarketplace()` returns false even with marketplace module.

**Solutions:**
1. ✅ Check module was actually spawned (check `Modules` array)
2. ✅ Verify module is correct type (e.g., `AMarketplaceModule`)
3. ✅ Ensure module's `ModuleGroup` is set correctly
4. ✅ Call aggregate functions after BeginPlay completes

---

## Best Practices

### ✅ DO

- **Use DefaultModuleClasses** for pre-built stations (MVP approach)
- **Check capabilities** before showing UI (e.g., `HasMarketplace()` before opening trade UI)
- **Display aggregate info** in station UI (power balance, facility counts)
- **Keep modules simple** in MVP (add complexity post-MVP)

### ❌ DON'T

- **Don't manually attach** modules in level (use DefaultModuleClasses)
- **Don't assume capabilities** exist (always check `HasDockingCapability()`, etc.)
- **Don't modify module properties** that are auto-set by C++ parent
- **Don't add complex module logic** until MVP validates trading gameplay

---

## MVP-Specific Notes

For the **Trade Simulator MVP**, focus on these modules:

### Essential Modules (Must Have)

1. **Docking Bay** (`BP_SpaceStationModule_DockingBay`)
   - Purpose: Player docks here to access station
   - Required: ✅ YES

2. **Marketplace** (`BP_SpaceStationModule_Marketplace`)
   - Purpose: Trading interface location
   - Required: ✅ YES

3. **Cargo Storage** (`BP_CargoBayModule`)
   - Purpose: Station stores traded goods here
   - Required: ✅ YES (for station inventory)

### Nice-to-Have Modules (Optional)

4. **Habitation** (`BP_SpaceStationModule_Habitation`)
   - Purpose: Visual completeness
   - Required: ⚠️ OPTIONAL (visual only)

5. **Corridor** (`BP_SpaceStationModule_Corridor`)
   - Purpose: Station navigation (post-MVP)
   - Required: ❌ NO (defer until interior exploration)

### Post-MVP Modules (Defer)

- Power generation (Reactor, Solar Array)
- Manufacturing (Fabrication, Processing)
- Defence (Turrets, Shields)

---

## Next Steps

### For Designers

1. ✅ Create missing module Blueprints (see `STATION_MODULE_BLUEPRINTS_GUIDE.md`)
2. ✅ Configure `DefaultModuleClasses` in `BP_SpaceStation`
3. ✅ Test station spawns with all modules
4. ✅ Create station UI that shows aggregate info

### For Programmers

1. ✅ Review aggregate functionality implementation
2. ✅ Add additional aggregate queries if needed (e.g., total cargo capacity)
3. ✅ Implement trading logic that uses `HasMarketplace()`
4. ✅ Create docking logic that uses `HasDockingCapability()`

---

## Related Documentation

- **Module Creation**: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`
- **Quick Reference**: `docs/reference/STATION_MODULE_BLUEPRINTS_QUICK_REFERENCE.md`
- **Station Editor**: `Content/Blueprints/STATION_EDITOR_README.md`
- **C++ API**: `Source/Adastrea/Public/Stations/SpaceStation.h`

---

## Summary

**Key Takeaways:**

1. ✅ Modules are **separate actors** that **attach to stations**
2. ✅ Add modules via **DefaultModuleClasses** array in Class Defaults
3. ✅ Station **automatically aggregates** functionality from modules
4. ✅ Query capabilities with `HasMarketplace()`, `HasDockingCapability()`, etc.
5. ✅ Get aggregate info with `GetTotalPowerConsumption()`, `GetModuleCountByGroup()`, etc.
6. ✅ Modules contribute to station's overall functionality (power, capabilities, facilities)

**Modules ADD functionality to the station core** through aggregation. The station queries its modules to determine what it can do.

---

**Last Updated**: 2026-01-12  
**Status**: Complete - Ready for MVP Trade Simulator  
**Author**: Adastrea Development Team
