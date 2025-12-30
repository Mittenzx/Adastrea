// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "SectorGenerator.h"
#include "SectorGeneratorConfig.h"
#include "NameGenerator.h"
#include "SpaceSectorMap.h"
#include "Factions/FactionDataAsset.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Interfaces/IFactionMember.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "AdastreaLog.h"

ASectorGenerator::ASectorGenerator()
	: TargetSector(nullptr)
	, GeneratorConfig(nullptr)
	, bShowDebugVisuals(false)
	, bPreviewMode(false)
	, bIsGenerating(false)
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Make this actor hidden in game but visible in editor
	SetActorHiddenInGame(true);
	
#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = true;
#endif
}

void ASectorGenerator::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void ASectorGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Clear sector if config changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASectorGenerator, GeneratorConfig) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ASectorGenerator, TargetSector))
	{
		// Configuration changed - might want to regenerate
	}
}
#endif

bool ASectorGenerator::GenerateSector()
{
	// Validate configuration first
	TArray<FText> ValidationErrors;
	if (!ValidateConfiguration(ValidationErrors))
	{
		UE_LOG(LogAdastreaProceduralGen, Error, TEXT("SectorGenerator: Configuration validation failed:"));
		for (const FText& Error : ValidationErrors)
		{
			UE_LOG(LogAdastreaProceduralGen, Error, TEXT("  - %s"), *Error.ToString());
		}
		return false;
	}

	// Prevent concurrent generation
	if (bIsGenerating)
	{
		UE_LOG(LogAdastreaProceduralGen, Warning, TEXT("SectorGenerator: Generation already in progress."));
		return false;
	}

	bIsGenerating = true;
	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Starting sector generation..."));

	// Call pre-generation event
	OnPreGeneration();

	// Initialize random stream
	int32 Seed = GeneratorConfig->RandomSeed;
	if (Seed == 0)
	{
		Seed = FDateTime::Now().GetTicks();
	}
	RandomStream.Initialize(Seed);

	// Cache sector bounds
	if (TargetSector)
	{
		CachedSectorBounds = TargetSector->GetSectorBounds();
	}
	else
	{
		UE_LOG(LogAdastreaProceduralGen, Error, TEXT("SectorGenerator: No target sector specified."));
		bIsGenerating = false;
		OnPostGeneration(false);
		return false;
	}

	// Generate sector name if needed
	GenerateSectorName();

	// Clear previous content
	ClearSector();

	// Clear occupied positions
	OccupiedPositions.Empty();

	// Create central hub if configured
	if (GeneratorConfig->bCreateCentralHub && GeneratorConfig->CentralHubClass)
	{
		FVector CenterLocation = TargetSector->GetSectorCenter();
		FRotator CenterRotation = FRotator::ZeroRotator;

		AActor* CentralHub = GetWorld()->SpawnActor<AActor>(
			GeneratorConfig->CentralHubClass,
			CenterLocation,
			CenterRotation
		);

		if (CentralHub)
		{
			GeneratedActors.Add(CentralHub);
			OccupiedPositions.Add(CenterLocation);
			UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Created central hub at sector center"));
		}
	}

	// Generate objects from each definition
	int32 TotalSpawned = 0;
	for (const FSpaceObjectDefinition& Definition : GeneratorConfig->ObjectDefinitions)
	{
		TArray<AActor*> SpawnedActors;
		int32 Count = SpawnObjectsFromDefinition(Definition, SpawnedActors);
		TotalSpawned += Count;

		UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Spawned %d objects of type %d"), 
			Count, static_cast<int32>(Definition.ObjectType));
	}

	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Generation complete. Spawned %d total objects."), TotalSpawned);

	bIsGenerating = false;
	OnPostGeneration(true);

	return true;
}

