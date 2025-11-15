# Station Management UI Guide

**Version**: 1.0.0  
**Last Updated**: November 15, 2025  
**System Status**: Alpha - Foundation Complete

---

## Table of Contents

- [Overview](#overview)
- [Station Management Widget](#station-management-widget)
- [Module Management](#module-management)
- [Blueprint Integration](#blueprint-integration)
- [Best Practices](#best-practices)
- [Examples](#examples)

---

## Overview

The Station Management UI provides a comprehensive interface for managing space stations in Adastrea. It features:

- **Module Viewing**: See all station modules at a glance
- **Filtering**: Group modules by type
- **Build Mode**: Add new modules to station
- **Status Monitoring**: Track operational status
- **Blueprint-Friendly**: Easy customization and extension

### Key Features

1. **Module Management**
   - View all attached modules
   - Filter by module group (Docking, Power, etc.)
   - Build and remove modules
   - Check operational status

2. **Station Status**
   - Operational percentage
   - Total module count
   - Resource availability (future)
   - Faction ownership

---

## Station Management Widget

### Overview

The `UStationManagementWidget` is a UMG widget that displays and manages station data.

### Adding to Your Game

#### 1. Create Blueprint Widget

1. Create new Widget Blueprint: `WBP_StationManagement`
2. Set Parent Class: `StationManagementWidget`
3. Design your UI layout in UMG

#### 2. Display Widget

```blueprint
Event: Player Docked at Station
├─ Create Widget: WBP_StationManagement
├─ Add to Viewport
└─ Station Management Widget → SetManagedStation
   └─ Station: [The station actor]
```

### Widget Properties

**Managed Station**:
- The space station being managed
- Set via `SetManagedStation` function
- Widget updates automatically

**Selected Module Group**:
- Current filter selection
- EStationModuleGroup enum
- None = show all modules

**Build Mode Active**:
- Whether in build/edit mode
- Enables module placement UI
- Disables normal station view

---

## Module Management

### Viewing Modules

**Get All Modules**:
```blueprint
Function: Refresh Module List
├─ Station Management Widget → GetModulesByGroup
│  └─ Group: SelectedModuleGroup
└─ For Each: Module
   └─ Create Module List Item Widget
      └─ Set Module Data: [Module]
```

**Get Module Count**:
```blueprint
Function: Update Stats
├─ Station Management Widget → GetTotalModuleCount
└─ Set Text: "Modules: {Count}"
```

### Module Groups

Filter modules by type:
- **None**: Show all modules
- **Docking**: Docking bays and hangars
- **Power**: Reactors and power systems
- **Storage**: Cargo and storage facilities
- **Defence**: Weapons and shields
- **LivingQuarters**: Crew habitation
- **Command**: Control centers
- **Industrial**: Manufacturing, refineries
- **Trade**: Trading posts, markets
- **Research**: Labs and research facilities
- **Entertainment**: Recreation areas

### Filtering Modules

```blueprint
Function: On Filter Button Clicked
└─ Station Management Widget → SetSelectedModuleGroup
   └─ Group: [Selected from dropdown]
```

### Operational Status

```blueprint
Function: Update Status Display
├─ Station Management Widget → GetOperationalStatusPercent
└─ Set Progress Bar: Status percent / 100
   └─ Color
      ├─ > 80%: Green
      ├─ > 50%: Yellow
      └─ < 50%: Red
```

---

## Building Modules

### Check if Module Can Be Built

```blueprint
Function: On Module Selected
├─ Station Management Widget → CanBuildModule
│  └─ Module Class: [Selected module]
└─ Branch: Can Build?
   ├─ True: Enable Build Button
   └─ False: Disable Build Button
```

### Request Module Construction

```blueprint
Function: On Build Button Clicked
├─ Station Management Widget → RequestBuildModule
│  └─ Module Class: [Selected module]
└─ Branch: Build Successful?
   ├─ True
   │  └─ Show Message: "Module construction started"
   └─ False
      └─ Show Error: "Cannot build module"
```

**Note**: Module building logic is placeholder. Extend in your project with:
- Resource cost checking
- Attachment point validation
- Construction time
- Required prerequisites

### Removing Modules

```blueprint
Function: On Remove Button Clicked
├─ Get Selected Module
├─ Show Confirmation Dialog
└─ On Confirmed
   └─ Station Management Widget → RequestRemoveModule
      └─ Module: [Selected module]
```

---

## Blueprint Integration

### StationManagementWidget Functions

```cpp
// Station management
void SetManagedStation(ASpaceStation* Station)
ASpaceStation* GetManagedStation()
void RefreshStationData()

// Module queries
TArray<ASpaceStationModule*> GetModulesByGroup(Group)
int32 GetTotalModuleCount()
float GetOperationalStatusPercent() // Returns 0-100

// Module actions
bool CanBuildModule(ModuleClass)
bool RequestBuildModule(ModuleClass)
bool RequestRemoveModule(Module)

// UI state
void SetSelectedModuleGroup(Group)
void ToggleBuildMode()
void CloseInterface()
```

### Events

Implement these in your Blueprint Widget:

```cpp
Event OnStationStatusUpdated()
// Fired when station data changes
// Update UI elements here

Event OnModuleAdded(NewModule)
// Fired when module is added
// Refresh module list

Event OnModuleRemoved(RemovedModule)
// Fired when module is removed
// Refresh module list
```

---

## Best Practices

### UI Design

1. **Clear Layout**
   - Module list on left
   - Details on right
   - Action buttons at bottom
   - Status bar at top

2. **Visual Feedback**
   - Highlight selected module
   - Show operational status with colors
   - Animate transitions
   - Display loading states

3. **Filtering**
   - Default to "All Modules"
   - Quick filter buttons for common types
   - Search functionality (future)
   - Sort options (future)

### Performance

1. **Update Frequency**
   - Widget auto-updates every 2 seconds
   - Manual refresh on user action
   - Throttle updates in large stations

2. **Module List**
   - Use virtual scrolling for many modules
   - Cache module widgets
   - Lazy-load module details

3. **Event Handling**
   - Use events for state changes
   - Avoid polling in Tick
   - Batch UI updates

### User Experience

1. **Information Display**
   - Show most important info first
   - Use icons for quick recognition
   - Color-code by status
   - Provide tooltips

2. **Actions**
   - Confirm destructive actions
   - Show resource costs before building
   - Disable unavailable actions
   - Provide clear error messages

3. **Navigation**
   - Easy to close/exit
   - Keyboard shortcuts (future)
   - Tab between sections
   - Back button for sub-menus

---

## Examples

### Example 1: Basic Station UI

**Widget Structure**:
```
WBP_StationManagement
├─ Top Bar
│  ├─ Station Name
│  ├─ Operational Status Progress Bar
│  └─ Close Button
├─ Left Panel (Module List)
│  ├─ Filter Dropdown
│  └─ Scroll Box
│     └─ Module List Items
├─ Right Panel (Details)
│  ├─ Selected Module Info
│  ├─ Module Stats
│  └─ Action Buttons
└─ Bottom Bar
   ├─ Build Mode Toggle
   └─ Total Module Count
```

**Event Graph**:
```blueprint
Event: Construct
├─ Station Management → OnStationStatusUpdated
│  └─ Bind: Update Status Bar
├─ Station Management → OnModuleAdded
│  └─ Bind: Refresh Module List
└─ Station Management → OnModuleRemoved
   └─ Bind: Refresh Module List

Function: Update Status Bar
├─ GetOperationalStatusPercent
└─ Set Progress Bar Value

Function: Refresh Module List
├─ GetModulesByGroup (SelectedModuleGroup)
├─ Clear Module List Widget
└─ For Each Module
   └─ Create Module List Item
```

### Example 2: Module List Item

**WBP_ModuleListItem**:
```
Canvas Panel
├─ Icon (Module Type)
├─ Name (Text Block)
├─ Status (Text Block with color)
└─ Select Button
```

**Blueprint**:
```blueprint
Function: Set Module Data (Module)
├─ Set Icon: Module→GetModuleTypeIcon()
├─ Set Name: Module→GetModuleName()
└─ Branch: Module→IsOperational()
   ├─ True
   │  ├─ Set Status Text: "Operational"
   │  └─ Set Color: Green
   └─ False
      ├─ Set Status Text: "Offline"
      └─ Set Color: Red

Event: Select Button Clicked
└─ Dispatch: OnModuleSelected(Module)
```

### Example 3: Build Mode UI

**Toggle Build Mode**:
```blueprint
Event: Build Mode Button Clicked
├─ Station Management → ToggleBuildMode
└─ Branch: Build Mode Active?
   ├─ True
   │  ├─ Show: Build Panel
   │  ├─ Hide: Module List
   │  └─ Set Button Text: "Exit Build Mode"
   └─ False
      ├─ Hide: Build Panel
      ├─ Show: Module List
      └─ Set Button Text: "Build Mode"
```

**Build Panel**:
```blueprint
Widget: Build Panel
├─ Module Selection Grid
│  └─ For Each: Available Module Types
│     └─ Create Module Card
│        ├─ Icon
│        ├─ Name
│        ├─ Cost (placeholder)
│        └─ Build Button
└─ Preview Window
   └─ 3D Preview of Selected Module
```

**Building**:
```blueprint
Event: Build Button Clicked (on Module Card)
├─ Get Selected Module Class
├─ CanBuildModule?
└─ Branch
   ├─ True
   │  ├─ RequestBuildModule
   │  └─ Show: "Building started"
   └─ False
      └─ Show Error: "Requirements not met"
```

### Example 4: Module Details Panel

**WBP_ModuleDetails**:
```blueprint
Function: Show Module Details (Module)
├─ Set Text: Module Name
├─ Set Text: Module Description
├─ Set Text: Module Group
├─ Set Progress: Power Generation
├─ Set Progress: Health
└─ Branch: Is Operational?
   ├─ True
   │  └─ Show: Operational Stats
   └─ False
      └─ Show: Repair Options
```

**Stats Display**:
```
Canvas Panel
├─ Health Bar
├─ Power Output (if applicable)
├─ Storage Capacity (if applicable)
├─ Crew Count (if applicable)
└─ Status Icons
```

---

## Station View Integration

### 3D Station View

Show station in 3D while managing:
```blueprint
Function: Focus On Module (Module)
├─ Get Player Camera
├─ Set Camera Location
│  └─ Interpolate to Module Location + Offset
└─ Set Camera Rotation
   └─ Look at Module
```

### Module Highlighting

```blueprint
Event: Module Selected in UI
├─ Clear All Highlights
└─ Branch: Module Valid?
   └─ True
      ├─ Module → Enable Highlight
      └─ Module → Play Pulse Effect
```

---

## Future Enhancements

1. **Resource Management**
   - Display station resources
   - Show production rates
   - Track consumption

2. **Crew Management**
   - Assign crew to modules
   - View crew status
   - Personnel management integration

3. **Power Grid**
   - Visualize power distribution
   - Manage power priorities
   - Monitor power consumption

4. **Trade Integration**
   - View station inventory
   - Manage trade routes
   - Set prices

5. **Defense Systems**
   - Configure weapons
   - Shield management
   - Threat detection

6. **Construction Queue**
   - Queue multiple modules
   - View construction progress
   - Prioritize builds

---

## Troubleshooting

### Widget Not Updating

1. Check SetManagedStation was called
2. Verify station is valid
3. Implement OnStationStatusUpdated event
4. Check timer is running

### Modules Not Showing

1. Verify modules are attached to station
2. Check filter settings
3. Call RefreshStationData manually
4. Test with GetTotalModuleCount

### Build/Remove Not Working

1. Implement logic in RequestBuildModule
2. Check CanBuildModule returns true
3. Verify permissions (if implemented)
4. Test module class is valid

---

## See Also

- [SpaceshipDataAssetGuide.md](SpaceshipDataAssetGuide.md) - For ship integration
- [TradingSystemGuide.md](TradingSystemGuide.md) - For trade station features
- [FactionSetupGuide.md](FactionSetupGuide.md) - For faction-owned stations

---

**Note**: This is an alpha system. Features are intentionally minimal to allow project-specific customization.
