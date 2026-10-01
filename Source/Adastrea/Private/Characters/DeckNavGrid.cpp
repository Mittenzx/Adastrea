// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/DeckNavGrid.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformTime.h"
#include "Algo/Reverse.h"
#include "AdastreaLog.h"

namespace
{
	int32 ColumnKey(int32 X, int32 Y) { return (Y << 16) | (X & 0xFFFF); }

	/** Down-trace attempts per column (floors, ceilings, slabs, start-inside retries). */
	constexpr int32 MaxTracesPerColumn = 24;
	/** Floors kept per column (stacked decks). */
	constexpr int32 MaxLayersPerColumn = 6;
}

void FDeckNavGrid::Build(UWorld* World, const AActor* Interior, const FBox& LocalBounds, const TArray<const AActor*>& Ignore,
	const FSettings& InSettings)
{
	const double StartTime = FPlatformTime::Seconds();
	Settings = InSettings;
	Nodes.Reset();
	Columns.Reset();
	BlockedColumns.Reset();
	Areas.Reset();
	if (!World || !Interior || !LocalBounds.IsValid)
	{
		return;
	}

	// Work in cm in the interior's frame regardless of the actor's scale.
	InteriorTransform = Interior->GetActorTransform();
	InteriorTransform.SetScale3D(FVector::OneVector);
	Cell = Settings.CellSize;
	const FVector Size = LocalBounds.GetSize();
	const double Columns2D = (Size.X / Cell) * (Size.Y / Cell);
	if (Columns2D > Settings.MaxColumns)
	{
		Cell *= FMath::Sqrt(Columns2D / Settings.MaxColumns);
	}
	SizeX = FMath::Max(1, FMath::CeilToInt(Size.X / Cell));
	SizeY = FMath::Max(1, FMath::CeilToInt(Size.Y / Cell));
	Origin = FVector2D(LocalBounds.Min.X + Cell * 0.5f, LocalBounds.Min.Y + Cell * 0.5f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(DeckNavGrid), /*bTraceComplex=*/false);
	for (const AActor* Actor : Ignore)
	{
		Params.AddIgnoredActor(Actor);
	}
	const float Top = LocalBounds.Max.Z + 10.0f;
	const float Bottom = LocalBounds.Min.Z - 10.0f;
	SampleTop = Top;
	SampleBottom = Bottom;
	SampleParams = Params;

	// 1) Sample every column top-down; keep each walkable floor a person fits on.
	for (int32 Y = 0; Y < SizeY; ++Y)
	{
		for (int32 X = 0; X < SizeX; ++X)
		{
			TArray<float, TInlineAllocator<4>> Floors;
			bool bBlocked = false;
			SampleColumn(World, Origin.X + X * Cell, Origin.Y + Y * Cell, Top, Bottom, Params, Floors, bBlocked, /*bLog=*/false);
			for (const float FloorZ : Floors)
			{
				FNode& Node = Nodes.AddDefaulted_GetRef();
				Node.Local = FVector(Origin.X + X * Cell, Origin.Y + Y * Cell, FloorZ);
				Node.X = X;
				Node.Y = Y;
				Columns.FindOrAdd(ColumnKey(X, Y)).Add(Nodes.Num() - 1);
			}
			if (bBlocked && Floors.Num() == 0)
			{
				BlockedColumns.Add(ColumnKey(X, Y));
			}
		}
	}

	// 2) Link neighbours within a step of each other. Diagonals need both orthogonal
	// neighbours open too, so paths never cut a wall corner.
	auto NodeNear = [this](int32 X, int32 Y, float Z) { return FindNodeInColumn(X, Y, Z, Settings.MaxStep); };
	const FIntPoint Forward[] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		const FNode Node = Nodes[Index];
		for (const FIntPoint& D : Forward)
		{
			const int32 Other = NodeNear(Node.X + D.X, Node.Y + D.Y, Node.Local.Z);
			if (Other == INDEX_NONE)
			{
				continue;
			}
			if (D.X != 0 && D.Y != 0 &&
				(NodeNear(Node.X + D.X, Node.Y, Node.Local.Z) == INDEX_NONE || NodeNear(Node.X, Node.Y + D.Y, Node.Local.Z) == INDEX_NONE))
			{
				continue;
			}
			Nodes[Index].Links.Add(Other);
			Nodes[Other].Links.Add(Index);
		}
	}

	// 3) Connected areas (a deck's walkable floor vs. stray ledges, roofs and furniture tops).
	for (int32 Seed = 0; Seed < Nodes.Num(); ++Seed)
	{
		if (Nodes[Seed].Area != INDEX_NONE)
		{
			continue;
		}
		const int32 Area = Areas.Num();
		TArray<int32>& Members = Areas.AddDefaulted_GetRef();
		TArray<int32> Stack = { Seed };
		Nodes[Seed].Area = Area;
		while (Stack.Num() > 0)
		{
			const int32 Current = Stack.Pop(EAllowShrinking::No);
			Members.Add(Current);
			for (const int32 Next : Nodes[Current].Links)
			{
				if (Nodes[Next].Area == INDEX_NONE)
				{
					Nodes[Next].Area = Area;
					Stack.Add(Next);
				}
			}
		}
	}

	BuildSeconds = FPlatformTime::Seconds() - StartTime;
	int32 Largest = 0;
	for (const TArray<int32>& Members : Areas)
	{
		Largest = FMath::Max(Largest, Members.Num());
	}
	UE_LOG(LogAdastrea, Log, TEXT("DeckNavGrid %s: %dx%d cells of %.0f cm from local (%.0f, %.0f), %d walkable points in %d areas (largest %d), %.1f ms"),
		*Interior->GetName(), SizeX, SizeY, Cell, Origin.X, Origin.Y, Nodes.Num(), Areas.Num(), Largest, BuildSeconds * 1000.0);
}

