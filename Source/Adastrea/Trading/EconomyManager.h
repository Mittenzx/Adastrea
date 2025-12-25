#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EconomyManager.generated.h"

// Forward declarations
class UMarketDataAsset;
class UTradeItemDataAsset;

/**
 * Economy Manager Subsystem
 * Manages market prices, supply/demand, and economic simulation
 * 
 * Usage:
 * 1. Get subsystem from GameInstance
 * 2. Register markets via RegisterMarket
 * 3. Query prices via GetItemPrice
 * 4. Record transactions via RecordTransaction
 * 
 * The economy updates automatically on a timer, simulating
 * price changes, stock replenishment, and market dynamics.
 */
UCLASS()
class ADASTREA_API UEconomyManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ====================
	// SUBSYSTEM LIFECYCLE
	// ====================
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// MARKET MANAGEMENT
	// ====================

	// All markets registered in the economy
	UPROPERTY(BlueprintReadWrite, Category="Economy")
	TArray<UMarketDataAsset*> ActiveMarkets;

	/**
	 * Register a market for economy simulation
	 * @param Market The market to register
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Markets")
	void RegisterMarket(UMarketDataAsset* Market);

	/**
	 * Unregister a market from economy simulation
	 * @param Market The market to unregister
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Markets")
	void UnregisterMarket(UMarketDataAsset* Market);

	// ====================
	// TIME MANAGEMENT
	// ====================

	// Current game time in hours
	UPROPERTY(BlueprintReadWrite, Category="Economy|Time")
	float CurrentGameTime;

	// Time scale multiplier (1.0 = normal speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Economy|Time", meta=(ClampMin="0.1", ClampMax="10.0"))
	float TimeScale;

	// Update interval in real seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Economy|Time", meta=(ClampMin="1.0", ClampMax="60.0"))
	float UpdateInterval;

	// ====================
	// ECONOMY SIMULATION
	// ====================

	/**
	 * Update economy state
	 * Called periodically via timer
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Simulation")
	void UpdateEconomy();

	/**
	 * Get current price for item at market
	 * @param Market The market to check
	 * @param Item The item to price
	 * @param bIsBuying True if player is buying, false if selling
	 * @return Current price per unit
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Pricing")
	float GetItemPrice(UMarketDataAsset* Market, UTradeItemDataAsset* Item, bool bIsBuying) const;

	/**
	 * Record a player transaction
	 * Updates supply/demand accordingly
	 * @param Market The market where transaction occurred
	 * @param Item The item traded
	 * @param Quantity Number of units traded
	 * @param bPlayerBought True if player bought, false if sold
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Transactions")
	void RecordTransaction(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, bool bPlayerBought);

	/**
	 * Get all markets in the economy
	 * @return Array of active markets
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Markets")
	TArray<UMarketDataAsset*> GetActiveMarkets() const { return ActiveMarkets; }

	/**
	 * Check if a market is registered
	 * @param Market The market to check
	 * @return True if market is registered
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Markets")
	bool IsMarketRegistered(UMarketDataAsset* Market) const;

	// ====================
	// CONSTRUCTOR
	// ====================

	UEconomyManager();

private:
	// Timer handle for periodic updates
	FTimerHandle UpdateTimerHandle;

	// Update market prices based on supply/demand
	void UpdateMarketPrices(UMarketDataAsset* Market, float DeltaHours);

	// Simulate background economic activity
	void SimulateBackgroundActivity(UMarketDataAsset* Market, float DeltaHours);
};
