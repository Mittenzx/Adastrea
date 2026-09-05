// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "ProjectileWeaponsLab.generated.h"

/**
 * Kinetic Weapons research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Kinetic Weapons domain's breakthrough Data items (see CraftingTree):
 * KineticWeaponResearch. Craftable as a station module.
 *
 * Upgraded from WeaponsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AProjectileWeaponsLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AProjectileWeaponsLab();
};
