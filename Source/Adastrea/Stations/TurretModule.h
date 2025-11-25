// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "TurretModule.generated.h"

/**
 * Point defense turret module for space stations
 * 
 * Defensive weapon emplacement for station protection.
 * Provides automated targeting and fire control against hostile vessels.
 * 
 * Power Consumption: 25 units
 * Module Group: Defence
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ATurretModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ATurretModule();
};
