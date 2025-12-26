#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITraderComponent.generated.h"

// Forward declarations
class UMarketDataAsset;
class UTradeItemDataAsset;
class UTradeContractDataAsset;
class UFactionDataAsset;

/**
 * Enum for AI trader personality/strategy
 */
UENUM(BlueprintType)
enum class EAITraderStrategy : uint8
{
	Conservative UMETA(DisplayName = "Conservative"),
	Balanced UMETA(DisplayName = "Balanced"),
	Aggressive UMETA(DisplayName = "Aggressive"),
	Opportunistic UMETA(DisplayName = "Opportunistic"),
	Specialist UMETA(DisplayName = "Specialist"),
	Manipulator UMETA(DisplayName = "Market Manipulator")
};

/**
 * AI trader behavior priorities
 */
UENUM(BlueprintType)
enum class EAITradeBehavior : uint8
{
	MarketMaking UMETA(DisplayName = "Market Making"),
	Arbitrage UMETA(DisplayName = "Arbitrage"),
	RoutePlanning UMETA(DisplayName = "Route Planning"),
	SpeculativeBuying UMETA(DisplayName = "Speculative Buying"),
	EventReaction UMETA(DisplayName = "Event Reaction"),
	Hoarding UMETA(DisplayName = "Hoarding"),
	Dumping UMETA(DisplayName = "Dumping"),
	ContractTrading UMETA(DisplayName = "Contract Trading")
};

/**
 * Trade route information
 */
USTRUCT(BlueprintType)
struct FTradeRoute
{
	GENERATED_BODY()

	// Origin market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trade Route")
	UMarketDataAsset* OriginMarket;

	// Destination market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trade Route")
	UMarketDataAsset* DestinationMarket;

	// Trade item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trade Route")
	UTradeItemDataAsset* TradeItem;

	// Expected profit per unit
	UPROPERTY(BlueprintReadWrite, Category="Trade Route")
	float ProfitPerUnit;

	// Distance between markets
	UPROPERTY(BlueprintReadWrite, Category="Trade Route")
	float Distance;

	// Estimated travel time
	UPROPERTY(BlueprintReadWrite, Category="Trade Route")
	float TravelTime;

	// Route profitability score (higher = better)
	UPROPERTY(BlueprintReadWrite, Category="Trade Route")
	float ProfitabilityScore;

	FTradeRoute()
		: OriginMarket(nullptr)
		, DestinationMarket(nullptr)
		, TradeItem(nullptr)
		, ProfitPerUnit(0.0f)
		, Distance(0.0f)
		, TravelTime(0.0f)
		, ProfitabilityScore(0.0f)
	{}
};

/**
 * AI trader's inventory tracking
 */
USTRUCT(BlueprintType)
struct FAITraderInventory
{
	GENERATED_BODY()

	// Trade item
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	UTradeItemDataAsset* TradeItem;

	// Quantity owned
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	int32 Quantity;

	// Purchase price per unit
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	float PurchasePrice;

	// Market where purchased
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	UMarketDataAsset* PurchaseMarket;

	FAITraderInventory()
		: TradeItem(nullptr)
		, Quantity(0)
		, PurchasePrice(0.0f)
		, PurchaseMarket(nullptr)
	{}
};

/**
 * AI Trader Component
 * Handles automated trading, arbitrage, market manipulation, and route planning
 */
