// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIShipPopulator.h"
#include "AI/AIMinerController.h"
#include "AI/AIPilotController.h"
#include "AI/PatrolController.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Mining/Asteroid.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Universe/GalaxySubsystem.h"
#include "Universe/OrganisationSubsystem.h"
#include "Universe/OwnershipComponent.h"

namespace
{
	TArray<TSubclassOf<ASpaceship>> ValidClasses(const TArray<TSubclassOf<ASpaceship>>& Classes)
	{
		TArray<TSubclassOf<ASpaceship>> Valid;
		for (const TSubclassOf<ASpaceship>& ShipClass : Classes)
		{
			if (ShipClass)
			{
				Valid.Add(ShipClass);
			}
		}
		return Valid;
	}

	TArray<ASpaceStation*> GatherStations(UWorld* World)
	{
		TArray<ASpaceStation*> Stations;
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			Stations.Add(*It);
		}
		return Stations;
	}

	int32 SpawnShips(UWorld* World, const TArray<TSubclassOf<ASpaceship>>& ShipClasses, int32 Count, TSubclassOf<AAIPilotController> PilotClass)
	{
		if (!World || Count <= 0 || ShipClasses.IsEmpty())
		{
			return 0;
		}

		const TArray<ASpaceStation*> Stations = GatherStations(World);
		if (Stations.IsEmpty())
		{
			UE_LOG(LogAdastreaShips, Warning, TEXT("AI populate: no stations in the level, nothing to fly between"));
			return 0;
		}

		int32 Spawned = 0;
		for (int32 i = 0; i < Count; ++i)
		{
			const TSubclassOf<ASpaceship>& ShipClass = ShipClasses[FMath::RandRange(0, ShipClasses.Num() - 1)];
			ASpaceStation* Home = Stations[i % Stations.Num()];
			if (AAIPilotController::SpawnPilotedShip(World, ShipClass, Home, PilotClass))
			{
				++Spawned;
			}
		}
		UE_LOG(LogAdastreaShips, Log, TEXT("AI populate: spawned %d/%d %s ships across %d stations"),
			Spawned, Count, *PilotClass->GetName(), Stations.Num());
		return Spawned;
	}

	void SpawnFromPlayerShip(const TArray<FString>& Args, UWorld* World, TSubclassOf<AAIPilotController> PilotClass, int32 DefaultCount, const TCHAR* CommandName)
	{
		const ASpaceship* PlayerShip = World ? Cast<ASpaceship>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
		if (!PlayerShip)
		{
			UE_LOG(LogAdastreaShips, Warning, TEXT("%s: fly a ship first (its class is used for the AI ships)"), CommandName);
			return;
		}
		const int32 Count = Args.IsEmpty() ? DefaultCount : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 100);
		SpawnShips(World, { PlayerShip->GetClass() }, Count, PilotClass);
	}

	FAutoConsoleCommandWithWorldAndArgs GSpawnAITradersCommand(
		TEXT("adastrea.SpawnAITraders"),
		TEXT("Spawn AI-piloted trader ships (copies of the player's ship). Usage: adastrea.SpawnAITraders [Count=3]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			SpawnFromPlayerShip(Args, World, AAIPilotController::StaticClass(), 3, TEXT("adastrea.SpawnAITraders"));
		}));

	FAutoConsoleCommandWithWorldAndArgs GSpawnAIMinersCommand(
		TEXT("adastrea.SpawnAIMiners"),
		TEXT("Spawn AI miners (copies of the player's ship) that mine asteroids in the level and sell at stations. Usage: adastrea.SpawnAIMiners [Count=2]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (World && !TActorIterator<AAsteroid>(World))
			{
				UE_LOG(LogAdastreaShips, Warning, TEXT("adastrea.SpawnAIMiners: no asteroids in this level yet; miners will idle until some exist"));
			}
			SpawnFromPlayerShip(Args, World, AAIMinerController::StaticClass(), 2, TEXT("adastrea.SpawnAIMiners"));
		}));
}

AAIShipPopulator::AAIShipPopulator()
{
	PrimaryActorTick.bCanEverTick = false;
	SetCanBeDamaged(false);
}

void AAIShipPopulator::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		Populate();
	}
	if (UOrganisationSubsystem* Orgs = bUseOrganisationRoster ? UOrganisationSubsystem::Get(this) : nullptr)
	{
		RosterChangedHandle = Orgs->OnRosterChanged.AddUObject(this, &AAIShipPopulator::HandleRosterChanged);
	}
}

