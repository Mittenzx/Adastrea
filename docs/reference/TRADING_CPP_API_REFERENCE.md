# Trading System C++ API Reference

**Quick reference for programming MVP trading features**

**Target Audience**: C++ programmers implementing trading gameplay  
**Version**: 1.0 (Trade Simulator MVP)  
**Last Updated**: 2026-01-17

---

## Core Classes Overview

| Class | Purpose | Location |
|-------|---------|----------|
| `UTradeItemDataAsset` | Defines tradeable items | `Trading/TradeItemDataAsset.h` |
| `UMarketDataAsset` | Defines marketplace configuration | `Trading/MarketDataAsset.h` |
| `UCargoComponent` | Manages ship cargo hold | `Trading/CargoComponent.h` |
| `UPlayerTraderComponent` | Player trading logic | `Trading/PlayerTraderComponent.h` |
| `UAITraderComponent` | AI trading logic | `Trading/AITraderComponent.h` |
| `UEconomyManager` | Global economy simulation | `Trading/EconomyManager.h` |
| `AMarketplaceModule` | Physical marketplace on station | `Stations/MarketplaceModule.h` |

---

## UTradeItemDataAsset

### Key Properties

```cpp
// Basic Info
FText ItemName;              // Display name
FName ItemID;                // Unique identifier (use for lookups)
ETradeItemCategory Category; // Food, Technology, Medical, etc.

// Pricing
float BasePrice;                      // Base price in credits
FPriceVolatility PriceVolatility;     // Volatility settings
bool bAffectedBySupplyDemand;         // Dynamic pricing enabled?

// Physical Properties
float VolumePerUnit;         // Cubic meters
float MassPerUnit;           // Kilograms

// AI Behavior
int32 AITradePriority;       // 1-10 (higher = more desirable)
bool bAIHoardable;           // AI will stockpile during shortages
bool bAIArbitrageEnabled;    // AI will use for arbitrage
```

### Key Functions

```cpp
// Check if item has specific tag
bool HasBehaviorTag(FName Tag) const;

// Calculate total volume/mass for quantity
float GetTotalVolume(int32 Quantity) const;
float GetTotalMass(int32 Quantity) const;

// Custom pricing override (BlueprintNativeEvent)
float OnCalculateCustomPrice_Implementation(
    float Supply, 
    float Demand, 
    float EventMultiplier, 
    float BaseCalculatedPrice
) const;
```

### Usage Example

```cpp
// Find item by ID
UTradeItemDataAsset* FindTradeItem(FName ItemID)
{
    // Use Asset Manager or direct reference
    UTradeItemDataAsset* Item = LoadObject<UTradeItemDataAsset>(
        nullptr, 
        TEXT("/Game/DataAssets/Trading/MVP/Items/DA_Item_Water")
    );
    return Item;
}

// Calculate cargo space needed
void CalculateCargoRequirements(UTradeItemDataAsset* Item, int32 Quantity)
{
    float TotalVolume = Item->GetTotalVolume(Quantity);
    float TotalMass = Item->GetTotalMass(Quantity);
    
    UE_LOG(LogTemp, Log, TEXT("Need %f m³ and %f kg for %d units"),
        TotalVolume, TotalMass, Quantity);
}

// Check if AI should prioritize this item
bool ShouldAIPrioritize(UTradeItemDataAsset* Item)
{
    return Item->AITradePriority >= 7 && Item->bAIHoardable;
}
```

---

## UMarketDataAsset

### Key Properties

```cpp
// Basic Info
FText MarketName;            // Display name
FName MarketID;              // Unique identifier
EMarketType MarketType;      // OpenMarket, BlackMarket, etc.
EMarketSize MarketSize;      // Small, Medium, Large, etc.

// Pricing Configuration
float TransactionTaxRate;    // 0.0 to 1.0 (e.g., 0.02 = 2%)
float SellPriceMarkup;       // Player buys: 1.2 = 20% markup
float BuyPriceMarkdown;      // Player sells: 0.8 = 20% markdown

// Access Control
bool bAllowPlayerBuying;     // Can player buy from market?
bool bAllowPlayerSelling;    // Can player sell to market?
bool bAllowAITraders;        // Can AI use this market?

// Inventory
TArray<FMarketInventoryEntry> Inventory;  // Items in stock
float StockRefreshRate;                    // Refresh interval (hours)

// Events
TArray<FMarketEvent> ActiveEvents;        // Active price events
float RandomEventChance;                   // Daily event probability
```

