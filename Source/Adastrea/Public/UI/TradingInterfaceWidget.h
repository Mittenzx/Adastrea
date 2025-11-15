// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Trading/TradeItemDataAsset.h"
#include "TradingInterfaceWidget.generated.h"

// Forward declarations
class UTradeTransaction;
class UFactionDataAsset;

/**
 * Trading Interface Widget for improved trade UI
 * 
 * This enhanced trading widget provides:
 * - Price comparison and history tracking
 * - Bulk trading operations
 * - Quick filters and sorting
 * - Trade route suggestions
 * - Market trend indicators
 * 
 * Usage:
 * 1. Create a Blueprint Widget based on this class
 * 2. Design the enhanced UI in UMG
 * 3. Call SetTradePartner to initiate trading
 * 4. Use filtering and sorting functions for better UX
 * 
 * Example:
 * - Player initiates trade with station/NPC
 * - Widget displays available items with trends
 * - Player uses bulk operations for efficiency
 * - Automatic price updates based on faction relations
 */
UCLASS()
class ADASTREA_API UTradingInterfaceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTradingInterfaceWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ========================================================================
	// Trade Partner
	// ========================================================================

	/** The faction we're trading with */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	UFactionDataAsset* TradePartnerFaction;

	/** Trade partner's available items */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	TArray<UTradeItemDataAsset*> AvailableItems;

	// ========================================================================
	// UI State
	// ========================================================================

	/** Currently selected item category filter */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	FString SelectedCategory;

	/** Current sort mode (Name, Price, Quantity, etc.) */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	FString SortMode;

	/** Whether to show buy or sell view */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	bool bShowBuyView;

	/** Items in the current transaction cart */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	TMap<UTradeItemDataAsset*, int32> TransactionCart;

	// ========================================================================
	// Blueprint Events
	// ========================================================================

	/**
	 * Event fired when trade items are updated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnTradeItemsUpdated();

	/**
	 * Event fired when a trade is completed
	 * @param bSuccess Whether the trade was successful
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnTradeCompleted(bool bSuccess);

	/**
	 * Event fired when cart contents change
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnCartUpdated();

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Set the trading partner faction
	 * @param Faction The faction to trade with
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetTradePartner(UFactionDataAsset* Faction);

	/**
	 * Get all available items filtered by category
	 * @param Category Category to filter by (empty = all)
	 * @return Array of trade items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	TArray<UTradeItemDataAsset*> GetFilteredItems(const FString& Category) const;

	/**
	 * Get sorted items based on current sort mode
	 * @return Sorted array of trade items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	TArray<UTradeItemDataAsset*> GetSortedItems() const;

	/**
	 * Get the price for an item considering faction relations
	 * @param Item The item to price
	 * @param Quantity Number of items
	 * @param bBuying Whether player is buying (true) or selling (false)
	 * @return Final price
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	float GetItemPrice(UTradeItemDataAsset* Item, int32 Quantity, bool bBuying) const;

	/**
	 * Get price trend indicator for an item (-1 = down, 0 = stable, 1 = up)
	 * @param Item The item to check
	 * @return Price trend indicator
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetPriceTrend(UTradeItemDataAsset* Item) const;

	/**
	 * Add item to transaction cart
	 * @param Item The item to add
	 * @param Quantity Number of items to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void AddToCart(UTradeItemDataAsset* Item, int32 Quantity);

	/**
	 * Remove item from transaction cart
	 * @param Item The item to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void RemoveFromCart(UTradeItemDataAsset* Item);

	/**
	 * Clear all items from cart
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void ClearCart();

	/**
	 * Get total value of items in cart
	 * @return Total cart value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	float GetCartTotal() const;

	/**
	 * Execute the trade transaction
	 * @return True if trade was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	bool ExecuteTrade();

	/**
	 * Set category filter
	 * @param Category Category name
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetCategoryFilter(const FString& Category);

	/**
	 * Set sort mode
	 * @param Mode Sort mode string (Name, Price, Quantity)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetSortMode(const FString& Mode);

	/**
	 * Toggle between buy and sell view
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void ToggleBuySellView();

	/**
	 * Get suggested trade routes based on current items
	 * @return Array of suggested destination factions
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	TArray<UFactionDataAsset*> GetSuggestedTradeRoutes() const;

	/**
	 * Close the trading interface
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void CloseInterface();

private:
	/** Refresh available items from trade partner */
	void RefreshAvailableItems();

	/** Calculate price modifier based on faction relations */
	float GetFactionPriceModifier() const;

	/** Timer handle for price updates */
	FTimerHandle PriceUpdateTimer;
};
