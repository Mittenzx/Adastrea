// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ProcessingModule.generated.h"

/**
 * Material processing module for space stations
 * 
 * Industrial facility for processing raw materials into refined goods.
 * Handles ore refining, chemical processing, and material conversion.
 * 
 * Power Consumption: 100 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AProcessingModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AProcessingModule();
};
