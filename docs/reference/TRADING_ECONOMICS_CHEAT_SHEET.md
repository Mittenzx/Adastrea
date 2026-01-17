# Trading Economics - Quick Reference

**MVP Trade Simulator - Cheat Sheet for Designers**

---

## 💰 Price Tiers

| Tier | Range | Examples | Starter Ship Can Afford? |
|------|-------|----------|--------------------------|
| **Basic** | 10-50cr | Water, Food, Fuel | ✅ Yes (100+ units) |
| **Industrial** | 50-300cr | Alloys, Components | ✅ Yes (10-20 units) |
| **Technology** | 500-5,000cr | Computers, Electronics, AI | ⚠️ Limited (1-2 units) |
| **Medical** | 200-1,000cr | Medicine, Equipment | ✅ Yes (2-5 units) |
| **Luxury** | 600-10,000cr | Art, Entertainment, Rare Items | ❌ Endgame only |

**Starter Capital**: 1,000 credits

---

## 📊 Profitable Trade Routes

### Beginner (Low Capital, Low Risk)
```
Agricultural → Industrial
Buy:  Water (8cr) × 100 = 800cr
Sell: Water (12cr) × 100 = 1,200cr
Profit: 400cr (50% ROI) ✅
```

### Intermediate (Medium Capital)
```
Research → Industrial
Buy:  Basic Computers (450cr) × 2 = 900cr
Sell: Basic Computers (700cr) × 2 = 1,400cr
Profit: 500cr (56% ROI) ✅
```

### Advanced (High Capital, High Risk)
```
Research → Luxury Bazaar
Buy:  AI Cores (4,000cr) × 1 = 4,000cr
Sell: AI Cores (10,000cr) × 1 = 10,000cr
Profit: 6,000cr (150% ROI) 💰
```

---

## 🏪 Station Economic Profiles

| Station Type | Produces (Cheap) | Imports (Expensive) | Strategy |
|--------------|------------------|---------------------|----------|
| **Agricultural** | Food, Water | Tech, Components | Buy food, sell tech |
| **Industrial** | Metals, Alloys | Food, Supplies | Buy materials, sell food |
| **Research** | Computers, Tech | Materials, Supplies | Buy tech, sell materials |
| **Luxury Bazaar** | - | Everything (2x markup) | Sell rare items only |
| **Trade Hub** | - | Balanced prices | Compare prices hub |

---

## 📦 Cargo Capacity Guide

| Ship Tier | Cargo Capacity | Cost | Best For |
|-----------|----------------|------|----------|
| **Starter** | 10 units | Starting ship | Small high-value items |
| **Medium** | 30 units | 50,000cr | Bulk commodities |
| **Advanced** | 100 units | 250,000cr | Large bulk shipments |

**Upgrade Priority**: Cargo capacity → Speed → Fuel efficiency

---

## 💡 Economic Formulas

### Price Calculation
```
Final Price = BasePrice × SupplyFactor × DemandFactor × MarketMarkup × (1 + Tax)

Where:
- SupplyFactor = 1.0 / Supply (high supply = lower price)
- DemandFactor = Demand (high demand = higher price)
- MarketMarkup = 1.1-2.0 (player buying) or 0.7-0.9 (player selling)
- Tax = 0.03-0.15 (3-15%)
```

### Profit Calculation
```
Profit = (SellPrice - BuyPrice) × Quantity - TravelCost

ROI% = (Profit / Investment) × 100
```

### Cargo Efficiency
```
Value Density = Price / Volume
Target: >50 cr/m³ for profitable trades
```

---

## 🎯 Balancing Guidelines

### Supply/Demand Levels
- **Abundant** (>2.0): Station produces this
- **Normal** (1.0): Balanced availability
- **Scarce** (<0.5): Station desperately needs this

### Price Markups
- **Low Markup** (1.1): Agricultural, Trade Hub
- **Medium Markup** (1.3): Industrial, Research
- **High Markup** (1.5-2.0): Luxury, Specialty

### Target Profit Margins
- **Beginner**: 30-50% per trade
- **Intermediate**: 50-80% per trade
- **Advanced**: 80-150% per trade

---

## ⚙️ Volatility Settings

| Level | Multiplier | Price Range | Use For |
|-------|------------|-------------|---------|
| **Stable** | 0.5 | ±20% | Basic necessities (Water, Food) |
| **Normal** | 1.0 | ±50% | Industrial goods, standard tech |
| **Volatile** | 1.5-2.0 | ±100% | Luxury items, advanced tech |
| **Extreme** | 2.5+ | ±200%+ | Rare items, contraband |

