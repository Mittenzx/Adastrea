# Trade Simulator MVP - Complete Development Checklist

**Purpose**: Week-by-week checklist for building the Trade Simulator MVP from scratch to playable demo.

**Timeline**: 12 weeks (Dec 2025 - Mar 2026)  
**Goal**: 30-minute playable demo with 75% "fun" rating

---

## Pre-Development Setup (Week 0)

### Project Setup
- [ ] Unreal Engine 5.6 installed
- [ ] Adastrea project cloned from repository
- [ ] Visual Studio or IDE configured
- [ ] Python 3.9+ installed for tools
- [ ] Git configured for version control

### Team Alignment
- [ ] Read `.github/instructions/trade-simulator-mvp.instructions.md`
- [ ] Read `.github/instructions/anti-patterns.instructions.md`
- [ ] Review `CRITICAL_REVIEW_SUMMARY.md`
- [ ] Understand MVP scope (trading ONLY, no combat/exploration)
- [ ] Accept 12-week timeline

### Tool Verification
- [ ] Run `python3 Tools/generate_mvp_trading_content.py --all`
- [ ] Run `python3 Tools/validate_trade_content.py`
- [ ] Run `python3 tests/test_trading_integration.py`
- [ ] All tools execute without errors

---

## Week 1-2: Reality Check & Design

### Documentation Review (Days 1-2)
- [ ] Read `docs/development/MVP_QUICK_START_GUIDE.md`
- [ ] Read `docs/reference/TRADING_ECONOMICS_CHEAT_SHEET.md`
- [ ] Read `docs/development/TRADING_SYSTEM_INTEGRATION_GUIDE.md`
- [ ] Review all Blueprint templates in `Content/Blueprints/Trading/`

### Game Design Document (Days 3-5)
- [ ] Write 2-page core loop specification
- [ ] Define success criteria (60%+ "fun" in Week 4)
- [ ] Create UI mockups (trading interface)
- [ ] Plan 2-station prototype level
- [ ] Identify Week 4 GO/NO-GO decision points

### Prototype Planning (Days 6-7)
- [ ] Choose 2 stations for prototype (Agricultural + Industrial recommended)
- [ ] Select 3 trade items for hardcoded prototype (Water, Protein, Electronics)
- [ ] Define hardcoded prices (3cr, 25cr, 500cr)
- [ ] Plan first trade route (Water: Agri 3cr → Industrial 87cr)

---

## Week 3-4: Hardcoded Prototype

**Goal**: Playable 10-minute experience with hardcoded values

### Ship Setup (Days 8-10)
- [ ] Create `BP_TradingShip_Prototype` from template
- [ ] Add `CargoComponent` (10 unit capacity)
- [ ] Add `PlayerTraderComponent` (1000cr starting credits)
- [ ] Configure flight controls (W/A/S/D, mouse look)
- [ ] Test flight between two empty points (no stations yet)

### Station Setup (Days 10-12)
- [ ] Place Agricultural Station at (1000, 0, 0)
- [ ] Place Industrial Station at (5000, 0, 0)
- [ ] Add docking collision volumes (500 unit radius)
- [ ] Implement "Press E to dock" prompt
- [ ] Test docking at both stations

### Trading UI - Hardcoded (Days 13-15)
- [ ] Create `WBP_TradingUI_Prototype` widget
- [ ] Hardcode 3 items with fixed prices
- [ ] Implement Buy button (deduct credits, add cargo)
- [ ] Implement Sell button (add credits, remove cargo)
- [ ] Display: Credits, Cargo Space, Item Prices
- [ ] Test buy/sell transactions

### First Trade Loop (Days 15-16)
- [ ] Start at Agricultural Station (docked)
- [ ] Buy 10 Water @ 3cr = 30cr
- [ ] Undock and fly to Industrial Station
- [ ] Dock at Industrial Station
- [ ] Sell 10 Water @ 87cr = 870cr
- [ ] **Verify profit: 840cr (84% ROI)**
- [ ] Repeat loop 3 times without crashes

### **Week 4 GO/NO-GO Playtest** (Day 17)
- [ ] External playtest with 5-10 people
- [ ] No instructions given (silent observation)
- [ ] Record: Completion rate, confusion points, fun rating
- [ ] **Decision**: 60%+ say "fun" → GO to Week 5-8
- [ ] **Decision**: <60% fun → Pivot or redesign

---

## Week 5-6: Structured Version - Data Assets

**Goal**: Convert prototype to Data Asset system with 15 items, 5 markets

