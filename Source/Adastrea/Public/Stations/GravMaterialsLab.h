// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "GravMaterialsLab.generated.h"

/**
 * Grav Materials research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Grav Materials domain's breakthrough Data items (see CraftingTree):
 * GravMaterialsResearch. Craftable as a station module.
 *
 * Upgraded from MaterialsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AGravMaterialsLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AGravMaterialsLab();
};