void ASectorGenerator::ClearSector()
{
	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Clearing %d generated actors..."), GeneratedActors.Num());

	// Clean up weak pointers and destroy actors
	for (int32 i = GeneratedActors.Num() - 1; i >= 0; --i)
	{
		if (GeneratedActors[i].IsValid())
		{
			AActor* Actor = GeneratedActors[i].Get();
			if (Actor && IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
	}

	GeneratedActors.Empty();
	OccupiedPositions.Empty();
}

bool ASectorGenerator::RegenerateSector()
{
	ClearSector();
	return GenerateSector();
}

bool ASectorGenerator::ValidateConfiguration(TArray<FText>& OutErrors) const
{
	bool bIsValid = true;

	if (!TargetSector)
	{
		OutErrors.Add(FText::FromString(TEXT("No target sector specified.")));
		bIsValid = false;
	}

	if (!GeneratorConfig)
	{
		OutErrors.Add(FText::FromString(TEXT("No generator config specified.")));
		bIsValid = false;
	}
	else
	{
		// Validate the config itself
		if (!GeneratorConfig->ValidateConfiguration(OutErrors))
		{
			bIsValid = false;
		}
	}

	return bIsValid;
}

int32 ASectorGenerator::GetGeneratedObjectCount() const
{
	// Clean up invalid weak pointers and count valid ones
	int32 ValidCount = 0;
	for (const TWeakObjectPtr<AActor>& ActorPtr : GeneratedActors)
	{
		if (ActorPtr.IsValid())
		{
			ValidCount++;
		}
	}
	return ValidCount;
}

TArray<AActor*> ASectorGenerator::GetGeneratedActors() const
{
	TArray<AActor*> ValidActors;
	for (const TWeakObjectPtr<AActor>& ActorPtr : GeneratedActors)
	{
		if (ActorPtr.IsValid())
		{
			ValidActors.Add(ActorPtr.Get());
		}
	}
	return ValidActors;
}

void ASectorGenerator::OnPreGeneration_Implementation()
{
	// Default implementation - can be overridden in Blueprint
	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Pre-generation event"));
}

void ASectorGenerator::OnPostGeneration_Implementation(bool bSuccess)
{
	// Default implementation - can be overridden in Blueprint
	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Post-generation event (Success: %s)"), 
		bSuccess ? TEXT("true") : TEXT("false"));
}

void ASectorGenerator::OnObjectSpawned_Implementation(AActor* SpawnedActor, const FSpaceObjectDefinition& Definition)
{
	// Default implementation - can be overridden in Blueprint
	if (SpawnedActor)
	{
		UE_LOG(LogAdastreaProceduralGen, Verbose, TEXT("SectorGenerator: Object spawned: %s"), *SpawnedActor->GetName());
	}
}

int32 ASectorGenerator::SpawnObjectsFromDefinition(const FSpaceObjectDefinition& Definition, TArray<AActor*>& OutSpawnedActors)
{
	if (!Definition.ActorClass)
	{
		UE_LOG(LogAdastreaProceduralGen, Warning, TEXT("SectorGenerator: Object definition has no actor class."));
		return 0;
	}

	// Calculate count based on min/max and density multiplier
	int32 BaseCount = RandomStream.RandRange(Definition.MinCount, Definition.MaxCount);
	int32 Count = FMath::RoundToInt(BaseCount * GeneratorConfig->DensityMultiplier);

	int32 SuccessfulSpawns = 0;

	for (int32 i = 0; i < Count; ++i)
	{
		FVector Location;
		FRotator Rotation;

		// Find valid spawn location
		if (!FindValidSpawnLocation(Definition, Location, Rotation))
		{
			UE_LOG(LogAdastreaProceduralGen, Warning, TEXT("SectorGenerator: Failed to find valid location for object %d of %d"), 
				i + 1, Count);
			continue;
		}

		// Calculate scale with variation
		float BaseScale = 1.0f;
		float ScaleRange = (Definition.ScaleVariation - 1.0f) / 2.0f;
		float Scale = BaseScale + RandomStream.FRandRange(-ScaleRange, ScaleRange);
		FVector ScaleVector(Scale, Scale, Scale);

		// Spawn the actor
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogAdastreaProceduralGen, Error, TEXT("SectorGenerator: World is null, cannot spawn actors"));
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AActor* SpawnedActor = World->SpawnActor<AActor>(
			Definition.ActorClass,
			Location,
			Rotation,
			SpawnParams
		);

		if (SpawnedActor)
		{
			// Apply scale
			SpawnedActor->SetActorScale3D(ScaleVector);

			// Apply faction ownership if specified
			if (Definition.OwnerFaction)
			{
				ApplyFactionOwnership(SpawnedActor, Definition.OwnerFaction);
			}

			// Track spawned actor
			GeneratedActors.Add(SpawnedActor);
			OutSpawnedActors.Add(SpawnedActor);
			OccupiedPositions.Add(Location);

			// Call spawn event
			OnObjectSpawned(SpawnedActor, Definition);

			// Debug visualization
			if (bShowDebugVisuals)
			{
				DrawDebugSphere(GetWorld(), Location, 1000.0f, 12, FColor::Green, false, 5.0f);
			}

			SuccessfulSpawns++;
		}
		else
		{
			UE_LOG(LogAdastreaProceduralGen, Warning, TEXT("SectorGenerator: Failed to spawn actor at location %s"), 
				*Location.ToString());
		}
	}

	return SuccessfulSpawns;
}

