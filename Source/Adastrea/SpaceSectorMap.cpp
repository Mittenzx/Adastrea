// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceSectorMap.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASpaceSectorMap::ASpaceSectorMap()
	: bNeighborCacheDirty(true)
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root scene component
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create billboard component for editor visualization
	SectorMarker = CreateDefaultSubobject<UBillboardComponent>(TEXT("SectorMarker"));
	SectorMarker->SetupAttachment(RootComponent);
	SectorMarker->bIsScreenSizeScaled = true;

	// Default sector name
	SectorName = FText::FromString(TEXT("Unnamed Sector"));
	Description = FText::FromString(TEXT("200km x 200km open world sector"));
}

void ASpaceSectorMap::BeginPlay()
{
	Super::BeginPlay();
	
	// Validate sector configuration
	if (!ValidateSectorConfiguration())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SpaceSectorMap: Sector '%s' has configuration issues"), *SectorName.ToString());
	}
	
	UE_LOG(LogAdastrea, Log, TEXT("SpaceSectorMap: Initialized sector '%s' at location (%.2f, %.2f, %.2f)"),
		*SectorName.ToString(),
		GetActorLocation().X,
		GetActorLocation().Y,
		GetActorLocation().Z);
}



#if WITH_EDITOR
void ASpaceSectorMap::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Mark neighbor cache as dirty
	bNeighborCacheDirty = true;

	// Log property changes in editor for debugging
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceSectorMap, SectorName))
		{
			UE_LOG(LogAdastrea, Log, TEXT("SpaceSectorMap: Sector name changed to '%s'"), *SectorName.ToString());
		}
	}
}

void ASpaceSectorMap::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	
	if (bFinished)
	{
		// Validate sector after move
		ValidateSectorConfiguration();
		
		// Mark neighbor cache as dirty since position changed
		bNeighborCacheDirty = true;
		
		UE_LOG(LogAdastrea, Log, TEXT("SpaceSectorMap: Sector '%s' moved to grid %s"), 
			*SectorName.ToString(), *GetGridCoordinates().ToString());
	}
}
#endif

FBox ASpaceSectorMap::GetSectorBounds() const
{
	const FVector Center = GetActorLocation();
	const float HalfSize = GetSectorHalfSize();
	
	// Create a box centered on this actor with SectorSize dimensions
	return FBox(
		Center - FVector(HalfSize, HalfSize, HalfSize),
		Center + FVector(HalfSize, HalfSize, HalfSize)
	);
}

float ASpaceSectorMap::GetSectorHalfSize() const
{
	return SectorSize / 2.0f;
}

bool ASpaceSectorMap::IsPositionInSector(const FVector& WorldPosition) const
{
	const FBox Bounds = GetSectorBounds();
	return Bounds.IsInside(WorldPosition);
}

FVector ASpaceSectorMap::GetRandomPositionInSector() const
{
	const FVector Center = GetActorLocation();
	const float HalfSize = GetSectorHalfSize();
	
	// Generate random position within sector bounds
	const FVector RandomOffset(
		FMath::FRandRange(-HalfSize, HalfSize),
		FMath::FRandRange(-HalfSize, HalfSize),
		FMath::FRandRange(-HalfSize, HalfSize)
	);
	
	return Center + RandomOffset;
}

FVector ASpaceSectorMap::GetSectorCenter() const
{
	return GetActorLocation();
}

FIntVector ASpaceSectorMap::GetGridCoordinates() const
{
	const FVector Center = GetSectorCenter();
	
	// Convert world position to grid coordinates
	// Each grid cell is one sector size (20,000,000 units = 200km)
	int32 GridX = FMath::RoundToInt(Center.X / SectorSize);
	int32 GridY = FMath::RoundToInt(Center.Y / SectorSize);
	int32 GridZ = FMath::RoundToInt(Center.Z / SectorSize);
	
	return FIntVector(GridX, GridY, GridZ);
}

TArray<AActor*> ASpaceSectorMap::GetActorsInSector(TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> ActorsInSector;
	
	if (!GetWorld())
	{
		return ActorsInSector;
	}
	
	const FBox Bounds = GetSectorBounds();
	
	// Get all actors of the specified class (or all actors if nullptr)
	TArray<AActor*> AllActors;
	if (ActorClass)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, AllActors);
	}
	else
	{
		// Get all actors in the world
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AllActors.Add(*It);
		}
	}
	
	// Filter actors within sector bounds
	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor != this && Bounds.IsInside(Actor->GetActorLocation()))
		{
			ActorsInSector.Add(Actor);
		}
	}
	
	return ActorsInSector;
}

