// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "CyberneticsLab.generated.h"

/**
 * Cybernetics research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Cybernetics domain's breakthrough Data items (see CraftingTree):
 * CyberneticsResearch. Craftable as a station module.
 *
 * Upgraded from BiologyLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACyberneticsLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    ACyberneticsLab();
};
