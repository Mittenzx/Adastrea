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
- `BP_SpaceStation` with marketplace modules (modular station system)
- `BP_SpaceStationModule_Marketplace` (marketplace configuration)
- `BP_TradingShip` (cargo management)
- `BP_TradingGameMode` (economy updates)
- `WBP_TradingUI` (market interface)

**Add Features:**
- Ship upgrades (cargo capacity)
- Supply/demand dynamics
- Multiple trade routes
- Basic economy simulation
- Marketplace module integration

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
// Space station with marketplace modules
class ASpaceStation : public AActor
{
    // Station automatically spawns modules from this array
    UPROPERTY(EditAnywhere, Category="Station")
    TArray<TSubclassOf<ASpaceStationModule>> DefaultModuleClasses;
    
    UFUNCTION(BlueprintCallable)
    AMarketplaceModule* GetMarketplaceModule() const;
    
    UFUNCTION(BlueprintCallable)
    bool HasMarketplace() const;
};

// Marketplace module (attached to stations)
class AMarketplaceModule : public ASpaceStationModule
{
    // Market configuration (inventory, prices, rules)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMarketDataAsset* MarketDataAsset;
    
    // Trading availability
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOpen;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText MarketplaceName;
    
    UFUNCTION(BlueprintCallable)
    UMarketDataAsset* GetMarketData() const;
    
    UFUNCTION(BlueprintCallable)
    bool IsAvailableForTrading() const;
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
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 CargoVolume; // Space per unit
};

// Market configuration (used by MarketplaceModule)
UCLASS()
class UMarketDataAsset : public UPrimaryDataAsset
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText MarketName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EMarketType MarketType; // Open, Black Market, etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EMarketSize MarketSize; // Small, Medium, Large, etc.
    
    // Pricing modifiers
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SellPriceMarkup; // 1.2 = +20% when player buys
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BuyPriceMarkdown; // 0.8 = -20% when player sells
    
    // Market inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMarketInventoryEntry> Inventory;
    
    UFUNCTION(BlueprintCallable)
    float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;
    
    UFUNCTION(BlueprintCallable)
    bool IsItemInStock(FName ItemID, int32 Quantity) const;
};
```

---

### Marketplace Module System

**Architecture:**
Trading in Adastrea uses a **modular station system**. Markets are implemented as `AMarketplaceModule` actors that attach to space stations.

**Key Concepts:**

1. **MarketplaceModule** = Physical trading facility on station
   - Attached to `ASpaceStation` as child actor
   - Contains reference to `UMarketDataAsset`
   - Can be opened/closed for trading
   - Multiple marketplaces per station supported

2. **MarketDataAsset** = Market configuration (inventory, prices)
   - Defines what items are sold/bought
   - Sets pricing rules (markup/markdown)
   - Manages stock levels and supply/demand
   - Reusable across multiple stations

**Station Configuration:**
```cpp
// In BP_SpaceStation Class Defaults:
DefaultModuleClasses:
  [0] BP_SpaceStationModule_DockingBay   // Required for docking
  [1] BP_SpaceStationModule_Marketplace  // Required for trading
  [2] BP_CargoBayModule                  // Optional storage
```

**Trading Flow:**
```
1. Player docks at station
2. Check: Station->HasMarketplace()
3. Get marketplace: Station->GetMarketplaceModule()
4. Check: Marketplace->IsAvailableForTrading()
5. Get market data: Marketplace->GetMarketData()
6. Open trading UI with market data
```

**Blueprint Example:**
```
Event: Player Requests Trade
    ↓
Get Docked Station
    ↓
Has Marketplace? (bool)
    ├─ True:
    │   ↓
    │   Get Marketplace Module
    │   ↓
    │   Is Available For Trading? (bool)
    │   ├─ True: Open Trading UI
    │   └─ False: Show "Market Closed"
    │
    └─ False: Show "No Marketplace Available"
