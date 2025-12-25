# Phase 1 Implementation Steps - Trade Simulator MVP

**Purpose**: Step-by-step guide to implement Phase 1 (Weeks 1-4) hardcoded prototype  
**Target**: Playable trading loop for Week 4 playtest (60%+ "fun" rating)  
**Last Updated**: 2025-12-25  
**Status**: Ready to implement

---

## 📋 Phase 1 Overview

**Goal**: Build minimum viable trading loop with hardcoded values

**What We're Building**:
- 1 flyable ship (10 cargo space)
- 2 stations (Agricultural and Industrial)
- 3 trade goods (Water, Food, Fuel)
- Basic trading UI
- Profit tracking

**Time Estimate**: 4 weeks (Weeks 1-4)

**Success Criteria**:
- Can fly between stations in ~1 minute
- Can buy cargo at Station A
- Can sell cargo at Station B
- Profit increases visibly
- Runs 10 minutes without crashes
- 60%+ playtesters say "fun"

---

## 🚀 Implementation Checklist

### Week 1: Core Infrastructure

#### Step 1: Create Folder Structure
```
Content/
├── Blueprints/
│   └── MVP/
│       ├── GameModes/
│       ├── Ships/
│       ├── Stations/
│       └── UI/
└── Maps/
    └── MVP/
```

**Actions**:
1. Open Unreal Editor
2. Navigate to Content folder
3. Create folder structure above
4. Save all

---

#### Step 2: Create Trading Game Mode

**File**: `Content/Blueprints/MVP/GameModes/BP_TradingGameMode`

**Parent Class**: `GameModeBase`

