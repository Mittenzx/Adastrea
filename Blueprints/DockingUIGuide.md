# Docking UI - Blueprint Guide

> **⚠️ DEPRECATED**: This guide has been consolidated into the unified Trading and Docking Complete Guide.  
> **See**: [Trading and Docking Complete Guide](../docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)

---

> **Create interaction prompts and docking confirmation UI for station approach**

**Blueprint Type**: Widget Blueprint  
**Complexity**: Beginner  
**Time to Complete**: 20-30 minutes  
**Last Updated**: 2026-01-03  
**Status**: **DEPRECATED** - Refer to unified guide above

---

## 📋 Overview

### What You'll Build

A simple UI widget that shows when the player is near a station and can dock. It displays a prompt, station name, and confirms when docking begins.

**End Result**: Players see "Press E to Dock at [Station Name]" when near stations, and get visual feedback when docking.

### Why This Blueprint Matters

Docking UI provides essential feedback for the MVP trading loop. Players need to know when they can interact with stations.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [x] `BP_TradingStation` created (see Trading Station Actor guide)
- [x] Player ship with interaction capability
- [x] Basic UMG knowledge
- [x] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Trading Station Actor Guide](TradingStationActorGuide.md) - For creating stations
- [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - For UMG basics

---

## 🔗 Dependencies

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_TradingStation` | Station to dock at | `Content/Stations/Trading/` |
| `BP_PlayerShip` | Ship that docks | `Content/Ships/Player/` |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/UI/Docking/`
3. **Right-click** → User Interface → Widget Blueprint
4. **Choose Parent Class**: `UserWidget`
5. **Name it**: `WBP_DockingPrompt`
6. **Double-click** to open the Blueprint Editor

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: StationName

- **Type**: `Text`
- **Category**: `Docking`
- **Instance Editable**: No
- **Tooltip**: "Name of the station player can dock at"
- **Default Value**: Empty

### Variable 2: IsVisible

- **Type**: `Boolean`
- **Category**: `Docking`
- **Instance Editable**: No
- **Tooltip**: "Whether prompt is currently shown"
- **Default Value**: `False`

### Variable 3: InteractionKey

- **Type**: `Text`
- **Category**: `Docking`
- **Instance Editable**: Yes
- **Tooltip**: "Key to press for docking (e.g., 'E')"
- **Default Value**: "E"

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing logic:**

### Function 1: ShowPrompt

**Purpose**: Display the docking prompt with station name

**Inputs**:
- `NewStationName` (Text) - Name of the station

**Outputs**: None

**Implementation**: See [Step 4](#step-4-implement-showprompt) below

### Function 2: HidePrompt

**Purpose**: Hide the docking prompt

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 5](#step-5-implement-hideprompt) below

### Function 3: ShowDockingConfirmation

**Purpose**: Show brief confirmation when docking starts

**Inputs**:
- `StationName` (Text) - Name of docked station

**Outputs**: None

**Implementation**: See [Step 6](#step-6-implement-showdockingconfirmation) below

---

## 📐 Widget Layout

### Visual Hierarchy

```
Canvas Panel (Root)
├── Vertical Box (DockingPromptContainer)
│   ├── Text Block (Text_StationName) - "Station Name"
│   ├── Horizontal Box (PromptRow)
│   │   ├── Text Block (Text_Prompt) - "Press"
│   │   ├── Border (KeyDisplay)
│   │   │   └── Text Block (Text_Key) - "E"
│   │   └── Text Block (Text_Action) - "to Dock"
│   └── Spacer (10px)
└── Border (ConfirmationPanel)
    └── Text Block (Text_Confirmation) - "Docking..."
```

### Key Widget Names (Important!)

| Widget | Name in Hierarchy | Purpose |
|--------|-------------------|---------|
| Vertical Box | `DockingPromptContainer` | Main prompt container |
| Text Block | `Text_StationName` | Shows station name |
| Text Block | `Text_Key` | Shows interaction key |
| Border | `ConfirmationPanel` | Docking confirmation |
| Text Block | `Text_Confirmation` | Confirmation message |

---

## 🛠️ Step-by-Step Implementation

### Step 1: Design the Prompt Layout

**Goal**: Create the visual structure for the docking prompt

**Instructions**:

1. In the **Designer** tab, drag a **Canvas Panel** to hierarchy (root)
2. **Add Vertical Box**:
   - Drag onto Canvas Panel
   - Rename: `DockingPromptContainer`
   - Anchors: Bottom Center
   - Position: X=0, Y=-150 (from bottom)
   - Size: 400 x 100
3. **Add Station Name Text**:
   - Drag **Text Block** into Vertical Box
   - Rename: `Text_StationName`
   - Font Size: 24
   - Color: Yellow
   - Alignment: Center
   - Text: "Station Name"
4. **Add Horizontal Box** (for prompt):
   - Drag into Vertical Box
   - Contains: "Press [E] to Dock"
5. **Add Prompt Parts**:
   - Text Block: `Text_Prompt` = "Press"
   - Border: `KeyDisplay` (background for key)
     - Padding: 5
     - Background: Dark gray
     - Contains Text Block: `Text_Key` = "E"
   - Text Block: `Text_Action` = "to Dock"
6. **Style all text**:
   - Font Size: 18
   - Color: White
   - Alignment: Center

**Expected Result**: Prompt looks like: **Station Name** | Press [E] to Dock

---

### Step 2: Add Confirmation Panel

**Goal**: Add docking confirmation message

**Instructions**:

1. **Add Border** to Canvas Panel (root):
   - Rename: `ConfirmationPanel`
   - Anchors: Center
   - Position: 0, 0
   - Size: 300 x 80
   - Background: Black with 80% opacity
   - Visibility: **Hidden** (by default)
2. **Add Text Block** inside Border:
   - Rename: `Text_Confirmation`
   - Text: "Docking at Station..."
   - Font Size: 20
   - Color: Green
   - Alignment: Center

**Expected Result**: Confirmation panel exists but is hidden.

---

### Step 3: Set Initial Visibility

**Goal**: Hide all UI elements by default

**Instructions**:

1. In **Event Graph**, locate `Event Construct`
2. **Add nodes**:
   ```
   Event Construct
     ↓
   Set Visibility (DockingPromptContainer)
     └─ Visibility: Hidden
     ↓
   Set Visibility (ConfirmationPanel)
     └─ Visibility: Hidden
     ↓
   Set IsVisible = False
   ```

**Detailed Instructions**:

1. From `Event Construct`:
   - Add node: `Set Visibility`
   - Target: `DockingPromptContainer` (drag from variable)
   - New Visibility: `Hidden`
2. **Repeat** for `ConfirmationPanel`
3. **Set variable**: `IsVisible` = `False`

**Expected Result**: Widget starts hidden when created.

---

### Step 4: Implement ShowPrompt

**Goal**: Make prompt visible with station name

**Blueprint Nodes**:

```
Function: ShowPrompt
  Input: NewStationName (Text)
  ↓
Set StationName (variable) = NewStationName
  ↓
Set Text (Text_StationName widget)
  └─ Text: StationName variable
  ↓
Set Text (Text_Key widget)
  └─ Text: InteractionKey variable
  ↓
Set Visibility (DockingPromptContainer)
  └─ Visibility: Visible
  ↓
Set IsVisible = True
```

**Detailed Instructions**:

1. **Create new function**: `ShowPrompt`
2. **Add input**: `NewStationName` (Type: Text)
3. **Set variable**: `Set StationName` = `NewStationName` (input)
4. **Update UI**:
   - Add node: `Set Text` (Text_StationName)
   - Text: `StationName` variable
5. **Update key display**:
   - Add node: `Set Text` (Text_Key)
   - Text: `InteractionKey` variable
6. **Show container**:
   - Add node: `Set Visibility` (DockingPromptContainer)
   - Visibility: `Visible`
7. **Update flag**:
   - Add node: `Set IsVisible` = `True`

**Expected Result**: Function makes prompt visible with provided station name.

---

### Step 5: Implement HidePrompt

**Goal**: Hide the docking prompt

**Blueprint Nodes**:

```
Function: HidePrompt
  ↓
Set Visibility (DockingPromptContainer)
  └─ Visibility: Hidden
  ↓
Set IsVisible = False
```

**Detailed Instructions**:

1. **Create new function**: `HidePrompt`
2. **Hide container**:
   - Add node: `Set Visibility` (DockingPromptContainer)
   - Visibility: `Hidden`
3. **Update flag**:
   - Add node: `Set IsVisible` = `False`

**Expected Result**: Function hides the prompt.

---

### Step 6: Implement ShowDockingConfirmation

**Goal**: Show brief confirmation when docking

**Blueprint Nodes**:

```
Function: ShowDockingConfirmation
  Input: StationName (Text)
  ↓
Format Text: "Docking at {0}..."
  └─ {0}: StationName
  ↓
Set Text (Text_Confirmation)
  ↓
Set Visibility (ConfirmationPanel)
  └─ Visibility: Visible
  ↓
Delay (2.0 seconds)
  ↓
Set Visibility (ConfirmationPanel)
  └─ Visibility: Hidden
```

**Detailed Instructions**:

1. **Create new function**: `ShowDockingConfirmation`
2. **Add input**: `StationName` (Type: Text)
3. **Format message**:
   - Add node: `Format Text`
   - Format: "Docking at {0}..."
   - {0}: `StationName` (input)
4. **Update text**:
   - Add node: `Set Text` (Text_Confirmation)
   - Text: Result from Format Text
5. **Show panel**:
   - Add node: `Set Visibility` (ConfirmationPanel)
   - Visibility: `Visible`
6. **Auto-hide after delay**:
   - Add node: `Delay`
   - Duration: `2.0`
7. **Hide panel**:
   - Add node: `Set Visibility` (ConfirmationPanel)
   - Visibility: `Hidden`

**Expected Result**: Function shows confirmation for 2 seconds then hides.

---

### Step 7: Add to Player HUD

**Goal**: Add this widget to player's screen

**Instructions for Player Controller or HUD Blueprint**:

1. **Open** your Player Controller or HUD Blueprint
2. In **Event BeginPlay**:
   ```
   Event BeginPlay
     ↓
   Create Widget (WBP_DockingPrompt)
     ↓
   Add to Viewport
     ↓
   Store reference in variable (DockingPromptWidget)
   ```
3. **Create variable**:
   - Name: `DockingPromptWidget`
   - Type: `WBP_DockingPrompt` (Object Reference)

**Expected Result**: Docking prompt widget is on screen (but hidden initially).

---

### Step 8: Connect to Station Interaction

**Goal**: Show/hide prompt based on station proximity

**Instructions for Trading Station Blueprint**:

1. In **BP_TradingStation**, find the **overlap events**
2. **On Begin Overlap** (when player enters range):
   ```
   Event ActorBeginOverlap
     ↓
   Cast to PlayerController (or get from pawn)
     ↓
   Get HUD (or get DockingPromptWidget reference)
     ↓
   Cast to WBP_DockingPrompt
     ↓
   Call ShowPrompt
     └─ StationName: (from station's MarketData or property)
   ```
3. **On End Overlap** (when player leaves range):
   ```
   Event ActorEndOverlap
     ↓
   Get DockingPromptWidget reference
     ↓
   Call HidePrompt
   ```
4. **On Interact** (when player presses E):
   ```
   Input Action E
     ↓
   Get DockingPromptWidget reference
     ↓
   Call ShowDockingConfirmation
     └─ StationName: (from station)
     ↓
   [Open Trading UI as normal]
   ```

**Expected Result**: Prompt shows when near station, confirmation when docking.

---

## 🧪 Testing Your Blueprint

### Test 1: Widget Visibility

**What to Test**: Widget starts hidden

**Steps**:
1. Add `WBP_DockingPrompt` to viewport in test level
2. Play in editor
3. Check if prompt is visible

**Success Criteria**:
- Widget is not visible at game start
- No errors in log

**If it fails**: Check Event Construct sets visibility to Hidden

---

### Test 2: Show and Hide Prompt

**What to Test**: ShowPrompt and HidePrompt functions work

**Steps**:
1. In test level, add timer to call ShowPrompt after 2 seconds
2. Add timer to call HidePrompt after 5 seconds
3. Play in editor
4. Watch for prompt appearance/disappearance

**Success Criteria**:
- Prompt appears at 2 seconds
- Shows correct station name
- Disappears at 5 seconds

**If it fails**: See [Troubleshooting](#common-issue-1-prompt-doesnt-show)

---

### Test 3: Station Integration

**What to Test**: Prompt shows when approaching station

**Steps**:
1. Place trading station in level
2. Connect overlap events to ShowPrompt/HidePrompt
3. Play and walk toward station
4. Watch for prompt
5. Walk away from station

**Success Criteria**:
- Prompt appears when in range
- Shows station name
- Disappears when leaving range

**If it fails**: Check station overlap events are bound correctly

---

### Test 4: Docking Confirmation

**What to Test**: Confirmation shows when docking

**Steps**:
1. Approach station (prompt should show)
2. Press E (or interaction key)
3. Watch for confirmation message

**Success Criteria**:
- "Docking at [Station]..." message appears
- Message disappears after 2 seconds
- Trading UI opens (if connected)

**If it fails**: Check ShowDockingConfirmation is called on interact

---

## 🐛 Troubleshooting

### Common Issue 1: Prompt Doesn't Show

**Symptoms**:
- ShowPrompt called but nothing appears
- Widget is on viewport but invisible

**Causes**:
- Container visibility not set to Visible
- Widget not added to viewport
- Z-order issue (behind other widgets)

**Solutions**:
1. In ShowPrompt function, verify:
   - Set Visibility (DockingPromptContainer) = Visible
2. Check widget is added to viewport:
   - In Player Controller/HUD BeginPlay
   - Call Add to Viewport
3. Check Z-Order in Add to Viewport:
   - Set Z-Order to high number (e.g., 10)

---

### Common Issue 2: Text Not Updating

**Symptoms**:
- Prompt shows but station name is wrong
- Shows "Station Name" default text

**Causes**:
- Set Text not being called
- Variable not being passed correctly

**Solutions**:
1. In ShowPrompt, add Print String with NewStationName to verify input
2. Check Set Text is connected properly
3. Verify Text_StationName widget binding:
   - Target should be the actual text widget
   - Text should be from StationName variable

---

### Common Issue 3: Confirmation Stays Forever

**Symptoms**:
- Docking confirmation shows
- Never disappears

**Causes**:
- Delay not working
- Second Set Visibility not called

**Solutions**:
1. Check Delay node is connected properly
2. Verify execution flows from Delay to Hide visibility
3. Add Print String after Delay to verify it executes
4. Check Duration is 2.0 seconds (not 0)

---

### Common Issue 4: Multiple Prompts Stacking

**Symptoms**:
- Entering/leaving station multiple times creates duplicates
- Multiple "Press E" prompts appear

**Causes**:
- Creating new widget each time instead of reusing
- Not storing widget reference

**Solutions**:
1. In Player Controller/HUD, check:
   - Widget created once in BeginPlay
   - Reference stored in variable
   - Same reference used for Show/Hide
2. Don't create new widget on each overlap
3. Just call ShowPrompt/HidePrompt on existing widget

---

## ✨ Optional Enhancements

**After the basic UI works, consider adding:**

1. **Distance Display**
   - What it adds: Show distance to station
   - Complexity: Simple
   - Guide: Calculate distance in station, pass to ShowPrompt

2. **Animated Prompt**
   - What it adds: Fade in/out animations
   - Complexity: Medium
   - Guide: Use UMG animations in Designer tab

3. **Docking Progress Bar**
   - What it adds: Show docking animation progress
   - Complexity: Medium
   - Guide: Add Progress Bar widget, update via timer

4. **Multiple Station Types**
   - What it adds: Different icons/colors per station type
   - Complexity: Medium
   - Guide: Pass station type, set icon/color in ShowPrompt

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading Station Actor](TradingStationActorGuide.md) - Station that triggers this UI
- [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) - Full trading interface
- [UMG Basics](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - Widget fundamentals
- [Player HUD Setup](../Assets/HUDSystemGuide.md) - Creating player HUD

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a simple UI widget
- ✅ How to show/hide UI elements programmatically
- ✅ How to pass data to UI functions (station name)
- ✅ How to use Delay for timed UI updates
- ✅ How to integrate UI with game events (overlap, input)
- ✅ How to format text dynamically

**Next Recommended**: Polish your trading loop with more visual feedback!

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Add station logo/icon to prompt
- Different prompt styles per station type
- Add sound effects on show/hide
- Animate prompt appearance with UMG animations

**Blueprint Best Practices Applied:**
- Separated visibility logic into functions (Show/Hide)
- Used text formatting for dynamic messages
- Implemented auto-hide with Delay node
- Reused single widget instance (no duplicates)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-03 | Initial guide following new template standard |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [UMG UI Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-ui-designer-for-unreal-engine)
- See [Blueprints README](README.md) for more guides
