// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StationGridSystem.generated.h"

/**
 * Station Grid System - Component for grid-based module placement
 * 
 * This component provides grid snapping functionality for module placement
 * in the station editor. It ensures modules are placed on a consistent grid
 * for alignment and connection purposes.
 * 
 * Features:
 * - Configurable grid size
 * - 3D grid snapping
 * - Nearest grid point calculation
 * - Valid position validation
 * - Connection point support (future expansion)
 * 
 * Usage:
 * 1. Add this component to the station editor manager or editor actor
 * 2. Configure GridSize to match module dimensions
 * 3. Use SnapToGrid() to align positions to the grid
 * 
 * @see UStationEditorManager
 */
UCLASS(BlueprintType, ClassGroup=(StationEditor), meta=(BlueprintSpawnableComponent))
class STATIONEDITOR_API UStationGridSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UStationGridSystem();

	// =====================
	// Configuration
	// =====================

	/** Size of each grid cell in world units (default 100 = 1 meter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Configuration", meta=(ClampMin=1.0f, UIMin=1.0f))
	float GridSize = 100.0f;

	/** Whether the grid is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Configuration")
	bool bGridEnabled = true;

	/** Origin offset for the grid (allows grid alignment adjustment) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Configuration")
	FVector GridOrigin = FVector::ZeroVector;

	/** Whether to snap rotation to fixed angles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Configuration")
	bool bSnapRotation = true;

	/** Rotation snap increment in degrees (e.g., 90 for 90-degree increments) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Configuration", meta=(ClampMin=1.0f, ClampMax=180.0f, UIMin=1.0f, UIMax=180.0f))
	float RotationSnapDegrees = 90.0f;

	// =====================
	// Grid Operations
	// =====================

	/**
	 * Snap a world position to the nearest grid point
	 * @param WorldPosition The position to snap
	 * @return The snapped position aligned to the grid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	FVector SnapToGrid(FVector WorldPosition) const;

	/**
	 * Get the nearest grid point to a world position
	 * @param WorldPosition The position to query
	 * @return The nearest grid point
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	FVector GetNearestGridPoint(FVector WorldPosition) const;

	/**
	 * Check if a position is exactly on a grid point
	 * @param WorldPosition The position to check
	 * @param Tolerance Tolerance for floating point comparison
	 * @return True if position is on a grid point
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	bool IsOnGrid(FVector WorldPosition, float Tolerance = 0.01f) const;

	/**
	 * Check if a position is valid for module placement
	 * @param WorldPosition The position to validate
	 * @return True if position is valid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	bool IsValidGridPosition(FVector WorldPosition) const;

	/**
	 * Snap a rotation to the configured rotation increment
	 * @param Rotation The rotation to snap
	 * @return The snapped rotation
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	FRotator SnapRotation(FRotator Rotation) const;

	/**
	 * Get the grid coordinate for a world position
	 * @param WorldPosition The world position
	 * @return Integer grid coordinates (X, Y, Z grid indices)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	FIntVector GetGridCoordinate(FVector WorldPosition) const;

	/**
	 * Convert grid coordinates back to world position
	 * @param GridCoordinate The grid coordinates
	 * @return World position at the center of that grid cell
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Operations")
	FVector GridCoordinateToWorld(FIntVector GridCoordinate) const;

	// =====================
	// Configuration
	// =====================

	/**
	 * Set the grid size
	 * @param NewGridSize New grid cell size in world units
	 */
	UFUNCTION(BlueprintCallable, Category="Grid|Configuration")
	void SetGridSize(float NewGridSize);

	/**
	 * Set the grid origin
	 * @param NewOrigin New grid origin position
	 */
	UFUNCTION(BlueprintCallable, Category="Grid|Configuration")
	void SetGridOrigin(FVector NewOrigin);

	/**
	 * Enable or disable the grid
	 * @param bEnabled Whether grid snapping is enabled
	 */
	UFUNCTION(BlueprintCallable, Category="Grid|Configuration")
	void SetGridEnabled(bool bEnabled);

	// =====================
	// Connection Points (Future Expansion)
	// =====================

	/**
	 * Get all valid connection points near a position
	 * @param WorldPosition Center position to search from
	 * @param SearchRadius Radius to search within
	 * @return Array of valid connection positions
	 */
	UFUNCTION(BlueprintCallable, Category="Grid|Connections")
	TArray<FVector> GetNearbyConnectionPoints(FVector WorldPosition, float SearchRadius) const;

	/**
	 * Check if two grid positions can be connected
	 * @param PositionA First position
	 * @param PositionB Second position
	 * @return True if positions are adjacent on the grid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Grid|Connections")
	bool ArePositionsAdjacent(FVector PositionA, FVector PositionB) const;

protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Helper to snap a single float value to grid
	 */
	float SnapValueToGrid(float Value, float Offset) const;

	/**
	 * Helper to snap an angle to the rotation increment
	 */
	float SnapAngle(float Angle) const;
};
