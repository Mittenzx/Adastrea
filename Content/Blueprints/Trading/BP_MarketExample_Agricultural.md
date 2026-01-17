# Agricultural Market Example Blueprint

**Type**: Example Configuration  
**Purpose**: Ready-to-use agricultural marketplace setup  
**Complexity**: Beginner  
**Time to Implement**: 5-10 minutes

---

## Overview

This example shows how to configure an agricultural station marketplace. Copy these settings directly into your Blueprint to create a functional food-producing economy.

## Blueprint Class: BP_Station_Agricultural

### Parent Class
- `ASpaceStation` (or your custom space station class)

### Components Required
1. **DockingBayModule** (required for ship docking)
2. **MarketplaceModule** (required for trading)
3. **CargoBayModule** (optional, for station storage)

---

## MarketplaceModule Configuration

### Class Defaults

**Market Data Asset**: `DA_Market_Agricultural` (created from templates)

**Basic Settings:**
```
Market Name: "Farming Collective Market"
Market Type: Open Market
Market Size: Medium
```

**Pricing Configuration:**
```
Sell Price Markup: 1.1
  (Player pays 10% above base when buying from station)

Buy Price Markdown: 0.9
  (Player receives 10% below base when selling to station)

Transaction Tax Rate: 0.02
  (2% tax on all transactions)
```

**Access Control:**
```
Allow Player Buying: ✓ true
Allow Player Selling: ✓ true
Allow AI Traders: ✓ true
```

---

## Market Inventory Configuration

### DA_Market_Agricultural Inventory

**Produce Items (Station Exports - Cheap Here):**

| Item | Current Stock | Max Stock | Supply Level | Demand Level |
|------|--------------|-----------|--------------|--------------|
| Purified Water | 10000 | 50000 | 2.0 (abundant) | 0.5 (low demand) |
| Protein Packs | 5000 | 20000 | 1.5 (good supply) | 0.8 (moderate) |
| Luxury Food | 500 | 2000 | 0.8 (limited) | 1.2 (high demand) |

**Import Items (Station Needs - Expensive Here):**

| Item | Current Stock | Max Stock | Supply Level | Demand Level |
|------|--------------|-----------|--------------|--------------|
| Basic Electronics | 100 | 500 | 0.2 (scarce) | 2.0 (high demand) |
| Ship Components | 50 | 300 | 0.3 (scarce) | 1.8 (high demand) |
| Advanced Medicine | 20 | 100 | 0.1 (very scarce) | 2.5 (very high) |

---

## Calculated Prices (Example)

### Water (Base Price: 10cr)
- **Station Buys** (from player): 10 × 0.9 × (2.0 / 0.5) = **36.0cr**
- **Station Sells** (to player): 10 × 1.1 × (0.5 / 2.0) = **2.75cr** ✅ Cheap!
- **Player Profit if Bought Here**: High potential if sold at industrial station

### Basic Electronics (Base Price: 500cr)
- **Station Buys** (from player): 500 × 0.9 × (0.2 / 2.0) = **45.0cr** ❌ Bad deal for player
- **Station Sells** (to player): 500 × 1.1 × (2.0 / 0.2) = **5500cr** ❌ Very expensive!
- **Player Strategy**: Don't buy electronics here - go to Research Station

---

## AI Trader Settings

```
AI Trader Count: 5
  (5 AI traders operating in this market)

AI Trade Frequency: 10
  (10 trade operations per game day)

Allow AI Price Manipulation: false
  (AI cannot manipulate supply/demand - MVP simplified)
```

---

## Stock Refresh Settings

```
Stock Refresh Rate: 24.0
  (Stock replenishes every 24 game hours)

Last Stock Refresh Time: 0.0
  (Automatically updated by EconomyManager)
```

---

## Market Events (Optional)

### Example: Drought Event

```cpp
Event Name: "Severe Drought"
Event Description: "Water supplies critically low due to equipment failure"

Event ID: "agricultural_drought"

Affected Item IDs:
  - "water_purified"

Price Multiplier: 3.0 (water becomes 3x more expensive)
Supply Multiplier: 0.3 (only 30% supply available)
Demand Multiplier: 2.0 (demand doubles)

Duration Hours: 48.0 (lasts 2 game days)
Start Time: 0.0 (set by EconomyManager when event triggers)
Is Active: false (activated by event system)
```

