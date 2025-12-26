// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "FuelDepotModule.generated.h"

/**
 * Fuel storage depot module for space stations
 * 
 * Specialized storage facility for ship fuel and propellants.
 * Includes safety systems, pumping equipment, and spill containment.
 * 
 * Power Consumption: 15 units
 * Module Group: Storage
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AFuelDepotModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AFuelDepotModule();
};