UCLASS(ClassGroup=(Trading), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UAITraderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAITraderComponent();

	// ====================
	// TRADER CONFIGURATION
	// ====================

	// Trader's faction affiliation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config")
	UFactionDataAsset* TraderFaction;

	// Trading strategy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config")
	EAITraderStrategy Strategy;

	// Available trading capital
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config", meta=(ClampMin="0"))
	int32 TradingCapital;

	// Maximum cargo capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config", meta=(ClampMin="0"))
	float CargoCapacity;

	// Average travel speed for route planning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config", meta=(ClampMin="0"))
	float TravelSpeed;

	// Trading skill level (1-10, affects decisions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config", meta=(ClampMin="1", ClampMax="10"))
	int32 TradingSkill;

	// Risk tolerance (0.0 = very risk-averse, 1.0 = risk-seeking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Config", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RiskTolerance;

	// ====================
	// BEHAVIOR SETTINGS
	// ====================

	// Enabled behaviors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior")
	TArray<EAITradeBehavior> EnabledBehaviors;

	// Minimum profit margin to execute trade (percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior", meta=(ClampMin="0.0", ClampMax="10.0"))
	float MinProfitMargin;

	// Whether this trader can manipulate market prices
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior")
	bool bCanManipulatePrices;

	// Whether this trader operates in black markets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior")
	bool bOperatesInBlackMarkets;

	// Whether this trader accepts contracts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior")
	bool bAcceptsContracts;

	// Frequency of trade operations per game day
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Trader|Behavior", meta=(ClampMin="1"))
	int32 TradeFrequency;

	// ====================
	// TRADER STATE
	// ====================

	// Current inventory
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	TArray<FAITraderInventory> Inventory;

	// Known markets
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	TArray<UMarketDataAsset*> KnownMarkets;

	// Active trade routes
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	TArray<FTradeRoute> ActiveRoutes;

	// Active contracts
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	TArray<UTradeContractDataAsset*> ActiveContracts;

	// Current location (market)
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	UMarketDataAsset* CurrentLocation;

	// Total profit earned
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	int32 TotalProfit;

	// Number of successful trades
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|State")
	int32 SuccessfulTrades;

	// ====================
	// RELATIONSHIPS
	// ====================

	// Friendly traders (cooperation)
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|Relationships")
	TArray<UAITraderComponent*> FriendlyTraders;

	// Rival traders (competition)
	UPROPERTY(BlueprintReadWrite, Category="AI Trader|Relationships")
	TArray<UAITraderComponent*> RivalTraders;

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Initialize the AI trader with starting parameters
	 * @param StartingCapital Initial trading capital
	 * @param StartingMarket Market where trader begins
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader")
	void Initialize(int32 StartingCapital, UMarketDataAsset* StartingMarket);

	/**
	 * Update AI trader logic (should be called regularly)
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader")
	void UpdateTrader(float DeltaTime);

	/**
	 * Find best trade routes between known markets
	 * @param MaxRoutes Maximum number of routes to return
	 * @return Array of profitable trade routes
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Route Planning")
	TArray<FTradeRoute> FindBestTradeRoutes(int32 MaxRoutes = 5);

	/**
	 * Calculate arbitrage opportunities
	 * @param TradeItem Item to check for arbitrage
	 * @return Best arbitrage route (or empty if none profitable)
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Arbitrage")
	FTradeRoute CalculateArbitrageOpportunity(UTradeItemDataAsset* TradeItem);

	/**
	 * Execute a trade at current location
	 * @param TradeItem Item to trade
	 * @param Quantity Quantity to trade
	 * @param bIsBuying True if buying, false if selling
	 * @return True if trade was successful
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Trading")
	bool ExecuteTrade(UTradeItemDataAsset* TradeItem, int32 Quantity, bool bIsBuying);

	/**
	 * Evaluate whether to accept a contract
	 * @param Contract The contract to evaluate
	 * @return True if contract should be accepted
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Contracts")
	bool EvaluateContract(UTradeContractDataAsset* Contract);

	/**
	 * Accept a contract
	 * @param Contract The contract to accept
	 * @return True if successfully accepted
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Contracts")
	bool AcceptContract(UTradeContractDataAsset* Contract);

	/**
	 * Attempt market manipulation (buy/sell to influence prices)
	 * @param TargetMarket Market to manipulate
	 * @param TradeItem Item to manipulate
	 * @param DesiredPriceChange Desired price change direction (positive = increase, negative = decrease)
	 * @return True if manipulation was executed
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Manipulation")
	bool AttemptMarketManipulation(UMarketDataAsset* TargetMarket, UTradeItemDataAsset* TradeItem, float DesiredPriceChange);

	/**
	 * React to market events
	 * @param Market The market where event occurred
	 * @param EventID The event identifier
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Events")
	void ReactToMarketEvent(UMarketDataAsset* Market, FName EventID);

	/**
	 * React to player actions
	 * @param Market The market where player traded
	 * @param TradeItem Item the player traded
	 * @param Quantity Quantity traded
	 * @param bPlayerBought True if player bought (AI may want to sell)
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Events")
	void ReactToPlayerAction(UMarketDataAsset* Market, UTradeItemDataAsset* TradeItem, int32 Quantity, bool bPlayerBought);

	/**
	 * Travel to another market
	 * @param DestinationMarket Market to travel to
	 * @return Estimated travel time
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Movement")
	float TravelToMarket(UMarketDataAsset* DestinationMarket);

	/**
	 * Get current cargo space usage
	 * @return Percentage of cargo space used (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Inventory")
	float GetCargoUsage() const;

	/**
	 * Get available cargo space
	 * @return Available cargo space
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Inventory")
	float GetAvailableCargoSpace() const;

	/**
	 * Check if behavior is enabled
	 * @param Behavior The behavior to check
	 * @return True if behavior is enabled
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Behavior")
	bool IsBehaviorEnabled(EAITradeBehavior Behavior) const;

	/**
	 * Add a market to known markets
	 * @param Market Market to add
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Knowledge")
	void DiscoverMarket(UMarketDataAsset* Market);

	/**
	 * Check if trader knows about a market
	 * @param Market Market to check
	 * @return True if market is known
	 */
	UFUNCTION(BlueprintCallable, Category="AI Trader|Knowledge")
	bool IsMarketKnown(UMarketDataAsset* Market) const;

	/**
	 * BlueprintNativeEvent: Custom trade decision logic
	 * Allows designers to implement custom AI logic
	 * @param TradeItem Item being considered
	 * @param Market Market where trade would occur
	 * @param bIsBuying True if buying
	 * @return True if AI should execute trade
	 */
	UFUNCTION(BlueprintNativeEvent, Category="AI Trader|Events")
	bool OnEvaluateTrade(UTradeItemDataAsset* TradeItem, UMarketDataAsset* Market, bool bIsBuying);

	/**
	 * BlueprintNativeEvent: Called when trade is executed
	 * @param TradeItem Item traded
	 * @param Quantity Amount traded
	 * @param Price Price per unit
	 * @param bWasBuying True if AI bought
	 */
	UFUNCTION(BlueprintNativeEvent, Category="AI Trader|Events")
	void OnTradeExecuted(UTradeItemDataAsset* TradeItem, int32 Quantity, float Price, bool bWasBuying);

	/**
	 * BlueprintNativeEvent: Called when contract is completed
	 * @param Contract The completed contract
	 * @param Profit Profit from contract
	 */
	UFUNCTION(BlueprintNativeEvent, Category="AI Trader|Events")
	void OnContractCompleted(UTradeContractDataAsset* Contract, int32 Profit);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Internal trade decision making
	void MakeTradeDecisions();

	// Internal route optimization
	void OptimizeTradeRoutes();

	// Internal inventory management
	void ManageInventory();

	// Update interval timer
	float UpdateTimer;
};
