// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "BiologyLabModule.generated.h"

/**
 * Biotech research lab module — a base domain-specialized research facility.
 *
 * Produces the Biotech domain's breakthrough Data items (see CraftingTree):
 * BioResearch, AdvancedBioResearch. Craftable as a station module.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ABiologyLabModule : public AResearchLabModule
{
    GENERATED_BODY()

public:
    ABiologyLabModule();
};
