// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "EncryptionLab.generated.h"

/**
 * Encryption research lab module — an upgraded (niche) domain-specialized research facility.
 *
 * Produces the Encryption domain's breakthrough Data items (see CraftingTree):
 * EncryptionResearch. Craftable as a station module.
 *
 * Upgraded from ElectronicsLabModule.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AEncryptionLab : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AEncryptionLab();
};
