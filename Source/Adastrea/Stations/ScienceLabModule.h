// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ScienceLabModule.generated.h"

/**
 * Research laboratory module for space stations
 * 
 * Scientific research facility for experimentation and development.
 * Supports various research disciplines including physics, biology, and engineering.
 * 
 * Power Consumption: 75 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AScienceLabModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AScienceLabModule();
};
