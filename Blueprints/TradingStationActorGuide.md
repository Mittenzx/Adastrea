# Trading Station Actor - Blueprint Guide

> **⚠️ DEPRECATED**: This guide has been consolidated into the unified Trading and Docking Complete Guide.  
> **See**: [Trading and Docking Complete Guide](../docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)

---

> **Create a space station that hosts a trading terminal for buying and selling**

**Blueprint Type**: Actor Blueprint  
**Complexity**: Beginner  
**Time to Complete**: 30-40 minutes  
**Last Updated**: 2026-01-03  
**Status**: **DEPRECATED** - Refer to unified guide above

---

## 📋 Overview

### What You'll Build

A space station actor with a trading terminal that players can interact with to open the trading UI. The station will have a market Data Asset that defines what items are available for trade.

**End Result**: Players can approach the station, see a "Press E to Trade" prompt, and open the trading interface to buy/sell items.

### Why This Blueprint Matters

Trading stations are essential for the MVP trading gameplay loop. They provide the locations where players conduct all trading transactions.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [x] `ASpaceStation` C++ class compiled (Stations module)
- [x] At least one `UMarketDataAsset` created (e.g., `DA_Market_TestStation`)
- [x] `WBP_TradingUI` widget created (see Trading UI guide)
- [x] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Trading UI Widget Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - For creating the UI
- [Trading System Guide](../Assets/TradingSystemGuide.md) - For creating Market Data Assets

---

## 🔗 Dependencies

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `ASpaceStation` | Stations | Base station actor with faction support |
| `UMarketDataAsset` | TradingSystem | Market inventory and pricing |

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `WBP_TradingUI` | Trading interface | `Content/UI/Trading/` |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/Stations/Trading/`
3. **Right-click** → Blueprint Class → Actor
4. **Choose Parent Class**: Search for and select `ASpaceStation`
5. **Name it**: `BP_TradingStation`
6. **Double-click** to open the Blueprint Editor

### Step 2: Configure Class Settings

1. Blueprint is already set to parent `ASpaceStation`
2. No additional class settings needed

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: MarketData

- **Type**: `Market Data Asset` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: Yes
- **Tooltip**: "The market data for this station's trading inventory"
- **Default Value**: None (set per instance)

### Variable 2: TradingUIClass

- **Type**: `Class Reference` (Choose `User Widget`)
- **Category**: `Trading`
- **Instance Editable**: Yes
- **Tooltip**: "The trading UI widget to spawn when player interacts"
- **Default Value**: `WBP_TradingUI`

### Variable 3: CurrentTradingWidget

- **Type**: `User Widget` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "Reference to currently open trading UI"
- **Default Value**: None (set at runtime)

### Variable 4: InteractionRange

- **Type**: `Float`
- **Category**: `Trading`
- **Instance Editable**: Yes
- **Tooltip**: "Maximum distance player can be to interact with station"
- **Default Value**: `500.0`

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing logic:**

### Function 1: OpenTradingUI

**Purpose**: Opens the trading interface for the player

**Inputs**:
- `PlayerController` (Player Controller Reference) - The player opening the UI

**Outputs**: None

**Implementation**: See [Step 4](#step-4-implement-opentradingui) below

### Function 2: CloseTradingUI

**Purpose**: Closes the trading interface

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 5](#step-5-implement-closetradingui) below

### Function 3: IsPlayerInRange

**Purpose**: Checks if player is close enough to interact

**Inputs**:
- `PlayerPawn` (Pawn Reference) - The player to check

**Outputs**:
- `Return Value` (Boolean) - True if in range

**Implementation**: See [Step 6](#step-6-implement-isplayerinrange) below

---

## 🛠️ Step-by-Step Implementation

### Step 1: Add Components

**Goal**: Add visual and interaction components to the station

**Instructions**:

1. In the **Components** panel, click **Add Component**
2. **Add Static Mesh Component**:
   - Name it: `StationMesh`
   - In Details: Set Static Mesh to your station model
   - Set Location: (0, 0, 0)
3. **Add Box Collision Component**:
   - Name it: `InteractionTrigger`
   - In Details: Set Box Extent to (500, 500, 500)
   - Enable: Generate Overlap Events
4. **Add Text Render Component** (optional for debugging):
   - Name it: `StationLabel`
   - Set Text: "Trading Station"
   - Set Location: (0, 0, 500)

**Expected Result**: Station has a mesh and interaction trigger visible in viewport.

---

### Step 2: Set Default Values

**Goal**: Configure default variable values in the Blueprint

**Instructions**:

1. **Compile** the Blueprint to see variables
2. In **Class Defaults** (toolbar button):
   - Set `TradingUIClass` to `WBP_TradingUI`
   - Set `InteractionRange` to `500.0`
   - Leave `MarketData` unset (will be set per instance)

**Expected Result**: Default values are visible in Class Defaults panel.

---

### Step 3: Implement Overlap Events

**Goal**: Detect when player enters/exits interaction range

**Blueprint Nodes**:

```
Event ActorBeginOverlap (InteractionTrigger)
  ↓
