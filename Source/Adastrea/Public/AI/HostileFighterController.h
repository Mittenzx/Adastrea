// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/CombatPilotController.h"
#include "HostileFighterController.generated.h"

/**
 * A hostile fighter pilot: makes attack runs (ACombatPilotController) on the nearest
 * of the player's ship and any patrol ship within EngageRange.
 *
 * Hostile ships are on team 1 (UShipHealthComponent::Team), so they don't shoot
 * each other. A disabled hostile's pilot bails out and the ship stays as a wreck.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AHostileFighterController : public ACombatPilotController
{
	GENERATED_BODY()

public:
	/** Whether a ship is flown by a hostile pilot. */
	static bool IsHostileShip(const AActor* Actor);

	/** Spawn a hostile fighter of ShipClass at Location, facing Facing, flown by a PilotClass (default: this class). */
	static ASpaceship* SpawnHostile(UWorld* World, TSubclassOf<ASpaceship> ShipClass, const FVector& Location, const FRotator& Facing,
		TSubclassOf<AHostileFighterController> PilotClass = nullptr);

protected:
	virtual ASpaceship* FindTarget() const override;
	virtual int32 GetCombatTeam() const override;
};
