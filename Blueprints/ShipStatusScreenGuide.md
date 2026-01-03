# Ship Status Screen - Blueprint Guide

> **Create a HUD widget showing ship information and stats**

**Blueprint Type**: Widget Blueprint  
**Complexity**: Intermediate  
**Time to Complete**: 40-50 minutes  
**Last Updated**: 2026-01-03

---

## 📋 Overview

### What You'll Build

A ship status screen widget that displays the player's ship information including name, class, description, and detailed statistics across core, combat, mobility, and utility categories with visual progress bars for ratings.

**End Result**: Players can press a key to toggle a full-screen ship status display showing all ship stats and ratings.

### Why This Blueprint Matters

The ship status screen provides essential information to players about their vessel's capabilities, helping them understand their ship's strengths and weaknesses for trading, combat, and navigation.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [x] `UShipStatusWidget` C++ class compiled (UI module)
- [x] At least one `USpaceshipDataAsset` created (e.g., `DA_Ship_PlayerStarter`)
- [x] Player ship Blueprint with spaceship data reference
- [x] Enhanced Input system set up (or legacy input)
- [x] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Spaceship Data Asset Guide](../Assets/SpaceshipSetupGuide.md) - For creating ship data
- [Enhanced Input Guide](../docs/development/ENHANCED_INPUT_GUIDE.md) - For input setup

---