void FDeckNavGrid::DumpToLog(float LocalFloorZ) const
{
	// The floor nearest LocalFloorZ in each column (a deck level, not the roof above it):
	// area as 0-9/a-z ('+' beyond), 'x' = floor seen but no room for a person,
	// '.' = nothing walkable at that level. X runs left to right, +Y downwards.
	for (int32 Y = 0; Y < SizeY; ++Y)
	{
		FString Row;
		for (int32 X = 0; X < SizeX; ++X)
		{
			TCHAR Char = TEXT('.');
			const int32 Node = FindNodeInColumn(X, Y, LocalFloorZ, 60.0f);
			if (Node != INDEX_NONE)
			{
				const int32 Area = Nodes[Node].Area;
				Char = Area < 10 ? TCHAR('0' + Area) : Area < 36 ? TCHAR('a' + Area - 10) : TEXT('+');
			}
			else if (BlockedColumns.Contains(ColumnKey(X, Y)))
			{
				Char = TEXT('x');
			}
			Row.AppendChar(Char);
		}
		UE_LOG(LogAdastrea, Log, TEXT("NavGrid %3d |%s|"), Y, *Row);
	}
}

float FDeckNavGrid::ToLocalZ(const FVector& WorldLocation) const
{
	return InteriorTransform.InverseTransformPosition(WorldLocation).Z;
}

