# Ship Cargo System - Blueprint Guide

> **⚠️ DEPRECATED**: This guide has been consolidated into the unified Trading and Docking Complete Guide.  
> **See**: [Trading and Docking Complete Guide](../docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)

---

> **Add cargo management to player ship for storing traded items**

**Blueprint Type**: Actor Component Blueprint  
**Complexity**: Beginner  
**Time to Complete**: 25-35 minutes  
**Last Updated**: 2026-01-03  
**Status**: **DEPRECATED** - Refer to unified guide above

---

## 📋 Overview

### What You'll Build

A cargo component attached to the player's ship that tracks inventory of trade items. The component manages adding, removing, and querying items in the ship's cargo hold.

**End Result**: Player ship can store items bought from stations, check available cargo space, and sell items from inventory.

### Why This Blueprint Matters

Cargo management is essential for the MVP trading loop. Without it, players can't actually carry the items they buy and sell.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [x] `UCargoComponent` C++ class compiled (TradingSystem module)
- [x] At least one `UTradeItemDataAsset` created (e.g., `DA_Item_Water`)
- [x] Player ship Blueprint (e.g., `BP_PlayerShip`)
- [x] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Trading System Guide](../Assets/TradingSystemGuide.md) - For creating Trade Item Data Assets
- [Ship Control Guide](../Assets/ShipControlGuide.md) - For basic ship setup

---

## 🔗 Dependencies

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UCargoComponent` | TradingSystem | Cargo inventory management functions |
| `UTradeItemDataAsset` | TradingSystem | Trade item definitions |

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_PlayerShip` | Ship that owns cargo | `Content/Ships/Player/` |

---

## 🎯 Setup Checklist

### Step 1: Open Player Ship Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/Ships/Player/` (or your ship location)
3. **Double-click** `BP_PlayerShip` to open it
4. If you don't have a player ship yet, create one:
   - Right-click → Blueprint Class → Pawn
   - Name it: `BP_PlayerShip`
   - Add basic ship components (mesh, movement, camera)

### Step 2: Add Cargo Component

1. In the **Components** panel, click **Add Component**
2. Search for: `Cargo Component`
3. **Select** it to add to the ship
4. **Rename** it (optional): `ShipCargo`

---

## 📦 Variables to Create

**Create these variables in your SHIP Blueprint (not the component):**

### Variable 1: CurrentCargoSpace

- **Type**: `Float`
- **Category**: `Cargo Display`
- **Instance Editable**: No
- **Tooltip**: "Current amount of cargo space used (for UI display)"
- **Default Value**: `0.0`

### Variable 2: MaxCargoSpace

- **Type**: `Float`
- **Category**: `Cargo Display`
- **Instance Editable**: No
- **Tooltip**: "Maximum cargo capacity (for UI display)"
- **Default Value**: `0.0` (will be read from component)

---

## 🔧 Functions to Create

**Create these functions in your SHIP Blueprint:**

### Function 1: GetCargoComponent

**Purpose**: Returns reference to the ship's cargo component

**Inputs**: None

**Outputs**:
- `Return Value` (Cargo Component Reference)