## 🔗 Dependencies

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UShipStatusWidget` | UI | Base widget with ship data integration |
| `USpaceshipDataAsset` | Ships | Ship properties and stats |

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_PlayerShip` | Ship with data to display | `Content/Ships/Player/` |
| `BP_PlayerController` | Input handling and HUD | `Content/Blueprints/` |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/UI/HUD/`
3. **Right-click** → User Interface → Widget Blueprint
4. **Choose Parent Class**: Search for and select `ShipStatusWidget`
5. **Name it**: `WBP_ShipStatus`
6. **Double-click** to open the Blueprint Editor

### Step 2: Configure Class Settings

1. Blueprint is already set to parent `ShipStatusWidget`
2. No additional class settings needed

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: IsVisible

- **Type**: `Boolean`
- **Category**: `Display`
- **Instance Editable**: No
- **Tooltip**: "Whether the status screen is currently visible"
- **Default Value**: `False`

### Variable 2: CachedShipData

- **Type**: `Spaceship Data Asset` (Object Reference)
- **Category**: `Display`
- **Instance Editable**: No
- **Tooltip**: "Cached reference to ship's data asset"
- **Default Value**: None (set at runtime)

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing logic:**

### Function 1: ToggleVisibility

**Purpose**: Show or hide the status screen

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 5](#step-5-implement-togglevisibility) below

### Function 2: RefreshAllStats

**Purpose**: Update all stat displays from ship data

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 6](#step-6-implement-refreshallstats) below

### Function 3: UpdateRatingBars

**Purpose**: Update progress bars for combat, mobility, utility ratings

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 7](#step-7-implement-updateratingbars) below

---

## 📐 Widget Layout

### Visual Hierarchy

```
Canvas Panel (Root)
├── Image_Background (Full screen, semi-transparent black)
├── Vertical Box (Main_Container)
│   ├── Horizontal Box (Header_Row)
│   │   ├── Text Block (Text_ShipName) - Large font
│   │   ├── Spacer
│   │   └── Button (Button_Close) - "X"
│   ├── Horizontal Box (Class_And_Ratings)
│   │   ├── Text Block (Text_ShipClass)
│   │   ├── Spacer
│   │   ├── Text Block (Text_CombatRating)
│   │   ├── Text Block (Text_MobilityRating)
│   │   └── Text Block (Text_UtilityRating)
│   ├── Text Block (Text_Description) - Multi-line
│   ├── Horizontal Box (Stats_Container)
│   │   ├── Vertical Box (Core_Stats_Column)
│   │   │   ├── Text Block (Header: "CORE STATS")
│   │   │   ├── Text Block (Text_Hull) + Progress Bar
│   │   │   ├── Text Block (Text_Cargo)
│   │   │   ├── Text Block (Text_Crew)
│   │   │   └── Text Block (Text_ModularPoints)
│   │   ├── Vertical Box (Combat_Stats_Column)
│   │   │   ├── Text Block (Header: "COMBAT")
│   │   │   ├── Text Block (Text_Armor)
│   │   │   ├── Text Block (Text_Shield)
│   │   │   ├── Text Block (Text_Weapons)
│   │   │   └── Text Block (Text_PointDefense)
│   │   ├── Vertical Box (Mobility_Stats_Column)
│   │   │   ├── Text Block (Header: "MOBILITY")
│   │   │   ├── Text Block (Text_MaxSpeed)
│   │   │   ├── Text Block (Text_Acceleration)
│   │   │   ├── Text Block (Text_Maneuverability)
│   │   │   └── Text Block (Text_JumpRange)
│   │   └── Vertical Box (Utility_Stats_Column)
│   │       ├── Text Block (Header: "UTILITY")
│   │       ├── Text Block (Text_SensorRange)
│   │       ├── Text Block (Text_SensorResolution)
│   │       ├── Text Block (Text_StealthRating)
│   │       └── Text Block (Text_RepairRating)
│   └── Horizontal Box (Footer_Ratings)
│       ├── Progress Bar (ProgressBar_CombatRating)
│       ├── Progress Bar (ProgressBar_MobilityRating)
│       └── Progress Bar (ProgressBar_UtilityRating)
```

### Key Widget Names (Important!)

| Widget | Name in Hierarchy | Purpose |
|--------|-------------------|---------|
| Text Block | `Text_ShipName` | Display ship name |
| Text Block | `Text_ShipClass` | Display ship class |
| Text Block | `Text_Description` | Ship description |
| Button | `Button_Close` | Close the screen |
| Text Block | `Text_Hull`, `Text_Cargo`, etc. | Individual stats |
| Progress Bar | `ProgressBar_CombatRating` | Combat rating visual |
| Progress Bar | `ProgressBar_MobilityRating` | Mobility rating visual |
| Progress Bar | `ProgressBar_UtilityRating` | Utility rating visual |

---

## 🛠️ Step-by-Step Implementation

### Step 1: Design the Widget Layout

**Goal**: Create the visual structure following the hierarchy above

**Instructions**:

1. In **Designer** tab, start with **Canvas Panel** (root)
2. **Add Background**:
   - Drag **Image** onto Canvas
   - Rename: `Image_Background`
   - Anchors: Fill (0,0 to 1,1)
   - Color: Black, Alpha 0.85
3. **Add Main Container**:
   - Drag **Vertical Box** onto Canvas
   - Rename: `Main_Container`
   - Anchors: Fill
   - Padding: 50px all sides
4. **Build Header**:
   - Add **Horizontal Box** to Main_Container
   - Add **Text Block**: `Text_ShipName` (Font 48, White)
   - Add **Spacer** (Fill)
   - Add **Button**: `Button_Close` (Text "X")
5. **Build Stats Sections**:
   - Follow hierarchy structure above
   - Create 4 columns using Horizontal Box + Vertical Boxes
   - Name each text block according to the hierarchy
   - Set "Is Variable" checkbox for all stat displays
6. **Add Progress Bars**:
   - At bottom, add 3 progress bars
   - Name them for Combat, Mobility, Utility
   - Set fill color: Combat=Red, Mobility=Blue, Utility=Green

**Expected Result**: Complete UI layout visible in Designer preview.

---

### Step 2: Set Initial Visibility

**Goal**: Hide the status screen by default

**Blueprint Nodes**:

```
Event Construct
  ↓
Set Visibility (Self)
  └─ Visibility: Hidden
  ↓
Set IsVisible = False
```

**Detailed Instructions**:

1. Switch to **Event Graph** tab
2. Locate `Event Construct`
3. **Add node**: `Set Visibility`
   - Target: `Self`
   - Visibility: `Hidden`
4. **Add node**: `Set IsVisible` = `False`

**Expected Result**: Widget starts hidden when created.

---

### Step 3: Get Ship Data Reference

**Goal**: Retrieve ship data from player's ship

**Blueprint Nodes**:

```
Custom Event: InitializeWithShip
  Input: ShipActor (Actor Reference)
  ↓
Cast to BP_PlayerShip
  ↓