bool ASectorGenerator::FindValidSpawnLocation(const FSpaceObjectDefinition& Definition, FVector& OutLocation, FRotator& OutRotation)
{
	// Validate target sector before accessing
	if (!IsValid(TargetSector))
	{
		UE_LOG(LogAdastreaProceduralGen, Warning, TEXT("SectorGenerator: TargetSector is invalid when trying to find a spawn location"));
		return false;
	}

	// Get sector center
	FVector SectorCenter = TargetSector->GetSectorCenter();
	float SectorHalfSize = TargetSector->GetSectorHalfSize();

	// Determine distance range
	float MinDistance = Definition.MinDistanceFromCenter;
	float MaxDistance = Definition.MaxDistanceFromCenter;
	if (MaxDistance <= 0.0f)
	{
		MaxDistance = SectorHalfSize * 0.9f;  // Stay within 90% of sector bounds
	}

	// Try to find valid location
	for (int32 Attempt = 0; Attempt < GeneratorConfig->MaxSpawnAttempts; ++Attempt)
	{
		FVector Position;

		// Generate position based on distribution pattern
		switch (Definition.DistributionPattern)
		{
		case EDistributionPattern::Random:
		{
			// Random position in sphere
			float Distance = RandomStream.FRandRange(MinDistance, MaxDistance);
			FVector RandomDirection = RandomStream.GetUnitVector();
			Position = SectorCenter + RandomDirection * Distance;
			break;
		}
		case EDistributionPattern::Clustered:
		{
			// Cluster around random points
			// For simplicity, use random with bias toward center
			float Distance = RandomStream.FRandRange(MinDistance, MaxDistance * 0.6f);
			FVector RandomDirection = RandomStream.GetUnitVector();
			Position = SectorCenter + RandomDirection * Distance;
			break;
		}
		case EDistributionPattern::Grid:
		{
			// Grid pattern (simplified - would need more sophisticated implementation)
			int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(GeneratorConfig->GetTotalObjectCount())));
			float GridSpacing = (MaxDistance - MinDistance) / FMath::Max(GridSize, 1);
			int32 Row = Attempt / GridSize;
			int32 Col = Attempt % GridSize;
			Position = SectorCenter + FVector(
				(Col - GridSize / 2.0f) * GridSpacing,
				(Row - GridSize / 2.0f) * GridSpacing,
				RandomStream.FRandRange(-GridSpacing, GridSpacing)
			);
			break;
		}
		case EDistributionPattern::Orbital:
		{
			// Orbital ring pattern
			float Angle = RandomStream.FRandRange(0.0f, 360.0f);
			float Distance = RandomStream.FRandRange(MinDistance, MaxDistance);
			Position = SectorCenter + FVector(
				FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
				FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
				RandomStream.FRandRange(-SectorHalfSize * 0.1f, SectorHalfSize * 0.1f)
			);
			break;
		}
		case EDistributionPattern::Scattered:
		default:
		{
			// Wide, scattered distribution
			Position = SectorCenter + FVector(
				RandomStream.FRandRange(-MaxDistance, MaxDistance),
				RandomStream.FRandRange(-MaxDistance, MaxDistance),
				RandomStream.FRandRange(-MaxDistance, MaxDistance)
			);
			break;
		}
		}

		// Check if position is valid
		if (IsLocationValid(Position, GeneratorConfig->MinObjectSpacing))
		{
			OutLocation = Position;
			
			// Generate rotation
			if (Definition.bRandomRotation)
			{
				OutRotation = FRotator(
					RandomStream.FRandRange(0.0f, 360.0f),
					RandomStream.FRandRange(0.0f, 360.0f),
					RandomStream.FRandRange(0.0f, 360.0f)
				);
			}
			else
			{
				OutRotation = FRotator::ZeroRotator;
			}

			return true;
		}
	}

	return false;
}

