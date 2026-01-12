# Space Station Module System - Architecture Diagram

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     ASpaceStation (Core)                        │
│                                                                 │
│  Properties:                                                    │
│  • DefaultModuleClasses (Editor config) ← Set in Class Defaults│
│  • Modules (Runtime array)             ← Auto-populated        │
│  • StationName, Health, etc.                                   │
│                                                                 │
│  Aggregate Functions (NEW):                                    │
│  • GetTotalPowerConsumption()    → Sums all module power      │
│  • GetPowerBalance()              → Net power                  │
│  • HasDockingCapability()        → Check for docking modules  │
│  • HasMarketplace()               → Check for marketplace      │
│  • GetModuleCountByGroup()       → Count by category          │
│  • GetModulesByGroup()           → Filter by category         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                           │
                           │ AttachToActor()
                           │ (Modules are children of station)
                           ▼
    ┌──────────────────────┴─────────────────────┐
    │                                             │
    ▼                                             ▼
┌─────────────────────┐                 ┌─────────────────────┐
│ ASpaceStationModule │                 │ ASpaceStationModule │
│                     │                 │                     │
│ DockingBay          │                 │ Marketplace         │
│ • Power: +50        │                 │ • Power: +40        │
│ • Group: Docking    │                 │ • Group: Public     │
│ • Contributes:      │                 │ • Contributes:      │
│   ✓ Docking         │                 │   ✓ Trading         │
└─────────────────────┘                 └─────────────────────┘
    │                                             │
    │                                             │
    ▼                                             ▼
┌─────────────────────┐                 ┌─────────────────────┐
│ CargoBayModule      │                 │ HabitationModule    │
│ • Power: +5         │                 │ • Power: +30        │
│ • Group: Storage    │                 │ • Group: Habitation │
│ • Contributes:      │                 │ • Contributes:      │
│   ✓ Storage         │                 │   ✓ Living quarters │
└─────────────────────┘                 └─────────────────────┘
```

## How Modules Add Functionality

### 1. Power Aggregation

```
Station Power Calculation:

Module 1 (DockingBay):    +50 power (consumes)
Module 2 (Marketplace):   +40 power (consumes)
Module 3 (CargoBay):      +5 power (consumes)
Module 4 (Reactor):       -500 power (generates) ← POST-MVP
                          ─────
GetTotalPowerConsumption() = +95 power
GetTotalPowerGeneration()  = 500 power (absolute value)
GetPowerBalance()          = +405 power (surplus)
```

### 2. Capability Queries

```
Station Capabilities:

HasDockingCapability() = true   ← Module with Group: Docking exists
HasMarketplace()       = true   ← AMarketplaceModule exists
HasCargoStorage()      = true   ← Module with Group: Storage exists
```

### 3. Module Counting

```
GetModuleCountByGroup(Docking)    = 1  (1 DockingBay)
GetModuleCountByGroup(Storage)    = 1  (1 CargoBay)
GetModuleCountByGroup(Public)     = 1  (1 Marketplace)
GetModuleCountByGroup(Habitation) = 1  (1 Habitation)
                                    ─
Total Modules = 4
```

## Editor Workflow

### Method: DefaultModuleClasses (Recommended for MVP)

```
┌──────────────────────────────────────────────────────────┐
│  1. Open BP_SpaceStation in Blueprint Editor             │
│     Content/Blueprints/Stations/BP_SpaceStation.uasset   │
└──────────────────────────────────────────────────────────┘
                          ▼
┌──────────────────────────────────────────────────────────┐
│  2. Click "Class Defaults" button (top toolbar)          │
└──────────────────────────────────────────────────────────┘
                          ▼
┌──────────────────────────────────────────────────────────┐
│  3. In Details Panel:                                    │
│     Station → Configuration → Default Module Classes     │
│                                                          │
│     Click + to add:                                      │
│     [0] BP_SpaceStationModule_DockingBay                │
│     [1] BP_SpaceStationModule_Marketplace               │
│     [2] BP_CargoBayModule                               │
│     [3] BP_SpaceStationModule_Habitation                │
└──────────────────────────────────────────────────────────┘
                          ▼
┌──────────────────────────────────────────────────────────┐
│  4. Compile and Save Blueprint                           │
└──────────────────────────────────────────────────────────┘
                          ▼
┌──────────────────────────────────────────────────────────┐
│  5. Place BP_SpaceStation in level                       │
│     Press Play                                           │
│     ✓ Modules spawn automatically in BeginPlay          │
│     ✓ Modules attach to station                         │
│     ✓ Aggregate functions work immediately              │
└──────────────────────────────────────────────────────────┘
```

## Blueprint Usage Example

### Trading UI - Check if Station Has Marketplace

```
┌────────────────────────────────────────────────┐
│ Event: Player Docks at Station                 │
└────────────────────────────────────────────────┘
                    ▼
