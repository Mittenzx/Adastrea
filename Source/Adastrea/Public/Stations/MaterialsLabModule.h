// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "MaterialsLabModule.generated.h"

/**
 * Materials research lab module — a base domain-specialized research facility.
 *
 * Produces the Materials domain's breakthrough Data items (see CraftingTree):
 * MaterialsResearch, NanoMaterialsResearch. Craftable as a station module.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AMaterialsLabModule : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AMaterialsLabModule();
};
