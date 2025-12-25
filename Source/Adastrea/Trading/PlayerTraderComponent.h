#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTraderComponent.generated.h"

// Forward declarations
class UTradeItemDataAsset;
class UMarketDataAsset;
class UCargoComponent;
class UEconomyManager;

/**
 * Player Trader Component
 * Manages player's trading operations, credits, and profit tracking
 * 
 * Usage:
 * 1. Add to player pawn
 * 2. Set initial credits
 * 3. Use BuyItem/SellItem for trading
 * 4. Query with GetCredits, GetProfit, etc.
 */
UCLASS(ClassGroup=(Trading), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerTraderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// ====================
	// PLAYER FINANCES
	// ====================

	// Current credits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trading|Finances", meta=(ClampMin="0"))
	int32 Credits;

	// Starting credits (for profit calculation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trading|Finances", meta=(ClampMin="0"))
	int32 StartingCredits;

	// ====================
	// CONSTRUCTOR
	// ====================

	UPlayerTraderComponent();

	// ====================
	// TRADING OPERATIONS
	// ====================

	/**
	 * Buy item from market
	 * @param Market The market to buy from
	 * @param Item The item to buy
	 * @param Quantity Number of units to buy
	 * @param CargoComponent The cargo component to add items to
	 * @return True if purchase was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Trading|Operations")
	bool BuyItem(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, UCargoComponent* CargoComponent);

	/**
	 * Sell item to market
	 * @param Market The market to sell to
	 * @param Item The item to sell
	 * @param Quantity Number of units to sell
	 * @param CargoComponent The cargo component to remove items from
	 * @return True if sale was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Trading|Operations")
	bool SellItem(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, UCargoComponent* CargoComponent);

	/**
	 * Check if player can afford to buy item
	 * @param Market The market to buy from
	 * @param Item The item to buy
	 * @param Quantity Number of units
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Queries")
	bool CanAfford(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Get total cost for buying item
	 * @param Market The market to buy from
	 * @param Item The item to buy
	 * @param Quantity Number of units
	 * @return Total cost in credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Queries")
	int32 GetBuyCost(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Get total value for selling item
	 * @param Market The market to sell to
	 * @param Item The item to sell
	 * @param Quantity Number of units
	 * @return Total value in credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Queries")
	int32 GetSellValue(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity) const;

	// ====================
	// FINANCE QUERIES
	// ====================

	/**
	 * Get current credits
	 * @return Current credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Finances")
	int32 GetCredits() const { return Credits; }

	/**
	 * Add credits to player
	 * @param Amount Amount to add
	 */
	UFUNCTION(BlueprintCallable, Category="Trading|Finances")
	void AddCredits(int32 Amount);

	/**
	 * Remove credits from player
	 * @param Amount Amount to remove
	 * @return True if successful (had enough credits)
	 */
	UFUNCTION(BlueprintCallable, Category="Trading|Finances")
	bool RemoveCredits(int32 Amount);

	/**
	 * Get total profit (current credits - starting credits)
	 * @return Profit (negative if loss)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Finances")
	int32 GetProfit() const { return Credits - StartingCredits; }

	/**
	 * Get profit percentage
	 * @return Profit as percentage of starting credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading|Finances")
	float GetProfitPercentage() const;

	/**
	 * Reset finances to starting state
	 */
	UFUNCTION(BlueprintCallable, Category="Trading|Finances")
	void ResetFinances();

	// ====================
	// EVENTS
	// ====================

	// Called when item is bought
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemBought, UTradeItemDataAsset*, Item, int32, Quantity, int32, TotalCost);
	UPROPERTY(BlueprintAssignable, Category="Trading|Events")
	FOnItemBought OnItemBought;

	// Called when item is sold
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemSold, UTradeItemDataAsset*, Item, int32, Quantity, int32, TotalValue);
	UPROPERTY(BlueprintAssignable, Category="Trading|Events")
	FOnItemSold OnItemSold;

	// Called when credits change
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreditsChanged, int32, NewCredits, int32, ChangeAmount);
	UPROPERTY(BlueprintAssignable, Category="Trading|Events")
	FOnCreditsChanged OnCreditsChanged;

	// Called when profit milestone reached
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfitMilestone, int32, ProfitAmount);
	UPROPERTY(BlueprintAssignable, Category="Trading|Events")
	FOnProfitMilestone OnProfitMilestone;

private:
	// Get economy manager subsystem
	UEconomyManager* GetEconomyManager() const;

	// Last profit milestone reached (for events)
	int32 LastProfitMilestone;

	// Check and broadcast profit milestones
	void CheckProfitMilestones();
};