### Example: Bumper Crop Event

```cpp
Event Name: "Bumper Crop"
Event Description: "Exceptional harvest yields surplus food"

Event ID: "agricultural_surplus"

Affected Item IDs:
  - "protein_packs"
  - "luxury_food"

Price Multiplier: 0.5 (food becomes 50% cheaper)
Supply Multiplier: 2.5 (250% supply available)
Demand Multiplier: 0.8 (reduced demand)

Duration Hours: 72.0 (lasts 3 game days)
Start Time: 0.0
Is Active: false
```

---

## Blueprint Event Graph (Optional Customization)

### OnStockRefreshed Event

```
Event OnStockRefreshed
  ↓
Print String: "Agricultural Station: Stock refreshed!"
  ↓
[Optional] Spawn VFX at station
  ↓
[Optional] Play sound effect
```

### OnMarketEventStarted Event

```
Event OnMarketEventStarted
  ↓
Branch: Event.EventID == "agricultural_drought"?
  ├─ True:
  │   ↓
  │   Print String: "ALERT: Water shortage at {StationName}!"
  │   ↓
  │   Show UI Notification
  │   ↓
  │   Broadcast to nearby ships
  │
  └─ False: [Other event handlers]
```

---

## Testing Checklist

### Functional Tests

- [ ] **Can Dock**: Ship can dock at station
- [ ] **Trading UI Opens**: Marketplace UI appears when docked
- [ ] **Item Visibility**: All 6 items (3 exports + 3 imports) are visible
- [ ] **Correct Prices**: Water is cheap (~3cr), Electronics are expensive (~5500cr)
- [ ] **Can Buy**: Successfully purchase water and food
- [ ] **Can Sell**: Successfully sell electronics to station
- [ ] **Stock Updates**: Stock levels decrease when buying

### Economic Balance Tests

- [ ] **Profitable Export**: Water purchased here sells for profit elsewhere
- [ ] **Unprofitable Import**: Electronics bought here lose money
- [ ] **Starter Affordable**: Player with 1000cr can buy at least 100 units of water
- [ ] **Progression Path**: Profits from food enable larger purchases

### Performance Tests

- [ ] **No Lag**: Trading UI responds in <100ms
- [ ] **Stock Refresh**: Refresh occurs without frame drops
- [ ] **Event System**: Market events activate/deactivate smoothly

---

## Common Issues & Fixes

### Issue: "No items showing in marketplace"
**Fix**: 
1. Check MarketDataAsset is assigned to MarketplaceModule
2. Verify Inventory array has items
3. Ensure `bInStock = true` for inventory entries

### Issue: "Prices don't match expected values"
**Fix**:
1. Verify SellPriceMarkup = 1.1 (not 11.0!)
2. Verify BuyPriceMarkdown = 0.9 (not 9.0!)
3. Check Supply/Demand levels are correct
4. Formula: `Price = BasePrice × Markup × (Demand / Supply)`

### Issue: "Can't buy items"
**Fix**:
1. Check `bAllowPlayerBuying = true`
2. Verify CurrentStock > 0
3. Check player has enough credits
4. Ensure cargo space available

---

## Quick Setup Steps

1. **Create Station**: Place BP_SpaceStation_Agricultural in level
2. **Assign Modules**: Add DockingBay + Marketplace + CargoBay modules
3. **Configure Market**: Set DA_Market_Agricultural on MarketplaceModule
4. **Test**: Dock ship, open trading UI, verify items and prices
5. **Tweak**: Adjust prices if needed for balance

**Total Time**: 5-10 minutes from start to testable marketplace!

---

## Related Examples

- `BP_MarketExample_Industrial.md` - Manufacturing economy
- `BP_MarketExample_Research.md` - Technology hub
- `BP_MarketExample_TradeHub.md` - Balanced general market
- `BP_MarketExample_Luxury.md` - High-end goods

---

**Created**: 2026-01-17  
**Updated**: 2026-01-17  
**Version**: 1.0 (Trade Simulator MVP)