### Key Functions

```cpp
// Get current price for item (buy or sell)
float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;

// Get inventory entry for item
bool GetInventoryEntry(FName ItemID, FMarketInventoryEntry& OutEntry) const;

// Check if item is in stock
bool IsItemInStock(FName ItemID, int32 Quantity) const;

// Get items by category
TArray<FMarketInventoryEntry> GetItemsByCategory(ETradeItemCategory Category) const;

// Get active events for item
TArray<FMarketEvent> GetActiveEventsForItem(FName ItemID) const;

// Custom pricing override (BlueprintNativeEvent)
float OnCalculateCustomMarketPrice_Implementation(
    UTradeItemDataAsset* TradeItem,
    bool bIsBuying,
    float BasePrice
) const;

// Event callbacks (BlueprintNativeEvent)
void OnStockRefreshed_Implementation();
void OnMarketEventStarted_Implementation(const FMarketEvent& Event);
void OnMarketEventEnded_Implementation(const FMarketEvent& Event);
```

### Usage Example

```cpp
// Get buy price for water at agricultural market
float GetBuyPrice(UMarketDataAsset* Market, UTradeItemDataAsset* Item)
{
    // Player is buying from station (station is selling)
    bool bIsBuying = true;
    float Price = Market->GetItemPrice(Item, bIsBuying);
    return Price;
}

// Check if player can afford purchase
bool CanAfford(UMarketDataAsset* Market, UTradeItemDataAsset* Item, 
               int32 Quantity, float PlayerCredits)
{
    float PricePerUnit = Market->GetItemPrice(Item, true);
    float TotalCost = PricePerUnit * Quantity;
    
    // Add transaction tax
    float Tax = TotalCost * Market->TransactionTaxRate;
    TotalCost += Tax;
    
    return PlayerCredits >= TotalCost;
}

// Find cheapest market for an item
UMarketDataAsset* FindCheapestMarket(UTradeItemDataAsset* Item, 
                                      TArray<UMarketDataAsset*> Markets)
{
    UMarketDataAsset* CheapestMarket = nullptr;
    float LowestPrice = MAX_flt;
    
    for (UMarketDataAsset* Market : Markets)
    {
        if (!Market->IsItemInStock(Item->ItemID, 1))
            continue;
            
        float Price = Market->GetItemPrice(Item, true);
        if (Price < LowestPrice)
        {
            LowestPrice = Price;
            CheapestMarket = Market;
        }
    }
    
    return CheapestMarket;
}
```

---

## FMarketInventoryEntry

### Structure Definition

```cpp
USTRUCT(BlueprintType)
struct FMarketInventoryEntry
{
    GENERATED_BODY()
    
    // The trade item
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTradeItemDataAsset* TradeItem;
    
    // Current stock level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStock;
    
    // Maximum stock level
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStock;
    
    // Supply level (1.0 = typical)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SupplyLevel;
    
    // Demand level (1.0 = typical)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DemandLevel;
    
    // Last trade price
    UPROPERTY(BlueprintReadWrite)
    float LastTradePrice;
    
    // In stock flag
    UPROPERTY(BlueprintReadWrite)
    bool bInStock;
};
```

### Usage Example

```cpp
// Update inventory after purchase
void ProcessPurchase(UMarketDataAsset* Market, FName ItemID, int32 Quantity)
{
    for (FMarketInventoryEntry& Entry : Market->Inventory)
    {
        if (Entry.TradeItem && Entry.TradeItem->ItemID == ItemID)
        {
            // Reduce stock
            Entry.CurrentStock -= Quantity;
            
            // Update supply/demand
            Entry.SupplyLevel -= (Quantity / 1000.0f); // Reduce supply
            Entry.DemandLevel += (Quantity / 2000.0f); // Increase demand
            
            // Clamp values
            Entry.SupplyLevel = FMath::Max(0.1f, Entry.SupplyLevel);
            Entry.DemandLevel = FMath::Min(5.0f, Entry.DemandLevel);
            
            // Update in-stock flag
            Entry.bInStock = Entry.CurrentStock > 0;
            
            break;
        }
    }
}
```

---

