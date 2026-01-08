# MVP Blueprint Implementation Guide

> **Quick start guide for implementing all MVP Blueprints in order**

**Last Updated**: 2026-01-08  
**For**: Trade Simulator MVP Phase 1  
**Time Required**: 3-4 hours total

---

## 📋 Overview

This guide provides the **recommended order** for creating all MVP Blueprints. Follow this sequence to build the Trade Simulator MVP from scratch.

---

## 🎯 Implementation Order

### Prerequisites (Before Starting)

✅ **Verify you have**:
- [ ] Unreal Engine 5.6+ installed
- [ ] Adastrea project compiled (TradingSystem module)
- [ ] At least 2 Market Data Assets created
- [ ] At least 5 Trade Item Data Assets created
- [ ] Basic Unreal Engine familiarity

**If missing Data Assets**, see:
- `Assets/TradingSystemGuide.md` - Creating market and item data

---

## Step-by-Step Blueprint Creation

### Phase 1: Core Systems (90 minutes)

**1. Trading Game Mode** (35-45 min) ⭐ **START HERE**
- **Guide**: [BP_TradingGameMode_GUIDE.md](BP_TradingGameMode_GUIDE.md)
- **Why first**: Economy system is needed by all other Blueprints
- **Creates**: Dynamic pricing, supply/demand simulation
- **Test**: Economy initializes, prices update periodically

**2. Trading Ship (Player)** (40-50 min)
- **Guide**: [BP_TradingShip_GUIDE.md](BP_TradingShip_GUIDE.md)
- **Depends on**: Game Mode (for economy access)
- **Creates**: Player's flyable ship with cargo
- **Test**: Ship flies, docks, tracks cargo space

**3. Ship Cargo System** (25-35 min)
- **Guide**: [ShipCargoSystemGuide.md](ShipCargoSystemGuide.md)
- **Depends on**: Trading Ship
- **Creates**: Cargo inventory component
- **Test**: Can add/remove cargo, check capacity

---

### Phase 2: Trading Interface (100 minutes)

**4. Trading Station Actor** (30-40 min)
- **Guide**: [TradingStationActorGuide.md](TradingStationActorGuide.md)
- **Depends on**: Trading Ship
- **Creates**: Dockable stations with trading terminals
- **Test**: Ship can dock, station triggers UI

**5. Docking UI** (20-30 min)
- **Guide**: [DockingUIGuide.md](DockingUIGuide.md)
- **Depends on**: Trading Station
- **Creates**: "Press E to dock" prompts
- **Test**: Prompt appears near stations

**6. Trading UI Widget** (45-60 min)
- **Guide**: [TradingSystemBlueprintGuide_SIMPLIFIED.md](TradingSystemBlueprintGuide_SIMPLIFIED.md)
- **Depends on**: Trading Station, Ship Cargo, Game Mode
- **Creates**: Buy/sell interface with market data
- **Test**: Can buy/sell items, see prices, track credits

---

### Phase 3: Player Information (30 minutes)

**7. Market Display Widget** (25-35 min)
- **Guide**: [WBP_MarketDisplay_GUIDE.md](WBP_MarketDisplay_GUIDE.md)
- **Depends on**: Game Mode
- **Creates**: Price comparison across markets
- **Test**: Shows prices, highlights profitable trades

---

## 🧪 Integration Testing

After creating all Blueprints, test the complete flow:

### Test 1: Full Trading Loop (5 minutes)

1. **Start game** with all Blueprints in level
2. **Fly to Station A** using Trading Ship
3. **Dock** at station (E key)
4. **Open trading UI** (E key when docked)
5. **Check Market Display** - note prices
6. **Buy cheap goods** (e.g., Water at Agricultural Station)
7. **Undock** and fly to Station B
8. **Dock** at Station B (Industrial Station)
9. **Sell goods** for higher price
10. **Check profit** - credits should increase

**Success Criteria**:
- ✅ Can complete full loop without errors
- ✅ Profit calculation correct
- ✅ Cargo space updates
- ✅ Economy reacts to transactions

---

### Test 2: Economy Simulation (3 minutes)

1. **Wait 60 seconds** without trading
2. **Check Market Display** - prices should change slightly
3. **Buy large quantity** (e.g., 50 units)
4. **Wait 30 seconds** for economy update
5. **Check price** - should be higher (supply decreased)

**Success Criteria**:
- ✅ Prices fluctuate over time
- ✅ Transactions affect supply/demand
- ✅ Prices gradually return to normal

---

### Test 3: Player Progression (2 minutes)

1. **Complete 5-10 trades** to earn credits
2. **Note starting credits** (e.g., 1000)
3. **Calculate profit** from trades
4. **Verify credits match** expected total

**Success Criteria**:
- ✅ Credits accumulate correctly
- ✅ Cargo space tracked accurately
- ✅ Can afford more expensive items

---

## 🐛 Common Issues Across All Blueprints

### Issue: "Game Mode not found"

**Solution**:
1. Open **World Settings** (Window → World Settings)
2. Set **GameMode Override** to `BP_TradingGameMode`
3. Verify in Details panel under "Game Mode"

