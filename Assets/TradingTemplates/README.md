# MVP Trading Templates - README

This directory contains YAML templates for rapid MVP trading content creation.

## 📁 Contents

- **MVPTradeItems.yaml** - 15 pre-configured trade items across 5 categories
- **MVPMarkets.yaml** - 5 market configurations with different economic profiles
- **README.md** - This file

## 🚀 Quick Start

### Generate All Content

```bash
cd /path/to/Adastrea
python3 Tools/generate_mvp_trading_content.py --all
```

### Output Location

Generated configurations will be in:
- Items: `Content/DataAssets/Trading/MVP/Items/`
- Markets: `Content/DataAssets/Trading/MVP/Markets/`

## 📦 Trade Items (15 Total)

### Food & Consumables (3 items)
- **Water Purified** (10cr) - Basic necessity, high volume trade
- **Protein Packs** (25cr) - Standard rations
- **Luxury Food** (100cr) - High-end market goods

### Industrial (3 items)
- **Basic Alloys** (50cr) - Construction materials
- **Refined Metals** (150cr) - Manufacturing inputs
- **Ship Components** (300cr) - Maintenance parts

### Technology (5 items)
- **Basic Computers** (500cr) - Standard tech
- **Advanced Electronics** (1,200cr) - Specialized components
- **AI Cores** (5,000cr) - Rare, high-value
- **Hydrogen Fuel** (75cr) - Common fuel
- **Antimatter Cores** (10,000cr) - Endgame power sources

### Medical (2 items)
- **Basic Medicine** (200cr) - Standard supplies
- **Advanced Medicine** (800cr) - Specialized equipment

### Luxury (2 items)
- **Entertainment Systems** (600cr) - Recreation tech
- **Rare Art** (3,000cr) - Collectibles

## 🏪 Markets (5 Configurations)

### 1. Agricultural Station
**Profile**: Food producer, tech importer
- **Cheap**: Water (80%), Protein Packs (90%), Food
- **Expensive**: Tech (150%), Components (180%)
- **Strategy**: Buy food here, sell tech

### 2. Industrial Station
**Profile**: Manufacturing hub
- **Cheap**: Alloys (70%), Metals (80%), Components (85%)
- **Expensive**: Food (130%), Supplies (150%)
- **Strategy**: Buy materials here, sell food/supplies

### 3. Research Station
**Profile**: Technology center
- **Cheap**: Computers (90%), Electronics (85%)
- **Expensive**: Materials (120%), Supplies (130%)
- **Strategy**: Buy tech here, sell materials

### 4. Luxury Bazaar
**Profile**: High-end market
- **Expensive**: Everything (200% markup)
- **Good Buyback**: Luxury items (80% of base)
- **Strategy**: Sell rare/luxury items here for best profit

### 5. Trade Hub
**Profile**: Balanced general market
- **Balanced**: All prices near base (±20%)
- **High Variety**: 10+ different items
- **Strategy**: Central trading post, compare prices

## 🎯 Profitable Trade Routes

### Beginner Route (Low Capital, Low Risk)
1. **Agricultural → Industrial**:
   - Buy: Water (8cr) × 100 = 800cr investment
   - Sell: Water (12cr) × 100 = 1,200cr return
   - **Profit**: 400cr (50% ROI)

2. **Industrial → Agricultural**:
   - Buy: Basic Alloys (35cr) × 20 = 700cr investment
   - Sell: Basic Alloys (65cr) × 20 = 1,300cr return
   - **Profit**: 600cr (86% ROI)

### Intermediate Route (Medium Capital, Medium Risk)
1. **Research → Industrial**:
   - Buy: Basic Computers (450cr) × 2 = 900cr investment
   - Sell: Basic Computers (700cr) × 2 = 1,400cr return
   - **Profit**: 500cr (56% ROI)

2. **Agricultural → Research**:
   - Buy: Protein Packs (22cr) × 30 = 660cr investment
   - Sell: Protein Packs (38cr) × 30 = 1,140cr return
   - **Profit**: 480cr (73% ROI)

### Advanced Route (High Capital, High Risk)
1. **Research → Luxury Bazaar**:
   - Buy: AI Cores (4,000cr) × 1 = 4,000cr investment
   - Sell: AI Cores (10,000cr) × 1 = 10,000cr return
   - **Profit**: 6,000cr (150% ROI)

2. **Luxury Bazaar → Trade Hub**:
   - Buy: Rare Art (2,500cr) × 1 = 2,500cr investment
   - Sell: Rare Art (4,500cr) × 1 = 4,500cr return
   - **Profit**: 2,000cr (80% ROI)

## 🔧 Customization

### Modify Existing Items

1. Edit `MVPTradeItems.yaml`
2. Change properties (price, volume, volatility, etc.)
3. Regenerate: `python3 Tools/generate_mvp_trading_content.py --items`

### Add New Items

Add to YAML:
```yaml
- item_id: my_new_item
  item_name: My New Item
  description: Item description here
  category: Technology  # Food, RawMaterials, Technology, etc.
  base_price: 500
  volume_per_unit: 5.0
  mass_per_unit: 3.0
  legality: Legal
  volatility_multiplier: 1.0
  min_price_deviation: 0.7
  max_price_deviation: 1.8
  typical_market_stock: 1000
  replenishment_rate: 100
  standard_lot_size: 10
  affected_by_supply_demand: true
  affected_by_market_events: true
  ai_trade_priority: 5
  ai_hoardable: false
  ai_arbitrage_enabled: true
```

