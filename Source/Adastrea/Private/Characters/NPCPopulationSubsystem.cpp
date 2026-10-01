// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/NPCPopulationSubsystem.h"
#include "Characters/AdastreaNPC.h"
#include "Characters/AdastreaNPCController.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Stations/StationInterior.h"
#include "Player/PlayerInteractableComponent.h"
#include "Player/WorldInteractable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "AdastreaLog.h"

namespace
{
	TAutoConsoleVariable<int32> CVarNPCMax(
		TEXT("adastrea.NPC.Max"), 6,
		TEXT("Most crew spawned aboard the interior the player is walking."));
	TAutoConsoleVariable<bool> CVarNPCEnable(
		TEXT("adastrea.NPC.Enable"), true,
		TEXT("Spawn crew in the interior the player is walking."));

	/** Seconds after the player arrives before the deck is baked: boarding lowers the
	 * avatar in over a moment and turns the deck's walk collision on. */
	constexpr float SettleSeconds = 1.5f;

	const TCHAR* FirstNames[] = { TEXT("Ade"), TEXT("Mara"), TEXT("Kenji"), TEXT("Sofia"), TEXT("Tomas"), TEXT("Priya"),
		TEXT("Leon"), TEXT("Yara"), TEXT("Ivo"), TEXT("Nadia"), TEXT("Ravi"), TEXT("Elin"), TEXT("Omar"), TEXT("Hana"),
		TEXT("Jonas"), TEXT("Zara"), TEXT("Mateo"), TEXT("Ines"), TEXT("Kofi"), TEXT("Lena") };
	const TCHAR* LastNames[] = { TEXT("Okafor"), TEXT("Varga"), TEXT("Sato"), TEXT("Moreau"), TEXT("Lindqvist"), TEXT("Rao"),
		TEXT("Petrov"), TEXT("Haddad"), TEXT("Kowalski"), TEXT("Mensah"), TEXT("Ibarra"), TEXT("Novak"), TEXT("Chen"),
		TEXT("Brandt"), TEXT("Silva"), TEXT("Adeyemi") };

	/** Who's aboard, in order: a ship needs a pilot and an engineer before anyone else. */
	const ENPCRole RoleOrder[] = { ENPCRole::Pilot, ENPCRole::Engineer, ENPCRole::Crew, ENPCRole::Medic,
		ENPCRole::Cook, ENPCRole::Security, ENPCRole::Crew, ENPCRole::Engineer };
}

bool UNPCPopulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UNPCPopulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNPCPopulationSubsystem, STATGROUP_Tickables);
}

void UNPCPopulationSubsystem::Deinitialize()
{
	ClearCrew();
	Super::Deinitialize();
}

AActor* UNPCPopulationSubsystem::GetPlayerInterior(ASpaceshipAvatar*& OutAvatar) const
{
	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	OutAvatar = PC ? Cast<ASpaceshipAvatar>(PC->GetPawn()) : nullptr;
	if (!OutAvatar)
	{
		return nullptr;
	}
	if (OutAvatar->CurrentInterior)
	{
		return OutAvatar->CurrentInterior;
	}
	if (OutAvatar->bWalkingStation)
	{
		// Station rooms are separate actors; the one the avatar stands in is the nearest.
		AActor* Nearest = nullptr;
		double NearestDist = 5000.0;
		for (TActorIterator<AStationInterior> It(GetWorld()); It; ++It)
		{
			const double Dist = FVector::Dist(It->GetActorLocation(), OutAvatar->GetActorLocation());
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				Nearest = *It;
			}
		}
		return Nearest;
	}
	return nullptr;
}

void UNPCPopulationSubsystem::Tick(float DeltaTime)
{
	ASpaceshipAvatar* Avatar = nullptr;
	AActor* Interior = GetPlayerInterior(Avatar);
	if (Interior != CurrentInterior.Get())
	{
		// Left (or switched) interiors: the old crew goes back to being numbers.
		ClearCrew();
		Grid = FDeckNavGrid();
		Fixtures.Reset();
		CurrentInterior = Interior;
		SettleTime = 0.0f;
		bPopulated = false;
	}
	if (!Interior || bPopulated || !CVarNPCEnable.GetValueOnGameThread())
	{
		return;
	}
	SettleTime += DeltaTime;
	if (SettleTime >= SettleSeconds)
	{
		bPopulated = true;
		Populate(Avatar, Interior);
	}
}

