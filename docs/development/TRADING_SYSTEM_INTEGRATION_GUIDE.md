# Trading System Integration Guide

**Purpose**: Step-by-step guide to integrate all MVP trading tools and content into Unreal Engine for the Trade Simulator prototype.

**Target Audience**: Programmers and technical designers setting up the trading prototype  
**Time to Complete**: 2-4 hours (first-time setup)  
**Prerequisites**: Unreal Engine 5.6, Python 3.9+, Adastrea trading system C++ code compiled

---

## Table of Contents

1. [Overview](#overview)
2. [Phase 1: Content Generation](#phase-1-content-generation)
3. [Phase 2: Data Asset Creation](#phase-2-data-asset-creation)
4. [Phase 3: Blueprint Setup](#phase-3-blueprint-setup)
5. [Phase 4: Level Setup](#phase-4-level-setup)
6. [Phase 5: Testing & Validation](#phase-5-testing--validation)
7. [Troubleshooting](#troubleshooting)

---

## Overview

### What You'll Build

By the end of this guide, you'll have:
- ✅ 15 trade items as Data Assets in Unreal
- ✅ 5 markets configured on space stations
- ✅ 1 playable trading ship with cargo system
- ✅ Trading UI that connects to markets
- ✅ 2 stations in a level ready for trading
- ✅ Profitable trade routes validated

### Integration Workflow

```
Python Tools → YAML Templates → Data Assets → Blueprints → Level → Playtest
     ↓              ↓               ↓            ↓          ↓         ↓
 Generate      Customize       Import       Configure   Place    Validate
  Content       Values        to Unreal     Components  Actors   Economics
```

### Time Breakdown

| Phase | Task | Time |
|-------|------|------|
| 1 | Generate content | 5 min |
| 2 | Create Data Assets | 30-45 min |
| 3 | Setup Blueprints | 45-60 min |
| 4 | Build test level | 30-45 min |
| 5 | Test & validate | 15-30 min |
| **Total** | | **2-4 hours** |

---

## Phase 1: Content Generation

### Step 1.1: Generate Trading Content

```bash
cd /path/to/Adastrea

# Generate all MVP content (15 items + 5 markets)
python3 Tools/generate_mvp_trading_content.py --all

# Output location:
# Content/DataAssets/Trading/MVP/Items/*.txt (15 files)
# Content/DataAssets/Trading/MVP/Markets/*.txt (5 files)
```

**Expected Output**:
```
✅ Generated 15 trade item configurations
✅ Generated 5 market configurations
✅ Files created in Content/DataAssets/Trading/MVP/
```

### Step 1.2: Validate Content

```bash
# Validate economic balance
python3 Tools/validate_trade_content.py

# Expected results:
# ✅ 15 trade items validated
# ✅ 5 markets validated
# ✅ 12+ profitable trade routes found
# ✅ 3+ items affordable for starter trades
```

**If validation fails**, check:
- YAML files are not corrupted
- Price ranges are reasonable
- Volume values are positive
- Market inventory references valid items

### Step 1.3: Test Performance

```bash
# Profile performance (should be A+ grade)
python3 Tools/trading_performance_profiler.py --analyze

# Expected:
# Market Lookups: <1ms ✅ A+
# Price Calculations: <1ms ✅ A+
# Overall Grade: A+
```

---

## Phase 2: Data Asset Creation

### Step 2.1: Create Trade Item Data Assets

**In Unreal Editor:**

1. Open Content Browser
2. Navigate to `Content/DataAssets/Trading/MVP/Items/`
3. For **each** of the 15 items:

   a. **Right-click** → Blueprint → Data Asset  
   b. **Select**: `TradeItemDataAsset` (C++ class)  
   c. **Name**: `DA_Item_[ItemName]` (e.g., `DA_Item_WaterPurified`)  
   d. **Open** the Data Asset  
   e. **Copy-paste** values from corresponding `.txt` file:

   **Example for Water:**
   ```
   From: Content/DataAssets/Trading/MVP/Items/DA_Item_WaterPurified.uasset.txt

   Basic Info:
   - Item Name: "Purified Water"
   - Description: "Essential commodity..."
   - Item ID: "water_purified"

   Economy:
   - Base Price: 10.0
   - Volume Per Unit: 1.0
   - Volatility: 0.2

   Supply/Demand:
   - Base Supply Level: 2.0 (high supply)
   - Base Demand Level: 0.8 (medium demand)

   AI Trading:
   - Min Trade Quantity: 5
   - Max Trade Quantity: 50
   - Priority: 1.0
   ```

   f. **Set each property** in the Data Asset  
   g. **Save** the Data Asset

4. **Repeat** for all 15 items

**Time**: ~30 minutes (2 min per item)

### Step 2.2: Create Market Data Assets

**In Unreal Editor:**

1. Navigate to `Content/DataAssets/Trading/MVP/Markets/`
2. For **each** of the 5 markets:

   a. **Right-click** → Blueprint → Data Asset  
   b. **Select**: `MarketDataAsset` (C++ class)  
   c. **Name**: `DA_Market_[StationName]` (e.g., `DA_Market_AgriculturalStationMarket`)  
   d. **Open** the Data Asset  
   e. **Copy-paste** values from corresponding `.txt` file:

   **Example for Agricultural Market:**
   ```
   Market Configuration:
   - Market Name: "Farming Collective Market"
   - Market Type: Open Market
   - Market Size: Medium

   Pricing:
   - Sell Price Markup: 1.1 (player pays 10% more)
   - Buy Price Markdown: 0.9 (player gets 10% less)

   Inventory (6 items):
   
   [0] Water Purified:
       - Trade Item: DA_Item_WaterPurified (reference)
       - Current Stock: 10000
       - Max Stock: 50000
       - Supply Level: 2.0 (abundant)
       - Demand Level: 0.5 (low demand)
   
   [1] Protein Packs:
       - Trade Item: DA_Item_ProteinPacks
       - Current Stock: 5000
       - Max Stock: 20000
       - Supply Level: 1.5
       - Demand Level: 0.8
   
   [2] Luxury Food:
       - Trade Item: DA_Item_LuxuryFood
       - Current Stock: 500
       - Max Stock: 2000
       - Supply Level: 0.8
       - Demand Level: 1.2
   
   [3] Basic Computers:
       - Trade Item: DA_Item_BasicComputers
       - Current Stock: 50
       - Max Stock: 200
       - Supply Level: 0.3 (scarce)
       - Demand Level: 2.0 (high demand - import)
   
   [4] Ship Components:
       - Trade Item: DA_Item_ShipComponents
       - Current Stock: 30
       - Max Stock: 150
       - Supply Level: 0.2
       - Demand Level: 2.5
   
   [5] Advanced Medicine:
       - Trade Item: DA_Item_AdvancedMedicine
       - Current Stock: 20
       - Max Stock: 100
       - Supply Level: 0.1
       - Demand Level: 3.0
   ```

   f. **Set Market Configuration** properties  
   g. **Add Inventory array entries** (6-8 per market)  
   h. **Reference corresponding TradeItem Data Assets**  
   i. **Set supply/demand levels** per item  
   j. **Save** the Data Asset

3. **Repeat** for all 5 markets

**Time**: ~45 minutes (9 min per market)

### Step 2.3: Verify Data Assets

**Checklist:**
- [ ] 15 `DA_Item_*` Data Assets created
- [ ] 5 `DA_Market_*` Data Assets created
- [ ] All Data Assets have no errors (check Output Log)
- [ ] Trade item references resolve correctly in markets
- [ ] Prices are reasonable (10cr to 10,000cr range)
- [ ] Supply/demand values are 0.1 to 3.0 range

---

## Phase 3: Blueprint Setup

### Step 3.1: Create Trading Ship Blueprint

**Follow**: `Content/Blueprints/Trading/BP_TradingShip_Starter_Template.md`

**Quick Setup (30 min):**

1. **Create Blueprint**:
   - Right-click → Blueprint Class → Pawn
   - Name: `BP_TradingShip_Starter`
   - Parent Class: `ASpaceship` (if available) or `APawn`

2. **Add Components**:
   ```
   BP_TradingShip_Starter (Root)
   ├─ ShipMesh (Static Mesh)
   ├─ CargoComponent (C++ component: UCargoComponent)
   ├─ PlayerTraderComponent (C++ component: UPlayerTraderComponent)
   ├─ DockingCollision (Sphere Collision)
   ├─ SpringArm
   └─ Camera
   ```

3. **Configure CargoComponent**:
   - Cargo Capacity: 10 (starter ship)
   - Max Cargo Weight: 1000.0
   - Cargo Volume Limit: 10.0

4. **Configure DockingCollision**:
   - Sphere Radius: 500.0
   - Collision Preset: OverlapAllDynamic
   - Generate Overlap Events: ✅ True

5. **Create Event Graph Functions**:
   - `OnDockingAreaEntered` - Detect when near station
   - `RequestDocking` - Initiate docking sequence
   - `OpenTradingUI` - Show market interface
   - `BuyItem` - Purchase from market
   - `SellItem` - Sell to market
   - `Undock` - Leave station

6. **Implement Buy Function** (example):
   ```
   Event: OnBuyButtonClicked
     ↓
   Get Player Credits
     ↓
   Get Item Price (from MarketDataAsset)
     ↓
   Branch: Can Afford?
     ├─ True:
     │   ↓
     │   CargoComponent->AddCargo(ItemID, Quantity)
     │   ↓
     │   Branch: Success?
     │   ├─ True:
     │   │   ↓
     │   │   Subtract Credits
     │   │   ↓
     │   │   Update UI
     │   │   ↓
     │   │   Play Success Sound/VFX
     │   │
     │   └─ False:
     │       ↓
     │       Show Error: "Not enough cargo space"
     │
     └─ False:
         ↓
         Show Error: "Not enough credits"
   ```

7. **Save** and **Compile**

**Time**: ~30 minutes

### Step 3.2: Create Trading UI Widget

**Follow**: `Content/Blueprints/Trading/WBP_TradingUI_Template.md`

**Quick Setup (30 min):**

1. **Create Widget Blueprint**:
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_TradingUI`

2. **Widget Hierarchy**:
   ```
   Canvas Panel (Root)
   ├─ Vertical Box (Main Container)
   │   ├─ Text Block (Station Name)
   │   ├─ Horizontal Box (Player Info)
   │   │   ├─ Text Block (Credits: 1000cr)
   │   │   └─ Text Block (Cargo: 5/10)
   │   ├─ Scroll Box (Item List)
   │   │   └─ WBP_TradeItemRow (repeated for each item)
   │   └─ Horizontal Box (Actions)
   │       ├─ Button (Buy)
   │       ├─ Button (Sell)
   │       └─ Button (Close)
   ```

3. **Create Child Widget** `WBP_TradeItemRow`:
   ```
   Horizontal Box (Root)
   ├─ Text Block (Item Name: "Water")
   ├─ Text Block (Stock: "1000 units")
   ├─ Text Block (Buy Price: "11cr")
   ├─ Text Block (Sell Price: "9cr")
   ├─ Editable Text Box (Quantity: "1")
   └─ Checkbox (Select)
   ```

4. **Bind Properties** to functions:
   - Credits → Get Player Credits
   - Cargo → Get Current Cargo / Max Cargo
   - Item List → Populate from MarketDataAsset

5. **Implement Button Events**:
   - Buy → Call `BP_TradingShip->BuyItem()`
   - Sell → Call `BP_TradingShip->SellItem()`
   - Close → Remove from viewport

6. **Save** and **Compile**

**Time**: ~30 minutes

### Step 3.3: Setup Market Blueprints

**For Agricultural Station:**

**Follow**: `Content/Blueprints/Trading/BP_MarketExample_Agricultural.md`

1. **Open or Create**: `BP_SpaceStation_Agricultural`
2. **Add MarketplaceModule**:
   - Default Module Classes → Add: `BP_SpaceStationModule_Marketplace`
3. **Configure Marketplace**:
   - Market Data Asset: `DA_Market_AgriculturalStationMarket`
   - Is Open: ✅ True
   - Marketplace Name: "Farming Collective Market"
4. **Save**

**For Industrial Station:**

**Follow**: `Content/Blueprints/Trading/BP_MarketExample_Industrial.md`

1. **Open or Create**: `BP_SpaceStation_Industrial`
2. **Add MarketplaceModule**:
   - Default Module Classes → Add: `BP_SpaceStationModule_Marketplace`
3. **Configure Marketplace**:
   - Market Data Asset: `DA_Market_IndustrialStationMarket`
   - Is Open: ✅ True
   - Marketplace Name: "Industrial Exchange"
4. **Save**

**Time**: ~15 minutes (both stations)

---

## Phase 4: Level Setup

### Step 4.1: Create Test Level

1. **Create New Level**:
   - File → New Level → Empty Level
   - Name: `L_TradingPrototype`
   - Save to: `Content/Maps/Trading/`

2. **Add Lighting**:
   - Directional Light (Sun)
   - Sky Atmosphere
   - Sky Light

3. **Add Player Start**:
   - Place at: (0, 0, 100)

### Step 4.2: Place Stations

1. **Add Agricultural Station**:
   - Drag `BP_SpaceStation_Agricultural` into level
   - Location: (1000, 0, 0)
   - Rotation: (0, 0, 0)

2. **Add Industrial Station**:
   - Drag `BP_SpaceStation_Industrial` into level
   - Location: (5000, 0, 0)
   - Rotation: (0, 0, 0)

3. **Distance**: 4000 units apart (~40 seconds flight time)

### Step 4.3: Configure Player Ship

1. **Set Game Mode**:
   - World Settings → Game Mode Override: `BP_TradingGameMode`
   - Default Pawn Class: `BP_TradingShip_Starter`

2. **Initial Credits**: 1000 (set in PlayerTraderComponent)

3. **Save** level

**Time**: ~30 minutes

---

## Phase 5: Testing & Validation

### Step 5.1: Play Test Checklist

**Start Test**:
- Hit **Play** in editor (PIE)

**Test Sequence**:

1. **Flight**:
   - [ ] Ship spawns correctly
   - [ ] Flight controls work
   - [ ] Camera follows ship

2. **Docking at Agricultural Station**:
   - [ ] Fly to agricultural station (1000, 0, 0)
   - [ ] Docking collision triggers
   - [ ] "Press E to dock" prompt appears
   - [ ] Trading UI opens when docking

3. **View Market**:
   - [ ] Station name shows: "Farming Collective Market"
   - [ ] 6 items displayed
   - [ ] Water price: ~3cr (cheap)
   - [ ] Electronics price: ~5500cr (expensive)
   - [ ] Player credits: 1000cr
   - [ ] Cargo space: 0/10

4. **Buy Water** (starter trade):
   - [ ] Select Water
   - [ ] Set quantity: 10 units
   - [ ] Total cost: ~30cr
   - [ ] Click "Buy"
   - [ ] Success message
   - [ ] Credits: 970cr
   - [ ] Cargo: 10/10 (full!)

5. **Undock**:
   - [ ] Click "Close" or "Undock"
   - [ ] UI closes
   - [ ] Ship can move again

6. **Fly to Industrial Station**:
   - [ ] Navigate to (5000, 0, 0)
   - [ ] Flight time: ~40 seconds

7. **Dock at Industrial Station**:
   - [ ] Docking works
   - [ ] "Industrial Exchange" UI opens
   - [ ] 8 items displayed

8. **Sell Water** (complete trade):
   - [ ] Water buy price: ~87cr (expensive here!)
   - [ ] Select Water
   - [ ] Quantity: 10 units (all cargo)
   - [ ] Expected revenue: ~870cr
   - [ ] Click "Sell"
   - [ ] Success message
   - [ ] Credits: 1840cr (profit: 840cr!)
   - [ ] Cargo: 0/10 (empty)

9. **Verify Profit**:
   - [ ] Started with: 1000cr
   - [ ] Ended with: 1840cr
   - [ ] Profit: 840cr (84% ROI)
   - [ ] ✅ Trade route validated!

### Step 5.2: Economic Validation

Run profit calculator to confirm:

```bash
python3 Tools/trade_profit_calculator.py \
  --from agricultural --to industrial \
  --item water_purified --quantity 10 \
  --capital 1000
```

**Expected Output**:
```
Route: Agricultural → Industrial
Item: Water Purified
Quantity: 10 units
Buy Price: 3cr × 10 = 30cr
Sell Price: 87cr × 10 = 870cr
Profit: 840cr (2800% ROI)
✅ Highly profitable starter trade
```

### Step 5.3: Performance Validation

**In Editor**:
- Enable **stat game** and **stat fps**
- **Target**: 60 FPS with trading UI open
- **Check**: No stuttering during trades

**Command Line Test**:
```bash
python3 Tools/trading_performance_profiler.py --analyze
```

**Expected**: Overall Grade A+ (no optimization needed)

### Step 5.4: Integration Test Suite

```bash
cd tests
python3 test_trading_integration.py
```

**Expected Results**:
```
✅ test_content_generation_successful
✅ test_all_markets_have_items
✅ test_profitable_routes_exist
✅ test_starter_ship_affordability
✅ test_price_balance_reasonable
✅ test_market_diversity
... (11 tests total)

8-10 passing, 1-3 revealing balance issues (acceptable)
```

---

## Troubleshooting

### Issue: "Trading UI doesn't open"

**Symptoms**: Docking works, but no UI appears

**Fixes**:
1. Check `BP_TradingShip` Event Graph:
   - Is `OpenTradingUI` function called after docking?
   - Is `WBP_TradingUI` referenced correctly?
2. Check `WBP_TradingUI`:
   - Is "Is Variable" checked?
   - Is widget added to viewport?
3. Check Output Log for Blueprint errors

### Issue: "Can't buy items - always says 'not enough credits'"

**Symptoms**: Buy button fails even with sufficient credits

**Fixes**:
1. Check price calculation:
   - Is `GetItemPrice()` function correct?
   - Is it using `SellPriceMarkup` (not markdown)?
   - Formula should be: `BasePrice × SellPriceMarkup`
2. Check credits:
   - Is `PlayerTraderComponent` initialized?
   - Is initial credits set correctly?
3. Debug print:
   - Print `PlayerCredits` vs `TotalCost` before branch

### Issue: "Cargo always says 'no space'"

**Symptoms**: Can't add cargo even when empty

**Fixes**:
1. Check `CargoComponent` configuration:
   - Cargo Capacity: Should be > 0 (e.g., 10)
   - Volume limits: Should match item volumes
2. Check `AddCargo()` function:
   - Is volume calculated correctly?
   - Is `VolumePerUnit` property set in TradeItemDataAsset?
3. Test in isolation:
   - Create test function that just adds 1 item
   - Check if basic cargo addition works

### Issue: "Prices seem wrong"

**Symptoms**: All items same price, or prices don't match templates

**Fixes**:
1. Check `MarketDataAsset`:
   - Is `SellPriceMarkup` set correctly? (e.g., 1.1)
   - Is `BuyPriceMarkdown` set correctly? (e.g., 0.9)
2. Check `TradeItemDataAsset`:
   - Is `BasePrice` set? (not 0)
3. Verify calculation:
   - Player Buying: `BasePrice × SellPriceMarkup`
   - Player Selling: `BasePrice × BuyPriceMarkdown`
4. Run validator:
   ```bash
   python3 Tools/validate_trade_content.py
   ```

### Issue: "Performance is bad"

**Symptoms**: FPS drops, stuttering

**Fixes**:
1. Profile:
   ```bash
   python3 Tools/trading_performance_profiler.py --stress-test
   ```
2. Check for issues:
   - Too many AI traders updating every frame?
   - Price calculations on Tick instead of cached?
   - UI updating every frame unnecessarily?
3. Optimize:
   - Cache prices (recalculate only on market events)
   - Stagger AI updates across frames
   - Update UI only when values change

### Issue: "Data Assets show errors in editor"

**Symptoms**: Red errors in Data Asset editor

**Fixes**:
1. Check references:
   - Are all `TradeItem` references valid?
   - Are Data Assets in correct folders?
2. Recompile C++:
   - Close editor
   - Rebuild Adastrea project
   - Reopen editor
3. Regenerate:
   - Delete problematic Data Asset
   - Re-run generator
   - Recreate Data Asset with new values

---

## Next Steps

After completing integration:

1. **Add More Content** (Week 2-4):
   - More trade items (20-30 total)
   - More markets (10 total)
   - Ship progression (medium and large cargo ships)

2. **Add Dynamic Economy** (Week 5-8):
   - Supply/demand simulation
   - Market events (shortages, surpluses)
   - AI trader behavior

3. **Add Polish** (Week 9-12):
   - VFX for trades
   - Sound effects
   - Tutorial system
   - Save/load

4. **Test with Players** (Week 4, 8, 12):
   - External playtesting
   - Gather feedback
   - Iterate on fun factor

---

## Summary

**What You Built**:
- ✅ 15 trade items in Unreal
- ✅ 5 markets on stations
- ✅ Playable trading ship
- ✅ Working trading UI
- ✅ 2-station test level
- ✅ Validated trade routes

**Time Spent**: 2-4 hours

**Next**: Follow MVP Quick Start Guide for week-by-week development plan

**Success Criteria**:
- Can fly between stations (1 min flight time)
- Can buy/sell items successfully
- Profit calculation works (see 840cr profit example)
- No crashes or critical bugs

---

**Guide Complete**: You now have a working Trade Simulator MVP prototype!

**Questions?** Check other documentation:
- `MVP_QUICK_START_GUIDE.md` - Week-by-week roadmap
- `TRADING_ECONOMICS_CHEAT_SHEET.md` - Balance formulas
- `TRADING_CPP_API_REFERENCE.md` - C++ programming
- Blueprint templates - Detailed implementation guides

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 1-4 Prototype Phase)