## UCargoComponent

### Key Properties

```cpp
// Cargo capacity in cubic meters
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
float CargoCapacityVolume;

// Current cargo volume
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
float CurrentCargoVolume;

// Cargo items (ItemID -> Quantity)
UPROPERTY(BlueprintReadOnly, Category="State")
TMap<FName, int32> CargoInventory;
```

### Key Functions

```cpp
// Add cargo to hold
UFUNCTION(BlueprintCallable)
bool AddCargo(FName ItemID, int32 Quantity);

// Remove cargo from hold
UFUNCTION(BlueprintCallable)
bool RemoveCargo(FName ItemID, int32 Quantity);

// Get quantity of item in cargo
UFUNCTION(BlueprintCallable, BlueprintPure)
int32 GetCargoQuantity(FName ItemID) const;

// Check if cargo space available
UFUNCTION(BlueprintCallable, BlueprintPure)
bool HasCargoSpace(float VolumeRequired) const;

// Get available cargo space
UFUNCTION(BlueprintCallable, BlueprintPure)
float GetAvailableCargoSpace() const;

// Calculate total cargo value
UFUNCTION(BlueprintCallable, BlueprintPure)
float CalculateTotalCargoValue() const;
```

### Usage Example

```cpp
// Buy items and add to cargo
bool BuyAndLoadCargo(UCargoComponent* Cargo, UTradeItemDataAsset* Item, int32 Quantity)
{
    // Check cargo space
    float VolumeNeeded = Item->GetTotalVolume(Quantity);
    if (!Cargo->HasCargoSpace(VolumeNeeded))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough cargo space!"));
        return false;
    }
    
    // Add to cargo
    if (Cargo->AddCargo(Item->ItemID, Quantity))
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded %d units of %s"), 
            Quantity, *Item->ItemName.ToString());
        return true;
    }
    
    return false;
}

// Sell all cargo of specific type
int32 SellAllCargoOfType(UCargoComponent* Cargo, FName ItemID)
{
    int32 QuantityInCargo = Cargo->GetCargoQuantity(ItemID);
    if (QuantityInCargo > 0)
    {
        Cargo->RemoveCargo(ItemID, QuantityInCargo);
        return QuantityInCargo;
    }
    return 0;
}

// Calculate profit potential
float CalculateProfitPotential(UCargoComponent* Cargo, 
                                UMarketDataAsset* SellMarket,
                                TMap<FName, float> PurchasePrices)
{
    float TotalProfit = 0.0f;
    
    for (auto& Entry : Cargo->CargoInventory)
    {
        FName ItemID = Entry.Key;
        int32 Quantity = Entry.Value;
        
        // Get item data
        UTradeItemDataAsset* Item = FindTradeItemByID(ItemID);
        if (!Item) continue;
        
        // Get sell price at market
        float SellPrice = SellMarket->GetItemPrice(Item, false); // false = selling
        
        // Get original purchase price
        float* PurchasePricePtr = PurchasePrices.Find(ItemID);
        float PurchasePrice = PurchasePricePtr ? *PurchasePricePtr : Item->BasePrice;
        
        // Calculate profit for this item
        float ItemProfit = (SellPrice - PurchasePrice) * Quantity;
        TotalProfit += ItemProfit;
    }
    
    return TotalProfit;
}
```

---

## AMarketplaceModule

### Key Properties

```cpp
// Market configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
UMarketDataAsset* MarketDataAsset;

// Trading availability
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
bool bIsOpen;

// Display name
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
FText MarketplaceName;

// Module type (inherited from ASpaceStationModule)
EStationModuleGroup ModuleGroup = EStationModuleGroup::Public;

// Power consumption (inherited)
float ModulePower = 40.0f;
```

### Key Functions

```cpp
// Get market data
UFUNCTION(BlueprintCallable)
UMarketDataAsset* GetMarketData() const;

// Check if available for trading
UFUNCTION(BlueprintCallable)
bool IsAvailableForTrading() const;

// Open/close market
UFUNCTION(BlueprintCallable)
void SetMarketOpen(bool bOpen);
```

### Usage Example

