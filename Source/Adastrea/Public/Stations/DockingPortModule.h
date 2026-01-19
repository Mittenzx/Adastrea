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
	 * Typically 1-2 points for docking ports
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
	 * Default: 1 for compact docking ports
	 * 
	 * NOTE: Ensure DockingPoints array has at least this many elements defined
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking", meta=(ClampMin=1, ClampMax=4))
	int32 MaxDockedShips = 1;

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