void UNPCPopulationSubsystem::Populate(ASpaceshipAvatar* Avatar, AActor* Interior)
{
	UWorld* World = GetWorld();
	FBox LocalBounds = Interior->CalculateComponentsBoundingBoxInLocalSpace(/*bNonColliding=*/true);
	LocalBounds = LocalBounds.ExpandBy(20.0f);

	// People never count as floor or wall.
	TArray<const AActor*> People;
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		People.Add(*It);
	}
	Grid.Build(World, Interior, LocalBounds, People);

	// Anything the player can use in here is somewhere crew can go and work.
	FTransform InteriorTransform = Interior->GetActorTransform();
	InteriorTransform.SetScale3D(FVector::OneVector);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (Candidate == Interior || Candidate->IsA<APawn>())
		{
			continue;
		}
		const bool bInside = Candidate->GetOwner() == Interior ||
			LocalBounds.IsInside(InteriorTransform.InverseTransformPosition(Candidate->GetActorLocation()));
		if (bInside && (Candidate->Implements<UWorldInteractable>() || Candidate->FindComponentByClass<UPlayerInteractableComponent>()))
		{
			Fixtures.Add(Candidate);
		}
	}

	// Crew size: what the ship needs to run, capped for performance and by deck size.
	int32 Count = 4;
	if (Avatar && Avatar->SourceShip && Avatar->SourceShip->ShipDataAsset)
	{
		Count = Avatar->SourceShip->ShipDataAsset->CrewRequired;
	}
	Count = FMath::Clamp(Count, 1, FMath::Max(0, CVarNPCMax.GetValueOnGameThread()));
	// ...and by the floor the player can actually reach: about one person per 10 m^2,
	// so a cramped fighter's single corridor doesn't fill up with people.
	const int32 PlayerNode = Avatar ? Grid.FindNearestNode(Avatar->GetActorLocation() - FVector(0.0f, 0.0f, 88.0f), 400.0f) : INDEX_NONE;
	Count = FMath::Min(Count, Grid.GetAreaSize(PlayerNode) / 60);
	UE_LOG(LogAdastrea, Log, TEXT("NPCPopulation: %s has %d fixtures; spawning %d crew"), *Interior->GetName(), Fixtures.Num(), Count);
	SpawnCrew(Count);
}

int32 UNPCPopulationSubsystem::SpawnCrew(int32 Count)
{
	UWorld* World = GetWorld();
	ASpaceshipAvatar* Avatar = nullptr;
	AActor* Interior = GetPlayerInterior(Avatar);
	if (!Interior || Interior != CurrentInterior.Get() || !Grid.IsValid() || !Avatar)
	{
		return 0;
	}
	// Spawn on the floor the player can reach, a little away from them.
	const int32 PlayerNode = Grid.FindNearestNode(Avatar->GetActorLocation() - FVector(0.0f, 0.0f, 88.0f), 400.0f);
	if (PlayerNode == INDEX_NONE)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("NPCPopulation: the player isn't on %s's walkable grid; no crew spawned"), *Interior->GetName());
		return 0;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	int32 Spawned = 0;
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Node = Grid.GetRandomReachableNode(PlayerNode, Random, 250.0f);
		if (Node == INDEX_NONE)
		{
			break;
		}
		const FVector Location = Grid.GetNodeLocation(Node) + FVector(0.0f, 0.0f, 92.0f);
		const FRotator Facing(0.0f, Random.FRandRange(-180.0f, 180.0f), 0.0f);
		AAdastreaNPC* NPC = World->SpawnActor<AAdastreaNPC>(AAdastreaNPC::StaticClass(), Location, Facing, Params);
		if (!NPC)
		{
			continue;
		}
		NPC->DisplayName = FText::FromString(FString::Printf(TEXT("%s %s"),
			FirstNames[Random.RandRange(0, UE_ARRAY_COUNT(FirstNames) - 1)],
			LastNames[Random.RandRange(0, UE_ARRAY_COUNT(LastNames) - 1)]));
		NPC->CrewRole = RoleOrder[Crew.Num() % UE_ARRAY_COUNT(RoleOrder)];
		if (AAdastreaNPCController* Brain = Cast<AAdastreaNPCController>(NPC->GetController()))
		{
			Brain->HomeInterior = Interior;
		}
		Crew.Add(NPC);
		++Spawned;
	}
	return Spawned;
}

