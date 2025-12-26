// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "CargoBayModule.generated.h"

/**
 * Cargo storage bay module for space stations
 * 
 * General purpose storage facility for goods and materials.
 * Features automated inventory management and climate control.
 * 
 * Power Consumption: 5 units
 * Module Group: Storage
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACargoBayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ACargoBayModule();
};
