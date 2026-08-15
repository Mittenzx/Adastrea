# SpaceStation Module Configuration - Visual Guide

## Problem: Empty Dropdown in Class Defaults

### Before Fix ❌

```
Class Defaults Editor
┌────────────────────────────────────────┐
│ ASpaceStation                          │
│                                        │
│ Category: Station                      │
│ ┌────────────────────────────────────┐ │
│ │ Modules  [Array]                   │ │
│ │   [+] Add Element                  │ │
│ │   ├─[0] None ▼                     │ │ ← EMPTY DROPDOWN!
│ │   │   └─ (No options available)    │ │   No results shown
│ │   └─[1] None ▼                     │ │
│ │       └─ (No options available)    │ │
│ └────────────────────────────────────┘ │
│                                        │
└────────────────────────────────────────┘

Why? TArray<ASpaceStationModule*> expects
EXISTING INSTANCES in the level, but none exist yet!
```

### After Fix ✅

```
Class Defaults Editor
┌────────────────────────────────────────┐
│ ASpaceStation                          │
│                                        │
│ Category: Station | Configuration      │
│ ┌────────────────────────────────────┐ │
│ │ DefaultModuleClasses  [Array]      │ │
│ │   [+] Add Element                  │ │
│ │   ├─[0] BP_DockingBayModule ▼      │ │ ← DROPDOWN WORKS!
│ │   │   ├─ BP_DockingBayModule       │ │   Shows all module
│ │   │   ├─ BP_ReactorModule          │ │   Blueprint classes
│ │   │   ├─ BP_CargoBayModule         │ │
│ │   │   └─ BP_HabitationModule       │ │
│ │   └─[1] BP_ReactorModule ▼         │ │
│ └────────────────────────────────────┘ │
│                                        │
│ Category: Station                      │
│ ┌────────────────────────────────────┐ │
│ │ Modules  [Read Only]               │ │
│ │   └─ (Populated at runtime)        │ │ ← Runtime tracking
│ └────────────────────────────────────┘ │
│                                        │
└────────────────────────────────────────┘

Why? TSubclassOf<ASpaceStationModule> allows
selecting BLUEPRINT CLASSES from project!
```

---

## System Architecture

### Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         DESIGN TIME                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Designer Opens Class Defaults                                 │
│           ↓                                                     │
│  Configures DefaultModuleClasses Array                         │
│           ↓                                                     │
│  ┌───────────────────────────────────────┐                     │
│  │  DefaultModuleClasses (TSubclassOf)   │                     │
│  ├───────────────────────────────────────┤                     │
│  │  [0] BP_DockingBayModule              │  ← Classes, not     │
│  │  [1] BP_ReactorModule                 │    instances        │
│  │  [2] BP_CargoBayModule                │                     │
│  └───────────────────────────────────────┘                     │
│           ↓                                                     │
│  Saves Blueprint                                               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│                         RUNTIME                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Station Spawned in Level                                      │
│           ↓                                                     │
│  BeginPlay() Executes                                          │
│           ↓                                                     │
│  ┌───────────────────────────────────┐                         │
│  │  For Each Class in                │                         │
│  │  DefaultModuleClasses:            │                         │
│  │    ↓                              │                         │
│  │  SpawnActor<ModuleClass>()        │  ← Spawn instances     │
│  │    ↓                              │    from classes        │
│  │  AddModule(NewModule)             │                         │
│  └───────────────────────────────────┘                         │
│           ↓                                                     │
│  ┌───────────────────────────────────┐                         │
│  │  Modules (Actor Pointers)         │                         │
│  ├───────────────────────────────────┤                         │
│  │  [0] DockingBayModule_C_0*        │  ← Actual spawned      │
│  │  [1] ReactorModule_C_0*           │    instances           │
│  │  [2] CargoBayModule_C_0*          │                         │
│  └───────────────────────────────────┘                         │
│           ↓                                                     │
│  Modules Attached and Functional                               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Array Comparison

### Visual Comparison of Two Arrays