Get Spaceship Data (from ship)
  ↓
Set CachedShipData (variable)
  ↓
Call RefreshAllStats
```

**Detailed Instructions**:

1. **Create custom event**: `InitializeWithShip`
2. **Add input**: `ShipActor` (Type: Actor Reference)
3. **Cast** to your player ship class (e.g., `BP_PlayerShip`)
4. From cast success, **call** `Get Spaceship Data` or similar getter
5. **Store** in `CachedShipData` variable
6. **Call** `RefreshAllStats` (we'll create this next)

**Expected Result**: Event stores ship data reference for later use.

---

### Step 4: Bind Close Button

**Goal**: Close the screen when X button clicked

**Blueprint Nodes**:

```
Button_Close → OnClicked
  ↓
Call ToggleVisibility
```

**Detailed Instructions**:

1. **Select** `Button_Close` in Designer
2. In **Details**, scroll to **Events**
3. Click **+** next to **On Clicked**
4. In Event Graph, **call** `ToggleVisibility` function

**Expected Result**: Clicking X closes the screen.

---

### Step 5: Implement ToggleVisibility

**Goal**: Show/hide the screen and manage input mode

**Blueprint Nodes**:

```
Function: ToggleVisibility
  ↓
Get IsVisible (variable)
  ↓
Branch
  ├─ True (Currently Visible - Hide it):
  │   ↓
  │ Set Visibility (Self) = Hidden
  │   ↓
  │ Set IsVisible = False
  │   ↓
  │ Get Player Controller
  │   ↓
  │ Set Input Mode Game Only
  │   ↓
  │ Set Show Mouse Cursor = False
  │
  └─ False (Currently Hidden - Show it):
      ↓
    Set Visibility (Self) = Visible
      ↓
    Set IsVisible = True
      ↓
    RefreshAllStats
      ↓
    Get Player Controller
      ↓
    Set Input Mode UI Only
      └─ Widget to Focus: Self
      ↓
    Set Show Mouse Cursor = True
```

**Detailed Instructions**:

1. **Create function**: `ToggleVisibility`
2. **Get** `IsVisible` variable
3. **Add** `Branch` node with IsVisible as condition
4. **True path** (hide):
   - Set Visibility = Hidden
   - Set IsVisible = False
   - Get Player Controller
   - Set Input Mode Game Only
   - Set Show Mouse Cursor = False
5. **False path** (show):
   - Set Visibility = Visible
   - Set IsVisible = True
   - Call RefreshAllStats
   - Get Player Controller
   - Set Input Mode UI Only (Widget to Focus = Self)
   - Set Show Mouse Cursor = True

**Expected Result**: Function toggles visibility and input mode.

---

### Step 6: Implement RefreshAllStats

**Goal**: Update all stat displays from cached ship data

**Blueprint Nodes**:

```
Function: RefreshAllStats
  ↓
Branch (Is CachedShipData Valid?)
  ├─ False: Print "No ship data" and return
  └─ True: Continue
      ↓
    // Basic Info
    Get Display Name (from CachedShipData)
      └─ Set Text (Text_ShipName)
      ↓
    Get Ship Class (from CachedShipData)
      └─ Set Text (Text_ShipClass)
      ↓
    Get Description (from CachedShipData)
      └─ Set Text (Text_Description)
      ↓
    // Core Stats
    Get Hull Strength
      └─ Format Text: "Hull: {0}"
        └─ Set Text (Text_Hull)
      ↓
    Get Cargo Capacity
      └─ Format Text: "Cargo: {0} units"
        └─ Set Text (Text_Cargo)
      ↓
    // ... repeat for all stats ...
      ↓
    Call UpdateRatingBars
```

**Detailed Instructions**:

1. **Create function**: `RefreshAllStats`
2. **Check** `Is Valid (CachedShipData)`
3. If invalid, print error and return
4. **For each stat** in the UI:
   - Call getter on CachedShipData (e.g., `Get Hull Strength`)
   - Format as text (e.g., "Hull: 1000")
   - Set corresponding text block
5. **Repeat** for all stat categories:
   - Core: Hull, Cargo, Crew, Modular Points
   - Combat: Armor, Shield, Weapons, Point Defense
   - Mobility: Speed, Acceleration, Maneuverability, Jump Range
   - Utility: Sensor Range, Resolution, Stealth, Repair
6. **Call** `UpdateRatingBars` at the end

**Expected Result**: All stat text blocks display current ship data.

---

### Step 7: Implement UpdateRatingBars

**Goal**: Update progress bars showing overall ratings

**Blueprint Nodes**:

```
Function: UpdateRatingBars
  ↓
