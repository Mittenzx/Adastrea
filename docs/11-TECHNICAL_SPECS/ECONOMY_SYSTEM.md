# Economy System Technical Specification

**Trade Simulator MVP | Dynamic Market Simulation | Last Updated: August 9, 2026**

---

## 🎯 Overview

The Economy System simulates **supply/demand dynamics** across markets, creating profitable trade routes for players.

**Files**: `Source/Adastrea/Private/Trading/EconomyManager.cpp/h`
**Integration**: GameInstanceSubsystem — global, persistent across levels

---

## Raw resources, production and drones (built 2026-10-03)

**Raw resources in markets.** Every crafting-tree item is on sale at every station
(`UCraftingTreeLoader::PopulateMarketInventory`). The 30 raw resources (ores, ice, gases,
salvage) are now the same trade item assets the drones mine (`DA_TradeItem_<Item>`, made by
`Tools/ue_make_resource_types.py`), so mined ore and market ore are one item with one price
(50 / 110 / 260 cr by galaxy tier). Each station then sets supply, demand and stock of its raw
resources from `Galaxy.json` (`ASpaceStation::ApplyRegionalSupply`):

| Where the resource is found | Supply | Demand | Stock | Price vs base |
|---|---|---|---|---|
| This sector, High richness | 1.35 | 0.85 | 30-60k | ~0.63x |
| This sector, Medium (or a field POI) | 1.25 | 0.90 | 20-40k | ~0.72x |
| This sector, Low | 1.15 | 0.95 | 12-24k | ~0.83x |
| Another sector of the same system | 1.00 | 1.00 | 4-10k | 1x |
| Nowhere in the system (imported) | 0.85 | 1.20 | 0.4-2k | ~1.41x |

So ore is cheap at the mine and dear where it isn't found: hauling it is a trade route.
Entries hand-authored on a station's `StationMarket` keep their own values. Items had no
category in the JSON (everything was "Refined Goods"); they now take their recipe's category.
`adastrea.MarketInfo` lists a station's raw resource prices.

**Production.** `UCraftingManager` (GameInstance subsystem) owns the one shared crafting tree
and runs recipes at stations. Docked, open **Production Floor** from the station menu: one tab
per facility the station has modules for (Processing, Fabrication, Reactor, the labs), its
recipes with how many runs the hold can afford, and the station's job queue. Space queues one
run, Q five, X as many as the hold allows; B moves to the job list, where X cancels a job
(unstarted runs refund their ingredients). Ingredients leave the hold when a job is queued.
A run takes 6-90 s by tier, divided by the number of modules of that facility; each facility
works one job at a time, up to 4 queued. Finished goods wait at the station and go into the
hold while you're docked there. Jobs keep running in other sectors and are saved. Recipes that
need research are listed but locked. Console: `adastrea.CraftInfo`, `adastrea.Craft Output
[Runs]`, `adastrea.CraftSpeed X`.

**Drones.** Gas pockets have their own drone behaviour: drones fly into the pocket and draw gas
through a spinning intake ring while drifting round it; no clamping or drilling, no rock dust.
Wreck debris and ice are worked like rock: cutting plate free is drilling with another name,
so salvage keeps the mining behaviour and the HUD says SALVAGING. The kind comes from
`UAsteroidDataAsset::ResourceKind` (Auto works it out from the ore). **Cargo drones**
(`ACargoDrone`) move goods between ships a pod (25 volume) at a time: take from a wreck or a
ship of the same owner, give to a ship of the same owner. Lock a wreck and press the drone key
to strip its hold; press again to stop. Goods from someone else's wreck are stolen (see
[PIRACY_AND_LAW.md](PIRACY_AND_LAW.md), step 3).

**Names.** The HUD names stations by `StationName`, else their level label (kept in packaged
builds by `ASpaceStation::SavedLabel`), and ships by roster name, else ship type
(`AdastreaNames::ForActor`), never by object name.

---

## 📦 Core Class: UEconomyManager

