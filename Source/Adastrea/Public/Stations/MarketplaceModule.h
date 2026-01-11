// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "Trading/MarketDataAsset.h"
#include "MarketplaceModule.generated.h"

/**
 * Trade marketplace module for space stations
 * 
 * Commercial facility for buying, selling, and trading goods.
 * Includes vendor stalls, trading terminals, and auction systems.
 * 
 * Features:
 * - Configurable market data asset for inventory and pricing
 * - Can be opened/closed for trading
 * - Supports multiple marketplace types per station
 * 
 * Power Consumption: 40 units
 * Module Group: Public
 * 
 * Usage:
 * 1. Create a Blueprint based on this class (e.g., BP_SpaceStationModule_Marketplace)
 * 2. In Class Defaults, set the Market Data Asset property
 * 3. Optionally configure the marketplace name and open status
 * 4. Add to station's DefaultModuleClasses array
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AMarketplaceModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AMarketplaceModule();

	/**
	 * Market configuration for this marketplace
	 * Defines inventory, prices, and trading rules
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace",
		meta=(ToolTip="Market data asset defining what this marketplace sells and buys"))
	UMarketDataAsset* MarketDataAsset;

	/**
	 * Whether this marketplace is currently open for trading
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
	bool bIsOpen;

	/**
	 * Display name for this marketplace (e.g., "Central Market", "Black Market")
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
	FText MarketplaceName;

	/**
	 * Get the market data asset for this marketplace
	 * @return The market data asset, or nullptr if not set
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Marketplace")
	UMarketDataAsset* GetMarketData() const { return MarketDataAsset; }

	/**
	 * Check if this marketplace is open and has a valid market configuration
	 * @return True if marketplace can be used for trading
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Marketplace")
	bool IsAvailableForTrading() const { return bIsOpen && MarketDataAsset != nullptr; }
};
