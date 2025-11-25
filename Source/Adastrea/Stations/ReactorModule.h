// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ReactorModule.generated.h"

/**
 * Power reactor module for space stations
 * 
 * Main power generation facility using fusion reactor technology.
 * Generates substantial power output to support station operations.
 * 
 * Power Generation: -500 units (negative = generates power)
 * Module Group: Power
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AReactorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AReactorModule();
};
