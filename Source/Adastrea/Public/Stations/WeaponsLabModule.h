// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "WeaponsLabModule.generated.h"

/**
 * Weapons research lab module — a base domain-specialized research facility.
 *
 * Produces the Weapons domain's breakthrough Data items (see CraftingTree):
 * DefenceResearch, AdvancedDefenceResearch. Craftable as a station module.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AWeaponsLabModule : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AWeaponsLabModule();
};