---

## 🚀 Progression Targets

### Week 1-4 (Prototype)
- **Starter Ship**: 10 cargo
- **Capital**: 1,000 → 5,000cr
- **Trades**: 3-5 routes discovered
- **Profit/Hour**: 3,000-10,000cr

### Week 5-8 (Structured)
- **Medium Ship**: 30 cargo (unlock at 50,000cr)
- **Capital**: 50,000 → 150,000cr
- **Trades**: 10+ routes available
- **Profit/Hour**: 15,000-30,000cr

### Week 9-12 (Polish)
- **Advanced Ship**: 100 cargo (unlock at 250,000cr)
- **Capital**: 250,000+ cr
- **Trades**: All routes mastered
- **Profit/Hour**: 50,000-100,000cr

---

## 🎮 Player Experience Targets

### First Trade (Minutes 0-5)
- **Investment**: 800cr (80% of starting capital)
- **Route**: Agricultural → Industrial (Water)
- **Profit**: 400cr (50% ROI)
- **Result**: Player sees profit mechanic

### First Ship Upgrade (30-60 minutes)
- **Target**: Medium ship (50,000cr)
- **Trades Needed**: ~15-20 profitable loops
- **Time**: 45-60 minutes of gameplay
- **Result**: Noticeable capacity increase

### Demo Completion (30 minutes)
- **Capital Target**: 50,000-75,000cr
- **Ship**: Upgraded to Medium
- **Routes**: 3-5 mastered
- **Satisfaction**: "I made progress!"

---

## 🔧 Quick Fixes

### "Prices too similar between markets"
```yaml
# Increase supply/demand differences
agricultural_station:
  water: {supply: 2.5, demand: 0.4}  # Very cheap

industrial_station:
  water: {supply: 0.3, demand: 2.5}  # Very expensive
```

### "Can't afford first trade"
```yaml
# Lower basic item prices OR increase starting capital
water:
  base_price: 8  # Instead of 10

# OR in game config:
starting_credits: 1500  # Instead of 1000
```

### "Cargo fills too quickly"
```yaml
# Reduce volume for common items
water:
  volume_per_unit: 0.5  # Instead of 1.0
```

---

## 📐 Economic Balance Tests

### Test 1: Starter Affordability
```python
starter_capital = 1000
cheapest_item = 10  # Water
max_purchase = starter_capital / cheapest_item  # Should be 100+
```

### Test 2: Profit Viability
```python
buy_price = 8
sell_price = 12
profit_margin = (sell_price - buy_price) / buy_price  # Should be 30-50%+
```

### Test 3: Cargo Balance
```python
cargo_capacity = 10
item_volume = 1.0
items_per_load = cargo_capacity / item_volume  # Should be 5-20
```

### Test 4: Upgrade Reachability
```python
ship_upgrade_cost = 50000
profit_per_trip = 500
trips_needed = ship_upgrade_cost / profit_per_trip  # Should be 20-40
time_per_trip = 5  # minutes
total_time = (trips_needed * time_per_trip) / 60  # Should be 1-3 hours
```

---

## 🎯 Design Principles

1. **First Trade Must Succeed**: Starter capital covers at least 1 full cargo load
2. **Clear Profit Feedback**: 30%+ margins so player sees gains
3. **Fast Iteration**: 3-5 minute trade loops
4. **Visible Progression**: Ship upgrades at 30-60 minute intervals
5. **Risk/Reward Balance**: Higher profits require higher capital

---

## 🗺️ Sample Economic Map

```
[Agricultural Station] ──(Water 8cr)──> [Industrial Station]
       ↑                                        │
       │                                        │
  (Alloys 65cr)                          (Alloys 35cr)
       │                                        │
       │                                        ↓
[Research Station] <──(Tech 450cr)── [Trade Hub]
       │                                        ↑
       │                                        │
  (Tech 700cr)                          (Various)
       │                                        │
       └──────────────────────────────────────┘
```

---

## 📞 Quick Commands

```bash
# Generate content
python3 Tools/generate_mvp_trading_content.py --all

# Validate content
python3 Tools/validate_trade_content.py

# View generated items
ls Content/DataAssets/Trading/MVP/Items/

# View generated markets
ls Content/DataAssets/Trading/MVP/Markets/
```

---

**Last Updated**: January 17, 2026  
**For**: Trade Simulator MVP (Weeks 1-12)  
**See Also**: 
- `docs/development/MVP_QUICK_START_GUIDE.md`
- `Assets/TradingTemplates/README.md`
- `.github/instructions/trade-simulator-mvp.instructions.md`