```
┌──────────────────────────────────────┬──────────────────────────────────────┐
│  DefaultModuleClasses                │  Modules                             │
│  (EDITOR CONFIGURATION)              │  (RUNTIME TRACKING)                  │
├──────────────────────────────────────┼──────────────────────────────────────┤
│                                      │                                      │
│  Property Type:                      │  Property Type:                      │
│  TSubclassOf<ASpaceStationModule>    │  ASpaceStationModule*                │
│                                      │                                      │
│  Access:                             │  Access:                             │
│  EditAnywhere, BlueprintReadOnly     │  VisibleAnywhere, BlueprintReadOnly  │
│                                      │                                      │
│  Category:                           │  Category:                           │
│  Station | Configuration             │  Station                             │
│                                      │                                      │
│  Content:                            │  Content:                            │
│  ┌────────────────────────────────┐  │  ┌────────────────────────────────┐ │
│  │ [0] BP_DockingBayModule        │  │  │ [0] DockingBayModule_C_0*      │ │
│  │     (Class reference)          │  │  │     (Actor instance)           │ │
│  │                                │  │  │                                │ │
│  │ [1] BP_ReactorModule           │  │  │ [1] ReactorModule_C_0*         │ │
│  │     (Class reference)          │  │  │     (Actor instance)           │ │
│  │                                │  │  │                                │ │
│  │ [2] BP_CargoBayModule          │  │  │ [2] CargoBayModule_C_0*        │ │
│  │     (Class reference)          │  │  │     (Actor instance)           │ │
│  └────────────────────────────────┘  │  └────────────────────────────────┘ │
│                                      │                                      │
│  When to Edit:                       │  When to Edit:                       │
│  ✅ In Class Defaults                │  ❌ Never (auto-populated)           │
│  ✅ In Blueprint editor              │                                      │
│                                      │                                      │
│  Purpose:                            │  Purpose:                            │
│  Define WHAT to spawn                │  Track WHAT is spawned               │
│                                      │                                      │
│  Lifecycle:                          │  Lifecycle:                          │
│  Design time configuration           │  Runtime population                  │
│                                      │                                      │
│  Usage:                              │  Usage:                              │
│  • Template definition               │  • Query attached modules            │
│  • Station variants                  │  • Iterate for functionality         │
│  • Default loadout                   │  • Dynamic add/remove                │
│                                      │                                      │
└──────────────────────────────────────┴──────────────────────────────────────┘
```

---

## BeginPlay Execution Flow

### Visual Flow Diagram

```
                        Station BeginPlay() Starts
                                   ↓
                    ┌──────────────────────────────┐
                    │ Check DefaultModuleClasses   │
                    │ Array Length                 │
                    └──────────────┬───────────────┘
                                   ↓
                    ┌──────────────────────────────┐
                    │ Length > 0?                  │
                    └──────┬───────────────────┬───┘
                           │ Yes               │ No
                           ↓                   ↓
            ┌──────────────────────────┐  [Exit BeginPlay]
            │ Get World Reference      │
            └──────────┬───────────────┘
                       ↓
            ┌──────────────────────────┐
            │ For Each ModuleClass     │
            │ in DefaultModuleClasses  │
            └──────────┬───────────────┘
                       ↓
            ┌──────────────────────────────────────────┐
            │ Iteration Loop                           │
            ├──────────────────────────────────────────┤
            │                                          │
            │  Current Class Valid?                    │
            │         ↓ Yes                            │
            │  ┌──────────────────────────────────┐    │
            │  │ SpawnActor<ModuleClass>()        │    │
            │  │  - Location: Station location    │    │
            │  │  - Rotation: Station rotation    │    │
            │  │  - Owner: This station           │    │
            │  │  - Collision: Always spawn       │    │
            │  └──────────────┬───────────────────┘    │
            │                 ↓                        │
            │  ┌──────────────────────────────────┐    │
            │  │ Spawn Successful?                │    │
            │  └──────┬───────────────────────┬───┘    │
            │         ↓ Yes                  ↓ No      │
            │  ┌──────────────────────┐  ┌─────────┐  │
            │  │ AddModule(NewModule) │  │ Log Err │  │
            │  │  - Adds to Modules[] │  └─────────┘  │
            │  │  - Attaches to actor │               │
            │  │  - Logs success      │               │
            │  └──────────────────────┘               │
            │         ↓                                │
            │  [Continue to next class]                │
            │                                          │
            └──────────────────────────────────────────┘
                       ↓
            ┌──────────────────────────┐
            │ Log Final Module Count   │
            │ to Output Log            │
            └──────────────────────────┘
                       ↓
                  [BeginPlay Complete]
```

---

## Usage Workflow

### Step-by-Step Visual Guide

