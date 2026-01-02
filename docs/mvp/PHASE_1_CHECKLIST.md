# Phase 1 Implementation Checklist

**Purpose**: Track Phase 1 (Weeks 1-4) implementation progress  
**Last Updated**: 2025-12-25  
**Target**: Playable prototype by Week 4

---

## 📅 Week 1: Core Infrastructure

### Day 1-2: Project Setup

- [ ] Create folder structure in Content Browser
  - [ ] `Content/Blueprints/GameModes/`
  - [ ] `Content/Blueprints/Trading/Ships/`
  - [ ] `Content/Blueprints/Trading/Stations/`
  - [ ] `Content/UI/Trading/`
  - [ ] `Content/Maps/Trading/`

- [ ] Create BP_TradingGameMode
  - [ ] Set up basic game mode class
  - [ ] Configure starting credits (1000)
  - [ ] Configure initial cargo capacity (10)

### Day 3-5: Player Ship

- [ ] Create BP_SimpleTradingShip
  - [ ] Add StaticMesh component (placeholder cube)
  - [ ] Add FloatingPawnMovement component
  - [ ] Add SpringArm and Camera components
  - [ ] Add PlayerTraderComponent (from C++)
  - [ ] Add simple Blueprint cargo tracking (Map<String, Integer>)

- [ ] Configure ship movement
  - [ ] Set up WASD input (MoveForward, MoveRight)
  - [ ] Set up EQ input (MoveUp)
  - [ ] Test movement in empty level
  - [ ] Adjust speed for ~1 minute travel time

- [ ] Add ship variables
  - [ ] CurrentStation reference
  - [ ] bIsDocked boolean
  - [ ] OpenTradingUI function
  - [ ] CloseTradingUI function

### Day 6-7: Trading Stations

- [ ] Create BP_Station_Agricultural
  - [ ] Add StaticMesh (green placeholder)
  - [ ] Add BoxCollision (docking trigger)
  - [ ] Add TextRender (station name)
  - [ ] Configure collision (OverlapAllDynamic)
  
- [ ] Add hardcoded prices (Station A)
  - [ ] Water: 8.0 (sell to player) / 6.0 (buy from player)
  - [ ] Food: 22.0 / 18.0
  - [ ] Fuel: 45.0 / 40.0
  
- [ ] Implement docking logic
  - [ ] OnBeginOverlap → Call ship's OpenTradingUI
  - [ ] OnEndOverlap → Call ship's CloseTradingUI
  - [ ] Test docking works

- [ ] Create BP_Station_Industrial (duplicate)
  - [ ] Change mesh color (blue/grey)
  - [ ] Change station name text
  - [ ] Update prices (HIGHER than Agricultural)
    - [ ] Water: 12.0 / 10.0
    - [ ] Food: 35.0 / 28.0
    - [ ] Fuel: 60.0 / 55.0

---

## 📅 Week 2: Trading UI

### Day 8-10: HUD

- [ ] Create WBP_SimpleHUD
  - [ ] Add Canvas Panel (root)
  - [ ] Add Text_Credits widget
  - [ ] Add Text_CargoSpace widget
  - [ ] Add Text_Profit widget
  - [ ] Add Text_Instructions widget

- [ ] Implement HUD logic
  - [ ] Event Construct → Get player ship
  - [ ] Event Tick → Update credits display
  - [ ] Event Tick → Update cargo space display
  - [ ] Event Tick → Update profit display (green if positive)

- [ ] Add HUD to ship
  - [ ] In BP_SimpleTradingShip BeginPlay
  - [ ] Create and add WBP_SimpleHUD to viewport
  - [ ] Test HUD appears and updates

### Day 11-14: Trading Interface

- [ ] Create WBP_TradeItemRow
  - [ ] Add Horizontal Box layout
  - [ ] Add Text_ItemName (e.g., "Water")
  - [ ] Add Text_Price (e.g., "8 cr")
  - [ ] Add Text_Stock (e.g., "Stock: 1000")
  - [ ] Add SpinBox_Quantity (0-10)
  - [ ] Add Button_Buy (green)
  - [ ] Add Button_Sell (orange)

- [ ] Implement TradeItemRow logic
  - [ ] Button_Buy OnClicked:
    - [ ] Check player has credits
    - [ ] Check player has cargo space
    - [ ] Deduct credits
    - [ ] Add to cargo
    - [ ] Show success message
  - [ ] Button_Sell OnClicked:
    - [ ] Check player has item in cargo
    - [ ] Add credits to player
    - [ ] Remove from cargo
    - [ ] Show profit in green

