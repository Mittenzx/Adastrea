# Trade Simulator MVP - Development Instructions

## 🎯 Mission Statement

Build a **playable, fun trading game in 12 weeks** that validates market interest and provides foundation for funding.

**Core Experience**: Buy low, sell high, upgrade ship, repeat with better margins.

**Success Metric**: 60% of playtesters say "this is fun" after 10 minutes.

---

## 📅 CURRENT PHASE

**Status**: Trade Simulator MVP Development (Dec 2025 - Mar 2026)  
**Priority**: #1 - All other features are deprioritized  
**Goal**: Playable 30-minute demo by Week 12

---

## 🎮 MVP SCOPE (What We're Building)

### Core Gameplay Loop

```
1. Dock at station A
2. View market prices
3. Buy cargo (low price)
4. Undock and fly to station B
5. Dock at station B
6. Sell cargo (high price)
7. Calculate profit
8. Upgrade ship with profits
9. Repeat with better efficiency
```

**Time to Complete Loop**: 3-5 minutes  
**Player Progression**: Larger cargo hold → more profit per run → better ship

---

## 📦 MINIMUM VIABLE CONTENT

### Phase 1: Hardcoded Prototype (Weeks 1-4)

**What to Build:**
- 1 flyable ship (hardcoded stats)
- 2 stations (visual distinction)
- 3 trade goods (different prices)
- Basic trading UI (buy/sell interface)
- Profit counter (track earnings)
- No save/load (session only)

**What to Hardcode:**
```cpp
// It's OK to hardcode for prototype
float GoodPrice_Station1 = 100.0f;
float GoodPrice_Station2 = 150.0f;
int32 CargoSpace = 10;
```

**Success Criteria:**
- Can fly between stations in 1 minute
- Can buy cargo at Station A
- Can sell cargo at Station B
- Can see profit increase
- Runs for 10 minutes without crashes

---

### Phase 2: Structured Version (Weeks 5-8)

**Convert to Data Assets:**
- `UTradeItemDataAsset` (10-20 goods)
- `UMarketDataAsset` (5-10 stations)
- `USpaceshipDataAsset` (3 ships: small, medium, large)
- Economy config (supply/demand simulation)

**Add Blueprints:**
- `BP_TradingStation` (docking + UI) - Location: `Content/Blueprints/Trading/Stations/`
- `BP_TradingShip` (cargo management) - Location: `Content/Blueprints/Trading/Ships/`
- `BP_TradingGameMode` (economy updates) - Location: `Content/Blueprints/GameModes/`
- `WBP_TradingUI` (market interface) - Location: `Content/UI/Trading/`

**Note**: See `docs/reference/FILE_STRUCTURE_STANDARD.md` for complete file organization rules.

**Add Features:**
- Ship upgrades (cargo capacity)
- Supply/demand dynamics
- Multiple trade routes
- Basic economy simulation

**Success Criteria:**
- 5-10 minutes of varied gameplay
- Multiple trade routes with different profitability
- Progression feels rewarding
- Economy reacts to player actions

---

### Phase 3: Polish & Demo (Weeks 9-12)

**Content Expansion:**
- 3 ships (starter, mid-tier, advanced)
- 10 stations (different economies)
- 20 trade goods (variety)
- 5-10 upgrade options
- Tutorial/instructions

**Polish:**
- UI improvements (better visuals)
- VFX for docking, purchases
- Sound effects (station ambience, purchase confirmation)
- Balance adjustments
- Bug fixes

**Demo Package:**
- 30-minute playable experience
- Clear progression arc
- No crashes or major bugs
- "Juice" - feedback for actions
- Save/load (basic)

---

## 🚫 OUT OF SCOPE (Do NOT Build)

**Explicitly Excluded from MVP:**
- ❌ Combat system (no weapons, no fights)
- ❌ Exploration mechanics (no scanning)
- ❌ Faction diplomacy (beyond price modifiers)
- ❌ Crew management (solo pilot only)
- ❌ Quest system (trading IS the game)
- ❌ Multiplayer/networking
- ❌ Advanced AI (basic economy only)
- ❌ Character customization
- ❌ Story/narrative
- ❌ Procedural generation (handcrafted content)

**If Someone Asks to Add These:**
Response: "Great idea! Let's add it AFTER we validate the trading loop is fun."

---

## 💻 TECHNICAL GUIDELINES

### Keep It Simple

**✅ DO:**
- Hardcode first, structure later
- Use existing Unreal components
- Simple UI (function over form initially)
- Minimal C++ (use Blueprints where possible)
- Direct approach over clever solutions

