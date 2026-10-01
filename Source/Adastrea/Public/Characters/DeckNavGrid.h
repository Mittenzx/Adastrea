// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

class UWorld;
class AActor;

/**
 * Walkable grid for one interior (ship deck, station), baked at runtime from the
 * interior's own collision — no navmesh, no authored waypoints, so every deck the
 * pipeline produces is walkable by NPCs as soon as it is mounted.
 *
 * Built in the interior's local frame (decks can be yawed with their ship): each
 * XY column is traced top-down on the Pawn channel (what walls and floors block),
 * recording every walkable floor in it, so stacked decks and mezzanines become
 * separate layers. A floor point is kept only if a person-sized capsule fits there.
 * Neighbouring points within a step height of each other are connected, so stairs
 * and ramps link the layers.
 *
 * Paths are A* over that graph, then string-pulled so NPCs walk straight lines
 * across open rooms instead of zig-zagging cell to cell.
 */
class ADASTREA_API FDeckNavGrid
{
public:
	struct FSettings
	{
		/** Grid spacing (cm). Under twice the agent radius, so a thin wall always blocks a cell. */
		float CellSize = 40.0f;
		/** Capsule that must fit for a point to be walkable (cm): a character's
		 * capsule (r34) plus a little margin — any thinner and paths squeeze
		 * through gaps the body can't. */
		float AgentRadius = 36.0f;
		float AgentHalfHeight = 86.0f;
		/** Highest step between neighbouring points (cm). */
		float MaxStep = 45.0f;
		/** Floors steeper than this aren't walkable (cos of the max slope). */
		float MinFloorNormalZ = 0.7f;
		/** Safety cap: decks bigger than this many columns are sampled coarser. */
		int32 MaxColumns = 60000;
	};

	/**
	 * Bake the grid for Interior inside LocalBounds (interior-local space).
	 * Actors in Ignore (people) are left out of every trace.
	 */
	void Build(UWorld* World, const AActor* Interior, const FBox& LocalBounds, const TArray<const AActor*>& Ignore,
		const FSettings& InSettings = FSettings());

	bool IsValid() const { return Nodes.Num() > 0; }
	int32 NumNodes() const { return Nodes.Num(); }
	double GetBuildSeconds() const { return BuildSeconds; }

	/** Walkable node nearest a world point (searching a few cells around it), or INDEX_NONE. */
	int32 FindNearestNode(const FVector& WorldLocation, float MaxDistance = 150.0f) const;

	/** World-space floor point of a node. */
	FVector GetNodeLocation(int32 Node) const;

	/** A random node reachable from FromNode (same connected area), at least MinDistance away if possible. */
	int32 GetRandomReachableNode(int32 FromNode, FRandomStream& Random, float MinDistance = 0.0f) const;

	/**
	 * Shortest walkable path between two world points, as world floor points (start excluded).
	 * Points within AvoidRadius of any world location in Avoid (people standing in the way)
	 * are treated as blocked, so the path goes round them or fails if they close it off.
	 */
	bool FindPath(const FVector& WorldStart, const FVector& WorldEnd, TArray<FVector>& OutPath,
		const TArray<FVector>& Avoid = TArray<FVector>(), float AvoidRadius = 70.0f) const;

	/** True if the two nodes are in the same connected area. */
	bool AreConnected(int32 A, int32 B) const;

	/** Walkable points in the connected area a node belongs to (0 if none). */
	int32 GetAreaSize(int32 Node) const;

	/** Log a top-down map of the floor nearest LocalFloorZ, one character per cell (debugging). */
	void DumpToLog(float LocalFloorZ) const;

	/** Height of a world point in the interior's frame. */
	float ToLocalZ(const FVector& WorldLocation) const;

	/** Log every surface the bake finds in one column, and why each is or isn't walkable. */
	void ProbeColumn(UWorld* World, float LocalX, float LocalY) const;

private:
	struct FNode
	{
		FVector Local = FVector::ZeroVector; // floor point, interior-local
		int32 X = 0;
		int32 Y = 0;
		int32 Area = INDEX_NONE;
		TArray<int32, TInlineAllocator<8>> Links;
	};

	/** Trace one column top-down; floors a person fits on go to OutFloors. */
	void SampleColumn(UWorld* World, float LX, float LY, float Top, float Bottom, const FCollisionQueryParams& Params,
		TArray<float, TInlineAllocator<4>>& OutFloors, bool& bOutBlocked, bool bLog) const;
	int32 FindNodeInColumn(int32 X, int32 Y, float LocalZ, float Tolerance) const;
	bool HasLineOfSight(int32 FromNode, int32 ToNode, const TBitArray<>* Blocked = nullptr) const;
	FIntPoint ToCell(const FVector& Local) const;

	FSettings Settings;
	FTransform InteriorTransform;
	FVector2D Origin = FVector2D::ZeroVector; // local XY of cell (0,0)
	float Cell = 40.0f;
	int32 SizeX = 0;
	int32 SizeY = 0;
	TArray<FNode> Nodes;
	/** Column -> nodes in it (one per floor layer). */
	TMap<int32, TArray<int32, TInlineAllocator<2>>> Columns;
	/** Columns where a floor was found but a person didn't fit (debugging). */
	TSet<int32> BlockedColumns;
	/** Nodes per connected area, for picking reachable destinations. */
	TArray<TArray<int32>> Areas;
	double BuildSeconds = 0.0;
	float SampleTop = 0.0f;
	float SampleBottom = 0.0f;
	FCollisionQueryParams SampleParams;
};
