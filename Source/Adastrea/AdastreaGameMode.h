// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdastreaGameMode.generated.h"

// Forward declarations
class ASpaceship;

/**
 * Base game mode for Adastrea
 * 
 * This is the primary game mode that controls gameplay rules and state.
 * 
 * Features:
 * - Automatic player spaceship spawning when SpaceSectorMap is present
 * - Configurable spawn behavior (center vs random position)
 * - Blueprint-friendly customization
 * 
 * Usage:
 * 1. Set as the GameMode Override in World Settings
 * 2. Configure DefaultSpaceshipClass to the desired spaceship Blueprint
 * 3. Toggle bAutoSpawnPlayerShip to enable/disable automatic spawning
 * 4. Choose spawn location type with bSpawnAtCenter
 */
UCLASS(minimalapi)
class AAdastreaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAdastreaGameMode();

	/**
	 * Default spaceship class to spawn for the player
	 * Set this to your spaceship Blueprint in editor or via derived Blueprint
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	TSubclassOf<ASpaceship> DefaultSpaceshipClass;

	/**
	 * Whether to automatically spawn a player spaceship on level start
	 * Only works if a SpaceSectorMap actor exists in the level
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	bool bAutoSpawnPlayerShip;

	/**
	 * If true, spawn at sector center. If false, spawn at random position in sector
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	bool bSpawnAtCenter;

protected:
	virtual void BeginPlay() override;

	/**
	 * Spawns the player spaceship in the sector and possesses it
	 * Called automatically from BeginPlay if bAutoSpawnPlayerShip is true
	 */
	UFUNCTION(BlueprintCallable, Category="Player Spawn")
	virtual void SpawnPlayerSpaceship();
};