**❌ DON'T:**
- Create complex systems
- Over-engineer for future features
- Premature optimization
- Expose everything to Blueprints
- Build "just in case" features

---

### Code Structure (Phase 2+)

**Core Classes:**
```cpp
// Trading station actor
class ATradingStation : public AActor
{
    UPROPERTY()
    UMarketDataAsset* MarketData;
    
    UFUNCTION(BlueprintCallable)
    void OpenTradingUI();
    
    UFUNCTION(BlueprintCallable)
    float GetItemPrice(FName ItemID) const;
};

// Trading ship component
class UCargoComponent : public UActorComponent
{
    UPROPERTY()
    TMap<FName, int32> CargoInventory;
    
    UFUNCTION(BlueprintCallable)
    bool AddCargo(FName ItemID, int32 Quantity);
    
    UFUNCTION(BlueprintCallable)
    int32 GetCargoSpace() const;
};

// Economy manager
class UEconomyManager : public UGameInstanceSubsystem
{
    UFUNCTION()
    void UpdatePrices(float DeltaTime);
    
    UFUNCTION(BlueprintCallable)
    float CalculatePrice(FName ItemID, FName StationID) const;
};
```

**Data Assets:**
```cpp
// Trade item definition
UCLASS()
class UTradeItemDataAsset : public UDataAsset
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText ItemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BasePrice;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Volatility; // Price fluctuation
};

// Market configuration
UCLASS()
class UMarketDataAsset : public UDataAsset
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText StationName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, float> ItemPriceMultipliers;
};
```

---

### UI Requirements

**Trading Interface Must Have:**
- List of available items
- Current prices (buy/sell)
- Player credits (money)
- Available cargo space
- Profit/loss calculation
- Buy/sell buttons
- Quantity selector

**Visual Feedback:**
- Show price differences between stations
- Highlight profitable trades
- Display cargo capacity visually
- Show profit on completed trade
- Animate money changes

---

## 🧪 TESTING REQUIREMENTS

### Week 4: Prototype Playtest

**Test with 5-10 people** (external, not team):
- Give NO instructions
- Watch them play silently
- Record struggles/confusion
- Ask afterward:
  1. "Was this fun?" (target: 60%+ yes)
  2. "Would you play more?" (target: 40%+ yes)
  3. "What confused you?"
  4. "What felt good?"

**GO/NO-GO Decision:**
- ✅ GO if 60%+ had fun → Continue to Phase 2
- ❌ NO-GO if <60% fun → Pivot or redesign core loop

---

### Week 8: Structured Playtest

**Test with 10-20 people:**
- Give brief tutorial (2 minutes)
- Let them play 10-15 minutes
- Track metrics:
  - Completion rate
  - Number of trades
  - Confusion points
  - Fun rating
  - Buy intent

**Target Metrics:**
- 80%+ complete at least 3 trades
- 70%+ say "had fun"
- <3 major confusion points
- 50%+ "would buy" interest

---

### Week 12: Demo Validation

**Test with 20+ people:**
- Full 30-minute demo
- Minimal instructions
- Capture metrics:
  - Completion rate (target: 90%)
  - Fun rating (target: 75%+ "fun")
  - Buy intent (target: 50%+ "would buy")
  - Retention (would play again: 60%+)

**If Targets Met:**
Ready for publisher pitch.

**If Targets NOT Met:**
Additional polish or feature adjustments needed.

---

## 🎨 CONTENT CREATION GUIDELINES

### Trade Items

**Categories (3-5 categories, 4-5 items each):**
- Basic Goods (food, water, fuel)
- Luxury Items (rare foods, art, entertainment)
- Industrial (metals, components, machinery)
- Medical (medicine, equipment)
- Technology (electronics, software, computers)

**Item Properties:**
- Base price (credits)
- Volume (cargo space)
- Volatility (price changes)
- Station availability

**Example Items:**
```yaml
# Food category
- Water: 10 credits, 1 unit, low volatility, everywhere
- Protein Packs: 25 credits, 1 unit, medium volatility, common
- Luxury Food: 100 credits, 2 units, high volatility, rare

# Technology category  
- Basic Computer: 500 credits, 5 units, low volatility, common
- Advanced AI Core: 5000 credits, 3 units, high volatility, rare
```

---

### Stations

**5-10 Stations with Different Economies:**

**Agricultural Station:**
- Produces: Food, water
- Imports: Technology, machinery
- Economy: Stable prices

**Industrial Station:**
- Produces: Metals, components
- Imports: Food, medicine
- Economy: High volatility