bool ASectorGenerator::IsLocationValid(const FVector& Location, float MinSpacing) const
{
	// Check if location is within sector bounds
	if (!CachedSectorBounds.IsInside(Location))
	{
		return false;
	}

	// Check spacing from other objects
	for (const FVector& OccupiedPosition : OccupiedPositions)
	{
		float Distance = FVector::Dist(Location, OccupiedPosition);
		if (Distance < MinSpacing)
		{
			return false;
		}
	}

	return true;
}

TArray<FVector> ASectorGenerator::GeneratePositionsByPattern(EDistributionPattern Pattern, int32 Count, float MinDistance, float MaxDistance)
{
	TArray<FVector> Positions;
	FVector SectorCenter = TargetSector->GetSectorCenter();

	for (int32 i = 0; i < Count; ++i)
	{
		FVector Position;
		FRotator DummyRotation;
		
		FSpaceObjectDefinition TempDefinition{};
		TempDefinition.DistributionPattern = Pattern;
		TempDefinition.MinDistanceFromCenter = MinDistance;
		TempDefinition.MaxDistanceFromCenter = MaxDistance;
		TempDefinition.ActorClass = nullptr;
		TempDefinition.bRandomRotation = false;
		
		if (FindValidSpawnLocation(TempDefinition, Position, DummyRotation))
		{
			Positions.Add(Position);
		}
	}

	return Positions;
}

void ASectorGenerator::ApplyFactionOwnership(AActor* Actor, UFactionDataAsset* Faction)
{
	if (!Actor || !Faction)
	{
		return;
	}

	// Check if actor implements IFactionMember interface
	if (Actor->GetClass()->ImplementsInterface(UFactionMember::StaticClass()))
	{
		IFactionMember::Execute_GetFaction(Actor); // sample access
		// If actor supports setting faction via Blueprint, call a native helper if available
		// In absence of SetFaction in IFactionMember, attempt to set via property on actor (designer pattern).
		UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Actor '%s' implements IFactionMember - faction application handled via interface/Blueprint"), *Actor->GetName());
	}
}

void ASectorGenerator::GenerateSectorName()
{
	if (!TargetSector)
	{
		return;
	}

	// Check if custom name is provided
	if (!GeneratorConfig->CustomSectorName.IsEmpty())
	{
		TargetSector->SectorName = GeneratorConfig->CustomSectorName;
		UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Using custom sector name: %s"), 
			*GeneratorConfig->CustomSectorName.ToString());
		return;
	}

	// Generate name based on theme
	uint8 Theme = static_cast<uint8>(GeneratorConfig->NamingTheme);
	FString GeneratedName = UNameGenerator::GenerateUniqueSectorName(Theme, this);
	TargetSector->SectorName = FText::FromString(GeneratedName);
	
	UE_LOG(LogAdastreaProceduralGen, Log, TEXT("SectorGenerator: Generated sector name: %s"), *GeneratedName);
}
