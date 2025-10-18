# Trading System - API Reference

## Overview
Complete API reference for the AdAstrea Trading System. This document covers all public classes, functions, events, and data structures.

---

## Table of Contents
1. [UTradeItemDataAsset](#utradeitemadasset)
2. [UMarketDataAsset](#umarketdataasset)
3. [UTradeContractDataAsset](#utradecontractdataasset)
4. [UAITraderComponent](#uaitradercomponent)
5. [UTradeTransactionManager](#utradetransactionmanager)
6. [Enums](#enums)
7. [Structs](#structs)

---

## UTradeItemDataAsset

**Inherits**: `UPrimaryDataAsset`

**Purpose**: Defines tradeable items with pricing, legality, and AI behavior

### Properties

#### Basic Info
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
FText ItemName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
FText Description;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
FName ItemID;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
ETradeItemCategory Category;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
UMaterialDataAsset* MaterialData;
```

#### Pricing
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
float BasePrice;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
FPriceVolatility PriceVolatility;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
bool bAffectedBySupplyDemand;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
bool bAffectedByMarketEvents;
```

#### Legality
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality")
ELegalityStatus LegalityStatus;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality")
FTradeRestrictions TradeRestrictions;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality")
float ContrabandFineMultiplier;
```

#### Trade Properties
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties")
float VolumePerUnit;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties")
float MassPerUnit;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties")
int32 StandardLotSize;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties")
int32 TypicalMarketStock;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties")
int32 ReplenishmentRate;
```

#### AI Behavior
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
int32 AITradePriority;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
bool bAIHoardable;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
bool bAIArbitrageEnabled;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
TArray<FName> BehaviorTags;
```

### Functions

#### CalculatePrice
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Pricing")
float CalculatePrice(float Supply, float Demand, float MarketEventMultiplier = 1.0f) const;
```
**Description**: Calculate current price based on market conditions  
**Parameters**:
- `Supply`: Current supply level (0.0 to 1.0+, where 1.0 is typical)
- `Demand`: Current demand level (0.0 to 1.0+, where 1.0 is typical)
- `MarketEventMultiplier`: Additional price multiplier from events (1.0 = no effect)

**Returns**: Calculated price in credits

**Example**:
```cpp
float CurrentPrice = TradeItem->CalculatePrice(0.5f, 1.5f, 1.2f);
// Supply is low (0.5), demand is high (1.5), event multiplier (1.2)
```

#### GetFactionModifiedPrice
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Pricing")
float GetFactionModifiedPrice(float BasePriceToModify, FName BuyerFactionID, FName SellerFactionID) const;
```
**Description**: Get price after applying faction relationships  
**Returns**: Modified price based on faction relationships

#### CanBeTradedByFaction
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
bool CanBeTradedByFaction(FName FactionID, int32 Reputation) const;
```
**Description**: Check if item can be traded by a specific faction  
**Returns**: True if trade is allowed

#### RequiresPermit
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
bool RequiresPermit(FName FactionID) const;
```
**Description**: Check if item requires a permit for trading  
**Returns**: True if permit is required

#### CalculateContrabandFine
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
float CalculateContrabandFine(int32 Quantity) const;
```
**Description**: Calculate fine for illegal trade if caught  
**Returns**: Fine amount in credits

#### HasBehaviorTag
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Tags")
bool HasBehaviorTag(FName Tag) const;
```
**Description**: Check if this item has a specific behavior tag  
**Returns**: True if item has the tag

#### GetTotalVolume / GetTotalMass
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
float GetTotalVolume(int32 Quantity) const;

UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
float GetTotalMass(int32 Quantity) const;
```
**Description**: Get total volume or mass for a quantity  
**Returns**: Total volume in cubic meters or mass in kilograms

#### IsHighValue
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
bool IsHighValue() const;
```
**Description**: Check if this item is considered high-value  
**Returns**: True if base price is above luxury threshold

### Events

#### OnCalculateCustomPrice
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Trade Item|Events")
float OnCalculateCustomPrice(float Supply, float Demand, float EventMultiplier, float BaseCalculatedPrice) const;
```
**Description**: Custom price calculation override  
**Usage**: Implement in Blueprint to add custom pricing logic

**Example**:
```cpp
// In Blueprint
Event OnCalculateCustomPrice
  Input: Supply, Demand, EventMultiplier, BaseCalculatedPrice
  
  // Add 10% discount for bulk orders
  If Quantity > 100
    Return BaseCalculatedPrice * 0.9
  Else
    Return BaseCalculatedPrice
```

#### OnItemTraded
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Trade Item|Events")
void OnItemTraded(int32 Quantity, float Price, FName BuyerFactionID, FName SellerFactionID);
```
**Description**: Called when this item is traded  
**Usage**: Trigger custom events on trade

---

## UMarketDataAsset

**Inherits**: `UPrimaryDataAsset`

**Purpose**: Defines market behavior, inventory, and pricing

### Properties

#### Basic Info
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
FText MarketName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
FName MarketID;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
EMarketType MarketType;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
EMarketSize MarketSize;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
UFactionDataAsset* ControllingFaction;
```

#### Market Configuration
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
float TransactionTaxRate;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
float SellPriceMarkup;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
float BuyPriceMarkdown;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
bool bAllowPlayerBuying;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
bool bAllowPlayerSelling;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
bool bAllowAITraders;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
int32 MinReputationRequired;
```

#### Inventory
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
TArray<FMarketInventoryEntry> Inventory;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
float StockRefreshRate;

UPROPERTY(BlueprintReadWrite, Category="Inventory")
float LastStockRefreshTime;
```

#### Market Events
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Events")
TArray<FMarketEvent> ActiveEvents;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Events")
float RandomEventChance;
```

#### AI Traders
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders")
int32 AITraderCount;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders")
int32 AITradeFrequency;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders")
bool bAllowAIPriceManipulation;
```

### Functions

#### GetItemPrice
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Pricing")
float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;
```
**Description**: Get current price for an item (buy or sell)  
**Parameters**:
- `TradeItem`: The item to get price for
- `bIsBuying`: True if player is buying, false if selling

**Returns**: Current price per unit

#### GetInventoryEntry
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Inventory")
bool GetInventoryEntry(FName ItemID, FMarketInventoryEntry& OutEntry) const;
```
**Description**: Get inventory entry for a specific item  
**Returns**: True if item was found in inventory

#### IsItemInStock
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Inventory")
bool IsItemInStock(FName ItemID, int32 Quantity) const;
```
**Description**: Check if an item is in stock  
**Returns**: True if sufficient stock is available

#### GetItemsByCategory
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Inventory")
TArray<FMarketInventoryEntry> GetItemsByCategory(ETradeItemCategory Category) const;
```
**Description**: Get all items in a specific category  
**Returns**: Array of inventory entries matching category

#### CanPlayerAccess
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Access")
bool CanPlayerAccess(int32 PlayerReputation) const;
```
**Description**: Check if player has sufficient reputation to access market  
**Returns**: True if player can access market

#### GetActiveEventsForItem
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Events")
TArray<FMarketEvent> GetActiveEventsForItem(FName ItemID) const;
```
**Description**: Get active market events affecting a specific item  
**Returns**: Array of active events affecting this item

#### GetEventPriceMultiplier
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Events")
float GetEventPriceMultiplier(FName ItemID) const;
```
**Description**: Calculate total price multiplier from all active events for an item  
**Returns**: Combined price multiplier from all events

#### UpdateMarket
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Update")
void UpdateMarket(float DeltaHours);
```
**Description**: Update market inventory and prices based on elapsed time

#### RefreshStock
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Update")
void RefreshStock();
```
**Description**: Refresh stock levels for all items

#### UpdateMarketEvents
```cpp
UFUNCTION(BlueprintCallable, Category="Market|Events")
void UpdateMarketEvents(float CurrentGameTime);
```
**Description**: Process market event updates (check expiration, etc.)

### Events

#### OnCalculateCustomMarketPrice
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
float OnCalculateCustomMarketPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying, float BasePrice) const;
```
**Description**: Custom price calculation override for market-specific pricing

#### OnStockRefreshed
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
void OnStockRefreshed();
```
**Description**: Called when market stock is refreshed

#### OnMarketEventStarted / OnMarketEventEnded
```cpp
UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
void OnMarketEventStarted(const FMarketEvent& Event);

UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
void OnMarketEventEnded(const FMarketEvent& Event);
```
**Description**: Called when market events start or end

---

## UTradeContractDataAsset

**Inherits**: `UPrimaryDataAsset`

**Purpose**: Defines trade contracts and missions

### Key Functions

#### CanPlayerAccept
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Contract")
bool CanPlayerAccept(int32 PlayerReputation, float PlayerCargoCapacity) const;
```

#### GetTotalCargoVolume / GetTotalCargoMass
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Contract")
float GetTotalCargoVolume() const;

UFUNCTION(BlueprintCallable, Category="Trade Contract")
float GetTotalCargoMass() const;
```

#### GetEstimatedCompletionTime
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Contract")
float GetEstimatedCompletionTime(float AverageSpeed) const;
```

#### CalculateProfitMargin
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Contract")
int32 CalculateProfitMargin(int32 CargoAcquisitionCost, int32 FuelCost) const;
```

#### AcceptContract / CompleteContract / FailContract
```cpp
UFUNCTION(BlueprintCallable, Category="Trade Contract")
bool AcceptContract(FName PlayerID, float CurrentGameTime);

UFUNCTION(BlueprintCallable, Category="Trade Contract")
bool CompleteContract(float CurrentGameTime);

UFUNCTION(BlueprintCallable, Category="Trade Contract")
bool FailContract(const FText& Reason);
```

---

## UAITraderComponent

**Inherits**: `UActorComponent`

**Purpose**: AI trading logic and automation

### Key Functions

#### Initialize
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader")
void Initialize(int32 StartingCapital, UMarketDataAsset* StartingMarket);
```

#### FindBestTradeRoutes
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader|Route Planning")
TArray<FTradeRoute> FindBestTradeRoutes(int32 MaxRoutes = 5);
```

#### CalculateArbitrageOpportunity
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader|Arbitrage")
FTradeRoute CalculateArbitrageOpportunity(UTradeItemDataAsset* TradeItem);
```

#### ExecuteTrade
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader|Trading")
bool ExecuteTrade(UTradeItemDataAsset* TradeItem, int32 Quantity, bool bIsBuying);
```

#### AttemptMarketManipulation
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader|Manipulation")
bool AttemptMarketManipulation(UMarketDataAsset* TargetMarket, UTradeItemDataAsset* TradeItem, float DesiredPriceChange);
```

#### ReactToMarketEvent / ReactToPlayerAction
```cpp
UFUNCTION(BlueprintCallable, Category="AI Trader|Events")
void ReactToMarketEvent(UMarketDataAsset* Market, FName EventID);

UFUNCTION(BlueprintCallable, Category="AI Trader|Events")
void ReactToPlayerAction(UMarketDataAsset* Market, UTradeItemDataAsset* TradeItem, int32 Quantity, bool bPlayerBought);
```

---

## UTradeTransactionManager

**Purpose**: Track and analyze transaction history

### Key Functions

#### RecordTransaction
```cpp
UFUNCTION(BlueprintCallable, Category="Transaction History")
void RecordTransaction(const FTradeTransaction& Transaction);
```

#### Query Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Transaction History")
TArray<FTradeTransaction> GetTransactionsByItem(FName ItemID) const;

UFUNCTION(BlueprintCallable, Category="Transaction History")
TArray<FTradeTransaction> GetTransactionsByMarket(UMarketDataAsset* Market) const;

UFUNCTION(BlueprintCallable, Category="Transaction History")
TArray<FTradeTransaction> GetTransactionsByTrader(FName TraderID) const;

UFUNCTION(BlueprintCallable, Category="Transaction History")
TArray<FTradeTransaction> GetTransactionsByTimeRange(float StartTime, float EndTime) const;
```

#### Analytics Functions
```cpp
UFUNCTION(BlueprintCallable, Category="Transaction History|Analytics")
int32 GetTotalTradeVolume(FName ItemID, float StartTime, float EndTime) const;

UFUNCTION(BlueprintCallable, Category="Transaction History|Analytics")
float GetAveragePrice(FName ItemID, float StartTime, float EndTime) const;

UFUNCTION(BlueprintCallable, Category="Transaction History|Analytics")
float GetPriceTrend(FName ItemID, float TimeWindow) const;

UFUNCTION(BlueprintCallable, Category="Transaction History|Analytics")
TArray<FName> GetMostTradedItems(int32 Count, float StartTime, float EndTime) const;

UFUNCTION(BlueprintCallable, Category="Transaction History|Analytics")
int32 GetPlayerProfitLoss(FName PlayerID) const;
```

---

## Enums

### ETradeItemCategory
```cpp
UENUM(BlueprintType)
enum class ETradeItemCategory : uint8
{
    RawMaterials,
    RefinedGoods,
    Components,
    Technology,
    Food,
    Luxury,
    Contraband,
    Military,
    Medical,
    Data,
    Other
};
```

### ELegalityStatus
```cpp
UENUM(BlueprintType)
enum class ELegalityStatus : uint8
{
    Legal,
    Regulated,
    Restricted,
    Illegal,
    Contraband
};
```

### EMarketType
```cpp
UENUM(BlueprintType)
enum class EMarketType : uint8
{
    OpenMarket,
    BlackMarket,
    FactionExclusive,
    CommodityExchange,
    LuxuryBazaar,
    MilitarySupply,
    ResearchHub,
    IndustrialDepot
};
```

### EAITraderStrategy
```cpp
UENUM(BlueprintType)
enum class EAITraderStrategy : uint8
{
    Conservative,
    Balanced,
    Aggressive,
    Opportunistic,
    Specialist,
    Manipulator
};
```

### EAITradeBehavior
```cpp
UENUM(BlueprintType)
enum class EAITradeBehavior : uint8
{
    MarketMaking,
    Arbitrage,
    RoutePlanning,
    SpeculativeBuying,
    EventReaction,
    Hoarding,
    Dumping,
    ContractTrading
};
```

---

## Structs

### FPriceVolatility
```cpp
USTRUCT(BlueprintType)
struct FPriceVolatility
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolatilityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinPriceDeviation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxPriceDeviation;
};
```

### FMarketEvent
```cpp
USTRUCT(BlueprintType)
struct FMarketEvent
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AffectedItemIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PriceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SupplyMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DemandMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationHours;

    UPROPERTY(BlueprintReadWrite)
    float StartTime;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive;
};
```

### FTradeRoute
```cpp
USTRUCT(BlueprintType)
struct FTradeRoute
{
    UPROPERTY(BlueprintReadWrite)
    UMarketDataAsset* OriginMarket;

    UPROPERTY(BlueprintReadWrite)
    UMarketDataAsset* DestinationMarket;

    UPROPERTY(BlueprintReadWrite)
    UTradeItemDataAsset* TradeItem;

    UPROPERTY(BlueprintReadWrite)
    float ProfitPerUnit;

    UPROPERTY(BlueprintReadWrite)
    float Distance;

    UPROPERTY(BlueprintReadWrite)
    float TravelTime;

    UPROPERTY(BlueprintReadWrite)
    float ProfitabilityScore;
};
```

### FTradeTransaction
```cpp
USTRUCT(BlueprintType)
struct FTradeTransaction
{
    UPROPERTY(BlueprintReadWrite)
    FGuid TransactionID;

    UPROPERTY(BlueprintReadWrite)
    ETransactionType TransactionType;

    UPROPERTY(BlueprintReadWrite)
    UTradeItemDataAsset* TradeItem;

    UPROPERTY(BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(BlueprintReadWrite)
    float PricePerUnit;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalValue;

    // ... many more fields (see TradeTransaction.h)
};
```

---

## Usage Examples

### Example 1: Create and Price Trade Item

```cpp
// C++
UTradeItemDataAsset* IronOre = NewObject<UTradeItemDataAsset>();
IronOre->ItemName = FText::FromString(TEXT("Iron Ore"));
IronOre->BasePrice = 50.0f;
IronOre->Category = ETradeItemCategory::RawMaterials;

float CurrentPrice = IronOre->CalculatePrice(0.8f, 1.2f, 1.0f);
// Supply slightly low, demand slightly high
```

### Example 2: Execute Trade

```cpp
// C++
UMarketDataAsset* Market = GetCurrentMarket();
UTradeItemDataAsset* Item = GetSelectedItem();
int32 Quantity = 100;

float BuyPrice = Market->GetItemPrice(Item, true);
int32 TotalCost = FMath::RoundToInt(BuyPrice * Quantity);

if (PlayerCredits >= TotalCost)
{
    PlayerCredits -= TotalCost;
    AddToPlayerInventory(Item, Quantity);
    
    // Record transaction
    FTradeTransaction Transaction;
    Transaction.TransactionType = ETransactionType::Buy;
    Transaction.TradeItem = Item;
    Transaction.Quantity = Quantity;
    Transaction.PricePerUnit = BuyPrice;
    Transaction.TotalValue = TotalCost;
    Transaction.BuyerID = PlayerID;
    Transaction.SellerID = Market->MarketID;
    Transaction.Market = Market;
    Transaction.Timestamp = GetGameTime();
    
    TransactionManager->RecordTransaction(Transaction);
}
```

### Example 3: AI Trader Setup

```cpp
// C++
UAITraderComponent* AITrader = NewObject<UAITraderComponent>(ShipActor);
AITrader->RegisterComponent();

AITrader->Strategy = EAITraderStrategy::Aggressive;
AITrader->TradingCapital = 100000;
AITrader->CargoCapacity = 1500.0f;
AITrader->TradingSkill = 8;
AITrader->RiskTolerance = 0.8f;
AITrader->MinProfitMargin = 0.20f;

AITrader->EnabledBehaviors.Add(EAITradeBehavior::Arbitrage);
AITrader->EnabledBehaviors.Add(EAITradeBehavior::MarketMaking);
AITrader->EnabledBehaviors.Add(EAITradeBehavior::EventReaction);

AITrader->Initialize(100000, StartingMarket);
```

---

## Best Practices

1. **Always validate inputs**: Check for null references before using Data Assets
2. **Cache frequently accessed data**: Store current market, player inventory in local variables
3. **Use events for extensibility**: Override BlueprintNativeEvents for custom behavior
4. **Profile AI traders**: Monitor performance with many active AI traders
5. **Balance update frequency**: Update prices/stock at reasonable intervals, not every frame
6. **Save transaction history**: Persist important data for analytics and debugging

---

## Version History

- **v1.0**: Initial release with core trading functionality
- **v1.1**: Added AI trader behaviors
- **v1.2**: Added market events system
- **v1.3**: Added contract system

---

## Support

For issues or questions:
- Check TradingSystemGuide.md for detailed documentation
- Review TradingBlueprintExamples.md for usage examples
- Consult YAML templates in Assets/TradingTemplates/