**Luxury Station:**
- Produces: Art, entertainment
- Imports: Luxury food, rare goods
- Economy: Expensive, low volatility

**Research Station:**
- Produces: Technology, medicine
- Imports: Industrial materials
- Economy: High tech prices

**Trade Hub:**
- Produces: Nothing (imports everything)
- Imports: All categories
- Economy: Medium prices, balanced

**Station Properties:**
```yaml
name: Industrial Station Alpha
location: (1000, 5000, 0)
produces:
  - Metals: 0.8x base price
  - Components: 0.9x base price
imports:
  - Food: 1.3x base price
  - Medicine: 1.5x base price
docking_bays: 4
```

---

### Ships

**3 Ships with Clear Progression:**

**Starter Ship (Compact Trader):**
- Cargo: 10 units
- Speed: Medium
- Cost: Starting ship
- Description: Basic trading vessel

**Mid-Tier Ship (Merchant Hauler):**
- Cargo: 30 units
- Speed: Slow
- Cost: 50,000 credits
- Description: Specialized cargo hauler

**Advanced Ship (Trade Liner):**
- Cargo: 100 units
- Speed: Fast
- Cost: 250,000 credits
- Description: Professional trading vessel

---

## 📊 ECONOMY SIMULATION

### Simple Supply/Demand

**Core Formula:**
```
Current Price = Base Price × Station Multiplier × Supply Factor

Supply Factor = 1.0 + (Demand - Supply) / 100
```

**Rules:**
1. When player sells, supply increases (price drops)
2. When player buys, demand increases (price rises)
3. Prices gradually return to base over time
4. Station-specific multipliers (some stations always cheaper/expensive)

**Example:**
```
Water base price: 10 credits
Agricultural station: 0.8x = 8 credits (they produce it)
Industrial station: 1.2x = 12 credits (they need it)

Player buys 100 water at Ag station:
- Price rises from 8 to 8.5 (supply reduced)

Player sells 100 water at Industrial:
- Price drops from 12 to 11.5 (supply increased)

Profit: 11.5 × 100 - 8 × 100 = 350 credits
```

---

## 🎯 PROGRESSION SYSTEM

### Player Advancement

**Level 1 (Starting):**
- Ship: Compact Trader (10 cargo)
- Credits: 1,000
- Routes: 2 stations unlocked

**Level 2 (After ~30 minutes):**
- Ship: Merchant Hauler (30 cargo)
- Credits: 50,000
- Routes: 5 stations unlocked
- Upgrade: Better fuel efficiency

**Level 3 (After ~60 minutes):**
- Ship: Trade Liner (100 cargo)
- Credits: 250,000+
- Routes: All 10 stations
- Upgrade: All systems

**Progression Metrics:**
```
Time to Level 2: 30-45 minutes
Time to Level 3: 60-90 minutes
Total demo length: 30 minutes (ends at Level 2)
```

---

## 🎥 DEMO STRUCTURE (Week 12 Deliverable)

### 30-Minute Demo Flow

**Minutes 0-5: Tutorial**
- Start with 1,000 credits
- Docked at Agricultural Station
- Tutorial UI explains:
  - "Buy water (it's cheap here)"
  - "Fly to Industrial Station"
  - "Sell water (it's expensive there)"
- First trade guided

**Minutes 5-15: Core Loop**
- Player explores 2-3 stations
- Learns price differences
- Makes 5-10 trades
- Earns ~20,000 credits
- Sees progression path

**Minutes 15-25: Expansion**
- Unlocks more stations
- More complex trade routes
- Higher value items
- Bigger profits

**Minutes 25-30: Climax**
- Can afford ship upgrade
- Shows power of better ship
- Teases more content
- "To be continued..."

**Demo Ends:**
- Save game
- "Thanks for playing!"
- "Coming soon: More ships, stations, and features"

---

## 📈 SUCCESS METRICS (Week 12)

### Mandatory Metrics

**Completion:**
- [ ] 90%+ players complete full 30 minutes
- [ ] <10% quit early due to bugs/confusion

**Fun Factor:**
- [ ] 75%+ say "I had fun"
- [ ] 60%+ say "I would play again"
- [ ] 50%+ say "I would buy this"

**Engagement:**
- [ ] Average 10+ trades per session
- [ ] 80%+ explore all available stations
- [ ] 70%+ pursue ship upgrade

**Technical:**
- [ ] 0 crashes
- [ ] <5 known bugs
- [ ] 60 FPS on mid-range PC
- [ ] <30 second load time

**If All Metrics Met:**
→ Ready for publisher pitch  
→ Proceed with funding pursuit

