// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "Trading/CraftingTreeLoader.h"
#include "FabricationModule.generated.h"

class UCargoComponent;

/**
 * Fabrication module for space stations
 *
 * Advanced manufacturing bay that builds complex items (components, parts,
 * station modules, research-lab hulls). Holds a FIFO queue of jobs; each job
 * carries the remaining work units. The station feeds BuildRatePerSecond worth
 * of work each tick via AdvanceJob(), and a job pops when its work hits zero.
 *
 * Power Consumption: 150 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AFabricationModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AFabricationModule();

	/** Maximum number of queued jobs this bay can hold. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fabrication", meta=(ClampMin="1"))
	int32 MaxJobCount = 4;

	/** Units of work applied to the front job per second when running. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fabrication", meta=(ClampMin="1.0"))
	float BuildRatePerSecond = 50.0f;

	/** Number of jobs currently queued (not yet built). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fabrication")
	int32 GetQueuedJobCount() const;

	/** Whether a job slot is free. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fabrication")
	bool CanAcceptJob() const;

	/** Queue a job needing WorkUnits of fabrication. Returns true if accepted. */
	UFUNCTION(BlueprintCallable, Category="Fabrication")
	bool AddJob(float WorkUnits);

	/**
	 * Apply WorkDone units to the front job. Returns the number of jobs that
	 * completed this tick (0, 1, or more if work overran into subsequent jobs).
	 */
	UFUNCTION(BlueprintCallable, Category="Fabrication")
	int32 AdvanceJob(float WorkDone);

	/** Progress fraction (0..1) of the front-most job; 0 if empty. */
		UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fabrication")
		float GetFrontJobProgress() const;

		// ====================
		// CRAFTING (Phase 6) — this module runs Fabrication recipes from CraftingTree
		// ====================

		/** All recipes this facility can produce (ProducedIn == "Fabrication"). */
		UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fabrication|Crafting")
		TArray<FCraftingRecipe> GetCraftableRecipes() const;

		/** Whether enough ingredients are held in Cargo to craft OutputItem here. */
		UFUNCTION(BlueprintCallable, BlueprintPure, Category="Fabrication|Crafting")
		bool CanCraft(FName OutputItemID, UCargoComponent* Cargo) const;

		/**
		 * Craft OutputItem in this facility, consuming ingredients from Cargo and
		 * adding the output. Returns true on success.
		 */
		UFUNCTION(BlueprintCallable, Category="Fabrication|Crafting")
		bool CraftItem(FName OutputItemID, UCargoComponent* Cargo);

		/** Get the crafting-tree loader (lazily created + loaded). */
		UCraftingTreeLoader* GetCraftingLoader() const;

	private:
		/** Remaining work units per queued job (FIFO; front = index 0). */
		UPROPERTY(VisibleAnywhere, Category="Fabrication")
		TArray<float> JobQueue;
};