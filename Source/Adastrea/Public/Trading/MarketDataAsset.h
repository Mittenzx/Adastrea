#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MarketDataAsset.generated.h"

// Forward declarations
class UTradeItemDataAsset;
class UFactionDataAsset;

/**
 * Enum for market types
 * Defines the general type and behavior of the market
 */
UENUM(BlueprintType)
enum class EMarketType : uint8
{
	OpenMarket UMETA(DisplayName = "Open Market"),
	BlackMarket UMETA(DisplayName = "Black Market"),
	FactionExclusive UMETA(DisplayName = "Faction Exclusive"),
	CommodityExchange UMETA(DisplayName = "Commodity Exchange"),
	LuxuryBazaar UMETA(DisplayName = "Luxury Bazaar"),
	MilitarySupply UMETA(DisplayName = "Military Supply"),
	ResearchHub UMETA(DisplayName = "Research Hub"),
	IndustrialDepot UMETA(DisplayName = "Industrial Depot")
};

/**
 * Enum for market size/tier
 * Affects stock levels and variety
 */
UENUM(BlueprintType)
enum class EMarketSize : uint8
{
	Small UMETA(DisplayName = "Small Outpost"),
	Medium UMETA(DisplayName = "Medium Station"),
	Large UMETA(DisplayName = "Large Hub"),
	Megacity UMETA(DisplayName = "Megacity Exchange"),
	Capital UMETA(DisplayName = "Capital World Market")
};

/**
 * Current market event affecting prices
 */
USTRUCT(BlueprintType)
struct FMarketEvent
{
	GENERATED_BODY()

	// Display name of the event
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event")
	FText EventName;

	// Description of the event
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event", meta=(MultiLine=true))
	FText EventDescription;

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event")
	FName EventID;

	// Items affected by this event (empty = all items)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event")
	TArray<FName> AffectedItemIDs;

	// Price multiplier for affected items (1.0 = no change)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event", meta=(ClampMin="0.1", ClampMax="10.0"))
	float PriceMultiplier;

	// Supply multiplier for affected items (1.0 = no change)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event", meta=(ClampMin="0.0", ClampMax="5.0"))
	float SupplyMultiplier;

	// Demand multiplier for affected items (1.0 = no change)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event", meta=(ClampMin="0.0", ClampMax="5.0"))
	float DemandMultiplier;

	// Duration of event in game hours (0 = infinite)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Event", meta=(ClampMin="0"))
	float DurationHours;

	// When event started (game time)
	UPROPERTY(BlueprintReadWrite, Category="Market Event")
	float StartTime;

	// Whether event is currently active
	UPROPERTY(BlueprintReadWrite, Category="Market Event")
	bool bIsActive;

	FMarketEvent()
		: EventName(FText::FromString(TEXT("Market Event")))
		, EventDescription(FText::FromString(TEXT("A market event is occurring.")))
		, EventID(NAME_None)
		, PriceMultiplier(1.0f)
		, SupplyMultiplier(1.0f)
		, DemandMultiplier(1.0f)
		, DurationHours(24.0f)
		, StartTime(0.0f)
		, bIsActive(false)
	{}
};

/**
 * Market inventory entry tracking stock and pricing
 */
USTRUCT(BlueprintType)
struct FMarketInventoryEntry
{
	GENERATED_BODY()

	// The trade item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	UTradeItemDataAsset* TradeItem;

	// Current stock level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="0"))
	int32 CurrentStock;

	// Maximum stock level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta=(ClampMin="0"))
	int32 MaxStock;

	// Base supply level (1.0 = typical)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="0.0"))
	float SupplyLevel;

	// Base demand level (1.0 = typical)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="0.0"))
	float DemandLevel;

	// Last price this was sold at
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	float LastTradePrice;

	// Whether this item is currently in stock
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	bool bInStock;

	FMarketInventoryEntry()
		: TradeItem(nullptr)
		, CurrentStock(0)
		, MaxStock(1000)
		, SupplyLevel(1.0f)
		, DemandLevel(1.0f)
		, LastTradePrice(0.0f)
		, bInStock(false)
	{}
};

/**
 * Market configuration data asset
 * Defines market behavior, inventory, and pricing
 */
