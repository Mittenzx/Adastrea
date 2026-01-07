// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "Components/SceneComponent.h"
#include "DockingBayModule.generated.h"

/**
 * Large docking bay module for space stations
 * 
 * Provides docking facilities for larger ships and freighters.
 * Consumes significant power to operate hangar doors, tractor beams,
 * and docking assistance systems.
 * 
 * Features:
 * - Multiple docking points for simultaneous ship docking
 * - Configurable docking capacity
 * - Blueprint-accessible docking state tracking
 * 
 * Power Consumption: 50 units
 * Module Group: Docking
 * 
 * Usage:
 * 1. Add docking points using the DockingPoints array in editor
 * 2. Each docking point is a scene component marking a ship attachment location
 * 3. Use GetAvailableDockingPoint() to find free docking spots
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingBayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ADockingBayModule();

	/**
	 * Array of docking point components
	 * Each point represents a location where a ship can dock
	 * Add these in the Blueprint editor to define docking positions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking")
	TArray<USceneComponent*> DockingPoints;

	/**
	 * Maximum number of ships that can dock simultaneously
	 * Default: 4 for large docking bays
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking", meta=(ClampMin=1, ClampMax=20))
	int32 MaxDockedShips = 4;

	/**
	 * Current number of docked ships
	 * Updated when ships dock/undock
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
	int32 CurrentDockedShips = 0;

	/**
	 * Get the number of available docking spots
	 * @return Number of free docking points
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Docking")
	int32 GetAvailableDockingSpots() const { return MaxDockedShips - CurrentDockedShips; }

	/**
	 * Check if the docking bay has available capacity
	 * @return True if ships can dock, false if full
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Docking")
	bool HasAvailableDocking() const { return CurrentDockedShips < MaxDockedShips; }

	/**
	 * Get the first available docking point
	 * @return Scene component representing the docking point, or nullptr if none available
	 */
	UFUNCTION(BlueprintCallable, Category="Docking")
	USceneComponent* GetAvailableDockingPoint() const;

	/**
	 * Get all docking point components
	 * @return Array of all docking points
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Docking")
	TArray<USceneComponent*> GetDockingPoints() const { return DockingPoints; }
};
