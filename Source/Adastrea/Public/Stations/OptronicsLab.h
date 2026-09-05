// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "OptronicsLab.generated.h"

/**
 * Optronics research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Optronics domain's breakthrough Data items (see CraftingTree):
 * OptronicsResearch. Craftable as a station module.
 *
 * Upgraded from ElectronicsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AOptronicsLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AOptronicsLab();
};