### Trade Item Data Assets (Days 18-21)
- [ ] Follow `TRADING_SYSTEM_INTEGRATION_GUIDE.md` Phase 2.1
- [ ] Create 15 `DA_Item_*` Data Assets in Unreal
  - [ ] Water Purified (10cr)
  - [ ] Protein Packs (25cr)
  - [ ] Luxury Food (100cr)
  - [ ] Hydrogen Fuel (50cr)
  - [ ] Basic Alloys (70cr)
  - [ ] Refined Metals (150cr)
  - [ ] Ship Components (350cr)
  - [ ] Basic Computers (650cr)
  - [ ] Advanced Electronics (5500cr)
  - [ ] Basic Medicine (200cr)
  - [ ] Advanced Medicine (800cr)
  - [ ] Entertainment Systems (600cr)
  - [ ] Rare Art (3000cr)
  - [ ] AI Cores (8000cr)
  - [ ] Antimatter Cores (10000cr)
- [ ] Verify all items load without errors
- [ ] Test one item in trading UI

### Market Data Assets (Days 22-25)
- [ ] Follow `TRADING_SYSTEM_INTEGRATION_GUIDE.md` Phase 2.2
- [ ] Create 5 `DA_Market_*` Data Assets
  - [ ] Agricultural Station Market (6 items, cheap food)
  - [ ] Industrial Station Market (8 items, expensive food/cheap materials)
  - [ ] Research Station Market (7 items, expensive tech)
  - [ ] Luxury Station Market (5 items, all expensive)
  - [ ] Trade Hub Market (12 items, balanced prices)
- [ ] Configure inventory for each market (stock levels, supply/demand)
- [ ] Set markup/markdown ratios per market
- [ ] Test loading all markets

### Blueprint Refactoring (Days 26-28)
- [ ] Replace hardcoded prices with Data Asset lookups
- [ ] Implement `GetItemPrice(TradeItem, MarketData)` function
- [ ] Update Buy/Sell logic to use Data Assets
- [ ] Test all 15 items trade correctly
- [ ] Verify profit calculator matches in-game prices

---

## Week 7-8: Economic Features & AI

**Goal**: Living economy with AI traders and ship progression

### Ship Progression System (Days 29-32)
- [ ] Follow `SHIP_PROGRESSION_SYSTEM.md`
- [ ] Create `DA_Ship_CompactTrader` (10 cargo, starter)
- [ ] Create `DA_Ship_MerchantHauler` (30 cargo, 50,000cr)
- [ ] Create `DA_Ship_TradeLiner` (100 cargo, 250,000cr)
- [ ] Implement unlock system (lifetime earnings threshold)
- [ ] Create Shipyard UI (`WBP_ShipyardMenu`)
- [ ] Test ship switching and cargo transfer
- [ ] Verify progression: 15-20 min to first upgrade

### AI Trader System (Days 33-37)
- [ ] Follow `AI_TRADER_BEHAVIOR_SYSTEM.md`
- [ ] Implement `UAITraderComponent` C++ class
- [ ] Create `BP_AITrader_LocalShuttle` (40% of traders)
- [ ] Create `BP_AITrader_Merchant` (35% of traders)
- [ ] Create `BP_AITrader_Specialist` (15% of traders)
- [ ] Create `BP_AITrader_Newbie` (10% of traders)
- [ ] Configure 33 total AI traders across 5 stations
  - [ ] Agricultural: 6 traders
  - [ ] Industrial: 8 traders
  - [ ] Research: 5 traders
  - [ ] Luxury: 4 traders
  - [ ] Trade Hub: 10 traders
- [ ] Test AI pathfinding between stations
- [ ] Verify AI trades affect prices (±5%)
- [ ] Balance: Player still profits despite AI competition

### Economic Validation (Days 38-39)
- [ ] Run `python3 Tools/validate_trade_content.py`
- [ ] Verify 12+ profitable routes exist
- [ ] Run `python3 Tools/trade_profit_calculator.py --best`
- [ ] Test 5 different trade routes manually
- [ ] Check supply/demand dynamics work
- [ ] Ensure no runaway inflation/deflation

### **Week 8 Playtest** (Day 40)
- [ ] External playtest with 10-20 people
- [ ] Give brief tutorial (2 minutes)
- [ ] Let them play 10-15 minutes
- [ ] Target metrics:
  - [ ] 80%+ complete 3+ trades
  - [ ] 70%+ say "had fun"
  - [ ] <3 major confusion points
  - [ ] 50%+ "would buy" interest
- [ ] Adjust balance based on feedback

---

## Week 9-10: Dynamic Features & Polish

**Goal**: Market events, save/load, tutorial system