┌────────────────────────────────────────────────┐
│ Get Docked Station                             │
│   → Station (ASpaceStation*)                   │
└────────────────────────────────────────────────┘
                    ▼
┌────────────────────────────────────────────────┐
│ Call: Has Marketplace?                         │
│   Input: Station                               │
│   Return: bool                                 │
└────────────────────────────────────────────────┘
           ▼                    ▼
     ┌─────┴────┐         ┌─────┴────┐
     │   True   │         │  False   │
     └──────────┘         └──────────┘
          ▼                     ▼
┌──────────────────┐  ┌──────────────────────┐
│ Open Trading UI  │  │ Show Message:        │
│ Get Marketplace  │  │ "No marketplace"     │
│ Module → Start   │  │ "Cannot trade here"  │
│ Trading          │  └──────────────────────┘
└──────────────────┘
```

### Station Status UI - Display Facilities

```
┌────────────────────────────────────────────────┐
│ Event: Update Station Info Widget             │
└────────────────────────────────────────────────┘
                    ▼
┌────────────────────────────────────────────────┐
│ Get Module Count By Group: Docking             │
│   → Set Text: "Docking Bays: X"                │
└────────────────────────────────────────────────┘
                    ▼
┌────────────────────────────────────────────────┐
│ Get Module Count By Group: Storage             │
│   → Set Text: "Cargo Holds: X"                 │
└────────────────────────────────────────────────┘
                    ▼
┌────────────────────────────────────────────────┐
│ Get Power Balance                              │
│   → Branch: Positive/Negative                  │
│      Positive: Set Text: "+X MW (Surplus)"     │
│      Negative: Set Text: "-X MW (Deficit)"     │
└────────────────────────────────────────────────┘
```

## Module Groups (Categories)

```
EStationModuleGroup Enum:

┌─────────────┬──────────────────────┬────────────┐
│   Group     │      Purpose         │ MVP Status │
├─────────────┼──────────────────────┼────────────┤
│ Docking     │ Ship docking         │ ✅ MVP     │
│ Storage     │ Cargo storage        │ ✅ MVP     │
│ Public      │ Markets, social      │ ✅ MVP     │
│ Habitation  │ Living quarters      │ ✅ MVP     │
│ Connection  │ Corridors            │ ✅ MVP     │
│ Power       │ Generators           │ ⏸ Post-MVP│
│ Processing  │ Manufacturing        │ ⏸ Post-MVP│
│ Defence     │ Weapons, shields     │ ⏸ Post-MVP│
│ Other       │ Miscellaneous        │ ⏸ Later   │
└─────────────┴──────────────────────┴────────────┘
```

## Key Concepts

### 1. Modules are Separate Actors
- Each module is an independent `AActor`
- Attached to parent station using `AttachToActor()`
- Has own mesh, collision, health

### 2. Station Aggregates Functionality
- Station queries all modules for capabilities
- Calculates totals (power, counts, etc.)
- Provides convenience functions for common checks

### 3. Configuration in Editor
- Use `DefaultModuleClasses` in Class Defaults
- Modules spawn automatically in `BeginPlay()`
- No manual attachment needed

### 4. Blueprint Integration
- All aggregate functions are Blueprint-callable
- Use `BlueprintPure` for queries (no side effects)
- Clear naming conventions (Get*, Has*, etc.)

## Benefits of This Architecture

✅ **Flexible**: Add/remove modules dynamically  
✅ **Modular**: Each module is self-contained  
✅ **Visual**: Modules exist in 3D space  
✅ **Extensible**: Easy to add new module types  
✅ **Designer-Friendly**: Configure in Blueprints  
✅ **Aggregated**: Station provides combined functionality  

## Related Documentation

- **[ADDING_MODULES_TO_STATIONS.md](ADDING_MODULES_TO_STATIONS.md)** - Complete usage guide
- **[STATION_MODULE_BLUEPRINTS_GUIDE.md](../setup/STATION_MODULE_BLUEPRINTS_GUIDE.md)** - Module creation guide
- **[STATION_MODULE_BLUEPRINTS_QUICK_REFERENCE.md](STATION_MODULE_BLUEPRINTS_QUICK_REFERENCE.md)** - Quick reference
- **C++ API**: `Source/Adastrea/Public/Stations/SpaceStation.h`

---

**Last Updated**: 2026-01-12  
**Status**: Complete - Ready for MVP Trade Simulator