```

**Creating a Market:**

1. **Create Market Data Asset**:
   - Right-click in Content Browser
   - Data Asset → MarketDataAsset
   - Configure inventory, prices, rules

2. **Create Marketplace Module Blueprint**:
   - Already exists: `BP_SpaceStationModule_Marketplace`
   - In Class Defaults, assign your MarketDataAsset
   - Set `bIsOpen = true`
   - Set `MarketplaceName` (e.g., "Central Market")

3. **Add to Station**:
   - Open `BP_SpaceStation` Class Defaults
   - Add to `DefaultModuleClasses` array
   - Station spawns marketplace automatically on BeginPlay

**Market Types:**
- `OpenMarket` - General trading (MVP default)
- `BlackMarket` - Illegal goods (post-MVP)
- `CommodityExchange` - Bulk trading (post-MVP)
- `LuxuryBazaar` - High-end goods (post-MVP)
- `IndustrialDepot` - Manufacturing supplies (post-MVP)

**MVP Focus:**
For prototype, use `OpenMarket` type with simple inventory. Advanced market types and features (black markets, faction-exclusive markets, dynamic events) are post-MVP.

**Performance Note:**
Each marketplace module consumes 40 power units. Stations automatically calculate total power from all modules via `GetPowerBalance()`.

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

**Station Architecture:**
Each station is an `ASpaceStation` actor with attached modules. For trading, stations MUST have:
- **DockingBayModule** - Required for ship docking
- **MarketplaceModule** - Required for trading functionality
- **CargoBayModule** - Optional (enhances station storage)

**Agricultural Station:**
- Produces: Food, water
- Imports: Technology, machinery
- Economy: Stable prices
- **Marketplace Config**:
  - Market Type: `OpenMarket`
  - Market Size: `Medium`
  - Sell Markup: 1.1x (cheap goods)
  - Buy Markdown: 0.9x (pays less for imports)

**Industrial Station:**
- Produces: Metals, components
- Imports: Food, medicine
- Economy: High volatility
- **Marketplace Config**:
  - Market Type: `IndustrialDepot`
  - Market Size: `Large`
  - Sell Markup: 1.3x (expensive imports)
  - Buy Markdown: 0.7x (cheap exports)

**Luxury Station:**
- Produces: Art, entertainment
- Imports: Luxury food, rare goods
- Economy: Expensive, low volatility
- **Marketplace Config**:
  - Market Type: `LuxuryBazaar`
  - Market Size: `Small`
  - Sell Markup: 2.0x (very expensive)
  - Buy Markdown: 0.8x (reasonable prices for luxury goods)

**Research Station:**
- Produces: Technology, medicine
- Imports: Industrial materials
- Economy: High tech prices
- **Marketplace Config**:
  - Market Type: `ResearchHub`
  - Market Size: `Medium`
  - Sell Markup: 1.5x (tech is expensive)
  - Buy Markdown: 0.75x (needs materials)

**Trade Hub:**
- Produces: Nothing (imports everything)
- Imports: All categories
- Economy: Medium prices, balanced
- **Marketplace Config**:
  - Market Type: `OpenMarket`
  - Market Size: `Large`
  - Sell Markup: 1.2x (fair markup)
  - Buy Markdown: 0.85x (fair prices)

**Station Module Setup (YAML):**
```yaml
name: Industrial Station Alpha
location: (1000, 5000, 0)

modules:
  - type: DockingBay
    count: 2
  - type: Marketplace
    market_data: DA_Market_Industrial
    is_open: true
    name: "Industrial Exchange"
  - type: CargoBay
    count: 3

market_config:
  produces:
    - Metals: 0.8x base price (station produces, cheap here)
    - Components: 0.9x base price
  imports:
    - Food: 1.3x base price (station needs, expensive here)
    - Medicine: 1.5x base price
  
  sell_markup: 1.3  # Player pays 30% more when buying
  buy_markdown: 0.7  # Player gets 30% less when selling
```

**Blueprint Setup:**
```
BP_SpaceStation_Industrial (Class Defaults):
  DefaultModuleClasses:
    [0] BP_SpaceStationModule_DockingBay
    [1] BP_SpaceStationModule_Marketplace
    [2] BP_CargoBayModule
    [3] BP_CargoBayModule
    [4] BP_CargoBayModule

BP_SpaceStationModule_Marketplace (Class Defaults):
  MarketDataAsset: DA_Market_Industrial
  bIsOpen: true
  MarketplaceName: "Industrial Exchange"
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

## 🏪 MARKETPLACE MODULE INTEGRATION

### Implementation Checklist

**Phase 1 (Prototype):**
- [ ] Use existing `AMarketplaceModule` C++ class (already implemented)
- [ ] Create 2-3 `UMarketDataAsset` instances for test stations
- [ ] Verify station spawns marketplace module correctly
- [ ] Test `HasMarketplace()` and `GetMarketplaceModule()` functions
- [ ] Open trading UI when player docks and requests trade

**Phase 2 (Structured):**
- [ ] Create 10+ unique MarketDataAssets for different station types
- [ ] Configure market inventory for each station economy
- [ ] Set appropriate sell markup / buy markdown per market
- [ ] Implement supply/demand tracking in MarketDataAsset
- [ ] Add market events (price fluctuations, shortages)

**Phase 3 (Polish):**
- [ ] Visual indicators for marketplace modules on stations
- [ ] Market status UI (open/closed, stock levels)
- [ ] Market comparison tool (price differences between stations)
- [ ] Trading history tracker
- [ ] Achievement for first trade completion

### Code Examples

**Check Station Has Trading:**
```cpp
// C++ approach
void APlayerShip::RequestTrade(ASpaceStation* DockedStation)
{
    if (!DockedStation || !DockedStation->HasMarketplace())
    {
        ShowMessage(TEXT("This station has no marketplace"));
        return;
    }
    
    AMarketplaceModule* Marketplace = DockedStation->GetMarketplaceModule();
    if (!Marketplace->IsAvailableForTrading())
    {
        ShowMessage(TEXT("Marketplace is currently closed"));
        return;
    }
    
    UMarketDataAsset* MarketData = Marketplace->GetMarketData();
    OpenTradingUI(MarketData);
}
```