UCLASS(BlueprintType)
class ADASTREA_API UMarketDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// BASIC INFO
	// ====================

	// Display name of the market
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
	FText MarketName;

	// Description of the market
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info", meta=(MultiLine=true))
	FText Description;

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
	FName MarketID;

	// Market type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
	EMarketType MarketType;

	// Market size/tier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
	EMarketSize MarketSize;

	// Faction that controls this market
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Info")
	UFactionDataAsset* ControllingFaction;

	// ====================
	// MARKET CONFIGURATION
	// ====================

	// Base transaction tax percentage (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TransactionTaxRate;

	// Price markup for selling (1.0 = base price, 1.2 = 20% markup)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config", meta=(ClampMin="1.0", ClampMax="3.0"))
	float SellPriceMarkup;

	// Price markdown for buying (1.0 = base price, 0.8 = 20% below base)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config", meta=(ClampMin="0.1", ClampMax="1.0"))
	float BuyPriceMarkdown;

	// Whether this market allows player buying
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
	bool bAllowPlayerBuying;

	// Whether this market allows player selling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
	bool bAllowPlayerSelling;

	// Whether AI traders can use this market
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config")
	bool bAllowAITraders;

	// Reputation requirement to access market (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Config", meta=(ClampMin="-100", ClampMax="100"))
	int32 MinReputationRequired;

	// ====================
	// INVENTORY
	// ====================

	// Items available in this market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<FMarketInventoryEntry> Inventory;

	// Stock refresh rate in game hours
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta=(ClampMin="0"))
	float StockRefreshRate;

	// Last stock refresh time
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	float LastStockRefreshTime;

	// ====================
	// MARKET EVENTS
	// ====================

	// Active market events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Market Events")
	TArray<FMarketEvent> ActiveEvents;

	// Random event chance per day (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Market Events", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RandomEventChance;

	// ====================
	// AI TRADER SETTINGS
	// ====================

	// Number of AI traders operating in this market
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders", meta=(ClampMin="0"))
	int32 AITraderCount;

	// Frequency of AI trade operations per day
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders", meta=(ClampMin="0"))
	int32 AITradeFrequency;

	// Whether AI can manipulate prices
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Traders")
	bool bAllowAIPriceManipulation;

	// ====================
	// Constructor
	// ====================

	UMarketDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Get current price for an item (buy or sell)
	 * @param TradeItem The item to get price for
	 * @param bIsBuying True if player is buying, false if selling
	 * @return Current price per unit
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Pricing")
	float GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const;

	/**
	 * Get inventory entry for a specific item
	 * @param ItemID The item ID to find
	 * @param OutEntry The found inventory entry
	 * @return True if item was found in inventory
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Inventory")
	bool GetInventoryEntry(FName ItemID, FMarketInventoryEntry& OutEntry) const;

	/**
	 * Check if an item is in stock
	 * @param ItemID The item to check
	 * @param Quantity The quantity needed
	 * @return True if sufficient stock is available
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Inventory")
	bool IsItemInStock(FName ItemID, int32 Quantity) const;

	/**
	 * Get all items in a specific category
	 * @param Category The category to filter by
	 * @return Array of inventory entries matching category
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Inventory")
	TArray<FMarketInventoryEntry> GetItemsByCategory(ETradeItemCategory Category) const;

	/**
	 * Check if player has sufficient reputation to access market
	 * @param PlayerReputation The player's reputation with controlling faction
	 * @return True if player can access market
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Access")
	bool CanPlayerAccess(int32 PlayerReputation) const;

	/**
	 * Get active market events affecting a specific item
	 * @param ItemID The item to check
	 * @return Array of active events affecting this item
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Events")
	TArray<FMarketEvent> GetActiveEventsForItem(FName ItemID) const;

	/**
	 * Calculate total price multiplier from all active events for an item
	 * @param ItemID The item to check
	 * @return Combined price multiplier from all events
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Events")
	float GetEventPriceMultiplier(FName ItemID) const;

	/**
	 * Update market inventory and prices based on elapsed time
	 * @param DeltaHours Hours elapsed since last update
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Update")
	void UpdateMarket(float DeltaHours);

	/**
	 * Refresh stock levels for all items
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Update")
	void RefreshStock();

	/**
	 * Process a market event update (check expiration, etc.)
	 * @param CurrentGameTime Current game time in hours
	 */
	UFUNCTION(BlueprintCallable, Category="Market|Events")
	void UpdateMarketEvents(float CurrentGameTime);

	/**
	 * BlueprintNativeEvent: Custom price calculation override
	 * Allows designers to implement custom pricing logic in Blueprint
	 * @param TradeItem The item being priced
	 * @param bIsBuying Whether this is a buy transaction
	 * @param BasePrice The calculated base price
	 * @return Final price after custom modifications
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
	float OnCalculateCustomMarketPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying, float BasePrice) const;

	/**
	 * BlueprintNativeEvent: Called when market stock is refreshed
	 * Allows designers to customize refresh behavior
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
	void OnStockRefreshed();

	/**
	 * BlueprintNativeEvent: Called when a new market event starts
	 * @param Event The event that started
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
	void OnMarketEventStarted(const FMarketEvent& Event);

	/**
	 * BlueprintNativeEvent: Called when a market event ends
	 * @param Event The event that ended
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Market|Events")
	void OnMarketEventEnded(const FMarketEvent& Event);
};