int32 ASpaceSectorMap::GetActorCountInSector(TSubclassOf<AActor> ActorClass) const
{
	if (!GetWorld())
	{
		return 0;
	}
	
	const FBox Bounds = GetSectorBounds();
	int32 Count = 0;
	
	// Count actors within bounds
	if (ActorClass)
	{
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, AllActors);
		
		for (AActor* Actor : AllActors)
		{
			if (Actor && Actor != this && Bounds.IsInside(Actor->GetActorLocation()))
			{
				Count++;
			}
		}
	}
	else
	{
		// Count all actors in bounds
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor != this && Bounds.IsInside(Actor->GetActorLocation()))
			{
				Count++;
			}
		}
	}
	
	return Count;
}

TArray<ASpaceSectorMap*> ASpaceSectorMap::GetNeighboringSectors() const
{
	// Return cached neighbors if valid (cast away const for cache update)
	ASpaceSectorMap* MutableThis = const_cast<ASpaceSectorMap*>(this);
	
	if (bNeighborCacheDirty || CachedNeighboringSectors.Num() == 0)
	{
		MutableThis->RefreshNeighborCache();
	}
	
	return CachedNeighboringSectors;
}

float ASpaceSectorMap::GetDistanceToSector(const ASpaceSectorMap* OtherSector) const
{
	if (!OtherSector)
	{
		return -1.0f;
	}
	
	const FVector CurrentCenter = GetSectorCenter();
	const FVector OtherCenter = OtherSector->GetSectorCenter();
	
	return FVector::Dist(CurrentCenter, OtherCenter);
}

bool ASpaceSectorMap::ValidateSectorConfiguration() const
{
	bool bIsValid = true;
	
	if (!GetWorld())
	{
		return false;
	}
	
	// Check for overlapping sectors
	TArray<AActor*> AllSectorActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceSectorMap::StaticClass(), AllSectorActors);
	
	const FVector CurrentCenter = GetSectorCenter();
	const float MinDistance = SectorSize * 0.5f; // Sectors should be at least half a sector apart to avoid overlap
	
	for (AActor* Actor : AllSectorActors)
	{
		ASpaceSectorMap* OtherSector = Cast<ASpaceSectorMap>(Actor);
		if (OtherSector && OtherSector != this)
		{
			FVector OtherCenter = OtherSector->GetSectorCenter();
			float Distance = FVector::Dist(CurrentCenter, OtherCenter);
			
			if (Distance < MinDistance)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("SpaceSectorMap: Sector '%s' overlaps with '%s' (distance: %.2f)"),
					*SectorName.ToString(), *OtherSector->SectorName.ToString(), Distance);
				bIsValid = false;
			}
		}
	}
	
	// Check if sector name is valid
	if (SectorName.IsEmpty())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SpaceSectorMap: Sector at (%s) has empty name"),
			*GetActorLocation().ToString());
		bIsValid = false;
	}
	
	return bIsValid;
}

FString ASpaceSectorMap::GetDebugInfo() const
{
	FString DebugInfo;
	
	DebugInfo += FString::Printf(TEXT("Sector: %s\n"), *SectorName.ToString());
	DebugInfo += FString::Printf(TEXT("Description: %s\n"), *Description.ToString());
	DebugInfo += FString::Printf(TEXT("Location: %s\n"), *GetActorLocation().ToString());
	DebugInfo += FString::Printf(TEXT("Grid Coordinates: %s\n"), *GetGridCoordinates().ToString());
	DebugInfo += FString::Printf(TEXT("Size: %.2f km\n"), SectorSize / 100000.0f); // Convert to km
	DebugInfo += FString::Printf(TEXT("Bounds: %s\n"), *GetSectorBounds().ToString());
	DebugInfo += FString::Printf(TEXT("Actor Count: %d\n"), GetActorCountInSector());
	DebugInfo += FString::Printf(TEXT("Neighboring Sectors: %d\n"), GetNeighboringSectors().Num());
	
	return DebugInfo;
}

void ASpaceSectorMap::RefreshNeighborCache()
{
	// Rebuild the neighbor cache directly instead of going through GetNeighboringSectors()
	// to avoid any potential recursive dependency between the cache refresher and the getter.
	CachedNeighboringSectors.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		bNeighborCacheDirty = false;
		return;
	}

	// Collect all sector actors in the world
	TArray<AActor*> AllSectorActors;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceSectorMap::StaticClass(), AllSectorActors);

	const FVector CurrentCenter = GetSectorCenter();

	for (AActor* Actor : AllSectorActors)
	{
		ASpaceSectorMap* OtherSector = Cast<ASpaceSectorMap>(Actor);
		if (!OtherSector || OtherSector == this)
		{
			continue;
		}

		const float Distance = GetDistanceToSector(OtherSector);

		// Basic neighbor rule: any distinct sector within one sector size is considered a neighbor.
		// This uses the same distance computation utilities as the rest of this class and avoids
		// introducing new behavior outside of the existing spatial model.
		if (Distance >= 0.0f && Distance <= SectorSize)
		{
			CachedNeighboringSectors.Add(OtherSector);
		}
	}

	bNeighborCacheDirty = false;
}