void FDeckNavGrid::SampleColumn(UWorld* World, float LX, float LY, float Top, float Bottom, const FCollisionQueryParams& Params,
	TArray<float, TInlineAllocator<4>>& OutFloors, bool& bOutBlocked, bool bLog) const
{
	const FQuat Rotation = InteriorTransform.GetRotation();
	const FCollisionShape Capsule = FCollisionShape::MakeCapsule(Settings.AgentRadius, Settings.AgentHalfHeight);
	float Z = Top;
	int32 Layers = 0;
	for (int32 Attempt = 0; Attempt < MaxTracesPerColumn && Z > Bottom && Layers < MaxLayersPerColumn; ++Attempt)
	{
		const FVector Start = InteriorTransform.TransformPosition(FVector(LX, LY, Z));
		const FVector End = InteriorTransform.TransformPosition(FVector(LX, LY, Bottom));
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
		{
			break;
		}
		if (Hit.bStartPenetrating)
		{
			if (bLog)
			{
				UE_LOG(LogAdastrea, Log, TEXT("  from z=%.1f: started inside %s"), Z, *GetNameSafe(Hit.GetComponent()));
			}
			Z -= 10.0f; // started inside a slab: step through it
			continue;
		}
		const FVector LocalHit = InteriorTransform.InverseTransformPosition(Hit.ImpactPoint);
		const FVector LocalNormal = InteriorTransform.InverseTransformVectorNoScale(Hit.ImpactNormal);
		const FVector Centre = InteriorTransform.TransformPosition(LocalHit + FVector(0.0f, 0.0f, Settings.AgentHalfHeight + 3.0f));
		const bool bFloor = LocalNormal.Z >= Settings.MinFloorNormalZ;
		bool bFits = false;
		if (bFloor)
		{
			// Blocking only: trigger volumes (the helm seat's, doors) overlap people but don't stop them.
			bFits = !World->OverlapBlockingTestByChannel(Centre, Rotation, ECC_Pawn, Capsule, Params);
			if (bFits)
			{
				OutFloors.Add(LocalHit.Z);
			}
			else
			{
				bOutBlocked = true;
			}
			++Layers;
		}
		if (bLog)
		{
			FString Blockers;
			if (bFloor && !bFits)
			{
				TArray<FOverlapResult> Overlaps;
				World->OverlapMultiByChannel(Overlaps, Centre, Rotation, ECC_Pawn, Capsule, Params);
				for (const FOverlapResult& Overlap : Overlaps)
				{
					if (Overlap.bBlockingHit)
					{
						Blockers += FString::Printf(TEXT(" %s"), *GetNameSafe(Overlap.GetComponent()));
					}
				}
			}
			UE_LOG(LogAdastrea, Log, TEXT("  from z=%.1f: hit %s at z=%.1f normalZ=%.2f %s%s"), Z, *GetNameSafe(Hit.GetComponent()),
				LocalHit.Z, LocalNormal.Z, !bFloor ? TEXT("(not floor)") : bFits ? TEXT("WALKABLE") : TEXT("blocked by"), *Blockers);
		}
		Z = LocalHit.Z - 2.0f; // carry on below this surface
	}
}

void FDeckNavGrid::ProbeColumn(UWorld* World, float LocalX, float LocalY) const
{
	UE_LOG(LogAdastrea, Log, TEXT("DeckNavGrid probe at local (%.0f, %.0f), z %.0f..%.0f:"), LocalX, LocalY, SampleTop, SampleBottom);
	TArray<float, TInlineAllocator<4>> Floors;
	bool bBlocked = false;
	SampleColumn(World, LocalX, LocalY, SampleTop, SampleBottom, SampleParams, Floors, bBlocked, /*bLog=*/true);
}

FIntPoint FDeckNavGrid::ToCell(const FVector& Local) const
{
	return FIntPoint(FMath::RoundToInt((Local.X - Origin.X) / Cell), FMath::RoundToInt((Local.Y - Origin.Y) / Cell));
}

int32 FDeckNavGrid::FindNodeInColumn(int32 X, int32 Y, float LocalZ, float Tolerance) const
{
	if (X < 0 || Y < 0 || X >= SizeX || Y >= SizeY)
	{
		return INDEX_NONE;
	}
	const auto* Column = Columns.Find(ColumnKey(X, Y));
	if (!Column)
	{
		return INDEX_NONE;
	}
	int32 Best = INDEX_NONE;
	float BestDelta = Tolerance;
	for (const int32 Node : *Column)
	{
		const float Delta = FMath::Abs(Nodes[Node].Local.Z - LocalZ);
		if (Delta <= BestDelta)
		{
			BestDelta = Delta;
			Best = Node;
		}
	}
	return Best;
}

FVector FDeckNavGrid::GetNodeLocation(int32 Node) const
{
	return Nodes.IsValidIndex(Node) ? InteriorTransform.TransformPosition(Nodes[Node].Local) : FVector::ZeroVector;
}