- [ ] Create WBP_TradingUI
  - [ ] Add Canvas Panel (root)
  - [ ] Add semi-transparent background
  - [ ] Add Vertical Box layout
  - [ ] Add Text_StationName
  - [ ] Add player info (credits, cargo)
  - [ ] Add Scroll Box for items
  - [ ] Add 3x WBP_TradeItemRow (Water, Food, Fuel)
  - [ ] Add Button_Close

- [ ] Implement TradingInterface logic
  - [ ] Event Construct → Get station and ship
  - [ ] Update station name
  - [ ] Populate item rows with station prices
  - [ ] Button_Close → Remove from parent, hide cursor
  - [ ] Set Input Mode: UI Only when shown

- [ ] Wire to stations
  - [ ] Station OnBeginOverlap creates WBP_TradingUI
  - [ ] Pass station reference to widget
  - [ ] Test opening/closing UI

---

## 📅 Week 3: Core Gameplay Loop

### Day 15-17: Test Level

- [ ] Create L_TradingTest map
  - [ ] Add directional light
  - [ ] Add sky atmosphere
  - [ ] Add post-process volume

- [ ] Place stations
  - [ ] Agricultural Station at (0, 0, 0)
  - [ ] Industrial Station at (10000, 0, 0)
  - [ ] Verify distance = ~1 minute flight

- [ ] Configure level
  - [ ] Add Player Start near Agricultural Station
  - [ ] Set GameMode Override: BP_TradingGameMode
  - [ ] Set Default Pawn: BP_SimpleTradingShip
  - [ ] Add visual markers (optional lights)

- [ ] Test complete loop
  - [ ] PIE → Ship spawns
  - [ ] Fly to Agricultural Station
  - [ ] Dock (UI appears)
  - [ ] Buy 10 Water (80 credits)
  - [ ] Undock (UI closes)
  - [ ] Fly to Industrial Station (~60 seconds)
  - [ ] Dock (UI appears)
  - [ ] Sell 10 Water (100 credits received)
  - [ ] Profit = +20 credits
  - [ ] HUD shows correct values

### Day 18-21: Balance & Polish

- [ ] Balance testing
  - [ ] Test profit margins are clear (25-50%)
  - [ ] Test cargo capacity allows good runs
  - [ ] Test starting credits allow 5+ trades
  - [ ] Adjust prices if needed

- [ ] Bug fixes
  - [ ] Fix UI not appearing
  - [ ] Fix credits not updating
  - [ ] Fix cargo not working
  - [ ] Fix ship speed issues
  - [ ] Fix docking trigger issues

- [ ] Quality of life
  - [ ] Add sound effects (optional)
  - [ ] Add VFX on purchase (optional)
  - [ ] Improve station visuals (optional)
  - [ ] Add tutorial text on HUD

---

## 📅 Week 4: Testing & Validation

### Day 22-24: Internal Testing

- [ ] **Stability Test** (run for 30 minutes):
  - [ ] No crashes
  - [ ] No memory leaks
  - [ ] No broken UI
  - [ ] Performance is smooth

- [ ] **Complete Trade Loop** (5+ times):
  - [ ] Buy at Station A
  - [ ] Fly to Station B
  - [ ] Sell at Station B
  - [ ] Return to Station A
  - [ ] Profit accumulates correctly

- [ ] **Edge Cases**:
  - [ ] Try to buy with 0 credits
  - [ ] Try to buy with full cargo
  - [ ] Try to sell items not in cargo
  - [ ] Undock mid-trade
  - [ ] Spam buy/sell buttons

- [ ] **User Experience**:
  - [ ] Is it clear what to do?
  - [ ] Are prices visible?
  - [ ] Is profit obvious?
  - [ ] Any confusion points?

### Day 25-26: Playtest Preparation

- [ ] Package game
  - [ ] File → Package Project → Windows 64-bit
  - [ ] Development configuration
  - [ ] Test packaged build yourself
  - [ ] Verify it runs on another PC

- [ ] Create playtest materials
  - [ ] Write README.txt with instructions
  - [ ] Create feedback form (Google Forms/TypeForm)
  - [ ] Questions:
    - [ ] "Was this fun?" (Yes/No)
    - [ ] "Would you play more?" (Yes/No/Maybe)
    - [ ] "What confused you?"
    - [ ] "What felt good?"
    - [ ] "What was frustrating?"
  - [ ] Create ZIP package (game + README)