```cpp
UCLASS()
class UEconomyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // Registration
    UFUNCTION(BlueprintCallable, Category="Economy")
    void RegisterMarket(UMarketDataAsset* Market);
    
    UFUNCTION(BlueprintCallable, Category="Economy")
    void UnregisterMarket(UMarketDataAsset* Market);
    
    // Periodic update (called every in-game hour)
    UFUNCTION(BlueprintCallable, Category="Economy")
    void UpdateEconomy(float DeltaHours);
    
    // Price queries
    UFUNCTION(BlueprintCallable, Category="Economy")
    float GetItemPrice(FName ItemID, FName MarketID) const;
    
    UFUNCTION(BlueprintCallable, Category="Economy")
    float GetItemPriceAtMarket(UTradeItemDataAsset* Item, UMarketDataAsset* Market, bool bIsBuying) const;
    
    // Market events
    UFUNCTION(BlueprintCallable, Category="Events")
    void TriggerMarketEvent(FName MarketID, FName ItemID, float PriceMultiplier, float DurationHours);
    
    UFUNCTION(BlueprintCallable, Category="Events")
    void TriggerGlobalEvent(FName ItemID, float PriceMultiplier, float DurationHours);
    
    // AI trader simulation
    UFUNCTION(BlueprintCallable, Category="AI")
    void SimulateAITrading(float DeltaHours);
    
    // Data access
    UFUNCTION(BlueprintCallable, Category="Economy")
    TArray<UMarketDataAsset*> GetRegisteredMarkets() const;
    
    UFUNCTION(BlueprintCallable, Category="Economy")
    UMarketDataAsset* GetMarket(FName MarketID) const;
    
private:
    UPROPERTY()
    TMap<FName, UMarketDataAsset*> RegisteredMarkets;
    
    UPROPERTY()
    TArray<FMarketEvent> ActiveGlobalEvents;
    
    // Timer
    FTimerHandle EconomyUpdateTimer;
    
    // Configuration
    UPROPERTY(EditAnywhere, Category="Config")
    float UpdateIntervalHours = 1.0f;
    
    UPROPERTY(EditAnywhere, Category="Config")
    float AISimulationIntervalHours = 6.0f;
};
```

---

## 🔄 Economy Update Loop

```cpp
void UEconomyManager::UpdateEconomy(float DeltaHours)
{
    // 1. Update each registered market
    for (auto& Pair : RegisteredMarkets)
    {
        UMarketDataAsset* Market = Pair.Value;
        if (Market)
        {
            Market->UpdateMarket(DeltaHours);
        }
    }
    
    // 2. Update global events
    for (int32 i = ActiveGlobalEvents.Num() - 1; i >= 0; --i)
    {
        FMarketEvent& Event = ActiveGlobalEvents[i];
        Event.RemainingHours -= DeltaHours;
        if (Event.RemainingHours <= 0.0f)
        {
            ActiveGlobalEvents.RemoveAt(i);
            // Broadcast event ended
        }
    }
    
    // 3. Simulate AI trading (periodic)
    AccumulatedAISimulationTime += DeltaHours;
    if (AccumulatedAISimulationTime >= AISimulationIntervalHours)
    {
        SimulateAITrading(AccumulatedAISimulationTime);
        AccumulatedAISimulationTime = 0.0f;
    }
}
```

---

## 📈 Market Simulation (Per MarketDataAsset)

```cpp
void UMarketDataAsset::UpdateMarket(float DeltaHours)
{
    // Stock refresh cycle
    if (StockRefreshRate > 0.0f)
    {
        LastStockRefreshTime += DeltaHours;
        if (LastStockRefreshTime >= StockRefreshRate)
        {
            RefreshStock();
            LastStockRefreshTime = 0.0f;
        }
    }
    
    // Supply/Demand normalization (toward 1.0 over 24 hours)
    float NormalizationRate = DeltaHours / 24.0f;
    for (FMarketInventoryEntry& Entry : Inventory)
    {
        if (!Entry.TradeItem) continue;
        
        Entry.SupplyLevel = FMath::Lerp(Entry.SupplyLevel, 1.0f, NormalizationRate);
        Entry.DemandLevel = FMath::Lerp(Entry.DemandLevel, 1.0f, NormalizationRate);
    }
    
    // Active market events
    UpdateMarketEvents(CurrentGameTime);
}

void UMarketDataAsset::RefreshStock()
{
    for (FMarketInventoryEntry& Entry : Inventory)
    {
        if (!Entry.TradeItem) continue;
        
        // Replenish toward max stock
        int32 ReplenishAmount = Entry.TradeItem->ReplenishmentRate;
        Entry.CurrentStock = FMath::Min(Entry.CurrentStock + ReplenishAmount, Entry.MaxStock);
        Entry.bInStock = Entry.CurrentStock > 0;
    }
    
    OnStockRefreshed();
}
```