Branch (Is CachedShipData Valid?)
  └─ True:
      ↓
    Get Combat Rating (from CachedShipData)
      ├─ Divide by 100.0
      └─ Set Percent (ProgressBar_CombatRating)
      ↓
    Get Mobility Rating (from CachedShipData)
      ├─ Divide by 100.0
      └─ Set Percent (ProgressBar_MobilityRating)
      ↓
    Get Utility Rating (from CachedShipData)
      ├─ Divide by 100.0
      └─ Set Percent (ProgressBar_UtilityRating)
```

**Detailed Instructions**:

1. **Create function**: `UpdateRatingBars`
2. **Check** `Is Valid (CachedShipData)`
3. **Get Combat Rating** (C++ function from ShipStatusWidget or directly from data):
   - If rating is 0-100, divide by 100 to get 0.0-1.0
   - Set Percent on `ProgressBar_CombatRating`
4. **Repeat** for Mobility and Utility ratings
5. Progress bars automatically show fill based on percent

**Expected Result**: Rating bars visually show ship's ratings.

---

### Step 8: Add Input Binding

**Goal**: Allow player to press key to toggle screen

**Instructions for Player Controller**:

1. **Open** `BP_PlayerController` (or your player controller)
2. **Create variable**: `ShipStatusWidget` (Type: `WBP_ShipStatus` Reference)
3. In **Event BeginPlay**:
   ```
   Event BeginPlay
     ↓
   Create Widget (WBP_ShipStatus)
     ↓
   Set ShipStatusWidget (variable)
     ↓
   Add to Viewport
     ↓
   Get Player Pawn
     ↓
   InitializeWithShip (call on widget)
     └─ ShipActor: Player Pawn
   ```
4. **Add Input Action**:
   - If Enhanced Input: `IA_ToggleShipStatus`
   - If Legacy: Key event (e.g., "I" key)
5. **On Input**:
   ```
   Input Action (or Key Press)
     ↓
   Get ShipStatusWidget (variable)
     ↓
   ToggleVisibility (call on widget)
   ```

**Expected Result**: Pressing key toggles ship status screen.

---

## 🧪 Testing Your Blueprint

### Test 1: Widget Displays

**What to Test**: Widget shows correctly when toggled

**Steps**:
1. Compile and save Widget Blueprint
2. Set up input binding in Player Controller
3. Play in editor
4. Press toggle key (e.g., "I")

**Success Criteria**:
- Widget appears on screen
- Background is semi-transparent
- Layout is organized
- No errors in log

**If it fails**: Check widget is added to viewport in Player Controller

---

### Test 2: Ship Data Displays

**What to Test**: All ship stats show correctly

**Steps**:
1. Ensure player ship has valid `SpaceshipDataAsset` assigned
2. Toggle widget open
3. Check all stat fields

**Success Criteria**:
- Ship name displays
- Ship class displays
- All stat values are populated (not "0" or blank)
- Description shows
- No "null" or "None" text

**If it fails**: See [Troubleshooting](#common-issue-1-stats-show-as-zero)

---

### Test 3: Rating Bars Display

**What to Test**: Progress bars show visual ratings

**Steps**:
1. Open widget
2. Look at bottom progress bars
3. Check they're filled appropriately

**Success Criteria**:
- Combat bar shows (should be red)
- Mobility bar shows (should be blue)
- Utility bar shows (should be green)
- Fill amounts match ship's ratings

**If it fails**: Check UpdateRatingBars is being called

---

### Test 4: Toggle and Input Mode

**What to Test**: Can toggle widget and return to game

**Steps**:
1. Press toggle key - widget opens
2. Try to move ship (should not move - UI mode)
3. Mouse should be visible
4. Press toggle key again - widget closes
5. Try to move ship (should move - game mode)

**Success Criteria**:
- Input mode switches correctly
- Mouse cursor visibility correct
- Can't control ship when UI open
- Can control ship when UI closed

**If it fails**: Check ToggleVisibility input mode logic

---

## 🐛 Troubleshooting

### Common Issue 1: Stats Show as Zero

**Symptoms**:
- All stats display as "0"
- Ship name is blank
- Widget opens but no data

**Causes**:
- CachedShipData is null
- InitializeWithShip not called
- Ship doesn't have SpaceshipDataAsset assigned

**Solutions**:
1. In Player Controller, verify `InitializeWithShip` is called
2. Check player ship has `SpaceshipDataAsset` set in Details panel
3. Add Print String after getting ship data to verify it's valid
4. In RefreshAllStats, check if CachedShipData is valid before using

---

### Common Issue 2: Widget Won't Open

**Symptoms**:
- Pressing toggle key does nothing
- No widget appears

**Causes**:
- Widget not added to viewport
- Input binding not working
- Widget reference is null

**Solutions**:
1. Check Player Controller BeginPlay adds widget to viewport
2. Verify input action is bound (check Input Mapping Context)
3. Add Print String in input event to confirm it fires
4. Check ShipStatusWidget variable is set after Create Widget

---

### Common Issue 3: Can't Close Widget

**Symptoms**:
- Widget opens but won't close
- X button doesn't work
- Toggle key doesn't work

**Causes**:
- Button OnClicked not bound
- ToggleVisibility logic wrong
- Input blocked

**Solutions**:
1. Check Button_Close has OnClicked event bound
2. In ToggleVisibility, verify both True and False paths work
3. Add Print String to show current IsVisible state
4. Check input action can fire when UI is open

---

### Common Issue 4: Input Stuck in UI Mode

**Symptoms**:
- Widget closes but can't control ship
- Mouse cursor stuck visible

**Causes**:
- Input mode not restored to game
- Set Input Mode Game Only not called

**Solutions**:
1. In ToggleVisibility False (hide) path, verify:
   - Set Input Mode Game Only is called
   - Set Show Mouse Cursor = False is called
2. Get correct Player Controller (index 0)
3. Check execution flow reaches these nodes

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Stat Comparison**
   - What it adds: Show stat changes when upgrading
   - Complexity: Medium
   - Guide: Add "previous stats" variables and show delta

2. **Animated Transitions**
   - What it adds: Fade in/out animations
   - Complexity: Simple
   - Guide: Use UMG animations in Designer tab

3. **Equipment Slots Display**
   - What it adds: Show installed weapons/modules
   - Complexity: Advanced
   - Guide: Add scroll box with equipment list

4. **Real-time Stat Updates**
   - What it adds: Update stats when ship takes damage
   - Complexity: Medium
   - Guide: Bind to ship damage events, refresh hull display

---

## 📚 Related Guides

**If you want to learn more:**

- [Ship Data Asset Setup](../Assets/SpaceshipSetupGuide.md) - Creating ship data
- [Enhanced Input System](../docs/development/ENHANCED_INPUT_GUIDE.md) - Input setup
- [UMG Widget Creation](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - Widget basics
- [HUD System Integration](../docs/systems/HUD_SYSTEM_GUIDE.md) - Complete HUD setup

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a complex UMG widget with multiple sections
- ✅ How to integrate C++ widget class with Blueprint
- ✅ How to read and display data from Data Assets
- ✅ How to toggle between game and UI input modes
- ✅ How to format and display numeric stats as text
- ✅ How to use progress bars for visual feedback

**Next Recommended Blueprint**: Ship Controls UI - For flight controls display

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Change stat formatting (units, decimals)
- Add color coding for stat values (red=low, green=high)
- Customize rating bar colors per ship class
- Add tabs for different stat pages

**Blueprint Best Practices Applied:**
- Cached ship data reference (no repeated lookups)
- Separated display update into functions (RefreshAllStats, UpdateRatingBars)
- Used input mode switching for proper UI behavior
- Validated data before use (Is Valid checks)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-03 | Updated to new template standard |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review C++ source: `Source/Adastrea/Public/UI/ShipStatusWidget.h`
- See [Blueprints README](README.md) for more guides