### Modify Market Configurations

1. Edit `MVPMarkets.yaml`
2. Adjust:
   - `sell_price_markup` (player buy price multiplier)
   - `buy_price_markdown` (player sell price multiplier)
   - Inventory `supply_level` (>1.5 = abundant, <0.5 = scarce)
   - Inventory `demand_level` (>1.5 = high demand, <0.5 = low demand)
3. Regenerate: `python3 Tools/generate_mvp_trading_content.py --markets`

## 📊 Economic Balance Guidelines

### Price Ranges
- **Basic Necessities**: 10-50cr (Water, Food, Fuel)
- **Industrial Goods**: 50-300cr (Alloys, Components)
- **Technology**: 500-5,000cr (Computers, Electronics, AI)
- **Medical**: 200-1,000cr (Medicine, Equipment)
- **Luxury**: 600-10,000cr (Entertainment, Art, Rare Items)

### Volatility
- **Stable** (0.5): Basic necessities (Water, Food)
- **Normal** (1.0): Industrial goods, standard tech
- **Volatile** (1.5-2.0): Luxury items, advanced tech
- **Extreme** (2.5+): Rare items, contraband

### Supply/Demand Economics
- **Abundant Supply** (>2.0): Items this market produces
- **Normal Supply** (1.0): Balanced availability
- **Scarce Supply** (<0.5): Items this market desperately needs
- **High Demand** (>2.0): Critical needs (food for industrial, tech for agricultural)
- **Low Demand** (<0.5): Items this market has in excess

### Profitable Spreads
- **Beginner Spread**: 30-50% profit per trade
- **Intermediate Spread**: 50-80% profit per trade
- **Advanced Spread**: 80-150% profit per trade
- **Risk Factor**: Higher profit = higher capital requirement

## 🎮 Gameplay Tuning

### Starter Ship (10 Cargo Capacity)
**Recommended Starting Capital**: 1,000 credits

**First Trade Options**:
1. Water: 100 units @ 8cr = 800cr investment (fits in cargo)
2. Protein Packs: 40 units @ 22cr = 880cr investment
3. Basic Alloys: 20 units @ 35cr = 700cr investment

**Profit Target**: 300-500cr per trip (30-50% ROI)

### Mid-Tier Ship (30 Cargo Capacity)
**Capital Requirement**: ~50,000 credits

**Trade Options**:
1. Bulk commodities (Water, Alloys, Fuel)
2. Mixed cargo (diversify risk)
3. Multiple stops per run

**Profit Target**: 2,000-5,000cr per trip

### Advanced Ship (100 Cargo Capacity)
**Capital Requirement**: ~250,000 credits

**Trade Options**:
1. High-value single items (AI Cores, Antimatter)
2. Large bulk shipments
3. Multi-station circuits

**Profit Target**: 10,000-50,000cr per trip

## 🧪 Testing & Validation

### Price Balance Check
```python
# Check price spread is reasonable
min_price = base_price * min_price_deviation
max_price = base_price * max_price_deviation
spread = max_price - min_price

# Target: spread should be 50-150% of base_price for playability
```

### Cargo Balance Check
```python
# Check cargo capacity vs. item volume
cargo_capacity = 10.0  # Starter ship
item_volume = 5.0

max_items = cargo_capacity / item_volume  # Should be 2+
```

### Profit Validation
```python
# Minimum viable profit per trip
trip_time_minutes = 5
credits_per_hour = (profit_per_trip / trip_time_minutes) * 60

# Target: 3,000-10,000 credits/hour for starter ship
```

## 🐛 Common Issues

### "Prices too similar between markets"
- **Fix**: Increase supply/demand differences
- Adjust `supply_level` and `demand_level` in inventory entries
- Target: 2x price difference minimum between markets

### "Can't afford first trade"
- **Fix**: Reduce starting item prices OR increase starting capital
- Adjust `base_price` in MVPTradeItems.yaml
- OR increase StartingCredits in player config

### "Cargo fills too quickly"
- **Fix**: Reduce `volume_per_unit` for common items
- OR increase ship `CargoCapacity`
- Balance: starter ship should hold 2-10 lots

### "No clear profitable routes"
- **Fix**: Create intentional supply/demand imbalances
- Agricultural: High supply food, low supply tech
- Industrial: High supply materials, low supply food
- Research: High supply tech, low supply everything else

## 📖 Further Reading

- **MVP Quick Start Guide**: `docs/development/MVP_QUICK_START_GUIDE.md`
- **Trade Simulator MVP Instructions**: `.github/instructions/trade-simulator-mvp.instructions.md`
- **Trading System Documentation**: `Assets/TradingSystemGuide.md` (if exists)

## 🤝 Contributing

### Adding New Templates

1. Follow existing YAML structure
2. Maintain economic balance (test profit margins)
3. Document new items/markets in this README
4. Regenerate and test in-game

### Balance Adjustments

1. Test trade routes for profitability
2. Adjust prices/supply/demand as needed
3. Regenerate content
4. Playtest with 3+ loops

---

**Last Updated**: January 17, 2026  
**Version**: 1.0 (MVP Initial Release)  
**Maintainer**: Adastrea Development Team
