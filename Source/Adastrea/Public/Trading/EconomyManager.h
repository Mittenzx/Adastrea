#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EconomyManager.generated.h"

// Forward declarations
class UMarketDataAsset;
class UTradeItemDataAsset;

/**
 * Economy Manager Subsystem
 * 
 * Manages the dynamic trading economy system for Adastrea.
 * Operates as a Game Instance Subsystem to persist across level transitions.
 * 
 * Features:
 * - Dynamic supply/demand simulation
 * - Market registration and management
 * - Price fluctuation based on transactions
 * - Economic recovery over time
 * - Background market activity simulation
 * - Game time tracking
 * 
 * Usage:
 * - Access via UGameInstance::GetSubsystem<UEconomyManager>()
 * - Register markets with RegisterMarket()
 * - Record transactions with RecordTransaction()
 * - Query prices with GetItemPrice()
 * 
 * Integration:
 * - PlayerTraderComponent uses this for all trading operations
 * - AITraderComponent uses this for NPC trading
 * - MarketDataAsset provides market configuration
 * - TradeItemDataAsset defines tradeable items
 * 
 * Economy Simulation:
 * - Player purchases decrease supply, increase demand
 * - Player sales increase supply, decrease demand
 * - Prices gradually return to baseline over time
 * - Background activity simulates NPC trading
 * - Time scale affects simulation speed
 */
UCLASS()
class ADASTREA_API UEconomyManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ====================
	// CONSTRUCTOR
	// ====================

	UEconomyManager();

	// ====================
	// SUBSYSTEM LIFECYCLE
	// ====================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// ECONOMY CONFIGURATION
	// ====================

	/**
	 * Rate at which supply/demand adjusts per transaction
	 * Default: 0.05 (5% change per transaction)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="0.01", ClampMax="0.5"))
	float SupplyDemandAdjustmentRate;

	/**
	 * Minimum supply/demand level
	 * Prevents prices from going too low
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="0.01", ClampMax="1.0"))
	float MinSupplyDemandLevel;

	/**
	 * Maximum supply/demand level
	 * Prevents prices from going too high
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MaxSupplyDemandLevel;

	/**
	 * Rate at which supply/demand returns to baseline
	 * Default: 0.1 (10% recovery per game hour)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="0.01", ClampMax="1.0"))
	float EconomicRecoveryRate;

	/**
	 * Current game time (in hours)
	 * Updated by economy simulation
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Economy|State")
	float CurrentGameTime;

	/**
	 * Time scale multiplier
	 * 1.0 = real-time, 60.0 = 1 real second = 1 game minute
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="0.1", ClampMax="1000.0"))
	float TimeScale;

	/**
	 * Update interval in real seconds
	 * How often the economy simulation runs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy|Config", meta=(ClampMin="1.0", ClampMax="60.0"))
	float UpdateInterval;

	// ====================
	// MARKET MANAGEMENT
	// ====================

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

	/**
	 * Check if a market is registered
	 * @param Market The market to check
	 * @return True if the market is registered
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Markets")
	bool IsMarketRegistered(UMarketDataAsset* Market) const;

	/**
	 * Get all registered markets
	 * @return Array of all active markets
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Markets")
	TArray<UMarketDataAsset*> GetActiveMarkets() const;

	/**
	 * Get number of registered markets
	 * @return Count of active markets
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Markets")
	int32 GetMarketCount() const;

	// ====================
	// TRADING OPERATIONS
	// ====================

	/**
	 * Get the current price of an item at a market
	 * @param Market The market to query
	 * @param Item The item to price
	 * @param bIsBuying True if player is buying, false if selling
	 * @return The price in credits
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Trading")
	float GetItemPrice(UMarketDataAsset* Market, UTradeItemDataAsset* Item, bool bIsBuying) const;

	/**
	 * Record a transaction and update market state
	 * Updates supply/demand based on transaction
	 * @param Market The market where transaction occurred
	 * @param Item The item that was traded
	 * @param Quantity Number of units traded
	 * @param bPlayerBought True if player bought, false if player sold
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Trading")
	void RecordTransaction(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, bool bPlayerBought);

	// ====================
	// TIME & SIMULATION
	// ====================

	/**
	 * Get current game time in hours
	 * @return Game time in hours
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Time")
	float GetGameTime() const;

	/**
	 * Set the time scale
	 * Clamps to safe range (0.1-10.0) to prevent destabilizing economy simulation
	 * @param NewTimeScale New time scale multiplier
	 */
	UFUNCTION(BlueprintCallable, Category="Economy|Time")
	void SetTimeScale(float NewTimeScale);

	/**
	 * Get the time scale
	 * @return Current time scale multiplier
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Economy|Time")
	float GetTimeScale() const;

protected:
	// ====================
	// INTERNAL STATE
	// ====================

	/** Active markets in the economy */
	UPROPERTY()
	TArray<UMarketDataAsset*> ActiveMarkets;

	/** Timer handle for economy updates */
	FTimerHandle UpdateTimerHandle;

	// ====================
	// INTERNAL SIMULATION
	// ====================

	/**
	 * Update economy simulation (called by timer)
	 * Updates all registered markets
	 */
	void UpdateEconomy();

	/**
	 * Update prices for a specific market
	 * Applies economic recovery over time
	 * @param Market The market to update
	 * @param DeltaHours Time elapsed in game hours
	 */
	void UpdateMarketPrices(UMarketDataAsset* Market, float DeltaHours);

	/**
	 * Simulate background trading activity
	 * Replenishes stock, simulates NPC trading
	 * @param Market The market to simulate
	 * @param DeltaHours Time elapsed in game hours
	 */
	void SimulateBackgroundActivity(UMarketDataAsset* Market, float DeltaHours);
};