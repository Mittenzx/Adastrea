# Trading UI - Quick Start Guide

**Goal**: Get trading UI working in 30 minutes

---

## Step 1: Open Unreal Editor (5 min)

1. Launch Unreal Engine with Adastrea project
2. Let shaders compile if first time
3. Navigate to `Content/UI/Trading/` (create folder if needed)

---

## Step 2: Create Widget Blueprint (10 min)

### Create WBP_TradingUI

1. **Create Widget**:
   - Right-click in Content Browser
   - User Interface → Widget Blueprint
   - Name: `WBP_TradingUI`

2. **Set Parent Class**:
   - Open `WBP_TradingUI`
   - Click "Class Settings" (top toolbar)
   - In the **Details** panel, under **Class Options**, find **Parent Class**
   - Open the dropdown, switch to **All Classes**, and search for `TradingInterfaceWidget`
   - Select the C++ class `TradingInterfaceWidget` (from the C++ / native classes list, not another Blueprint)
   - Compile & Save

3. **Quick Layout** (Minimum Viable):
   ```
   Canvas Panel (Root)
   └── Vertical Box
       ├── Text Block: "Trading Station" (bind to Market Name)
       ├── Button: "Buy" (click → call ToggleBuySellView)
       ├── Scroll Box: "ItemList" (name it)
       ├── Text Block: "Credits: " (bind to GetPlayerCredits)
       ├── Button: "Execute Trade" (click → call ExecuteTrade)
       └── Button: "Close" (click → call CloseMarket)
   ```

4. **Implement OnMarketInventoryUpdated**:
   ```blueprint
   Event OnMarketInventoryUpdated
   ├── Clear ItemList children
   ├── Get Available Items (C++ function)
   └── For Each Item:
       ├── Create Text widget
       ├── Set text: "[ItemName] - [Price] CR"
       └── Add to ItemList
   ```

5. **Compile & Save**

---

## Step 3: Create Test Data Assets (10 min)

### Create Test Market

1. **Create Market Asset**:
   - Right-click in Content Browser
   - Blueprint → Data Asset
   - Choose: `MarketDataAsset`
   - Name: `DA_TestMarket`

2. **Configure Market**:
   ```
   Market Name: "Test Trading Station"
   Market Type: Open Market
   Market Size: Small
   Allow Player Buying: ✓ True
   Allow Player Selling: ✓ True
   ```

### Create Test Items

1. **Create Water Item**:
   - Data Asset → `TradeItemDataAsset`
   - Name: `DA_TradeItem_Water`
   - Configure:
     ```
     Item Name: "Water"
     Base Price: 10
     Volume Per Unit: 1.0
     Category: Food
     ```

2. **Create Food Item**:
   - Data Asset → `TradeItemDataAsset`
   - Name: `DA_TradeItem_Food`
   - Configure:
     ```
     Item Name: "Food Rations"
     Base Price: 25
     Volume Per Unit: 1.0
     Category: Food
     ```

### Add Items to Market

1. Open `DA_TestMarket`
2. In Inventory array, add 2 entries:
   ```
   Entry 0:
   - Trade Item: DA_TradeItem_Water
   - Current Stock: 1000
   - Max Stock: 1000
   - Supply Level: 1.0
   - Demand Level: 1.0
   
   Entry 1:
   - Trade Item: DA_TradeItem_Food
   - Current Stock: 500
   - Max Stock: 500
   - Supply Level: 1.0
   - Demand Level: 1.0
   ```

---

## Step 4: Setup Player (5 min)

1. **Open Player Pawn Blueprint** (or create test pawn)

2. **Add Components**:
   - Click "Add Component"
   - Search: `PlayerTraderComponent`
   - Add it
   - Set Credits: 1000

3. **Add Cargo Component**:
   - Click "Add Component"
   - Search: `CargoComponent`
   - Add it
   - Set Cargo Capacity: 100

4. **Compile & Save**

---

## Step 5: Test It! (5 min)

### Create Test Level Blueprint

1. Open your test level
2. Open Level Blueprint (Blueprints → Open Level Blueprint)

3. **Add Test Code**:
   ```blueprint
   Event BeginPlay
   └── Delay 1.0 seconds
       └── Get Player Controller
           └── Create Widget (WBP_TradingUI)
               └── Promote to Variable: "TradingWidget"
                   └── Call OpenMarket (DA_TestMarket)
                       └── Add to Viewport
   ```

4. **Play in Editor** (Alt+P)

### What You Should See

- Widget appears with "Test Trading Station"
- Two items listed (Water, Food)
- Your credits shown (1000 CR)
- Buttons functional

### Test Actions

1. Click "Execute Trade" → should show "cart empty" error ✓
2. **Manual Test**: In WBP_TradingUI, add test button:
   ```blueprint
   Button "Test Buy"
   └── OnClicked
       └── Add to Cart (DA_TradeItem_Water, 10)
           └── Execute Trade
   ```
3. Click "Test Buy" → credits should decrease ✓
4. Click "Close" → widget closes ✓

---

## Troubleshooting

### "Widget not showing items"
- Check OnMarketInventoryUpdated is implemented
- Verify market has inventory entries
- Check CreateWidget is working in loop

### "Can't buy items"
- Verify PlayerTraderComponent exists on player
- Verify CargoComponent exists on player
- Check Credits > 0
- Check Cargo Capacity > 0

### "Prices showing 0"
- Items need BasePrice > 0
- Check TradeItemDataAsset configured correctly

### "No components found"
- Ensure components added to player Blueprint
- Compile player Blueprint
- Check components in Details panel

---

## Next Steps

Once basic version works:

1. **Improve Layout** (1-2 hours):
   - Add proper styling
   - Better item display
   - Shopping cart UI

2. **Add Features** (2-3 hours):
   - Quantity input
   - Cart display
   - Buy/Sell toggle visuals
   - Error messages

3. **Full Implementation** (4-6 hours):
   - Follow TRADING_UI_BLUEPRINT_GUIDE.md
   - Create WBP_TradeItemRow
   - Add all polish

---

## Quick Reference

### Essential Functions

```cpp
// Setup
bool OpenMarket(UMarketDataAsset*)
void CloseMarket()

// Data
TArray<FMarketInventoryEntry> GetAvailableItems()
int32 GetPlayerCredits()
int32 GetItemPrice(Item, Quantity)

// Actions
bool AddToCart(Item, Quantity)
bool ExecuteTrade()
void ToggleBuySellView()
```

### Events to Implement

```cpp
OnMarketInventoryUpdated()     // Update item list
OnCartUpdated()                // Update cart display  
OnCreditsUpdated(Credits)      // Update credits text
OnTradeCompleted(Success, Msg) // Show result
```

---

## Success!

If you can:
- ✓ Open widget
- ✓ See items
- ✓ Buy items
- ✓ See credits decrease
- ✓ Close widget

**You're done!** The basic trading UI works.

Now follow the full guide for polish and features.

---

**Time**: 30 minutes  
**Difficulty**: Easy  
**Next**: TRADING_UI_BLUEPRINT_GUIDE.md for full implementation
