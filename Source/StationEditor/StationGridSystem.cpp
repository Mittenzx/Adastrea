// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationGridSystem.h"
#include "AdastreaLog.h"

UStationGridSystem::UStationGridSystem()
{
	PrimaryComponentTick.bCanEverTick = false;

	GridSize = 100.0f;
	bGridEnabled = true;
	GridOrigin = FVector::ZeroVector;
	bSnapRotation = true;
	RotationSnapDegrees = 90.0f;
}

void UStationGridSystem::BeginPlay()
{
	Super::BeginPlay();
}

// =====================
// Grid Operations
// =====================

FVector UStationGridSystem::SnapToGrid(FVector WorldPosition) const
{
	if (!bGridEnabled)
	{
		return WorldPosition;
	}

	return FVector(
		SnapValueToGrid(WorldPosition.X, GridOrigin.X),
		SnapValueToGrid(WorldPosition.Y, GridOrigin.Y),
		SnapValueToGrid(WorldPosition.Z, GridOrigin.Z)
	);
}

FVector UStationGridSystem::GetNearestGridPoint(FVector WorldPosition) const
{
	// GetNearestGridPoint always snaps, regardless of bGridEnabled
	return FVector(
		SnapValueToGrid(WorldPosition.X, GridOrigin.X),
		SnapValueToGrid(WorldPosition.Y, GridOrigin.Y),
		SnapValueToGrid(WorldPosition.Z, GridOrigin.Z)
	);
}

bool UStationGridSystem::IsOnGrid(FVector WorldPosition, float Tolerance) const
{
	FVector SnappedPosition = GetNearestGridPoint(WorldPosition);
	return FVector::DistSquared(WorldPosition, SnappedPosition) <= (Tolerance * Tolerance);
}

bool UStationGridSystem::IsValidGridPosition(FVector WorldPosition) const
{
	// For now, all grid positions are valid
	// This can be extended to check against:
	// - Maximum build distance
	// - Forbidden zones
	// - Required connection to existing modules
	return true;
}

FRotator UStationGridSystem::SnapRotation(FRotator Rotation) const
{
	if (!bSnapRotation)
	{
		return Rotation;
	}

	return FRotator(
		SnapAngle(Rotation.Pitch),
		SnapAngle(Rotation.Yaw),
		SnapAngle(Rotation.Roll)
	);
}

FIntVector UStationGridSystem::GetGridCoordinate(FVector WorldPosition) const
{
	// Offset by grid origin
	FVector RelativePosition = WorldPosition - GridOrigin;

	// Calculate grid indices
	return FIntVector(
		FMath::RoundToInt(RelativePosition.X / GridSize),
		FMath::RoundToInt(RelativePosition.Y / GridSize),
		FMath::RoundToInt(RelativePosition.Z / GridSize)
	);
}

FVector UStationGridSystem::GridCoordinateToWorld(FIntVector GridCoordinate) const
{
	return FVector(
		GridCoordinate.X * GridSize + GridOrigin.X,
		GridCoordinate.Y * GridSize + GridOrigin.Y,
		GridCoordinate.Z * GridSize + GridOrigin.Z
	);
}

// =====================
// Configuration
// =====================

void UStationGridSystem::SetGridSize(float NewGridSize)
{
	if (NewGridSize >= 1.0f)
	{
		GridSize = NewGridSize;
		UE_LOG(LogAdastreaStations, Log, TEXT("StationGridSystem::SetGridSize - Set grid size to %.2f"), GridSize);
	}
	else
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationGridSystem::SetGridSize - Invalid grid size %.2f (must be >= 1.0)"), NewGridSize);
	}
}

void UStationGridSystem::SetGridOrigin(FVector NewOrigin)
{
	GridOrigin = NewOrigin;
	UE_LOG(LogAdastreaStations, Log, TEXT("StationGridSystem::SetGridOrigin - Set grid origin to (%.2f, %.2f, %.2f)"),
		GridOrigin.X, GridOrigin.Y, GridOrigin.Z);
}

void UStationGridSystem::SetGridEnabled(bool bEnabled)
{
	bGridEnabled = bEnabled;
	UE_LOG(LogAdastreaStations, Log, TEXT("StationGridSystem::SetGridEnabled - Grid %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

// =====================
// Connection Points (Future Expansion)
// =====================

TArray<FVector> UStationGridSystem::GetNearbyConnectionPoints(FVector WorldPosition, float SearchRadius) const
{
	TArray<FVector> ConnectionPoints;

	// Calculate grid bounds to search
	int32 GridRadius = FMath::CeilToInt(SearchRadius / GridSize);
	FIntVector CenterGrid = GetGridCoordinate(WorldPosition);

	// Search in a cube around the center position
	for (int32 X = -GridRadius; X <= GridRadius; ++X)
	{
		for (int32 Y = -GridRadius; Y <= GridRadius; ++Y)
		{
			for (int32 Z = -GridRadius; Z <= GridRadius; ++Z)
			{
				FIntVector GridPos(CenterGrid.X + X, CenterGrid.Y + Y, CenterGrid.Z + Z);
				FVector WorldPoint = GridCoordinateToWorld(GridPos);

				// Check if within actual radius
				if (FVector::Dist(WorldPosition, WorldPoint) <= SearchRadius)
				{
					ConnectionPoints.Add(WorldPoint);
				}
			}
		}
	}

	return ConnectionPoints;
}

bool UStationGridSystem::ArePositionsAdjacent(FVector PositionA, FVector PositionB) const
{
	FIntVector GridA = GetGridCoordinate(PositionA);
	FIntVector GridB = GetGridCoordinate(PositionB);

	// Calculate Manhattan distance in grid space
	int32 DiffX = FMath::Abs(GridA.X - GridB.X);
	int32 DiffY = FMath::Abs(GridA.Y - GridB.Y);
	int32 DiffZ = FMath::Abs(GridA.Z - GridB.Z);

	// Adjacent means exactly one cell apart in one direction
	int32 TotalDiff = DiffX + DiffY + DiffZ;
	return TotalDiff == 1;
}

// =====================
// Private Helpers
// =====================

float UStationGridSystem::SnapValueToGrid(float Value, float Offset) const
{
	// Offset value, snap to grid, then offset back
	float OffsetValue = Value - Offset;
	float SnappedValue = FMath::RoundToFloat(OffsetValue / GridSize) * GridSize;
	return SnappedValue + Offset;
}

float UStationGridSystem::SnapAngle(float Angle) const
{
	// Normalize angle to 0-360 range
	float NormalizedAngle = FMath::Fmod(Angle, 360.0f);
	if (NormalizedAngle < 0.0f)
	{
		NormalizedAngle += 360.0f;
	}

	// Snap to nearest increment
	float SnappedAngle = FMath::RoundToFloat(NormalizedAngle / RotationSnapDegrees) * RotationSnapDegrees;

	// Keep in 0-360 range
	SnappedAngle = FMath::Fmod(SnappedAngle, 360.0f);

	return SnappedAngle;
}
