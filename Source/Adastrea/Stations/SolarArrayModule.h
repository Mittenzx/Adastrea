// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "SolarArrayModule.generated.h"

/**
 * Solar panel array module for space stations
 * 
 * Photovoltaic power generation using deployable solar panels.
 * Provides supplemental power with zero fuel consumption.
 * Efficiency depends on proximity to star and panel orientation.
 * 
 * Power Generation: -100 units (negative = generates power)
 * Module Group: Power
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ASolarArrayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ASolarArrayModule();
};
