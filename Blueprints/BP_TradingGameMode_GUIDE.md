# Trading Game Mode - Blueprint Guide

> **⚠️ DEPRECATED**: This guide has been consolidated into the unified Trading and Docking Complete Guide.  
> **See**: [Trading and Docking Complete Guide](../docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)

---

> **Create the game mode that manages economy simulation and game rules**

**Blueprint Type**: Game Mode  
**Complexity**: Intermediate  
**Time to Complete**: 35-45 minutes  
**Last Updated**: 2026-01-08  
**Status**: **DEPRECATED** - Refer to unified guide above

---

## 📋 Overview

### What You'll Build

A custom Game Mode that manages the trading economy simulation, updates market prices over time, and handles game session rules for the Trade Simulator MVP.

**End Result**: Dynamic economy where prices fluctuate based on supply/demand, creating varied trade opportunities for players.

### Why This Blueprint Matters

The economy simulation is what makes trading interesting. Without it, prices stay static and gameplay becomes boring repetition. This Game Mode breathes life into the trading world.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [ ] Unreal Engine 5.6+
- [ ] `AGameModeBase` class available (standard UE5 class)
- [ ] `UEconomyManager` subsystem compiled (TradingSystem module) - Or implement simple version in Blueprint
- [ ] At least 2-3 `UMarketDataAsset` created (e.g., `DA_Market_AgriculturalStation`, `DA_Market_IndustrialStation`)
- [ ] At least 5-10 `UTradeItemDataAsset` created
- [ ] `BP_TradingShip` created (see [Trading Ship Guide](BP_TradingShip_GUIDE.md))

**If you're missing any prerequisites**, see:
- [Trading Ship Guide](BP_TradingShip_GUIDE.md) - For player ship setup
- [MVP Quick Start](../docs/mvp/README.md) - For building TradingSystem module

---