void AAIShipPopulator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this))
	{
		Orgs->OnRosterChanged.Remove(RosterChangedHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AAIShipPopulator::HandleRosterChanged()
{
	SpawnRosterShips();
}

int32 AAIShipPopulator::Populate()
{
	if (bUseOrganisationRoster)
	{
		const int32 Spawned = SpawnRosterShips();
		if (Spawned >= 0)
		{
			return Spawned;
		}
	}

	const TArray<TSubclassOf<ASpaceship>> TraderClasses = ValidClasses(ShipClasses);
	TArray<TSubclassOf<ASpaceship>> MinerClasses = ValidClasses(MinerShipClasses);
	if (MinerClasses.IsEmpty())
	{
		MinerClasses = TraderClasses;
	}

	return SpawnShips(GetWorld(), TraderClasses, ShipCount, AAIPilotController::StaticClass())
		+ SpawnShips(GetWorld(), MinerClasses, MinerCount, AAIMinerController::StaticClass());
}

int32 AAIShipPopulator::SpawnRosterShips()
{
	UWorld* World = GetWorld();
	UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	const FName SectorId = Galaxy ? Galaxy->ResolveCurrentSectorId(this) : NAME_None;
	if (!World || !Orgs || SectorId.IsNone())
	{
		return -1;
	}
	const TArray<FShipRecord*> Roster = Orgs->GetShipsInSector(SectorId);
	if (Roster.IsEmpty())
	{
		return -1;
	}

	const TArray<ASpaceStation*> Stations = GatherStations(World);
	if (Stations.IsEmpty())
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("AI populate: sector '%s' has %d roster ships but no stations to fly between"),
			*SectorId.ToString(), Roster.Num());
		return 0;
	}

	const TArray<TSubclassOf<ASpaceship>> TraderClasses = ValidClasses(ShipClasses);
	TArray<TSubclassOf<ASpaceship>> MinerClasses = ValidClasses(MinerShipClasses);
	if (MinerClasses.IsEmpty())
	{
		MinerClasses = TraderClasses;
	}

	int32 Spawned = 0;
	int32 Skipped = 0;
	for (int32 i = 0; i < Roster.Num(); ++i)
	{
		FShipRecord& Record = *Roster[i];
		if (Record.SpawnedShip.IsValid())
		{
			continue;
		}

		TSubclassOf<ASpaceship> ShipClass = Record.ShipClass.IsNull() ? nullptr : Record.ShipClass.LoadSynchronous();
		if (!ShipClass)
		{
			const TArray<TSubclassOf<ASpaceship>>& Pool = Record.Role == EShipRole::Miner ? MinerClasses : TraderClasses;
			ShipClass = Pool.IsEmpty() ? nullptr : Pool[FMath::RandRange(0, Pool.Num() - 1)];
		}
		if (!ShipClass)
		{
			++Skipped;
			continue;
		}

		TSubclassOf<AAIPilotController> PilotClass = AAIPilotController::StaticClass();
		if (Record.Role == EShipRole::Miner)
		{
			PilotClass = AAIMinerController::StaticClass();
		}
		else if (Record.Role == EShipRole::Patrol)
		{
			PilotClass = APatrolController::StaticClass();
		}
		ASpaceship* Ship = AAIPilotController::SpawnPilotedShip(World, ShipClass, Stations[i % Stations.Num()], PilotClass);
		if (!Ship)
		{
			++Skipped;
			continue;
		}

		UOwnershipComponent* Ownership = NewObject<UOwnershipComponent>(Ship, TEXT("Ownership"));
		Ownership->OwnerId = Record.OwnerId;
		Ownership->ShipRecordId = Record.Id;
		Ownership->RecordName = Record.Name;
		Ownership->RegisterComponent();
		Ship->AddInstanceComponent(Ownership);

		Orgs->BindSpawnedShip(Record.Id, Ship);
		++Spawned;
	}

	// Quiet when a roster change (a purchase elsewhere) had nothing new for this sector.
	if (Spawned > 0 || Skipped > 0)
	{
		UE_LOG(LogAdastreaShips, Log, TEXT("AI populate: spawned %d roster ships for sector '%s' (%d in roster%s)"),
			Spawned, *SectorId.ToString(), Roster.Num(),
			Skipped > 0 ? *FString::Printf(TEXT(", %d skipped: no ship class or spawn failed"), Skipped) : TEXT(""));
	}
	return Spawned;
}
