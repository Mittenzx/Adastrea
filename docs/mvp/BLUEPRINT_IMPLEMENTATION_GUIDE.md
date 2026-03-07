# Trading MVP Blueprint Implementation Guide

**Purpose**: Step-by-step instructions for implementing trading MVP in Blueprints
**Target**: Designers and Blueprint developers
**Last Updated**: 2026-01-13
**Phase**: Phase 2 (Weeks 5-8) - Structured Version
**Note**: For Phase 1, see [PHASE_1_IMPLEMENTATION_STEPS.md](PHASE_1_IMPLEMENTATION_STEPS.md)

---

## 📌 When to Use This Guide

**Use this guide if**:
- ✅ You've completed Phase 1 (hardcoded prototype)
- ✅ Week 4 playtest showed 60%+ "fun" rating (GO decision)
- ✅ You're ready to convert to Data Assets (Phase 2)

**Don't use this yet if**:
- ❌ Still in Phase 1 (Week 1-4) - Use PHASE_1_IMPLEMENTATION_STEPS.md instead
- ❌ Haven't validated core loop is fun
- ❌ Week 4 playtest hasn't happened yet

**Phase 2 Goal**: Add variety and progression with Data Assets (Weeks 5-8)

---

## 📋 Prerequisites

Before starting, ensure you have:
- [ ] Unreal Engine 5.6 project open
- [ ] Source code compiled (EconomyManager, CargoComponent, PlayerTraderComponent)
- [ ] Trade item YAML templates reviewed (`Content/DataAssets/Trading/TradeItemTemplates.yaml`)
- [ ] Market YAML templates reviewed (`Content/DataAssets/Trading/MarketTemplates.yaml`)

---

## Part 1: Create Data Assets

### Step 1: Create Trade Items

**Create 10 essential items** (start simple, expand later):

1. **Navigate**: `Content/DataAssets/Trading/Items/`
2. **Right-click** → Blueprint Class → Search "TradeItemDataAsset"
3. **Create these items**:
   - `DA_TradeItem_Water`
   - `DA_TradeItem_ProteinPacks`
   - `DA_TradeItem_BasicFuel`
   - `DA_TradeItem_BasicComputer`
   - `DA_TradeItem_MedicalSupplies`
   - `DA_TradeItem_RawOre`
   - `DA_TradeItem_AdvancedAICore`
   - `DA_TradeItem_LuxuryFood`
   - `DA_TradeItem_AlienArtifacts`
   - `DA_TradeItem_RefinedMetal`

4. **For each item**, open and configure using YAML template values

**Example: Water**
```
Basic Info:
  Item Name: "Water"
  Description: "Essential liquid for survival"
  Item ID: "Water"
  Category: Food & Consumables

Pricing:
  Base Price: 10.0
  Price Volatility:
    Volatility Multiplier: 1.0
    Min Price Deviation: 0.5
    Max Price Deviation: 2.0
  Affected By Supply Demand: ✓

Trade Properties:
  Volume Per Unit: 1.0
  Mass Per Unit: 1.0
  Standard Lot Size: 10
  Typical Market Stock: 1000
  Replenishment Rate: 100

Legality:
  Legality Status: Legal
  Contraband Fine Multiplier: 1.0

AI Behavior:
  AI Trade Priority: 5
  AI Hoardable: ☐
  AI Arbitrage Enabled: ☐
```

5. **Save all** items

---

### Step 2: Create Markets

**Create 5 essential markets**:

1. **Navigate**: `Content/DataAssets/Trading/Markets/`
2. **Right-click** → Blueprint Class → Search "MarketDataAsset"
3. **Create these markets**:
   - `DA_Market_AgriculturalStationAlpha`
   - `DA_Market_IndustrialStationBeta`
   - `DA_Market_MiningOutpostGamma`
   - `DA_Market_LuxuryBazaar`
   - `DA_Market_CentralTradeHub`

4. **For each market**, configure using YAML template

