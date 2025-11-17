// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaGameMode.h"
#include "SpaceSectorMap.h"
#include "Ships/Spaceship.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AAdastreaGameMode::AAdastreaGameMode()
{
	// Set default pawn class to our Blueprinted character
	// This would typically be set to a blueprint class in the editor
	
	// Default auto-spawn settings
	bAutoSpawnPlayerShip = true;
	bSpawnAtCenter = false; // Use random position by default for variety
	DefaultSpaceshipClass = nullptr; // Must be set in Blueprint or editor
}

void AAdastreaGameMode::BeginPlay()
{
	Super::BeginPlay();

           FAdastreaDirectorModule & Module = FModuleManager::LoadModuleChecked<FAdastreaDirectorModule>("AdastreaDirector");
            FPythonBridge* Bridge = Module.GetPythonBridge();

    if (Bridge)
    {
        UE_LOG(LogTemp, Log, TEXT("Python bridge found"));
        UE_LOG(LogTemp, Log, TEXT("Is Ready: %s"), Bridge->IsReady() ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogTemp, Log, TEXT("Process ID: %d"), Bridge->GetProcessId());
    }
    Make sure your C++ is set up so Bridge->IsReady(), Bridge->GetProcessId() can be called.
        Blueprint:

    You’ll typically need a wrapper function, or expose this logic via blueprint callable nodes.


	// Automatically spawn player spaceship if enabled and configured
	if (bAutoSpawnPlayerShip)
	{
		SpawnPlayerSpaceship();
	}
}

void AAdastreaGameMode::SpawnPlayerSpaceship()
{
	// Validate we have a spaceship class to spawn
	if (!DefaultSpaceshipClass)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: Cannot auto-spawn player spaceship - DefaultSpaceshipClass is not set"));
		return;
	}

	// Find the SpaceSectorMap actor in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceSectorMap::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: No SpaceSectorMap found in level - cannot auto-spawn player spaceship"));
		return;
	}

	// Use the first sector map found
	if (FoundActors.Num() > 1)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: Multiple SpaceSectorMap actors found (%d), using the first one"), FoundActors.Num());
	}

	ASpaceSectorMap* SectorMap = Cast<ASpaceSectorMap>(FoundActors[0]);
	if (!SectorMap)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameMode: Failed to cast actor to SpaceSectorMap"));
		return;
	}

	// Get spawn location based on configuration
	FVector SpawnLocation;
	if (bSpawnAtCenter)
	{
		SpawnLocation = SectorMap->GetSectorCenter();
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Spawning player spaceship at sector center: %s"), *SpawnLocation.ToString());
	}
	else
	{
		SpawnLocation = SectorMap->GetRandomPositionInSector();
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Spawning player spaceship at random position: %s"), *SpawnLocation.ToString());
	}

	// Set spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Name = FName(TEXT("PlayerSpaceship"));

	// Spawn the spaceship
	FRotator SpawnRotation = FRotator::ZeroRotator; // Facing forward
	ASpaceship* PlayerShip = GetWorld()->SpawnActor<ASpaceship>(DefaultSpaceshipClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!PlayerShip)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameMode: Failed to spawn player spaceship"));
		return;
	}

	UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Successfully spawned player spaceship at %s"), *SpawnLocation.ToString());

	// Get the player controller and possess the spaceship
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->Possess(PlayerShip);
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameMode: Player controller possessed spaceship"));
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaGameMode: No player controller found to possess spaceship"));
	}
}
