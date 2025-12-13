# Station Editor C++ Blueprint Integration Guide

## Overview

This guide explains how to create the Blueprint widget assets that integrate with the C++ Station Editor implementation.

## Prerequisites

- C++ code compiled successfully
- Unreal Engine 5.6 Editor open
- Content Browser navigation to appropriate folders

## Part 1: Create Blueprint Widgets

### 1. WBP_StationEditor (Main Editor Widget)

**Location**: `Content/UI/Station/WBP_StationEditor.uasset`

**Parent Class**: `StationEditorWidgetCpp` (C++ class)

**Widget Hierarchy**:
```
Canvas Panel (Root)
└─ Border (FullScreen, Name: "Background")
   ├─ Fill: Semi-transparent black (0, 0, 0, 0.8)
   └─ Horizontal Box (Name: "MainLayout")
      ├─ Vertical Box (Name: "LeftPanel", Size: 280px width)
      │  ├─ Text Block: "Available Modules"
      │  │  └─ Font Size: 18, Color: White
      │  └─ Scroll Box (Name: "ModuleListScrollBox") **CRITICAL: Must match C++ BindWidget**
      │     └─ Auto-populated by C++
      │
      ├─ Vertical Box (Name: "CenterPanel", Fill, Padding: 20px)
      │  └─ Text Block: "Station Editor Active"
      │     └─ Font Size: 24, Color: White, Alignment: Center
      │
      └─ Vertical Box (Name: "RightPanel", Size: 320px width, Padding: 10px)
         ├─ Text Block: "Station Statistics"
         │  └─ Font Size: 18, Color: White
         ├─ Spacer (10px)
         ├─ Text Block (Name: "PowerDisplayText") **CRITICAL: Must match C++ BindWidget**
         │  └─ Text: "Power: 0 / 0 MW"
         │  └─ Font Size: 14, Color: Yellow
         ├─ Spacer (5px)
         ├─ Text Block (Name: "ModuleCountDisplay") **CRITICAL: Must match C++ BindWidget**
         │  └─ Text: "Modules: 0 / 25"
         │  └─ Font Size: 14, Color: White
         ├─ Spacer (5px)
         ├─ Progress Bar (Name: "PowerBalanceBar") **CRITICAL: Must match C++ BindWidget**
         │  └─ Fill Color: Green (default), Size: Full Width, Height: 20px
         ├─ Spacer (20px)
         ├─ Button (Name: "CloseButton") **CRITICAL: Must match C++ BindWidget**
         │  └─ Text: "Close Editor"
         │  └─ Size: Full Width, Height: 40px
         ├─ Spacer (20px)
         ├─ Text Block: "Construction Queue"
         │  └─ Font Size: 16, Color: White
         └─ Scroll Box (Name: "QueueScrollBox") **CRITICAL: Must match C++ BindWidget**
            └─ Auto-populated by C++
```

**Widget Class Settings**:
- Is Variable: Checked (all named widgets)
- All named widgets with "**CRITICAL**" comment MUST have exact names matching C++ BindWidget properties

**Blueprint Configuration**:
- Variables:
  - `ModuleListItemClass`: Set to `WBP_ModuleListItem` (created in step 2)
  - `QueueItemClass`: Set to `WBP_ConstructionQueueItem` (created in step 3)

**NO Blueprint Graph Logic Required** - All logic is in C++

---

### 2. WBP_ModuleListItem (Module Catalog Item)

**Location**: `Content/UI/Station/WBP_ModuleListItem.uasset`

**Parent Class**: `ModuleListItemWidget` (C++ class)

