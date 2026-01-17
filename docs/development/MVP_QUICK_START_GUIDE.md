# Trade Simulator MVP - Quick Start Guide

**Last Updated**: January 17, 2026  
**Status**: Active Development - Week 1-4 Prototype Phase  
**Goal**: Playable trading demo in 4 weeks

---

## 🎯 Overview

This guide gets you from zero to a playable trading prototype in **2-4 weeks**. Focus is on **speed over perfection** - we'll structure and polish after validating the core loop is fun.

**Core Loop**: Dock → Buy low → Fly → Dock → Sell high → Profit → Upgrade → Repeat

---

## 📦 What's Included (Ready to Use)

The following MVP trading content has been **pre-generated** and is ready for implementation:

### ✅ 15 Trade Items (Spanning 5 Categories)

Located: `Content/DataAssets/Trading/MVP/Items/`

**Food & Consumables** (3 items):
- Water Purified - 10 credits (basic necessity)
- Protein Packs - 25 credits (standard rations)
- Luxury Food - 100 credits (high-end market)

**Industrial** (3 items):
- Basic Alloys - 50 credits (construction materials)
- Refined Metals - 150 credits (manufacturing)
- Ship Components - 300 credits (maintenance parts)

**Technology** (5 items):
- Basic Computers - 500 credits
- Advanced Electronics - 1,200 credits
- AI Cores - 5,000 credits (rare, high-value)
- Hydrogen Fuel - 75 credits
- Antimatter Cores - 10,000 credits (endgame)

**Medical** (2 items):
- Basic Medicine - 200 credits
- Advanced Medicine - 800 credits

**Luxury** (2 items):
- Entertainment Systems - 600 credits
- Rare Art - 3,000 credits

### ✅ 5 Market Configurations (Different Economic Profiles)

Located: `Content/DataAssets/Trading/MVP/Markets/`

1. **Agricultural Station** - Produces food (cheap), needs tech (expensive)
2. **Industrial Station** - Produces metals (cheap), needs supplies (expensive)
3. **Research Station** - Produces tech (moderate), needs materials (expensive)
4. **Luxury Bazaar** - Everything expensive, rare items available
5. **Trade Hub** - Balanced prices, high variety, central location

---

## 🚀 Phase 1: Hardcoded Prototype (Week 1-4)

### Step 1: Generate Trading Content (5 minutes)

```bash
# From project root
cd /path/to/Adastrea

# Generate all MVP trading content
python3 Tools/generate_mvp_trading_content.py --all

# Or generate specific content
python3 Tools/generate_mvp_trading_content.py --items    # Trade items only
python3 Tools/generate_mvp_trading_content.py --markets  # Markets only
```

**Output**: 
- 15 trade item configurations in `Content/DataAssets/Trading/MVP/Items/`
- 5 market configurations in `Content/DataAssets/Trading/MVP/Markets/`

### Step 2: Create Simple Test Level (10 minutes)

**In Unreal Editor**:

1. Create new level: `Maps/MVP_TradingTest.umap`
2. Add lighting: Directional Light + Sky Sphere
3. Place 2 stations:
   - `BP_SpaceStation` at (0, 0, 0) - Agricultural Station
   - `BP_SpaceStation` at (50000, 0, 0) - Industrial Station
   - Distance: ~50km (about 1 minute flight time)
4. Add player start: PlayerStart actor at (-5000, 0, 1000)

### Step 3: Configure Stations with Marketplaces (15 minutes)

**Station A - Agricultural (Cheap Food, Expensive Tech)**:

1. Open `BP_SpaceStation` instance (Station A)
2. In Details Panel → Station → Configuration:
   - Set `Station Name` = "Agricultural Station Alpha"
3. Add Marketplace Module:
   - In `DefaultModuleClasses` array, add:
     - `BP_SpaceStationModule_DockingBay` (for docking)
     - `BP_SpaceStationModule_Marketplace` (for trading)
4. Configure Marketplace:
   - Create new `DA_Market_AgriculturalStationMarket` Data Asset
   - Copy values from `Content/DataAssets/Trading/MVP/Markets/DA_Market_AgriculturalStationMarket.uasset.txt`
   - Key settings:
     - Sell Markup: 1.1 (10% markup)
     - Buy Markdown: 0.9 (90% buyback)
   - Inventory: Add items from generated list (use generated .txt as reference)

**Station B - Industrial (Cheap Metals, Expensive Food)**:

1. Repeat process for Station B
2. Use `DA_Market_IndustrialStationMarket` configuration
3. Key differences:
   - Sell Markup: 1.3 (30% markup - more expensive)
   - Buy Markdown: 0.7 (70% buyback)
   - Inventory favors industrial goods over food