## 🔗 Dependencies

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `BP_TradingShip` | Default player pawn | `Content/Blueprints/Ships/` |
| `BP_TradingStation` | Stations in world | `Content/Blueprints/Stations/` |

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `AGameModeBase` | Engine | Base game mode functionality |
| `UEconomyManager` | TradingSystem | Economy simulation and price updates (optional) |
| `UMarketDataAsset` | TradingSystem | Market inventory and pricing |
| `UTradeItemDataAsset` | TradingSystem | Item properties |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/Blueprints/GameModes/`
3. **Right-click** → Blueprint Class
4. **Choose Parent Class**: `Game Mode Base`
5. **Name it**: `BP_TradingGameMode`
6. **Double-click** to open the Blueprint Editor

### Step 2: Configure Class Settings

1. Click **Class Settings** (toolbar button)
2. Under **Default Pawn Class**:
   - Set to: `BP_TradingShip`
3. Under **HUD Class** (optional):
   - Leave as default or set custom HUD if created
4. Under **Player Controller Class**:
   - Leave as default (can customize later)

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: AllMarkets

- **Type**: `Array of Market Data Asset` (Object Reference Array)
- **Category**: `Economy`
- **Instance Editable**: Yes
- **Tooltip**: "All markets in the game world to simulate"
- **Default Value**: Empty (populate in level/instance)

### Variable 2: UpdateInterval

- **Type**: `Float`
- **Category**: `Economy`
- **Instance Editable**: Yes
- **Tooltip**: "How often (in seconds) to update market prices"
- **Default Value**: `30.0` (update every 30 seconds)

### Variable 3: PriceVolatility

- **Type**: `Float`
- **Category**: `Economy`
- **Instance Editable**: Yes
- **Tooltip**: "How much prices can change per update (0.0-1.0)"
- **Default Value**: `0.1` (10% max change)

### Variable 4: SupplyRecoveryRate

- **Type**: `Float`
- **Category**: `Economy`
- **Instance Editable**: Yes
- **Tooltip**: "How fast prices return to base values (0.0-1.0)"
- **Default Value**: `0.05` (5% per update)

### Variable 5: CurrentPrices

- **Type**: `Map of Name to Float` (TMap<FName, float>)
- **Category**: `Economy|Runtime`
- **Instance Editable**: No
- **Tooltip**: "Current price for each item (ItemID -> Price)"
- **Default Value**: Empty (populated at runtime)

### Variable 6: MarketSupply

- **Type**: `Map of Name to Integer` (TMap<FName, int32>)
- **Category**: `Economy|Runtime`
- **Instance Editable**: No
- **Tooltip**: "Current supply level for each item (ItemID -> Supply)"
- **Default Value**: Empty

### Variable 7: MarketDemand

- **Type**: `Map of Name to Integer` (TMap<FName, int32>)
- **Category**: `Economy|Runtime`
- **Instance Editable**: No
- **Tooltip**: "Current demand level for each item (ItemID -> Demand)"
- **Default Value**: Empty

### Variable 8: EconomyTimerHandle

- **Type**: `Timer Handle`
- **Category**: `Economy|Runtime`
- **Instance Editable**: No
- **Tooltip**: "Handle for economy update timer"
- **Default Value**: None

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing the main logic:**

### Function 1: InitializeEconomy

**Purpose**: Sets up initial prices and supply/demand for all markets

**Inputs**:
- None (uses `AllMarkets` array)

**Outputs**:
- None

**Implementation**: See [Step 1](#step-1-initialize-economy-on-game-start) below

### Function 2: UpdateEconomy

**Purpose**: Called periodically to update prices based on supply/demand

**Inputs**:
- None

**Outputs**:
- None

**Implementation**: See [Step 2](#step-2-implement-economy-update-logic) below

### Function 3: GetItemPrice

**Purpose**: Returns current price for a specific item at a market

**Inputs**:
- `ItemID` (Name) - The item identifier
- `MarketData` (Market Data Asset) - Which market to check

**Outputs**:
- `Price` (Float) - Current item price

**Implementation**: See [Step 3](#step-3-implement-price-calculation) below

### Function 4: OnPlayerTransaction

**Purpose**: Updates supply/demand when player buys or sells

**Inputs**:
- `ItemID` (Name) - Item traded
- `Quantity` (Integer) - Amount traded
- `WasBuying` (Boolean) - True if player bought, False if sold

**Outputs**:
- None

**Implementation**: See [Step 4](#step-4-implement-transaction-handling) below

### Function 5: CalculatePriceModifier

**Purpose**: Calculates price multiplier based on supply/demand ratio

**Inputs**:
- `ItemID` (Name) - Item to calculate for

**Outputs**:
- `Modifier` (Float) - Price multiplier (0.5 to 2.0)

**Implementation**: See [Step 5](#step-5-implement-price-modifier-calculation) below

---

## 🛠️ Step-by-Step Implementation

### Step 1: Initialize Economy on Game Start

**Goal**: Set up initial market prices and supply/demand values

**Blueprint Nodes**:

```
Event BeginPlay
  ↓
Call Parent (Super::BeginPlay)
  ↓
Print String: "Initializing economy..."
  ↓
InitializeEconomy
  ↓
Set Timer by Function Name
  ├─ Function Name: "UpdateEconomy"
  ├─ Time: UpdateInterval
  ├─ Looping: True
  └─ Output: EconomyTimerHandle
  ↓
Print String: "Economy active. Updates every [X] seconds"
```

**Detailed Instructions**:

1. In **Event Graph**, locate `Event BeginPlay`
2. **Drag from execution pin** and add `Parent: BeginPlay` (right-click → Add call to parent function)
3. Add `Print String` for debug: "Initializing economy..."
4. Call `InitializeEconomy` function
5. Add `Set Timer by Function Name`:
   - Function Name: "UpdateEconomy"
   - Time: Get `UpdateInterval` variable
   - Looping: `True`
   - Connect output to `EconomyTimerHandle` variable (Set)
6. Add another `Print String` showing update interval

**Expected Result**: 
- Console prints initialization message
- Economy updates start automatically
- Timer runs every UpdateInterval seconds

---

**InitializeEconomy Function**:

```
InitializeEconomy
  ↓
Clear: CurrentPrices Map
Clear: MarketSupply Map
Clear: MarketDemand Map
  ↓