- [ ] Recruit playtesters
  - [ ] Find 5-10 people (NOT developers)
  - [ ] Friends, family, gaming communities
  - [ ] Distribute ZIP file
  - [ ] Share feedback form link

### Day 27-28: External Playtest

- [ ] Run playtest
  - [ ] Send package to testers
  - [ ] Give them 2-3 days
  - [ ] Remind them to complete feedback
  - [ ] Answer questions if they get stuck

- [ ] Collect results
  - [ ] Wait for all responses
  - [ ] Compile feedback
  - [ ] Calculate metrics:
    - [ ] % who said "fun"
    - [ ] % who said "would play more"
    - [ ] List of confusion points
    - [ ] List of what felt good

- [ ] Analyze feedback
  - [ ] Read all comments
  - [ ] Identify patterns
  - [ ] Note what worked
  - [ ] Note what needs fixing

---

## 🎯 Week 4 GO/NO-GO Decision

### Success Metrics

**Calculate**:
- [ ] % playtesters who said "fun" = ____%
- [ ] % playtesters who said "would play more" = ____%
- [ ] Number of major confusion points = ____
- [ ] % who completed at least one trade = ____%
- [ ] Number of critical bugs = ____

**GO Decision Criteria** (all must be true):
- [ ] ✅ 60%+ said "fun"
- [ ] ✅ 40%+ said "would play more"
- [ ] ✅ <3 major confusion points
- [ ] ✅ 80%+ completed one trade loop
- [ ] ✅ No critical bugs (crashes, blockers)

**Decision**:
- [ ] **GO**: Proceed to Phase 2 (Weeks 5-8)
- [ ] **NO-GO**: Analyze issues, iterate, retest

---

## 📊 Phase 1 Completion

### Final Deliverables

**Code/Content**:
- [ ] BP_TradingGameMode (working)
- [ ] BP_SimpleTradingShip (flyable)
- [ ] BP_Station_Agricultural (functional)
- [ ] BP_Station_Industrial (functional)
- [ ] WBP_SimpleHUD (displays info)
- [ ] WBP_TradingUI (trading works)
- [ ] WBP_TradeItemRow (buy/sell works)
- [ ] L_TradingTest (playable level)

**Documentation**:
- [ ] Playtest results compiled
- [ ] Feedback analysis complete
- [ ] GO/NO-GO decision documented
- [ ] Lessons learned documented
- [ ] Phase 2 plan updated (if GO)

**Validation**:
- [ ] External playtest completed (5-10 people)
- [ ] Metrics calculated
- [ ] Decision made and documented
- [ ] Next steps defined

---

## 🔄 If NO-GO

### Iteration Plan

**Identify Core Issue**:
- [ ] Too tedious? → Reduce flight time, increase rewards
- [ ] Too confusing? → Add tutorial, improve UI clarity
- [ ] Not rewarding? → Increase profit margins, add feedback
- [ ] Technically broken? → Fix critical bugs
- [ ] Not fun fundamentally? → Consider pivot

**Iterate**:
- [ ] Make focused changes (1-2 weeks)
- [ ] Internal test again
- [ ] Run second playtest
- [ ] Re-evaluate GO/NO-GO

**Pivot Options** (if still NO-GO):
- [ ] Different core loop (combat? exploration?)
- [ ] Different game genre
- [ ] Reassess MVP scope
- [ ] Consult critical review documents

---

## 🎉 If GO

### Phase 2 Preparation

**Document What Worked**:
- [ ] Which aspects tested well?
- [ ] What did players enjoy most?
- [ ] What should be expanded in Phase 2?

**Plan Phase 2** (Weeks 5-8):
- [ ] Convert hardcoded values to Data Assets
- [ ] Add 5-10 stations with regional economies
- [ ] Add 15-20 trade goods with variety
- [ ] Implement supply/demand system
- [ ] Add ship progression (3 ships)
- [ ] See `TRADING_MVP_GUIDE.md` Phase 2

**Communicate Success**:
- [ ] Update CHANGELOG.md
- [ ] Share playtest results with team
- [ ] Celebrate validation milestone! 🎊

---

**Last Updated**: 2025-12-25  
**Phase**: 1 of 3  
**Status**: Ready to start  
**Next**: Begin Week 1, Day 1
