# Trading System Technical Specification

**Trade Simulator MVP | Core System | Last Updated: August 9, 2026**

---

## 🎯 Overview

The Trading System is the **core MVP gameplay loop**. All other systems exist to support trading.

**Files**: `Source/Adastrea/Private/Trading/`
**DataAssets**: `Content/DataAssets/Trading/Items/`, `Content/DataAssets/Trading/Markets/`

---

## 📦 Core Classes

### UTradeItemDataAsset
```cpp
// Definition of a tradeable good
UCLASS(Blueprintable, BlueprintType)
class UTradeItemDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText ItemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName ItemID;                    // Stable ID for saves
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    ETradeItemCategory Category;     // Food, Technology, Industrial, Luxury, Resources
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
    float BasePrice = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
    FFloatRange PriceVolatility;     // Min/Max deviation from base
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical")
    float VolumePerUnit = 1.0f;      // Cargo space per unit
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical")
    float MassPerUnit = 1.0f;        // Physics mass
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market")
    int32 StandardLotSize = 100;     // Default trade quantity
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market")
    int32 TypicalMarketStock = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market")
    int32 ReplenishmentRate = 100;   // Per refresh cycle
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
    int32 AITradePriority = 5;       // 1-10
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags")
    TArray<FName> BehaviorTags;      // "Perishable", "Illegal", "Luxury"
    
    // Blueprint override for custom pricing
    UFUNCTION(BlueprintNativeEvent, Category="Pricing")
    float OnCalculateCustomPrice(float Supply, float Demand, float EventMultiplier, float BaseCalculatedPrice) const;
    
    // Helpers
    UFUNCTION(BlueprintPure, Category="Physical")
    float GetTotalVolume(int32 Quantity) const { return VolumePerUnit * Quantity; }
    
    UFUNCTION(BlueprintPure, Category="Physical")
    float GetTotalMass(int32 Quantity) const { return MassPerUnit * Quantity; }
    
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
```

### UMarketDataAsset
```cpp
// Configuration for a station's market
UCLASS(Blueprintable, BlueprintType)
class UMarketDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText MarketName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName MarketID;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
    EMarketType MarketType;          // OpenMarket, LuxuryBazaar, IndustrialDepot, ResearchHub
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
    EMarketSize MarketSize;          // Small, Medium, Large, Massive
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
    float TransactionTaxRate = 0.05f;    // 5%
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
    float SellPriceMarkup = 1.2f;        // Player buys: +20%
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
    float BuyPriceMarkdown = 0.8f;       // Player sells: -20%
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
    TArray<FMarketInventoryEntry> Inventory;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation")
    float StockRefreshRate = 24.0f;      // Hours
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Simulation")
    TArray<FMarketEvent> ActiveEvents;   // Price modifiers
    
    // Core pricing function
    UFUNCTION(BlueprintCallable, Category="Trading")
    float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;
    
    // Inventory management
    UFUNCTION(BlueprintCallable, Category="Trading")
    bool IsItemInStock(FName ItemID, int32 Quantity) const;
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    TArray<FMarketInventoryEntry> GetItemsByCategory(ETradeItemCategory Category) const;
    
    // Simulation (called by EconomyManager)
    UFUNCTION(BlueprintCallable, Category="Simulation")
    void UpdateMarket(float DeltaHours);
    
    UFUNCTION(BlueprintCallable, Category="Simulation")
    void RefreshStock();
    
    // Blueprint override for custom market pricing
    UFUNCTION(BlueprintNativeEvent, Category="Pricing")
    float OnCalculateCustomMarketPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying, float BasePrice) const;
    
    // Events
    UFUNCTION(BlueprintImplementableEvent, Category="Events")
    void OnStockRefreshed();
    
    UFUNCTION(BlueprintImplementableEvent, Category="Events")
    void OnMarketEventStarted(const FMarketEvent& Event);
    
    UFUNCTION(BlueprintImplementableEvent, Category="Events")
    void OnMarketEventEnded(const FMarketEvent& Event);
};
```

### FMarketInventoryEntry (Struct)
```cpp
USTRUCT(BlueprintType)
struct FMarketInventoryEntry
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTradeItemDataAsset* TradeItem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStock = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStock = 10000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SupplyLevel = 1.0f;     // 0.0 - 2.0 (affects price)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DemandLevel = 1.0f;     // 0.0 - 2.0 (affects price)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInStock = true;
};
```

### UCargoComponent
```cpp
// Ship cargo inventory
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCargoComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capacity")
    int32 MaxCapacity = 50;           // Total volume units
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TMap<FName, int32> CargoInventory; // ItemID -> Quantity
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 UsedCapacity = 0;
    
    UFUNCTION(BlueprintCallable, Category="Cargo")
    bool AddCargo(FName ItemID, int32 Quantity, float VolumePerUnit);
    
    UFUNCTION(BlueprintCallable, Category="Cargo")
    bool RemoveCargo(FName ItemID, int32 Quantity);
    
    UFUNCTION(BlueprintPure, Category="Cargo")
    int32 GetCargoQuantity(FName ItemID) const;
    
    UFUNCTION(BlueprintPure, Category="Cargo")
    int32 GetAvailableSpace() const { return MaxCapacity - UsedCapacity; }
    
    UFUNCTION(BlueprintPure, Category="Cargo")
    float GetFillRatio() const { return (float)UsedCapacity / MaxCapacity; }
    
    // Delegate for UI updates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCargoChangedDelegate, FName, ItemID, int32, NewQuantity);
    UPROPERTY(BlueprintAssignable, Category="Cargo")
    FCargoChangedDelegate OnCargoChanged;
};
```