ForEach Loop: AllMarkets
  ↓ (Loop Body for each Market)
  Get Market Items (from MarketDataAsset)
  ↓
  ForEach Loop: Market Items
    ↓ (Loop Body for each Item)
    Get Item Base Price
    ↓
    Get Item ID
    ↓
    Add to CurrentPrices Map
      ├─ Key: Item ID
      └─ Value: Base Price
    ↓
    Add to MarketSupply Map
      ├─ Key: Item ID
      └─ Value: 100 (neutral starting supply)
    ↓
    Add to MarketDemand Map
      ├─ Key: Item ID
      └─ Value: 100 (neutral starting demand)
  ↓
Print String: "Economy initialized with [X] items"
```

**Detailed Instructions**:

1. Open the **InitializeEconomy** function
2. **Clear all maps** (CurrentPrices, MarketSupply, MarketDemand)
3. Add **ForEach Loop** with `AllMarkets` array as input
4. For each market:
   - Get the market's available items list
   - Add another **ForEach Loop** for items
5. For each item:
   - Get item's `Base Price`
   - Get item's `ID` (FName)
   - **Add to CurrentPrices**: ItemID → BasePrice
   - **Add to MarketSupply**: ItemID → 100
   - **Add to MarketDemand**: ItemID → 100
6. After loops, print total item count

**Expected Result**: 
- All items have initial prices (base prices)
- Supply and demand start at neutral (100)
- Console shows item count

---

### Step 2: Implement Economy Update Logic

**Goal**: Periodically adjust prices based on supply/demand changes

**Blueprint Nodes**:

```
UpdateEconomy
  ↓
Print String: "Updating economy..."
  ↓
ForEach Loop: CurrentPrices Map
  ↓ (Loop Body for each ItemID, Price pair)
  Get Item ID (Key)
  ↓
  CalculatePriceModifier (ItemID)
  ↓
  Get Base Price from first Market with this item
  ↓
  New Price = Base Price × Price Modifier
  ↓
  Lerp (Current Price → New Price, SupplyRecoveryRate)
  ↓
  Update CurrentPrices Map
    ├─ Key: Item ID
    └─ Value: Lerped Price
  ↓
  Gradually recover supply/demand toward 100
    ├─ Current Supply = Lerp(Current → 100, Recovery Rate)
    └─ Current Demand = Lerp(Current → 100, Recovery Rate)
  ↓
Print String: "Economy updated. [X] items adjusted"
```

**Detailed Instructions**:

1. Open the **UpdateEconomy** function
2. Add debug `Print String`
3. Add **ForEach Loop** iterating over `CurrentPrices` map
4. For each item:
   - Get the ItemID (map key)
   - Call `CalculatePriceModifier` with ItemID
   - Get item's base price from Data Asset
   - Calculate new price: BasePrice × Modifier
   - **Lerp** from current price to new price (for smooth changes)
     - Alpha = `SupplyRecoveryRate`
   - Update `CurrentPrices` map with lerped value
5. Also gradually recover supply/demand to neutral (100):
   - Get current Supply from MarketSupply map
   - Lerp toward 100 with `SupplyRecoveryRate`
   - Set back to MarketSupply map
   - Repeat for MarketDemand
6. Print completion message

**Expected Result**: 
- Prices change gradually over time
- Supply/demand slowly return to neutral
- Console shows update every UpdateInterval

---

### Step 3: Implement Price Calculation

**Goal**: Get current price for an item at a specific market

**Blueprint Nodes**:

```
GetItemPrice (ItemID, MarketData input → returns Float)
  ↓
Get from CurrentPrices Map (Key: ItemID)
  ↓
Branch (Was found?)
  ├─ True:
  │   ↓
  │   Get Market Price Multiplier from MarketData
  │   ├─ Lookup ItemID in MarketData's PriceMultipliers
  │   └─ Default: 1.0 if not found
  │   ↓
  │   Final Price = Current Price × Market Multiplier
  │   ↓
  │   Return: Final Price
  │
  └─ False:
      ↓
      Print String: "WARNING: Item [ID] not in economy"
      ↓
      Return: 0.0