---

## 💹 Price Calculation (Detailed)

```cpp
float UMarketDataAsset::GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const
{
    if (!TradeItem) return 0.0f;
    
    // Find inventory entry for supply/demand
    FMarketInventoryEntry Entry;
    bool bFound = GetInventoryEntry(TradeItem->ItemID, Entry);
    
    float Supply = bFound ? Entry.SupplyLevel : 1.0f;
    float Demand = bFound ? Entry.DemandLevel : 1.0f;
    float EventMultiplier = GetEventPriceMultiplier(TradeItem->ItemID);
    
    // Start with base price
    float Price = TradeItem->BasePrice;
    
    // Supply/Demand Dynamics
    if (TradeItem->bAffectedBySupplyDemand)
    {
        // High supply = lower price (inverse)
        const float MinSupply = 0.1f;
        float SupplyFactor = FMath::Clamp(
            1.0f / FMath::Max(Supply, MinSupply),
            TradeItem->PriceVolatility.MinPriceDeviation,
            TradeItem->PriceVolatility.MaxPriceDeviation
        );
        
        // High demand = higher price
        float DemandFactor = FMath::Clamp(
            Demand,
            TradeItem->PriceVolatility.MinPriceDeviation,
            TradeItem->PriceVolatility.MaxPriceDeviation
        );
        
        Price *= SupplyFactor * DemandFactor * TradeItem->PriceVolatility.VolatilityMultiplier;
    }
    
    // Market Events
    if (TradeItem->bAffectedByMarketEvents)
    {
        Price *= EventMultiplier;
    }
    
    // Clamp to volatility bounds
    Price = FMath::Clamp(
        Price,
        TradeItem->BasePrice * TradeItem->PriceVolatility.MinPriceDeviation,
        TradeItem->BasePrice * TradeItem->PriceVolatility.MaxPriceDeviation
    );
    
    // Market Markup/Markdown
    if (bIsBuying)
    {
        Price *= SellPriceMarkup;      // Player buys: 1.2 = +20%
    }
    else
    {
        Price *= BuyPriceMarkdown;     // Player sells: 0.8 = -20%
    }
    
    // Transaction Tax
    Price *= (1.0f + TransactionTaxRate);
    
    // Blueprint Overrides (in order)
    Price = TradeItem->OnCalculateCustomPrice(Supply, Demand, EventMultiplier, Price);
    Price = OnCalculateCustomMarketPrice(TradeItem, bIsBuying, Price);
    
    return Price;
}
```

---

## 🎪 Market Events

```cpp
USTRUCT(BlueprintType)
struct FMarketEvent
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventDescription;
    
    // Affected items (empty = all items)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AffectedItemIDs;
    
    // Price multiplier (1.5 = +50%, 0.7 = -30%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PriceMultiplier = 1.0f;
    
    // Duration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationHours = 24.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RemainingHours = 24.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;
    
    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* EventIcon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EventColor = FLinearColor::Yellow;
};

// Event Types (Examples)
/*
- "Harvest Festival" → Food items: 0.7x price (surplus)
- "Miner Strike" → Raw Materials: 1.8x price (shortage)  
- "Tech Boom" → Electronics: 1.5x price (high demand)
- "Pirate Blockade" → All items: 1.3x price (supply disruption)
- "Trade Festival" → Luxury items: 0.8x price (promotion)
*/
```

---

## 🤖 AI Trading Simulation

