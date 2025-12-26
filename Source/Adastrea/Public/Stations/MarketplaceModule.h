// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "MarketplaceModule.generated.h"

/**
 * Trade marketplace module for space stations
 * 
 * Commercial facility for buying, selling, and trading goods.
 * Includes vendor stalls, trading terminals, and auction systems.
 * 
 * Power Consumption: 40 units
 * Module Group: Public
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AMarketplaceModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AMarketplaceModule();
};