**Example: Agricultural Station Alpha**
```
Market Info:
  Market Name: "Agricultural Station Alpha"
  Description: "Food production facility"
  Market ID: "AgStation_01"
  Market Type: Open Market
  Market Size: Medium Station

Market Config:
  Transaction Tax Rate: 0.05
  Sell Price Markup: 1.2
  Buy Price Markdown: 0.8
  Allow Player Buying: ✓
  Allow Player Selling: ✓
  Allow AI Traders: ☐
  Min Reputation Required: -100

Inventory:
  [Add entries - click + button for each item]

  Entry 0:
    Trade Item: DA_TradeItem_Water
    Current Stock: 1000
    Max Stock: 2000
    Supply Level: 1.5
    Demand Level: 0.5

  Entry 1:
    Trade Item: DA_TradeItem_ProteinPacks
    Current Stock: 500
    Max Stock: 1000
    Supply Level: 1.3
    Demand Level: 0.7

  Entry 2:
    Trade Item: DA_TradeItem_BasicComputer
    Current Stock: 10
    Max Stock: 50
    Supply Level: 0.3
    Demand Level: 1.8

  Entry 3:
    Trade Item: DA_TradeItem_BasicFuel
    Current Stock: 200
    Max Stock: 500
    Supply Level: 0.8
    Demand Level: 1.2

Stock Refresh Rate: 24.0
```

5. **Verify price differences** between stations (profitable routes exist)
6. **Save all** markets

---

## Part 2: Create Trading Station Blueprint

### Step 3: Create BP_TradingStation

1. **Create new Blueprint**:
   - Navigate: `Content/Blueprints/Trading/Stations/`
   - Right-click → Blueprint Class → Actor
   - Name: `BP_TradingStation`

2. **Add Components**:
   ```
   BP_TradingStation (Actor)
   ├─ StaticMesh (StaticMeshComponent) [station model]
   ├─ DockingTrigger (BoxComponent)
   ├─ StationNameText (TextRenderComponent)
   └─ MarkerArrow (ArrowComponent) [docking direction]
   ```

3. **Configure Docking Trigger**:
   - Size: 500 x 500 x 500 (adjust for ship)
   - Collision Preset: "Overlap All Dynamic"
   - Generate Overlap Events: ✓

4. **Add Variables**:
   ```
   MarketData: Object Reference (MarketDataAsset)
     - Instance Editable: ✓
     - Category: "Trading"

   StationName: Text
     - Instance Editable: ✓
     - Category: "Trading"

   bPlayerDocked: Boolean
     - Default: false
   ```

5. **Event BeginPlay**:
   ```blueprint
   Event BeginPlay
   ├─ Get Game Instance
   ├─ Get Subsystem (EconomyManager)
   ├─ Register Market (MarketData)
   └─ Set Text (StationNameText) = StationName
   ```

6. **Docking Logic** (DockingTrigger → OnComponentBeginOverlap):
   ```blueprint
   OnComponentBeginOverlap (DockingTrigger)
   ├─ Get Owner (Other Actor)
   ├─ Get Component by Class (PlayerTraderComponent)
   ├─ Branch (Is Valid?)
   │  ├─ TRUE:
   │  │  ├─ Set bPlayerDocked = True
   │  │  ├─ Print String: "Docked at {StationName}"
   │  │  ├─ Create Widget (WBP_TradingUI)
   │  │  ├─ Set MarketData on widget
   │  │  ├─ Add to Viewport
   │  │  ├─ Set Input Mode UI Only
   │  │  └─ Show Mouse Cursor
   │  └─ FALSE: (do nothing)
   ```

7. **Undocking Logic** (DockingTrigger → OnComponentEndOverlap):
   ```blueprint
   OnComponentEndOverlap (DockingTrigger)
   ├─ Set bPlayerDocked = False
   └─ Print String: "Undocked from {StationName}"
   ```

8. **Compile and save**

---

## Part 3: Create Player Ship with Trading

### Step 4: Create BP_TradingShip

1. **Create new Blueprint**:
   - Navigate: `Content/Blueprints/Trading/Ships/`
   - Right-click → Blueprint Class → Pawn
   - Name: `BP_TradingShip`

2. **Add Components**:
   ```
   BP_TradingShip (Pawn)
   ├─ ShipMesh (StaticMeshComponent)
   ├─ SpringArm (SpringArmComponent)
   ├─ Camera (CameraComponent)
   ├─ FloatingPawnMovement (FloatingPawnMovement)
   ├─ CargoComponent (CargoComponent) [NEW!]
   └─ PlayerTraderComponent (PlayerTraderComponent) [NEW!]
   ```

