// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "HabitationModule.generated.h"

/**
 * Living quarters habitation module for space stations
 * 
 * Residential facility providing living spaces for station inhabitants.
 * Includes life support, private quarters, and communal areas.
 * 
 * Power Consumption: 30 units
 * Module Group: Habitation
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AHabitationModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AHabitationModule();
};
