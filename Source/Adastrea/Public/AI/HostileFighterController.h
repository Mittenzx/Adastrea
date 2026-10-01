// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIPilotController.h"
#include "HostileFighterController.generated.h"

UENUM(BlueprintType)
enum class EHostileFighterState : uint8
{
	/** No target: drift and wait. */
	Idle,
	/** Nose on the lead point, closing and firing in bursts. */
	Attack,
	/** Peel off past the target, then swing round for another run. */
	BreakOff
};

/**
 * A hostile fighter pilot: hunts the player's ship and shoots it with the same forward
 * guns the player has (UShipWeaponComponent), flying through the same throttle/turn
 * API as the AI traders.
 *
 * Attack runs: turn onto the lead point and close at full throttle, firing in bursts
 * whenever the lead point is inside FireConeDegrees. Too close, or after a long run,
 * it breaks off to a point beside and past the target, then comes round again.
 *
 * Hostile ships are on team 1 (UShipHealthComponent::Team), so they don't shoot
 * each other. A disabled hostile's pilot bails out and the ship stays as a wreck.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AHostileFighterController : public AAIPilotController
{
	GENERATED_BODY()

public:
	AHostileFighterController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual FString GetObjectiveDescription() const override;

	/** Fire when the lead point is within this angle of the nose (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.5"))
	float FireConeDegrees = 6.0f;

	/** Break off when closer than this (cm), measured centre to centre. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="100.0"))
	float BreakOffDistance = 2500.0f;

	/** Longest attack run before breaking off anyway (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="1.0"))
	float MaxAttackSeconds = 8.0f;

	/** How long a break-off lasts (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.5"))
	float BreakOffSeconds = 3.0f;

	/** Trigger held per burst, then released for BurstPause (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.1"))
	float BurstSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.0"))
	float BurstPause = 0.8f;

	/** Gun damage relative to the player's guns, so a fight lasts more than a few seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.0"))
	float DamageScale = 0.35f;

	/** Gun scatter (degrees): AI aim is worse than a pilot's. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="0.0"))
	float GunSpreadDegrees = 1.2f;

	/** Engage the player only within this distance (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hostile", meta=(ClampMin="1000.0"))
	float EngageRange = 60000.0f;

	UPROPERTY(BlueprintReadOnly, Category="Hostile")
	EHostileFighterState CombatState = EHostileFighterState::Idle;

	/** Whether a ship is flown by a hostile pilot. */
	static bool IsHostileShip(const AActor* Actor);

	/** Spawn a hostile fighter of ShipClass at Location, facing Facing, and start it hunting. */
	static ASpaceship* SpawnHostile(UWorld* World, TSubclassOf<ASpaceship> ShipClass, const FVector& Location, const FRotator& Facing);

protected:
	virtual void TickPilot(float DeltaSeconds) override;
	virtual void OnShipWrecked() override;

private:
	/** The player's ship if it can be attacked (flown, not docked, not destroyed), else null. */
	ASpaceship* FindTarget() const;
	void SetFiring(bool bFire);
	void BeginBreakOff(const ASpaceship* Target);

	TWeakObjectPtr<ASpaceship> Target;
	FVector BreakPoint = FVector::ZeroVector;
	float StateSeconds = 0.0f;
	float BurstClock = 0.0f;
};
