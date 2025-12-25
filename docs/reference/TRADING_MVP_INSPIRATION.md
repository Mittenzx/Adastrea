# Trading MVP Inspiration - Freelancer Simple Model

**Purpose**: Quick reference for Trade Simulator MVP developers (Week 1-12 ONLY)  
**Last Updated**: 2025-12-25  
**Phase**: Trade Simulator MVP Development

---

## ⚠️ CRITICAL: MVP SCOPE ONLY

This document extracts the **simplest, most relevant** trading concepts from game research for the 12-week MVP.

**DO NOT read the full research document** (`TRADING_RESEARCH_GAMES_ANALYSIS.md`) until:
1. Week 4 playtest validates core loop is fun (60%+ "fun" rating)
2. GO decision is made to continue to Phase 2
3. You need post-MVP feature planning

**This extract is all you need for MVP development.**

---

## Core Concept: Buy Low, Sell High

### The Entire MVP in 5 Lines

```
Player starts at Station A
Station A sells Water for 10 credits
Player buys Water, fills cargo hold
Player flies to Station B (1 minute)
Station B buys Water for 15 credits
Player sells Water, makes profit
Player uses profit to upgrade cargo hold
Repeat with bigger margins
```

**Success Metric**: 60%+ of playtesters say "this is fun" after 10 minutes.

---

## Inspiration Source: Freelancer

Freelancer (2003) is the **perfect model** for the MVP because:

✅ **Simple to understand**: Buy low, sell high  
✅ **Quick feedback loop**: 3-5 minutes per trade run  
✅ **Clear progression**: Bigger cargo = bigger profits  
✅ **Regional variety**: Different areas have different prices  
✅ **Risk/reward balance**: Safe routes vs. dangerous routes  
✅ **Accessible**: Players understand in 30 seconds  

**Complexity Rating**: ⭐⭐⭐ (3/5) - Perfect for MVP

---

## Phase 1: Hardcoded Prototype (Week 1-4)

### What to Build

**Minimum Viable Trade Loop**:

1. **1 Ship**: 
   - 10 cargo space
   - Basic flight controls
   - Docking ability

2. **2 Stations**:
   - Station A (Agricultural)
   - Station B (Industrial)
   - Visual distinction
   - 1 minute flight apart

3. **3 Trade Goods**:
   - Water: Station A = 10 credits, Station B = 15 credits
   - Food: Station A = 25 credits, Station B = 40 credits
   - Fuel: Station A = 50 credits, Station B = 65 credits

4. **Basic UI**:
   - Show prices at current station
   - Buy button (deducts credits, adds cargo)
   - Sell button (adds credits, removes cargo)
   - Profit counter

5. **Profit Tracking**:
   - Start with 100 credits
   - Track total earnings
   - Show profit per run

**Success Criteria**:
- [ ] Can fly between stations in 1 minute
- [ ] Can buy cargo at Station A
- [ ] Can sell cargo at Station B
- [ ] Can see profit increase visibly
- [ ] Runs for 10 minutes without crashes
- [ ] 60%+ playtesters say "this is fun"

### What to Hardcode (It's OK!)

```cpp
// Week 1-4: Hardcode everything
TMap<FName, float> Prices_StationA;
Prices_StationA.Add("Water", 10.0f);
Prices_StationA.Add("Food", 25.0f);
Prices_StationA.Add("Fuel", 50.0f);

TMap<FName, float> Prices_StationB;
Prices_StationB.Add("Water", 15.0f);
Prices_StationB.Add("Food", 40.0f);
Prices_StationB.Add("Fuel", 65.0f);

int32 CargoCapacity = 10;
float PlayerCredits = 100.0f;
```

**Don't worry about**:
- ❌ Data Assets
- ❌ Economy simulation
- ❌ Supply/demand
- ❌ Price changes
- ❌ Save/load
- ❌ Anything else

**Just validate the core loop is fun.**

---

