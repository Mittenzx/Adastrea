// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "BeamWeaponsLab.generated.h"

/**
 * Beam Weapons research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Beam Weapons domain's breakthrough Data items (see CraftingTree):
 * BeamWeaponResearch. Craftable as a station module.
 *
 * Upgraded from WeaponsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ABeamWeaponsLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    ABeamWeaponsLab();
};