```cpp
void UEconomyManager::SimulateAITrading(float DeltaHours)
{
    for (auto& Pair : RegisteredMarkets)
    {
        UMarketDataAsset* Market = Pair.Value;
        if (!Market) continue;
        
        int32 TradesThisCycle = FMath::RandRange(0, Market->AITraderCount);
        
        for (int32 i = 0; i < TradesThisCycle; ++i)
        {
            // Pick random item from market inventory
            if (Market->Inventory.Num() == 0) continue;
            
            int32 Index = FMath::RandRange(0, Market->Inventory.Num() - 1);
            FMarketInventoryEntry& Entry = Market->Inventory[Index];
            if (!Entry.TradeItem || !Entry.bInStock) continue;
            
            // AI buys or sells based on priority and profitability
            float BuyPrice = Market->GetItemPrice(Entry.TradeItem, true);
            float SellPrice = Market->GetItemPrice(Entry.TradeItem, false);
            
            if (Entry.TradeItem->AITradePriority > 5 && Entry.SupplyLevel > 1.2f)
            {
                // AI sells (high supply)
                int32 Qty = FMath::RandRange(1, Entry.TradeItem->StandardLotSize);
                Entry.CurrentStock = FMath::Max(0, Entry.CurrentStock - Qty);
                Entry.SupplyLevel = FMath::Max(0.5f, Entry.SupplyLevel - 0.1f);
                Entry.DemandLevel = FMath::Min(2.0f, Entry.DemandLevel + 0.05f);
            }
            else if (Entry.DemandLevel > 1.2f)
            {
                // AI buys (high demand)
                int32 Qty = FMath::RandRange(1, Entry.TradeItem->StandardLotSize);
                Entry.CurrentStock += Qty;
                Entry.SupplyLevel = FMath::Min(2.0f, Entry.SupplyLevel + 0.1f);
                Entry.DemandLevel = FMath::Max(0.5f, Entry.DemandLevel - 0.05f);
            }
        }
    }
}
```

---

## 🎯 Creating Profitable Trade Routes

### Design Principles

1. **Complementary Economies**: Station A produces what Station B needs
2. **Distance Cost**: Flight time = opportunity cost
3. **Risk/Reward**: Longer routes = higher profit but more time
4. **Dynamic Shifts**: Events create temporary opportunities

### Example Route (Agricultural → Industrial)
```
Agricultural Station (Farming Collective)
  Produces: FoodRations (base 25), Water (base 10), ProteinPacks (base 50)
  Exports: Cheap food → High supply → Low price
  
Industrial Station (Industrial Exchange)  
  Needs: FoodRations, Water (workers need sustenance)
  Imports: High demand → High price
  
Route: Agricultural → Industrial
  Buy FoodRations at 20-30 credits
  Sell at Industrial for 40-60 credits
  Profit: 20-40 credits/unit
  Cargo: 50 units = 1,000-2,000 credits/trip
  Time: ~2 min flight + 1 min docking each = 4 min round trip
  Rate: ~15,000-30,000 credits/hour
```

### Route Variety (Target 5-10 Routes)

| Route | Buy At | Sell At | Goods | Est. Profit/hr | Difficulty |
|-------|--------|---------|-------|----------------|------------|
| Food Run | Agricultural | Industrial | Food, Water | 15K-30K | Easy |
| Tech Run | Research Hub | Luxury Bazaar | Electronics, AI Cores | 50K-100K | Medium |
| Resource Run | Mining Outpost | Industrial | Ore, Rare Earth | 30K-60K | Medium |
| Luxury Run | Luxury Bazaar | Agricultural | Art, Jewelry | 80K-150K | Hard |
| Circuit | All 3 | All 3 | Mixed | 40K-80K | Balanced |

---

## 🔧 Configuration (GameInstance)

```ini
; DefaultGame.ini
[/Script/Adastrea.EconomyManager]
UpdateIntervalHours=1.0
AISimulationIntervalHours=6.0

; Base prices balanced for 10-min loop = ~10K profit
; Starter ship cargo 50 → 200 credits/unit avg = 10K/trip
; 2 trips per 10 min = 6 trips/hr = 60K/hr (feels good)
```

---

## 🧪 Testing

```bash
python tests/test_trading_system.py

# Validates:
# - All 15 items have valid price ranges
# - Cargo efficiency rankings (credits/m³)
# - Market specialization creates distinct economies
# - Profitable routes exist between market pairs
```

---

## 📋 MVP Checklist

- [x] UEconomyManager C++ complete
- [x] MarketDataAsset::UpdateMarket() complete
- [x] Supply/Demand normalization complete
- [x] Stock refresh cycle complete
- [x] Market events system complete
- [x] AI trading simulation complete
- [x] Price formula with all factors complete
- [x] 5 MarketDataAssets created & validated
- [x] 15 TradeItemDataAssets with supply/demand config
- [ ] Register markets in GameInstance on level load
- [ ] Timer for periodic economy updates
- [ ] Blueprint access to GetItemPrice()
- [ ] Market events trigger via Blueprint
- [ ] PIE test: prices change over time
- [ ] PIE test: AI trading affects supply/demand

---

*Back to [System Reference](../06-SYSTEM_REFERENCE.md) | [INDEX.md](../INDEX.md)*