### Step 4: Create Player Ship Blueprint (20 minutes)

**Option A: Use Existing BP_Spaceship**

1. Open `BP_Spaceship` (or create `BP_TradingShip_Starter`)
2. Add Components:
   - `CargoComponent` (set Cargo Capacity = 10)
   - `PlayerTraderComponent` (handles buying/selling)
3. Class Defaults:
   ```
   CargoCapacity: 10.0
   StartingCredits: 1000
   ```

**Option B: Hardcode for Speed (Recommended for Week 1-2)**

Create simple Blueprint with:
- Float variable: `PlayerCredits` = 1000
- Map variable: `CargoInventory` (ItemName → Quantity)
- Float: `CargoCapacity` = 10.0

### Step 5: Create Trading UI (30 minutes)

**Create Widget Blueprint: `WBP_TradingUI`**

**Required UI Elements**:
- **Header**: Station name, player credits
- **Item List** (ScrollBox):
  - Item name
  - Current price (Buy / Sell)
  - Player's cargo quantity
  - Available stock
  - Buy button (+1, +10, +Max)
  - Sell button (-1, -10, -All)
- **Cargo Display**:
  - Cargo space used / total
  - Progress bar visualization
- **Close button**

**Functionality (Blueprint)**:
```
Event: Buy Button Clicked
  → Get Item Price (market data)
  → Check: Player has credits?
  → Check: Player has cargo space?
  → Deduct credits
  → Add to cargo
  → Update UI

Event: Sell Button Clicked
  → Get Item Price (market data)
  → Check: Player has item in cargo?
  → Add credits
  → Remove from cargo
  → Update UI
```

### Step 6: Implement Docking & Trading (20 minutes)

**In Player Ship Blueprint**:

```
Event: Player Presses 'T' (Trade key)
  → Get Overlapping Actors (sphere trace)
  → Filter: Type = SpaceStation
  → Branch: Found Station?
     ├─ True:
     │   → Get Station's Marketplace Module
     │   → Branch: Has Marketplace?
     │      ├─ True: Open Trading UI
     │      └─ False: Show "No marketplace available"
     └─ False: Show "Not near station"
```

**Docking Logic** (simplified for prototype):
- Proximity check: within 5000 units
- No complex docking animation (add in Phase 2)
- Just open UI when close enough

### Step 7: Test & Iterate (15 minutes)

**Test Checklist**:
- [ ] Can spawn into level
- [ ] Can fly to both stations
- [ ] Can open trading UI at Station A
- [ ] Can buy Water (cheap at A)
- [ ] Cargo space decreases
- [ ] Credits decrease
- [ ] Can fly to Station B
- [ ] Can sell Water (expensive at B)
- [ ] Credits increase (profit!)
- [ ] Can repeat loop 3+ times

**Target Metrics**:
- Complete loop time: 3-5 minutes
- Profit per loop: 100-300 credits (10-30%)
- 10 minutes of gameplay = 2-3 loops without crashes

---

## 🧪 Week 4 Playtest (GO/NO-GO Decision)

### Playtest Setup

**Participants**: 5-10 external testers (not team members)

**Instructions** (minimal):
1. "You're a space trader"
2. "Try to make money"
3. (Give no other guidance)

**Watch silently**:
- Time to first trade
- Confusion points
- Do they understand profit?
- Do they repeat the loop?

**Post-Play Questions**:
1. "Was this fun?" (target: 60%+ yes)
2. "Would you play more?" (target: 40%+ yes)
3. "What confused you?"
4. "What felt good?"

### GO/NO-GO Criteria

**✅ GO to Phase 2** if:
- 60%+ say "this is fun"
- 70%+ complete at least 1 full loop
- Core loop is clear
- No critical crashes

**❌ NO-GO** if:
- <60% fun rating
- Confusion about basic mechanics
- Loop unclear even after 10 minutes

**If NO-GO**: Iterate prototype, retest in 1 week

---

## 📊 Phase 2: Structured Version (Week 5-8)

Once prototype validates fun, convert to Data Assets:

### Convert Hardcoded Values

**Replace**:
```cpp
float WaterPrice_StationA = 8.0f;
```

**With**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly)
UTradeItemDataAsset* WaterItem;

