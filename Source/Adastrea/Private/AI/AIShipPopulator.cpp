// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIShipPopulator.h"
#include "AI/AIPilotController.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"

namespace
{
	int32 SpawnTraders(UWorld* World, const TArray<TSubclassOf<ASpaceship>>& ShipClasses, int32 Count)
	{
		if (!World || ShipClasses.IsEmpty())
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
			if (AAIPilotController::SpawnAITrader(World, ShipClass, Home))
			{
				++Spawned;
			}
		}
		UE_LOG(LogAdastreaShips, Log, TEXT("AI populate: spawned %d/%d ships across %d stations"), Spawned, Count, Stations.Num());
		return Spawned;
	}

	FAutoConsoleCommandWithWorldAndArgs GSpawnAITradersCommand(
		TEXT("adastrea.SpawnAITraders"),
		TEXT("Spawn AI-piloted trader ships (copies of the player's ship). Usage: adastrea.SpawnAITraders [Count=3]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const ASpaceship* PlayerShip = World ? Cast<ASpaceship>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
			if (!PlayerShip)
			{
				UE_LOG(LogAdastreaShips, Warning, TEXT("adastrea.SpawnAITraders: fly a ship first (its class is used for the AI ships)"));
				return;
			}
			const int32 Count = Args.IsEmpty() ? 3 : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 100);
			SpawnTraders(World, { PlayerShip->GetClass() }, Count);
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
	TArray<TSubclassOf<ASpaceship>> ValidClasses;
	for (const TSubclassOf<ASpaceship>& ShipClass : ShipClasses)
	{
		if (ShipClass)
		{
			ValidClasses.Add(ShipClass);
		}
	}
	return SpawnTraders(GetWorld(), ValidClasses, ShipCount);
}
