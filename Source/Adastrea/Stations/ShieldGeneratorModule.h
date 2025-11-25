// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ShieldGeneratorModule.generated.h"

/**
 * Shield generator module for space stations
 * 
 * Defensive shield projection system for station protection.
 * Generates energy barriers to absorb incoming damage from weapons and debris.
 * High power consumption due to continuous field generation.
 * 
 * Power Consumption: 200 units
 * Module Group: Defence
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AShieldGeneratorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AShieldGeneratorModule();
};
