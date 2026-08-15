# Trade Simulator MVP Overview

**What We're Building | Scope | Success Criteria | Last Updated: August 9, 2026**

---

## 🎯 Mission Statement

Build a **playable, fun trading game in 12 weeks** that validates market interest and provides foundation for funding.

**Core Experience**: Buy low, sell high, upgrade ship, repeat with better margins.

**Success Metric**: 60% of playtesters say "this is fun" after 10 minutes (Week 4).

---

## 📦 Minimum Viable Content

### Phase 1: Hardcoded Prototype (Weeks 1-4) ✅ COMPLETE
**What Was Built:**
- 1 flyable ship (hardcoded stats)
- 2 stations (visual distinction)
- 3 trade goods (different prices)
- Basic trading UI (buy/sell interface)
- Profit counter (track earnings)
- No save/load (session only)

**What Was Hardcoded (OK for prototype):**
```cpp
float GoodPrice_Station1 = 100.0f;
float GoodPrice_Station2 = 150.0f;
int32 CargoSpace = 10;
```

**Success Criteria (Achieved):**
- ✅ Can fly between stations in 1 minute
- ✅ Can buy cargo at Station A
- ✅ Can sell cargo at Station B
- ✅ Can see profit increase
- ✅ Runs for 10 minutes without crashes

---

### Phase 2: Structured Version (Weeks 5-8) ✅ COMPLETE
**Converted to Data Assets:**
- `UTradeItemDataAsset` (15+ goods) ✅
- `UMarketDataAsset` (5 stations) ✅
- `USpaceshipDataAsset` (3 ships) ✅
- Economy config (supply/demand simulation) ✅

**Added Blueprints:**
- `BP_SpaceStation` with marketplace modules ✅
- `BP_SpaceStationModule_Marketplace` ✅
- `BP_TradingShip` (cargo management) ✅
- `WBP_TradingUI` (market interface) - **NEEDS FINAL WIRING**

**Added Features:**
- Ship upgrades (cargo capacity) ✅
- Supply/demand dynamics ✅
- Multiple trade routes ✅
- Basic economy simulation ✅
- Marketplace module integration ✅

---

### Phase 3: Polish & Demo (Weeks 9-12) 🔄 IN PROGRESS (Week 12)
**Content Expansion (Week 9-10) ✅:**
- 3 ships (starter, mid-tier, advanced) ✅
- 10 stations (different economies) - **NEED 5 MORE MARKETS**
- 20 trade goods (variety) - **HAVE 15, NEED 5 MORE**
- 5-10 upgrade options ✅
- Tutorial/instructions - **NEEDS IMPLEMENTATION**

**Polish (Week 11) ✅:**
- UI improvements ✅
- VFX for docking, purchases - **NEEDS IMPLEMENTATION**
- Sound effects - **NEEDS IMPLEMENTATION**
- Balance adjustments ✅
- Bug fixes ✅

**Demo Package (Week 12) 🔄 ACTIVE NOW:**
- 30-minute playable experience
- Clear progression arc
- No crashes or major bugs
- "Juice" - feedback for actions
- Save/load (basic)

---

## 🚫 Explicitly Out of Scope (Do NOT Build)

Until MVP validates market interest, these are **deferred**:

| System | Reason |
|--------|--------|
| Combat system | No weapons, no fights |
| Exploration mechanics | No scanning |
| Crew management | Solo pilot only |
| Quest system | Trading IS the game |
| Faction diplomacy | Beyond price modifiers |
| Multiplayer/networking | Post-MVP |
| Advanced AI | Basic economy only |
| Character customization | Post-MVP |
| Story/narrative | Post-MVP |
| Procedural generation | Handcrafted content |

**Standard Response**: "Great idea! Let's add it AFTER we validate the trading loop is fun."

---

## 💻 Technical Architecture (MVP Scope Only)

### Core Classes (Implemented in C++)

```cpp
// Space station with marketplace modules
class ASpaceStation : public AActor {
    TArray<TSubclassOf<ASpaceStationModule>> DefaultModuleClasses;
    UFUNCTION(BlueprintCallable) AMarketplaceModule* GetMarketplaceModule() const;
    UFUNCTION(BlueprintCallable) bool HasMarketplace() const;
};

// Marketplace module (attached to stations)
class AMarketplaceModule : public ASpaceStationModule {
    UPROPERTY(EditAnywhere) UMarketDataAsset* MarketDataAsset;
    UPROPERTY(EditAnywhere) bool bIsOpen;
    UPROPERTY(EditAnywhere) FText MarketplaceName;
    UFUNCTION(BlueprintCallable) UMarketDataAsset* GetMarketData() const;
    UFUNCTION(BlueprintCallable) bool IsAvailableForTrading() const;
};

// Trading ship component
class UCargoComponent : public UActorComponent {
    UPROPERTY() TMap<FName, int32> CargoInventory;
    UFUNCTION(BlueprintCallable) bool AddCargo(FName ItemID, int32 Quantity);
    UFUNCTION(BlueprintCallable) int32 GetCargoSpace() const;
};

// Economy manager
class UEconomyManager : public UGameInstanceSubsystem {
    UFUNCTION() void UpdatePrices(float DeltaTime);
    UFUNCTION(BlueprintCallable) float CalculatePrice(FName ItemID, FName StationID) const;
};
```

### Data Assets (Implemented)

