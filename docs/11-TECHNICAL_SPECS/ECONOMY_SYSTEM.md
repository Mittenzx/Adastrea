# Economy System Technical Specification

**Trade Simulator MVP | Dynamic Market Simulation | Last Updated: August 9, 2026**

---

## 🎯 Overview

The Economy System simulates **supply/demand dynamics** across markets, creating profitable trade routes for players.

**Files**: `Source/Adastrea/Private/Trading/EconomyManager.cpp/h`
**Integration**: GameInstanceSubsystem — global, persistent across levels

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
python test_trading_system.py

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