# Blueprint UI Workflow Templates and Sample Graphs

## Overview
This document provides detailed Blueprint workflow templates for implementing the Station Editor UI and Faction Selection system. Each section includes step-by-step node graphs and best practices.

---

## Table of Contents
1. [Station Editor Widget Setup](#station-editor-widget-setup)
2. [Module Addition Workflow](#module-addition-workflow)
3. [Module Removal Workflow](#module-removal-workflow)
4. [Faction Selection UI](#faction-selection-ui)
5. [Module List Display](#module-list-display)
6. [Technology Level Filtering](#technology-level-filtering)
7. [Visual Feedback Systems](#visual-feedback-systems)

---

## Station Editor Widget Setup

### Creating the Widget Blueprint

**Steps:**
1. **Create Widget Blueprint**
   - Right-click in Content Browser
   - Select **User Interface → Widget Blueprint**
   - Name it `WBP_StationEditor`
   - Open the widget

2. **Set Parent Class**
   - In **File → Reparent Blueprint**
   - Search for and select **StationEditorWidget** (from StationEditor module)
   - This gives you access to all C++ functions

3. **Design UI Layout**
   - Add a **Canvas Panel** as root
   - Typical layout sections:
     - **Left Panel**: Module palette/selection
     - **Center**: 3D viewport or station visualization
     - **Right Panel**: Properties and module list
     - **Top Bar**: Faction selector
     - **Bottom Bar**: Action buttons (Save, Cancel)

### Initialize Station Reference

**Event Graph: Construct Event**
```
Event Construct
  ↓
Get Player Controller
  ↓
Cast to [YourPlayerControllerClass]
  ↓
Get Current Station (custom function in your game)
  ↓
Set Station (inherited from UStationEditorWidget)
  ↓
Refresh UI (custom event)
```

**Node Details:**
- `Event Construct`: Called when widget is created
- `Set Station`: C++ function from UStationEditorWidget (StationEditor module)
- `Refresh UI`: Custom event to update all UI elements

---

## Module Addition Workflow

### Basic Add Module Flow

**Blueprint Graph:**
```
[Button_AddModule] On Clicked
  ↓
Get Selected Module Class (from UI variable)
  ↓
Branch (Is Valid?)
  ├─ False → Show Error Message ("Please select a module type")
  └─ True → Continue
       ↓
     Get Placement Location (from UI or mouse position)
       ↓
     Is Valid Placement (inherited C++ function)
       ├─ False → Show Error Message ("Invalid placement location")
       └─ True → Continue
            ↓
          Add Module (inherited C++ function)
            ↓
          Branch (Is Valid Module Reference?)
            ├─ False → Show Error Message ("Failed to create module")
            └─ True → Success Path
                 ↓
               Update Module List UI
                 ↓
               Play Success Sound/Animation
                 ↓
               Clear Selection
```

### Advanced Add Module with Faction Validation

**Blueprint Graph:**
```
[Button_AddModule] On Clicked
  ↓
Get Selected Module Class
  ↓
Can Add Module For Faction (inherited C++ function)
  ├─ False → Branch Path
  │    ↓
  │  Get Current Station
  │    ↓
  │  Get Faction
  │    ↓
  │  Get Technology Level
  │    ↓
  │  Format Text: "Requires Tech Level {0}"
  │    ↓
  │  Show Error Dialog
  │
  └─ True → Continue with Add Module Flow (from above)
```

**Key Nodes:**
- **Can Add Module For Faction**: C++ function that checks tech requirements
- **Format Text**: Creates dynamic error messages
- **Show Error Dialog**: Custom UI popup function

### Sample Implementation Details

**Variable Setup:**
```
Variables in WBP_StationEditor:
- SelectedModuleClass (Type: Class Reference to SpaceStationModule)
- PlacementLocation (Type: Vector)
- CurrentPreviewModule (Type: SpaceStationModule Reference)
```

**Helper Function: Get Placement Location**
```
Function: GetPlacementLocation
Returns: Vector

Graph:
  Get Mouse Position 2D
    ↓
  Convert Screen to World Location
    ↓
  Project to Station Grid (custom snap function)
    ↓
  Return Value
```

---

## Module Removal Workflow

### Basic Remove Module Flow

**Blueprint Graph:**
```
[Button_RemoveModule] On Clicked
  ↓
Get Selected Module (from list UI variable)
  ↓
Branch (Is Valid?)
  ├─ False → Show Error Message ("No module selected")
  └─ True → Continue
       ↓
     Show Confirmation Dialog ("Remove this module?")
       ├─ User Selected: Cancel → End
       └─ User Selected: Confirm
            ↓
          Remove Module (inherited C++ function)
            ↓
          Branch (Success?)
            ├─ False → Show Error Message ("Failed to remove module")
            └─ True → Success Path
                 ↓
               Update Module List UI
                 ↓
               Clear Selection
                 ↓
               Play Success Animation
```

### Module List Item Click Handler

**Blueprint Graph:**
```
[ListItem_Module] On Clicked (Custom Event in list item widget)
  ↓
Get Module Reference (from list item data)
  ↓
Set Selected Module (variable in main widget)
  ↓
Update Remove Button State
  ├─ Enable if valid module
  └─ Disable if null
  ↓
Highlight Module in 3D View (custom visualization)
```

---

## Faction Selection UI

### Dropdown/Combo Box Implementation

**Widget Setup:**
1. Add a **Combo Box (String)** to your UI
2. Name it: `ComboBox_FactionSelector`

**Event Graph: Populate Faction List**
```
Event Construct
  ↓
Get All Faction Data Assets (custom function)
  ↓
ForEach Loop
  ├─ Loop Body
  │    ↓
  │  Get Faction Name (from Faction Data Asset)
  │    ↓
  │  Convert Text to String
  │    ↓
  │  Add Option (to ComboBox_FactionSelector)
  │
  └─ Completed
       ↓
     Set Selected Index (to current faction)
```

**Event Graph: On Faction Selected**
```
[ComboBox_FactionSelector] On Selection Changed
  ↓
Get Selected Option
  ↓
Find Faction Data Asset by Name (custom function)
  ↓
Branch (Is Valid?)
  ├─ False → Reset to previous faction
  └─ True → Continue
       ↓
     Set Station Faction (inherited C++ function)
       ↓
     Update Faction Info Display
       ├─ Set Faction Name Text
       ├─ Set Faction Description Text
       ├─ Set Primary Color (for UI elements)
       └─ Set Secondary Color (for UI elements)
       ↓
     Update Available Modules List
       ↓
     Refresh Module Palette UI
```

### Faction Info Panel

**Widget Components:**
```
[Text_FactionName] (Text Block)
  - Binding: Get Current Station → Get Faction → Get Faction Name

[Text_FactionDescription] (Text Block - Multi-line)
  - Binding: Get Current Station → Get Faction → Description

[ColorBlock_Primary] (Image widget)
  - Binding: Get Current Station → Get Faction → Primary Color → Set Color and Opacity

[Text_TechLevel] (Text Block)
  - Binding: Get Current Station → Get Faction → Technology Level → Format as "Tech Level: {0}"

[Text_MilitaryStrength] (Progress Bar)
  - Binding: Get Current Station → Get Faction → Military Strength → Divide by 10 (for 0-1 range)

[Text_EconomicPower] (Progress Bar)
  - Binding: Get Current Station → Get Faction → Economic Power → Divide by 10
```

### Advanced: Faction Traits Display

**Blueprint Graph: Populate Traits List**
```
Function: UpdateTraitsDisplay

Get Current Station
  ↓
Get Faction
  ↓
Get Traits (inherited C++ function - returns array)
  ↓
Clear Children (of TraitsContainer widget)
  ↓
ForEach Loop (Traits array)
  ├─ Loop Body
  │    ↓
  │  Create Widget (WBP_TraitListItem)
  │    ↓
  │  Set Trait Name (in list item)
  │    ↓
  │  Set Trait Description (in list item)
  │    ↓
  │  Set Modifier Value (in list item)
  │    ↓
  │  Add Child to TraitsContainer (Vertical Box)
  │
  └─ Completed
```

**Trait List Item Widget (WBP_TraitListItem):**
```
Components:
- HorizontalBox
  ├─ Text_TraitName (Bold)
  ├─ Spacer
  ├─ Text_ModifierValue (colored: green if positive, red if negative)
  └─ Text_TraitDescription (small font)
```

---

## Module List Display

### Creating a Scrollable Module List

**Widget Setup:**
1. Add a **Scroll Box** named `ScrollBox_ModuleList`
2. Add a **Vertical Box** inside named `VerticalBox_ModuleContainer`

**Blueprint Graph: Refresh Module List**
```
Function: RefreshModuleList

Get All Modules (inherited C++ function)
  ↓
Clear Children (VerticalBox_ModuleContainer)
  ↓
ForEach Loop (Modules array)
  ├─ Loop Body
  │    ↓
  │  Create Widget (WBP_ModuleListItem)
  │    ↓
  │  Set Module Reference (in list item)
  │    ↓
  │  Set Module Type (in list item)
  │    ↓
  │  Set Module Group (in list item)
  │    ↓
  │  Get Module Faction (C++ function)
  │    ↓
  │  Branch (Has faction?)
  │    ├─ True → Set Faction Color Indicator
  │    └─ False → Hide Faction Indicator
  │    ↓
  │  Bind On Click Event (to SelectModule custom event)
  │    ↓
  │  Add Child to VerticalBox_ModuleContainer
  │
  └─ Completed
       ↓
     Update Module Count Display
```

**Module List Item Widget (WBP_ModuleListItem):**
```
Components:
- Border (with colored left edge showing module faction)
  └─ HorizontalBox
       ├─ Image_ModuleIcon (based on ModuleGroup)
       ├─ VerticalBox
       │    ├─ Text_ModuleType
       │    └─ Text_ModuleGroup
       ├─ Spacer
       └─ Button_Remove
```

### Module List Item Blueprint

**Event Graph:**
```
Event: On Module Reference Set (custom)
  ↓
Get Module Type
  ↓
Set Text_ModuleType
  ↓
Get Module Group (enum)
  ↓
Select Icon Based on Group
  ├─ Docking → Icon_Docking
  ├─ Power → Icon_Power
  ├─ Storage → Icon_Storage
  ├─ Processing → Icon_Processing
  ├─ Defence → Icon_Defence
  ├─ Habitation → Icon_Habitation
  ├─ Public → Icon_Public
  ├─ Connection → Icon_Connection
  └─ Other → Icon_Generic
  ↓
Set Image_ModuleIcon
```

---

## Technology Level Filtering

### Filter Module Palette by Tech Level

**Blueprint Graph:**
```
Function: UpdateAvailableModules

Get Available Module Types (inherited C++ function)
  ↓
Clear Module Palette UI
  ↓
ForEach Loop (Available Types array)
  ├─ Loop Body
  │    ↓
  │  Create Module Button (WBP_ModulePaletteButton)
  │    ↓
  │  Set Module Type Name
  │    ↓
  │  Get Module Class by Type (custom function)
  │    ↓
  │  Can Add Module For Faction (C++ function)
  │    ├─ True → Enable button
  │    └─ False → Disable button with tooltip
  │         ↓
  │       Get Current Station
  │         ↓
  │       Get Faction
  │         ↓
  │       Get Technology Level
  │         ↓
  │       Format Tooltip: "Requires Tech Level {X}"
  │    ↓
  │  Add to Module Palette Container
  │
  └─ Completed
```

### Visual Tech Level Indicators

**Blueprint Graph: Update Tech Level Display**
```
Function: UpdateTechLevelDisplay

Get Current Station
  ↓
Get Faction
  ↓
Branch (Is Valid?)
  ├─ False → Hide Tech Level UI
  └─ True → Continue
       ↓
     Get Technology Level
       ↓
     Set ProgressBar_TechLevel (Percent = TechLevel / 10)
       ↓
     Set Text_TechLevel ("Tech Level: {0}/10")
       ↓
     Set Tech Level Color
       ├─ 1-3 → Red (Low Tech)
       ├─ 4-6 → Yellow (Standard)
       ├─ 7-9 → Green (Advanced)
       └─ 10 → Cyan (Experimental)
```

---

## Visual Feedback Systems

### Module Preview on Hover

**Blueprint Graph:**
```
Event Tick (or custom update function)
  ↓
Branch (Is Module Palette Hovered?)
  ├─ False → Hide Preview
  └─ True → Continue
       ↓
     Get Hovered Module Class
       ↓
     Branch (Has Preview Module?)
       ├─ True → Update Preview Position
       └─ False → Spawn Preview Module
            ↓
          Get Mouse Position in World
            ↓
          Spawn Preview Actor (ModuleClass)
            ↓
          Set Material to Translucent Preview Material
            ↓
          Disable Collision
       ↓
     Is Valid Placement (C++ function)
       ├─ True → Set Preview Color to Green
       └─ False → Set Preview Color to Red
```

### Placement Grid Visualization

**Blueprint Graph:**
```
Function: DrawPlacementGrid

Get Current Station Location
  ↓
Set Grid Parameters
  ├─ Grid Size: 1000 units
  ├─ Cell Size: 100 units
  └─ Grid Color: (R:0.5, G:0.5, B:0.5, A:0.3)
  ↓
For Loop (X axis)
  ↓
  For Loop (Y axis)
    ↓
    Calculate Line Position
      ↓
    Draw Debug Line (or use Line Batch Component)
```

### Success/Error Feedback

**Blueprint Graph: Show Notification**
```
Function: ShowNotification (Parameters: Message Text, IsSuccess Bool)

Branch (IsSuccess?)
  ├─ True → Set Color to Green
  └─ False → Set Color to Red
  ↓
Create Widget (WBP_NotificationToast)
  ↓
Set Message Text
  ↓
Set Background Color
  ↓
Add to Viewport
  ↓
Play Fade In Animation
  ↓
Delay (2.0 seconds)
  ↓
Play Fade Out Animation
  ↓
Remove from Parent
```

### Module Selection Highlight

**Blueprint Graph:**
```
Function: HighlightSelectedModule (Parameter: Module Reference)

Clear Previous Highlight
  ↓
Branch (Is Valid Module?)
  ├─ False → End
  └─ True → Continue
       ↓
     Get Module Mesh Component
       ↓
     Create Dynamic Material Instance
       ↓
     Set Material Parameters
       ├─ EmissiveColor: Yellow/Orange
       └─ EmissiveStrength: 2.0
       ↓
     Apply Material to Mesh
       ↓
     Play Selection Sound
```

---

## Complete Example: Station Editor Main Flow

### Full Widget Event Graph

**Initialization:**
```
Event Construct
  ↓
├─ Initialize UI
│    ↓
│  Populate Faction List
│    ↓
│  Set Default Faction
│
├─ Set Station Reference
│    ↓
│  Get Player Controller
│    ↓
│  Get Current Station
│    ↓
│  Set Station (C++ function)
│
├─ Refresh All UI
│    ↓
│  Update Faction Info Display
│    ↓
│  Update Module List
│    ↓
│  Update Available Modules
│    ↓
│  Update Tech Level Display
│
└─ Bind Input Events
     ↓
   Bind Mouse Click for Placement
     ↓
   Bind Hotkeys (Delete, Esc, etc.)
```

**Main Interaction Loop:**
```
Event Tick
  ↓
Branch (Is Placing Module?)
  ├─ True → Update Preview Position
  └─ False → Skip
  ↓
Branch (Mouse Button Pressed?)
  ├─ True → Execute Placement
  └─ False → Skip
  ↓
Update Hover State
  ↓
Update Tooltip Display
```

---

## Blueprint Performance Tips

### Optimization Best Practices

1. **Avoid Tick for UI Updates**
   ```
   Instead of Event Tick:
   - Use Event-driven updates (button clicks)
   - Use Property Bindings sparingly
   - Cache frequently accessed references
   ```

2. **Efficient List Updates**
   ```
   Instead of rebuilding entire list:
   - Add/Remove individual items
   - Use object pooling for list items
   - Update only changed properties
   ```

3. **Reference Caching**
   ```
   Store frequently used references as variables:
   - Current Station
   - Current Faction
   - Player Controller
   - Main Camera
   ```

### Example Cached Reference Pattern

**Variables:**
```
CachedStation (Type: SpaceStation Reference)
CachedFaction (Type: FactionDataAsset Reference)
CachedPlayerController (Type: PlayerController Reference)
```

**Initialization:**
```
Event Construct
  ↓
Get Player Controller → Set CachedPlayerController
  ↓
Get Current Station → Set CachedStation
  ↓
Get Faction → Set CachedFaction
```

**Usage:**
```
Anywhere in the widget:
Use CachedStation instead of repeatedly calling Get Current Station
Use CachedFaction instead of Get Station → Get Faction
```

---

## Testing Your Blueprint Implementation

### Debug Checklist

1. **Test Module Addition**
   - [ ] Can add basic modules
   - [ ] Tech level restrictions work
   - [ ] Placement validation works
   - [ ] UI updates correctly

2. **Test Module Removal**
   - [ ] Can select modules from list
   - [ ] Remove button works
   - [ ] UI updates after removal
   - [ ] Confirmation dialog appears

3. **Test Faction Selection**
   - [ ] Dropdown populates
   - [ ] Faction info displays
   - [ ] Colors update correctly
   - [ ] Available modules filter

4. **Test Visual Feedback**
   - [ ] Preview module appears
   - [ ] Valid/invalid placement colors
   - [ ] Selection highlights
   - [ ] Success/error notifications

### Debug Print Nodes

Add these temporarily for debugging:

```
After Add Module:
Print String ("Module Added: " + ModuleType + " at " + Location)

After Set Faction:
Print String ("Faction Changed to: " + FactionName)

In Can Add Module:
Print String ("Tech Check: Required=" + RequiredLevel + ", Current=" + CurrentLevel)
```

---

## Common Blueprint Patterns

### Pattern 1: Safe Null Checking

```
Get Reference
  ↓
Branch (Is Valid?)
  ├─ True → Use Reference
  └─ False → Show Error OR Use Default Value
```

### Pattern 2: Event-Driven UI Update

```
[Any Change Event]
  ↓
Call Custom Event: "RefreshUI"
  ↓
In RefreshUI:
  - Update all relevant UI elements
  - Single source of truth for UI state
```

### Pattern 3: Async Operation Handling

```
[Start Long Operation]
  ↓
Show Loading Widget
  ↓
Delay (or use Event Dispatcher from C++)
  ↓
Hide Loading Widget
  ↓
Update UI with Results
```

---

## Additional Resources

- **UMG Best Practices**: Unreal Engine Documentation
- **Data Binding**: See UE5 Property Binding guide in Unreal Engine documentation
- **C++ Function Reference**: See header files for complete API
- **Example Widgets**: Check `Content/UI/Examples/` (if available)

---

## Quick Reference: Essential Blueprint Nodes

### From UStationEditorWidget (StationEditor Module C++)
- `Set Station` - Assign the station to edit
- `Add Module` - Spawn and add module
- `Remove Module` - Remove module from station
- `Move Module` - Reposition module
- `Get All Modules` - Get module list
- `Is Valid Placement` - Check if location is valid
- `Set Station Faction` - Change station faction
- `Get Available Module Types` - Get buildable modules
- `Can Add Module For Faction` - Check tech requirements

### From SpaceStation (C++)
- `Get Faction` - Get station's faction
- `Has Faction Trait` - Check for trait
- `Get Faction Trait Modifier` - Get trait value
- `Is Allied With Faction` - Check alliance
- `Is At War With Faction` - Check war status

### From FactionDataAsset (C++)
- `Get Traits` - Get all traits
- `Has Trait` - Check specific trait
- `Get Relationship` - Get faction relationship
- `Is Allied With` - Check alliance
- `Is At War With` - Check war

---

## Template Checklist

When creating your Station Editor widget:

1. **Setup Phase**
   - [ ] Create WBP_StationEditor extending UStationEditorWidget (StationEditor module)
   - [ ] Design UI layout
   - [ ] Add all necessary UI components

2. **Initialization**
   - [ ] Implement Event Construct
   - [ ] Set station reference
   - [ ] Populate faction list
   - [ ] Initialize cached references

3. **Core Features**
   - [ ] Implement Add Module workflow
   - [ ] Implement Remove Module workflow
   - [ ] Implement Faction Selection
   - [ ] Implement Module List display

4. **Visual Polish**
   - [ ] Add module preview
   - [ ] Add placement grid
   - [ ] Add selection highlights
   - [ ] Add success/error notifications

5. **Testing**
   - [ ] Test all workflows
   - [ ] Test edge cases (null references, etc.)
   - [ ] Test performance with many modules
   - [ ] Test visual feedback

---

Happy Blueprint coding! 🎮
