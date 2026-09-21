// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIShipPopulator.h"
#include "AI/AIMinerController.h"
#include "AI/AIPilotController.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Mining/Asteroid.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"

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

	int32 SpawnShips(UWorld* World, const TArray<TSubclassOf<ASpaceship>>& ShipClasses, int32 Count, TSubclassOf<AAIPilotController> PilotClass)
	{
		if (!World || Count <= 0 || ShipClasses.IsEmpty())
		{
			return 0;
		}

		TArray<ASpaceStation*> Stations;
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			Stations.Add(*It);
		}
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
}

int32 AAIShipPopulator::Populate()
{
	const TArray<TSubclassOf<ASpaceship>> TraderClasses = ValidClasses(ShipClasses);
	TArray<TSubclassOf<ASpaceship>> MinerClasses = ValidClasses(MinerShipClasses);
	if (MinerClasses.IsEmpty())
	{
		MinerClasses = TraderClasses;
	}

	return SpawnShips(GetWorld(), TraderClasses, ShipCount, AAIPilotController::StaticClass())
		+ SpawnShips(GetWorld(), MinerClasses, MinerCount, AAIMinerController::StaticClass());
}