```

**Detailed Instructions**:

1. Open the **GetItemPrice** function
2. Add **Find** node with `CurrentPrices` map:
   - Key: `ItemID` input parameter
3. Add `Branch` checking if item was found
4. **If found**:
   - Get `MarketData` input parameter
   - Find item in market's `ItemPriceMultipliers` map
   - Default to 1.0 if not in this market's multipliers
   - **Multiply**: CurrentPrice × MarketMultiplier
   - Return result
5. **If not found**:
   - Print warning with ItemID
   - Return `0.0`

**Expected Result**: 
- Returns correct price for items
- Accounts for market-specific multipliers
- Warning if item doesn't exist

---

### Step 4: Implement Transaction Handling

**Goal**: Update supply/demand when player trades

**Blueprint Nodes**:

```
OnPlayerTransaction (ItemID, Quantity, WasBuying input)
  ↓
Print String: "Transaction: [Quantity]x [ItemID] (Buying: [Bool])"
  ↓
Branch (WasBuying?)
  ├─ True: (Player bought = supply decreased)
  │   ↓
  │   Get MarketSupply for ItemID
  │   ↓
  │   New Supply = Current - Quantity
  │   ↓
  │   Set MarketSupply (ItemID → New Supply)
  │   ↓
  │   Get MarketDemand for ItemID
  │   ↓
  │   New Demand = Current + (Quantity / 2)
  │   ↓
  │   Set MarketDemand (ItemID → New Demand)
  │
  └─ False: (Player sold = supply increased)
      ↓
      Get MarketSupply for ItemID
      ↓
      New Supply = Current + Quantity
      ↓
      Set MarketSupply (ItemID → New Supply)
      ↓
      Get MarketDemand for ItemID
      ↓
      New Demand = Current - (Quantity / 2)
      ↓
      Set MarketDemand (ItemID → New Demand)
  ↓
Print String: "Supply/Demand updated for [ItemID]"
```

**Detailed Instructions**:

1. Open the **OnPlayerTransaction** function
2. Add debug `Print String` showing transaction details
3. Add `Branch` checking `WasBuying` input
4. **If player bought** (took from market supply):
   - Get current Supply from `MarketSupply` map
   - Subtract `Quantity` (supply decreased)
   - Set new Supply in map
   - Get current Demand from `MarketDemand` map
   - Add `Quantity / 2` (buying signals demand increase)
   - Set new Demand in map
5. **If player sold** (added to market supply):
   - Get current Supply
   - Add `Quantity` (supply increased)
   - Update Supply
   - Get current Demand
   - Subtract `Quantity / 2` (selling signals demand decrease)
   - Update Demand
6. Print confirmation message

**Expected Result**: 
- Supply decreases when player buys
- Supply increases when player sells
- Demand adjusts accordingly
- Console confirms changes

---

### Step 5: Implement Price Modifier Calculation

**Goal**: Calculate how supply/demand affects price

**Blueprint Nodes**:

```
CalculatePriceModifier (ItemID input → returns Float)
  ↓
Get MarketSupply for ItemID (default: 100)
  ↓
Get MarketDemand for ItemID (default: 100)
  ↓
Supply/Demand Ratio = Demand / Supply
  ↓
Clamp (Ratio, Min: 0.5, Max: 2.0)
  ↓
Return: Clamped Ratio
```

**Formula Explanation**:
- If Demand > Supply: Ratio > 1.0 (price increases)
- If Supply > Demand: Ratio < 1.0 (price decreases)
- Example: Demand=150, Supply=50 → Ratio=3.0 → Clamped to 2.0 (price doubles)

**Detailed Instructions**:

1. Open the **CalculatePriceModifier** function
2. **Get Supply** from `MarketSupply` map (ItemID key)
   - Use `Find` with default value = 100
3. **Get Demand** from `MarketDemand` map (ItemID key)
   - Use `Find` with default value = 100
4. **Divide** Demand by Supply:
   - Add **Float / Float** node
   - Demand (numerator) / Supply (denominator)
5. **Clamp** the result:
   - Min: `0.5` (prices can't drop below 50% base)
   - Max: `2.0` (prices can't rise above 200% base)
6. **Return** the clamped ratio

**Expected Result**: 
- Returns 1.0 when supply/demand balanced
- Returns 0.5-2.0 based on imbalance
- Never returns extreme values (clamped)

---

### Step 6: Integrate with Trading UI (Event Dispatcher Pattern)

**Goal**: Allow other Blueprints to react to economy changes

**Blueprint Nodes**:

```
Event Dispatcher: OnEconomyUpdated
  ├─ Output: Array of Item IDs that changed
  └─ Output: Array of new Prices

