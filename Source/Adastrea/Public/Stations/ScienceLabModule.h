// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ScienceLabModule.generated.h"

/**
 * Science lab module for space stations
 *
 * Performs research. Holds research slots and a research throughput; the
 * research system feeds breakthrough Data items (see CraftingTree) into the lab
 * to be produced. Tracks active vs available projects.
 *
 * Power Consumption: 75 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AScienceLabModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AScienceLabModule();

	/** Maximum concurrent research projects. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScienceLab", meta=(ClampMin="1"))
	int32 MaxResearchProjects = 3;

	/** Number of research projects currently active. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ScienceLab", meta=(ClampMin="0"))
	int32 ActiveProjects = 0;

	/** Research conducted per second while a project runs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ScienceLab", meta=(ClampMin="0.1"))
	float ResearchRatePerSecond = 10.0f;

	/** Whether a new research project can be started. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ScienceLab")
	bool CanStartResearch() const;

	/** Start a research project. Returns true if capacity was available. */
	UFUNCTION(BlueprintCallable, Category="ScienceLab")
	bool StartResearch();

	/** Complete one research project (advance). Returns true if one finished. */
	UFUNCTION(BlueprintCallable, Category="ScienceLab")
	bool CompleteResearchProject();
};