3. **Configure Components**:
   ```
   CargoComponent:
     - Cargo Capacity: 10.0 (starter ship)

   PlayerTraderComponent:
     - Credits: 1000
     - Starting Credits: 1000

   FloatingPawnMovement:
     - Max Speed: 1000.0
     - Acceleration: 500.0
   ```

4. **Add Input Bindings**:
   - Project Settings → Input → Action Mappings
   - Add: `MoveForward` (W, S keys)
   - Add: `MoveRight` (A, D keys)
   - Add: `MoveUp` (Space, C keys)

5. **Movement Logic**:
   ```blueprint
   InputAxis MoveForward
   ├─ Get Forward Vector
   ├─ Add Movement Input (Direction, Scale Value)

   InputAxis MoveRight
   ├─ Get Right Vector
   ├─ Add Movement Input (Direction, Scale Value)

   InputAxis MoveUp
   ├─ Get Up Vector
   ├─ Add Movement Input (Direction, Scale Value)
   ```

6. **Compile and save**

---

## Part 4: Create Trading UI

### Step 5: Create WBP_TradingUI

1. **Create Widget Blueprint**:
   - Navigate: `Content/Blueprints/Trading/UI/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_TradingUI`

2. **UI Structure**:
   ```
   Canvas Panel
   ├─ Background (Image) [semi-transparent dark]
   ├─ Header (Horizontal Box)
   │  ├─ StationNameText (Text Block)
   │  └─ CloseButton (Button)
   ├─ PlayerInfo (Horizontal Box)
   │  ├─ CreditsText (Text Block) "Credits: 1000"
   │  ├─ CargoText (Text Block) "Cargo: 5/10"
   │  └─ ProfitText (Text Block) "Profit: +50"
   └─ ItemList (Scroll Box)
      └─ [Generated item rows]
   ```

3. **Add Variables**:
   ```
   MarketData: Object Reference (MarketDataAsset)
     - Instance Editable: ☐

   PlayerShipRef: Object Reference (Actor)
     - Instance Editable: ☐

   CargoComponentRef: Object Reference (CargoComponent)

   TraderComponentRef: Object Reference (PlayerTraderComponent)

   ItemRowWidgetClass: Widget Class Reference (WBP_TradeItemRow)
     - Default: WBP_TradeItemRow
   ```

4. **Event Construct**:
   ```blueprint
   Event Construct
   ├─ Get Owning Player Pawn → PlayerShipRef
   ├─ Get Component by Class (CargoComponent) → CargoComponentRef
   ├─ Get Component by Class (PlayerTraderComponent) → TraderComponentRef
   ├─ Call: PopulateItemList
   └─ Call: UpdatePlayerInfo
   ```

5. **Function: PopulateItemList**:
   ```blueprint
   Function: PopulateItemList
   ├─ Clear Children (ItemList ScrollBox)
   ├─ Get Inventory (MarketData)
   ├─ ForEach Inventory Entry:
   │  ├─ Create Widget (WBP_TradeItemRow)
   │  ├─ Set Item Data on widget
   │  ├─ Set Market Data on widget
   │  ├─ Bind to BuyClicked event
   │  ├─ Bind to SellClicked event
   │  └─ Add Child to Scroll Box
   ```

6. **Function: UpdatePlayerInfo**:
   ```blueprint
   Function: UpdatePlayerInfo
   ├─ Get Credits (TraderComponent) → CreditsValue
   ├─ Set Text (CreditsText): "Credits: {CreditsValue}"
   ├─ Get Used Cargo Space → UsedSpace
   ├─ Get Cargo Capacity → TotalSpace
   ├─ Set Text (CargoText): "Cargo: {UsedSpace}/{TotalSpace}"
   ├─ Get Profit → ProfitValue
   └─ Set Text (ProfitText): "Profit: {ProfitValue}"
   ```

7. **CloseButton → OnClicked**:
   ```blueprint
   OnClicked (CloseButton)
   ├─ Remove from Parent (self)
   ├─ Set Input Mode Game Only
   └─ Hide Mouse Cursor
   ```

8. **Compile and save**

---

### Step 6: Create WBP_TradeItemRow

1. **Create Widget Blueprint**:
   - Name: `WBP_TradeItemRow`