### UPlayerTraderComponent
```cpp
// Player credits and trading logic
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UPlayerTraderComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Credits")
    int32 StartingCredits = 10000;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Credits")
    int32 Credits = 10000;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
    int32 TotalProfit = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
    int32 TradesCompleted = 0;
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    bool CanAfford(int32 Cost) const { return Credits >= Cost; }
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    bool SpendCredits(int32 Amount);
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    void AddCredits(int32 Amount);
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    bool ExecuteTrade(UMarketDataAsset* Market, UTradeDataAsset* Item, int32 Quantity, bool bIsBuying);
    
    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreditsChangedDelegate, int32, NewCredits);
    UPROPERTY(BlueprintAssignable, Category="Trading")
    FCreditsChangedDelegate OnCreditsChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTradeCompletedDelegate, FName, ItemID, int32, Quantity, int32, Profit);
    UPROPERTY(BlueprintAssignable, Category="Trading")
    FTradeCompletedDelegate OnTradeCompleted;
};
```

### UEconomyManager (GameInstanceSubsystem)
```cpp
// Global economy simulation
UCLASS()
class UEconomyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // Called periodically (e.g., every in-game hour)
    UFUNCTION(BlueprintCallable, Category="Economy")
    void UpdateEconomy(float DeltaHours);
    
    // Get current price for item at market
    UFUNCTION(BlueprintCallable, Category="Economy")
    float GetItemPrice(FName ItemID, FName MarketID) const;
    
    // Register market for simulation
    UFUNCTION(BlueprintCallable, Category="Economy")
    void RegisterMarket(UMarketDataAsset* Market);
    
    // Market events
    UFUNCTION(BlueprintCallable, Category="Events")
    void TriggerMarketEvent(FName MarketID, FName ItemID, float PriceMultiplier, float DurationHours);
    
private:
    UPROPERTY()
    TMap<FName, UMarketDataAsset*> RegisteredMarkets;
    
    // Timer for periodic updates
    FTimerHandle EconomyUpdateTimer;
};
```

---

## 💰 Pricing Formula

```
BasePrice = TradeItem.BasePrice

// Supply/Demand (if enabled for item)
if (TradeItem.bAffectedBySupplyDemand) {
    SupplyFactor = Clamp(1.0 / Max(Supply, 0.1), MinDeviation, MaxDeviation)
    DemandFactor = Clamp(Demand, MinDeviation, MaxDeviation)
    BasePrice *= SupplyFactor * DemandFactor * VolatilityMultiplier
}

// Market Events (if enabled)
if (TradeItem.bAffectedByMarketEvents) {
    BasePrice *= EventMultiplier  // Product of all active events for this item
}

// Clamp to volatility bounds
BasePrice = Clamp(BasePrice, BasePrice * MinDeviation, BasePrice * MaxDeviation)

// Market Markup/Markdown
if (bIsBuying) {
    BasePrice *= Market.SellPriceMarkup      // 1.2 = +20%
} else {
    BasePrice *= Market.BuyPriceMarkdown     // 0.8 = -20%
}

// Transaction Tax
BasePrice *= (1.0 + Market.TransactionTaxRate)  // 1.05 = +5%

// Blueprint Overrides (in order)
BasePrice = TradeItem.OnCalculateCustomPrice(Supply, Demand, EventMultiplier, BasePrice)
BasePrice = Market.OnCalculateCustomMarketPrice(TradeItem, bIsBuying, BasePrice)

Return BasePrice
```

---

## 🔄 Trading Flow (Blueprint)

```
Player Approaches Station
    ↓
Trigger Volume (DockingBayModule)
    ↓
OnDocked(Station)
    ↓
Station->HasMarketplace() ?
    ├─ No → "No marketplace here"
    └─ Yes → Station->GetMarketplaceModule()
                ↓
            Marketplace->IsAvailableForTrading() ?
                ├─ No → "Market closed"
                └─ Yes → Marketplace->GetMarketData()
                            ↓
                        CreateWidget(WBP_TradingUI)
                        Widget->SetMarketData(MarketData)
                        Widget->AddToViewport()
                            ↓
                        Player clicks BUY/SELL
                            ↓
                        PlayerTraderComponent->ExecuteTrade()
                            ↓
                        CargoComponent->AddCargo/RemoveCargo()
                        PlayerTraderComponent->SpendCredits/AddCredits()
                            ↓
                        Play SFX + VFX
                        Update UI
```

---

## 🧪 Testing

```bash
# Validates all DataAssets, pricing math, cargo efficiency
python test_trading_system.py

# Output: trading_system_report.json with:
# - Price ranges per item
# - Cargo efficiency rankings (credits/m³)
# - Market specialization analysis
# - Profitable trade routes
```

---

## 📋 MVP Checklist

- [x] TradeItemDataAsset C++ complete
- [x] MarketDataAsset C++ complete  
- [x] EconomyManager C++ complete
- [x] CargoComponent C++ complete
- [x] PlayerTraderComponent C++ complete
- [x] 15 TradeItem DataAssets created
- [x] 5 Market DataAssets created
- [ ] 5 more TradeItems (target 20)
- [ ] 5 more Markets (target 10)
- [ ] BP_TradingShip with components wired
- [ ] BP_TradeStation with modules configured
- [ ] WBP_TradingUI graph logic wired
- [ ] PIE test: full buy→fly→sell loop
- [ ] Save/Load for credits, cargo, position

---

*Back to [System Reference](../06-SYSTEM_REFERENCE.md) | [INDEX.md](../INDEX.md)*