Get Other Actor
  ↓
Cast to [Your Player Pawn Class]
  ├─ Success:
  │   ↓
  │ Print String: "Press E to Trade"
  │   ↓
  │ Enable Input (for this actor)
  │
  └─ Fail: (End)

Event ActorEndOverlap (InteractionTrigger)
  ↓
Get Other Actor
  ↓
Cast to [Your Player Pawn Class]
  ├─ Success:
  │   ↓
  │ Print String: ""
  │   ↓
  │ Disable Input (for this actor)
  │
  └─ Fail: (End)
```

**Detailed Instructions**:

1. In **Event Graph**, **select** `InteractionTrigger` in Components
2. In Details panel, scroll to **Events**
3. Click **+** next to **On Component Begin Overlap**
4. **Drag from Other Actor** and add **Cast To [YourPlayerPawn]**
5. From **Cast Success**:
   - Add **Print String**: "Press E to Trade" (On Screen, Duration 0)
   - Add **Enable Input**: Target = Self, Player Controller = Get Player Controller
6. **Repeat for End Overlap**:
   - Clear the prompt (Print String with empty text)
   - **Disable Input**: Target = Self

**Expected Result**: Walking near station shows prompt, walking away removes it.

---

### Step 4: Implement OpenTradingUI

**Goal**: Create and show the trading widget

**Blueprint Nodes**:

```
Function: OpenTradingUI
  Input: PlayerController
  ↓
Branch (Is CurrentTradingWidget Valid?)
  ├─ True: Already open, return
  └─ False: Continue
      ↓
    Create Widget
      ├─ Class: TradingUIClass
      └─ Owning Player: PlayerController
      ↓
    Set CurrentTradingWidget (variable)
      ↓
    Cast to TradingInterfaceWidget
      ↓
    Call: Open Market (MarketData)
      ↓
    Add to Viewport
      ↓
    Set Input Mode UI Only
      ├─ Target: PlayerController
      └─ Widget to Focus: CurrentTradingWidget
      ↓
    Set Show Mouse Cursor: True
```

**Detailed Instructions**:

1. **Create new function**: Graph → Add Function → `OpenTradingUI`
2. **Add input parameter**:
   - Name: `PlayerController`
   - Type: `Player Controller` (Object Reference)
3. **Check if already open**:
   - Add node: `Is Valid` (Target: `CurrentTradingWidget`)
   - Add node: `Branch`
   - If True: Add `Return Node` (stop here)
4. **Create widget** (False path):
   - Add node: `Create Widget`
   - Class: `TradingUIClass` (variable)
   - Owning Player: `PlayerController` (input parameter)
5. **Store reference**:
   - Add node: `Set CurrentTradingWidget` (variable)
   - Connect created widget to it
6. **Initialize trading** (if your UI extends C++ TradingInterfaceWidget):
   - Add node: `Cast to TradingInterfaceWidget`
   - From cast success: Call `Open Market`
   - Market: `MarketData` (variable)
7. **Show widget**:
   - Add node: `Add to Viewport`
   - Target: `CurrentTradingWidget`
8. **Set input mode**:
   - Add node: `Set Input Mode UI Only`
   - Player Controller: Input parameter
   - Widget to Focus: `CurrentTradingWidget`
9. **Show cursor**:
   - Add node: `Set Show Mouse Cursor`
   - Target: `PlayerController`
   - Value: `True`

**Expected Result**: Function is created and ready to call.

---

### Step 5: Implement CloseTradingUI

**Goal**: Remove the trading widget and return to game input

**Blueprint Nodes**:

```
Function: CloseTradingUI
  ↓
Branch (Is CurrentTradingWidget Valid?)
  ├─ False: Nothing to close, return
  └─ True: Continue
      ↓
    Remove from Parent (CurrentTradingWidget)
      ↓
    Set CurrentTradingWidget = None
      ↓
    Get Player Controller
      ↓
    Set Input Mode Game Only
      ↓
    Set Show Mouse Cursor: False