## Phase 2: Structured Version (Week 5-8) - IF GO DECISION

**ONLY proceed if Week 4 playtest shows 60%+ "fun" rating.**

### What to Build (If GO)

1. **Regional Specialization** (Freelancer Concept):

```yaml
# 5 Station Types with Economic Identity

Agricultural Station:
  produces: [Water, Food, Organic_Materials]
  imports: [Technology, Machinery, Fuel]
  price_modifiers:
    Water: 0.8x  # They produce it, cheap here
    Food: 0.9x
    Technology: 1.3x  # They need it, expensive here
    
Industrial Station:
  produces: [Metals, Components, Machinery]
  imports: [Water, Food, Fuel]
  price_modifiers:
    Metals: 0.7x
    Components: 0.8x
    Water: 1.2x
    Food: 1.3x
    
Luxury Station:
  produces: [Art, Entertainment, Luxury_Goods]
  imports: [All_Basic_Goods]
  price_modifiers:
    Luxury_Goods: 0.9x
    All_Basic: 1.4x  # Everything expensive
    
Research Station:
  produces: [Technology, Medicine, Data]
  imports: [Components, Rare_Materials]
  price_modifiers:
    Technology: 0.8x
    Medicine: 0.85x
    Components: 1.3x
    
Trade Hub:
  produces: Nothing
  imports: Everything
  price_modifiers:
    All: 1.0x  # Balanced prices
```

2. **Simple Supply/Demand**:

```cpp
// Price changes based on player actions
float GetPrice(FName ItemID, FName StationID, bool bBuying) {
    float BasePrice = GetBasePrice(ItemID, StationID);
    float SupplyFactor = GetSimpleSupplyFactor(ItemID, StationID);
    
    if (bBuying) {
        return BasePrice * SupplyFactor; // Higher when low stock
    } else {
        return BasePrice / SupplyFactor; // Lower when high stock
    }
}

float GetSimpleSupplyFactor(FName ItemID, FName StationID) {
    int32 CurrentStock = GetStockLevel(ItemID, StationID);
    int32 TargetStock = 100; // Balanced inventory
    
    // Range: 0.5x to 1.5x
    return 0.5f + (TargetStock - CurrentStock) / 200.0f;
}
```

3. **Trade Routes**:

```
Route 1 (Beginner - Safe):
Agricultural Station → Industrial Station
Water (8 → 12 credits) = 4 credit profit/unit
Food (23 → 36 credits) = 13 credit profit/unit
10 cargo = 130 credits profit per run

Route 2 (Intermediate):
Research Station → Luxury Station
Technology (160 → 240 credits) = 80 credit profit/unit
10 cargo = 800 credits profit per run

Route 3 (Advanced - Loop):
Agricultural → Industrial → Research → Luxury → Agricultural
Multi-stop optimization
Higher total profit but more time
```

4. **Ship Progression**:

```yaml
Starter Ship:
  cargo: 10 units
  speed: Medium
  cost: Starting ship

Merchant Hauler:
  cargo: 30 units
  speed: Slow
  cost: 50,000 credits
  unlock: After 20 trades

Trade Liner:
  cargo: 100 units
  speed: Fast
  cost: 250,000 credits
  unlock: After 100 trades
```

### What to Convert to Data Assets (If GO)

- `UTradeItemDataAsset`: 20 trade goods (4 per category)
- `UMarketDataAsset`: 5-10 stations with regional identities
- `USpaceshipDataAsset`: 3 ships with clear progression
- Simple economy config (supply/demand parameters)

### What Still NOT to Build (If GO)

- ❌ Production chains
- ❌ Automation / AI traders
- ❌ Fleet management
- ❌ Station building
- ❌ Complex manufacturing
- ❌ Market manipulation
- ❌ Contracts / missions

---

## Phase 3: Polish & Demo (Week 9-12)

**Goal**: 30-minute polished demo for publisher pitch.

### What to Polish