[In UpdateEconomy function, at the end]
UpdateEconomy
  ↓
  [... existing logic ...]
  ↓
Call OnEconomyUpdated
  ├─ Changed Items: [Array of ItemIDs]
  └─ New Prices: [Array of Prices]
```

**Detailed Instructions**:

1. In **My Blueprint** panel, find **Event Dispatchers** section
2. **Add Event Dispatcher** named `OnEconomyUpdated`
3. Add output parameters:
   - `ChangedItems` (Array of Names)
   - `NewPrices` (Array of Floats)
4. In **UpdateEconomy** function, at the end:
   - Collect all ItemIDs that changed
   - Collect their new prices
   - **Call OnEconomyUpdated** with these arrays
5. Other Blueprints can now bind to this event

**Expected Result**: 
- Trading UI can listen for price changes
- Station displays update automatically
- HUD can show price alerts

---

### Step 7: Add Debug Commands (Optional)

**Goal**: Allow developer to manually trigger economy updates

**Blueprint Nodes**:

```
Event InputAction DebugUpdateEconomy (Console Command)
  ↓
Print String: "Manual economy update triggered"
  ↓
UpdateEconomy
```

**Detailed Instructions**:

1. In **Event Graph**, right-click → Add Event → Input → Console Events
2. Add **Execute Console Command** event
3. Command: "UpdateEconomy"
4. Add debug print
5. Call `UpdateEconomy` function
6. Can now type `UpdateEconomy` in console to test

**Expected Result**: 
- Can trigger updates anytime via console
- Useful for testing economy changes
- Helps debug price calculations

---

## 🧪 Testing Your Blueprint

### Test 1: Economy Initializes

**What to Test**: Game mode sets up economy on start

**Steps**:
1. Set **World Settings → GameMode Override** to `BP_TradingGameMode`
2. In `BP_TradingGameMode` instance, populate `AllMarkets` array with 2-3 markets
3. Press Play (PIE)
4. Check console output

**Success Criteria**: 
- Console shows "Initializing economy..."
- Console shows "Economy initialized with [X] items"
- No errors in Output Log
- Timer shows in debugger (if checking)

**If it fails**: See [Troubleshooting: Economy Not Initializing](#common-issue-1-economy-not-initializing)

---

### Test 2: Prices Update Over Time

**What to Test**: Economy updates periodically

**Steps**:
1. Start game
2. Wait 30 seconds (or whatever UpdateInterval is set to)
3. Check console for "Updating economy..." messages
4. Open trading UI at a station
5. Note item prices
6. Wait another 30 seconds
7. Check if prices changed

**Success Criteria**: 
- Console shows economy updates at regular intervals
- Prices fluctuate slightly over time
- No crashes or errors

**If it fails**: See [Troubleshooting: Updates Not Running](#common-issue-2-updates-not-running)

---

### Test 3: Player Transactions Affect Prices

**What to Test**: Buying/selling changes supply/demand

**Steps**:
1. Start game and dock at station
2. Note price of Water (e.g., 10 credits)
3. Buy 50 units of Water
4. Close and reopen trading UI
5. Check Water price (should be slightly higher)
6. Sell 100 units of Water
7. Check Water price again (should be lower)

**Success Criteria**: 
- Buying increases price
- Selling decreases price
- Changes visible in trading UI
- Console shows transaction messages

**If it fails**: See [Troubleshooting: Transactions Don't Affect Prices](#common-issue-3-transactions-dont-affect-prices)

---

### Test 4: Prices Return to Normal

**What to Test**: Supply/demand recovers over time

**Steps**:
1. Buy large quantity of item (e.g., 100 units)
2. Note the increased price
3. Wait 2-3 minutes (several update cycles)
4. Check price again

**Success Criteria**: 
- Price gradually returns toward base price
- Not instant recovery (gradual)
- Eventually stabilizes near base price

**If it fails**: See [Troubleshooting: Prices Don't Recover](#common-issue-4-prices-dont-recover)

---

## 🐛 Troubleshooting

### Common Issue 1: Economy Not Initializing

**Symptoms**:
- No console messages about economy
- Trading UI shows 0 or blank prices
- Game crashes on start

**Causes**:
- AllMarkets array empty
- Markets not properly configured
- BeginPlay not calling InitializeEconomy
- Data Assets not loaded

**Solutions**:
1. **Check AllMarkets populated**:
   - Select `BP_TradingGameMode` in World Settings
   - In Details panel, find `AllMarkets`
   - Add at least 1 market Data Asset
2. **Verify market Data Assets valid**:
   - Open each market Data Asset
   - Check they have items configured
   - Check items have valid base prices > 0
3. **Verify BeginPlay logic**:
   - Open BP_TradingGameMode
   - Check Event BeginPlay exists
   - Check InitializeEconomy is called
   - Check no compile errors (green checkmark)
4. **Check module compiled**:
   - Verify TradingSystem module compiled successfully
   - Verify Data Asset classes available in editor

---

### Common Issue 2: Updates Not Running

**Symptoms**:
- Initial prices set, but never change
- No "Updating economy..." messages after first one
- Timer not firing

**Causes**:
- Timer not set correctly
- Looping = False
- UpdateInterval = 0
- Timer handle not stored

**Solutions**:
1. **Verify timer settings**:
   - In BeginPlay, check `Set Timer by Function Name`
   - Function Name: "UpdateEconomy" (exact spelling)
   - Looping: **Must be True**
   - Time: > 0 (recommend 30.0)
2. **Check function name**:
   - Function must be named exactly "UpdateEconomy"
   - Case-sensitive
   - No spaces
3. **Verify timer handle**:
   - Output of Set Timer should connect to `EconomyTimerHandle`
   - Variable should be Timer Handle type
4. **Check for errors**:
   - Look in Output Log for timer errors
   - Check UpdateEconomy function has no compile errors

---

### Common Issue 3: Transactions Don't Affect Prices

**Symptoms**:
- Buy/sell items, but prices stay same
- No console messages about transactions
- Supply/demand not changing

**Causes**:
- OnPlayerTransaction not being called
- Trading UI not connected to Game Mode
- ItemID mismatch (wrong names)
- Math error in supply/demand logic

**Solutions**:
1. **Connect Trading UI to Game Mode**:
   - In `WBP_TradingUI` when buying/selling
   - Get Game Mode reference
   - Cast to `BP_TradingGameMode`
   - Call `OnPlayerTransaction` with correct parameters
2. **Verify ItemID matches**:
   - Print ItemID in OnPlayerTransaction
   - Check it matches Data Asset IDs exactly
   - Case-sensitive, no typos
3. **Add debug prints**:
   - In OnPlayerTransaction, print supply/demand before and after
   - Verify values are changing
   - Check math is correct (adding/subtracting right amounts)
4. **Check WasBuying parameter**:
   - True when player buys
   - False when player sells
   - Verify correct boolean passed

---

### Common Issue 4: Prices Don't Recover

**Symptoms**:
- Prices change from transactions
- But never return to base values
- Stuck at extreme prices

**Causes**:
- SupplyRecoveryRate = 0
- Supply/demand not lerping toward 100
- Lerp alpha value wrong
- Recovery logic not in UpdateEconomy

**Solutions**:
1. **Check SupplyRecoveryRate**:
   - Should be 0.01 to 0.1
   - 0.05 is good default (5% per update)
   - Not 0 (no recovery)
   - Not 1 (instant recovery)
2. **Verify lerp logic in UpdateEconomy**:
   - Supply lerps toward 100
   - Demand lerps toward 100
   - Alpha = SupplyRecoveryRate
   - Results saved back to maps
3. **Check update frequency**:
   - If UpdateInterval too long, recovery seems slow
   - Try 10-30 seconds for testing
   - Adjust SupplyRecoveryRate accordingly
4. **Add debug visualization**:
   - Print current supply/demand values
   - Watch them change over time
   - Should gradually approach 100

---

### Common Issue 5: Prices Too Extreme

**Symptoms**:
- Prices go to 0 or 1000000
- Economy becomes unplayable
- Prices not clamped properly

**Causes**:
- Clamp min/max values wrong
- Divide by zero in ratio calculation
- PriceVolatility too high
- Supply can go to 0

**Solutions**:
1. **Verify clamp in CalculatePriceModifier**:
   - Min should be 0.5 (prices can't go below 50%)
   - Max should be 2.0 (prices can't go above 200%)
   - Check clamp node connected correctly
2. **Prevent divide by zero**:
   - Before dividing Demand/Supply
   - Add check: if Supply <= 0, set to 1
   - Prevents infinity results
3. **Limit supply changes**:
   - Clamp Supply: Min=1, Max=1000
   - Clamp Demand: Min=1, Max=1000
   - Prevents extreme values
4. **Adjust PriceVolatility**:
   - Should be 0.05 to 0.2
   - Not 1.0 (too volatile)
   - Test with different values

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Market-Specific Events**
   - What it adds: Random events affect specific markets (drought, boom)
   - Complexity: Medium
   - Implementation: Add event system that modifies supply/demand randomly

2. **Trading Routes Display**
   - What it adds: Show profitable trade routes on HUD
   - Complexity: Medium
   - Implementation: Compare prices across markets, highlight best deals

3. **Economic Statistics**
   - What it adds: Track total trades, average prices, player profit
   - Complexity: Simple
   - Implementation: Add tracking variables, display in menu

4. **NPC Traders**
   - What it adds: AI traders also affect economy
   - Complexity: Advanced (out of MVP scope)
   - Note: Consider for Phase 2

5. **Save/Load Economy State**
   - What it adds: Persist prices across sessions
   - Complexity: Medium
   - Implementation: Serialize CurrentPrices, Supply, Demand maps to save game

---

## 📚 Related Guides

**If you want to learn more:**

- [Trading Ship Guide](BP_TradingShip_GUIDE.md) - Player ship setup
- [Trading UI Widget Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - UI integration
- [Trading Station Guide](TradingStationActorGuide.md) - Station setup
- [Data Assets Guide](../Assets/TradingSystemGuide.md) - Creating market data
- [MVP Phase 1 Checklist](../docs/mvp/PHASE_1_CHECKLIST.md) - Overall MVP progress

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a custom Game Mode in Unreal Engine
- ✅ How to implement dynamic economy simulation
- ✅ How to use timers for periodic updates
- ✅ How to use TMap (dictionaries) for tracking prices
- ✅ How to calculate supply/demand effects on prices
- ✅ How to integrate Game Mode with other Blueprints
- ✅ How to use Event Dispatchers for event-driven architecture

**Next Recommended Blueprint**: [Market Display Widget](WBP_MarketDisplay_GUIDE.md) - Visualize economy for players

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Adjust UpdateInterval for faster/slower economy changes
- Modify PriceVolatility for more/less price fluctuation
- Change clamp values in CalculatePriceModifier for wider price ranges
- Add seasonal events (harvest season, tech boom, etc.)
- Implement faction-specific economic bonuses

**Blueprint Best Practices Applied:**
- Used Timer Handle properly (stored in variable)
- Used TMap for efficient price lookups
- Event Dispatcher for loose coupling with UI
- Gradual changes (Lerp) for smooth gameplay
- Clamping to prevent extreme values

**Performance Considerations:**
- Economy updates only run periodically (not every frame)
- TMap lookups are O(1) - very fast
- Minimal garbage allocation (reusing variables)
- Can scale to 100+ items without performance issues

**Gameplay Balance Tips:**
- UpdateInterval: 20-60 seconds (feel responsive but not chaotic)
- SupplyRecoveryRate: 0.03-0.1 (gradual but noticeable)
- PriceVolatility: 0.05-0.15 (interesting but not overwhelming)
- Clamp range: 0.5-2.0 (prevents degenerate strategies)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-08 | Initial guide for MVP Phase 1 |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [Blueprint Image Guide](../docs/reference/BLUEPRINT_IMAGE_GUIDE.md) for node documentation
- See [MVP Documentation](../docs/mvp/README.md)

---

**Guide created for**: Trade Simulator MVP (Phase 1)  
**Maintained by**: Adastrea Development Team