```

**Detailed Instructions**:

1. **Create new function**: `CloseTradingUI`
2. **Check if open**:
   - Add node: `Is Valid` (Target: `CurrentTradingWidget`)
   - Add node: `Branch`
   - If False: Add `Return Node`
3. **Remove widget** (True path):
   - Add node: `Remove from Parent`
   - Target: `CurrentTradingWidget`
4. **Clear reference**:
   - Add node: `Set CurrentTradingWidget` = `None`
5. **Restore input**:
   - Add node: `Get Player Controller`
   - Add node: `Set Input Mode Game Only`
   - Add node: `Set Show Mouse Cursor` = `False`

**Expected Result**: Function is created and ready to call.

---

### Step 6: Implement IsPlayerInRange

**Goal**: Check if player is close enough to interact

**Blueprint Nodes**:

```
Function: IsPlayerInRange
  Input: PlayerPawn
  Output: Boolean
  ↓
Get Actor Location (Self)
  ↓
Get Actor Location (PlayerPawn)
  ↓
Vector - Vector (get direction)
  ↓
Vector Length (get distance)
  ↓
<= (compare)
  ├─ A: Distance
  └─ B: InteractionRange (variable)
  ↓
Return Node (result)
```

**Detailed Instructions**:

1. **Create new function**: `IsPlayerInRange`
2. **Add input**:
   - Name: `PlayerPawn`
   - Type: `Pawn` (Object Reference)
3. **Set return type**: Boolean (in function details)
4. **Get positions**:
   - Add node: `Get Actor Location` (Target: `Self`)
   - Add node: `Get Actor Location` (Target: `PlayerPawn` input)
5. **Calculate distance**:
   - Add node: `Vector - Vector` (subtract positions)
   - Add node: `Vector Length` (get magnitude)
6. **Compare to range**:
   - Add node: `<=` (Less than or equal)
   - A: Distance (from previous step)
   - B: `InteractionRange` (variable)
7. **Return result**:
   - Add node: `Return Node`
   - Connect comparison result to return value

**Expected Result**: Function returns true if player is close enough.

---

### Step 7: Add Input Binding

**Goal**: Respond to player pressing E key

**Blueprint Nodes**:

```
Input Action E (or your interaction key)
  ↓
Get Player Pawn
  ↓
IsPlayerInRange (custom function)
  ↓
Branch
  ├─ True:
  │   ↓
  │ Get Player Controller
  │   ↓
  │ OpenTradingUI (custom function)
  │
  └─ False: (End)
