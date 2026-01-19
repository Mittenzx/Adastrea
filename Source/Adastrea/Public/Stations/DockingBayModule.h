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

	// BeginPlay override to auto-populate docking points from tagged components
	virtual void BeginPlay() override;

	/**
	 * Array of docking point components - AUTOMATICALLY POPULATED
	 * 
	 * This array is automatically filled at BeginPlay by finding all Scene Components
	 * tagged with "DockingPoint". You don't need to manually populate this array.
	 * 
	 * To add docking points:
	 * 1. Add Scene Component to Blueprint
	 * 2. Tag it with "DockingPoint" in the component's Tags array
	 * 3. Position the component where ships should dock
	 * 
	 * IMPORTANT: Ensure you have at least MaxDockedShips components tagged with "DockingPoint"
	 * 
	 * NOTE: This array is read-only because it's populated automatically.
	 * If you need to modify it at runtime, use PopulateDockingPointsFromTags() again.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Docking")
	TArray<USceneComponent*> DockingPoints;

	/**
	 * Maximum number of ships that can dock simultaneously
	 * Default: 4 for large docking bays
	 * 
	 * NOTE: Ensure DockingPoints array has at least this many elements defined
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking", meta=(ClampMin=1, ClampMax=20))
	int32 MaxDockedShips = 4;

	/**
	 * Current number of docked ships
	 * Updated when ships dock/undock using DockShip() and UndockShip()
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

	/**
	 * Populate the DockingPoints array from components tagged with "DockingPoint"
	 * 
	 * This function is called automatically at BeginPlay, but can be called again
	 * if you add docking points at runtime.
	 * 
	 * Searches for all USceneComponent children tagged with "DockingPoint" and
	 * adds them to the DockingPoints array in the order they are found.
	 */
	UFUNCTION(BlueprintCallable, Category="Docking")
	void PopulateDockingPointsFromTags();

	/**
	 * Dock a ship at this module
	 * Increments CurrentDockedShips if capacity available
	 * @return True if ship was docked, false if no capacity
	 */
	UFUNCTION(BlueprintCallable, Category="Docking")
	bool DockShip();

	/**
	 * Undock a ship from this module
	 * Decrements CurrentDockedShips if any ships are docked
	 * @return True if ship was undocked, false if no ships docked
	 */
	UFUNCTION(BlueprintCallable, Category="Docking")
	bool UndockShip();
};
