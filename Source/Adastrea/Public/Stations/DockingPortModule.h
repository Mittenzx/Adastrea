// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "Components/SceneComponent.h"
#include "DockingPortModule.generated.h"

/**
 * Small docking port module for space stations
 * 
 * Provides a compact docking point for smaller vessels and shuttles.
 * Lower power requirements compared to full docking bays.
 * Typically supports a single docking point.
 * 
 * Features:
 * - Single or dual docking points for small ships
 * - Lower power consumption than docking bays
 * - Blueprint-accessible docking state tracking
 * 
 * Power Consumption: 10 units
 * Module Group: Docking
 * 
 * Usage:
 * 1. Add docking points using the DockingPoints array in editor
 * 2. Each docking point is a scene component marking a ship attachment location
 * 3. Use GetAvailableDockingPoint() to find free docking spots
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ADockingPortModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ADockingPortModule();

	/**
	 * Array of docking point components
	 * Each point represents a location where a ship can dock
	 * Add these in the Blueprint editor to define docking positions
	 * Typically 1-2 points for docking ports
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking")
	TArray<USceneComponent*> DockingPoints;

	/**
	 * Maximum number of ships that can dock simultaneously
	 * Default: 1 for compact docking ports
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking", meta=(ClampMin=1, ClampMax=4))
	int32 MaxDockedShips = 1;

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
	 * Check if the docking port has available capacity
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
