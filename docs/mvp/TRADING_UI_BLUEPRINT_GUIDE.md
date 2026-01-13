# Trading UI Blueprint Implementation Guide

**Goal**: Create a functional trading UI for the MVP using UMG and the TradingInterfaceWidget C++ class  
**Time Estimate**: 4-6 hours for complete implementation  
**Last Updated**: 2026-01-13  
**Difficulty**: Intermediate

**Quick Version**: For 30-minute setup, see [TRADING_UI_QUICKSTART.md](TRADING_UI_QUICKSTART.md)

---

## Table of Contents

1. [Overview](#overview)
2. [Widget Structure](#widget-structure)
3. [Creating WBP_TradingUI](#creating-wbp_tradingui)
4. [Creating WBP_TradeItemRow](#creating-wbp_tradeitemrow)
5. [Implementing Blueprint Logic](#implementing-blueprint-logic)
6. [Testing the UI](#testing-the-ui)

---

## Overview

### What We're Building

A functional trading interface with:
- **Market Inventory List**: Shows available items with prices
- **Buy/Sell Toggle**: Switch between buying from and selling to market
- **Shopping Cart**: Add items, see total cost
- **Player Info Panel**: Credits, cargo space, profit
- **Transaction Button**: Execute trade with validation

### C++ Integration Points

The `UTradingInterfaceWidget` C++ class provides:
- `OpenMarket(UMarketDataAsset*)` - Initialize trading with a market
- `GetAvailableItems()` - Get market inventory
- `AddToCart(Item, Quantity)` - Add items to cart
- `ExecuteTrade()` - Complete transaction
- Blueprint Events for UI updates

---

## Widget Structure

```
WBP_TradingUI (Main Widget)
├── Background Panel
├── Header
│   ├── Market Name (Text)
│   └── Close Button
├── Left Panel (Market Info)
│   ├── Buy/Sell Toggle
│   ├── Category Filter (Optional)
│   ├── Sort Dropdown (Optional)
│   └── Item List (Scroll Box)
│       └── WBP_TradeItemRow (repeated)
├── Right Panel (Player Info & Cart)
│   ├── Player Stats Panel
│   │   ├── Credits Display
│   │   ├── Cargo Space Bar
│   │   └── Profit Display
│   ├── Shopping Cart Panel
│   │   ├── Cart Title
│   │   ├── Cart Items List
│   │   └── Cart Total
│   └── Transaction Button Panel
│       ├── Trade Button
│       └── Clear Cart Button
└── Status/Error Message (Text)
```

---

## Creating WBP_TradingUI

### Step 1: Create the Blueprint Widget

1. In Content Browser, navigate to `Content/UI/Trading/`
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_TradingUI`
4. Open the widget

### Step 2: Set Parent Class

1. In Class Settings (top toolbar)
2. Set Parent Class to `TradingInterfaceWidget`
3. This gives access to all C++ functions

### Step 3: Design the Layout

#### Root Canvas
```
Canvas Panel (Root)
├── Full screen (0,0 to 1920,1080 or anchored to screen)
└── ZOrder: 0
```

#### Background Overlay
```
Overlay
├── Anchor: Fill Screen
├── Image (Background)
│   ├── Color: Black with 80% opacity
│   └── Blur effect (optional)
└── Border (Modal Frame)
    ├── Size: 1600 x 900
    ├── Position: Centered
    └── Color: Dark gray
```

#### Main Container
```
Horizontal Box (Main Container)
├── Slot 1: Left Panel (Market) - Fill, Ratio 1.5
└── Slot 2: Right Panel (Player) - Fill, Ratio 1.0
```

### Step 4: Left Panel - Market Inventory

```
Vertical Box (Market Panel)
├── Header Section
│   ├── Text Block: "Market Name" (bound to Market variable)
│   └── Horizontal Box (Controls)
│       ├── Button: "Buy" (green when active)
│       ├── Button: "Sell" (red when active)
│       └── ComboBox: Category Filter (optional)
├── Scroll Box (Item List)
│   └── Named: "ItemListScrollBox"
│       └── Items added dynamically via Blueprint
└── Footer
    └── Text: Item count
```

### Step 5: Right Panel - Player & Cart

```
Vertical Box (Player Panel)
├── Player Stats Section
│   ├── Text: "Your Credits"
│   ├── Text Block: Credits Amount (Large font, bound)
│   ├── Progress Bar: Cargo Space
│   │   ├── Percent: Bound to cargo fill percentage
│   │   └── Colors: Green → Yellow → Red
│   ├── Text: "Cargo: X/Y units"
│   └── Text: "Profit: +/- X credits" (Green/Red)
├── Spacer (10px)
├── Shopping Cart Section
│   ├── Text: "Shopping Cart"
│   ├── Scroll Box (Cart Items)
│   │   └── Named: "CartScrollBox"
│   └── Horizontal Box
│       ├── Text: "Total:"
│       └── Text Block: Cart Total (bound, large)
├── Spacer (10px)
└── Transaction Buttons
    ├── Button: "Complete Trade" (Primary, Green)
    │   ├── IsEnabled: Bound to ValidateTransaction
    │   └── OnClicked: Call ExecuteTrade
    └── Button: "Clear Cart" (Secondary)
        └── OnClicked: Call ClearCart
```

---

## Creating WBP_TradeItemRow

This is a reusable widget for each trade item in the list.

### Step 1: Create the Widget

1. Create new Widget Blueprint: `WBP_TradeItemRow`
2. Parent class: `UserWidget` (standard)

### Step 2: Design the Row

```
Border (Root)
├── Padding: 5px
├── Background: Slightly transparent
└── Horizontal Box
    ├── Image: Item Icon (64x64)
    ├── Vertical Box: Item Info
    │   ├── Text Block: Item Name (Bold)
    │   ├── Text: Item Description (Small, Truncated)
    │   └── Text: Stock Level
    ├── Spacer
    ├── Vertical Box: Price Info
    │   ├── Text: "Price"
    │   └── Text Block: Price Value (Large)
    └── Horizontal Box: Actions
        ├── Button: "-" (Decrease)
        ├── EditableTextBox: Quantity (Number only)
        ├── Button: "+" (Increase)
        └── Button: "Add to Cart"
```

### Step 3: Add Variables

```cpp
// Exposed variables
UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn))
UTradeItemDataAsset* ItemData;

UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn))
bool bIsBuyMode;

UPROPERTY(BlueprintReadWrite)
int32 Quantity = 1;
```

### Step 4: Implement Functions

**On Construct**:
```blueprint
1. If ItemData is valid:
   - Set ItemName text to ItemData.ItemName
   - Set ItemDescription text to ItemData.Description
   - Calculate and display price
   - Update stock display
```

**Add to Cart Button**:
```blueprint
1. Get parent WBP_TradingUI reference
2. Call AddToCart(ItemData, Quantity)
3. Show success feedback (color flash, sound)
4. Reset quantity to 1
```

---

## Implementing Blueprint Logic

### WBP_TradingUI Event Graph

#### Event: OnMarketInventoryUpdated (C++ Event)

```blueprint
OnMarketInventoryUpdated
├── Clear ItemListScrollBox children
├── Get Available Items (C++)
├── For Each Item:
│   ├── Create WBP_TradeItemRow
│   ├── Set ItemData = Current Item
│   ├── Set bIsBuyMode = bShowBuyView
│   └── Add to ItemListScrollBox
└── Update item count text
```

#### Event: OnCartUpdated (C++ Event)

```blueprint
OnCartUpdated
├── Clear CartScrollBox children
├── For Each item in ShoppingCart (C++):
│   ├── Create simple text widget
│   ├── Format: "[Quantity]x [ItemName] - [Price] credits"
│   └── Add to CartScrollBox
├── Get Cart Total (C++)
└── Update total display text
```

#### Event: OnCreditsUpdated (C++ Event)

```blueprint
OnCreditsUpdated
├── Update Credits text block
└── Format with thousand separators: "1,234,567 CR"
```

#### Event: OnCargoSpaceUpdated (C++ Event)

```blueprint
OnCargoSpaceUpdated
├── Calculate fill percentage: UsedSpace / TotalSpace
├── Set Progress Bar percent
├── Update cargo text: "50 / 100 units"
├── Update color based on percentage:
│   ├── < 70%: Green
│   ├── 70-90%: Yellow
│   └── > 90%: Red
```

#### Event: OnTradeCompleted (C++ Event)

```blueprint
OnTradeCompleted
├── If Success:
│   ├── Play success sound
│   ├── Show success message (fade out after 3s)
│   └── Spawn credits VFX
└── Else:
    ├── Play error sound
    └── Show error message (ErrorMessage parameter)
```

#### Button: Buy/Sell Toggle

```blueprint
OnClicked
├── Call ToggleBuySellView() (C++)
├── Update button visuals:
│   ├── Active button: Highlighted
│   └── Inactive button: Dimmed
└── Clear cart (handled by C++)
```

#### Button: Complete Trade

```blueprint
OnClicked
├── Validate Transaction (C++)
├── If valid:
│   ├── Call ExecuteTrade() (C++)
│   └── [C++ handles rest via events]
└── Else:
    └── Show validation error message
```

#### Button: Close

```blueprint
OnClicked
└── Call CloseMarket() (C++)
    └── [C++ removes widget from viewport]
```

---

## Binding Properties

### Credits Display Binding

```blueprint
Function: GetCreditsText
├── Get Player Credits (C++)
├── Format as text: "1,234,567 CR"
└── Return formatted text
```

### Cargo Progress Bar Binding

```blueprint
Function: GetCargoFillPercent
├── Get Cargo Space Info (C++)
├── Calculate: UsedSpace / TotalSpace
└── Return as float (0.0 to 1.0)
```

### Profit Display Binding

```blueprint
Function: GetProfitText
├── Get Player Profit (C++)
├── Format with + or - sign
├── Set color: Green if positive, Red if negative
└── Return formatted text
```

### Trade Button Enabled Binding

```blueprint
Function: CanExecuteTrade
├── Call ValidateTransaction(ErrorMessage) (C++)
└── Return boolean result
```

---

## Testing the UI

### Step 1: Create Test Market

1. Create `DA_Market_TestStation` in Content Browser
2. Set market properties:
   - Name: "Test Trading Station"
   - Type: Open Market
   - Configure inventory with 3-5 items

### Step 2: Create Test Trade Items

Create simple items like:
- `DA_TradeItem_Water` (BasePrice: 10, Volume: 1)
- `DA_TradeItem_Food` (BasePrice: 25, Volume: 1)
- `DA_TradeItem_Electronics` (BasePrice: 100, Volume: 5)

### Step 3: Test in Play Mode

1. Add PlayerTraderComponent to player pawn
2. Add CargoComponent to player pawn
3. Set initial credits (e.g., 1000)
4. Create widget instance:
```blueprint
Create Widget: WBP_TradingUI
├── Call OpenMarket(TestMarket)
└── Add to Viewport
```

### Test Cases

- [ ] Market loads and displays items correctly
- [ ] Buy mode shows correct prices
- [ ] Items can be added to cart
- [ ] Cart total calculates correctly
- [ ] Trade button enables/disables appropriately
- [ ] Transaction completes successfully
- [ ] Credits decrease after buying
- [ ] Cargo space updates after buying
- [ ] Sell mode shows player's inventory
- [ ] Selling increases credits
- [ ] Error messages show for invalid trades
- [ ] Close button works

---

## Styling Guidelines (MVP)

### Colors

```
Background: #1A1A1A (Dark Gray)
Primary: #00FF00 (Green - Buy/Success)
Secondary: #FF4444 (Red - Sell/Error)
Text: #FFFFFF (White)
Disabled: #666666 (Gray)
```

### Fonts

```
Header: Bold, 24pt
Body: Regular, 14pt
Small: Regular, 12pt
Price/Credits: Bold, 18pt
```

### Spacing

```
Panel Padding: 20px
Element Spacing: 10px
Button Padding: 15px horizontal, 10px vertical
```

---

## Common Issues & Solutions

### Issue: Items Not Showing

**Solution**: Ensure OnMarketInventoryUpdated event is implemented and CreateWidget is working.

### Issue: Trade Button Not Enabling

**Solution**: Check ValidateTransaction binding is correctly implemented.

### Issue: Prices Not Updating

**Solution**: Verify EconomyManager is initialized and market is registered.

### Issue: Cargo Space Not Working

**Solution**: Ensure CargoComponent exists on player pawn and has CargoCapacity set.

---

## Next Steps

After basic UI is working:

1. **Add Filtering**: Implement category dropdown
2. **Add Sorting**: Price, Name, Stock sorting
3. **Add Search**: Text search for items
4. **Add VFX**: Success particles, button feedback
5. **Add SFX**: Button clicks, transaction sounds
6. **Add Animations**: Smooth transitions, item highlights
7. **Add Tooltips**: Hover information for items
8. **Polish Layout**: Better spacing, colors, icons

---

## Quick Reference

### Essential C++ Functions

```cpp
// Opening/Closing
bool OpenMarket(UMarketDataAsset* Market)
void CloseMarket()

// Getting Data
TArray<FMarketInventoryEntry> GetAvailableItems()
int32 GetItemPrice(UTradeItemDataAsset* Item, int32 Quantity)
int32 GetPlayerCredits()
int32 GetPlayerProfit()

// Cart Operations
bool AddToCart(UTradeItemDataAsset* Item, int32 Quantity)
void RemoveFromCart(UTradeItemDataAsset* Item)
void ClearCart()
int32 GetCartTotal()

// Transaction
bool ValidateTransaction(FText& OutErrorMessage)
bool ExecuteTrade()

// UI Controls
void ToggleBuySellView()
void SetCategoryFilter(ETradeItemCategory Category)
```

### Blueprint Events to Implement

```cpp
OnMarketInventoryUpdated()  // Rebuild item list
OnCartUpdated()             // Update cart display
OnCreditsUpdated(int32)     // Update credits text
OnCargoSpaceUpdated(float, float)  // Update cargo bar
OnTradeCompleted(bool, FText)      // Show result
```

---

**Last Updated**: 2025-12-29  
**For**: Trade Simulator MVP  
**Status**: Ready for Implementation
