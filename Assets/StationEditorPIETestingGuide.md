# Station Editor PIE Testing Implementation Guide

**Version**: 1.0  
**Last Updated**: December 6, 2025  
**System Status**: Fully Implemented in C++, Ready for Blueprint Integration

---

## Table of Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Part 1: Blueprint Widget Setup](#part-1-blueprint-widget-setup)
- [Part 2: Input Binding Configuration](#part-2-input-binding-configuration)
- [Part 3: Game Integration](#part-3-game-integration)
- [Part 4: Testing in PIE](#part-4-testing-in-pie)
- [Advanced Features](#advanced-features)
- [Troubleshooting](#troubleshooting)
- [Reference](#reference)

---

## Overview

This guide provides step-by-step instructions for implementing and testing the Station Editor system in PIE (Play In Editor). The Station Editor allows players to construct and modify space stations by placing, removing, and connecting modules.

### What You'll Build

- A runtime-accessible station editor UI
- Key binding to toggle the editor (e.g., F9)
- Full module placement, removal, and management
- Undo/redo functionality
- Construction queue system
- Real-time statistics and validation

### System Architecture

```
┌─────────────────────────────────────────────────┐
│         Player Input (F9 Key Press)             │
└──────────────────┬──────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────┐
│      BP_SpaceshipController (or Game HUD)       │
│  - Creates WBP_StationEditor widget             │
│  - Initializes UStationEditorManager            │
│  - Sets up catalog and configuration            │
└──────────────────┬──────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────┐
│           WBP_StationEditor (UI Widget)         │
│  - Module selection grid                        │
│  - Station statistics display                   │
│  - Construction queue panel                     │
│  - Connection visualization                     │
└──────────────────┬──────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────┐
│      UStationEditorManager (C++ Manager)        │
│  - Module placement logic                       │
│  - Validation and collision checking            │
│  - Power balance calculation                    │
│  - Undo/redo system                             │
└──────────────────┬──────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────┐
│           ASpaceStation (Game World)            │
│  - Actual station with modules                  │
│  - Visual representation                        │
│  - Collision and physics                        │
└─────────────────────────────────────────────────┘
```

---

## Prerequisites

Before starting, ensure you have:

✅ **Unreal Engine 5.6** installed  
✅ **Adastrea project** opened in the editor  
✅ **C++ code compiled** successfully  
✅ Basic familiarity with **UMG (Unreal Motion Graphics)**  
✅ Understanding of **Enhanced Input System**  

### Required C++ Classes (Already Implemented)

These classes are already in the project:

- `UStationEditorManager` - Core editor manager
- `UStationModuleCatalog` - Module catalog data asset
- `UStationEditorWidget` - Base widget class with C++ hooks
- `AStationBuildPreview` - Preview actor for placement
- `UStationGridSystem` - Grid snapping system
- `ASpaceStation` - Station actor
- `ASpaceStationModule` - Base module class

### Documentation References

- [StationEditorSystemGuide.md](StationEditorSystemGuide.md) - Complete C++ API reference
- [StationManagementGuide.md](StationManagementGuide.md) - Station management patterns
- [PIE_TESTING_GUIDE.md](PIE_TESTING_GUIDE.md) - General PIE testing practices

---

## Quick Start

**TL;DR - Get the editor running in 15 minutes:**

1. Create `DA_StationModuleCatalog` Data Asset
2. Create `WBP_StationEditor` Widget Blueprint
3. Add input action `IA_OpenStationEditor`
4. Bind key in `IMC_Spaceship` (e.g., F9)
5. Add toggle logic to Player Controller
6. Press Play, approach station, press F9

---

## Part 1: Blueprint Widget Setup

### Step 1.1: Create Module Catalog Data Asset

The catalog defines which modules can be built.

1. **Create the Data Asset:**
   - Right-click in Content Browser
   - Navigate to: `Content/DataAssets/Stations/`
   - Create folder if it doesn't exist
   - Select **Miscellaneous → Data Asset**
   - Choose **StationModuleCatalog** as the class
   - Name it: `DA_StationModuleCatalog`

2. **Populate Module Entries:**
   Open `DA_StationModuleCatalog` and add entries:

   ```
   Module Entry 0:
   ├─ ModuleClass: BP_ReactorModule
   ├─ DisplayName: "Reactor Core"
   ├─ Description: "Generates power for the station"
   ├─ RequiredTechLevel: 1
   ├─ ModuleGroup: Power
   ├─ BuildCost:
   │  ├─ Credits: 5000
   │  ├─ BuildTime: 60.0
   │  └─ Materials: (optional)
   └─ PreviewMesh: SM_Reactor_Preview
   
   Module Entry 1:
   ├─ ModuleClass: BP_DockingBay
   ├─ DisplayName: "Docking Bay"
   ├─ Description: "Allows ships to dock"
   ├─ RequiredTechLevel: 1
   ├─ ModuleGroup: Docking
   └─ ... (configure similarly)
   ```

   **Tip:** Start with 3-5 basic modules for testing.

3. **Save the catalog**

### Step 1.2: Create Station Editor Widget Blueprint

1. **Create Widget Blueprint:**
   - Content Browser → `Content/UI/Station/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_StationEditor`
   - Parent Class: `StationEditorWidget` (C++ class)

2. **Design the UI Layout:**

   Open `WBP_StationEditor` in the UMG Designer:

   ```
   Canvas Panel (Root)
   ├─ Border (Background - Fullscreen)
   │  └─ Overlay
   │     ├─ Image (Semi-transparent dark background)
   │     └─ Horizontal Box (Main Layout)
   │        ├─ Left Panel (Module List) - 280px
   │        │  ├─ Text Block: "Available Modules"
   │        │  └─ Scroll Box: ModuleListScrollBox
   │        │     └─ (Populated at runtime)
   │        │
   │        ├─ Center Panel (3D Viewport Area) - Fill
   │        │  ├─ Text Block: "Station Editor Active"
   │        │  └─ (Game viewport visible through transparency)
   │        │
   │        └─ Right Panel (Statistics) - 320px
   │           ├─ Vertical Box
   │           │  ├─ Text Block: "Station Statistics"
   │           │  ├─ Text Block: PowerDisplay (bind to function)
   │           │  ├─ Text Block: ModuleCountDisplay
   │           │  ├─ Progress Bar: PowerBalance
   │           │  └─ Button: "Close Editor"
   │           └─ Vertical Box (Construction Queue)
   │              └─ (Populated at runtime)
   ```

3. **Create Essential Widgets:**

   **Module List Item** (`WBP_ModuleListItem`):
   ```
   Border (150x80)
   └─ Vertical Box
      ├─ Text Block: ModuleName
      ├─ Text Block: Cost
      └─ Button: "Build"
   ```

   **Construction Queue Item** (`WBP_ConstructionQueueItem`):
   ```
   Border
   └─ Horizontal Box
      ├─ Text Block: ModuleName
      ├─ Progress Bar: BuildProgress
      └─ Button: "Cancel"
   ```

### Step 1.3: Configure Widget Blueprint Variables

In `WBP_StationEditor`, add these variables:

```
Variables:
├─ EditorManager: UStationEditorManager (Instance Editable)
├─ CurrentStation: ASpaceStation (Instance Editable)
├─ ModuleCatalog: UStationModuleCatalog (Instance Editable, default: DA_StationModuleCatalog)
├─ ModuleListScrollBox: UScrollBox (Bind to widget)
├─ PowerDisplayText: UTextBlock (Bind to widget)
├─ CloseButton: UButton (Bind to widget)
└─ bIsEditorOpen: Boolean (default: false)
```

### Step 1.4: Implement Widget Event Graph

#### Event Construct

```blueprint
Event Construct
├─ EnsureEditorManager
│  └─ If EditorManager is null
│     └─ Create EditorManager Object
│        ├─ Outer: Self
│        └─ Class: StationEditorManager
├─ Set EditorManager Properties
│  ├─ ModuleCatalog = ModuleCatalog variable
│  ├─ PlayerTechLevel = 5 (for testing)
│  └─ PlayerCredits = 100000 (for testing)
└─ RefreshModuleList
```

#### Function: RefreshModuleList

```blueprint
Function: RefreshModuleList
├─ Clear ModuleListScrollBox children
├─ Get Available Modules (from EditorManager)
└─ For Each Module Entry
   ├─ Create WBP_ModuleListItem
   ├─ Set Module Data
   │  ├─ Module Name
   │  ├─ Build Cost
   │  └─ Preview Image
   ├─ Bind Build Button
   │  └─ On Clicked → PlaceModuleAtCursor
   └─ Add to ScrollBox
```

#### Function: PlaceModuleAtCursor

```blueprint
Function: PlaceModuleAtCursor (ModuleClass)
├─ Get Player Controller
├─ Get Mouse Position in World
│  └─ DeprojectScreenToWorld
├─ Line Trace for placement position
│  └─ From: Camera Location
│  └─ To: Mouse direction * 10000
├─ If Hit Station or Near Station
│  ├─ EditorManager → PlaceModule
│  │  ├─ ModuleClass
│  │  ├─ Hit Location
│  │  └─ Default Rotation
│  └─ RefreshStatistics
└─ Else
   └─ Show Error: "Too far from station"
```

#### Function: RefreshStatistics

```blueprint
Function: RefreshStatistics
├─ Get Station Statistics (from EditorManager)
├─ Update PowerDisplayText
│  └─ Format: "Power: {Generated} / {Consumed} MW"
├─ Update Module Count
│  └─ Format: "Modules: {Current} / {Max}"
└─ Update PowerBalance Progress Bar
   └─ Percent = PowerGenerated / (PowerGenerated + PowerConsumed)
```

#### Event: Close Button Clicked

```blueprint
Event: CloseButton Clicked
├─ If EditorManager is valid
│  └─ EditorManager → Save
├─ Remove from Parent
└─ Set Input Mode Game Only
```

---

## Part 2: Input Binding Configuration

### Step 2.1: Create Input Action

1. Navigate to `Content/Input/Actions/`
2. Right-click → Input → Input Action
3. Name: `IA_OpenStationEditor`
4. Configure:
   - Value Type: **Digital (bool)**
   - Trigger: **Pressed** (not Down)
   - Consume Input: ✅ Checked

### Step 2.2: Add to Input Mapping Context

1. Open `Content/Input/IMC_Spaceship`
2. Add new mapping:
   ```
   Mappings:
   ├─ ... (existing mappings)
   └─ IA_OpenStationEditor
      └─ Key: F9
      └─ Modifiers: (none)
      └─ Triggers: Pressed
   ```
3. Save

### Step 2.3: Update Input Config Data Asset

1. Open `Content/DataAssets/Input/DA_InputConfig`
2. Add new property:
   ```
   Properties:
   ├─ ... (existing actions)
   └─ OpenStationEditorAction: IA_OpenStationEditor
   ```
3. Save

---

## Part 3: Game Integration

### Step 3.1: Add to Player Controller

Open `BP_SpaceshipController` (or your game's player controller):

#### Add Variables

```
Variables:
├─ StationEditorWidget: WBP_StationEditor (null by default)
├─ bStationEditorOpen: Boolean (default: false)
├─ NearbyStation: ASpaceStation (null by default)
└─ StationSearchRadius: Float (default: 5000.0)
```

#### Implement Toggle Function

```blueprint
Function: ToggleStationEditor
├─ Branch: bStationEditorOpen?
│  ├─ TRUE: Close Editor
│  │  ├─ If StationEditorWidget is valid
│  │  │  ├─ StationEditorWidget → Remove from Parent
│  │  │  └─ Set StationEditorWidget = null
│  │  ├─ Set Input Mode: Game Only
│  │  └─ Set bStationEditorOpen = false
│  │
│  └─ FALSE: Open Editor
│     ├─ Find Nearby Station
│     │  └─ GetActorsInRadius (center: player pawn, radius: StationSearchRadius)
│     │     └─ Filter by ASpaceStation class
│     ├─ If Station Found
│     │  ├─ Create WBP_StationEditor Widget
│     │  ├─ Set CurrentStation = Found Station
│     │  ├─ EditorManager → BeginEditing(Station)
│     │  ├─ Add to Viewport (ZOrder: 10)
│     │  ├─ Set Input Mode: UI Only
│     │  │  └─ Widget to Focus: StationEditorWidget
│     │  └─ Set bStationEditorOpen = true
│     └─ Else
│        └─ Show Message: "No station nearby"
```

#### Bind Input Action

In Event Graph:

```blueprint
Event: Enhanced Input Action (IA_OpenStationEditor)
└─ ToggleStationEditor
```

**Alternative Setup (if using DA_InputConfig):**

```blueprint
Event BeginPlay
├─ Parent: BeginPlay
├─ Get Enhanced Input Subsystem
├─ Add Mapping Context (IMC_Spaceship)
└─ Bind Action (IA_OpenStationEditor)
   └─ Triggered → ToggleStationEditor
```

### Step 3.2: Add Station Detection Helper

```blueprint
Function: FindNearestStation → ASpaceStation
├─ Get All Actors of Class (ASpaceStation)
├─ Filter by Distance
│  └─ Distance < StationSearchRadius
├─ Sort by Distance
└─ Return: Closest Station (or null)
```

---

## Part 4: Testing in PIE

### Step 4.1: Setup Test Level

1. **Create or Open Test Level:**
   - Use existing `TestLevel` or create new
   - File → New Level → Empty Level
   - Add: Player Start, Directional Light, Sky Atmosphere

2. **Place a Station:**
   - Drag `BP_SpaceStation` (or a station blueprint) into level
   - Position: Near player start (within 5000 units)
   - Configure station:
     - Add a few starting modules if desired
     - Set faction (optional)

3. **Configure Game Mode:**
   - World Settings → Game Mode Override: `BP_SpaceGameMode`
   - Default Pawn Class: `BP_PlayerShip`
   - Player Controller Class: `BP_SpaceshipController`

4. **Save Level**

### Step 4.2: Initial PIE Test

1. **Start PIE** (Alt+P)
2. **Expected behavior:**
   - Player spawns in ship
   - Station is visible nearby
   - Controls work (WASD, mouse)

3. **Approach Station:**
   - Fly toward the station
   - Get within 5000 units

4. **Open Station Editor:**
   - Press **F9**
   - Expected: UI appears with module list and statistics

5. **Test Module Placement:**
   - Click a module from the list
   - UI should show placement preview (if preview system is active)
   - Click in 3D space to place
   - Module appears on station

6. **Test Statistics:**
   - Power balance updates
   - Module count increases

7. **Close Editor:**
   - Click "Close" button
   - UI disappears
   - Input returns to ship control

### Step 4.3: Advanced Testing

#### Test Undo/Redo

```blueprint
Add Undo/Redo Buttons to WBP_StationEditor:
├─ Button: Undo (Ctrl+Z)
│  └─ On Clicked → EditorManager → Undo()
└─ Button: Redo (Ctrl+Y)
   └─ On Clicked → EditorManager → Redo()

Test:
1. Place several modules
2. Click Undo - last module removed
3. Click Redo - module returns
```

#### Test Construction Queue

```blueprint
Add Queue Panel:
├─ Text Block: "Construction Queue"
└─ Scroll Box: QueueListScrollBox

Function: RefreshConstructionQueue
├─ Clear QueueListScrollBox
├─ Get Construction Queue (from EditorManager)
└─ For Each Queue Item
   ├─ Create WBP_ConstructionQueueItem
   ├─ Set Item Data
   │  ├─ Module Name
   │  ├─ Progress (TimeRemaining / TotalBuildTime)
   │  └─ Bind Cancel Button
   └─ Add to ScrollBox

Test:
1. Queue multiple modules
2. Watch progress bars update
3. Test cancel functionality
```

#### Test Power Validation

```blueprint
Add validation feedback:
├─ Before placing module
│  └─ EditorManager → WouldCausePowerDeficit(ModuleClass)
├─ If deficit detected
│  ├─ Show warning icon
│  └─ Optionally disable placement
└─ Test by placing many power-consuming modules

Test:
1. Place reactor (power generation)
2. Place many power-consuming modules
3. Verify warning when power would be exceeded
```

#### Test Module Connections

```blueprint
Add Connections Visualization:
├─ Get All Connections (from EditorManager)
└─ For Each Connection
   ├─ Draw Debug Line (or use Widget Component)
   │  └─ From: ModuleA Location
   │  └─ To: ModuleB Location
   └─ Color by ConnectionType
      ├─ Power: Yellow
      ├─ Data: Blue
      └─ LifeSupport: Green

Test:
1. Place adjacent modules
2. EditorManager → AutoGenerateConnections
3. Verify lines appear between modules
```

### Step 4.4: Performance Testing

1. **Test with Many Modules:**
   - Place 20-50 modules
   - Check frame rate
   - Monitor statistics refresh rate

2. **Stress Test:**
   - Rapid placement and removal
   - Quick undo/redo actions
   - Check for memory leaks

3. **UI Responsiveness:**
   - Scroll through long module lists
   - Toggle view modes quickly
   - Check notification system with many messages

---

## Advanced Features

### Feature 1: Module Preview System

Show a preview of where the module will be placed.

```blueprint
Function: UpdatePlacementPreview
├─ Get Mouse World Position (line trace)
├─ If SelectedModuleClass is valid
│  ├─ EditorManager → ShowPreview(SelectedModuleClass)
│  ├─ EditorManager → UpdatePreview(HitLocation, DefaultRotation)
│  └─ Check Placement Validity
│     ├─ EditorManager → CanPlaceModule
│     ├─ If valid → Preview color: Green
│     └─ If invalid → Preview color: Red
└─ Else
   └─ EditorManager → HidePreview()

Call this function:
- On Tick (or Timer every 0.1s)
- Only when editor is open and module selected
```

### Feature 2: Camera Control

Allow players to rotate around station while editing.

```blueprint
Add Camera Controls:
├─ Middle Mouse Button: Rotate camera
├─ Mouse Wheel: Zoom in/out
└─ Right Mouse Button: Pan camera

Implementation:
1. Store camera pivot point (station center)
2. On mouse input:
   ├─ Rotate pivot
   ├─ Update camera position
   └─ Maintain distance from pivot
3. Reset camera when editor closes
```

### Feature 3: Module Search and Filters

```blueprint
Add Search Box:
├─ Editable Text: SearchBox
└─ On Text Changed
   ├─ Filter module list by name
   └─ RefreshModuleList (with filter)

Add Filter Buttons:
├─ Button: All Modules
├─ Button: Power Modules
├─ Button: Docking Modules
├─ Button: Storage Modules
└─ Button: Defense Modules

Each button:
└─ Set ModuleGroupFilter
   └─ RefreshModuleList (with filter)
```

### Feature 4: Hotkeys

```blueprint
Add Hotkey Support:
├─ Ctrl+Z: Undo
├─ Ctrl+Y: Redo
├─ Delete: Remove selected module
├─ R: Rotate selected module
├─ G: Toggle grid snapping
└─ Esc: Close editor

Implementation in Widget:
Override OnKeyDown event
├─ Switch on Key
│  ├─ Z (with Ctrl): Call Undo
│  ├─ Y (with Ctrl): Call Redo
│  ├─ Delete: Remove selected
│  └─ Esc: Close editor
└─ Return: Handled
```

### Feature 5: Notifications System

Display editor messages to the player.

```blueprint
Add Notification Panel:
├─ Vertical Box: NotificationList (bottom-right)
└─ Function: ShowNotification(Message, Severity)
   ├─ Create notification widget
   │  ├─ Text: Message
   │  ├─ Color by severity
   │  │  ├─ Info: Blue
   │  │  ├─ Warning: Yellow
   │  │  ├─ Error: Red
   │  │  └─ Success: Green
   │  └─ Auto-dismiss after 5 seconds
   └─ Add to NotificationList

Bind to EditorManager events:
├─ OnModulePlaced → "Module placed successfully"
├─ OnModuleRemoved → "Module removed"
├─ OnPowerBalanceChanged → "Power balance updated"
└─ On validation failure → Show error reason
```

### Feature 6: Save/Load Station Layouts

```blueprint
Add Save/Load Buttons:
├─ Button: "Save Layout"
│  └─ Save station module configuration to file
└─ Button: "Load Layout"
   └─ Load and apply saved configuration

Implementation:
├─ Serialize station data
│  └─ Array of: ModuleClass, Position, Rotation
├─ Save to JSON or Blueprint SaveGame
└─ Load and reconstruct on demand
```

---

## Troubleshooting

### Issue: Widget Not Appearing

**Symptoms:** F9 pressed, nothing happens

**Solutions:**
1. Check Output Log for errors
2. Verify Input Action is bound correctly
3. Check widget is being created (add Print String)
4. Ensure Input Mode is set to UI Only
5. Check widget Z-order (should be > 0)

**Debug:**
```blueprint
In ToggleStationEditor, add:
├─ Print String: "Toggle called"
├─ After Create Widget: Print: "Widget created"
└─ After Add to Viewport: Print: "Widget added"
```

### Issue: No Nearby Station Found

**Symptoms:** Message "No station nearby" always shows

**Solutions:**
1. Verify station is placed in level
2. Check search radius (increase to 10000 for testing)
3. Ensure station class matches (ASpaceStation)
4. Add debug sphere to visualize radius

**Debug:**
```blueprint
In FindNearestStation:
├─ Draw Debug Sphere (Radius: StationSearchRadius)
└─ Print: "Stations found: {Count}"
```

### Issue: Modules Not Placing

**Symptoms:** Click on module, nothing spawns

**Solutions:**
1. Check EditorManager is valid
2. Verify BeginEditing was called
3. Check CanPlaceModule result
4. Verify module class is valid
5. Check collision settings

**Debug:**
```blueprint
In PlaceModuleAtCursor:
├─ Print: "Attempting to place: {ModuleClass}"
├─ Check CanPlaceModule result
│  └─ Print result enum value
└─ After PlaceModule call
   ├─ Print: "Module placed: {Module is valid?}"
   └─ If null, print error from EditorManager
```

### Issue: Statistics Not Updating

**Symptoms:** Power balance, module count don't change

**Solutions:**
1. Ensure RefreshStatistics is called after placement
2. Bind to EditorManager events
3. Check EditorManager → GetStationStatistics returns valid data
4. Verify station has modules array populated

**Debug:**
```blueprint
In RefreshStatistics:
├─ Print: "Refreshing stats"
├─ Get Statistics
│  └─ Print: "Power: {PowerGenerated} / {PowerConsumed}"
│  └─ Print: "Modules: {TotalModules}"
└─ If all zeros, check station data
```

### Issue: Editor Doesn't Close

**Symptoms:** UI stuck on screen, input not responding

**Solutions:**
1. Verify Remove from Parent is called
2. Check Set Input Mode is called
3. Ensure widget reference is cleared
4. Try ESC key as alternative close method

**Debug:**
```blueprint
In Close function:
├─ Print: "Closing editor"
├─ After Remove from Parent: Print: "Removed"
└─ After Input Mode: Print: "Input restored"
```

### Issue: Preview Not Showing

**Symptoms:** No visual feedback when selecting modules

**Solutions:**
1. Check AStationBuildPreview spawns correctly
2. Verify EditorManager → ShowPreview is called
3. Check preview materials are assigned
4. Ensure preview actor is not hidden

**Debug:**
```blueprint
After ShowPreview:
├─ Print: "Preview shown for: {ModuleClass}"
├─ Check PreviewActor is valid
└─ Check PreviewActor visibility
```

### Issue: Performance Lag

**Symptoms:** Editor feels slow or choppy

**Solutions:**
1. Reduce statistics refresh frequency (use Timer, not Tick)
2. Limit module list to visible items only
3. Use object pooling for list items
4. Disable unnecessary visual effects
5. Check for expensive Blueprint operations in Tick

**Optimization:**
```blueprint
Instead of Tick:
├─ Use Timer (0.5 second interval)
│  └─ RefreshStatistics
└─ Only refresh on events when possible
   └─ Bind to OnModulePlaced, OnModuleRemoved
```

---

## Reference

### Key C++ Classes

#### UStationEditorManager

Primary C++ manager class providing all editing functionality.

**Key Functions:**
- `BeginEditing(Station)` - Start editing session
- `PlaceModule(Class, Position, Rotation)` - Place a module
- `RemoveModule(Module)` - Remove a module
- `CanPlaceModule(Class, Position, Rotation)` - Validate placement
- `GetStationStatistics()` - Get current stats
- `Undo()` / `Redo()` - Undo/redo operations
- `Save()` / `Cancel()` - End editing

**Key Events:**
- `OnModulePlaced` - Module successfully placed
- `OnModuleRemoved` - Module removed
- `OnPowerBalanceChanged` - Power changed
- `OnStatisticsUpdated` - Stats recalculated

#### UStationModuleCatalog

Data Asset containing available modules.

**Key Functions:**
- `GetAvailableModules()` - Get all modules
- `GetModulesByGroup(Group)` - Filter by group
- `GetModulesByTechLevel(Level)` - Filter by tech level
- `GetModuleBuildCost(Class)` - Get build cost

#### UStationEditorWidget

Base C++ widget class with Blueprint integration points.

**Inherited Functions:**
- `AddModule(Class, Location)` - Add module
- `RemoveModule(Module)` - Remove module
- `GetAllModules()` - Get station modules
- All manager pass-through functions

### Enums

#### EModulePlacementResult
- `Success` - Module placed successfully
- `InvalidModule` - Module class is null or invalid
- `InvalidPosition` - Position is not valid
- `CollisionDetected` - Collision with existing module
- `InsufficientPower` - Not enough power available
- `InsufficientTech` - Tech level too low
- `InsufficientFunds` - Not enough credits
- `NoStation` - No station selected
- `NotEditing` - Not in edit mode

#### EStationModuleGroup
- `All` - All modules (catch-all, used for filtering)
- `Docking` - Docking bays
- `Power` - Reactors and power
- `Storage` - Cargo storage
- `Processing` - Manufacturing and processing
- `Defence` - Weapons and shields
- `Habitation` - Living quarters and crew spaces
- `Public` - Public/trade areas (markets, commerce)
- `Connection` - Connectors, corridors, links
- `Other` - Miscellaneous/uncategorized modules

#### EStationEditorViewMode
- `Edit` - Standard editing mode
- `Preview` - Preview-only mode
- `Connections` - Connection visualization mode

### Structs

#### FStationStatistics
```cpp
int32 TotalModules
int32 MaxModules
int32 CurrentPopulation
int32 MaxPopulation
float PowerGenerated
float PowerConsumed
float DefenseRating
float EfficiencyRating
float CargoCapacity
float CargoUsed
```

#### FConstructionQueueItem
```cpp
int32 QueueId
TSubclassOf<ASpaceStationModule> ModuleClass
FVector TargetPosition
FRotator TargetRotation
float TotalBuildTime
float TimeRemaining
bool bIsBuilding
bool bIsPaused
```

#### FModuleConnection
```cpp
ASpaceStationModule* ModuleA
ASpaceStationModule* ModuleB
EModuleConnectionType ConnectionType
bool bIsActive
float Capacity
float CurrentUsage
```

### HTML Mockups

Visual design references:
- `STATION_EDITOR_MOCKUP.html` - Main editor UI mockup
- `STATION_EDITOR_CONNECTIONS_MOCKUP.html` - Connections view mockup

Open these in a browser to see intended visual design.

### Configuration Files

**Module Catalog Example:**
```
Content/DataAssets/Stations/DA_StationModuleCatalog
├─ Entry 0: Reactor Module
│  ├─ Class: BP_ReactorModule
│  ├─ Tech: 1, Cost: 5000, Time: 60s
│  └─ Group: Power
├─ Entry 1: Docking Bay
│  └─ ... (similar structure)
└─ Entry N: ...
```

**Input Configuration:**
```
Content/Input/IMC_Spaceship
├─ ... (existing mappings)
└─ IA_OpenStationEditor → F9
```

### Testing Checklist

Use this checklist for thorough PIE testing:

- [ ] Widget appears when F9 pressed
- [ ] Widget disappears when closed
- [ ] Module list populates correctly
- [ ] Modules can be selected
- [ ] Modules can be placed
- [ ] Modules appear in 3D world
- [ ] Statistics update correctly
- [ ] Power balance calculated
- [ ] Module count accurate
- [ ] Undo removes last module
- [ ] Redo restores module
- [ ] Construction queue works
- [ ] Queue progress updates
- [ ] Can cancel queued items
- [ ] Placement validation works
- [ ] Collision detection works
- [ ] Tech level filtering works
- [ ] Cost checking works
- [ ] Preview system works (if implemented)
- [ ] Camera controls work (if implemented)
- [ ] Hotkeys work (if implemented)
- [ ] Notifications appear (if implemented)
- [ ] No performance issues
- [ ] No memory leaks
- [ ] No crash scenarios

---

## Next Steps

After completing PIE testing:

1. **Polish the UI:**
   - Improve visual design
   - Add animations
   - Enhance user feedback

2. **Add Advanced Features:**
   - Module rotation control
   - Module upgrading
   - Module selling/scrapping
   - Station templates

3. **Integration:**
   - Connect to trading system
   - Add faction restrictions
   - Integrate with crew management
   - Add quest triggers

4. **Multiplayer Support:**
   - Server-side validation
   - Replication setup
   - Permission system
   - Conflict resolution

5. **Production Readiness:**
   - Localization
   - Accessibility
   - Tutorial integration
   - Help system

---

## Related Documentation

- [StationEditorSystemGuide.md](StationEditorSystemGuide.md) - Complete C++ API
- [StationManagementGuide.md](StationManagementGuide.md) - Station management patterns
- [PIE_TESTING_GUIDE.md](PIE_TESTING_GUIDE.md) - General PIE testing
- [BLUEPRINT_QUICK_START_PIE.md](../BLUEPRINT_QUICK_START_PIE.md) - Blueprint PIE basics
- [SpaceStationModule System](../Source/Adastrea/Stations/) - Station C++ source

---

**Version History:**
- **1.0** (2025-12-06): Initial comprehensive PIE testing guide

**Maintained by**: Adastrea Development Team  
**Last Updated**: December 6, 2025

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────┐
│             STATION EDITOR QUICK REFERENCE              │
├─────────────────────────────────────────────────────────┤
│ Key Binding:         F9 (toggle editor)                 │
│ Close Editor:        ESC or Close button                │
│ Place Module:        Click module, click in 3D space    │
│ Remove Module:       Select module, Delete key          │
│ Undo:               Ctrl+Z                              │
│ Redo:               Ctrl+Y                              │
│ Rotate Module:       R key (if implemented)             │
│ Toggle Grid:         G key (if implemented)             │
├─────────────────────────────────────────────────────────┤
│ Required Assets:                                        │
│ ├─ DA_StationModuleCatalog (Data Asset)                │
│ ├─ WBP_StationEditor (Widget Blueprint)                │
│ ├─ IA_OpenStationEditor (Input Action)                 │
│ └─ Module Blueprints (BP_ReactorModule, etc.)          │
├─────────────────────────────────────────────────────────┤
│ Common Issues:                                          │
│ ├─ Widget not appearing → Check Input binding          │
│ ├─ No station found → Increase search radius           │
│ ├─ Modules not placing → Check collision/validation    │
│ └─ Stats not updating → Call RefreshStatistics         │
└─────────────────────────────────────────────────────────┘
```

---

**End of Guide**