```cpp
// Find marketplace on station and initiate trade
void InitiateTrade(ASpaceStation* Station, ASpaceship* PlayerShip)
{
    // Check if station has marketplace
    if (!Station->HasMarketplace())
    {
        UE_LOG(LogTemp, Warning, TEXT("Station has no marketplace!"));
        return;
    }
    
    // Get marketplace module
    AMarketplaceModule* Marketplace = Station->GetMarketplaceModule();
    if (!Marketplace)
    {
        UE_LOG(LogTemp, Error, TEXT("Marketplace module null!"));
        return;
    }
    
    // Check if open for trading
    if (!Marketplace->IsAvailableForTrading())
    {
        UE_LOG(LogTemp, Warning, TEXT("Market is closed!"));
        return;
    }
    
    // Get market data
    UMarketDataAsset* MarketData = Marketplace->GetMarketData();
    if (!MarketData)
    {
        UE_LOG(LogTemp, Error, TEXT("No market data assigned!"));
        return;
    }
    
    // Open trading UI
    OpenTradingUI(PlayerShip, MarketData);
}
```

---

## Common Patterns

### Pattern: Calculate Trade Profit

```cpp
struct FTradeRoute
{
    UMarketDataAsset* BuyMarket;
    UMarketDataAsset* SellMarket;
    UTradeItemDataAsset* Item;
    int32 Quantity;
    float Profit;
    float ROI;
};

FTradeRoute CalculateTradeRoute(UMarketDataAsset* BuyMarket,
                                 UMarketDataAsset* SellMarket,
                                 UTradeItemDataAsset* Item,
                                 int32 Quantity)
{
    FTradeRoute Route;
    Route.BuyMarket = BuyMarket;
    Route.SellMarket = SellMarket;
    Route.Item = Item;
    Route.Quantity = Quantity;
    
    // Calculate buy cost
    float BuyPrice = BuyMarket->GetItemPrice(Item, true);
    float BuyTax = BuyPrice * Quantity * BuyMarket->TransactionTaxRate;
    float TotalCost = (BuyPrice * Quantity) + BuyTax;
    
    // Calculate sell revenue
    float SellPrice = SellMarket->GetItemPrice(Item, false);
    float SellTax = SellPrice * Quantity * SellMarket->TransactionTaxRate;
    float TotalRevenue = (SellPrice * Quantity) - SellTax;
    
    // Calculate profit and ROI
    Route.Profit = TotalRevenue - TotalCost;
    Route.ROI = (Route.Profit / TotalCost) * 100.0f;
    
    return Route;
}
```

### Pattern: Find Best Trade Route

```cpp
FTradeRoute FindBestRoute(TArray<UMarketDataAsset*> Markets,
                          TArray<UTradeItemDataAsset*> Items,
                          float PlayerCapital,
                          float PlayerCargoSpace)
{
    FTradeRoute BestRoute;
    float MaxProfit = 0.0f;
    
    for (UMarketDataAsset* BuyMarket : Markets)
    {
        for (UMarketDataAsset* SellMarket : Markets)
        {
            if (BuyMarket == SellMarket) continue;
            
            for (UTradeItemDataAsset* Item : Items)
            {
                // Check if item available in both markets
                if (!BuyMarket->IsItemInStock(Item->ItemID, 1)) continue;
                if (!SellMarket->IsItemInStock(Item->ItemID, 1)) continue;
                
                // Calculate max quantity player can afford/carry
                float BuyPrice = BuyMarket->GetItemPrice(Item, true);
                int32 MaxAfford = FMath::FloorToInt(PlayerCapital / BuyPrice);
                int32 MaxCarry = FMath::FloorToInt(PlayerCargoSpace / Item->VolumePerUnit);
                int32 MaxQuantity = FMath::Min(MaxAfford, MaxCarry);
                
                if (MaxQuantity <= 0) continue;
                
                // Calculate route profitability
                FTradeRoute Route = CalculateTradeRoute(BuyMarket, SellMarket, Item, MaxQuantity);
                
                if (Route.Profit > MaxProfit)
                {
                    MaxProfit = Route.Profit;
                    BestRoute = Route;
                }
            }
        }
    }
    
    return BestRoute;
}
```

### Pattern: Execute Trade Transaction