### Issue: "Data Assets not loading"

**Solution**:
1. Check Data Assets exist in Content Browser
2. Verify paths: `Content/DataAssets/Trading/`
3. Ensure module compiled successfully
4. Restart editor if needed

### Issue: "Input not working"

**Solution**:
1. Open **Project Settings** → Input
2. Verify action/axis mappings exist:
   - MoveForward, MoveRight, MoveUp (axes)
   - Interact (action, E key)
   - Turn, LookUp (axes for mouse)
3. Add if missing

### Issue: "UI not appearing"

**Solution**:
1. Check widget added to viewport in Blueprint
2. Verify `Set Input Mode UI Only` called
3. Check widget Z-order (should be on top)
4. Enable "Is Variable" on widget components

---

## 📊 Progress Checklist

Track your implementation progress:

### Core Systems
- [ ] BP_TradingGameMode created and economy working
- [ ] BP_TradingShip flying and docking
- [ ] Cargo system tracking inventory

### Trading Interface
- [ ] BP_TradingStation placed in level
- [ ] Docking UI prompts working
- [ ] Trading UI opening and functional

### Player Information
- [ ] Market Display showing prices
- [ ] Profit opportunities highlighted

### Integration Tests
- [ ] Full trading loop works end-to-end
- [ ] Economy simulation active
- [ ] Credits accumulate correctly

---

## 🎓 Learning Path

**If you're new to Unreal Blueprint**:

1. **Start with easiest**: Docking UI (20 min)
2. **Then try**: Ship Cargo System (25 min)
3. **Build up to**: Trading Ship (40 min)
4. **Advanced**: Game Mode (35 min) and Trading UI (45 min)

**If you're experienced**:
Follow the recommended order (Game Mode → Ship → Trading UI)

---

## 📚 Additional Resources

**Blueprint Guides**:
- [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- [Blueprint Standards](../docs/reference/BLUEPRINT_GUIDE_STANDARDS.md)
- [Blueprint Image Guide](../docs/reference/BLUEPRINT_IMAGE_GUIDE.md)

**System Documentation**:
- [Trading System Guide](../Assets/TradingSystemGuide.md)
- [Spaceship System Guide](../Assets/SpaceshipSystemGuide.md)
- [MVP Phase 1 Checklist](../docs/mvp/PHASE_1_CHECKLIST.md)

**UE5 Official Docs**:
- [Unreal Engine Documentation](https://docs.unrealengine.com/5.6/en-US/)
- [Blueprint Visual Scripting](https://docs.unrealengine.com/5.6/en-US/blueprints-visual-scripting-in-unreal-engine/)
- [UMG UI Designer](https://docs.unrealengine.com/5.6/en-US/umg-ui-designer-for-unreal-engine/)

---

## ⏱️ Time Estimates

**Realistic timeline for complete MVP Blueprint implementation**:

| Phase | Time | Includes |
|-------|------|----------|
| Setup & Prerequisites | 30 min | Data Assets, project setup |
| Phase 1: Core Systems | 90 min | Game Mode, Ship, Cargo |
| Phase 2: Trading Interface | 100 min | Station, Docking, Trading UI |
| Phase 3: Player Info | 30 min | Market Display |
| Integration Testing | 10 min | Full loop testing |
| **Total** | **4 hours** | Complete MVP implementation |

**Speed run** (experienced developers): 2-3 hours  
**Learning mode** (beginners): 5-6 hours

---

## 🎯 Next Steps After MVP Blueprints

Once all MVP Blueprints work:

1. **Content Creation**:
   - Add 5-10 stations to world
   - Create 15-20 trade items
   - Design 3 ship types (starter, mid, advanced)

2. **Polish**:
   - Add VFX (docking effects, purchase confirmation)
   - Add SFX (ship sounds, UI clicks, transaction sounds)
   - Improve UI visuals (colors, fonts, icons)

3. **Balance**:
   - Adjust item prices for interesting trades
   - Tune economy volatility (not too wild)
   - Set progression pace (time to upgrade ship)

4. **Testing**:
   - External playtesting (5-10 people)
   - Gather feedback on fun factor
   - Iterate based on data

---

## ❓ Getting Help

**If you get stuck**:

1. **Check troubleshooting** in individual guides
2. **Review prerequisites** - missing something?
3. **Add debug prints** - where does it fail?
4. **Check Output Log** - any errors?
5. **Ask for help** with specific details:
   - Which guide/step you're on
   - What you expected vs what happened
   - Screenshots of Blueprint/error
   - What you've tried already

---

## ✨ Congratulations!

After completing all guides, you'll have:
- ✅ Fully functional trading gameplay loop
- ✅ Dynamic economy simulation
- ✅ Flyable ship with cargo management
- ✅ Trading interface with price comparison
- ✅ Foundation for MVP Phase 1 demo

**You've built the core of a trading game!** 🚀

---

**Guide Version**: 1.0  
**Created**: 2026-01-08  
**Maintained by**: Adastrea Development Team
