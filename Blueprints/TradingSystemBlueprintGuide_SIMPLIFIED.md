# Trading UI Widget - Blueprint Guide

> **Create a functional trading interface for buying and selling items at stations**

**Blueprint Type**: Widget Blueprint  
**Complexity**: Intermediate  
**Time to Complete**: 45-60 minutes  
**Last Updated**: 2026-01-03

---

## 📋 Overview

### What You'll Build

A trading UI that allows players to buy items from a space station's market and sell items from their cargo hold. The interface shows available items, prices, player credits, and cargo space.

**End Result**: Players can click a "Trade" button at a station, see a market interface, and complete buy/sell transactions.

### Why This Blueprint Matters

Trading is the core gameplay loop of the MVP. This UI is essential for the 30-minute playable demo.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [x] `UTradingInterfaceWidget` C++ class compiled (TradingSystem module)
- [x] At least one `UMarketDataAsset` created (e.g., `DA_Market_TestStation`)
- [x] At least three `UTradeItemDataAsset` created (e.g., `DA_Item_Water`, `DA_Item_Food`, `DA_Item_Fuel`)
- [x] Unreal Engine 5.6+

**If you're missing any prerequisites**, see:
- [Trading System Setup Guide](../Assets/TradingSystemGuide.md) - For creating Data Assets
- [MVP Quick Start](../docs/mvp/README.md) - For building the TradingSystem module

---

## 🔗 Dependencies

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UTradingInterfaceWidget` | TradingSystem | `OpenMarket()`, `GetAvailableItems()`, `AddToCart()`, `ExecuteTrade()` |
| `UMarketDataAsset` | TradingSystem | Market inventory and pricing data |
| `UTradeItemDataAsset` | TradingSystem | Item properties (name, price, volume) |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/UI/Trading/`
3. **Right-click** → User Interface → Widget Blueprint
4. **Choose Parent Class**: `UserWidget`
5. **Name it**: `WBP_TradingUI`
6. **Double-click** to open the Blueprint Editor

### Step 2: Configure Class Settings

1. Click **Class Settings** (toolbar button)
2. **Reparent Blueprint**: File → Reparent Blueprint → Search for `TradingInterfaceWidget` → Select it
3. This gives you access to all C++ trading functions

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: CurrentMarket

- **Type**: `Market Data Asset` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "The market currently being traded with"
- **Default Value**: None (set at runtime)

### Variable 2: IsBuyingMode

- **Type**: `Boolean`
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "True = buying from market, False = selling to market"
- **Default Value**: `True`

### Variable 3: PlayerCredits

- **Type**: `Integer`
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "Player's current money"
- **Default Value**: `0` (set at runtime)

### Variable 4: SelectedItem

- **Type**: `Trade Item Data Asset` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "Currently selected item for transaction"
- **Default Value**: None

---

## 🔧 Functions to Create

**Create these custom functions:**

### Function 1: RefreshItemList

