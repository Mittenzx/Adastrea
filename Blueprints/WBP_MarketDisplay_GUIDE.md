# Market Display Widget - Blueprint Guide

> **Create a widget that shows current market prices and profitable trade opportunities**

**Blueprint Type**: Widget Blueprint  
**Complexity**: Beginner  
**Time to Complete**: 25-35 minutes  
**Last Updated**: 2026-01-08

---

## 📋 Overview

### What You'll Build

A UI widget that displays current item prices across multiple markets, highlights profitable trade routes, and updates in real-time as the economy changes.

**End Result**: Players can see at a glance where to buy low and sell high, making the trading gameplay intuitive and engaging.

### Why This Blueprint Matters

Without price comparison, trading becomes trial-and-error guesswork. This widget transforms trading into an informed, strategic activity—essential for MVP's "buy low, sell high" core loop.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [ ] Unreal Engine 5.6+
- [ ] `UUserWidget` base class available (standard UE5)
- [ ] `BP_TradingGameMode` created and economy system working (see [Game Mode Guide](BP_TradingGameMode_GUIDE.md))
- [ ] At least 2-3 `UMarketDataAsset` created
- [ ] At least 5-10 `UTradeItemDataAsset` created
- [ ] Basic understanding of UI widgets in Unreal

**If you're missing any prerequisites**, see:
- [Trading Game Mode Guide](BP_TradingGameMode_GUIDE.md) - For economy system
- [Trading UI Widget Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - For similar widget patterns
- [MVP Quick Start](../docs/mvp/README.md) - For overall setup

---

## 🔗 Dependencies

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_TradingGameMode` | Economy data source | `Content/Blueprints/GameModes/` |
| `WBP_MarketPriceRow` | Individual price row display | `Content/UI/Trading/` (create in Step 8) |

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UUserWidget` | UMG | Base widget functionality |
| `UMarketDataAsset` | TradingSystem | Market information |
| `UTradeItemDataAsset` | TradingSystem | Item properties |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/UI/Trading/`
3. **Right-click** → User Interface → Widget Blueprint
4. **Choose Parent Class**: `User Widget`
5. **Name it**: `WBP_MarketDisplay`
6. **Double-click** to open the Widget Editor

### Step 2: Design the Layout

**We'll build this UI in Step 8 after variables/functions are ready**

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: MarketsToDisplay

- **Type**: `Array of Market Data Asset` (Object Reference Array)
- **Category**: `Display Config`
- **Instance Editable**: Yes
- **Tooltip**: "Which markets to show prices for"
- **Default Value**: Empty (set when widget created)

### Variable 2: ItemsToTrack

- **Type**: `Array of Trade Item Data Asset` (Object Reference Array)
- **Category**: `Display Config`
- **Instance Editable**: Yes
- **Tooltip**: "Which items to display prices for"
- **Default Value**: Empty (populated from markets or manually)

### Variable 3: GameModeRef

- **Type**: `BP_TradingGameMode` (Object Reference)
- **Category**: `Runtime`
- **Instance Editable**: No
- **Tooltip**: "Reference to game mode for price queries"
- **Default Value**: None (set in Construct event)

### Variable 4: PriceRowsContainer

- **Type**: `Scroll Box` (Widget Reference)
- **Category**: `UI References`
- **Instance Editable**: No
- **Tooltip**: "Container holding all price row widgets"
- **Default Value**: None (bind to widget in Designer)

### Variable 5: RefreshButton

- **Type**: `Button` (Widget Reference)
- **Category**: `UI References`
- **Instance Editable**: No
- **Tooltip**: "Button to manually refresh prices"
- **Default Value**: None (bind to widget in Designer)

### Variable 6: LastUpdateText

- **Type**: `Text Block` (Widget Reference)
- **Category**: `UI References`
- **Instance Editable**: No
- **Tooltip**: "Displays when prices were last updated"
- **Default Value**: None (bind to widget in Designer)

### Variable 7: AutoRefresh

- **Type**: `Boolean`
- **Category**: `Display Config`
- **Instance Editable**: Yes
- **Tooltip**: "If true, updates automatically when economy changes"
- **Default Value**: `True`

### Variable 8: HighlightProfitableThreshold

- **Type**: `Float`
- **Category**: `Display Config`
- **Instance Editable**: Yes
- **Tooltip**: "Minimum price difference (%) to highlight as profitable"
- **Default Value**: `20.0` (20% profit margin)

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing the main logic:**

### Function 1: InitializeDisplay

**Purpose**: Sets up the widget with markets and items to display

**Inputs**:
- `Markets` (Array of Market Data Asset) - Markets to show
- `Items` (Array of Trade Item Data Asset) - Items to track (optional, can auto-detect)

**Outputs**:
- None

**Implementation**: See [Step 1](#step-1-initialize-display-data) below

### Function 2: RefreshPrices

**Purpose**: Updates all displayed prices from game mode

**Inputs**:
- None

**Outputs**:
- None

**Implementation**: See [Step 2](#step-2-implement-price-refresh) below

### Function 3: CreatePriceRow

**Purpose**: Creates a single price row widget for an item

**Inputs**:
- `Item` (Trade Item Data Asset) - Item to display
- `Markets` (Array of Market Data Asset) - Markets to show prices for

**Outputs**:
- `RowWidget` (WBP_MarketPriceRow) - Created widget

**Implementation**: See [Step 3](#step-3-create-price-row-widget) below

### Function 4: CalculateProfitOpportunity

**Purpose**: Finds best buy/sell price difference for an item

**Inputs**:
- `ItemID` (Name) - Item to check

**Outputs**:
- `BestProfit` (Float) - Percentage profit possible
- `BuyMarket` (Market Data Asset) - Where to buy
- `SellMarket` (Market Data Asset) - Where to sell

**Implementation**: See [Step 4](#step-4-calculate-profit-opportunities) below

### Function 5: HighlightProfitableRows

**Purpose**: Visually highlights rows with good profit margins

**Inputs**:
- None (uses `HighlightProfitableThreshold`)

**Outputs**:
- None

**Implementation**: See [Step 5](#step-5-highlight-profitable-trades) below

### Function 6: OnEconomyUpdated

**Purpose**: Event handler for economy changes (from Game Mode)

**Inputs**:
- `ChangedItems` (Array of Names) - Items with price changes
- `NewPrices` (Array of Floats) - Updated prices

**Outputs**:
- None

**Implementation**: See [Step 6](#step-6-bind-to-economy-updates) below

---

## 📐 Widget Layout

### Visual Hierarchy

```
Canvas Panel (Root)
├── Background Image
│   └── Border (Frame)
├── Header Section
│   ├── Vertical Box
│   │   ├── Title Text: "Market Prices"
│   │   ├── Horizontal Box (Controls)
│   │   │   ├── Refresh Button
│   │   │   └── Last Update Text
│   │   └── Column Headers (Horizontal Box)
│   │       ├── "Item" (Text)
│   │       ├── "Market 1" (Text)
│   │       ├── "Market 2" (Text)
│   │       └── "Best Deal" (Text)
├── Content Section
│   └── Scroll Box: "PriceRowsContainer"
│       └── [Dynamic price row widgets added at runtime]
└── Footer Section (Optional)
    └── Text: "Prices update every 30s"
```

### Key Widget Names (Important!)

| Widget | Name in Hierarchy | Bind to Variable |
|--------|-------------------|------------------|
| Scroll Box | `PriceRowsContainer` | `PriceRowsContainer` |
| Button | `RefreshButton` | `RefreshButton` |
| Text Block | `LastUpdateText` | `LastUpdateText` |
| Text Block | `TitleText` | None (static) |

**Why Names Matter**: These will be referenced in Blueprint code.

---

## 🛠️ Step-by-Step Implementation

### Step 1: Initialize Display Data

**Goal**: Set up which markets and items to display

**Blueprint Nodes**:

```
Function: InitializeDisplay (Markets, Items inputs)
  ↓
Set MarketsToDisplay = Markets input
  ↓
Branch (Items array empty?)
  ├─ True: (Auto-detect items from markets)
  │   ↓
  │   ForEach Market in Markets
  │     ↓
  │     Get Market's Available Items
  │     ↓
  │     Add Unique to ItemsToTrack
  │   ↓
  │   Print String: "Auto-detected [X] items"
  │
  └─ False: (Use provided items)
      ↓
      Set ItemsToTrack = Items input
      ↓
      Print String: "Displaying [X] items"
  ↓
Get Game Mode
  ↓
Cast to BP_TradingGameMode
  ↓
Set GameModeRef variable
  ↓
RefreshPrices
```

**Detailed Instructions**:

1. Open the **InitializeDisplay** function
2. Set `MarketsToDisplay` = Markets input parameter
3. Add `Branch` checking if Items input array is empty
4. **If empty** (auto-detect):
   - Add `ForEach Loop` for Markets
   - For each market, get its available items list
   - Add each item to `ItemsToTrack` (use `Add Unique` to avoid duplicates)
5. **If not empty**:
   - Set `ItemsToTrack` = Items input
6. Get Game Mode:
   - `Get Game Mode` node
   - `Cast to BP_TradingGameMode`
   - Store in `GameModeRef` variable
7. Call `RefreshPrices` to populate initial display

**Expected Result**: 
- Widget knows which markets/items to show
- GameModeRef set correctly
- Ready to display prices

---

### Step 2: Implement Price Refresh

**Goal**: Update all displayed prices from current economy state

**Blueprint Nodes**:

```
Function: RefreshPrices
  ↓
Clear Children (PriceRowsContainer)
  ↓
ForEach Item in ItemsToTrack
  ↓ (Loop Body)
  CreatePriceRow (Item, MarketsToDisplay)
  ↓
  Add Child to Content (PriceRowsContainer, Created Row)
  ↓
HighlightProfitableRows
  ↓
Update Last Update Text
  ├─ Get Current Time
  └─ Format: "Updated: [HH:MM:SS]"
  ↓
Print String: "Prices refreshed - [X] items displayed"
```

**Detailed Instructions**:

1. Open the **RefreshPrices** function
2. **Clear existing rows**:
   - Get `PriceRowsContainer` widget
   - Call `Clear Children` to remove old data
3. **Create new rows**:
   - Add `ForEach Loop` for `ItemsToTrack` array
   - For each item:
     - Call `CreatePriceRow` function
     - Pass item and `MarketsToDisplay`
     - Get returned row widget
     - Call `Add Child to Content` on PriceRowsContainer
4. Call `HighlightProfitableRows` to apply visual highlights
5. **Update timestamp**:
   - Get current time (use `Get Real Time Seconds` node)
   - Convert to formatted string (HH:MM:SS)
   - Set `LastUpdateText` widget text
6. Add debug print

**Expected Result**: 
- All price rows display correctly
- Profitable trades highlighted
- Timestamp shows when updated

---

### Step 3: Create Price Row Widget

**Goal**: Generate a widget showing one item's prices across markets

**Blueprint Nodes**:

```
Function: CreatePriceRow (Item, Markets input → returns Widget)
  ↓
Create Widget (Class: WBP_MarketPriceRow)
  ↓
Set Item Name (from Item Data Asset)
  ↓
ForEach Market in Markets
  ↓ (Loop Body)
  Get Item Price from GameModeRef
    ├─ ItemID: Get from Item
    └─ MarketData: Current Market
  ↓
  Add Price to Row Widget
  ↓
Calculate Best Deal (Profit Opportunity)
  ↓
Set Best Deal Text on Row Widget
  ↓
Return: Row Widget
```

**Note**: This function requires `WBP_MarketPriceRow` widget to exist. We'll create it in Step 8.

**Detailed Instructions**:

1. Open the **CreatePriceRow** function
2. Call `Create Widget`:
   - Class: `WBP_MarketPriceRow` (we'll create this)
   - Owning Player: Get Player Controller
   - Store result in local variable
3. **Set item name**:
   - Get Item's `Display Name` from Data Asset
   - Call `SetItemName` on row widget
4. **Get prices for each market**:
   - ForEach loop through Markets input
   - For each market:
     - Get Item's ID (FName)
     - Call `GameModeRef → GetItemPrice(ItemID, Market)`
     - Call `AddMarketPrice` on row widget with result
5. **Calculate profit opportunity**:
   - Call `CalculateProfitOpportunity` with ItemID
   - Get best profit percentage
   - Format as string: "+25% profit"
   - Call `SetBestDeal` on row widget
6. Return the created row widget

**Expected Result**: 
- Returns populated row widget
- Shows item name and prices
- Shows best trade opportunity

---

### Step 4: Calculate Profit Opportunities

**Goal**: Find the best buy/sell price difference for an item

**Blueprint Nodes**:

```
Function: CalculateProfitOpportunity (ItemID input → returns Float, Market, Market)
  ↓
Local Variables:
  ├─ LowestPrice = 999999.0
  ├─ HighestPrice = 0.0
  ├─ BuyMarket = None
  └─ SellMarket = None
  ↓
ForEach Market in MarketsToDisplay
  ↓ (Loop Body)
  Get Price = GameModeRef.GetItemPrice(ItemID, Market)
  ↓
  Branch (Price < LowestPrice?)
    ├─ True:
    │   ├─ Set LowestPrice = Price
    │   └─ Set BuyMarket = Market
    └─ False: (continue)
  ↓
  Branch (Price > HighestPrice?)
    ├─ True:
    │   ├─ Set HighestPrice = Price
    │   └─ Set SellMarket = Market
    └─ False: (continue)
  ↓
Calculate Profit % = ((HighestPrice - LowestPrice) / LowestPrice) × 100
  ↓
Return: Profit %, BuyMarket, SellMarket
```

**Detailed Instructions**:

1. Open the **CalculateProfitOpportunity** function
2. Create local variables:
   - `LowestPrice` (Float) = 999999.0
   - `HighestPrice` (Float) = 0.0
   - `BuyMarket` (Market Data Asset Reference) = None
   - `SellMarket` (Market Data Asset Reference) = None
3. **Find lowest and highest prices**:
   - ForEach loop through `MarketsToDisplay`
   - For each market:
     - Get current price via `GameModeRef.GetItemPrice`
     - Compare to LowestPrice:
       - If lower: Update LowestPrice and BuyMarket
     - Compare to HighestPrice:
       - If higher: Update HighestPrice and SellMarket
4. **Calculate profit percentage**:
   - Formula: (HighestPrice - LowestPrice) / LowestPrice × 100
   - This gives percentage profit
5. **Return** three values:
   - Profit percentage
   - BuyMarket
   - SellMarket

**Expected Result**: 
- Returns best trade route for item
- Profit % shows potential earnings
- Markets show where to buy/sell

---

### Step 5: Highlight Profitable Trades

**Goal**: Visually mark rows with good profit margins

**Blueprint Nodes**:

```
Function: HighlightProfitableRows
  ↓
Get All Children (PriceRowsContainer)
  ↓
ForEach Child Widget
  ↓ (Loop Body)
  Cast to WBP_MarketPriceRow
  ↓
  Get Profit Percentage (from row widget)
  ↓
  Branch (Profit % >= HighlightProfitableThreshold?)
    ├─ True:
    │   ↓
    │   Set Row Highlighted (True)
    │   ├─ Change background color to green tint
    │   ├─ Set profit text color to bright green
    │   └─ Add "⭐" icon or similar indicator
    │
    └─ False:
        ↓
        Set Row Highlighted (False)
        └─ Use default colors
```

**Detailed Instructions**:

1. Open the **HighlightProfitableRows** function
2. Get all children from `PriceRowsContainer`:
   - Call `Get All Children` node
   - Returns array of widgets
3. **Process each row**:
   - ForEach loop through children
   - Cast each to `WBP_MarketPriceRow`
   - Get the row's stored profit percentage
4. **Apply highlighting**:
   - Branch: Is profit >= `HighlightProfitableThreshold`?
   - If yes:
     - Call `SetHighlighted(True)` on row widget
     - This changes visual appearance (green background)
   - If no:
     - Call `SetHighlighted(False)`
     - Normal appearance

**Expected Result**: 
- Good trade opportunities stand out visually
- Easy to spot profitable items at a glance
- Configurable threshold (20% default)

---

### Step 6: Bind to Economy Updates

**Goal**: Auto-refresh when economy changes (if AutoRefresh enabled)

**Blueprint Nodes**:

```
Event Construct (Widget initialization)
  ↓
Get Game Mode
  ↓
Cast to BP_TradingGameMode
  ↓
Bind Event to OnEconomyUpdated
  ├─ Event: Custom Event "HandleEconomyUpdate"
  └─ Binding stored in GameModeRef
  ↓
Print String: "Market Display listening to economy"

---

Custom Event: HandleEconomyUpdate (ChangedItems, NewPrices inputs)
  ↓
Branch (AutoRefresh?)
  ├─ True:
  │   ↓
  │   Print String: "Economy changed, refreshing display"
  │   ↓
  │   RefreshPrices
  │
  └─ False:
      ↓
      Print String: "Economy changed (auto-refresh disabled)"
```

**Detailed Instructions**:

1. In **Event Graph**, find `Event Construct`
2. Get Game Mode and cast to `BP_TradingGameMode`
3. **Bind to economy updates**:
   - Create Custom Event named `HandleEconomyUpdate`
   - Add inputs matching OnEconomyUpdated dispatcher:
     - ChangedItems (Array of Names)
     - NewPrices (Array of Floats)
   - In Event Construct:
     - Call `Bind Event to OnEconomyUpdated` (from GameModeRef)
     - Connect to HandleEconomyUpdate event
4. **In HandleEconomyUpdate**:
   - Check `AutoRefresh` variable
   - If true: Call `RefreshPrices`
   - If false: Just log (user must manually refresh)

**Expected Result**: 
- Widget updates automatically when prices change
- Can disable auto-refresh for performance
- Console confirms economy updates

---

### Step 7: Add Manual Refresh Button Handler

**Goal**: Let player manually refresh prices

**Blueprint Nodes**:

```
Event PreConstruct (Design Time only)
  ↓
Bind Event to RefreshButton.OnClicked
  ↓
When button clicked:
  ↓
  Print String: "Manual refresh requested"
  ↓
  RefreshPrices
  ↓
  Play Sound: UI_ButtonClick (optional)
```

**Detailed Instructions**:

1. In **Event Graph**, find or create `Event Pre Construct`
2. Get `RefreshButton` widget reference
3. **Bind click event**:
   - Call `Bind Event to OnClicked` (from RefreshButton)
   - Create custom event or inline graph
4. **On button click**:
   - Add debug print
   - Call `RefreshPrices`
   - Optionally play UI sound for feedback

**Expected Result**: 
- Clicking refresh button updates prices immediately
- Works even if AutoRefresh disabled
- Provides instant feedback

---

### Step 8: Create the Price Row Widget (Sub-Widget)

**Goal**: Create `WBP_MarketPriceRow` that displays one item's info

**Steps to Create Sub-Widget**:

1. **Create new Widget Blueprint**:
   - Navigate to `Content/UI/Trading/`
   - Right-click → Widget Blueprint
   - Name: `WBP_MarketPriceRow`
   - Parent: `User Widget`

2. **Design Layout**:
   ```
   Horizontal Box (Root)
   ├── Item Name (Text Block) - 200px width
   ├── Market 1 Price (Text Block) - 100px width
   ├── Market 2 Price (Text Block) - 100px width
   ├── Market 3 Price (Text Block) - 100px width
   └── Best Deal (Text Block) - 150px width, green color
   ```

3. **Create Variables**:
   - `ItemName` (Text Block) - bind to widget
   - `MarketPrices` (Array of Text Block) - bind to price widgets
   - `BestDealText` (Text Block) - bind to widget
   - `ProfitPercentage` (Float) - stores calculated profit
   - `IsHighlighted` (Boolean) - visual state

4. **Create Functions**:
   - `SetItemName(Name as Text)` - Sets item name
   - `AddMarketPrice(Price as Float)` - Adds price to next column
   - `SetBestDeal(Profit as Float, BuyMarket as Text, SellMarket as Text)` - Shows best trade
   - `SetHighlighted(Highlighted as Boolean)` - Changes background color

5. **Implement SetHighlighted**:
   ```
   SetHighlighted (Highlighted input)
     ↓
     Branch (Highlighted?)
       ├─ True:
       │   ├─ Set Background Color: Green tint (R:0.1, G:0.3, B:0.1, A:0.3)
       │   └─ Set BestDealText Color: Bright Green
       └─ False:
           ├─ Set Background Color: Default (White/Transparent)
           └─ Set BestDealText Color: Default
   ```

**Expected Result**: 
- Row widget displays item data cleanly
- Can be created/configured from parent widget
- Visual highlighting works

---

## 🧪 Testing Your Blueprint

### Test 1: Widget Displays Initial Prices

**What to Test**: Market display shows prices on creation

**Steps**:
1. Create `WBP_MarketDisplay` widget
2. In Construct or Init function:
   - Set MarketsToDisplay (2-3 markets)
   - Call `InitializeDisplay`
3. Add widget to viewport (e.g., in PlayerController or HUD)
4. Press Play

**Success Criteria**: 
- Widget appears on screen
- Shows item names
- Shows prices for each market
- No errors in console

**If it fails**: See [Troubleshooting: Widget Not Displaying](#common-issue-1-widget-not-displaying)

---

### Test 2: Prices Update on Refresh

**What to Test**: Manual refresh updates prices

**Steps**:
1. Start game with widget visible
2. Note current prices
3. Wait 30+ seconds (economy update)
4. Click Refresh button
5. Check if prices changed

**Success Criteria**: 
- Prices update when button clicked
- Timestamp updates
- New prices match game mode's economy

**If it fails**: See [Troubleshooting: Refresh Not Working](#common-issue-2-refresh-not-working)

---

### Test 3: Profitable Trades Highlighted

**What to Test**: Good trades are visually highlighted

**Steps**:
1. Start game
2. Look at market display
3. Make a trade that affects supply/demand significantly
4. Wait for economy update or click refresh
5. Check for green-highlighted rows

**Success Criteria**: 
- Rows with 20%+ profit margin are green
- Other rows remain normal color
- "Best Deal" column shows profit %

**If it fails**: See [Troubleshooting: Highlighting Not Working](#common-issue-3-highlighting-not-working)

---

### Test 4: Auto-Refresh Works

**What to Test**: Widget updates when economy changes

**Steps**:
1. Ensure AutoRefresh = True
2. Start game
3. Don't touch refresh button
4. Wait for 2-3 economy update cycles (60-90 seconds)
5. Watch prices change automatically

**Success Criteria**: 
- Prices update without manual refresh
- Updates align with economy update intervals
- Console shows economy update messages

**If it fails**: See [Troubleshooting: Auto-Refresh Not Working](#common-issue-4-auto-refresh-not-working)

---

## 🐛 Troubleshooting

### Common Issue 1: Widget Not Displaying

**Symptoms**:
- Blank screen where widget should be
- Widget invisible
- No price rows shown

**Causes**:
- Widget not added to viewport
- MarketsToDisplay empty
- ItemsToTrack empty
- PriceRowsContainer not bound

**Solutions**:
1. **Verify widget added to viewport**:
   - Check PlayerController or HUD Blueprint
   - Must call `Create Widget` + `Add to Viewport`
2. **Check MarketsToDisplay populated**:
   - In InitializeDisplay call, pass valid markets
   - Check markets array not empty
   - Verify markets are valid Data Assets
3. **Verify ItemsToTrack has items**:
   - Check auto-detection logic
   - Or manually pass items array
   - Must have at least 1 item
4. **Check widget bindings**:
   - In Widget Designer, verify PriceRowsContainer variable is bound
   - Check "Is Variable" checkbox on Scroll Box

---

### Common Issue 2: Refresh Not Working

**Symptoms**:
- Click refresh button, nothing happens
- Prices don't update
- No console messages

**Causes**:
- Button click not bound
- GameModeRef is null
- RefreshPrices function not called
- CreatePriceRow failing

**Solutions**:
1. **Check button binding**:
   - In Event Graph, verify OnClicked bound
   - Test with Print String in click handler
2. **Verify GameModeRef**:
   - In InitializeDisplay, check Cast to BP_TradingGameMode succeeds
   - Add Print String showing GameModeRef value
   - Check game mode is BP_TradingGameMode in World Settings
3. **Debug RefreshPrices**:
   - Add Print String at start of function
   - Check if loop runs (ItemsToTrack not empty)
   - Verify CreatePriceRow returns valid widget
4. **Check WBP_MarketPriceRow exists**:
   - Verify sub-widget created
   - No compile errors in sub-widget

---

### Common Issue 3: Highlighting Not Working

**Symptoms**:
- All rows same color
- No green highlighting for profitable trades
- Profit % calculated but not applied

**Causes**:
- HighlightProfitableThreshold too high (or too low)
- SetHighlighted function not implemented
- Profit % not stored in row widget
- Visual bindings not set in sub-widget

**Solutions**:
1. **Adjust threshold**:
   - Lower HighlightProfitableThreshold to 10% for testing
   - Check if some items now highlight
   - Default 20% may be too high for test data
2. **Verify SetHighlighted implemented**:
   - Open WBP_MarketPriceRow
   - Check SetHighlighted function exists
   - Check it actually changes colors (not just variable)
3. **Debug profit calculation**:
   - In CalculateProfitOpportunity, print profit %
   - Verify some items have >20% differences
   - Check if profit % passed to row widget correctly
4. **Check visual binding**:
   - In WBP_MarketPriceRow Designer
   - Verify background image/border exists
   - Check color can be changed via Blueprint

---

### Common Issue 4: Auto-Refresh Not Working

**Symptoms**:
- Prices never update automatically
- Must manually click refresh
- AutoRefresh = True but no effect

**Causes**:
- Event binding not set up
- OnEconomyUpdated dispatcher not firing
- HandleEconomyUpdate not calling RefreshPrices
- Game Mode not dispatching event

**Solutions**:
1. **Verify binding in Construct**:
   - Check `Bind Event to OnEconomyUpdated` exists
   - Verify it's called in Event Construct
   - Check binding connects to HandleEconomyUpdate
2. **Check Game Mode dispatcher**:
   - Open BP_TradingGameMode
   - In UpdateEconomy function, verify it calls OnEconomyUpdated
   - Add Print String before calling dispatcher
3. **Test HandleEconomyUpdate**:
   - Add Print String at start of event
   - Check if it fires when economy updates
   - Verify AutoRefresh variable is True
4. **Manual test**:
   - In console, call UpdateEconomy command
   - Check if widget refreshes
   - This isolates binding vs economy update issues

---

### Common Issue 5: Wrong Prices Displayed

**Symptoms**:
- Prices don't match trading UI
- Prices seem random
- All prices showing 0 or same value

**Causes**:
- Calling wrong GetItemPrice function
- ItemID mismatch (wrong names)
- Market multipliers not applied
- Economy not initialized

**Solutions**:
1. **Verify GetItemPrice call**:
   - Must call GameModeRef.GetItemPrice
   - Pass correct ItemID (FName)
   - Pass correct MarketData reference
2. **Check ItemID matches**:
   - Print ItemID in CreatePriceRow
   - Verify exact match with Data Asset IDs
   - Case-sensitive, no typos
3. **Test economy initialized**:
   - Check console for "Economy initialized"
   - Verify CurrentPrices map populated
   - Check GetItemPrice returns non-zero
4. **Compare with Trading UI**:
   - Open trading UI at same station
   - Should show same prices
   - If different, issue is in game mode logic

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Price History Graph**
   - What it adds: Line graph showing price changes over time
   - Complexity: Medium
   - Implementation: Store price history array, use UMG line chart widget

2. **Sorting Options**
   - What it adds: Sort by item name, price, profit %
   - Complexity: Simple
   - Implementation: Add dropdown, re-sort ItemsToTrack before refresh

3. **Filtering**
   - What it adds: Show only items player can afford, or only profitable
   - Complexity: Simple
   - Implementation: Add filter checkboxes, filter ItemsToTrack

4. **Trade Route Visualization**
   - What it adds: Arrow pointing from buy market to sell market
   - Complexity: Medium
   - Implementation: Add UMG canvas arrows based on best deal calculation

5. **Notification System**
   - What it adds: Alert when excellent trade opportunity appears
   - Complexity: Medium
   - Implementation: Check profit % on update, show popup if >50%

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading Game Mode Guide](BP_TradingGameMode_GUIDE.md) - Economy system
- [Trading UI Widget Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - Trading interface
- [Trading Ship Guide](BP_TradingShip_GUIDE.md) - Player ship
- [UMG Widget Guide](https://docs.unrealengine.com/5.6/en-US/umg-ui-designer-for-unreal-engine/) - Official UE5 UI docs

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create data-driven UI widgets in Unreal
- ✅ How to dynamically create and populate widget children
- ✅ How to bind to Event Dispatchers for reactive UI
- ✅ How to implement highlight/emphasis effects
- ✅ How to calculate and display profit opportunities
- ✅ How to create reusable sub-widgets
- ✅ How to integrate UI with game systems (Economy/Game Mode)

**Next Recommended Blueprint**: [Tutorial System](WBP_TutorialOverlay_GUIDE.md) - Guide players through first trades

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Change HighlightProfitableThreshold for different difficulty levels
- Modify row widget layout for different information density
- Add custom sorting/filtering logic
- Implement price alerts and notifications

**Blueprint Best Practices Applied:**
- Modular design (separate row widget)
- Event-driven updates (dispatcher binding)
- Configurable parameters (editable variables)
- Graceful degradation (auto-detect items if not provided)

**Performance Considerations:**
- Only refreshes when economy changes (not every frame)
- Reuses widgets where possible (Clear Children + recreate)
- Efficient profit calculation (single pass per item)
- Can disable auto-refresh if performance issue

**UMG Best Practices:**
- Used Scroll Box for long lists
- Proper widget hierarchy (Canvas → Containers → Content)
- Bound variables for easy access
- Separate concerns (display vs calculation)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-08 | Initial guide for MVP Phase 1 |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [UMG UI Guide](https://docs.unrealengine.com/5.6/en-US/umg-ui-designer-for-unreal-engine/)
- See [MVP Documentation](../docs/mvp/README.md)

---

**Guide created for**: Trade Simulator MVP (Phase 1)  
**Maintained by**: Adastrea Development Team
