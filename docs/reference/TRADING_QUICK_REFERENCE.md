# Trading System Quick Reference

**Quick Start**: 5-minute overview for developers starting trading implementation

---

## 🎯 What You Need to Know

### The Core Loop
```
Dock → Buy Cargo → Fly → Dock → Sell Cargo → Profit → Upgrade → Repeat
```

**Target**: 3-5 minute cycle, 30-50% profit per trade

---

## 📂 Key Files

### Data Assets (C++)
```
Source/Adastrea/Trading/
├── TradeItemDataAsset.h/.cpp    ← Individual items (Water, Food, etc.)
├── MarketDataAsset.h/.cpp       ← Station markets (inventory, prices)
├── TradeTransaction.h/.cpp      ← Transaction history tracking
├── AITraderComponent.h/.cpp     ← AI traders (DEFERRED for MVP)
└── TradeContractDataAsset.h/.cpp ← Missions (DEFERRED for MVP)
```

### Documentation
```
docs/systems/TRADING_ARCHITECTURE.md   ← Full architecture (23KB, read first)
docs/mvp/TRADING_MVP_GUIDE.md         ← Implementation guide (29KB, step-by-step)
docs/mvp/SYSTEM_CLASSIFICATION.md     ← What's MVP vs. deferred
```

---

## 🚀 Quick Start: Phase 1 (Weeks 1-4)

### Day 1: Create Trading Station Blueprint

**Location**: `Content/Blueprints/Trading/Stations/BP_SimpleTradingStation`

**Components**:
```
- StaticMesh (station model)
- BoxCollision (docking trigger, 500x500x500)
- TextRender (station name)
```

**Variables** (hardcoded for prototype):
```blueprint
StationName: "Trading Station Alpha"
WaterPrice: 8.0
FoodPrice: 30.0
ElectronicsPrice: 120.0
WaterStock: 1000
FoodStock: 500
ElectronicsStock: 100
```

**Event**:
```blueprint
OnComponentBeginOverlap (BoxCollision)
└─ If OtherActor is PlayerShip:
   └─ OpenTradingUI(this)
```

### Day 2: Create Player Ship Blueprint

**Location**: `Content/Blueprints/Trading/Ships/BP_SimplePlayerShip`

**Variables**:
```blueprint
Credits: 1000 (integer)
CargoCapacity: 10 (integer)
CargoHold: Map<String, Integer>  // "Water" → 5
```

**Functions**:
```blueprint
AddCargo(ItemName, Quantity) → bool
RemoveCargo(ItemName, Quantity) → bool
GetAvailableCargoSpace() → int
```

### Day 3-4: Create Trading UI

**Location**: `Content/Blueprints/Trading/UI/WBP_SimpleTradingUI`

**Display**:
- Player credits
- Cargo space (5/10)
- Item list with prices
- Buy/Sell buttons

**Buy Logic**:
```blueprint
OnBuyClicked(Item, Qty, Price)
├─ Cost = Qty * Price
├─ If Credits >= Cost AND CargoSpace >= Qty:
│  ├─ Credits -= Cost
│  ├─ AddCargo(Item, Qty)
│  └─ Update UI
└─ Else: Show error
```

### Day 5-7: Test & Balance

**Create Test Level**: `Content/Maps/L_TradingTest`
- Place 2 stations 10,000 units apart
- Station A: Water=8, Food=30, Electronics=120
- Station B: Water=12, Food=20, Electronics=80

**Test**:
1. Buy 10 Water @ Station A (80 credits)
2. Fly to Station B (1-2 minutes)
3. Sell 10 Water @ Station B (120 credits)
4. Profit: 40 credits (50% margin)
5. Repeat 5-10 times

**Validate**:
- [ ] Can complete loop in &lt;5 minutes
- [ ] Can see profit increase
- [ ] No crashes
- [ ] UI clear and responsive

---

## 📊 Important Numbers

### Progression
```
Starting: 1000 credits, 10 cargo
Medium Ship: 50,000 credits, 30 cargo (Week 8 goal)
Large Ship: 250,000 credits, 100 cargo (post-demo)
```

### Profit Margins
```
Target: 30-50% per trade
Example: Buy Water @ 8, Sell @ 12 = 50% profit
Avoid: 200%+ exploits (too easy)
```

### Time to Upgrade
```
Small → Medium: 30-45 minutes (demo length: 30 min)
Medium → Large: 60-90 minutes (post-MVP)
```

---

## 🎨 Data Assets (Phase 2, Week 5+)

### Create Trade Item

**Location**: `Content/DataAssets/Trading/Items/DA_TradeItem_Water`

**Parent Class**: `TradeItemDataAsset`

**MVP Properties**:
```
ItemName: "Water"
BasePrice: 10.0
VolumePerUnit: 1.0
Category: Food & Consumables
bAffectedBySupplyDemand: true
```

