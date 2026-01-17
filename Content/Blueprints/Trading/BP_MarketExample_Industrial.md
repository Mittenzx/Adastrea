# Industrial Station Market Example Blueprint

**Type**: Example Configuration  
**Purpose**: Manufacturing/industrial economy marketplace  
**Complexity**: Beginner  
**Time to Implement**: 5-10 minutes

---

## Overview

Industrial station that produces manufactured goods and requires raw materials. Opposite economic profile from agricultural stations - expensive food, cheap metals.

## Blueprint Class: BP_Station_Industrial

### Components Required
1. **DockingBayModule** (required)
2. **MarketplaceModule** (required)
3. **CargoBayModule** x3 (recommended for large inventory)
4. **ManufacturingModule** (optional, for visual flavor)

---

## MarketplaceModule Configuration

### Class Defaults

**Market Data Asset**: `DA_Market_Industrial`

**Basic Settings:**
```
Market Name: "Industrial Depot"
Market Type: Industrial Depot
Market Size: Large
```

**Pricing Configuration:**
```
Sell Price Markup: 1.3
  (Player pays 30% markup - industrial goods are expensive)

Buy Price Markdown: 0.7
  (Player receives 30% less - station needs materials badly)

Transaction Tax Rate: 0.03
  (3% tax - slightly higher than agricultural)
```

**Access Control:**
```
Allow Player Buying: ✓ true
Allow Player Selling: ✓ true
Allow AI Traders: ✓ true
AI Trader Count: 8 (busy industrial hub)
```

---

## Market Inventory Configuration

### DA_Market_Industrial Inventory

**Manufactured Exports (Station Produces - Medium Prices):**

| Item | Current Stock | Max Stock | Supply Level | Demand Level |
|------|--------------|-----------|--------------|--------------|
| Basic Alloys | 8000 | 30000 | 1.5 (good supply) | 0.8 (moderate) |
| Refined Metals | 5000 | 20000 | 1.3 (good supply) | 0.9 (moderate) |
| Ship Components | 2000 | 10000 | 1.0 (normal) | 1.2 (some demand) |
| Basic Computers | 1000 | 5000 | 0.9 (limited) | 1.3 (demand) |

**Required Imports (Station Needs - Pays Well):**

| Item | Current Stock | Max Stock | Supply Level | Demand Level |
|------|--------------|-----------|--------------|--------------|
| Purified Water | 500 | 3000 | 0.3 (scarce) | 2.0 (high demand) |
| Protein Packs | 200 | 1000 | 0.2 (very scarce) | 2.3 (very high) |
| Hydrogen Fuel | 3000 | 15000 | 0.8 (limited) | 1.6 (demand) |
| Basic Medicine | 100 | 500 | 0.15 (very scarce) | 2.8 (extreme) |

---

## Calculated Prices (Example)

### Basic Alloys (Base Price: 50cr)
- **Station Buys** (from player): 50 × 0.7 × (1.5 / 0.8) = **65.6cr**
- **Station Sells** (to player): 50 × 1.3 × (0.8 / 1.5) = **34.7cr** ✅ Reasonable
- **Strategy**: Good item to buy here and sell to other industrials

### Purified Water (Base Price: 10cr)
- **Station Buys** (from player): 10 × 0.7 × (0.3 / 2.0) = **1.05cr** ❌ Terrible deal!
- **Station Sells** (to player): 10 × 1.3 × (2.0 / 0.3) = **86.7cr** ❌ Very expensive!
- **Strategy**: BRING water from agricultural stations! Huge arbitrage opportunity

### Ship Components (Base Price: 300cr)
- **Station Buys**: 300 × 0.7 × (1.0 / 1.2) = **175cr**
- **Station Sells**: 300 × 1.3 × (1.2 / 1.0) = **468cr** ✅ Good for buying
- **Strategy**: Buy components here, sell to stations without manufacturing

---

## Profitable Trade Routes

### From Agricultural → Industrial

**Water Arbitrage:**
- Buy at Agricultural: ~3cr/unit
- Sell at Industrial: ~87cr/unit
- **Profit**: 84cr/unit (2,800% ROI!) 💰💰💰

**Food Supplies:**
- Buy Protein Packs at Agricultural: ~25cr
- Sell at Industrial: ~180cr
- **Profit**: 155cr/unit (620% ROI!)

### From Industrial → Research