**Widget Hierarchy**:
```
Border (Root, Size: 260px width, 100px height)
├─ Fill Color: Dark Gray (0.1, 0.1, 0.1, 1.0)
├─ Border Color: Light Gray (0.3, 0.3, 0.3, 1.0)
└─ Vertical Box (Padding: 10px)
   ├─ Text Block (Name: "ModuleNameText") **CRITICAL: Must match C++ BindWidget**
   │  └─ Text: "Module Name"
   │  └─ Font Size: 14, Bold, Color: White
   ├─ Spacer (5px)
   ├─ Text Block (Name: "DescriptionText") **CRITICAL: Must match C++ BindWidget**
   │  └─ Text: "Module description"
   │  └─ Font Size: 10, Color: Light Gray, Wrap Text: True
   ├─ Spacer (5px)
   ├─ Text Block (Name: "CostText") **CRITICAL: Must match C++ BindWidget**
   │  └─ Text: "0 Credits | 0s"
   │  └─ Font Size: 11, Color: Yellow
   ├─ Spacer (5px)
   └─ Button (Name: "BuildButton") **CRITICAL: Must match C++ BindWidget**
      └─ Text: "Build"
      └─ Size: Full Width, Height: 30px
```

**NO Blueprint Graph Logic Required** - All logic is in C++

---

### 3. WBP_ConstructionQueueItem (Queue Entry Widget)

**Location**: `Content/UI/Station/WBP_ConstructionQueueItem.uasset`

**Parent Class**: `ConstructionQueueItemWidget` (C++ class)

**Widget Hierarchy**:
```
Border (Root, Size: 300px width, 80px height)
├─ Fill Color: Dark Blue (0.1, 0.1, 0.2, 1.0)
└─ Horizontal Box (Padding: 10px)
   ├─ Vertical Box (Fill)
   │  ├─ Text Block (Name: "ItemNameText") **CRITICAL: Must match C++ BindWidget**
   │  │  └─ Text: "Module Name"
   │  │  └─ Font Size: 13, Color: White
   │  ├─ Spacer (5px)
   │  ├─ Progress Bar (Name: "BuildProgressBar") **CRITICAL: Must match C++ BindWidget**
   │  │  └─ Fill Color: Blue, Size: Full Width, Height: 20px
   │  └─ Text Block (Name: "TimeRemainingText") **CRITICAL: Must match C++ BindWidget**
   │     └─ Text: "00:00"
   │     └─ Font Size: 11, Color: Light Gray
   └─ Button (Name: "CancelButton", Size: 60px width) **CRITICAL: Must match C++ BindWidget**
      └─ Text: "Cancel"
```

**NO Blueprint Graph Logic Required** - All logic is in C++

---

## Part 2: Create Input Action

### IA_OpenStationEditor

**Location**: `Content/Input/Actions/IA_OpenStationEditor.uasset`

**Settings**:
- Value Type: **Digital (bool)**
- Consume Input: **Checked**
- Triggers: **Pressed**

---

## Part 3: Update Input Mapping Context

### IMC_Spaceship

**Location**: `Content/Input/IMC_Spaceship.uasset`

**Add New Mapping**:
1. Open IMC_Spaceship
2. Add new mapping entry:
   - Action: `IA_OpenStationEditor`
   - Key: `F9`
   - Modifiers: None
   - Triggers: Pressed

---

## Part 4: Create Module Catalog Data Asset

### DA_StationModuleCatalog

**Location**: `Content/DataAssets/Stations/DA_StationModuleCatalog.uasset`

**Class**: `StationModuleCatalog` (from StationEditor module)

**Module Entries** (Add 5 test modules):

#### Entry 0: Reactor Core
- Module Class: `BP_ReactorModule` (if exists, or leave empty for testing)
- Display Name: "Reactor Core"
- Description: "Generates power for the station"
- Required Tech Level: 1
- Module Group: Power
- Build Cost:
  - Credits: 5000
  - Build Time: 60.0
- Preview Mesh: (Optional)

#### Entry 1: Docking Bay
- Module Class: `BP_DockingBay`
- Display Name: "Docking Bay"
- Description: "Allows ships to dock and transfer cargo"
- Required Tech Level: 1
- Module Group: Docking
- Build Cost:
  - Credits: 3000
  - Build Time: 45.0

#### Entry 2: Cargo Storage
- Module Class: `BP_CargoStorage`
- Display Name: "Cargo Storage"
- Description: "Stores cargo and materials"
- Required Tech Level: 1
- Module Group: Storage
- Build Cost:
  - Credits: 2000
  - Build Time: 30.0

