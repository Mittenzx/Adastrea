// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "DockingBayModule.generated.h"

/**
 * Large docking bay module for space stations
 * 
 * Provides docking facilities for larger ships and freighters.
 * Consumes significant power to operate hangar doors, tractor beams,
 * and docking assistance systems.
 * 
 * Power Consumption: 50 units
 * Module Group: Docking
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingBayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ADockingBayModule();
};
