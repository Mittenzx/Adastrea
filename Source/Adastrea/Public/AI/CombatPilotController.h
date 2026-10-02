// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIPilotController.h"
#include "CombatPilotController.generated.h"

UENUM(BlueprintType)
enum class ECombatPilotState : uint8
{
	/** No target: drift and wait (subclasses may do something else instead). */
	Idle,
	/** Nose on the lead point, closing and firing in bursts. */
	Attack,
	/** Peel off past the target, then swing round for another run. */
	BreakOff
};

/**
 * A fighter pilot that makes attack runs on whatever FindTarget picks, with the same
 * forward guns the player has (UShipWeaponComponent), flying through the same
 * throttle/turn API as the AI traders.
 *
 * Attack runs: turn onto the lead point and close at full throttle, firing in bursts
 * whenever the lead point is inside FireConeDegrees. Too close, or after a long run,
 * it breaks off to a point beside and past the target, then comes round again.
 *
 * Subclasses choose the side and the target: AHostileFighterController hunts the
 * player and patrols; APatrolController answers distress calls and hunts hostiles.
 */
UCLASS(Abstract)
class ADASTREA_API ACombatPilotController : public AAIPilotController
{
	GENERATED_BODY()

public:
	ACombatPilotController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual FString GetObjectiveDescription() const override;

	/** Fire when the lead point is within this angle of the nose (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.5"))
	float FireConeDegrees = 6.0f;

	/** Break off when closer than this (cm), measured centre to centre. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="100.0"))
	float BreakOffDistance = 2500.0f;

	/** Longest attack run before breaking off anyway (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="1.0"))
	float MaxAttackSeconds = 8.0f;

	/** How long a break-off lasts (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.5"))
	float BreakOffSeconds = 3.0f;

	/** Trigger held per burst, then released for BurstPause (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.1"))
	float BurstSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.0"))
	float BurstPause = 0.8f;

	/** Gun damage relative to the player's guns, so a fight lasts more than a few seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.0"))
	float DamageScale = 0.35f;

	/** Gun scatter (degrees): AI aim is worse than a pilot's. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="0.0"))
	float GunSpreadDegrees = 1.2f;

	/** Engage targets only within this distance (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Pilot", meta=(ClampMin="1000.0"))
	float EngageRange = 60000.0f;

	UPROPERTY(BlueprintReadOnly, Category="Combat Pilot")
	ECombatPilotState CombatState = ECombatPilotState::Idle;

	/** The ship being attacked, if any. */
	ASpaceship* GetCombatTarget() const { return Target.Get(); }

protected:
	virtual void TickPilot(float DeltaSeconds) override;
	virtual void OnShipWrecked() override;

	/** The ship to attack this tick, or null. */
	virtual ASpaceship* FindTarget() const PURE_VIRTUAL(ACombatPilotController::FindTarget, return nullptr;);

	/** CombatTeam the ship fights on, set when the pilot takes it. */
	virtual int32 GetCombatTeam() const PURE_VIRTUAL(ACombatPilotController::GetCombatTeam, return 0;);

	/** Nothing to attack this tick. Default: drift at low throttle. */
	virtual void TickNoTarget(float DeltaSeconds);

	/** Whether Ship can be attacked at all: flown, not wrecked, docked or hidden. */
	static bool IsAttackable(const ASpaceship* Ship);

	/** Of Candidates, the one nearest to the ship within EngageRange, keeping the current target unless another is much nearer. */
	ASpaceship* PickNearest(const TArray<ASpaceship*>& Candidates) const;

	void SetFiring(bool bFire);

private:
	void BeginBreakOff(const ASpaceship* Victim);

	TWeakObjectPtr<ASpaceship> Target;
	FVector BreakPoint = FVector::ZeroVector;
	float StateSeconds = 0.0f;
	float BurstClock = 0.0f;
};