**Configuration**:
1. Create new Blueprint class
2. Set Default Pawn Class: `None` (we'll set in map)
3. Set HUD Class: `None` (for now)
4. Set Player Controller Class: `PlayerController`

**Variables** (hardcoded for prototype):
```
StartingCredits: Integer = 1000
InitialCargoCapacity: Integer = 10
```

**Functions**:
```blueprint
Function: InitializePlayer(PlayerPawn)
├─ Get PlayerTraderComponent
├─ Set Credits = StartingCredits
├─ Set StartingCredits = StartingCredits
└─ Print: "Starting with 1000 credits"
```

**Save and close**

---

#### Step 3: Create Simple Player Ship

**File**: `Content/Blueprints/MVP/Ships/BP_SimpleTradingShip`

**Parent Class**: `Pawn`

**Components**:
1. `StaticMesh` (Root)
   - Use any placeholder mesh (cube is fine)
   - Scale: 100x100x100
   - Material: Any bright color (player recognition)

2. `FloatingPawnMovement`
   - Max Speed: 1000
   - Acceleration: 2000
   - Deceleration: 4000
   - Turn Rate: 100

3. `SpringArm`
   - Target Arm Length: 1500
   - Enable Camera Lag: true
   - Camera Lag Speed: 3.0

4. `Camera`
   - Attach to SpringArm

5. **PlayerTraderComponent** (C++ component)
   - Add from Components panel
   - Find "Player Trader Component"
   - Credits: 1000
   - Starting Credits: 1000

**Note on Cargo Management for Phase 1**:

The C++ `CargoComponent` requires `UTradeItemDataAsset*` parameters, which conflicts with Phase 1's "no Data Assets" approach. For Phase 1, we'll use a **simple Blueprint-based cargo system** instead:

- Use a `TMap<String, Integer>` variable to track cargo (ItemName → Quantity)
- Track cargo space manually using simple integer arithmetic
- Phase 2 will migrate to the proper C++ `CargoComponent` with Data Assets

**Variables**:
```
CurrentStation: Reference to BP_SimpleTradingStation (nullable)
bIsDocked: Boolean = false

// Simple cargo tracking (Phase 1 only - no Data Assets)
CargoHold: Map<String, Integer>  // ItemName → Quantity
MaxCargoSpace: Integer = 10
CurrentCargoUsed: Integer = 0
```

**Input Setup**:
1. Project Settings → Input
2. Add Axis Mappings:
   - MoveForward: W (+1.0), S (-1.0)
   - MoveRight: D (+1.0), A (-1.0)
   - MoveUp: E (+1.0), Q (-1.0)

**Event Graph**:

```blueprint
Event BeginPlay
├─ Add to Viewport: WBP_SimpleHUD
└─ Set Show Mouse Cursor: false

InputAxis MoveForward
├─ Get Control Rotation
├─ Get Forward Vector
├─ Add Movement Input (Scale by Axis Value)

InputAxis MoveRight
├─ Get Control Rotation
├─ Get Right Vector
├─ Add Movement Input (Scale by Axis Value)

InputAxis MoveUp
├─ Make Vector (0, 0, AxisValue)
├─ Add Movement Input (World Space)
```

**Save and close**

---

#### Step 4: Create Agricultural Station (Station A)

**File**: `Content/Blueprints/MVP/Stations/BP_Station_Agricultural`

**Parent Class**: `Actor`

**Components**:
1. `StaticMesh` (Root)
   - Use any placeholder (cube or sphere)
   - Scale: 500x500x500
   - Material: Green (agricultural)

2. `BoxCollision` (Docking Trigger)
   - Name: "DockingTrigger"
   - Box Extent: 600x600x600
   - Collision Preset: "OverlapAllDynamic"
   - Generate Overlap Events: ✅ True

3. `TextRender` (Station Name)
   - Text: "Agricultural Station"
   - World Size: 100
   - Horizontal Alignment: Center
   - Location: (0, 0, 600) above station

**Variables** (Hardcoded prices):
```
StationName: String = "Agricultural Station Alpha"
StationType: String = "Agricultural"

// Prices from PLAYER perspective (clearer naming)
PlayerBuysAt_Water: Float = 8.0       // What player PAYS to buy from station
PlayerBuysAt_Food: Float = 22.0
PlayerBuysAt_Fuel: Float = 45.0

// Stock levels (for display, infinite for prototype)
Stock_Water: Integer = 1000
Stock_Food: Integer = 500
Stock_Fuel: Integer = 300

// Prices when player sells TO station
PlayerSellsAt_Water: Float = 6.0     // What player RECEIVES when selling to station
PlayerSellsAt_Food: Float = 18.0
PlayerSellsAt_Fuel: Float = 40.0
```

**Event Graph**:

```blueprint
Event OnComponentBeginOverlap (DockingTrigger)
├─ Other Actor → Cast to BP_SimpleTradingShip
├─ If Cast Success:
│  ├─ Set CurrentStation = Self
│  ├─ Set bIsDocked = true
│  ├─ Print: "Docked at Agricultural Station"
│  └─ Call: OpenTradingUI

Event OnComponentEndOverlap (DockingTrigger)
├─ Other Actor → Cast to BP_SimpleTradingShip
├─ If Cast Success:
│  ├─ Set CurrentStation = None
│  ├─ Set bIsDocked = false
│  └─ Call: CloseTradingUI
```

**Function: GetPlayerBuyPrice** (what player PAYS when buying from station):
```blueprint
Input: ItemName (String)
Output: Price (Float)

Switch on ItemName:
├─ "Water" → Return PlayerBuysAt_Water (8.0)
├─ "Food" → Return PlayerBuysAt_Food (22.0)
├─ "Fuel" → Return PlayerBuysAt_Fuel (45.0)
└─ Default → Return 0.0
```

**Function: GetPlayerSellPrice** (what player RECEIVES when selling to station):
```blueprint
Input: ItemName (String)
Output: Price (Float)

Switch on ItemName:
├─ "Water" → Return PlayerSellsAt_Water (6.0)
├─ "Food" → Return PlayerSellsAt_Food (18.0)
├─ "Fuel" → Return PlayerSellsAt_Fuel (40.0)
└─ Default → Return 0.0
```

**Save and close**

---

#### Step 5: Create Industrial Station (Station B)

**File**: `Content/Blueprints/MVP/Stations/BP_Station_Industrial`

**Duplicate from**: `BP_Station_Agricultural`

**Changes**:
1. Material color: Blue/Grey (industrial)
2. TextRender text: "Industrial Station"
3. **Different prices** (key difference):

```
StationName: String = "Industrial Station Beta"
StationType: String = "Industrial"

// Industrial Station PRICES (reverse economics from player perspective)
// Water is EXPENSIVE here (they need it)
PlayerBuysAt_Water: Float = 12.0    // What player PAYS - higher than Agricultural
PlayerBuysAt_Food: Float = 35.0     // Higher
PlayerBuysAt_Fuel: Float = 60.0     // Higher

// What player receives when selling to this station
PlayerSellsAt_Water: Float = 10.0   // What player RECEIVES - good profit!
PlayerSellsAt_Food: Float = 28.0
PlayerSellsAt_Fuel: Float = 55.0

// Stock levels
Stock_Water: Integer = 200    // Limited water
Stock_Food: Integer = 150
Stock_Fuel: Integer = 500     // Lots of fuel
```

**Expected Profit Routes**:
```
Route 1: Agricultural → Industrial (Water)
- Buy Water @ 8 credits (PlayerBuysAt_Water at Agricultural)
- Sell Water @ 10 credits (PlayerSellsAt_Water at Industrial)
- Profit: 2 credits per unit (25%)
- 10 cargo = 20 credits profit

Route 2: Agricultural → Industrial (Food)
- Buy Food @ 22 credits (PlayerBuysAt_Food at Agricultural)
- Sell Food @ 28 credits (PlayerSellsAt_Food at Industrial)
- Profit: 6 credits per unit (27%)
- 10 cargo = 60 credits profit
```

**Save and close**

---

### Week 2: Trading UI

#### Step 6: Create Simple HUD

**File**: `Content/Blueprints/MVP/UI/WBP_SimpleHUD`

**Parent Class**: `UserWidget`

**Widget Structure**:
```
Canvas Panel (Root)
├── Text_Credits (Top-left)
│   └── Text: "Credits: 1000"
│       Font Size: 24
│       Color: Yellow
│
├── Text_CargoSpace (Top-left, below credits)
│   └── Text: "Cargo: 0/10"
│       Font Size: 20
│       Color: White
│
├── Text_Profit (Top-left, below cargo)
│   └── Text: "Profit: +0"
│       Font Size: 20
│       Color: Green
│
└── Text_Instructions (Bottom-center)
    └── Text: "Fly into green station to dock"
        Font Size: 18
        Color: White
```

**Variables**:
```
PlayerShip: Reference to BP_SimpleTradingShip
```

**Event Graph**:
```blueprint
Event Construct
├─ Get Player Pawn
└─ Cast to BP_SimpleTradingShip → Store in PlayerShip

Event Tick
├─ Get PlayerTraderComponent from PlayerShip
├─ Update Text_Credits: Format("Credits: {0}", GetCredits)
├─ Get PlayerShip.CurrentCargoUsed and MaxCargoSpace
├─ Update Text_CargoSpace: Format("Cargo: {0}/{1}", CurrentCargoUsed, MaxCargoSpace)
└─ Update Text_Profit: Format("Profit: {0}", GetProfit)
```

**Compile and save**

---

#### Step 7: Create Trading Interface

**File**: `Content/Blueprints/MVP/UI/WBP_TradingInterface`

**Parent Class**: `UserWidget`

**Widget Structure**:
```
Canvas Panel (Root)
├── Border_Background (Full screen, semi-transparent)
│   └── Vertical Box
│       ├── Text_StationName
│       │   └── Text: "Agricultural Station Alpha"
│       │       Font Size: 32
│       │
│       ├── Horizontal Box (Player Info)
│       │   ├── Text: "Credits: 1000"
│       │   └── Text: "Cargo: 5/10"
│       │
│       ├── Text_Title
│       │   └── Text: "Trading"
│       │       Font Size: 24
│       │
│       ├── Scroll Box (Item List)
│       │   ├── WBP_TradeItemRow (Water)
│       │   ├── WBP_TradeItemRow (Food)
│       │   └── WBP_TradeItemRow (Fuel)
│       │
│       └── Button_Close
│           └── Text: "Close (Undock)"
```

**Variables**:
```
CurrentStation: Reference to BP_Station_Agricultural or Industrial
PlayerShip: Reference to BP_SimpleTradingShip
```

**Event Graph**:
```blueprint
Event Construct
├─ Get Player Pawn → Cast to BP_SimpleTradingShip
├─ Get CurrentStation from PlayerShip
├─ Update StationName text
├─ Populate item rows
└─ Set Input Mode: UI Only

Button_Close OnClicked
├─ Remove from Parent
├─ Set Input Mode: Game Only
└─ Hide Mouse Cursor
```

**Compile and save**

---

#### Step 8: Create Trade Item Row

**File**: `Content/Blueprints/MVP/UI/WBP_TradeItemRow`

**Parent Class**: `UserWidget`

**Widget Structure**:
```
Horizontal Box
├── Text_ItemName (150px)
│   └── Text: "Water"
│       Font Size: 18
│
├── Text_Price (100px)
│   └── Text: "8 cr"
│       Font Size: 18
│       Color: Yellow
│
├── Text_Stock (100px)
│   └── Text: "Stock: 1000"
│       Font Size: 16
│
├── SpinBox_Quantity (100px)
│   └── Min: 0, Max: 10, Value: 1
│
├── Button_Buy (100px)
│   └── Text: "Buy"
│       Color: Green
│
└── Button_Sell (100px)
    └── Text: "Sell"
        Color: Orange
```

**Variables**:
```
ItemName: String = "Water"
PlayerBuyPrice: Float = 8.0       // What player PAYS when buying from station
PlayerSellPrice: Float = 6.0      // What player RECEIVES when selling to station
StockLevel: Integer = 1000
CurrentStation: Reference to station
PlayerShip: Reference to ship
```

**Button_Buy OnClicked**:
```blueprint
Get Quantity from SpinBox_Quantity
Calculate TotalCost = Quantity * PlayerBuyPrice

Get PlayerTraderComponent
├─ Check: GetCredits >= TotalCost?
│  ├─ Yes:
│  │  ├─ Check: PlayerShip.CurrentCargoUsed + Quantity <= PlayerShip.MaxCargoSpace?
│  │  │  ├─ Yes:
│  │  │  │  ├─ Remove Credits (TotalCost)
│  │  │  │  ├─ Add to PlayerShip.CargoHold Map (ItemName, Quantity)
│  │  │  │  ├─ Update PlayerShip.CurrentCargoUsed += Quantity
│  │  │  │  ├─ Play success sound
│  │  │  │  ├─ Print: "Bought X Water for Y credits"
│  │  │  │  └─ Update UI
│  │  │  └─ No:
│  │  │      └─ Print: "Insufficient cargo space"
│  └─ No:
│      └─ Print: "Insufficient credits"
```

**Button_Sell OnClicked**:
```blueprint
Get Quantity from SpinBox_Quantity

Check PlayerShip.CargoHold Map
├─ Check: Contains ItemName with Quantity >= Quantity?
│  ├─ Yes:
│  │  ├─ Calculate TotalValue = Quantity * PlayerSellPrice
│  │  ├─ Remove from PlayerShip.CargoHold Map (ItemName, Quantity)
│  │  ├─ Update PlayerShip.CurrentCargoUsed -= Quantity
│  │  ├─ Add Credits (TotalValue)
│  │  ├─ Play success sound
│  │  ├─ Print: "Sold X Water for Y credits"
│  │  └─ Update UI (show profit in green)
│  └─ No:
│      └─ Print: "Don't have that item"
```

**Note**: Phase 1 uses simple Blueprint Map for cargo tracking. Phase 2 will migrate to C++ CargoComponent with Data Assets.

**Compile and save**

---

### Week 3: Test Level & Integration

#### Step 9: Create Test Level

**File**: `Content/Maps/MVP/L_TradingTest`

**Setup**:

1. **Create new level** (File → New Level → Empty Level)

2. **Add lighting**:
   - Directional Light (Sun)
   - Skybox or Sky Atmosphere
   - Post Process Volume (Ambient settings)

3. **Place stations**:
   - Agricultural Station:
     - Location: (0, 0, 0)
     - Rotation: (0, 0, 0)
   
   - Industrial Station:
     - Location: (10000, 0, 0)  // 10,000 units away
     - Rotation: (0, 0, 0)
   
   - Distance: ~1 minute flight at speed 1000

4. **Add Player Start**:
   - Location: (-1000, 0, 0)  // Near Agricultural Station
   - Rotation: (0, 0, 0)

5. **Configure World Settings**:
   - Game Mode Override: BP_TradingGameMode
   - Default Pawn Class: BP_SimpleTradingShip

6. **Add visual markers** (optional):
   - Colored lights above stations (green/blue)
   - Direction arrows
   - Distance markers

7. **Test flight time**:
   - PIE (Play In Editor)
   - Fly from Station A to Station B
   - Should take ~60 seconds at full speed

**Save level**

---

#### Step 10: Wire Everything Together

**In BP_SimpleTradingShip**:

Update Event Graph:

```blueprint
Function: OpenTradingUI
├─ Create Widget: WBP_TradingInterface
├─ Set CurrentStation reference
├─ Set PlayerShip reference
├─ Add to Viewport
├─ Set Input Mode: UI Only
└─ Show Mouse Cursor: true

Function: CloseTradingUI
├─ Remove WBP_TradingInterface from viewport
├─ Set Input Mode: Game Only
└─ Show Mouse Cursor: false
```

**In BP_Station_Agricultural and BP_Station_Industrial**:

Ensure docking trigger calls:
```blueprint
OnBeginOverlap → Ship.OpenTradingUI(This)
OnEndOverlap → Ship.CloseTradingUI()
```

**Compile all blueprints**

---

### Week 4: Testing & Validation

#### Step 11: Internal Testing

**Test Checklist**:

- [ ] **Launch Test**:
  - PIE in L_TradingTest
  - Ship spawns near Agricultural Station
  - HUD shows credits: 1000, cargo: 0/10

- [ ] **Movement Test**:
  - WASD moves ship correctly
  - EQ moves up/down
  - Camera follows smoothly
  - Can reach Industrial Station in ~1 minute

- [ ] **Docking Test**:
  - Fly into Agricultural Station collision box
  - Trading UI appears
  - Shows station name and items
  - Shows correct prices

- [ ] **Buying Test**:
  - Buy 10 Water @ 8 credits = 80 credits cost
  - Credits decrease to 920
  - Cargo increases to 10/10
  - HUD updates correctly

- [ ] **Flying With Cargo**:
  - Undock (close UI or fly away)
  - UI closes
  - Fly to Industrial Station
  - Takes ~1 minute

- [ ] **Selling Test**:
  - Dock at Industrial Station
  - Trading UI appears with Industrial prices
  - Sell 10 Water @ 10 credits = 100 credits received
  - Credits increase to 1020
  - Cargo decreases to 0/10
  - Profit shows +20 credits in green

- [ ] **Multiple Runs**:
  - Repeat buy-sell cycle 5 times
  - Profit accumulates correctly
  - No crashes or bugs
  - Runs for 10 minutes without issues

---

#### Step 12: Bug Fixes

**Common Issues**:

1. **UI doesn't appear**:
   - Check docking trigger collision settings
   - Verify OpenTradingUI is called
   - Check widget is added to viewport

2. **Credits don't update**:
   - Verify PlayerTraderComponent is present
   - Check credits are being modified
   - Ensure HUD is updating from component

3. **Cargo doesn't work**:
   - Verify CargoComponent is present
   - Check capacity is set correctly
   - Ensure cargo is being added/removed

4. **Ship flies too fast/slow**:
   - Adjust FloatingPawnMovement speed
   - Test different values (500-2000)
   - Balance for 1-minute flight time

5. **Can't dock**:
   - Increase collision box size
   - Check "Generate Overlap Events" is true
   - Verify collision preset

---

#### Step 13: Prepare Playtest

**Playtest Package**:

1. **Build Instructions**:
   ```
   Package Settings:
   - Windows 64-bit
   - Development configuration
   - Include: L_TradingTest only
   ```

2. **Create README.txt**:
   ```
   Trading Prototype - Playtest Instructions
   
   Goal: Test if "buy low, sell high" is fun
   
   Controls:
   - WASD: Move ship
   - E/Q: Up/Down
   - Mouse: Look around
   - Fly into stations to dock
   
   Instructions:
   1. You start with 1000 credits
   2. Fly to the GREEN station (ahead)
   3. Dock by flying into it
   4. Buy some Water (it's cheap here)
   5. Close the menu (fly away)
   6. Fly to the BLUE station (in the distance)
   7. Dock and sell your Water (it's expensive here)
   8. Check your profit!
   9. Try to make as much profit as possible in 10 minutes
   
   After playing:
   - Was this fun? (Yes/No)
   - Would you play more? (Yes/No)
   - What confused you?
   - What felt good?
   ```

3. **Package game**:
   - File → Package Project → Windows (64-bit)
   - Wait for packaging to complete
   - Test packaged build yourself first
   - Create ZIP with game + README.txt

---

#### Step 14: Run External Playtest

**Playtest Process**:

1. **Recruit testers** (5-10 people):
   - Friends/family
   - Gaming communities
   - Discord servers
   - NOT other developers (too technical)

2. **Distribution**:
   - Send ZIP file
   - Include README.txt
   - Ask them to play for 10-15 minutes
   - Collect feedback via form/survey

3. **Feedback Form**:
   ```
   Trading Prototype Feedback
   
   1. Was this fun? (Yes/No)
   2. Would you play more? (Yes/No/Maybe)
   3. What confused you?
   4. What felt good/satisfying?
   5. What was boring or frustrating?
   6. How much profit did you make?
   7. Any other comments?
   ```

4. **Analyze Results**:
   - Calculate % "Yes" to "Was this fun?"
   - Target: 60%+ for GO decision
   - Read all qualitative feedback
   - Identify patterns in confusion/issues

---

## 📊 Week 4 GO/NO-GO Decision

### Success Criteria

**GO to Phase 2 if**:
- ✅ 60%+ playtesters say "fun"
- ✅ 40%+ say "would play more"
- ✅ <3 major confusion points
- ✅ 80%+ complete at least one trade loop
- ✅ No critical bugs (crashes, game-breaking)

**NO-GO (Pivot) if**:
- ❌ <60% say "fun"
- ❌ >5 major confusion points
- ❌ <50% complete one trade loop
- ❌ Feedback indicates fundamental issues

### If GO:
- Proceed to Phase 2 (Weeks 5-8)
- Convert hardcoded values to Data Assets
- Add more stations and items
- Implement supply/demand system
- See `TRADING_MVP_GUIDE.md` Phase 2

### If NO-GO:
- Analyze core issue:
  - Too tedious? (reduce flight time)
  - Too confusing? (better tutorial)
  - Not rewarding? (increase profits)
  - Not fun fundamentally? (pivot to different gameplay)
- Iterate on prototype
- Retest in 1-2 weeks
- Consider pivoting to different MVP focus

---

## 🎯 Expected Outcomes

**At End of Week 4**:

✅ **Playable Prototype**:
- Can fly between stations
- Can buy and sell goods
- Can make profit
- Runs 10+ minutes without crashes

✅ **Validation Data**:
- 5-10 external playtest results
- Quantitative metrics (% fun, % would play)
- Qualitative feedback (what worked/didn't)
- GO/NO-GO decision made

✅ **Lessons Learned**:
- What aspects are fun
- What needs improvement
- What to prioritize in Phase 2
- Whether core loop is worth expanding

---

## 📝 Common Questions

**Q: Should I add more items/stations in Phase 1?**  
A: NO. Keep it minimal. 2 stations, 3 items. Validate core loop first.

**Q: Should I make it look good?**  
A: NO. Placeholder cubes are fine. Function over form in Phase 1.

**Q: Should I add save/load?**  
A: NO. Session-only is fine for prototype.

**Q: Should I use Data Assets in Phase 1?**  
A: NO. Hardcode everything. Speed is priority.

**Q: What if playtesters want more features?**  
A: Document requests, but focus on validating core loop. Add features in Phase 2 IF GO decision.

**Q: How do I know if 60% "fun" is good enough?**  
A: 60% is the minimum bar. If you get 60-70%, proceed cautiously. If 75%+, you have something solid.

---

## 🔗 Related Documents

- **Full MVP Guide**: `TRADING_MVP_GUIDE.md`
- **MVP Inspiration**: `../reference/TRADING_MVP_INSPIRATION.md`
- **Blueprint Guide**: `BLUEPRINT_IMPLEMENTATION_GUIDE.md`
- **Anti-Patterns**: `../../.github/instructions/anti-patterns.instructions.md`

---

**Last Updated**: 2025-12-25  
**Phase**: 1 of 3 (Weeks 1-4)  
**Status**: Ready to implement  
**Next**: Begin Step 1 folder setup