float Price = MarketData->GetItemPrice(WaterItem, true);
```

### Add Features
- Ship upgrade system (increase cargo capacity)
- 5-10 stations (use generated market configs)
- 10-20 trade goods (use generated items)
- Supply/demand dynamics
- Basic economy simulation

### Success Criteria (Week 8)
- 10-15 minutes of varied gameplay
- Multiple profitable trade routes
- Progression feels rewarding
- 70%+ playtesters say "fun"

---

## 🎨 Phase 3: Polish & Demo (Week 9-12)

### Content Expansion
- 3 ships with clear progression:
  - Starter (10 cargo)
  - Medium (30 cargo) - costs 50,000 credits
  - Advanced (100 cargo) - costs 250,000 credits
- 10 stations with different economies
- 20 trade goods with variety
- 5-10 upgrade options

### Polish
- UI improvements (icons, animations)
- VFX for docking, purchases
- Sound effects (ambience, confirmations)
- Balance adjustments
- Tutorial/instructions

### 30-Minute Demo
- Clear progression arc
- Multiple trade routes
- Ship upgrade achievable
- No crashes or major bugs
- "Juice" - satisfying feedback

### Week 12 Validation (Final Gate)

**Metrics** (20+ playtesters):
- 75%+ "had fun"
- 50%+ "would buy"
- 90%+ complete full 30 minutes
- 60%+ "would play again"

**If metrics met**: Ready for publisher pitch  
**If not met**: Additional polish iteration needed

---

## 🛠️ Development Tools & Resources

### Content Generation
```bash
# Generate all MVP content
python3 Tools/generate_mvp_trading_content.py --all

# View generated configurations
ls Content/DataAssets/Trading/MVP/Items/
ls Content/DataAssets/Trading/MVP/Markets/
```

### YAML Templates

**Source Templates**:
- `Assets/TradingTemplates/MVPTradeItems.yaml` - 15 trade items
- `Assets/TradingTemplates/MVPMarkets.yaml` - 5 market configs

**Customize**: Edit YAML, regenerate with script

### Code References

**Key Classes**:
- `UTradeItemDataAsset` - Item definitions
- `UMarketDataAsset` - Market configuration
- `UCargoComponent` - Ship cargo management
- `AMarketplaceModule` - Station trading facility

**Example Usage**:
```cpp
// Check if player can buy item
UCargoComponent* Cargo = Ship->FindComponentByClass<UCargoComponent>();
bool bHasSpace = Cargo->HasSpaceFor(TradeItem, Quantity);
bool bHasCredits = PlayerCredits >= (Price * Quantity);

if (bHasSpace && bHasCredits)
{
    Cargo->AddCargo(TradeItem, Quantity);
    PlayerCredits -= (Price * Quantity);
}
```

---

## 🎯 Success Mantras

**For Weeks 1-4**:
- ✅ "Make it playable, not perfect"
- ✅ "Hardcode first, structure later"
- ✅ "Test every 2 days minimum"
- ❌ "Don't over-engineer"
- ❌ "Don't add features not in loop"

**Key Question**:
> "Is this critical for validating the core trading loop is fun?"
> 
> If NO → defer it until after validation

---

## 📞 Getting Help

**Issues with setup?**
1. Check generated files exist in `Content/DataAssets/Trading/MVP/`
2. Verify Python script ran successfully
3. Review example configurations in generated .txt files

**Blueprint questions?**
1. See `.github/instructions/blueprint-documentation.instructions.md`
2. Use image-based documentation standard
3. Check existing trading system classes for examples

**Need to modify content?**
1. Edit YAML templates in `Assets/TradingTemplates/`
2. Re-run generation script
3. Update Data Assets in Unreal Editor

---

## ✅ Checklist: Week 1 Goals

Use this checklist to track progress:

**Setup** (Day 1-2):
- [ ] Generated MVP trading content (items + markets)
- [ ] Created test level with 2 stations
- [ ] Configured marketplace modules on stations
- [ ] Set up player ship with cargo component

**Core Loop** (Day 3-4):
- [ ] Trading UI displays items and prices
- [ ] Can buy items (deducts credits, adds cargo)
- [ ] Can sell items (adds credits, removes cargo)
- [ ] Cargo capacity enforced

**Integration** (Day 5-6):
- [ ] Can dock at stations (proximity check)
- [ ] Trading UI opens on interaction
- [ ] Can fly between stations in ~1 minute
- [ ] Profit calculation works

**Testing** (Day 7):
- [ ] Complete 3 full trade loops without crashes
- [ ] Profit visible and accumulating
- [ ] 10-minute playtest successful
- [ ] Ready for external feedback

---

**Remember**: The goal is **playable in 4 weeks**, not perfect. Build the minimum to test if trading is fun. Everything else comes after validation.

**One polished gameplay loop beats ten half-finished systems.**