2. **UI Structure**:
   ```
   Horizontal Box
   ├─ ItemNameText (Text Block) "Water"
   ├─ PriceText (Text Block) "10 CR"
   ├─ StockText (Text Block) "Stock: 1000"
   ├─ QuantityBox (Spin Box) [1-100]
   ├─ BuyButton (Button) "BUY"
   └─ SellButton (Button) "SELL"
   ```

3. **Add Variables**:
   ```
   ItemData: Object Reference (TradeItemDataAsset)
   MarketData: Object Reference (MarketDataAsset)
   Quantity: Integer (default: 1)
   ```

4. **Event Construct**:
   ```blueprint
   Event Construct
   ├─ Set Text (ItemNameText): ItemData->ItemName
   ├─ Get Economy Manager
   ├─ Get Item Price (Market, Item, bIsBuying=true) → BuyPrice
   ├─ Get Item Price (Market, Item, bIsBuying=false) → SellPrice
   ├─ Set Text (PriceText): "Buy: {BuyPrice} / Sell: {SellPrice}"
   ├─ Get Inventory Entry → StockValue
   └─ Set Text (StockText): "Stock: {StockValue}"
   ```

5. **BuyButton → OnClicked**:
   ```blueprint
   OnClicked (BuyButton)
   ├─ Get Value (QuantityBox) → Quantity
   ├─ Get Player Trader Component
   ├─ Get Cargo Component
   ├─ Buy Item (Market, Item, Quantity, Cargo)
   ├─ Branch (Success?)
   │  ├─ TRUE: Play Success Sound
   │  └─ FALSE: Play Error Sound
   └─ Update Parent UI
   ```

6. **SellButton → OnClicked**:
   ```blueprint
   OnClicked (SellButton)
   ├─ Get Value (QuantityBox) → Quantity
   ├─ Get Player Trader Component
   ├─ Get Cargo Component
   ├─ Sell Item (Market, Item, Quantity, Cargo)
   ├─ Branch (Success?)
   │  ├─ TRUE: Play Success Sound
   │  └─ FALSE: Play Error Sound
   └─ Update Parent UI
   ```

7. **Compile and save**

---

## Part 5: Create HUD

### Step 7: Create WBP_TradingHUD

1. **Create Widget Blueprint**:
   - Navigate: `Content/Blueprints/Trading/UI/`
   - Name: `WBP_TradingHUD`

2. **UI Structure**:
   ```
   Canvas Panel
   ├─ TopLeft (Vertical Box)
   │  ├─ CreditsText (Text Block) "Credits: 1000"
   │  ├─ CargoText (Text Block) "Cargo: 0/10"
   │  └─ ProfitText (Text Block) "Profit: +0"
   └─ TopRight (Vertical Box)
      ├─ SpeedText (Text Block) "Speed: 0"
      └─ NearestStationText (Text Block) "Nearest: 10km"
   ```

3. **Event Tick**:
   ```blueprint
   Event Tick
   ├─ Get Owning Player Pawn
   ├─ Get Component (PlayerTraderComponent)
   ├─ Get Credits → Update CreditsText
   ├─ Get Component (CargoComponent)
   ├─ Get Used/Available Space → Update CargoText
   ├─ Get Profit → Update ProfitText (green if positive)
   └─ Get Velocity → Update SpeedText
   ```

4. **Compile and save**

---

## Part 6: Setup Game Mode

### Step 8: Create BP_TradingGameMode

1. **Create Blueprint**:
   - Right-click → Blueprint Class → GameModeBase
   - Name: `BP_TradingGameMode`

2. **Configure**:
   ```
   Default Pawn Class: BP_TradingShip
   HUD Class: (leave default, we'll add HUD in PlayerController)
   ```

3. **Save**

---

### Step 9: Create BP_TradingPlayerController

1. **Create Blueprint**:
   - Right-click → Blueprint Class → PlayerController
   - Name: `BP_TradingPlayerController`

2. **Event BeginPlay**:
   ```blueprint
   Event BeginPlay
   ├─ Create Widget (WBP_TradingHUD)
   ├─ Add to Viewport
   └─ Store reference for later
   ```

3. **Update BP_TradingGameMode**:
   ```
   Player Controller Class: BP_TradingPlayerController
   ```