**If Metrics NOT Met:**
→ Additional polish iteration  
→ Adjust and retest  
→ Don't pitch until fixed

---

## 🚀 DEVELOPMENT WORKFLOW

### Weekly Cycle

**Monday:**
- Plan week's features
- Review previous week feedback
- Set specific goals

**Tuesday-Thursday:**
- Development work
- Daily playtests (5 minutes)
- Fix critical bugs

**Friday:**
- Week milestone demo
- External playtest (if available)
- Gather feedback

**Weekend:**
- Review feedback
- Plan adjustments
- Mental health break

---

### Daily Checklist

**Every Development Day:**
- [ ] Code compiles without warnings
- [ ] Can launch and play for 5 minutes
- [ ] One new feature/fix completed
- [ ] Daily playtest completed
- [ ] Progress documented

---

### Go/No-Go Gates

**Week 4 Gate:**
If NOT met, STOP and pivot:
- [ ] Prototype playable start-to-finish
- [ ] 60%+ playtesters say "fun"
- [ ] Core loop clear to players
- [ ] No critical bugs

**Week 8 Gate:**
If NOT met, extend polish phase:
- [ ] 5 stations, 15+ goods
- [ ] 70%+ "had fun"
- [ ] Ship progression working
- [ ] <10 known bugs

**Week 12 Gate:**
If NOT met, delay publisher pitch:
- [ ] All success metrics met
- [ ] 30-minute demo polished
- [ ] External validation positive
- [ ] Ready to show publishers

---

## 💡 DESIGN PRINCIPLES

### Core Design Pillars

**1. Immediate Understanding:**
"Buy low, sell high" is universal. No complex explanations needed.

**2. Fast Iteration:**
Complete trade loop in 3-5 minutes. Fast feedback on decisions.

**3. Clear Progression:**
Visible path to better ship. Achievable goals.

**4. Emergent Depth:**
Simple rules, complex strategies emerge naturally.

**5. Satisfying Feedback:**
Every action has immediate, visible result.

---

### What Makes It Fun?

**Optimization Puzzle:**
Finding best trade routes = solving puzzle

**Risk vs Reward:**
Rare goods = high profit but volatile prices

**Progression Fantasy:**
Start small, end big. Visible power growth.

**Economic Mastery:**
Understanding market = skill expression

**Relaxing Loop:**
Low-stress, zen-like trading experience

---

## 🎨 VISUAL FEEDBACK

### Required "Juice"

**Trading Actions:**
- Money counter animates up/down
- Cargo visually fills/empties
- Purchase sound + VFX
- Profit highlight (green numbers float up)

**Ship Upgrades:**
- Celebration moment (fanfare, VFX)
- Ship visual changes
- Stats comparison (before/after)
- "Unlocked" notification

**Market Updates:**
- Price arrows (up/down indicators)
- Profitable trades highlighted
- Best deals emphasized
- Recent trades shown

---

## 🔧 TECHNICAL PRIORITIES

### Must Have (Week 12)

**Performance:**
- 60 FPS on mid-range PC
- No stuttering during flight
- Fast load times (<30 seconds)

**Stability:**
- 0 crashes in 30-minute session
- No progression blockers
- Save/load works reliably

**Polish:**
- UI responsive (no lag)
- VFX for key moments
- Sound feedback for actions
- Tutorial is clear

### Can Wait (Post-MVP)

**Advanced Features:**
- Multiple save slots
- Leaderboards
- Statistics tracking
- Procedural generation
- Mod support

---

## 📞 NEXT ACTIONS

### Immediate (This Week)

1. Read full critical review documents
2. Accept trade simulator as MVP focus
3. Design 2-page trade loop specification
4. Set up Phase 1 development environment

### Week 1-2

1. Write detailed game design doc (5 pages max)
2. Create mockups for trading UI
3. Define data structures
4. Begin hardcoded prototype

### Week 3-4

1. Complete hardcoded prototype
2. Internal playtesting
3. Prepare external playtest
4. Week 4 GO/NO-GO decision

---

## 🎯 REMEMBER

**This is NOT the full Adastrea vision.**

This is a **focused MVP** to:
- Prove market interest
- Validate core gameplay
- Secure funding
- Build sustainable development

**Full vision comes AFTER successful MVP.**

One polished gameplay loop beats ten half-finished systems.

**Build it. Ship it. Validate it. Then iterate.**

---

**Last Updated**: 2025-12-24  
**Phase**: Trade Simulator MVP (Dec 2025 - Mar 2026)  
**Status**: Active Development  
**Priority**: #1 - All other features deprioritized