**Purpose**: Updates the UI to show current market inventory

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 3](#step-3-implement-refreshitemlist) below

### Function 2: UpdatePlayerInfo

**Purpose**: Updates credits and cargo space display

**Inputs**: None

**Outputs**: None

**Implementation**: See [Step 4](#step-4-implement-updateplayerinfo) below

---

## 📐 Widget Layout

### Visual Hierarchy

```
Canvas Panel (Root)
├── Background Panel (Image - Dark overlay)
├── Main Container (Horizontal Box)
│   ├── Left Panel (Market Items)
│   │   ├── Header Text: "Market Name"
│   │   ├── Buy/Sell Toggle Buttons
│   │   └── Item List (Scroll Box)
│   │       └── Name: "ItemListScrollBox"
│   └── Right Panel (Player Info)
│       ├── Credits Display (Text)
│       ├── Cargo Space Bar (Progress Bar)
│       └── Trade Button
└── Close Button (Top right)
```

### Key Widget Names (Important!)

| Widget | Name in Hierarchy | Bound Variable |
|--------|-------------------|----------------|
| Text Block | `Text_MarketName` | (None - set in code) |
| Text Block | `Text_PlayerCredits` | (None - set in code) |
| Progress Bar | `ProgressBar_CargoSpace` | (None - set in code) |
| Scroll Box | `ItemListScrollBox` | (None - reference in code) |
| Button | `Button_Trade` | (None - bind OnClicked event) |
| Button | `Button_Close` | (None - bind OnClicked event) |
| Button | `Button_BuyMode` | (None - bind OnClicked event) |
| Button | `Button_SellMode` | (None - bind OnClicked event) |

---

## 🛠️ Step-by-Step Implementation

### Step 1: Design the Widget Layout

**Goal**: Create the visual structure of the trading UI

**Instructions**:

1. In the **Designer** tab, drag a **Canvas Panel** to the hierarchy (root)
2. **Add Background**:
   - Drag an **Image** onto Canvas Panel
   - Rename it: `Image_Background`
   - Set Anchors: Fill screen (0,0 to 1,1)
   - Set Color: Black with Alpha 0.8
3. **Add Main Container**:
   - Drag a **Horizontal Box** onto Canvas Panel
   - Rename it: `HorizontalBox_MainContainer`
   - Set Position: Centered (960, 540)
   - Set Size: 1600 x 900
4. **Add Left Panel** (to Horizontal Box):
   - Drag a **Vertical Box**
   - Rename: `VerticalBox_LeftPanel`
   - Slot Settings: Fill, Weight 1.5
5. **Add Right Panel** (to Horizontal Box):
   - Drag a **Vertical Box**
   - Rename: `VerticalBox_RightPanel`
   - Slot Settings: Fill, Weight 1.0
6. **Add widgets to Left Panel**:
   - Text Block: `Text_MarketName` (Font size 32)
   - Horizontal Box with two buttons:
     - Button: `Button_BuyMode` (Text: "BUY")
     - Button: `Button_SellMode` (Text: "SELL")
   - Scroll Box: `ItemListScrollBox`
7. **Add widgets to Right Panel**:
   - Text Block: `Text_PlayerCredits` (Font size 28)
   - Progress Bar: `ProgressBar_CargoSpace`
   - Button: `Button_Trade` (Text: "CONFIRM TRADE")

**Expected Result**: A basic layout that looks like a trading interface.

---

### Step 2: Initialize on Construct

**Goal**: Set up the UI when the widget is created

**Blueprint Nodes**:

```
Event Construct
  ↓
Get Player Controller
  ↓
Cast to [YourPlayerControllerClass]
  ↓
Get Player Credits
  └─ Set PlayerCredits (variable)
  ↓
Call RefreshItemList (custom function)
  ↓
Call UpdatePlayerInfo (custom function)
```

**Detailed Instructions**:

1. In the **Event Graph**, locate `Event Construct`
2. **Drag from execution pin** and add **Get Player Controller**
3. **Cast to your player controller class** (e.g., `BP_PlayerController`)
4. From the casted controller, **get the player's credits**
   - This might be a custom function like `GetCredits()`
   - Store result in `PlayerCredits` variable
5. **Call your custom function** `RefreshItemList`
6. **Call your custom function** `UpdatePlayerInfo`

**Expected Result**: When the widget opens, it shows player info and market items.

---

### Step 3: Implement RefreshItemList

**Goal**: Populate the item list with market inventory

**Blueprint Nodes**:

```
Function: RefreshItemList
  ↓
Clear Children (ItemListScrollBox)
  ↓
Get Available Items (from C++ parent class)
  ↓
ForEach Loop (Items array)
  ├─ Loop Body:
  │   ↓
  │ Create Widget (WBP_TradeItemRow)
  │   ↓
  │ Set Item Data (on item row widget)
  │   ├─ Item Name
  │   ├─ Item Price
  │   └─ Current Stock
  │   ↓
  │ Bind OnClick Event (to SelectItem function)
  │   ↓
  │ Add Child to ItemListScrollBox
  │
  └─ Completed: (End)
```

**Detailed Instructions**:

1. **Create a new function**: Graph → Add Function → Name it `RefreshItemList`
2. **Add node**: `Clear Children`
   - Target: `ItemListScrollBox` (drag from variable)
3. **Add node**: `Get Available Items`
   - This is inherited from `UTradingInterfaceWidget` C++ parent
   - Returns: Array of `FMarketInventoryEntry`
4. **Add node**: `ForEachLoop`
   - Connect array from previous node to input
5. **In Loop Body**:
   - Add node: `Create Widget`
   - Class: `WBP_TradeItemRow` (you'll create this separately)
   - Owning Player: Get Player Controller
6. **Set item data** on the created widget:
   - Call `SetItemName` (passing `Array Element → Trade Item → Item Name`)
   - Call `SetItemPrice` (passing `Array Element → Current Price`)
   - Call `SetStock` (passing `Array Element → Current Stock`)
7. **Bind click event**: Call `Bind Event to OnItemClicked`
   - Event: Create custom event `SelectItem`
8. **Add to scroll box**: Call `Add Child` on `ItemListScrollBox`
   - Content: The created widget

**Expected Result**: The item list shows all market items with names and prices.

---

### Step 4: Implement UpdatePlayerInfo

**Goal**: Show player's credits and cargo space

**Blueprint Nodes**:

```
Function: UpdatePlayerInfo
  ↓
Format Text: "${0}" with PlayerCredits
  └─ Set Text (Text_PlayerCredits)
  ↓
Get Player Cargo Space Used
  ↓
Get Player Max Cargo Space
  ↓
Divide (Used / Max)
  └─ Set Percent (ProgressBar_CargoSpace)
  ↓
Branch (Cargo > 80%?)
  ├─ True: Set Fill Color to Red
  └─ False: Set Fill Color to Green
```

**Detailed Instructions**:

1. **Create a new function**: `UpdatePlayerInfo`
2. **Format credits display**:
   - Add node: `Format Text`
   - Format: `"Credits: ${0}"`
   - Parameter 0: `PlayerCredits` variable
   - Connect to `Set Text` on `Text_PlayerCredits`
3. **Get cargo data** from player controller:
   - Call `GetCargoSpaceUsed()` (custom function on your player)
   - Call `GetMaxCargoSpace()` (custom function)
4. **Calculate percentage**:
   - Add node: `Divide (float)`
   - A: Cargo Used
   - B: Max Cargo
   - Connect to `Set Percent` on `ProgressBar_CargoSpace`
5. **Set color based on fullness**:
   - Add node: `Branch`
   - Condition: `Cargo Used / Max Cargo > 0.8`
   - True: Set Fill Color to `(1, 0, 0, 1)` (Red)
   - False: Set Fill Color to `(0, 1, 0, 1)` (Green)

**Expected Result**: UI shows current credits and cargo space visually.

---

### Step 5: Implement Buy/Sell Toggle

**Goal**: Switch between buying and selling modes

**Blueprint Nodes**:

```
Button_BuyMode → OnClicked
  ↓
Set IsBuyingMode = True
  ↓
Call RefreshItemList

Button_SellMode → OnClicked
  ↓
Set IsBuyingMode = False
  ↓
Call RefreshItemList
```

**Detailed Instructions**:

1. **Select** `Button_BuyMode` in the Designer
2. In **Details panel**, scroll to **Events**
3. Click **+** next to **On Clicked**
4. In the Event Graph:
   - Set `IsBuyingMode = True`
   - Call `RefreshItemList`
5. **Repeat for** `Button_SellMode`:
   - Set `IsBuyingMode = False`
   - Call `RefreshItemList`

**Expected Result**: Clicking Buy/Sell changes the item list shown.

---

### Step 6: Implement Item Selection

**Goal**: Allow player to select an item to trade

**Blueprint Nodes**:

```
Custom Event: SelectItem
  Input: SelectedItemData (TradeItemDataAsset)
  ↓
Set SelectedItem = SelectedItemData
  ↓
Highlight Item in List (visual feedback)
  ↓
Enable Button_Trade
```

**Detailed Instructions**:

1. **Create custom event**: Right-click → Add Custom Event → `SelectItem`
2. **Add input parameter**:
   - Name: `SelectedItemData`
   - Type: `Trade Item Data Asset` (Object Reference)
3. **Set variable**: `Set SelectedItem` = `SelectedItemData` (input)
4. **Enable trade button**: `Set Is Enabled` on `Button_Trade` = `True`
5. **Optional**: Add visual feedback (highlight selected row)

**Expected Result**: Clicking an item enables the trade button.

---

### Step 7: Implement Trade Execution

**Goal**: Complete the buy/sell transaction

**Blueprint Nodes**:

```
Button_Trade → OnClicked
  ↓
Branch (SelectedItem valid?)
  ├─ False: Show Error Message
  └─ True: Continue
      ↓
    Branch (IsBuyingMode?)
      ├─ True: Call BuyItem (from C++ parent)
      └─ False: Call SellItem (from C++ parent)
      ↓
    Branch (Transaction Successful?)
      ├─ True:
      │   ↓
      │ Update PlayerCredits
      │   ↓
      │ Play Success Sound
      │   ↓
      │ Refresh UI
      │
      └─ False: Show Error Message
```

**Detailed Instructions**:

1. **Select** `Button_Trade` and add **OnClicked** event
2. **Check item selected**:
   - Add node: `Is Valid` (Target: `SelectedItem`)
   - Add node: `Branch`
   - If False: Call `Print String` with "No item selected"
3. **Check buy/sell mode**:
   - Add node: `Branch` (Condition: `IsBuyingMode`)
4. **If buying**:
   - Call `Add To Cart` (inherited from C++ parent)
   - Inputs: `SelectedItem`, Quantity: `1`
   - Then call `Execute Trade`
5. **If selling**:
   - Call `Sell Item` (if you have this function)
   - Or use `Add To Cart` with negative quantity
6. **Check result**:
   - `Execute Trade` returns `Boolean` (success/failure)
   - Add node: `Branch`
   - If True: Update UI, play sound
   - If False: Show error message
7. **Update UI**:
   - Call `RefreshItemList`
   - Call `UpdatePlayerInfo`

**Expected Result**: Clicking "CONFIRM TRADE" executes the transaction and updates the UI.

---

### Step 8: Implement Close Button

**Goal**: Allow player to close the trading UI

**Blueprint Nodes**:

```
Button_Close → OnClicked
  ↓
Remove from Parent (this widget)
  ↓
Set Input Mode Game Only
  ↓
Set Show Mouse Cursor = False
```

**Detailed Instructions**:

1. **Select** `Button_Close` and add **OnClicked** event
2. **Add node**: `Remove from Parent`
   - Target: `Self`
3. **Add node**: `Set Input Mode Game Only`
   - Target: Get Player Controller
4. **Add node**: `Set Show Mouse Cursor`
   - Target: Get Player Controller
   - Value: `False`

**Expected Result**: Clicking close removes the UI and returns to gameplay.

---

## 🧪 Testing Your Blueprint

### Test 1: UI Opens and Shows Data

**What to Test**: Widget displays market items correctly

**Steps**:
1. Create a test map with a station
2. Add a trigger to open the trading UI
3. Set the `CurrentMarket` variable to your test market
4. Play in editor (PIE)
5. Trigger the UI

**Success Criteria**: 
- UI opens without errors
- Market items are visible
- Player credits display
- Cargo bar shows

**If it fails**: Check that Data Assets are valid and set correctly

---

### Test 2: Buy Transaction

**What to Test**: Player can buy an item

**Steps**:
1. Open trading UI in PIE
2. Ensure you're in "BUY" mode
3. Click on an item (e.g., Water)
4. Click "CONFIRM TRADE"
5. Check player credits decreased
6. Check cargo space increased

**Success Criteria**:
- Credits decrease by item price
- Item added to cargo
- UI updates to show new values

**If it fails**: See [Troubleshooting](#common-issue-1-transaction-fails)

---

### Test 3: Sell Transaction

**What to Test**: Player can sell an item

**Steps**:
1. Ensure player has items in cargo
2. Open trading UI
3. Click "SELL" button
4. Select an item player owns
5. Click "CONFIRM TRADE"
6. Check credits increased

**Success Criteria**:
- Credits increase by sell price
- Item removed from cargo
- UI updates

---

## 🐛 Troubleshooting

### Common Issue 1: Transaction Fails

**Symptoms**:
- "CONFIRM TRADE" does nothing
- Error message in log: "Invalid market"

**Causes**:
- `CurrentMarket` variable not set
- Market Data Asset is null

**Solutions**:
1. In your station Blueprint, ensure you call `OpenMarket(MarketDataAsset)` when player docks
2. Check that the Market Data Asset is valid and has inventory entries
3. Add `Print String` nodes to debug which variable is null

---

### Common Issue 2: Item List is Empty

**Symptoms**:
- UI opens but shows no items
- Scroll box is empty

**Causes**:
- Market Data Asset has empty inventory array
- `GetAvailableItems()` returns empty array

**Solutions**:
1. Open your Market Data Asset in editor
2. Check the `Inventory` array has entries
3. Ensure each entry has a valid `Trade Item` reference
4. Set `Current Stock` > 0 for items

---

### Common Issue 3: Cargo Space Not Updating

**Symptoms**:
- Progress bar doesn't change
- Cargo display stuck at 0%

**Causes**:
- Player controller doesn't implement cargo functions
- Functions return hardcoded values

**Solutions**:
1. Ensure your player controller has `GetCargoSpaceUsed()` and `GetMaxCargoSpace()` functions
2. Check these functions return actual values, not 0
3. Add a cargo component to your player ship if missing

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Quantity Selector**
   - What it adds: Buy/sell multiple items at once
   - Complexity: Medium
   - Guide: Add a Spinner widget and multiply transaction by quantity

2. **Shopping Cart**
   - What it adds: Add multiple items before confirming
   - Complexity: Medium
   - Guide: Store items in an array and execute all trades together

3. **Price Comparison**
   - What it adds: Show buy vs sell prices side-by-side
   - Complexity: Simple
   - Guide: Add a second column to the item row widget

4. **Category Filtering**
   - What it adds: Filter items by type (Food, Fuel, etc.)
   - Complexity: Medium
   - Guide: Add a ComboBox and filter the item list array

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading System Guide](../Assets/TradingSystemGuide.md) - Full trading system documentation
- [Market Data Asset Setup](../Assets/TradingTemplates/Market_Template.yaml) - Creating markets
- [Trade Item Setup](../Assets/TradingTemplates/TradeItem_Template.yaml) - Creating trade items
- [MVP Trading Implementation](../docs/mvp/TRADING_UI_BLUEPRINT_GUIDE.md) - Advanced trading UI

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a Widget Blueprint extending a C++ parent class
- ✅ How to call C++ functions from Blueprint
- ✅ How to dynamically create and populate UI lists
- ✅ How to implement buy/sell transaction logic
- ✅ How to bind UI events to custom functions

**Next Recommended Blueprint**: `BP_TradingStation` - To create the station that opens this UI

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Change `WBP_TradeItemRow` widget to customize item display
- Modify transaction validation in the Trade button logic
- Add animations in the Designer tab for smoother transitions

**Blueprint Best Practices Applied:**
- Used custom functions to avoid code duplication
- Cached frequently-used references (ItemListScrollBox)
- Separated UI update logic from business logic (C++ handles actual trades)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-03 | Initial simplified guide based on template |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [Blueprint Image Guide](../docs/reference/BLUEPRINT_IMAGE_GUIDE.md) for node diagrams
- See the [Trading System Troubleshooting](../Assets/TradingSystemGuide.md#troubleshooting)