```cpp
bool ExecuteTrade(UPlayerTraderComponent* Trader,
                  UMarketDataAsset* Market,
                  UTradeItemDataAsset* Item,
                  int32 Quantity,
                  bool bBuying)
{
    // Validate inputs
    if (!Trader || !Market || !Item || Quantity <= 0)
    {
        return false;
    }
    
    // Get player credits and cargo
    float PlayerCredits = Trader->GetCurrentCredits();
    UCargoComponent* Cargo = Trader->GetCargoComponent();
    
    if (!Cargo) return false;
    
    // Calculate price
    float PricePerUnit = Market->GetItemPrice(Item, bBuying);
    float Tax = (PricePerUnit * Quantity) * Market->TransactionTaxRate;
    float TotalCost = (PricePerUnit * Quantity) + Tax;
    
    if (bBuying)
    {
        // Player buying from market
        
        // Check affordability
        if (PlayerCredits < TotalCost)
        {
            UE_LOG(LogTemp, Warning, TEXT("Not enough credits!"));
            return false;
        }
        
        // Check cargo space
        float VolumeNeeded = Item->GetTotalVolume(Quantity);
        if (!Cargo->HasCargoSpace(VolumeNeeded))
        {
            UE_LOG(LogTemp, Warning, TEXT("Not enough cargo space!"));
            return false;
        }
        
        // Check stock
        if (!Market->IsItemInStock(Item->ItemID, Quantity))
        {
            UE_LOG(LogTemp, Warning, TEXT("Not enough stock!"));
            return false;
        }
        
        // Execute transaction
        Trader->DeductCredits(TotalCost);
        Cargo->AddCargo(Item->ItemID, Quantity);
        
        // Update market (would be done by EconomyManager in real implementation)
        // ProcessMarketPurchase(Market, Item, Quantity);
        
        UE_LOG(LogTemp, Log, TEXT("Bought %d %s for %f credits"), 
            Quantity, *Item->ItemName.ToString(), TotalCost);
        
        return true;
    }
    else
    {
        // Player selling to market
        
        // Check if player has items
        int32 PlayerQuantity = Cargo->GetCargoQuantity(Item->ItemID);
        if (PlayerQuantity < Quantity)
        {
            UE_LOG(LogTemp, Warning, TEXT("Don't have enough items!"));
            return false;
        }
        
        // Calculate revenue (note: selling uses different price)
        float Revenue = TotalCost; // Already calculated above
        
        // Execute transaction
        Cargo->RemoveCargo(Item->ItemID, Quantity);
        Trader->AddCredits(Revenue);
        
        // Update market
        // ProcessMarketSale(Market, Item, Quantity);
        
        UE_LOG(LogTemp, Log, TEXT("Sold %d %s for %f credits"), 
            Quantity, *Item->ItemName.ToString(), Revenue);
        
        return true;
    }
}
```

---

## Performance Considerations

### Use TMap for Fast Lookups

```cpp
// BAD: Linear search
UMarketDataAsset* FindMarketSlow(TArray<UMarketDataAsset*> Markets, FName MarketID)
{
    for (UMarketDataAsset* Market : Markets)
    {
        if (Market && Market->MarketID == MarketID)
        {
            return Market;
        }
    }
    return nullptr;
}

// GOOD: O(1) lookup with TMap
class UEconomyManager
{
    UPROPERTY()
    TMap<FName, UMarketDataAsset*> MarketLookup;
    
    void BuildMarketIndex()
    {
        MarketLookup.Empty();
        for (UMarketDataAsset* Market : AllMarkets)
        {
            if (Market)
            {
                MarketLookup.Add(Market->MarketID, Market);
            }
        }
    }
    
    UMarketDataAsset* FindMarketFast(FName MarketID)
    {
        UMarketDataAsset** Found = MarketLookup.Find(MarketID);
        return Found ? *Found : nullptr;
    }
};
```

### Cache Expensive Calculations

```cpp
class UMarketDataAsset
{
    // Cached prices with dirty flag
    mutable TMap<FName, float> CachedBuyPrices;
    mutable TMap<FName, float> CachedSellPrices;
    mutable bool bPricesCacheDirty = true;
    
    float GetItemPrice(UTradeItemDataAsset* Item, bool bIsBuying) const
    {
        if (bPricesCacheDirty)
        {
            RecalculateAllPrices();
            bPricesCacheDirty = false;
        }
        
        TMap<FName, float>& Cache = bIsBuying ? CachedBuyPrices : CachedSellPrices;
        float* CachedPrice = Cache.Find(Item->ItemID);
        
        return CachedPrice ? *CachedPrice : CalculatePriceUncached(Item, bIsBuying);
    }
    
    void MarkPricesDirty()
    {
        bPricesCacheDirty = true;
    }
};
```