```
┌─────────────────────────────────────────────────────────────────┐
│ STEP 1: Open SpaceStation Blueprint                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Content Browser                                               │
│  ┌─────────────────────────────────────────────┐               │
│  │ Blueprints/Stations/                        │               │
│  │   📁 BP_SpaceStation                        │  ← Double     │
│  │   📁 BP_TradingStation                      │    click to   │
│  │   📁 BP_MilitaryStation                     │    open       │
│  └─────────────────────────────────────────────┘               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│ STEP 2: Open Class Defaults                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Blueprint Editor Toolbar                                      │
│  ┌─────────────────────────────────────────────┐               │
│  │ [Compile] [Save] [Class Defaults] [Browse]  │  ← Click      │
│  └─────────────────────────────────────────────┘    this       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│ STEP 3: Find DefaultModuleClasses Array                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Details Panel (right side)                                    │
│  ┌─────────────────────────────────────────────┐               │
│  │ ▼ Station | Configuration                   │  ← Expand     │
│  │    ┌─────────────────────────────────────┐  │    category   │
│  │    │ Default Module Classes  [Array]     │  │               │
│  │    │   [+] Add Element                   │  │  ← Found it!  │
│  │    └─────────────────────────────────────┘  │               │
│  └─────────────────────────────────────────────┘               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│ STEP 4: Add Module Classes                                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────┐               │
│  │ Default Module Classes  [Array]             │               │
│  │   [+] Add Element  ← Click                  │               │
│  │   ├─[0] None ▼                              │               │
│  │   │     ├─ BP_DockingBayModule  ← Select    │               │
│  │   │     ├─ BP_ReactorModule                 │               │
│  │   │     ├─ BP_CargoBayModule                │               │
│  │   │     └─ BP_HabitationModule              │               │
│  │   │                                          │               │
│  │   [+] Add Element  ← Click again            │               │
│  │   └─[1] None ▼                              │               │
│  │         ├─ BP_DockingBayModule              │               │
│  │         ├─ BP_ReactorModule     ← Select    │               │
│  │         ├─ BP_CargoBayModule                │               │
│  │         └─ BP_HabitationModule              │               │
│  └─────────────────────────────────────────────┘               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│ STEP 5: Save and Compile                                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────┐               │
│  │ [Compile] ← Click                           │               │
│  │ [Save]    ← Click                           │               │
│  └─────────────────────────────────────────────┘               │
│                                                                 │
│  ✅ Blueprint compiled successfully                             │
│  ✅ Asset saved                                                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│ STEP 6: Test in Level                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Place BP_SpaceStation in level                             │
│     └─ Drag from Content Browser to viewport                   │
│                                                                 │
│  2. Click Play (PIE)                                           │
│     └─ Modules spawn automatically at BeginPlay                │
│                                                                 │
│  3. Check Output Log                                           │
│     └─ Window → Developer Tools → Output Log                   │
│                                                                 │
│  Expected Output:                                              │
│  ┌─────────────────────────────────────────────┐               │
│  │ LogAdastreaStations: SpaceStation::Begin   │               │
│  │ Play - Spawned default module: DockingBay  │               │
│  │ Module_C_0 for station BP_SpaceStation_C_0 │               │
│  │                                             │               │
│  │ LogAdastreaStations: SpaceStation::Begin   │               │
│  │ Play - Spawned default module: ReactorMod  │               │
│  │ ule_C_0 for station BP_SpaceStation_C_0    │               │
│  │                                             │               │
│  │ LogAdastreaStations: SpaceStation::Begin   │               │
│  │ Play - Station BP_SpaceStation_C_0 initial │               │
│  │ ized with 2 default modules                │               │
│  └─────────────────────────────────────────────┘               │
│                                                                 │
│  ✅ SUCCESS: Modules spawned and attached!                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Common Configurations

### Trading Station Setup

```
┌─────────────────────────────────────────┐
│ BP_TradingStation                       │
├─────────────────────────────────────────┤
│                                         │
│ DefaultModuleClasses:                   │
│   ┌─────────────────────────────────┐   │
│   │ [0] BP_DockingBayModule         │   │  ← Player docking
│   │ [1] BP_DockingPortModule        │   │  ← NPC ships
│   │ [2] BP_CargoBayModule           │   │  ← Storage
│   │ [3] BP_MarketplaceModule        │   │  ← Trading UI
│   │ [4] BP_HabitationModule         │   │  ← Population
│   └─────────────────────────────────┘   │
│                                         │
│ Result:                                 │
│   ┌──────────────────────┐              │
│   │   [🚢]              │              │
│   │    │                │              │
│   │    ├─ DockingBay    │              │
│   │    ├─ DockingPort   │              │
│   │    ├─ CargoBay      │              │
│   │    ├─ Marketplace   │              │
│   │    └─ Habitation    │              │
│   │                     │              │
│   │  Trading Station    │              │
│   └──────────────────────┘              │
│                                         │
└─────────────────────────────────────────┘
```

### Military Station Setup

```
┌─────────────────────────────────────────┐
│ BP_MilitaryStation                      │
├─────────────────────────────────────────┤
│                                         │
│ DefaultModuleClasses:                   │
│   ┌─────────────────────────────────┐   │
│   │ [0] BP_DockingBayModule         │   │  ← Ship docking
│   │ [1] BP_ShieldGeneratorModule    │   │  ← Defense
│   │ [2] BP_TurretModule             │   │  ← Weapons
│   │ [3] BP_BarracksModule           │   │  ← Crew
│   │ [4] BP_ReactorModule            │   │  ← Power
│   └─────────────────────────────────┘   │
│                                         │
│ Result:                                 │
│   ┌──────────────────────┐              │
│   │   [⚔️]              │              │
│   │    │                │              │
│   │    ├─ DockingBay    │              │
│   │    ├─ Shields       │              │
│   │    ├─ Turrets       │              │
│   │    ├─ Barracks      │              │
│   │    └─ Reactor       │              │
│   │                     │              │
│   │  Military Station   │              │
│   └──────────────────────┘              │
│                                         │
└─────────────────────────────────────────┘
```

---

## Troubleshooting Flowchart

```
                      Problem: Modules Not Working
                                   ↓
                    ┌──────────────────────────────┐
                    │ Is dropdown showing classes? │
                    └──────┬───────────────────┬───┘
                           │ Yes               │ No
                           ↓                   ↓
            ┌─────────────────────┐    ┌──────────────────────┐
            │ Are modules spawning│    │ Module Blueprints    │
            │ in level?           │    │ don't exist yet      │
            └──────┬──────────────┘    │                      │
                   │                   │ FIX:                 │
            Yes    │    No             │ 1. Create module BPs │
            ↓      ↓                   │ 2. Refresh dropdown  │
    ┌─────────┐ ┌─────────────────┐   └──────────────────────┘
    │ Working!│ │ Check Output Log│
    └─────────┘ └────────┬────────┘
                         ↓
            ┌────────────────────────────┐
            │ "Failed to spawn module"?  │
            └────┬───────────────────┬───┘
                 │ Yes               │ No
                 ↓                   ↓
      ┌──────────────────┐  ┌───────────────────┐
      │ Module class null│  │ "Spawned default" │
      │ or invalid       │  │ messages present? │
      │                  │  └────────┬──────────┘
      │ FIX:             │           │
      │ Verify class     │    Yes    │    No
      │ selection valid  │           ↓          ↓
      └──────────────────┘  ┌─────────────┐ ┌────────────┐
                            │ BeginPlay   │ │ Check if   │
                            │ not running?│ │ BeginPlay  │
                            │             │ │ overridden │
                            │ FIX:        │ │ in BP      │
                            │ Check Play  │ └────────────┘
                            │ mode works  │
                            └─────────────┘