### Market Event System (Days 41-45)
- [ ] Follow `MARKET_EVENT_SYSTEM.md`
- [ ] Implement `FMarketEvent` struct and `UMarketEventManager`
- [ ] Create 6 event Data Assets:
  - [ ] Drought (Agricultural)
  - [ ] Equipment Breakdown (Industrial)
  - [ ] Research Delay (Research)
  - [ ] Bumper Crop (Agricultural)
  - [ ] Supply Convoy (Industrial)
  - [ ] Labor Strike (Industrial - jackpot!)
- [ ] Create `WBP_MarketEventAlert` notification widget
- [ ] Test event triggering (force-trigger for testing)
- [ ] Verify price changes match event specs (50-1000%)
- [ ] Test profitable arbitrage during events

### Save/Load System (Days 46-49)
- [ ] Follow `SAVE_LOAD_SYSTEM.md`
- [ ] Implement `UAdastreaTradeSimSaveGame` class
- [ ] Implement `SaveGame()` function (all properties)
- [ ] Implement `LoadGame()` function (restore state)
- [ ] Add auto-save triggers:
  - [ ] After trade completion
  - [ ] After ship upgrade
  - [ ] After docking at new station
  - [ ] Every 5 minutes (background timer)
- [ ] Create `WBP_MainMenu` with Continue/New Game buttons
- [ ] Add Save button to pause menu
- [ ] Test save/load cycle 10+ times
- [ ] Test corrupted save recovery

### Tutorial System (Days 50-52)
- [ ] Follow `TUTORIAL_SYSTEM.md`
- [ ] Create `BP_TutorialManager` actor
- [ ] Implement 9 tutorial steps:
  - [ ] Welcome screen
  - [ ] Flight basics (60s)
  - [ ] Docking (30s)
  - [ ] Market overview (45s)
  - [ ] First purchase (60s)
  - [ ] Undock (20s)
  - [ ] Navigate to Industrial (90s)
  - [ ] Dock and sell (60s)
  - [ ] Tutorial complete (30s)
- [ ] Create tutorial UI widgets (`WBP_TutorialPrompt`, arrows)
- [ ] Implement skip tutorial option
- [ ] Test tutorial with fresh player (no prior knowledge)
- [ ] Verify tutorial completes in 5-7 minutes

---

## Week 11: Testing & Bug Fixing

**Goal**: Eliminate critical bugs, optimize performance

### Performance Optimization (Days 53-55)
- [ ] Run `python3 Tools/trading_performance_profiler.py --analyze`
- [ ] Target: Overall Grade A+ (all metrics <1ms)
- [ ] Profile with 10x stress test
- [ ] Optimize any bottlenecks found
- [ ] Test with 100+ AI traders (stress test)
- [ ] Ensure 60 FPS on mid-range PC

### Integration Testing (Days 55-57)
- [ ] Run full test suite: `python3 tests/test_trading_integration.py`
- [ ] Verify all 11 tests pass
- [ ] Test every trade route manually (12+ routes)
- [ ] Test ship progression (10 cargo → 30 → 100)
- [ ] Test all 6 market events
- [ ] Test save/load at various points
- [ ] Test tutorial start-to-finish

### Bug Fixing (Days 57-60)
- [ ] Fix all critical bugs (crashes, progression blockers)
- [ ] Fix high-priority bugs (UI issues, balance problems)
- [ ] Test fixes thoroughly
- [ ] Re-run integration tests
- [ ] Document known minor bugs (low priority)

---

## Week 12: Demo Polish & Validation

**Goal**: 30-minute polished demo ready for external validation

### Content Finalization (Days 61-63)
- [ ] Verify all 15 trade items functional
- [ ] Verify all 5 markets configured correctly
- [ ] Verify all 3 ships accessible
- [ ] Test 5-10 profitable trade routes
- [ ] Balance check: Can reach 50,000cr in 15-20 min?
- [ ] Balance check: Can reach 250,000cr in 45-60 min total?

### UI/UX Polish (Days 64-66)
- [ ] Add VFX for:
  - [ ] Purchase confirmation (particles, sound)
  - [ ] Sell confirmation
  - [ ] Ship upgrade (celebration)
  - [ ] Profit display (floating green numbers)
- [ ] Add sound effects:
  - [ ] Market ambient sounds
  - [ ] Docking sounds
  - [ ] Transaction sounds
  - [ ] UI button clicks
- [ ] Polish tutorial UI (animations, clarity)
- [ ] Polish market event notifications
- [ ] Test all UI elements for clarity

### 30-Minute Demo Build (Days 67-68)
- [ ] Create standalone build
- [ ] Test demo start-to-finish (30 minutes)
- [ ] Verify tutorial → trading → progression works
- [ ] Check: Can player reach Merchant Hauler in 30 min?
- [ ] Test on 3 different PCs (low, mid, high spec)
- [ ] Ensure <30 second load time
- [ ] No crashes in 30-minute session

