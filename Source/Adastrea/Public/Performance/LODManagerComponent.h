// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LODManagerComponent.generated.h"

/**
 * LOD (Level of Detail) levels for visual quality
 */
UENUM(BlueprintType)
enum class ELODLevel : uint8
{
	High UMETA(DisplayName = "High Quality"),
	Medium UMETA(DisplayName = "Medium Quality"),
	Low UMETA(DisplayName = "Low Quality"),
	VeryLow UMETA(DisplayName = "Very Low Quality")
};

/**
 * LOD Manager Component for dynamic Level of Detail management
 * 
 * This component automatically adjusts visual quality based on distance
 * from the camera and performance requirements.
 * 
 * Usage:
 * 1. Add this component to actors that need LOD management
 * 2. Configure LOD distance thresholds
 * 3. Component automatically updates LOD level based on camera distance
 * 4. Blueprint events are fired when LOD changes
 * 
 * Example:
 * - Add to space station actor
 * - Set HighToMediumDistance to 5000 units
 * - Receive OnLODChanged event to update visuals
 */
UCLASS(ClassGroup = (Performance), meta = (BlueprintSpawnableComponent))
class ADASTREA_API ULODManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULODManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================================================
	// Configuration
	// ========================================================================

	/** Distance threshold for switching from High to Medium LOD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings",
		meta = (ClampMin = 100.0, ClampMax = 100000.0))
	float HighToMediumDistance;

	/** Distance threshold for switching from Medium to Low LOD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings",
		meta = (ClampMin = 100.0, ClampMax = 100000.0))
	float MediumToLowDistance;

	/** Distance threshold for switching from Low to VeryLow LOD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings",
		meta = (ClampMin = 100.0, ClampMax = 100000.0))
	float LowToVeryLowDistance;

	/** How often to update LOD (seconds) - higher values improve performance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings",
		meta = (ClampMin = 0.1, ClampMax = 5.0, UIMin = 0.1, UIMax = 5.0))
	float UpdateFrequency;

	/** Whether to use performance-based LOD adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings")
	bool bUsePerformanceLOD;

	/** Target frame rate for performance LOD (if enabled) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings",
		meta = (EditCondition = "bUsePerformanceLOD", ClampMin = 30.0, ClampMax = 120.0))
	float TargetFrameRate;

	// ========================================================================
	// State
	// ========================================================================

	/** Current LOD level */
	UPROPERTY(BlueprintReadOnly, Category = "LOD State")
	ELODLevel CurrentLODLevel;

	/** Distance to camera */
	UPROPERTY(BlueprintReadOnly, Category = "LOD State")
	float DistanceToCamera;

	// ========================================================================
	// Blueprint Events
	// ========================================================================

	/**
	 * Event fired when LOD level changes
	 * @param NewLODLevel The new LOD level
	 * @param OldLODLevel The previous LOD level
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LOD Events")
	void OnLODChanged(ELODLevel NewLODLevel, ELODLevel OldLODLevel);

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Manually set the LOD level (overrides automatic updates until next tick)
	 * @param NewLevel The LOD level to set
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetLODLevel(ELODLevel NewLevel);

	/**
	 * Force an immediate LOD update
	 */
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void ForceUpdateLOD();

	/**
	 * Get the recommended LOD level for a given distance
	 * @param Distance Distance to camera
	 * @return Recommended LOD level
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "LOD")
	ELODLevel GetRecommendedLODForDistance(float Distance) const;

	/**
	 * Check if actor should be culled entirely (beyond VeryLow LOD range)
	 * @return True if actor should be culled
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "LOD")
	bool ShouldCullActor() const;

private:
	/** Time since last LOD update */
	float TimeSinceLastUpdate;

	/** Update LOD based on distance and performance */
	void UpdateLOD();

	/** Get distance to player camera */
	float GetDistanceToCamera() const;

	/** Adjust LOD based on current frame rate */
	void AdjustLODForPerformance();
};
