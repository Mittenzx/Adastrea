// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "CorridorModule.generated.h"

/**
 * Connecting corridor module for space stations
 * 
 * Passageway connecting different station sections and modules.
 * Includes basic lighting and environmental systems.
 * 
 * Power Consumption: 2 units
 * Module Group: Connection
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACorridorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ACorridorModule();
};