4. **Save both**

---

## Part 7: Create Test Level

### Step 10: Setup L_TradingTest

1. **Create Level**:
   - File → New Level → Empty Level
   - Save as: `Content/Maps/L_TradingTest`

2. **Add Basic Components**:
   - DirectionalLight (sunlight)
   - SkyAtmosphere
   - PostProcessVolume (Infinite Extent = ✓)
   - ExponentialHeightFog (optional atmosphere)

3. **Place Stations**:
   ```
   Station 1 (Agricultural):
     - Location: (0, 0, 0)
     - Actor: BP_TradingStation
     - MarketData: DA_Market_AgriculturalStationAlpha
     - StationName: "Ag Station Alpha"

   Station 2 (Industrial):
     - Location: (10000, 0, 0)
     - Actor: BP_TradingStation
     - MarketData: DA_Market_IndustrialStationBeta
     - StationName: "Industrial Beta"
   ```

4. **Add Player Start**:
   - Location: (-1000, 0, 0) [near Station 1]
   - Rotation: facing Station 1

5. **Set World Settings**:
   - World Settings → GameMode Override: BP_TradingGameMode

6. **Save level**

---

## Part 8: Testing

### Step 11: First Playtest

1. **Launch Game** (PIE)

2. **Verify Basic Systems**:
   - [ ] Ship spawns and responds to WASD controls
   - [ ] HUD displays credits (1000)
   - [ ] HUD displays cargo (0/10)
   - [ ] Can fly toward station

3. **Test Docking**:
   - [ ] Fly into station trigger box
   - [ ] "Docked at..." message appears
   - [ ] Trading UI opens
   - [ ] Station name displays

4. **Test Buying**:
   - [ ] Items list shows with prices
   - [ ] Select Water, quantity 5
   - [ ] Click BUY
   - [ ] Credits decrease (1000 → 950)
   - [ ] Cargo increases (0/10 → 5/10)

5. **Test Flying to Station 2**:
   - [ ] Close trading UI
   - [ ] Fly to second station (10km away)
   - [ ] Takes 1-2 minutes at normal speed
   - [ ] Can dock at second station

6. **Test Selling**:
   - [ ] Dock at Station 2
   - [ ] Trading UI opens
   - [ ] Water shows higher sell price
   - [ ] Select Water, quantity 5
   - [ ] Click SELL
   - [ ] Credits increase (950 → 1010)
   - [ ] Cargo decreases (5/10 → 0/10)
   - [ ] Profit displays (+10)

7. **Expected Results**:
   - ✅ Complete trade loop in 3-5 minutes
   - ✅ Made profit on trade
   - ✅ No crashes or errors

---

## Part 9: Iteration

### Step 12: Balance and Polish

**If tests pass, proceed with**:

1. **Add more stations** (place at varying distances)
2. **Add more items** (expand profitable routes)
3. **Tune prices** (ensure 30-50% profit margins)
4. **Add visual feedback**:
   - Purchase confirmation sounds
   - Credit number animations
   - Cargo visual indicator

**If tests fail**:
- Check component references (null checks)
- Verify Data Asset properties
- Check Blueprint compile errors
- Review logs for errors

---

## Common Issues

### Issue: Trading UI doesn't open
**Fix**: Check DockingTrigger collision settings, ensure "Generate Overlap Events" is enabled

### Issue: Can't buy items
**Fix**: Verify PlayerTraderComponent and CargoComponent are on ship actor

### Issue: Prices are 0
**Fix**: Ensure EconomyManager registered the markets in BeginPlay

### Issue: Cargo space not enforced
**Fix**: Check CargoComponent HasSpaceFor logic in BuyItem function

---

## Next Steps

After MVP prototype works:

1. **Convert to Data Assets** (Phase 2)
2. **Add more content** (items, stations)
3. **Implement ship upgrades**
4. **Add tutorial system**
5. **Polish UI and VFX**
6. **Playtest with external testers**

---

**Success Criteria**: Complete one buy-sell-profit cycle in under 5 minutes with no errors.

**Ready to proceed to Phase 2 after**: 60%+ playtesters say "this is fun" (Week 4 gate)

---

**Last Updated**: 2025-12-25
**Status**: Implementation Guide for Phase 1
**Next**: Blueprint creation and testing
