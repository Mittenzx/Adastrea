// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIShipPopulator.generated.h"

class ASpaceship;

/**
 * Drop one in a level to fill it with AI-piloted ships at BeginPlay.
 * Ships are spread across the level's stations and each runs an AAIPilotController.
 *
 * When the level's galaxy sector has ships in the organisation roster
 * (Organisations.json), those ships are spawned, each with a UOwnershipComponent
 * naming its owner; ShipClasses / MinerShipClasses supply the Blueprint for
 * records that don't name one. Otherwise ShipCount traders and MinerCount
 * miners are spawned, unowned.
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

	/** Spawn this sector's roster ships when it has any (ShipCount/MinerCount are then ignored). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Population")
	bool bUseOrganisationRoster = true;

	/** Spawn the sector's roster ships, else ShipCount traders and MinerCount miners. Returns how many were created. */
	UFUNCTION(BlueprintCallable, Category="Population")
	int32 Populate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Spawn roster ships of the level's sector that aren't already flying. Returns -1 if the sector has no roster ships. */
	int32 SpawnRosterShips();

	/** Ships bought into this sector (or restored by a load) show up without a level reload. */
	void HandleRosterChanged();

	FDelegateHandle RosterChangedHandle;
};