void UNPCPopulationSubsystem::ClearCrew()
{
	for (const TWeakObjectPtr<AAdastreaNPC>& NPC : Crew)
	{
		if (NPC.IsValid())
		{
			if (AController* Brain = NPC->GetController())
			{
				Brain->Destroy();
			}
			NPC->Destroy();
		}
	}
	Crew.Reset();
}

const FDeckNavGrid* UNPCPopulationSubsystem::GetGrid(const AActor* Interior) const
{
	return Interior && Interior == CurrentInterior.Get() && Grid.IsValid() ? &Grid : nullptr;
}

const TArray<TWeakObjectPtr<AActor>>& UNPCPopulationSubsystem::GetFixtures(const AActor* Interior) const
{
	static const TArray<TWeakObjectPtr<AActor>> None;
	return Interior && Interior == CurrentInterior.Get() ? Fixtures : None;
}

void UNPCPopulationSubsystem::DrawDebug(float Duration) const
{
	UWorld* World = GetWorld();
	for (int32 Node = 0; Node < Grid.NumNodes(); ++Node)
	{
		DrawDebugPoint(World, Grid.GetNodeLocation(Node) + FVector(0.0f, 0.0f, 3.0f), 4.0f, FColor::Green, false, Duration);
	}
	for (const TWeakObjectPtr<AAdastreaNPC>& NPC : Crew)
	{
		if (const AAdastreaNPCController* Brain = NPC.IsValid() ? Cast<AAdastreaNPCController>(NPC->GetController()) : nullptr)
		{
			if (Brain->GetActivity() == ENPCActivity::Walking)
			{
				DrawDebugLine(World, NPC->GetActorLocation(), Brain->GetDestination(), FColor::Yellow, false, Duration, 0, 2.0f);
			}
		}
	}
}

void UNPCPopulationSubsystem::DumpGrid() const
{
	ASpaceshipAvatar* Avatar = nullptr;
	GetPlayerInterior(Avatar);
	const float FloorZ = Avatar ? Grid.ToLocalZ(Avatar->GetActorLocation()) - 88.0f : 0.0f;
	Grid.DumpToLog(FloorZ);
}

// --- Console ---

namespace
{
	UNPCPopulationSubsystem* GetPopulation(UWorld* World)
	{
		return World ? World->GetSubsystem<UNPCPopulationSubsystem>() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GNPCSpawnCommand(
		TEXT("adastrea.NPC.Spawn"),
		TEXT("Spawn more crew in the interior the player is walking. Usage: adastrea.NPC.Spawn [Count=1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UNPCPopulationSubsystem* Population = GetPopulation(World))
			{
				const int32 Count = Args.IsEmpty() ? 1 : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 40);
				UE_LOG(LogAdastrea, Log, TEXT("adastrea.NPC.Spawn: spawned %d"), Population->SpawnCrew(Count));
			}
		}));

	FAutoConsoleCommandWithWorld GNPCClearCommand(
		TEXT("adastrea.NPC.Clear"),
		TEXT("Remove the crew from the interior the player is walking."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UNPCPopulationSubsystem* Population = GetPopulation(World))
			{
				Population->ClearCrew();
			}
		}));

	FAutoConsoleCommandWithWorld GNPCDumpGridCommand(
		TEXT("adastrea.NPC.DumpGrid"),
		TEXT("Log a top-down map of the current interior's walkable grid."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (const UNPCPopulationSubsystem* Population = GetPopulation(World))
			{
				Population->DumpGrid();
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs GNPCProbeCommand(
		TEXT("adastrea.NPC.ProbeColumn"),
		TEXT("Log every surface the walkable-grid bake finds at an interior-local point. Usage: adastrea.NPC.ProbeColumn X Y"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const UNPCPopulationSubsystem* Population = GetPopulation(World);
			if (Population && Args.Num() >= 2)
			{
				Population->ProbeColumn(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs GNPCDebugCommand(
		TEXT("adastrea.NPC.Debug"),
		TEXT("Draw the walkable grid and where each NPC is heading. Usage: adastrea.NPC.Debug [Seconds=10]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (const UNPCPopulationSubsystem* Population = GetPopulation(World))
			{
				Population->DrawDebug(Args.IsEmpty() ? 10.0f : FCString::Atof(*Args[0]));
			}
		}));
}
