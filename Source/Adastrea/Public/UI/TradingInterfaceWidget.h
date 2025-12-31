// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Trading/TradeItemDataAsset.h"
#include "TradingInterfaceWidget.generated.h"

// Forward declarations
class UTradeTransaction;
class UFactionDataAsset;
class UMarketDataAsset;
class UPlayerTraderComponent;
class UCargoComponent;
class UEconomyManager;

/**
 * Trading Interface Widget for MVP Trade Simulator
 * 
 * This trading widget provides essential features for the MVP:
 * - View market inventory with prices
 * - Buy/Sell toggle interface
 * - Shopping cart/transaction system
 * - Player credits and cargo space tracking
 * - Profit calculation
 * - Real-time price updates from EconomyManager
 * 
 * Usage:
 * 1. Create a Blueprint Widget based on this class (WBP_TradingUI)
 * 2. Design the UI in UMG with text blocks, buttons, and lists
 * 3. Call OpenMarket to initiate trading at a station
 * 4. Widget automatically connects to player's trader and cargo components
 * 
 * MVP Requirements Met:
 * - Buy low, sell high core loop
 * - Clear price comparison
 * - Cargo capacity visualization
 * - Profit tracking
 * - Simple, functional UI
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
	// Market & Components
	// ========================================================================

	/** The market we're trading with */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	TObjectPtr<UMarketDataAsset> CurrentMarket;

	/** Player's trader component */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	TObjectPtr<UPlayerTraderComponent> PlayerTrader;

	/** Player's cargo component */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	TObjectPtr<UCargoComponent> PlayerCargo;

	/** Economy manager subsystem */
	UPROPERTY(BlueprintReadOnly, Category = "Trading")
	TObjectPtr<UEconomyManager> EconomyManager;

	// ========================================================================
	// UI State
	// ========================================================================

	/** Currently selected item category filter */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	ETradeItemCategory SelectedCategoryFilter;

	/** Current sort mode (Name, Price, Stock) */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	FString SortMode;

	/** Whether showing buy or sell view */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	bool bShowBuyView;

	/** Items in the current shopping cart */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	TMap<UTradeItemDataAsset*, int32> ShoppingCart;

	/** Selected item for detail view (optional) */
	UPROPERTY(BlueprintReadWrite, Category = "Trading")
	TObjectPtr<UTradeItemDataAsset> SelectedItem;

	// ========================================================================
	// Blueprint Events
	// ========================================================================

	/**
	 * Event fired when market inventory is updated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnMarketInventoryUpdated();

	/**
	 * Event fired when shopping cart contents change
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnCartUpdated();

	/**
	 * Event fired when a trade transaction completes
	 * @param bSuccess Whether the trade was successful
	 * @param ErrorMessage Error message if failed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnTradeCompleted(bool bSuccess, const FText& ErrorMessage);

	/**
	 * Event fired when player credits change
	 * @param NewCredits Updated credit amount
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnCreditsUpdated(int32 NewCredits);

	/**
	 * Event fired when cargo space changes
	 * @param UsedSpace Current used space
	 * @param TotalSpace Total cargo capacity
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trading")
	void OnCargoSpaceUpdated(float UsedSpace, float TotalSpace);

public:
	// ========================================================================
	// Blueprint Functions - Market Setup
	// ========================================================================

	/**
	 * Open trading interface for a specific market
	 * Automatically finds player's trader and cargo components
	 * @param Market The market to trade with
	 * @return True if successfully opened
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	bool OpenMarket(UMarketDataAsset* Market);

	/**
	 * Close the trading interface
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void CloseMarket();

	/**
	 * Set trade partner faction (compatibility method)
	 * This is a wrapper that finds a market controlled by the given faction
	 * @param TradePartner The faction to trade with
	 * @return True if a market was found and opened
	 * 
	 * Note: For MVP, this is a stub that should be connected to actual market lookup
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	bool SetTradePartner(UFactionDataAsset* TradePartner);

	// ========================================================================
	// Blueprint Functions - Market Information
	// ========================================================================

	/**
	 * Get all available items in current market
	 * @return Array of market inventory entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	TArray<FMarketInventoryEntry> GetAvailableItems() const;

	/**
	 * Get filtered items by category
	 * @param Category Category to filter by
	 * @return Array of filtered inventory entries matching the specified category
	 * @note To get all items without filtering, use GetAvailableItems() instead
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	TArray<FMarketInventoryEntry> GetFilteredItems(ETradeItemCategory Category) const;

	/**
	 * Get price for an item at current market
	 * @param Item The item to price
	 * @param Quantity Number of units
	 * @return Total price for quantity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetItemPrice(UTradeItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Get player's current credits
	 * @return Player credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetPlayerCredits() const;

	/**
	 * Get player's cargo space info
	 * @param OutUsedSpace Used cargo space
	 * @param OutTotalSpace Total cargo capacity
	 * @param OutAvailableSpace Available cargo space
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	void GetCargoSpaceInfo(float& OutUsedSpace, float& OutTotalSpace, float& OutAvailableSpace) const;

	/**
	 * Get player's current profit
	 * @return Total profit (current credits - starting credits)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetPlayerProfit() const;

	// ========================================================================
	// Blueprint Functions - Shopping Cart
	// ========================================================================

	/**
	 * Add item to shopping cart
	 * @param Item The item to add
	 * @param Quantity Number of units to add
	 * @return True if successfully added to cart
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	bool AddToCart(UTradeItemDataAsset* Item, int32 Quantity);

	/**
	 * Remove item from shopping cart
	 * @param Item The item to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void RemoveFromCart(UTradeItemDataAsset* Item);

	/**
	 * Update quantity of item in cart
	 * @param Item The item to update
	 * @param NewQuantity New quantity (0 removes item)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void UpdateCartQuantity(UTradeItemDataAsset* Item, int32 NewQuantity);

	/**
	 * Clear all items from cart
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void ClearCart();

	/**
	 * Get total cost/value of items in cart
	 * @return Total cart value in credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetCartTotal() const;

	/**
	 * Get number of items in cart
	 * @return Item count in cart
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	int32 GetCartItemCount() const;

	/**
	 * Check if cart is empty
	 * @return True if cart has no items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	bool IsCartEmpty() const { return ShoppingCart.Num() == 0; }

	// ========================================================================
	// Blueprint Functions - Transaction Validation
	// ========================================================================

	/**
	 * Check if player can afford current cart
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	bool CanAffordCart() const;

	/**
	 * Check if player has enough cargo space for cart
	 * @return True if enough space available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	bool HasCargoSpaceForCart() const;

	/**
	 * Validate transaction can complete
	 * @param OutErrorMessage Error message if validation fails
	 * @return True if transaction is valid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trading")
	bool ValidateTransaction(FText& OutErrorMessage) const;

	// ========================================================================
	// Blueprint Functions - Execute Trade
	// ========================================================================

	/**
	 * Execute the current shopping cart transaction
	 * @return True if trade was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	bool ExecuteTrade();

	// ========================================================================
	// Blueprint Functions - UI Controls
	// ========================================================================

	/**
	 * Set category filter
	 * @param Category Category to filter by
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetCategoryFilter(ETradeItemCategory Category);

	/**
	 * Set sort mode
	 * @param Mode Sort mode string (Name, Price, Stock)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetSortMode(const FString& Mode);

	/**
	 * Toggle between buy and sell view
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void ToggleBuySellView();

	/**
	 * Set selected item for detail view
	 * @param Item The item to select
	 */
	UFUNCTION(BlueprintCallable, Category = "Trading")
	void SetSelectedItem(UTradeItemDataAsset* Item);

private:
	/** Initialize component references */
	void InitializeComponents();

	/** Refresh market inventory display */
	void RefreshMarketDisplay();

	/** Update UI with current player state */
	void UpdatePlayerState();

	/** Bind to component events */
	void BindComponentEvents();

	/** Unbind from component events */
	void UnbindComponentEvents();

	/** Timer handle for periodic updates */
	FTimerHandle UpdateTimer;

protected:
	/** Callback for player credits changed */
	UFUNCTION()
	void OnPlayerCreditsChanged(int32 NewCredits, int32 ChangeAmount);

	/** Callback for cargo changed */
	UFUNCTION()
	void OnPlayerCargoChanged(UTradeItemDataAsset* Item, int32 Quantity);
};
