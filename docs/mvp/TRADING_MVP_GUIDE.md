# Trading MVP Implementation Guide

**Target**: 12-Week Trade Simulator MVP  
**Phase**: Implementation Roadmap  
**Last Updated**: 2025-12-25

---

## 🎯 Quick Start

**Goal**: Build playable trading game validating "buy low, sell high" is fun.

**Success Criteria**:
- Week 4: 60%+ playtesters say "fun" → GO to Phase 2
- Week 12: 75%+ "fun", 50%+ "would buy" → Ready for funding

**This Guide**: Step-by-step implementation for developers

---

## Table of Contents

- [Phase 1: Hardcoded Prototype (Weeks 1-4)](#phase-1-hardcoded-prototype-weeks-1-4)
- [Phase 2: Structured Version (Weeks 5-8)](#phase-2-structured-version-weeks-5-8)
- [Phase 3: Polish & Demo (Weeks 9-12)](#phase-3-polish--demo-weeks-9-12)
- [Testing Checklist](#testing-checklist)
- [Common Issues](#common-issues)

---

## Phase 1: Hardcoded Prototype (Weeks 1-4)

### Week 1: Basic Infrastructure

#### Day 1-2: Project Setup

**Verify Existing Code**:
```bash
# Check trading files exist
ls Source/Adastrea/Trading/

# Expected output:
# TradeItemDataAsset.h/.cpp
# MarketDataAsset.h/.cpp
# TradeTransaction.h/.cpp
# AITraderComponent.h/.cpp
```

**Create Blueprint Folders**:
```
Content/
├── Blueprints/
│   ├── Trading/
│   │   ├── Stations/
│   │   ├── Ships/
│   │   └── UI/
│   └── MVP/
```

#### Day 3-5: Simple Trading Station

**Create `BP_SimpleTradingStation`**:

1. **Create Blueprint** (Content/Blueprints/Trading/Stations/):
   - Parent Class: `Actor`
   - Name: `BP_SimpleTradingStation`

2. **Add Components**:
   ```
   - StaticMesh (station model or placeholder)
   - BoxCollision (docking trigger)
   - TextRender (station name above station)
   ```

3. **Configure Docking Trigger**:
   - BoxCollision size: 500x500x500 (adjust for ship size)
   - Collision Preset: `Overlap All Dynamic`
   - Generate Overlap Events: ✅ True

4. **Add Blueprint Variables**:
   ```blueprint
   StationName: String = "Trading Station Alpha"
   
   // Hardcoded item prices (Station A)
   WaterPrice: Float = 8.0
   FoodPrice: Float = 30.0
   ElectronicsPrice: Float = 120.0
   
   // Hardcoded stock levels
   WaterStock: Integer = 1000
   FoodStock: Integer = 500
   ElectronicsStock: Integer = 100
   ```

5. **Implement Docking**:
   ```blueprint
   Event: OnComponentBeginOverlap (BoxCollision)
   ├─ Cast to PlayerShip
   ├─ If Successful:
   │  ├─ Print String: "Docked at [StationName]"
   │  └─ Call: OpenTradingUI
   ```

**Create Second Station** (different prices):
- Duplicate `BP_SimpleTradingStation`
- Rename to `BP_SimpleTradingStation_B`
- Change prices:
  ```
  WaterPrice = 12.0   (higher than Station A)
  FoodPrice = 20.0    (lower than Station A)
  ElectronicsPrice = 80.0  (lower than Station A)
  ```

#### Day 6-7: Simple Player Ship

**Create `BP_SimplePlayerShip`**:

1. **Create Blueprint**:
   - Parent Class: `Pawn`
   - Name: `BP_SimplePlayerShip`

2. **Add Components**:
   ```
   - StaticMesh (ship model)
   - FloatingPawnMovement (for simple flight)
   - Camera
   - SpringArm (camera boom)
   ```

3. **Add Variables**:
   ```blueprint
   Credits: Integer = 1000
   CargoCapacity: Integer = 10
   CargoHold: Map<String, Integer>  // ItemName -> Quantity
   ```

4. **Add Movement Input**:
   ```blueprint
   InputAxis MoveForward
   ├─ Add Movement Input (World Direction)
   
   InputAxis MoveRight
   ├─ Add Movement Input (World Direction)
   
   InputAxis MoveUp
   ├─ Add Movement Input (World Direction)
   ```

### Week 2: Trading UI

#### Create `WBP_SimpleTradingUI`

**Widget Structure**:
```
Canvas Panel
├── Background (Border)
├── Station Name (Text)
├── Player Credits (Text)
├── Cargo Space (Text: "5/10")
├── Item List (Vertical Box)
│   ├── WBP_TradeItemRow (Water)
│   ├── WBP_TradeItemRow (Food)
│   └── WBP_TradeItemRow (Electronics)
└── Close Button
```

**Create `WBP_TradeItemRow` Widget**:
```
Horizontal Box
├── Item Name (Text)
├── Item Price (Text)
├── Stock Level (Text)
├── Quantity Selector (Spin Box)
├── Buy Button
└── Sell Button
```

**Blueprint Logic** (`WBP_SimpleTradingUI`):

```blueprint
Event Construct
├─ Get Player Ship Reference
├─ Get Current Station Reference
├─ Populate Item List
└─ Update UI Display

Function: OnBuyButtonClicked(ItemName, Quantity, PricePerUnit)
├─ Calculate Total Cost = Quantity * PricePerUnit
├─ Check: PlayerCredits >= TotalCost?
│  ├─ Yes:
│  │  ├─ Check: CargoSpace Available?
│  │  │  ├─ Yes:
│  │  │  │  ├─ Deduct Credits
│  │  │  │  ├─ Add to Cargo
│  │  │  │  ├─ Update UI
│  │  │  │  └─ Play Success Sound
│  │  │  └─ No: Print "Insufficient Cargo Space"
│  └─ No: Print "Insufficient Funds"

Function: OnSellButtonClicked(ItemName, Quantity, PricePerUnit)
├─ Check: Has Item in Cargo?
│  ├─ Yes:
│  │  ├─ Add Credits
│  │  ├─ Remove from Cargo
│  │  ├─ Update UI
│  │  └─ Play Success Sound
│  └─ No: Print "Don't have that item"
```

**HUD Integration**:

```blueprint
// In BP_SimplePlayerShip

Function: OpenTradingUI(Station)
├─ Create Widget: WBP_SimpleTradingUI
├─ Set CurrentStation = Station
├─ Add to Viewport
├─ Set Input Mode: UI Only
└─ Show Mouse Cursor

Function: CloseTradingUI
├─ Remove Widget from Viewport
├─ Set Input Mode: Game Only
└─ Hide Mouse Cursor
```

### Week 3: Core Gameplay Loop

#### Implement Full Trading Cycle

**Test Level Setup** (`L_SimpleTradingTest`):

1. Place 2 stations far apart (10,000 units)
2. Add Player Start near Station A
3. Set GameMode to use `BP_SimplePlayerShip`

**Profit Tracking**:

```blueprint
// In BP_SimplePlayerShip

Variable: StartingCredits = 1000
Variable: TotalProfit = 0

Function: CalculateProfit
├─ TotalProfit = Credits - StartingCredits
└─ Return TotalProfit

Event Tick
├─ Update HUD with Profit
```

**Basic HUD** (`WBP_SimpleHUD`):

```
Canvas Panel
├── Credits Display (Text)
├── Cargo Space Display (Text)
├── Profit Display (Text, Green if positive)
└── Nearest Station Distance (Text)
```

#### Balance Testing

**Initial Prices** (ensure profitability):

| Item | Station A (Buy) | Station B (Sell) | Profit/Unit |
|------|----------------|------------------|-------------|
| Water | 8 credits | 12 credits | 4 credits (50%) |
| Food | 30 credits | 20 credits | -10 credits (LOSS) |
| Electronics | 120 credits | 80 credits | -40 credits (LOSS) |

**Reverse Route**:

| Item | Station B (Buy) | Station A (Sell) | Profit/Unit |
|------|----------------|------------------|-------------|
| Water | 12 credits | 8 credits | -4 credits (LOSS) |
| Food | 20 credits | 30 credits | 10 credits (50%) |
| Electronics | 80 credits | 120 credits | 40 credits (50%) |

**Cargo Example**:
```
Player with 1000 credits, 10 cargo space

Route 1: Station A → Station B
- Buy 10 Water @ 8 = 80 credits
- Sell 10 Water @ 12 = 120 credits
- Profit: 40 credits (50%)
- Time: 3 minutes
- Profit per minute: 13.3 credits/min

Route 2: Station B → Station A
- Buy 5 Electronics @ 80 = 400 credits
- Sell 5 Electronics @ 120 = 600 credits
- Profit: 200 credits (50%)
- Time: 3 minutes
- Profit per minute: 66.7 credits/min
```

### Week 4: Playtest & Validation

#### Prepare Playtest Build

**Checklist**:
- [ ] 2 stations with different prices
- [ ] 3 trade items with clear profit opportunities
- [ ] Starting credits allow at least 5 trades
- [ ] Flight time between stations: 1-2 minutes
- [ ] No crashes in 10 minute session
- [ ] UI shows all necessary information
- [ ] Profit counter visible and accurate

**Playtest Instructions**:

```
Instructions for Playtesters:

1. You start with 1000 credits
2. Fly to the green station and dock (fly into the box)
3. Buy some items (try different items)
4. Fly to the red station
5. Sell your items
6. Did you make a profit?
7. Try to earn as much as possible in 10 minutes

Questions:
- Was this fun? (Yes/No)
- Would you play more? (Yes/No)
- What confused you?
- What felt good?
```

#### Week 4 GO/NO-GO Decision

**Success Metrics**:
- ✅ 60%+ say "fun" → **GO** to Phase 2
- ✅ 40%+ say "would play more"
- ✅ <3 major confusion points
- ✅ 80%+ complete at least one trade loop

**If Metrics NOT Met**:
- Analyze feedback
- Identify core issue (tedious? confusing? not rewarding?)
- Pivot or iterate
- Retest in 1-2 weeks

**If GO Decision**:
- Proceed to Phase 2
- Document what worked
- Plan Data Asset conversion

---

## Phase 2: Structured Version (Weeks 5-8)

### Week 5: Data Asset Foundation

#### Create Trade Item Data Assets

**Step 1: Create Blueprint Folder**:
```
Content/DataAssets/Trading/Items/
```

**Step 2: Create First Item**:

1. Right-click in folder → Blueprint Class
2. Parent Class: `TradeItemDataAsset`
3. Name: `DA_TradeItem_Water`

**Step 3: Configure Properties**:

```
Basic Info:
- Item Name: "Water"
- Description: "Essential liquid for survival"
- Item ID: "Water"
- Category: Food & Consumables

Pricing:
- Base Price: 10.0
- Price Volatility:
  - Volatility Multiplier: 1.0
  - Min Price Deviation: 0.5 (50% of base)
  - Max Price Deviation: 2.0 (200% of base)
- Affected By Supply Demand: ✅ True
- Affected By Market Events: ☐ False (defer)

Trade Properties:
- Volume Per Unit: 1.0
- Mass Per Unit: 1.0
- Standard Lot Size: 10
- Typical Market Stock: 1000
- Replenishment Rate: 100

AI Behavior (defer most):
- AI Trade Priority: 5
- AI Hoardable: ☐ False
- AI Arbitrage Enabled: ☐ False

Legality (defer):
- Leave at defaults
```

**Step 4: Create More Items**:

Repeat for each item (10-20 items total):

**Food Category**:
- `DA_TradeItem_Water` (10 credits, 1 volume)
- `DA_TradeItem_ProteinPacks` (25 credits, 1 volume)
- `DA_TradeItem_LuxuryFood` (100 credits, 2 volume)

**Technology Category**:
- `DA_TradeItem_BasicComputer` (500 credits, 5 volume)
- `DA_TradeItem_AdvancedAICore` (5000 credits, 3 volume)

**Raw Materials**:
- `DA_TradeItem_IronOre` (15 credits, 2 volume)
- `DA_TradeItem_RareMetals` (200 credits, 1 volume)

**Luxury Category**:
- `DA_TradeItem_Art` (1000 credits, 3 volume)
- `DA_TradeItem_AlienArtifacts` (10000 credits, 2 volume)

#### Create Market Data Assets

**Step 1: Create Folder**:
```
Content/DataAssets/Trading/Markets/
```

**Step 2: Create First Market**:

1. Blueprint Class → Parent: `MarketDataAsset`
2. Name: `DA_Market_AgriculturalStation`

**Step 3: Configure Market**:

```
Market Info:
- Market Name: "Agricultural Station Alpha"
- Description: "Food production facility"
- Market ID: "AgStation_01"
- Market Type: Open Market
- Market Size: Medium Station
- Controlling Faction: (leave null for MVP)

Market Config:
- Transaction Tax Rate: 0.05 (5%)
- Sell Price Markup: 1.2 (station sells to player at 20% markup)
- Buy Price Markdown: 0.8 (station buys from player at 20% discount)
- Allow Player Buying: ✅ True
- Allow Player Selling: ✅ True
- Allow AI Traders: ☐ False (defer)
- Min Reputation Required: -100 (anyone can trade)

Inventory:
Add entries for each item (click + button):

Entry 1:
- Trade Item: DA_TradeItem_Water
- Current Stock: 1000
- Max Stock: 2000
- Supply Level: 1.5 (produces water, high supply)
- Demand Level: 0.5 (doesn't need water, low demand)

Entry 2:
- Trade Item: DA_TradeItem_ProteinPacks
- Current Stock: 500
- Max Stock: 1000
- Supply Level: 1.2
- Demand Level: 0.8

Entry 3:
- Trade Item: DA_TradeItem_BasicComputer
- Current Stock: 50
- Max Stock: 100
- Supply Level: 0.3 (doesn't produce tech, low supply)
- Demand Level: 1.8 (needs tech, high demand)

Stock Refresh Rate: 24.0 (refresh once per day)
```

**Step 4: Create Opposite Economy**:

Create `DA_Market_IndustrialStation`:
- Produces: Technology, Raw Materials
- Imports: Food, Water
- Water: Low supply (0.5), High demand (1.5)
- BasicComputer: High supply (1.5), Low demand (0.5)

**Result**: Clear arbitrage opportunities between stations.

### Week 6: Economy Manager

#### Create Economy Manager Subsystem

**Create C++ Class** (if not exists):

`Source/Adastrea/Trading/EconomyManager.h`:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EconomyManager.generated.h"

class UMarketDataAsset;
class UTradeItemDataAsset;

UCLASS()
class ADASTREA_API UEconomyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // All markets in game
    UPROPERTY(BlueprintReadWrite, Category="Economy")
    TArray<UMarketDataAsset*> ActiveMarkets;

    // Game time tracking (hours)
    UPROPERTY(BlueprintReadWrite, Category="Economy")
    float CurrentGameTime;

    // Time speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Economy")
    float TimeScale;

    // Update interval (real seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Economy")
    float UpdateInterval;

    /**
     * Update economy state
     * Called periodically via timer
     */
    UFUNCTION(BlueprintCallable, Category="Economy")
    void UpdateEconomy();

    /**
     * Get current price for item at market
     */
    UFUNCTION(BlueprintCallable, Category="Economy")
    float GetItemPrice(UMarketDataAsset* Market, 
                      UTradeItemDataAsset* Item, 
                      bool bIsBuying) const;

    /**
     * Record a player transaction
     * Updates supply/demand accordingly
     */
    UFUNCTION(BlueprintCallable, Category="Economy")
    void RecordTransaction(UMarketDataAsset* Market, 
                          UTradeItemDataAsset* Item, 
                          int32 Quantity, 
                          bool bPlayerBought);

    /**
     * Register a market for economy simulation
     */
    UFUNCTION(BlueprintCallable, Category="Economy")
    void RegisterMarket(UMarketDataAsset* Market);

private:
    FTimerHandle UpdateTimerHandle;
    
    void UpdateMarketPrices(UMarketDataAsset* Market, float DeltaHours);
    void SimulateBackgroundActivity(UMarketDataAsset* Market, float DeltaHours);
};
```

**Implementation** (`EconomyManager.cpp`):
```cpp
#include "EconomyManager.h"
#include "MarketDataAsset.h"
#include "TradeItemDataAsset.h"
#include "TimerManager.h"

void UEconomyManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentGameTime = 0.0f;
    TimeScale = 1.0f;
    UpdateInterval = 5.0f;  // Update every 5 seconds
    
    // Start update timer
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        GameInstance->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UEconomyManager::UpdateEconomy,
            UpdateInterval,
            true  // Loop
        );
    }
}

void UEconomyManager::UpdateEconomy()
{
    // Convert update interval to game time (1 real second = 1 game minute by default)
    float DeltaHours = (UpdateInterval * TimeScale) / 60.0f;
    CurrentGameTime += DeltaHours;
    
    // Update all markets
    for (UMarketDataAsset* Market : ActiveMarkets)
    {
        if (Market)
        {
            UpdateMarketPrices(Market, DeltaHours);
            SimulateBackgroundActivity(Market, DeltaHours);
        }
    }
}

float UEconomyManager::GetItemPrice(UMarketDataAsset* Market, 
                                    UTradeItemDataAsset* Item, 
                                    bool bIsBuying) const
{
    if (!Market || !Item) return 0.0f;
    return Market->GetItemPrice(Item, bIsBuying);
}

void UEconomyManager::RecordTransaction(UMarketDataAsset* Market, 
                                       UTradeItemDataAsset* Item, 
                                       int32 Quantity, 
                                       bool bPlayerBought)
{
    if (!Market || !Item) return;
    
    // Find inventory entry
    for (FMarketInventoryEntry& Entry : Market->Inventory)
    {
        if (Entry.TradeItem == Item)
        {
            if (bPlayerBought)
            {
                // Player bought from station
                Entry.CurrentStock -= Quantity;
                Entry.SupplyLevel *= 0.95f;  // Supply decreased
                Entry.DemandLevel *= 1.05f;  // Demand increased
            }
            else
            {
                // Player sold to station
                Entry.CurrentStock += Quantity;
                Entry.SupplyLevel *= 1.05f;  // Supply increased
                Entry.DemandLevel *= 0.95f;  // Demand decreased
            }
            
            // Clamp values
            Entry.SupplyLevel = FMath::Clamp(Entry.SupplyLevel, 0.1f, 3.0f);
            Entry.DemandLevel = FMath::Clamp(Entry.DemandLevel, 0.1f, 3.0f);
            break;
        }
    }
}

void UEconomyManager::UpdateMarketPrices(UMarketDataAsset* Market, float DeltaHours)
{
    // Gradually return supply/demand to baseline
    const float RecoveryRate = 0.1f * DeltaHours;  // 10% per hour
    
    for (FMarketInventoryEntry& Entry : Market->Inventory)
    {
        Entry.SupplyLevel = FMath::Lerp(Entry.SupplyLevel, 1.0f, RecoveryRate);
        Entry.DemandLevel = FMath::Lerp(Entry.DemandLevel, 1.0f, RecoveryRate);
    }
}

void UEconomyManager::SimulateBackgroundActivity(UMarketDataAsset* Market, float DeltaHours)
{
    // MVP: Simple stock replenishment
    for (FMarketInventoryEntry& Entry : Market->Inventory)
    {
        if (Entry.TradeItem)
        {
            int32 ReplenishAmount = Entry.TradeItem->ReplenishmentRate * DeltaHours;
            Entry.CurrentStock = FMath::Min(Entry.CurrentStock + ReplenishAmount, 
                                           Entry.MaxStock);
        }
    }
}

void UEconomyManager::RegisterMarket(UMarketDataAsset* Market)
{
    if (Market && !ActiveMarkets.Contains(Market))
    {
        ActiveMarkets.Add(Market);
    }
}
```

### Week 7: Integration & Ship Upgrades

#### Update Trading Station Blueprint

**Modify `BP_TradingStation`**:

```blueprint
Variables:
- MarketDataAsset: Object Reference to UMarketDataAsset
- EconomyManager: Object Reference (get from GameInstance)

Event BeginPlay:
├─ Get GameInstance
├─ Get Economy Manager Subsystem
├─ Economy Manager->RegisterMarket(MarketDataAsset)

OnPlayerDocked:
├─ Open Trading UI
└─ Pass MarketDataAsset to UI
```

#### Update Trading UI

**Modify `WBP_TradingUI`**:

```blueprint
Variables:
- MarketDataAsset: Object Reference
- EconomyManager: Object Reference

Function: PopulateItemList
├─ Get Inventory from MarketDataAsset
├─ For Each Inventory Entry:
│  ├─ Get Item Price from EconomyManager
│  ├─ Create Item Row Widget
│  └─ Add to List

Function: OnBuyItem(TradeItem, Quantity)
├─ Get Price from EconomyManager
├─ Calculate Total Cost
├─ If CanAfford and HasCargoSpace:
│  ├─ Deduct Credits
│  ├─ Add to Cargo
│  ├─ EconomyManager->RecordTransaction(Market, Item, Qty, true)
│  └─ Update UI
```

#### Ship Upgrade System

**Create `DA_Ship_Small`** (starter):
```
Ship Name: "Compact Trader"
Cargo Capacity: 10
Speed: Medium
Cost: 0 (starting ship)
```

**Create `DA_Ship_Medium`**:
```
Ship Name: "Merchant Hauler"
Cargo Capacity: 30
Speed: Slow
Cost: 50000 credits
```

**Create `DA_Ship_Large`**:
```
Ship Name: "Trade Liner"
Cargo Capacity: 100
Speed: Fast
Cost: 250000 credits
```

**Ship Upgrade UI** (`WBP_ShipUpgrade`):
```blueprint
Function: CanUpgradeToShip(NewShipData)
├─ Check: PlayerCredits >= NewShipData->Cost
└─ Return bool

Function: UpgradeShip(NewShipData)
├─ If CanUpgradeToShip:
│  ├─ Deduct Credits
│  ├─ Set PlayerShip->ShipData = NewShipData
│  ├─ Update CargoCapacity
│  ├─ Play Celebration VFX
│  └─ Show "Ship Upgraded!" message
```

### Week 8: Testing & Balance

#### Playtest #2 Goals

**Target Metrics**:
- 70%+ "had fun"
- Average 10+ trades per session
- 80%+ explore all available stations
- Clear progression path (see ship upgrade goal)

**Balance Testing**:

1. **Earnings Rate**:
   - Small ship (10 cargo): 100-200 credits/minute
   - Medium ship (30 cargo): 300-500 credits/minute
   - Large ship (100 cargo): 1000+ credits/minute

2. **Progression Time**:
   - Small → Medium: 30-45 minutes
   - Medium → Large: 60-90 minutes
   - Demo length: 30 minutes (reach Medium ship)

3. **Price Balance**:
   - Profit margins: 30-50% per trade
   - Avoid exploits: No items with 200%+ margins
   - Variety: Different routes have different profitability

---

## Phase 3: Polish & Demo (Weeks 9-12)

### Week 9: Content Expansion

**Add More Stations** (10 total):
1. Agricultural Station (produces food)
2. Industrial Station (produces technology)
3. Mining Station (produces raw materials)
4. Luxury Bazaar (high-end goods)
5. Research Hub (advanced technology)
6. Trade Hub (balanced, central location)
7. Military Depot (equipment, restricted)
8. Medical Station (medicine, supplies)
9. Refinery (processed materials)
10. Frontier Outpost (limited stock, high prices)

**Add More Items** (20+ total):
- Diverse price points (10 credits to 10,000 credits)
- Varied cargo volumes (1 to 10 units)
- Clear arbitrage opportunities between stations

**Item Categories**:
- Basic: 5-50 credits (Water, Food, Basic Materials)
- Standard: 50-500 credits (Components, Medicine)
- Advanced: 500-5000 credits (Technology, Equipment)
- Luxury: 5000+ credits (Art, Artifacts, Rare Goods)

### Week 10: Polish & VFX

**Visual Polish**:
- ✨ Docking effects (particles, lights)
- ✨ Purchase confirmation (flash, sound)
- ✨ Credits animation (numbers float up)
- ✨ Ship upgrade celebration (fireworks, fanfare)
- ✨ Cargo hold visualization (progress bar fills)

**Sound Effects**:
- 🔊 Station ambience (hum, chatter)
- 🔊 Docking sounds (clamps, airlock)
- 🔊 Buy/sell confirmation (cash register, beep)
- 🔊 Insufficient funds (error buzz)
- 🔊 Ship upgrade (fanfare)
- 🔊 Background music (relaxing trade music)

**UI Improvements**:
- Better button feedback (hover, press)
- Clearer price comparison (show profit margins)
- Transaction history (recent trades)
- "Best Deal" indicators (highlight profitable items)
- Station comparison tool (price tables)

### Week 11: Tutorial & Onboarding

**Tutorial System** (5 minutes):

```
Step 1: Welcome (30 seconds)
- "Welcome to Adastrea!"
- "You're a trader trying to earn credits"
- "Buy low, sell high, upgrade your ship"

Step 2: First Purchase (1 minute)
- "Dock at this station" (waypoint)
- "Open the trading menu"
- "Buy some Water (it's cheap here)"
- "Watch your credits decrease"

Step 3: First Sale (2 minutes)
- "Fly to the next station" (waypoint)
- "Dock at the new station"
- "Sell your Water (it's expensive here)"
- "You made a profit!"

Step 4: Profit Goal (1 minute)
- "Your goal: Earn 50,000 credits"
- "Then you can upgrade to a bigger ship"
- "Bigger ship = more cargo = more profit"

Step 5: Free Play
- "Now you're on your own!"
- "Explore, trade, earn credits"
- "Have fun!"
```

**Tutorial Implementation**:
```blueprint
// Tutorial Manager Component

Variable: CurrentTutorialStep = 0
Variable: bTutorialActive = true

Function: AdvanceTutorial
├─ CurrentTutorialStep++
├─ Show Next Instruction
└─ If Final Step: bTutorialActive = false

Triggers:
- OnFirstDock → AdvanceTutorial
- OnFirstPurchase → AdvanceTutorial
- OnFirstSale → AdvanceTutorial
- OnProfitThreshold → AdvanceTutorial
```

### Week 12: Demo Package

#### 30-Minute Demo Structure

**Demo Flow**:
```
Minutes 0-5: Tutorial (guided first trade)
├─ Start: 1000 credits, Small ship (10 cargo)
├─ Guided: Buy Water at Ag Station
├─ Guided: Sell Water at Industrial Station
├─ Result: ~1050 credits (learned the loop)

Minutes 5-15: Exploration & Learning
├─ Player discovers different stations
├─ Player learns which items are profitable where
├─ Player makes 5-10 trades
├─ Result: ~5000-10000 credits

Minutes 15-25: Optimization
├─ Player finds best trade routes
├─ Player maximizes profit per trip
├─ Player approaches ship upgrade threshold
├─ Result: ~40000-50000 credits

Minutes 25-30: Climax & Progression
├─ Player unlocks Medium ship (50000 credits)
├─ Player sees power increase (30 cargo vs 10)
├─ Player completes one high-profit run
├─ Result: "I want to keep playing!"
```

**Demo Ending**:
```
After 30 minutes:
- Auto-save game state
- Show statistics:
  - Total Profit Earned
  - Total Trades Completed
  - Best Single Trade
  - Time Played
- "Thanks for playing! Demo ends here"
- "Full game coming soon: More ships, stations, and features"
- Survey link (if applicable)
```

#### Demo Package Checklist

**Content**:
- [ ] 10 stations with varied economies
- [ ] 20+ trade items across price ranges
- [ ] 3 ships (Small, Medium, Large)
- [ ] Tutorial (5 minutes)
- [ ] Demo time limit (30 minutes)

**Polish**:
- [ ] VFX for key moments (docking, purchase, upgrade)
- [ ] Sound effects for all actions
- [ ] Background music
- [ ] Smooth UI animations
- [ ] Clear visual feedback

**Technical**:
- [ ] 60 FPS on mid-range PC
- [ ] 0 crashes in 30 minutes
- [ ] <30 second load time
- [ ] Save/load works
- [ ] No major bugs

**Balance**:
- [ ] Clear progression (Small → Medium in 30 min)
- [ ] Varied trade routes (not all same profit margin)
- [ ] No exploits (no instant-rich schemes)
- [ ] Fun curve (starts slow, ramps up)

---

## Testing Checklist

### Phase 1 Tests (Week 4)

**Functional Tests**:
- [ ] Can dock at Station A
- [ ] Can open trading UI
- [ ] Can buy item (credits decrease)
- [ ] Can add item to cargo
- [ ] Cargo capacity enforced (can't exceed)
- [ ] Can undock from Station A
- [ ] Can fly to Station B
- [ ] Can dock at Station B
- [ ] Can sell item (credits increase)
- [ ] Can remove item from cargo
- [ ] Profit counter accurate

**Playability Tests**:
- [ ] Flight controls responsive
- [ ] Station distance reasonable (1-2 min flight)
- [ ] UI clear and readable
- [ ] Button feedback obvious
- [ ] Can complete loop in 5 minutes

**Validation Tests**:
- [ ] 10-minute session no crashes
- [ ] All prices make sense
- [ ] Profit opportunities clear
- [ ] No confusing elements

### Phase 2 Tests (Week 8)

**Data Asset Tests**:
- [ ] All items have valid prices
- [ ] All items have valid volumes
- [ ] Markets load correctly
- [ ] Inventory populates
- [ ] Price calculation accurate

**Economy Tests**:
- [ ] Prices update after trades
- [ ] Supply/demand changes correctly
- [ ] Prices recover over time
- [ ] Multiple markets independent
- [ ] No price exploits

**Progression Tests**:
- [ ] Can earn credits consistently
- [ ] Ship upgrade unlocks at correct threshold
- [ ] Ship upgrade applies correctly
- [ ] Cargo capacity increases
- [ ] Upgrade feels rewarding

### Phase 3 Tests (Week 12)

**Polish Tests**:
- [ ] All VFX visible and appropriate
- [ ] All sound effects play
- [ ] UI animations smooth
- [ ] No visual glitches
- [ ] Music loops correctly

**Tutorial Tests**:
- [ ] Tutorial completes in 5 minutes
- [ ] All steps clear
- [ ] Can skip tutorial (second playthrough)
- [ ] Tutorial doesn't break game

**Demo Tests**:
- [ ] 30-minute limit works
- [ ] Save/load preserves state
- [ ] Statistics accurate
- [ ] Demo ending displays correctly
- [ ] Can replay demo

**Performance Tests**:
- [ ] 60 FPS in normal play
- [ ] <30 second load time
- [ ] No memory leaks
- [ ] No stuttering during trades

---

## Common Issues

### Issue: Prices Don't Update

**Symptom**: Player trades but prices stay the same

**Cause**: Economy Manager not recording transactions

**Fix**:
```blueprint
// In Trading UI, after successful trade:
Get Economy Manager
Call: RecordTransaction(Market, Item, Quantity, bPlayerBought)
```

### Issue: Cargo Capacity Not Enforced

**Symptom**: Player can buy infinite items

**Cause**: Missing capacity check

**Fix**:
```blueprint
Function: CanBuyItem(Item, Quantity)
├─ CurrentCargoUsed = Calculate Total Cargo Volume
├─ RequiredSpace = Item->VolumePerUnit * Quantity
├─ Return: (CurrentCargoUsed + RequiredSpace) <= CargoCapacity
```

### Issue: Ship Upgrade Doesn't Apply

**Symptom**: Upgrade purchased but cargo capacity unchanged

**Cause**: Not updating ship component

**Fix**:
```blueprint
Function: ApplyShipUpgrade(NewShipData)
├─ CargoCapacity = NewShipData->CargoCapacity
├─ Update UI displays
└─ Rebuild cargo hold UI
```

### Issue: Tutorial Gets Stuck

**Symptom**: Tutorial doesn't advance past step

**Cause**: Missing event trigger

**Fix**:
```blueprint
// Each tutorial step needs clear trigger:
OnDocked → Check: bTutorialActive && CurrentStep == 1
OnPurchased → Check: bTutorialActive && CurrentStep == 2
// etc.
```

### Issue: Demo Time Limit Not Working

**Symptom**: Demo continues past 30 minutes

**Cause**: Timer not started

**Fix**:
```blueprint
Event BeginPlay
├─ Set Timer: 30 minutes (1800 seconds)
├─ On Timer Complete: ShowDemoEndScreen
```

---

## Next Steps

After completing all phases:

1. **Validate Success Metrics** (Week 12)
   - If 75%+ "fun" AND 50%+ "would buy" → Ready for funding
   - If not → Analyze and iterate

2. **Prepare Pitch Materials**
   - Demo video (3-5 minutes)
   - Feature list
   - Roadmap for v1.0
   - Budget and timeline

3. **Post-MVP Planning**
   - What features to add next?
   - Prioritize based on playtester feedback
   - One system at a time

---

## Related Documents

- `docs/systems/TRADING_ARCHITECTURE.md` - System architecture
- `docs/mvp/SYSTEM_CLASSIFICATION.md` - MVP scope
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP mandate

---

**Remember**: Build the simplest thing that's fun. Every feature should answer: "Does this make trading more fun?"

---

**Last Updated**: 2025-12-25  
**Status**: Active Implementation Guide