int32 FDeckNavGrid::FindNearestNode(const FVector& WorldLocation, float MaxDistance) const
{
	const FVector Local = InteriorTransform.InverseTransformPosition(WorldLocation);
	const FIntPoint C = ToCell(Local);
	const int32 Rings = FMath::Max(1, FMath::CeilToInt(MaxDistance / Cell));
	int32 Best = INDEX_NONE;
	float BestDist = MaxDistance;
	for (int32 DY = -Rings; DY <= Rings; ++DY)
	{
		for (int32 DX = -Rings; DX <= Rings; ++DX)
		{
			if (const auto* Column = Columns.Find(ColumnKey(C.X + DX, C.Y + DY)))
			{
				if (C.X + DX < 0 || C.Y + DY < 0 || C.X + DX >= SizeX || C.Y + DY >= SizeY)
				{
					continue;
				}
				for (const int32 Node : *Column)
				{
					// Feet usually sit on the point; anything up to a capsule above it counts as "on" it.
					const FVector Delta = Nodes[Node].Local - Local;
					const float Vertical = Delta.Z > 0.0f ? Delta.Z * 2.0f : FMath::Max(0.0f, -Delta.Z - 100.0f);
					const float Dist = FMath::Sqrt(Delta.X * Delta.X + Delta.Y * Delta.Y + Vertical * Vertical);
					if (Dist < BestDist)
					{
						BestDist = Dist;
						Best = Node;
					}
				}
			}
		}
	}
	return Best;
}

int32 FDeckNavGrid::GetAreaSize(int32 Node) const
{
	return Nodes.IsValidIndex(Node) && Areas.IsValidIndex(Nodes[Node].Area) ? Areas[Nodes[Node].Area].Num() : 0;
}

bool FDeckNavGrid::AreConnected(int32 A, int32 B) const
{
	return Nodes.IsValidIndex(A) && Nodes.IsValidIndex(B) && Nodes[A].Area == Nodes[B].Area;
}

int32 FDeckNavGrid::GetRandomReachableNode(int32 FromNode, FRandomStream& Random, float MinDistance) const
{
	if (!Nodes.IsValidIndex(FromNode))
	{
		return INDEX_NONE;
	}
	const TArray<int32>& Area = Areas[Nodes[FromNode].Area];
	int32 Fallback = INDEX_NONE;
	for (int32 Try = 0; Try < 16; ++Try)
	{
		const int32 Candidate = Area[Random.RandRange(0, Area.Num() - 1)];
		if (FVector::Dist2D(Nodes[Candidate].Local, Nodes[FromNode].Local) >= MinDistance)
		{
			return Candidate;
		}
		Fallback = Candidate;
	}
	return Fallback;
}

bool FDeckNavGrid::HasLineOfSight(int32 FromNode, int32 ToNode, const TBitArray<>* Blocked) const
{
	// Walk the straight line in half-cell steps, following the floor height; every
	// step must land on a walkable point within a step of the last one.
	const FVector A = Nodes[FromNode].Local;
	const FVector B = Nodes[ToNode].Local;
	const float Length = FVector::Dist2D(A, B);
	const int32 Steps = FMath::CeilToInt(Length / (Cell * 0.5f));
	float Z = A.Z;
	for (int32 Step = 1; Step <= Steps; ++Step)
	{
		const FVector P = FMath::Lerp(A, B, static_cast<float>(Step) / Steps);
		const FIntPoint C = ToCell(P);
		const int32 Node = FindNodeInColumn(C.X, C.Y, Z, Settings.MaxStep);
		if (Node == INDEX_NONE || (Blocked && (*Blocked)[Node]))
		{
			return false;
		}
		Z = Nodes[Node].Local.Z;
	}
	return FMath::Abs(Z - B.Z) <= Settings.MaxStep;
}

