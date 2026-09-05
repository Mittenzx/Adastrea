// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "ElectronicsLabModule.generated.h"

/**
 * Computing research lab module — a base domain-specialized research facility.
 *
 * Produces the Computing domain's breakthrough Data items (see CraftingTree):
 * ComputingResearch, QuantumComputingResearch. Craftable as a station module.
 *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AElectronicsLabModule : public AResearchLabModule
{
    GENERATED_BODY()

public:
    AElectronicsLabModule();
};