**Blueprint Trading Flow:**
```
Event: OnPlayerDocked (Station Reference)
    ↓
Branch: Station->HasMarketplace()
    ├─ False → Show UI: "No Trading Available"
    │
    └─ True:
        ↓
        Get Marketplace Module
        ↓
        Branch: IsAvailableForTrading()
        ├─ False → Show UI: "Market Closed"
        │
        └─ True:
            ↓
            Get Market Data
            ↓
            Open Widget: WBP_TradingUI
            ↓
            Set Market Data Reference
```

**Create Market Data Asset (Editor):**
1. Content Browser → Right-Click
2. Data Asset → MarketDataAsset
3. Name: `DA_Market_Agricultural`
4. Configure properties:
   ```
   Market Name: "Farming Collective Market"
   Market Type: Open Market
   Market Size: Medium
   Sell Price Markup: 1.1 (player pays 10% more)
   Buy Price Markdown: 0.9 (player receives 10% less)
   
   Inventory:
     [0] Trade Item: DA_Item_Water
         Current Stock: 10000
         Max Stock: 50000
         Supply Level: 2.0 (abundant)
         Demand Level: 0.5 (low demand)
     
     [1] Trade Item: DA_Item_Food
         Current Stock: 5000
         Max Stock: 20000
         Supply Level: 1.5 (good supply)
         Demand Level: 0.8 (moderate demand)
     
     [2] Trade Item: DA_Item_Electronics
         Current Stock: 100
         Max Stock: 500
         Supply Level: 0.2 (scarce)
         Demand Level: 2.0 (high demand)
   ```

**Station Blueprint Configuration:**
```
1. Open BP_SpaceStation_Agricultural
2. Class Defaults → Station → Configuration
3. Default Module Classes:
   - Add: BP_SpaceStationModule_DockingBay
   - Add: BP_SpaceStationModule_Marketplace
   - Add: BP_CargoBayModule
4. Compile & Save

5. Open BP_SpaceStationModule_Marketplace (if custom config needed)
6. Class Defaults → Marketplace
7. Set:
   - Market Data Asset: DA_Market_Agricultural
   - Is Open: true
   - Marketplace Name: "Farming Collective Market"
8. Compile & Save
```

### Marketplace Module Properties

**Exposed to Blueprint:**
```cpp
// Read/Write (designer configurable)
MarketDataAsset (UMarketDataAsset*) - Market configuration
bIsOpen (bool) - Trading availability
MarketplaceName (FText) - Display name for UI

// Read-Only (calculated)
ModuleType = "Marketplace"
ModulePower = 40.0f (power consumption)
ModuleGroup = Public
```

**Station Query Functions:**
```cpp
// Check capabilities
bool HasMarketplace() const;
bool HasDockingCapability() const;
bool HasCargoStorage() const;

// Get modules
AMarketplaceModule* GetMarketplaceModule() const;
TArray<AMarketplaceModule*> GetMarketplaceModules() const;
TArray<ASpaceStationModule*> GetModulesByGroup(EStationModuleGroup) const;

// Aggregate data
float GetTotalPowerConsumption() const;
float GetPowerBalance() const;
int32 GetModuleCountByGroup(EStationModuleGroup) const;
```

### Common Issues

**Issue: Trading UI Won't Open**
- Verify: Station has marketplace module in DefaultModuleClasses
- Verify: MarketDataAsset assigned to marketplace module
- Verify: `bIsOpen = true` on marketplace
- Check: Station->HasMarketplace() returns true

**Issue: No Items in Market**
- Check: MarketDataAsset has entries in Inventory array
- Verify: Each inventory entry has valid TradeItem reference
- Check: CurrentStock > 0 for items
- Verify: TradeItemDataAsset exists and is valid

**Issue: Prices Don't Match Expected Values**
- Check: SellPriceMarkup and BuyPriceMarkdown settings
- Formula: Buy Price = BasePrice × SellPriceMarkup
- Formula: Sell Price = BasePrice × BuyPriceMarkdown
- Verify: BasePrice set correctly in TradeItemDataAsset

**Issue: Multiple Markets Interfere**
- Solution: Each MarketplaceModule should have unique MarketDataAsset
- Or: Use same MarketDataAsset if markets should share inventory
- Note: Shared MarketDataAsset = shared stock levels

### Testing Marketplace Modules

**Manual Test Checklist:**
- [ ] Station spawns with marketplace module visible
- [ ] Player can dock at station
- [ ] Trading UI opens when requested
- [ ] Can see market inventory in UI
- [ ] Can buy items (reduces credits, adds cargo)
- [ ] Can sell items (adds credits, removes cargo)
- [ ] Prices match expected calculations
- [ ] Stock levels update after transactions
- [ ] Market closes/opens based on `bIsOpen` flag

**Debug Commands:**
```
// In-game console
stat game          // Show game thread stats
stat scenerendering // Show rendering stats

// Print station info
GetStationInfo <StationName>

// Print market data
GetMarketInfo <StationName>

// Force market refresh
RefreshMarket <StationName>
```

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

**Technical Implementation:**
- Uses modular station architecture with `AMarketplaceModule`
- Market configuration via `UMarketDataAsset`
- Blueprint-friendly C++ components for trading gameplay
- Existing C++ infrastructure ready for MVP implementation
