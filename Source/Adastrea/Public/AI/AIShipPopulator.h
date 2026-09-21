// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIShipPopulator.generated.h"

class ASpaceship;

/**
 * Drop one in a level to fill it with AI-piloted trader ships at BeginPlay.
 * Ships are spread across the level's stations and each runs an AAIPilotController.
 *
 * For quick testing without placing anything, use the console command
 * `adastrea.SpawnAITraders [Count]` (uses the player's ship class).
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AAIShipPopulator : public AActor
{
	GENERATED_BODY()

public:
	AAIShipPopulator();

	/** Ship Blueprints to pick from at random (e.g. BP_Ship_Freighter, BP_Ship_Fighter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population")
	TArray<TSubclassOf<ASpaceship>> ShipClasses;

	/** How many AI ships to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population", meta=(ClampMin="0", ClampMax="100"))
	int32 ShipCount = 5;

	/** Ship Blueprints for AI miners (falls back to ShipClasses when empty). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population")
	TArray<TSubclassOf<ASpaceship>> MinerShipClasses;

	/** How many AI miners to spawn (they mine any asteroids already in the level). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population", meta=(ClampMin="0", ClampMax="100"))
	int32 MinerCount = 0;

	/** Spawn automatically when play begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population")
	bool bSpawnOnBeginPlay = true;

	/** Spawn ShipCount traders and MinerCount miners now. Returns how many were created. */
	UFUNCTION(BlueprintCallable, Category="Population")
	int32 Populate();

protected:
	virtual void BeginPlay() override;
};