**Electronics:**
- Buy Computers at Industrial: ~650cr
- Sell at Research: ~350cr
- **Profit**: -300cr/unit ❌ (DON'T DO THIS!)

**Components:**
- Buy Ship Components at Industrial: ~468cr
- Sell at Luxury/Research: ~750cr
- **Profit**: 282cr/unit (60% ROI)

---

## AI Trader Settings

```
AI Trader Count: 8
  (Busy industrial hub with many traders)

AI Trade Frequency: 15
  (15 operations per day - high activity)

Allow AI Price Manipulation: false
  (MVP simplified - no manipulation)
```

---

## Market Events (Examples)

### Strike Event

```cpp
Event Name: "Worker Strike"
Description: "Production halted due to labor dispute"

Event ID: "industrial_strike"

Affected Items:
  - "basic_alloys"
  - "refined_metals"
  - "ship_components"
  - "basic_computers"

Price Multiplier: 1.8 (prices increase 80%)
Supply Multiplier: 0.4 (only 40% supply available)
Demand Multiplier: 1.5 (increased demand)

Duration: 48 hours (2 game days)
```

### Equipment Malfunction

```cpp
Event Name: "Manufacturing Breakdown"
Description: "Critical equipment failure reduces output"

Event ID: "industrial_breakdown"

Affected Items:
  - "ship_components"

Price Multiplier: 2.5 (components very expensive)
Supply Multiplier: 0.2 (only 20% output)
Demand Multiplier: 2.0 (urgent need elsewhere)

Duration: 72 hours (3 game days)
```

### Supply Convoy Arrival

```cpp
Event Name: "Supply Convoy"
Description: "Large convoy arrives with essential supplies"

Event ID: "industrial_convoy"

Affected Items:
  - "water_purified"
  - "protein_packs"
  - "hydrogen_fuel"

Price Multiplier: 0.6 (prices drop 40%)
Supply Multiplier: 3.0 (triple supply)
Demand Multiplier: 0.7 (reduced need temporarily)

Duration: 24 hours (1 game day)
```

---

## Blueprint Event Graph (Optional)

### OnMarketEventStarted - Strike Handler

```
Event OnMarketEventStarted (Event: FMarketEvent)
  ↓
Branch: Event.EventID == "industrial_strike"?
  ├─ True:
  │   ↓
  │   Print String: "ALERT: Worker Strike at {StationName}!"
  │   ↓
  │   Show UI Notification (Red, Urgent)
  │   ↓
  │   Spawn Strike VFX (picket signs, stopped machinery)
  │   ↓
  │   Play Audio: Industrial Alarm
  │   ↓
  │   Broadcast to Nearby Ships: "Production halted"
  │
  └─ False: [Check other events]
```

---

## Testing Checklist

### Functional Tests

- [ ] **Can Dock**: Ship successfully docks
- [ ] **Trading UI**: Opens without errors
- [ ] **Item Visibility**: All 8 items visible (4 exports + 4 imports)
- [ ] **Price Accuracy**: 
  - Alloys cheap (~35cr)
  - Water very expensive (~87cr)
  - Components moderate (~468cr)
- [ ] **Buy/Sell**: Both operations work
- [ ] **Stock Updates**: Inventory changes on transaction

### Economic Balance Tests

- [ ] **Water Arbitrage**: Buying water from agricultural is highly profitable
- [ ] **Component Export**: Buying components here for resale works
- [ ] **Food Import**: Selling food to station is profitable
- [ ] **Starter Viable**: With 1000cr capital, can participate in trades

### Event System Tests

- [ ] **Strike Event**: Activates correctly, prices increase
- [ ] **Convoy Event**: Supply increases, prices drop temporarily
- [ ] **Event Expiration**: Events end after duration

---

## Common Issues & Fixes

### Issue: "Water is too cheap at industrial station"
**This is CORRECT!**
- Industrial stations PAY POORLY for water (1cr) when buying
- Industrial stations SELL EXPENSIVELY (87cr) to players
- This creates the arbitrage opportunity (buy at agri, sell at industrial)

### Issue: "AI traders depleting all water stock"
**Fix**:
1. Increase water MaxStock to 5000+
2. Increase water ReplenishmentRate in TradeItemDataAsset
3. Consider limiting AI trader purchase quantities

### Issue: "Strike event makes station unusable"
**This is BY DESIGN**:
- Strikes are meant to be disruptive
- Creates gameplay variety and challenge
- Players should adapt by visiting other stations
- Duration is only 48 hours - wait it out or go elsewhere

---

## Integration with Other Markets

### Best Trading Partners

**Primary Partners:**
1. **Agricultural Stations**: Import food/water (high profit!)
2. **Research Stations**: Export components (moderate profit)
3. **Luxury Stations**: Export refined goods (good profit)
4. **Trade Hubs**: Balanced exchange (safety option)

**Avoid:**
- **Other Industrial Stations**: Similar goods, minimal profit
- **Military Depots**: Usually prefer contraband/weapons (not MVP)

---

## Quick Setup Steps

1. **Place Station**: Add BP_SpaceStation_Industrial to level
2. **Add Modules**: 
   - 2x DockingBayModule
   - 1x MarketplaceModule
   - 3x CargoBayModule
3. **Configure Market**: Assign DA_Market_Industrial
4. **Set Name**: "Industrial Depot" or custom name
5. **Test Trade**: 
   - Dock ship
   - Open trading UI
   - Buy alloys (~35cr each)
   - Verify water is expensive (~87cr)

**Result**: Fully functional industrial marketplace in ~10 minutes!

---

## Pro Tips

### For Players
- **Always bring water/food** from agricultural to industrial
- **Water arbitrage** is the easiest starter money-maker
- **Bulk cargo** (alloys, metals) good for high-capacity ships
- **Events** can dramatically change profitability - adapt!

### For Designers
- **Balance supply/demand** to encourage travel between stations
- **Make some items scarce** to create urgency
- **Industrial stations** should be HUNGRY for consumables
- **Strike events** add gameplay variety but don't overuse

---

## Related Examples

- `BP_MarketExample_Agricultural.md` - Food producer (opposite economy)
- `BP_MarketExample_Research.md` - Tech hub
- `BP_MarketExample_TradeHub.md` - Balanced general market

---

**Created**: 2026-01-17  
**Updated**: 2026-01-17  
**Version**: 1.0 (Trade Simulator MVP)