---

## Blueprint Integration

### Exposing C++ to Blueprint

```cpp
// Property exposure
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trading")
float CurrentCredits;

// Function exposure
UFUNCTION(BlueprintCallable, Category="Trading")
bool BuyItem(FName ItemID, int32 Quantity);

// Pure getter (no side effects)
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading")
float GetTotalCargoValue() const;

// Event that Blueprint can implement
UFUNCTION(BlueprintImplementableEvent, Category="Trading")
void OnTradeCompleted(FName ItemID, int32 Quantity, float TotalCost);

// Event that Blueprint can override
UFUNCTION(BlueprintNativeEvent, Category="Trading")
float CalculateCustomDiscount();
```

### Calling Blueprint from C++

```cpp
// Call BlueprintImplementableEvent
void CompleteTradeTransaction(FName ItemID, int32 Quantity, float Cost)
{
    // C++ logic
    UpdateCredits(-Cost);
    UpdateInventory(ItemID, Quantity);
    
    // Notify Blueprint
    OnTradeCompleted(ItemID, Quantity, Cost);
}

// Call BlueprintNativeEvent with C++ fallback
float GetFinalPrice(float BasePrice)
{
    // Try Blueprint implementation
    float Discount = CalculateCustomDiscount();
    
    // Apply discount
    return BasePrice * (1.0f - Discount);
}

// C++ fallback implementation
float UPlayerTraderComponent::CalculateCustomDiscount_Implementation()
{
    // Default: no discount
    return 0.0f;
}
```

---

##Debugging Tips

### Log Trading Activity

```cpp
#define LOG_TRADING(Format, ...) \
    UE_LOG(LogTemp, Log, TEXT("[TRADING] " Format), ##__VA_ARGS__)

void LogTradeDetails(UMarketDataAsset* Market, UTradeItemDataAsset* Item, 
                     int32 Quantity, float Price)
{
    LOG_TRADING("Market: %s", *Market->MarketName.ToString());
    LOG_TRADING("Item: %s", *Item->ItemName.ToString());
    LOG_TRADING("Quantity: %d", Quantity);
    LOG_TRADING("Price/Unit: %.2f", Price);
    LOG_TRADING("Total: %.2f", Price * Quantity);
}
```

### Validate Data Assets

```cpp
#if WITH_EDITOR
EDataValidationResult UMarketDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);
    
    // Validate markup/markdown
    if (SellPriceMarkup < 1.0f)
    {
        Context.AddError(FText::FromString("SellPriceMarkup must be >= 1.0"));
        Result = EDataValidationResult::Invalid;
    }
    
    if (BuyPriceMarkdown > 1.0f)
    {
        Context.AddError(FText::FromString("BuyPriceMarkdown must be <= 1.0"));
        Result = EDataValidationResult::Invalid;
    }
    
    // Validate inventory
    for (const FMarketInventoryEntry& Entry : Inventory)
    {
        if (!Entry.TradeItem)
        {
            Context.AddError(FText::FromString("Inventory entry has null TradeItem"));
            Result = EDataValidationResult::Invalid;
        }
    }
    
    return Result;
}
#endif
```

---

## Related Documentation

- **Blueprint Templates**: `Content/Blueprints/Trading/`
- **YAML Templates**: `Assets/TradingTemplates/`
- **Quick Start Guide**: `docs/development/MVP_QUICK_START_GUIDE.md`
- **Economics Cheat Sheet**: `docs/reference/TRADING_ECONOMICS_CHEAT_SHEET.md`

---

**Quick Tips:**
- Use `BlueprintReadOnly` for config, `BlueprintReadWrite` for runtime state
- Cache expensive calculations with dirty flags
- Use TMap for O(1) lookups instead of TArray linear search
- Always validate Data Assets with `IsDataValid()`
- Log trading activity for debugging

**Performance Targets (MVP):**
- Market lookup: <0.01ms
- Price calculation: <0.05ms
- Inventory query: <0.1ms
- Full route analysis: <100ms

---

**Version**: 1.0  
**Created**: 2026-01-17  
**For**: Trade Simulator MVP