```

---

## Quick Reference Card

```
╔═══════════════════════════════════════════════════════════════╗
║               SPACESTATION MODULE CONFIGURATION                ║
║                    Quick Reference Card                        ║
╠═══════════════════════════════════════════════════════════════╣
║                                                               ║
║  PROPERTY NAME:  DefaultModuleClasses                         ║
║  PROPERTY TYPE:  TArray<TSubclassOf<ASpaceStationModule>>     ║
║  CATEGORY:       Station | Configuration                      ║
║  ACCESS:         EditAnywhere, BlueprintReadOnly              ║
║                                                               ║
║  PURPOSE:        Define module classes to spawn at BeginPlay  ║
║                                                               ║
╠═══════════════════════════════════════════════════════════════╣
║  HOW TO USE:                                                  ║
║  1. Open SpaceStation Blueprint                               ║
║  2. Click "Class Defaults" button                             ║
║  3. Find "Station | Configuration" category                   ║
║  4. Click [+] on "Default Module Classes"                     ║
║  5. Select module class from dropdown                         ║
║  6. Repeat for each module needed                             ║
║  7. Save and compile                                          ║
║  8. Test in PIE - modules spawn automatically                 ║
╠═══════════════════════════════════════════════════════════════╣
║  AVAILABLE MODULES:                                           ║
║  • BP_DockingBayModule      - Large ship docking              ║
║  • BP_DockingPortModule     - Small ship docking              ║
║  • BP_ReactorModule         - Power generation                ║
║  • BP_CargoBayModule        - Storage                         ║
║  • BP_HabitationModule      - Living quarters                 ║
║  • BP_ShieldGeneratorModule - Defense                         ║
║  • BP_MarketplaceModule     - Trading                         ║
║  • BP_ProcessingModule      - Manufacturing                   ║
║  • (any custom module you create)                             ║
╠═══════════════════════════════════════════════════════════════╣
║  VERIFICATION:                                                ║
║  Check Output Log for these messages:                         ║
║  "SpaceStation::BeginPlay - Spawned default module"           ║
║  "SpaceStation::BeginPlay - Station initialized with N        ║
║   default modules"                                            ║
╚═══════════════════════════════════════════════════════════════╝
```

---

**Last Updated**: January 10, 2026
**Created for**: Issue #[number] - Module dropdown empty
**Solution**: Added DefaultModuleClasses array for editor configuration
