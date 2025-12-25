# Trading MVP Quick Reference

**Quick answers for common development questions**

---

## 🎯 Core Components

### C++ Classes

| Component | Purpose | Key Functions |
|-----------|---------|---------------|
| `UEconomyManager` | Global economy simulation | `RegisterMarket()`, `GetItemPrice()`, `RecordTransaction()` |
| `UCargoComponent` | Ship cargo management | `AddCargo()`, `RemoveCargo()`, `GetAvailableCargoSpace()` |
| `UPlayerTraderComponent` | Player trading operations | `BuyItem()`, `SellItem()`, `GetCredits()`, `GetProfit()` |

### Data Assets

| Asset Type | Purpose | Create From |
|------------|---------|-------------|
| `UTradeItemDataAsset` | Trade good definition | TradeItemTemplates.yaml |
| `UMarketDataAsset` | Station market config | MarketTemplates.yaml |

---

## 📦 Getting Started (5 Minutes)

### 1. Get Economy Manager
```cpp
// C++
UEconomyManager* EconomyMgr = GameInstance->GetSubsystem<UEconomyManager>();
```
```blueprint
// Blueprint
Get Game Instance → Get Subsystem (EconomyManager)
```

### 2. Register Market
```cpp
// C++
EconomyMgr->RegisterMarket(MarketDataAsset);
```
```blueprint
// Blueprint
Economy Manager → Register Market (Market Data)
```

### 3. Get Item Price
```cpp
// C++
float Price = EconomyMgr->GetItemPrice(Market, Item, bIsBuying);
```
```blueprint
// Blueprint
Economy Manager → Get Item Price (Market, Item, Is Buying)
```

### 4. Buy Item
```cpp
// C++
PlayerTrader->BuyItem(Market, Item, Quantity, CargoComponent);
```
```blueprint
// Blueprint
Player Trader Component → Buy Item (Market, Item, Quantity, Cargo)
```

---

## 🏗️ Blueprint Setup

### Trading Station
```
Components:
├─ StaticMesh (station model)
├─ BoxCollision (docking trigger, size: 500x500x500)
└─ TextRender (station name)

Variables:
├─ MarketData (MarketDataAsset reference)
└─ StationName (Text)

Logic:
- BeginPlay: Register market with EconomyManager
- OnOverlapBegin: Open trading UI
- OnOverlapEnd: Close trading UI
```

### Player Ship
```
Components:
├─ StaticMesh (ship model)
├─ FloatingPawnMovement
├─ CargoComponent (capacity: 10)
└─ PlayerTraderComponent (credits: 1000)

Logic:
- Input: WASD movement
- HUD: Display credits, cargo, profit
```

---

## 💰 Price Calculation

### Formula
```
FinalPrice = BasePrice * (Demand / Supply) * Markup
```

### Supply/Demand Levels
- **0.1-0.5**: Low (high prices, station needs this)
- **0.5-1.5**: Normal (balanced prices)
- **1.5-3.0**: High (low prices, station produces this)

### Example
```
Water @ Agricultural Station:
- Base Price: 10
- Supply: 1.5 (produces water)
- Demand: 0.5 (doesn't need water)
- Markup: 1.2 (player buying)
→ Price = 10 * (0.5 / 1.5) * 1.2 = 4.0 credits

Water @ Industrial Station:
- Base Price: 10
- Supply: 0.5 (doesn't produce water)
- Demand: 1.5 (needs water)
- Markdown: 0.8 (player selling)
→ Price = 10 * (1.5 / 0.5) * 0.8 = 24.0 credits

Profit: 24 - 4 = 20 credits per unit (400% margin!)
```

---

## 🧪 Testing Checklist

### Phase 1 (Week 1-4)
- [ ] Ship flies with WASD
- [ ] Can dock at station (overlap trigger)
- [ ] Trading UI opens on dock
- [ ] Can buy item (credits decrease, cargo increases)
- [ ] Can fly to second station
- [ ] Can sell item (credits increase, cargo decreases)
- [ ] Profit calculation correct
- [ ] Complete loop in 3-5 minutes

### Success Metric
- 60%+ playtesters say "this is fun"

---

## 🔧 Common Issues

### "Trading UI doesn't open"
**Fix**: Check `DockingTrigger` collision:
- Preset: "Overlap All Dynamic"
- Generate Overlap Events: ✓

