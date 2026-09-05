// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ResearchLabModule.generated.h"

/**
 * Base class for domain-specialized research labs (Phase 5).
 *
 * Unlike the generic ScienceLabModule, each research lab is specialized to a
 * research DOMAIN (propulsion, materials, computing, weapons, biotech, ...) and
 * produces its own breakthrough Data items (see CraftingTree). A niche/upgraded
 * lab additionally names a prerequisite base lab it was built from.
 *
 * All research labs are craftable station modules (T6 base labs, T7 niche
 * labs), so the module-catalog / construction-gate plumbing extends directly.
 *
 * Module Group: Processing
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ADASTREA_API AResearchLabModule : public ASpaceStationModule
{
    GENERATED_BODY()

public:
    AResearchLabModule();

    /** Human-readable research domain (e.g. "Propulsion", "Kinetic Weapons"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ResearchLab")
    FString ResearchDomain;

    /** Crafting-tree item IDs this lab's breakthroughs produce (rl2/rl3 or niche). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ResearchLab")
    TArray<FString> BreakthroughItemIDs;

    /** Base lab this niche lab upgraded from (empty for base labs). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ResearchLab")
    TSubclassOf<AResearchLabModule> PrerequisiteLab;

    /** Maximum concurrent research projects. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ResearchLab", meta=(ClampMin="1"))
    int32 MaxResearchProjects = 3;

    /** Number of research projects currently active. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ResearchLab", meta=(ClampMin="0"))
    int32 ActiveProjects = 0;

    /** Research conducted per second while a project runs. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ResearchLab", meta=(ClampMin="0.1"))
    float ResearchRatePerSecond = 12.0f;

    /** Whether a new research project can be started. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="ResearchLab")
    bool CanStartResearch() const;

    /** Start a research project. Returns true if capacity was available. */
    UFUNCTION(BlueprintCallable, Category="ResearchLab")
    bool StartResearch();

    /** Complete one research project (advance). Returns true if one finished. */
    UFUNCTION(BlueprintCallable, Category="ResearchLab")
    bool CompleteResearchProject();

    /** True if this is a niche/upgraded lab (has a prerequisite base lab). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="ResearchLab")
    bool IsNicheLab() const;

    /** Get the first breakthrough item ID this lab produces, or empty string. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="ResearchLab")
    FString GetPrimaryBreakthrough() const;

protected:
    /** Initialize a concrete lab's domain + breakthroughs (called by subclass ctor). */
    void InitLab(const FString& InDomain, TArray<FString> InBreakthroughs,
        TSubclassOf<AResearchLabModule> InPrerequisite = nullptr);
};