1. **Visual Feedback**:
   - Profit numbers float up when selling
   - Cargo visually fills/empties
   - Price changes highlighted (green = profit opportunity)
   - Ship upgrade celebration moment

2. **Tutorial**:
   - 2-minute guided first trade
   - "Buy this water, it's cheap"
   - "Fly here (waypoint)"
   - "Sell it for profit"
   - Let player explore from there

3. **Content**:
   - 10 stations (2 of each type)
   - 20 goods (clear categories)
   - 5-10 viable trade routes
   - 3 ships with progression

4. **Progression Arc**:
   - Start: 100 credits, 10 cargo
   - 10 minutes: 5,000 credits, exploring routes
   - 20 minutes: 30,000 credits, considering upgrade
   - 30 minutes: Upgraded to Merchant Hauler, bigger profits

5. **Success Metrics**:
   - [ ] 0 crashes in 30-minute session
   - [ ] 60 FPS on mid-range PC
   - [ ] 75%+ playtesters say "had fun"
   - [ ] 50%+ playtesters say "would buy"

### What Still NOT to Build

- ❌ Save/load (session only is fine for demo)
- ❌ Combat system
- ❌ Exploration mechanics
- ❌ Story/narrative
- ❌ Multiplayer
- ❌ Anything not in MVP scope

---

## Key Design Principles (From Freelancer)

### 1. Simplicity & Clarity

**Good**:
```
Water costs 10 here, 15 there.
Buy here, sell there, make 5 credits profit.
```

**Bad**:
```
Dynamic pricing based on supply factor, demand factor, 
consumption rate, production rate, faction modifiers,
event multipliers, and player reputation...
```

### 2. Regional Identity

Each station type should feel distinct:
- Agricultural: Farms, green lighting, organic goods
- Industrial: Factories, metal, machinery
- Luxury: Fancy, expensive, entertainment
- Research: High-tech, medicine, data
- Trade Hub: Busy, balanced, central location

### 3. Risk vs. Reward

**MVP Version** (Week 9-12):
- Safe routes: Low profit per unit, short distance
- Risky routes: High profit per unit, long distance or dangerous path

**Post-MVP** (Version 1.0):
- Add illegal goods
- Add police scans
- Add pirate encounters

### 4. Clear Progression

```
10 cargo → make 100 credits/run → 20 runs → 2,000 credits
30 cargo → make 300 credits/run → 20 runs → 6,000 credits
100 cargo → make 1,000 credits/run → 20 runs → 20,000 credits
```

Player sees the power curve and wants to reach next tier.

### 5. Discovery-Driven

Players should discover:
- "Oh, Research Station pays well for Components!"
- "I can loop Agricultural → Industrial → Research → back!"
- "If I time it right, I can make even more profit!"

**Don't tell them everything. Let them explore.**

---

## What Makes Trading Fun

### The Core Fun

1. **Optimization Puzzle**: Finding best route/timing
2. **Clear Progress**: Seeing credits go up
3. **Skill Expression**: Better routes = more profit
4. **Power Fantasy**: Start small, end big
5. **Relaxing Loop**: Zen-like repeated trading

### The Feedback Loop

```
See price difference
↓
Calculate potential profit
↓
Make trade decision
↓
Execute trade
↓
See profit increase
↓
Feel smart/accomplished
↓
Want to do it again better
```

**This loop must complete in 3-5 minutes or it's not fun.**

### The Progression Fantasy

```
Day 1: "I'm a small-time trader with 100 credits"
Day 2: "I found a good Water route, made 1,000 credits"
Day 3: "I upgraded to Merchant Hauler, making 5,000/run"
Day 4: "I'm running complex loops, 20,000 credits in the bank"
Day 5: "I'm a trading tycoon with the best ship"
```

**Players need to feel this progression in 30 minutes (demo) or 2 hours (full game).**

---

## Common Pitfalls to Avoid

### ❌ Don't Add These (Yet)

