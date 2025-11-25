// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "DockingPortModule.generated.h"

/**
 * Small docking port module for space stations
 * 
 * Provides a compact docking point for smaller vessels and shuttles.
 * Lower power requirements compared to full docking bays.
 * 
 * Power Consumption: 10 units
 * Module Group: Docking
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingPortModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ADockingPortModule();
};