#### Entry 3: Living Quarters
- Module Class: `BP_LivingQuarters`
- Display Name: "Living Quarters"
- Description: "Houses station crew"
- Required Tech Level: 2
- Module Group: Habitation
- Build Cost:
  - Credits: 4000
  - Build Time: 90.0

#### Entry 4: Defense Turret
- Module Class: `BP_DefenseTurret`
- Display Name: "Defense Turret"
- Description: "Defends the station from threats"
- Required Tech Level: 2
- Module Group: Defence
- Build Cost:
  - Credits: 6000
  - Build Time: 120.0

---

## Part 5: Configure Player Controller

### BP_SpaceshipController (or your game's player controller)

**Parent Class**: Should be or inherit from `AdastreaPlayerController`

**Configuration**:
1. Open the Blueprint
2. Set the following properties:
   - `Station Editor Widget Class`: `WBP_StationEditor`
   - `Module Catalog`: `DA_StationModuleCatalog`
   - `Station Search Radius`: 5000.0

**Input Binding** (if not using DA_InputConfig):
1. In Event Graph, add:
   - Event: `Enhanced Input Action (IA_OpenStationEditor)`
   - Connected to: `Toggle Station Editor` (C++ function)

---

## Testing Checklist

After creating all assets:

1. **Compilation**:
   - [ ] Project compiles without errors
   - [ ] No missing BindWidget warnings in output log

2. **Basic Functionality**:
   - [ ] Press F9 near a station
   - [ ] Editor UI appears
   - [ ] Module list populates with 5 test modules
   - [ ] Statistics display shows power and module count
   - [ ] Close button works

3. **Module Placement** (if station and module BPs exist):
   - [ ] Click a module button
   - [ ] Module placement attempted at cursor
   - [ ] Statistics update after placement

4. **Construction Queue** (if implemented):
   - [ ] Queue items appear when modules queued
   - [ ] Progress bars update
   - [ ] Cancel button works

---

## Troubleshooting

### Widget Not Appearing
- Check Output Log for "StationEditorWidgetClass is not set"
- Verify BP_SpaceshipController has `WBP_StationEditor` assigned
- Ensure widget names match C++ BindWidget exactly

### Module List Empty
- Check `ModuleCatalog` is assigned in controller
- Verify DA_StationModuleCatalog has entries
- Check Output Log for catalog errors

### Placement Not Working
- Ensure station actor exists in level
- Check station is within 5000 units
- Verify module classes in catalog are valid

### BindWidget Errors
- Widget names are case-sensitive
- Must match C++ property names exactly:
  - `ModuleListScrollBox`
  - `PowerDisplayText`
  - `ModuleCountDisplay`
  - `PowerBalanceBar`
  - `CloseButton`
  - `QueueScrollBox`
  - (Same for child widgets)

---

## Architecture Notes

### C++ Logic Flow
1. Player presses F9
2. `AdastreaPlayerController::ToggleStationEditor()` called
3. `FindNearestStation()` locates station
4. `ShowStationEditor(Station)` creates widget
5. Calls `InitializeEditor(Station, Catalog)` on widget
6. Widget's `NativeConstruct()` runs:
   - Creates EditorManager
   - Binds to EditorManager events
   - Calls RefreshModuleList()
   - Calls RefreshStatistics()

### Event-Driven Updates
- Module placement → `OnModulePlaced` event → `RefreshStatistics()`
- Module removed → `OnModuleRemoved` event → `RefreshStatistics()`
- Statistics changed → `OnStatisticsUpdated` event → `RefreshStatistics()`
- Queue changed → `OnConstructionQueueChanged` event → `UpdateConstructionQueue()`

### No Blueprint Logic Needed
All UI logic is in C++:
- Event binding
- List population
- Statistics calculation
- Module placement
- Queue management

Blueprints only provide visual layout.

---

## Next Steps

After creating all assets:
1. Test in PIE (Play In Editor)
2. Verify F9 opens editor
3. Test module selection
4. Test statistics display
5. Test close button
6. Report any issues

---

**Version**: 1.0  
**Last Updated**: December 13, 2025  
**Maintained by**: Adastrea Development Team
