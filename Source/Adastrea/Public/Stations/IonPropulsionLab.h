// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "IonPropulsionLab.generated.h"

/**
 * Ion Propulsion research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Ion Propulsion domain's breakthrough Data items (see CraftingTree):
 * IonPropulsionResearch. Craftable as a station module.
 *
 * Upgraded from PhysicsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AIonPropulsionLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AIonPropulsionLab();
};