```

**Detailed Instructions**:

1. In **Event Graph**, right-click
2. Search for your interaction input (e.g., "E" or "Interact")
   - If using Enhanced Input: `Input Action [YourInteractAction]`
   - If using legacy: `E` key event
3. **From input event**:
   - Add node: `Get Player Pawn`
   - Add node: `IsPlayerInRange` (custom function)
   - Input: Player Pawn from previous step
4. **Add node**: `Branch`
   - Condition: Result from IsPlayerInRange
5. **True path**:
   - Add node: `Get Player Controller`
   - Add node: `OpenTradingUI` (custom function)
   - Input: Player Controller from previous step

**Expected Result**: Pressing E near station opens trading UI.

---

### Step 8: Test in Editor

**Goal**: Verify station works in Play in Editor

**Instructions**:

1. **Place station** in a test level
2. **Select the station** in Outliner
3. In **Details panel**, set `MarketData` to your test market (e.g., `DA_Market_TestStation`)
4. **Compile and Save** the Blueprint
5. **Click Play** (PIE)
6. **Walk near the station** - should see prompt
7. **Press E** - trading UI should open
8. **Close UI** (if your UI has close button) - should return to game

**Expected Result**: Station interaction works end-to-end.

---

## 🧪 Testing Your Blueprint

### Test 1: Station Placement

**What to Test**: Station appears correctly in level

**Steps**:
1. Drag `BP_TradingStation` into level
2. Set `MarketData` in Details panel
3. Play in editor

**Success Criteria**:
- Station mesh appears
- No errors in log
- Collision trigger is active

**If it fails**: Check that StationMesh has a valid mesh assigned

---

### Test 2: Interaction Range

**What to Test**: Player can only interact when close enough

**Steps**:
1. Place station in test level
2. Play and walk toward station from far away
3. Note when prompt appears
4. Walk away and note when prompt disappears

**Success Criteria**:
- Prompt appears at approximately 500 units (or your InteractionRange value)
- Prompt disappears when leaving range
- Pressing E outside range does nothing

**If it fails**: Check InteractionTrigger size and IsPlayerInRange function

---

### Test 3: Trading UI Opens

**What to Test**: Pressing E opens trading interface

**Steps**:
1. Walk near station (until prompt shows)
2. Press E (or your interact key)
3. Check UI opens
4. Check market items display
5. Close UI

**Success Criteria**:
- Trading UI appears
- Market items from MarketData show in list
- Input switches to UI (can't move character)
- Mouse cursor appears

**If it fails**: See [Troubleshooting](#common-issue-1-ui-doesnt-open)

---

## 🐛 Troubleshooting

### Common Issue 1: UI Doesn't Open

**Symptoms**:
- Pressing E does nothing
- No errors in log

**Causes**:
- Input not bound correctly
- Player not in range
- MarketData not set
- TradingUIClass not set

**Solutions**:
1. Check Input binding exists (look for E key event in graph)
2. Print debug string in E key event to confirm it fires
3. Check `MarketData` is set in station instance (Details panel)
4. Check `TradingUIClass` is set to `WBP_TradingUI`
5. Verify `WBP_TradingUI` Blueprint exists and compiles

---

### Common Issue 2: Prompt Doesn't Show

**Symptoms**:
- Walking near station shows no "Press E" message
- Can't interact

**Causes**:
- InteractionTrigger collision not set up
- Overlap events not bound
- Cast to player pawn fails

**Solutions**:
1. Select `InteractionTrigger` component
2. In Details, check **Generate Overlap Events** is enabled
3. Check Box Extent is large enough (500, 500, 500)
4. In overlap event, change cast type to match YOUR player pawn class
5. Add Print String after overlap event to verify it fires

---

### Common Issue 3: Can't Return to Game

**Symptoms**:
- Trading UI closes but still can't move
- Mouse cursor stuck visible

**Causes**:
- Input mode not restored
- Cursor not hidden

**Solutions**:
1. In `CloseTradingUI` function, verify:
   - `Set Input Mode Game Only` is called
   - `Set Show Mouse Cursor` = False is called
2. Check `CloseTradingUI` is actually being called (add Print String)
3. Verify trading UI has a close button that calls this function

---

### Common Issue 4: Multiple UIs Open

**Symptoms**:
- Pressing E repeatedly opens multiple UIs
- UIs stack on top of each other

**Causes**:
- Not checking if UI already open
- CurrentTradingWidget not being set

**Solutions**:
1. In `OpenTradingUI`, verify first node checks `Is Valid (CurrentTradingWidget)`
2. Check that created widget is stored in `CurrentTradingWidget`
3. Ensure `CloseTradingUI` sets `CurrentTradingWidget = None`

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Station Name Display**
   - What it adds: Show station name in 3D space above station
   - Complexity: Simple
   - Guide: Add Text Render component with station name

2. **Docking Animation**
   - What it adds: Play animation when player interacts
   - Complexity: Medium
   - Guide: Add Animation Blueprint and trigger on interaction

3. **Multiple Trading Terminals**
   - What it adds: Multiple interaction points on large stations
   - Complexity: Medium
   - Guide: Add multiple InteractionTrigger components

4. **Faction-Based Access**
   - What it adds: Only allow friendly factions to trade
   - Complexity: Medium
   - Guide: Check faction relationship before opening UI

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) - The UI this station opens
- [Ship Cargo System](ShipCargoSystemGuide.md) - Managing player inventory
- [Market Data Asset Setup](../Assets/TradingTemplates/Market_Template.yaml) - Creating markets
- [Space Station System](../docs/systems/STATION_EDITOR_COMPLETE_GUIDE.md) - Advanced station features

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a Blueprint extending a C++ actor class
- ✅ How to add and configure components
- ✅ How to implement overlap detection for player interaction
- ✅ How to create and call custom functions
- ✅ How to manage UI widget lifecycle (create, show, hide, destroy)
- ✅ How to switch between game and UI input modes

**Next Recommended Blueprint**: Ship Cargo System - To complete the trading loop

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Change `InteractionRange` per station type
- Add different market types (black market, premium, etc.)
- Customize interaction prompt per station
- Add faction-based pricing modifiers

**Blueprint Best Practices Applied:**
- Used custom functions to avoid code duplication
- Cached widget reference to prevent duplicate UIs
- Separated interaction detection from UI logic
- Used IsValid checks before accessing widgets

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-03 | Initial guide following new template standard |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [Blueprint Image Guide](../docs/reference/BLUEPRINT_IMAGE_GUIDE.md) for node diagrams
- See [Blueprints README](README.md) for more guides