**Implementation**: See [Step 3](#step-3-implement-getcargocomponent) below

### Function 2: UpdateCargoDisplay

**Purpose**: Updates cargo space variables for UI

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 4](#step-4-implement-updatecargodisplay) below

### Function 3: CanAddItem

**Purpose**: Checks if item fits in cargo hold

**Inputs**:
- `Item` (Trade Item Data Asset) - The item to check
- `Quantity` (Integer) - Number of units

**Outputs**:
- `Return Value` (Boolean) - True if item fits

**Implementation**: See [Step 5](#step-5-implement-canadditem) below

---

## 🛠️ Step-by-Step Implementation

### Step 1: Configure Cargo Component

**Goal**: Set the cargo capacity for the ship

**Instructions**:

1. In **Components** panel, **select** `Cargo Component` (or `ShipCargo`)
2. In **Details** panel, find **Cargo** category
3. Set **Cargo Capacity** to desired value (e.g., `100.0` for starter ship)
4. This is the maximum volume your ship can carry

**Expected Result**: Cargo component has a defined capacity.

---

### Step 2: Initialize Cargo on Begin Play

**Goal**: Set up cargo variables when ship spawns

**Blueprint Nodes**:

```
Event BeginPlay
  ↓
Get Component by Class (Cargo Component)
  ↓
Set MaxCargoSpace = Cargo Capacity (from component)
  ↓
Set CurrentCargoSpace = 0.0
```

**Detailed Instructions**:

1. In **Event Graph**, locate `Event BeginPlay`
2. **Add node**: `Get Component by Class`
   - Component Class: `Cargo Component`
3. **Add node**: `Get Cargo Capacity`
   - Target: Result from previous node
4. **Add node**: `Set MaxCargoSpace` (variable)
   - Connect capacity value to this
5. **Add node**: `Set CurrentCargoSpace` = `0.0`

**Expected Result**: Ship initializes cargo variables on spawn.

---

### Step 3: Implement GetCargoComponent

**Goal**: Create reusable function to access cargo component

**Blueprint Nodes**:

```
Function: GetCargoComponent
  ↓
Get Component by Class (Cargo Component)
  ↓
Return Node
```

**Detailed Instructions**:

1. **Create new function**: Graph → Add Function → `GetCargoComponent`
2. **Set return type**: `Cargo Component` (Object Reference)
3. **Add node**: `Get Component by Class`
   - Component Class: `Cargo Component`
4. **Add node**: `Return Node`
   - Connect component reference to return value

**Expected Result**: Function returns cargo component reference.

---

### Step 4: Implement UpdateCargoDisplay

**Goal**: Update cargo space variables for UI display

**Blueprint Nodes**:

```
Function: UpdateCargoDisplay
  ↓
GetCargoComponent (custom function)
  ↓
Get Cargo Space Used (C++ function)
  └─ Set CurrentCargoSpace (variable)
  ↓
Get Cargo Capacity (C++ function)
  └─ Set MaxCargoSpace (variable)
```

**Detailed Instructions**:

1. **Create new function**: `UpdateCargoDisplay`
2. **Add node**: `GetCargoComponent` (custom function)
3. **Add node**: `Get Cargo Space Used`
   - Target: Result from GetCargoComponent
   - This is a C++ function from UCargoComponent
4. **Add node**: `Set CurrentCargoSpace` (variable)
   - Connect used space to this
5. **Add node**: `Get Cargo Capacity`
   - Target: Cargo component
6. **Add node**: `Set MaxCargoSpace` (variable)
   - Connect capacity to this

**Expected Result**: Function updates cargo display variables.

---

### Step 5: Implement CanAddItem

**Goal**: Check if item will fit before adding

**Blueprint Nodes**:

```
Function: CanAddItem
  Inputs: Item, Quantity
  ↓
GetCargoComponent
  ↓
Get Item Volume (from Item Data Asset)
  ↓
Multiply (Volume × Quantity)
  └─ Total Volume Needed
  ↓
Get Cargo Space Used
  ↓
Get Cargo Capacity
  ↓
Subtract (Capacity - Used)
  └─ Available Space
  ↓
>= (Available >= Needed?)
  ↓
Return Node (result)
```

**Detailed Instructions**:

1. **Create new function**: `CanAddItem`
2. **Add inputs**:
   - `Item`: `Trade Item Data Asset` (Object Reference)
   - `Quantity`: `Integer`
3. **Set return type**: `Boolean`
4. **Get volume needed**:
   - Add node: `Get Volume Per Unit` (from Item)
   - Add node: `Multiply` (Volume × Quantity)
5. **Get available space**:
   - Add node: `GetCargoComponent` (custom function)
   - Add node: `Get Cargo Space Used`
   - Add node: `Get Cargo Capacity`
   - Add node: `Subtract` (Capacity - Used)
6. **Compare**:
   - Add node: `>=` (Available >= Needed)
7. **Return result**:
   - Add node: `Return Node`
   - Connect comparison to return value

**Expected Result**: Function returns true if item fits.

---

### Step 6: Implement Add Item Flow

**Goal**: Add items to cargo when purchased

**Blueprint Nodes**:

```
Custom Event: OnItemPurchased
  Inputs: Item, Quantity
  ↓
CanAddItem (custom function)
  ↓
Branch
  ├─ True:
  │   ↓
  │ GetCargoComponent
  │   ↓
  │ Add Cargo (C++ function)
  │   ├─ Item: Input
  │   └─ Quantity: Input
  │   ↓
  │ UpdateCargoDisplay
  │   ↓
  │ Print String: "Added X to cargo"
  │
  └─ False:
      ↓
    Print String: "Not enough cargo space!"
```

**Detailed Instructions**:

1. **Create custom event**: Right-click → Add Custom Event → `OnItemPurchased`
2. **Add inputs**:
   - `Item`: `Trade Item Data Asset`
   - `Quantity`: `Integer`
3. **Check space**:
   - Add node: `CanAddItem` (custom function)
   - Inputs: Item, Quantity from event
4. **Add node**: `Branch`
   - Condition: Result from CanAddItem
5. **True path (has space)**:
   - Add node: `GetCargoComponent`
   - Add node: `Add Cargo` (C++ function from component)
   - Item: Input from event
   - Quantity: Input from event
   - Add node: `UpdateCargoDisplay`
   - Add node: `Print String`: "Added to cargo"
6. **False path (no space)**:
   - Add node: `Print String`: "Not enough cargo space!"

**Expected Result**: Event adds items to cargo or shows error.

---

### Step 7: Implement Remove Item Flow

**Goal**: Remove items from cargo when sold

**Blueprint Nodes**:

```
Custom Event: OnItemSold
  Inputs: Item, Quantity
  ↓
GetCargoComponent
  ↓
Has Item (C++ function)
  ↓
Branch
  ├─ True:
  │   ↓
  │ Remove Cargo (C++ function)
  │   ├─ Item: Input
  │   └─ Quantity: Input
  │   ↓
  │ UpdateCargoDisplay
  │   ↓
  │ Print String: "Removed X from cargo"
  │
  └─ False:
      ↓
    Print String: "Don't have that item!"
```

**Detailed Instructions**:

1. **Create custom event**: `OnItemSold`
2. **Add inputs**: Same as OnItemPurchased
3. **Check if has item**:
   - Add node: `GetCargoComponent`
   - Add node: `Has Item` (C++ function)
   - Item: Input from event
4. **Add node**: `Branch`
5. **True path (has item)**:
   - Add node: `Remove Cargo` (C++ function)
   - Item and Quantity from event inputs
   - Add node: `UpdateCargoDisplay`
   - Add node: `Print String`: "Removed from cargo"
6. **False path (doesn't have)**:
   - Add node: `Print String`: "Don't have that item!"

**Expected Result**: Event removes items from cargo or shows error.

---

### Step 8: Connect to Trading UI

**Goal**: Link cargo events to actual trading

**Instructions**:

1. In your **Trading UI Blueprint** (`WBP_TradingUI`):
   - After successful purchase: Call `OnItemPurchased` on player ship
   - After successful sale: Call `OnItemSold` on player ship
2. Get player ship reference:
   - `Get Player Pawn` → Cast to `BP_PlayerShip`
3. Call events:
   - `OnItemPurchased` or `OnItemSold`
   - Pass Item Data Asset and Quantity

**Note**: The exact implementation depends on your Trading UI structure. The key is calling these events after successful transactions.

**Expected Result**: Trading UI triggers cargo changes.

---

## 🧪 Testing Your Blueprint

### Test 1: Cargo Component Exists

**What to Test**: Ship has cargo component properly attached

**Steps**:
1. Open `BP_PlayerShip`
2. Check Components panel
3. Select Cargo Component
4. Check Details shows Cargo Capacity

**Success Criteria**:
- Cargo Component is present
- Cargo Capacity is set (e.g., 100.0)
- No errors or warnings

**If it fails**: Re-add the Cargo Component following Step 1

---

### Test 2: Cargo Capacity Initializes

**What to Test**: Cargo variables initialize on spawn

**Steps**:
1. Place `BP_PlayerShip` in test level
2. Add Print String nodes after BeginPlay initialization
3. Play in editor
4. Check log/screen for printed values

**Success Criteria**:
- MaxCargoSpace equals component's Cargo Capacity
- CurrentCargoSpace starts at 0
- No null reference errors

**If it fails**: See [Troubleshooting](#common-issue-1-cargo-variables-not-initializing)

---

### Test 3: Can Add Items

**What to Test**: CanAddItem function works correctly

**Steps**:
1. In Event Graph, add test code:
   - Event BeginPlay
   - Delay 1 second
   - Load a test Trade Item (e.g., DA_Item_Water)
   - Call CanAddItem (Item, Quantity = 10)
   - Print result to screen
2. Play in editor
3. Check if function returns expected result

**Success Criteria**:
- With empty cargo: Returns True (items fit)
- With full cargo: Returns False (no space)
- Math is correct (volume × quantity checked properly)

**If it fails**: Check CanAddItem function logic and volume calculations

---

### Test 4: Add and Remove Items

**What to Test**: Can add/remove items to/from cargo

**Steps**:
1. Add test code in Event Graph:
   - Call OnItemPurchased (test item, quantity 5)
   - Delay 2 seconds
   - Call OnItemSold (same item, quantity 5)
2. Play in editor
3. Watch screen messages

**Success Criteria**:
- "Added to cargo" message appears
- CurrentCargoSpace increases
- "Removed from cargo" message appears
- CurrentCargoSpace decreases back to 0

**If it fails**: See [Troubleshooting](#common-issue-2-items-not-being-added)

---

## 🐛 Troubleshooting

### Common Issue 1: Cargo Variables Not Initializing

**Symptoms**:
- MaxCargoSpace and CurrentCargoSpace are 0
- No cargo capacity shown

**Causes**:
- Cargo Component not added to ship
- BeginPlay initialization not running
- GetComponentByClass failing

**Solutions**:
1. Check Components panel - Cargo Component should be visible
2. In Event Graph BeginPlay, add Print String to verify it runs
3. After GetComponentByClass, add IsValid check:
   - If Invalid: Component isn't on the actor
   - If Valid: Continue with initialization
4. Verify Cargo Capacity is set on component (not 0)

---

### Common Issue 2: Items Not Being Added

**Symptoms**:
- OnItemPurchased called but cargo stays empty
- No error messages

**Causes**:
- AddCargo C++ function not found
- Item or Quantity parameter is null/0
- Component reference is invalid

**Solutions**:
1. Verify UCargoComponent C++ class is compiled
2. Add Print String with Item name and Quantity to verify inputs
3. Add IsValid check on GetCargoComponent result
4. Check that AddCargo function exists in function list:
   - Drag from cargo component
   - Search "Add Cargo"
   - Should see function from C++

---

### Common Issue 3: Cargo Space Calculation Wrong

**Symptoms**:
- Items added but space doesn't decrease
- Can add infinite items
- Space calculation seems off

**Causes**:
- UpdateCargoDisplay not being called
- Volume Per Unit not set on Trade Items
- GetCargoSpaceUsed returning wrong value

**Solutions**:
1. Call UpdateCargoDisplay after every AddCargo/RemoveCargo
2. Open your Trade Item Data Assets (e.g., DA_Item_Water)
3. Check Volume Per Unit is set (e.g., 1.0 for water)
4. Test GetCargoSpaceUsed with Print String:
   - Before adding item: Should be 0
   - After adding: Should be Volume × Quantity

---

### Common Issue 4: Can't Remove Items

**Symptoms**:
- Items added successfully
- OnItemSold doesn't remove them
- "Don't have item" message even though added

**Causes**:
- HasItem check failing
- Item reference doesn't match
- RemoveCargo not working

**Solutions**:
1. Add Print String in OnItemSold to show Item name
2. Compare with Item name from OnItemPurchased - should match exactly
3. Use same Item Data Asset reference for add and remove
4. Check RemoveCargo C++ function:
   - Should return Boolean (success/failure)
   - Print the return value to debug

---

## ✨ Optional Enhancements

**After the basic system works, consider adding:**

1. **Cargo Manifest UI**
   - What it adds: Visual list of items in cargo
   - Complexity: Medium
   - Guide: Create widget showing all cargo entries with quantities

2. **Item Stacking**
   - What it adds: Combine identical items in cargo
   - Complexity: Simple (already handled by C++ if using FCargoEntry)
   - Guide: Verify AddCargo increments quantity for existing items

3. **Cargo Weight System**
   - What it adds: Items have weight affecting ship speed
   - Complexity: Medium
   - Guide: Read total weight from cargo, apply to ship movement

4. **Cargo Jettison**
   - What it adds: Dump cargo in emergencies
   - Complexity: Simple
   - Guide: Add RemoveCargo call with "jettison" option

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading UI Widget](TradingSystemBlueprintGuide_SIMPLIFIED.md) - UI that adds/removes cargo
- [Trading Station Actor](TradingStationActorGuide.md) - Stations where trading happens
- [Trade Item Setup](../Assets/TradingTemplates/TradeItem_Template.yaml) - Creating trade items
- [Cargo Component C++ API](../Source/Adastrea/Public/Trading/CargoComponent.h) - Full C++ reference

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to add a component to an existing Blueprint
- ✅ How to call C++ functions from a component
- ✅ How to create helper functions for cleaner code
- ✅ How to implement validation (CanAddItem check)
- ✅ How to create custom events for other Blueprints to call
- ✅ How to manage inventory data structures

**Next Recommended Blueprint**: Docking UI - To complete the player-station interaction

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Adjust Cargo Capacity per ship type (small/medium/large)
- Add specialized cargo holds (refrigerated, hazardous, etc.)
- Implement cargo organization (containers, sections)
- Add cargo insurance or protection systems

**Blueprint Best Practices Applied:**
- Used component architecture (cargo as separate component)
- Created reusable helper functions (GetCargoComponent)
- Implemented validation before operations (CanAddItem)
- Used custom events for cross-Blueprint communication

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-03 | Initial guide following new template standard |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review C++ source: `Source/Adastrea/Public/Trading/CargoComponent.h`
- See [Blueprints README](README.md) for more guides