### **Week 12 Final Validation** (Days 69-70)
- [ ] External playtest with 20+ people
- [ ] Full 30-minute demo
- [ ] Minimal instructions (tutorial teaches everything)
- [ ] Capture metrics:
  - [ ] **90%+ complete 30 minutes**
  - [ ] **75%+ say "I had fun"**
  - [ ] **60%+ say "I would play again"**
  - [ ] **50%+ say "I would buy this"**
  - [ ] 0 crashes
  - [ ] <5 known bugs
  - [ ] 60 FPS average
- [ ] **Decision**: Metrics met → Ready for publisher pitch
- [ ] **Decision**: Metrics NOT met → Additional polish iteration

---

## Post-Week 12: Publisher Pitch Preparation

### Demo Package
- [ ] 30-minute standalone build (Windows)
- [ ] Gameplay trailer (2-3 minutes)
- [ ] Pitch deck (10-15 slides)
- [ ] One-pager (game overview)
- [ ] Development roadmap (post-MVP)

### Pitch Materials
- [ ] Core loop demonstration video
- [ ] Player testimonials from playtests
- [ ] Metrics summary (75% fun, 50% buy intent)
- [ ] Budget estimate for full development
- [ ] Timeline to Early Access

---

## Success Criteria Summary

### Week 4 GO/NO-GO Gate
- **60%+** playtesters say "fun" after 10 minutes
- Core loop is clear and understandable
- No critical bugs in hardcoded prototype

### Week 8 Milestone
- **70%+** had fun in 10-15 minute session
- **80%+** completed 3+ trades successfully
- **50%+** interested in buying

### Week 12 Final Validation
- **75%+** fun rating after 30 minutes
- **60%+** would play again
- **50%+** would purchase
- **90%+** completion rate
- 0 crashes
- 60 FPS performance

---

## Risk Mitigation

### High-Risk Items
1. **Week 4 GO/NO-GO fails** (<60% fun)
   - Mitigation: Redesign core loop, extend prototype phase
   - Fallback: Pivot to different gameplay hook

2. **Economic balance issues** (runaway prices, no profits)
   - Mitigation: Use profit calculator tool extensively
   - Fallback: Simplify economy, remove supply/demand

3. **Performance problems** (<60 FPS)
   - Mitigation: Profile early, optimize continuously
   - Fallback: Reduce AI trader count, simplify visuals

4. **Save system fails** (corrupted saves, data loss)
   - Mitigation: Extensive testing, backup system
   - Fallback: Session-only (no saves) for demo

---

## Daily Development Routine

### Every Day
- [ ] Compile project (check for errors)
- [ ] Run game for 5 minutes (manual playtest)
- [ ] Fix one bug or add one feature
- [ ] Commit changes to version control
- [ ] Document progress in CHANGELOG

### Every Week
- [ ] Team meeting (progress review)
- [ ] Playtest session (internal or external)
- [ ] Update checklist (mark completed items)
- [ ] Adjust next week's plan based on feedback

---

## Tools Quick Reference

### Content Generation
```bash
# Generate all MVP content
python3 Tools/generate_mvp_trading_content.py --all

# Validate economic balance
python3 Tools/validate_trade_content.py

# Calculate trade profits
python3 Tools/trade_profit_calculator.py --best --cargo 10

# Build test scenario
python3 Tools/mvp_scenario_builder.py --scenario first_trade
```

### Testing
```bash
# Run integration tests
python3 tests/test_trading_integration.py

# Profile performance
python3 Tools/trading_performance_profiler.py --analyze
```

---

## Checklist Progress Tracking

**Week 1-2**: ☐ In Progress / ☑ Complete  
**Week 3-4**: ☐ In Progress / ☑ Complete  
**Week 5-6**: ☐ In Progress / ☑ Complete  
**Week 7-8**: ☐ In Progress / ☑ Complete  
**Week 9-10**: ☐ In Progress / ☑ Complete  
**Week 11**: ☐ In Progress / ☑ Complete  
**Week 12**: ☐ In Progress / ☑ Complete  

**Overall MVP Status**: ☐ Not Started / ☐ In Progress / ☑ Complete

---

## Summary

**Total Tasks**: 200+ checklist items  
**Timeline**: 12 weeks (70 working days)  
**Milestones**: 3 major (Week 4, 8, 12)  
**Success Metrics**: 75% fun, 60% replay, 50% buy intent  
**Deliverable**: 30-minute playable demo

**"One polished gameplay loop beats ten half-finished systems."**

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Status**: Complete development roadmap for Trade Simulator MVP

This checklist is your roadmap from zero to fundable demo. Follow it, test relentlessly, and ship a great trading game!
