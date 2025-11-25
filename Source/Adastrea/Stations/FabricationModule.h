// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "FabricationModule.generated.h"

/**
 * Manufacturing fabrication module for space stations
 * 
 * Advanced manufacturing facility for producing equipment and components.
 * Includes 3D printing, assembly lines, and quality control systems.
 * 
 * Power Consumption: 150 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AFabricationModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AFabricationModule();
};
