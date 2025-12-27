#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TradeTransaction.generated.h"

// Forward declarations
class UTradeItemDataAsset;
class UMarketDataAsset;

/**
 * Enum for transaction types
 */
UENUM(BlueprintType)
enum class ETransactionType : uint8
{
	Buy UMETA(DisplayName = "Buy"),
	Sell UMETA(DisplayName = "Sell"),
	Transfer UMETA(DisplayName = "Transfer"),
	Contract UMETA(DisplayName = "Contract"),
	Contraband UMETA(DisplayName = "Contraband"),
	Gift UMETA(DisplayName = "Gift")
};

/**
 * Trade Transaction Record
 * Tracks individual trade transactions for history and analytics
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FTradeTransaction
{
	GENERATED_BODY()

	// ====================
	// TRANSACTION INFO
	// ====================

	// Unique transaction ID
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FGuid TransactionID;

	// Type of transaction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	ETransactionType TransactionType;

	// Trade item involved
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	UTradeItemDataAsset* TradeItem;

	// Quantity traded
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	int32 Quantity;

	// Price per unit
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	float PricePerUnit;

	// Total transaction value
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	int32 TotalValue;

	// Tax paid
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	int32 TaxPaid;

	// ====================
	// PARTIES INVOLVED
	// ====================

	// Buyer ID (player or AI trader)
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FName BuyerID;

	// Seller ID (player or AI trader)
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FName SellerID;

	// Buyer's faction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FName BuyerFactionID;

	// Seller's faction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FName SellerFactionID;

	// ====================
	// LOCATION & TIME
	// ====================

	// Market where transaction occurred
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	UMarketDataAsset* Market;

	// Location coordinates
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FVector Location;

	// Game time when transaction occurred
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	float Timestamp;

	// Real-world time (for analytics)
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	FDateTime RealTimestamp;

	// ====================
	// MARKET CONDITIONS
	// ====================

	// Supply level at time of transaction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	float SupplyLevel;

	// Demand level at time of transaction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	float DemandLevel;

	// Active market events at time of transaction
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	TArray<FName> ActiveEventIDs;

	// ====================
	// FLAGS
	// ====================

	// Whether transaction was flagged as suspicious
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	bool bFlaggedAsSuspicious;

	// Whether transaction involved contraband
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	bool bInvolvedContraband;

	// Whether player was caught with contraband
	UPROPERTY(BlueprintReadWrite, Category="Transaction")
	bool bCaughtWithContraband;

	// ====================
	// CONSTRUCTOR
	// ====================

	FTradeTransaction()
		: TransactionID(FGuid::NewGuid())
		, TransactionType(ETransactionType::Buy)
		, TradeItem(nullptr)
		, Quantity(0)
		, PricePerUnit(0.0f)
		, TotalValue(0)
		, TaxPaid(0)
		, BuyerID(NAME_None)
		, SellerID(NAME_None)
		, BuyerFactionID(NAME_None)
		, SellerFactionID(NAME_None)
		, Market(nullptr)
		, Location(FVector::ZeroVector)
		, Timestamp(0.0f)
		, RealTimestamp(FDateTime::Now())
		, SupplyLevel(1.0f)
		, DemandLevel(1.0f)
		, bFlaggedAsSuspicious(false)
		, bInvolvedContraband(false)
		, bCaughtWithContraband(false)
	{}
};

/**
 * Transaction History Manager
 * Tracks and queries transaction history
 */
UCLASS(BlueprintType)
class ADASTREA_API UTradeTransactionManager : public UObject
{
	GENERATED_BODY()

public:
	// ====================
	// TRANSACTION HISTORY
	// ====================

	// All recorded transactions
	UPROPERTY(BlueprintReadWrite, Category="Transaction History")
	TArray<FTradeTransaction> TransactionHistory;

	// Maximum history size (older transactions are removed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transaction History", meta=(ClampMin="100"))
	int32 MaxHistorySize;

private:
	// Performance optimization: cached latest timestamp
	// Updated when transactions are recorded to avoid scanning entire history
	float CachedLatestTimestamp;
	
	// Track if cache needs invalidation
	bool bCacheValid;

	// ====================
	// Constructor
	// ====================

	UTradeTransactionManager();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Record a new transaction
	 * @param Transaction The transaction to record
	 */
	UFUNCTION(BlueprintCallable, Category="Transaction History")
	void RecordTransaction(const FTradeTransaction& Transaction);

	/**
	 * Get all transactions for a specific item
	 * @param ItemID The item to search for
	 * @return Array of transactions involving this item
	 */
	UFUNCTION(BlueprintCallable, Category="Transaction History")
	TArray<FTradeTransaction> GetTransactionsByItem(FName ItemID) const;

	/**
	 * Calculate average price for an item over time
	 * @param ItemID The item to calculate for
	 * @param StartTime Start of time range
	 * @param EndTime End of time range
	 * @return Average price per unit
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Transaction History|Analytics")
	float GetAveragePrice(FName ItemID, float StartTime, float EndTime) const;

	/**
	 * Get total player profit/loss
	 * @param PlayerID The player to calculate for
	 * @return Total profit (negative if loss)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Transaction History|Analytics")
	int32 GetPlayerProfitLoss(FName PlayerID) const;

	/**
	 * Clear transaction history
	 */
	UFUNCTION(BlueprintCallable, Category="Transaction History")
	void ClearHistory();

private:
	/**
	 * Get all transactions at a specific market
	 * Internal query function
	 */
	TArray<FTradeTransaction> GetTransactionsByMarket(UMarketDataAsset* Market) const;

	/**
	 * Get all transactions by a specific trader
	 * Internal query function
	 */
	TArray<FTradeTransaction> GetTransactionsByTrader(FName TraderID) const;

	/**
	 * Get transactions within a time range
	 * Internal query function
	 */
	TArray<FTradeTransaction> GetTransactionsByTimeRange(float StartTime, float EndTime) const;

	/**
	 * Calculate total trade volume for an item
	 * Internal analytics function
	 */
	int32 GetTotalTradeVolume(FName ItemID, float StartTime, float EndTime) const;

	/**
	 * Get price trend (increasing, decreasing, or stable)
	 * Internal analytics function
	 */
	float GetPriceTrend(FName ItemID, float TimeWindow) const;

	/**
	 * Get most traded items
	 * Internal analytics function
	 */
	TArray<FName> GetMostTradedItems(int32 Count, float StartTime, float EndTime) const;

	/**
	 * Export transaction history to string (for save/load)
	 * Internal serialization function
	 */
	FString ExportToString() const;

	/**
	 * Import transaction history from string
	 * Internal serialization function
	 */
	bool ImportFromString(const FString& Data);

	// Prune old transactions if history exceeds max size
	void PruneOldTransactions();
};