**Defer Properties**:
```
LegalityStatus: Legal (leave default)
TradeRestrictions: (leave empty)
AITradePriority: 5 (not used in MVP)
```

### Create Market

**Location**: `Content/DataAssets/Trading/Markets/DA_Market_AgriculturalStation`

**Parent Class**: `MarketDataAsset`

**MVP Properties**:
```
MarketName: "Agricultural Station Alpha"
MarketType: Open Market
MarketSize: Medium Station
TransactionTaxRate: 0.05 (5%)
SellPriceMarkup: 1.2 (station sells at 20% markup)
BuyPriceMarkdown: 0.8 (station buys at 20% discount)

Inventory:
  [0] TradeItem: DA_TradeItem_Water
      CurrentStock: 1000
      SupplyLevel: 1.5 (produces water, cheap)
      DemandLevel: 0.5 (doesn't need water)
```

---

## 💻 C++ Economy Manager (Phase 2, Week 6)

### Create Subsystem

**Location**: `Source/Adastrea/Trading/EconomyManager.h`

**Key Functions**:
```cpp
void UpdateEconomy(float DeltaTime);  // Called by timer
float GetItemPrice(Market, Item, bIsBuying);
void RecordTransaction(Market, Item, Qty, bPlayerBought);
void RegisterMarket(MarketDataAsset*);
```

**Price Calculation**:
```cpp
// Simple formula
float Price = Item->BasePrice 
            * (Demand / Supply) 
            * Market->SellPriceMarkup;
```

**Supply/Demand Update**:
```cpp
// When player buys
Entry.SupplyLevel *= 0.95f;  // Supply decreased
Entry.DemandLevel *= 1.05f;  // Demand increased

// Gradually recover
Entry.SupplyLevel = Lerp(Entry.SupplyLevel, 1.0f, RecoveryRate);
```

---

## ✅ Validation Gates

### Week 4: GO/NO-GO
```
Target: 60%+ say "fun"
Test: 5-10 external playtesters
Method: Silent observation, post-survey
Decision: GO to Phase 2 OR pivot/iterate
```

### Week 8: Progression Check
```
Target: 70%+ say "fun"
Test: 10-20 playtesters
Metrics: 
  - Average 10+ trades per session
  - 80%+ explore all stations
  - Clear progression path
```

### Week 12: Demo Validation
```
Target: 75%+ "fun", 50%+ "would buy"
Test: 20+ playtesters
Metrics:
  - 90%+ complete 30 minutes
  - 0 crashes
  - 60 FPS
  - &lt;30s load time
Result: Ready for funding OR iterate
```

---

## 🚫 What NOT to Build (MVP)

**Deferred Features** (don't implement until post-MVP):
- ❌ AI trader ships (background economy simulation only)
- ❌ Trade contracts/missions (trading IS the game)
- ❌ Faction diplomacy (price modifiers only)
- ❌ Market events (defer complexity)
- ❌ Contraband/legality (defer risk mechanics)
- ❌ Advanced economy (simple supply/demand only)
- ❌ Black markets (defer)
- ❌ Multiple save slots (one save only)

**Standard Response**: "Great idea for v2.0! Let's validate trading is fun first."

---

## 🔧 Common Issues

### Prices Don't Update
```
Fix: Call EconomyManager->RecordTransaction() after trade
Location: Trading UI, OnBuyClicked / OnSellClicked
```

### Cargo Capacity Not Enforced
```
Fix: Check available space before adding cargo
Code: (CurrentUsed + NewItems) <= CargoCapacity
```

### Ship Upgrade Doesn't Apply
```
Fix: Update CargoCapacity property after upgrade
Code: CargoCapacity = NewShipData->CargoCapacity
```

### Tutorial Gets Stuck
```
Fix: Check each step has clear trigger
Example: OnDocked → If TutorialStep==1 → AdvanceTutorial()
```

---

## 📚 Full Documentation

**Start Here**:
1. Read `docs/systems/TRADING_ARCHITECTURE.md` (23KB, 1 hour)
2. Read `docs/mvp/TRADING_MVP_GUIDE.md` (29KB, 1.5 hours)
3. Begin Phase 1 implementation (Week 1)

**Related**:
- `docs/mvp/SYSTEM_CLASSIFICATION.md` - MVP scope
- `CRITICAL_REVIEW_SUMMARY.md` - Why trading is priority
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP mandate

---

## 🎯 Success Mantra

> "Build the smallest, simplest trading system that's fun. Validate with players. Expand only after validation."

---

**One Question to Ask Before Adding Anything**:

*"Does this make the buy/sell/upgrade loop more fun?"*

- If YES → Consider for MVP
- If NO → Defer to post-MVP
- If UNSURE → Ask playtesters

---

**Last Updated**: 2025-12-25  
**For**: Quick developer onboarding  
**See**: Full architecture docs for complete details