1. **Complex Economy Simulation**: 
   - "But what if we simulate consumption rates?"
   - NO. Simple price lookup table for Week 1-4.

2. **Production Chains**:
   - "But what if stations produce goods from raw materials?"
   - NO. Regional price differences only.

3. **AI Traders**:
   - "But what if NPC ships trade and affect prices?"
   - NO. Player-only trading for MVP.

4. **Station Management**:
   - "But what if players can own stations?"
   - NO. Player is a trader, not a station owner.

5. **Fleet Automation**:
   - "But what if players can hire NPC traders?"
   - NO. Solo trading only for MVP.

### ✅ Do Add These (If Time in Week 9-12)

1. **Visual Polish**:
   - Floating profit numbers
   - Cargo animations
   - Price highlight effects

2. **Sound Effects**:
   - Purchase confirmation sound
   - Profit "cha-ching" sound
   - Ship upgrade fanfare

3. **Tutorial**:
   - 2-minute guided first trade
   - Clear objectives

4. **Quality of Life**:
   - Sort items by profit potential
   - Show distance to stations
   - Quick-buy/sell buttons

---

## Success Metrics Recap

### Week 4 (GO/NO-GO)
- [ ] Prototype playable start-to-finish
- [ ] 60%+ playtesters say "fun"
- [ ] Core loop clear to players
- [ ] No critical bugs
- [ ] **IF NOT MET: PIVOT OR STOP**

### Week 8 (Phase 2 Validation)
- [ ] 5-10 stations with regional economies
- [ ] 15-20 goods with variety
- [ ] 70%+ playtesters say "fun"
- [ ] Ship progression working
- [ ] <10 known bugs

### Week 12 (Demo Ready)
- [ ] 30-minute polished demo
- [ ] 75%+ "fun" rating
- [ ] 50%+ "would buy" interest
- [ ] 0 crashes
- [ ] 60 FPS
- [ ] Ready to pitch to publishers

---

## Quick Reference: What to Build When

### Week 1-4 (Prototype)
```cpp
// Just this
float Prices[2][3] = {
    {10, 25, 50},  // Station A
    {15, 40, 65}   // Station B
};
```

### Week 5-8 (Structured) - IF GO
```cpp
// Convert to
UTradeItemDataAsset* Items[20];
UMarketDataAsset* Markets[10];
UEconomyManager* Economy; // Simple supply/demand
```

### Week 9-12 (Polish)
```cpp
// Add
WBP_TradingUI->AddVisualFeedback();
TutorialManager->ShowGuidedTrade();
ProfitCounter->AnimateIncrease();
```

---

## Related Documents

**MVP Development**:
- `.github/instructions/trade-simulator-mvp.instructions.md` - Full MVP scope and timeline
- `docs/mvp/TRADING_MVP_GUIDE.md` - Detailed implementation guide
- `docs/reference/TRADING_MVP_QUICK_REFERENCE.md` - Quick developer reference

**Anti-Patterns (MUST READ)**:
- `.github/instructions/anti-patterns.instructions.md` - Mistakes to avoid
- `CRITICAL_REVIEW_SUMMARY.md` - Why we're doing MVP approach

**Post-MVP Only**:
- `docs/reference/TRADING_RESEARCH_GAMES_ANALYSIS.md` - Full game analysis (read AFTER Week 4 GO)
- `docs/reference/TRADING_RESEARCH_REVIEW.md` - Review of research document
- `docs/systems/TRADING_ARCHITECTURE.md` - Full system architecture (post-MVP)

---

## Remember

**One polished gameplay loop beats ten half-finished systems.**

Focus on making "buy low, sell high" fun in 4 weeks.  
Everything else comes after validation.

Build it. Ship it. Validate it. Then iterate.

---

**Last Updated**: 2025-12-25  
**Phase**: Trade Simulator MVP (Week 1-12)  
**Status**: Ready for Week 1 implementation