### "Can't buy items - insufficient funds"
**Fix**: Verify initial credits:
- PlayerTraderComponent → Credits: 1000

### "Prices are zero"
**Fix**: Ensure market registered:
- Station BeginPlay → RegisterMarket()

### "Cargo space not enforced"
**Fix**: Check cargo capacity:
- CargoComponent → CargoCapacity: 10

---

## 📊 Content Balance

### Item Tiers
| Tier | Price Range | Volume | Example |
|------|-------------|--------|---------|
| Basic | 10-50 | 1-2 | Water, Food |
| Standard | 50-500 | 2-5 | Computers, Medicine |
| Advanced | 500-5000 | 3-5 | AI Cores, Tech |
| Luxury | 5000+ | 1-3 | Artifacts, Art |

### Market Types
| Type | Tax | Markup | Markdown | Example |
|------|-----|--------|----------|---------|
| Open Market | 5% | 1.2x | 0.8x | Agricultural |
| Industrial | 3% | 1.15x | 0.85x | Manufacturing |
| Luxury | 10% | 1.5x | 0.6x | High-end |
| Trade Hub | 2% | 1.1x | 0.9x | Balanced |

### Profitable Routes
```
Ag Station (Water: 8 CR) → Industrial (Water: 12 CR) = 50% profit
Industrial (Computers: 425 CR) → Ag Station (Computers: 600 CR) = 41% profit
Mining (Ore: 48 CR) → Industrial (Ore: 160 CR) = 233% profit
```

---

## 🎮 Gameplay Targets

### Time Metrics
- **Single trade**: 3-5 minutes
- **10 trades**: 30-50 minutes
- **Ship upgrade**: 30-45 minutes
- **Demo length**: 30 minutes

### Progression
```
Start: 1000 credits, 10 cargo
→ After 30 min: 50,000 credits (unlock medium ship)
→ After 60 min: 250,000 credits (unlock large ship)
```

### Profit Rates
- **Small ship (10 cargo)**: 100-200 credits/minute
- **Medium ship (30 cargo)**: 300-500 credits/minute
- **Large ship (100 cargo)**: 1000+ credits/minute

---

## 🚀 Phase Checklist

### Phase 1: Prototype (Weeks 1-4)
- [x] C++ components created
- [ ] Data Assets created
- [ ] Blueprints implemented
- [ ] Test level created
- [ ] Playable prototype
- [ ] Week 4 playtest

### Phase 2: Structured (Weeks 5-8)
- [ ] 10+ items created
- [ ] 5+ stations placed
- [ ] Economy simulation working
- [ ] Ship upgrades implemented
- [ ] Balance testing

### Phase 3: Polish (Weeks 9-12)
- [ ] Content expansion (20 items, 10 stations)
- [ ] VFX and sound
- [ ] Tutorial system
- [ ] 30-minute demo package
- [ ] Week 12 validation

---

## 📖 Documentation

### For Blueprint Developers
- `docs/mvp/BLUEPRINT_IMPLEMENTATION_GUIDE.md` - Step-by-step Blueprint setup

### For Content Creators
- `Content/DataAssets/Trading/TradeItemTemplates.yaml` - Item examples
- `Content/DataAssets/Trading/MarketTemplates.yaml` - Market examples

### For Programmers
- `Source/Adastrea/Trading/EconomyManager.h` - Economy system
- `Source/Adastrea/Trading/CargoComponent.h` - Cargo management
- `Source/Adastrea/Trading/PlayerTraderComponent.h` - Trading operations

### For Project Leads
- `docs/mvp/TRADING_MVP_GUIDE.md` - Complete 12-week plan
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP mandate

---

## 💡 Quick Tips

1. **Start Simple**: 2 stations, 3 items, hardcoded first
2. **Test Early**: Playtest Week 4 (GO/NO-GO decision)
3. **One System**: Don't add combat/quests/crew until MVP validates
4. **Balance Prices**: 30-50% profit margins feel good
5. **Fast Feedback**: 3-5 minute trade loops keep players engaged

---

**Remember**: Build the simplest thing that's fun. Polish comes later.

---

**Last Updated**: 2025-12-25  
**Version**: 1.0  
**Status**: Active Reference
