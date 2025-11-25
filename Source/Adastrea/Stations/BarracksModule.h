// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "BarracksModule.generated.h"

/**
 * Crew barracks module for space stations
 * 
 * Communal living facility for station crew and personnel.
 * Efficient dormitory-style housing with shared amenities.
 * 
 * Power Consumption: 20 units
 * Module Group: Habitation
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ABarracksModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ABarracksModule();
};