bool FDeckNavGrid::FindPath(const FVector& WorldStart, const FVector& WorldEnd, TArray<FVector>& OutPath,
	const TArray<FVector>& Avoid, float AvoidRadius) const
{
	OutPath.Reset();
	const int32 Start = FindNearestNode(WorldStart);
	const int32 Goal = FindNearestNode(WorldEnd);
	if (!AreConnected(Start, Goal))
	{
		return false;
	}

	// Points people are standing on (never the start: that's where we are).
	TBitArray<> Blocked(false, Nodes.Num());
	const int32 Reach = FMath::CeilToInt(AvoidRadius / Cell);
	for (const FVector& WorldPoint : Avoid)
	{
		const FVector Local = InteriorTransform.InverseTransformPosition(WorldPoint);
		const FIntPoint C = ToCell(Local);
		for (int32 DY = -Reach; DY <= Reach; ++DY)
		{
			for (int32 DX = -Reach; DX <= Reach; ++DX)
			{
				if (const auto* Column = Columns.Find(ColumnKey(C.X + DX, C.Y + DY)))
				{
					for (const int32 Node : *Column)
					{
						// Same floor (the point is a capsule centre, ~90 cm above it) and within reach.
						const FVector Delta = Nodes[Node].Local - Local;
						if (FVector2D(Delta.X, Delta.Y).Size() <= AvoidRadius && Delta.Z > -180.0f && Delta.Z < 40.0f)
						{
							Blocked[Node] = true;
						}
					}
				}
			}
		}
	}
	Blocked[Start] = false;
	if (Blocked[Goal])
	{
		return false;
	}

	// A*.
	TArray<float> Cost;
	TArray<int32> Parent;
	Cost.Init(TNumericLimits<float>::Max(), Nodes.Num());
	Parent.Init(INDEX_NONE, Nodes.Num());
	struct FOpen
	{
		float Score;
		int32 Node;
		bool operator<(const FOpen& Other) const { return Score < Other.Score; }
	};
	TArray<FOpen> Open;
	Cost[Start] = 0.0f;
	Open.HeapPush({ static_cast<float>(FVector::Dist(Nodes[Start].Local, Nodes[Goal].Local)), Start });
	bool bFound = false;
	while (Open.Num() > 0)
	{
		FOpen Current;
		Open.HeapPop(Current, EAllowShrinking::No);
		if (Current.Node == Goal)
		{
			bFound = true;
			break;
		}
		const float CurrentCost = Cost[Current.Node];
		if (Current.Score - FVector::Dist(Nodes[Current.Node].Local, Nodes[Goal].Local) > CurrentCost + 1.0f)
		{
			continue; // stale entry
		}
		for (const int32 Next : Nodes[Current.Node].Links)
		{
			if (Blocked[Next])
			{
				continue;
			}
			const float NewCost = CurrentCost + FVector::Dist(Nodes[Current.Node].Local, Nodes[Next].Local);
			if (NewCost < Cost[Next])
			{
				Cost[Next] = NewCost;
				Parent[Next] = Current.Node;
				Open.HeapPush({ NewCost + static_cast<float>(FVector::Dist(Nodes[Next].Local, Nodes[Goal].Local)), Next });
			}
		}
	}
	if (!bFound)
	{
		return false;
	}

	TArray<int32> Raw;
	for (int32 Node = Goal; Node != INDEX_NONE; Node = Parent[Node])
	{
		Raw.Add(Node);
	}
	Algo::Reverse(Raw);

	// String-pull: from each kept point jump to the furthest one in straight sight.
	int32 Current = 0;
	while (Current < Raw.Num() - 1)
	{
		int32 Furthest = Current + 1;
		// Look ahead a bounded distance: long ship-length paths stay cheap to smooth.
		for (int32 Candidate = FMath::Min(Raw.Num() - 1, Current + 48); Candidate > Current + 1; --Candidate)
		{
			if (HasLineOfSight(Raw[Current], Raw[Candidate], &Blocked))
			{
				Furthest = Candidate;
				break;
			}
		}
		OutPath.Add(GetNodeLocation(Raw[Furthest]));
		Current = Furthest;
	}
	return true;
}
