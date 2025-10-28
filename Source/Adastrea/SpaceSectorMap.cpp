// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceSectorMap.h"
#include "AdastreaLog.h"

ASpaceSectorMap::ASpaceSectorMap()
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
