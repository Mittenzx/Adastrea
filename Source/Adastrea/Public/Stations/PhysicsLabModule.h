// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "PhysicsLabModule.generated.h"

/**
 * Propulsion research lab module — a base domain-specialized research facility.
 *
 * Produces the Propulsion domain's breakthrough Data items (see CraftingTree):
 * PropulsionResearch, AdvancedPropulsionResearch. Craftable as a station module.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API APhysicsLabModule : public AResearchLabModule
{
    GENERATED_BODY()

public:
    APhysicsLabModule();
};