```cpp
// Trade item definition
UCLASS() class UTradeItemDataAsset : public UDataAsset {
    UPROPERTY(EditAnywhere) FText ItemName;
    UPROPERTY(EditAnywhere) float BasePrice;
    UPROPERTY(EditAnywhere) float Volatility;
    UPROPERTY(EditAnywhere) float VolumePerUnit;
};

// Market configuration
UCLASS() class UMarketDataAsset : public UPrimaryDataAsset {
    UPROPERTY(EditAnywhere) FText MarketName;
    UPROPERTY(EditAnywhere) EMarketType MarketType;
    UPROPERTY(EditAnywhere) EMarketSize MarketSize;
    UPROPERTY(EditAnywhere) float SellPriceMarkup;    // 1.2 = +20% when buying
    UPROPERTY(EditAnywhere) float BuyPriceMarkdown;   // 0.8 = -20% when selling
    UPROPERTY(EditAnywhere) TArray<FMarketInventoryEntry> Inventory;
    UFUNCTION(BlueprintCallable) float GetItemPrice(UTradeItemDataAsset* Item, bool bBuying);
};
```

### Trading Flow (Blueprint Logic)

```
1. Player docks at station
2. Check: Station->HasMarketplace()
3. Get marketplace: Station->GetMarketplaceModule()
4. Check: Marketplace->IsAvailableForTrading()
5. Get market data: Marketplace->GetMarketData()
6. Open WBP_TradingUI with market data
7. Player buys/sells
8. Update CargoComponent + Credits
9. Play SFX/VFX
```

---

## 🎨 Content Guidelines

### Trade Items (15 Current, Target 20)
**Categories (3-5 items each):**
- Basic Goods: Water, Protein Packs, Food Rations
- Technology: Basic Computers, Advanced Electronics, AI Cores
- Industrial: Basic Alloys, Refined Metals, Ship Components
- Luxury: Luxury Foodstuffs, Entertainment Systems, Rare Artwork
- Resources: Hydrogen Fuel, Antimatter Cores, Quantum Processors

**Item Properties:**
- Base price (credits)
- Volume (cargo space per unit)
- Volatility (price fluctuation range)
- Station availability

### Stations (5 Current, Target 10)

| Station | Type | Produces | Imports | Markup/Markdown |
|---------|------|----------|---------|-----------------|
| Farming Collective | OpenMarket | Food, Water | Technology, Machinery | 1.1x / 0.9x |
| Industrial Exchange | IndustrialDepot | Steel, Components | Raw Materials, Food | 1.05x / 0.95x |
| Stellar Bazaar | LuxuryBazaar | Art, Jewelry | Luxury Food, Electronics | 1.25x / 0.75x |
| Research Consortium | ResearchHub | AI Cores, Quantum | Rare Materials, Components | 1.15x / 0.85x |
| Central Trading | OpenMarket | General Goods | Everything | 1.2x / 0.8x |

**Need 5 More:**
- Mining Outpost (RawMaterials)
- Military Depot (ShipComponents, Weapons)
- Medical Station (Medical Supplies)
- Agricultural Hub (Food, Bio)
- Tech Hub (Electronics, AI)

---

## 🧪 Testing Requirements

### Week 4: Prototype Playtest (Done)
- ✅ 5-10 external testers
- ✅ No instructions given
- ✅ 60%+ "had fun" → GO decision made

### Week 8: Structured Playtest (Done)
- ✅ 10-20 testers
- ✅ 2-min tutorial
- ✅ 70%+ "had fun", 50%+ "would buy"

### Week 12: Demo Validation (Target This Week)
- 20+ testers
- Full 30-minute demo
- Targets: 75%+ "fun", 50%+ "would buy", 90% completion, 60 FPS

---

## 📊 Current Status (August 9, 2026)

| Area | Status | Notes |
|------|--------|-------|
| C++ Trading Systems | ✅ Complete | TradeItem, Market, Economy, Cargo, PlayerTrader |
| DataAssets (Items) | ✅ 15/20 | Validated by test_trading_system.py |
| DataAssets (Markets) | ⚠️ 5/10 | Need 5 more MarketDataAssets |
| Ship Blueprints | ✅ 3 ships | BP_PlayerShip, BP_Ship_Freighter, BP_Ship_Corvette |
| Station Blueprints | ✅ Modular | BP_SpaceStation + modules |
| Trading UI | ⚠️ Template exists | WBP_TradingUI needs wiring |
| Trading Logic | ⚠️ Partial | C++ ready, Blueprint wiring needed |
| Test Level | ❌ Not created | Need MVP_TestLevel with 3 stations |
| VFX/Audio | ❌ Not added | Docking, trade confirmation |
| Save/Load | ⚠️ Code exists | SaveGameSubsystem needs testing |

---

## 🚀 Next 3 Days (MCP Execution Plan)

1. **Verify all 20 trade items + 10 markets** via MCP
2. **Create 3 core Blueprints** (TradingShip, TradeStation, TradingUI)
3. **Wire trading logic** in Blueprint graphs
4. **Build test level** with 3 stations
5. **PIE test** - you fly/dock/trade, I capture screenshots
6. **Add 5 missing markets** via MCP
7. **Add VFX/Audio** for polish
8. **Performance check** - 60 FPS target

---

## 📋 Definition of Done (Week 12)

- [ ] 10 stations with varied economies
- [ ] 20 trade goods across 5 categories
- [ ] 3 ships with clear progression
- [ ] Trading UI fully functional (buy/sell/profit display)
- [ ] Docking → Trade → Undock loop works without errors
- [ ] Save/Load works for credits, cargo, position
- [ ] 60 FPS on mid-range hardware (GTX 1060 class)
- [ ] No critical bugs in 1-hour stress test
- [ ] 30-minute guided demo experience
- [ ] Publisher pitch materials (screenshots, video, deck)

---

*Back to [INDEX.md](INDEX.md) | Next: [03-CURRENT_STATUS.md](03-CURRENT_STATUS.md)*