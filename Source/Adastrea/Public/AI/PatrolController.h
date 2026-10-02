// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/CombatPilotController.h"
#include "PatrolController.generated.h"

class ASpaceStation;

/**
 * A patrol pilot (roster role Patrol). Flies between the sector's stations without
 * trading; attacks any hostile it sees within SensorRange; and answers distress calls
 * handed out by UDistressSubsystem: it undocks if need be, flies to the call and
 * fights the hostiles there until the call closes, then goes back to patrolling.
 *
 * Patrols are Civil (CombatTeam), like the player, traders and station turrets.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API APatrolController : public ACombatPilotController
{
	GENERATED_BODY()

public:
	APatrolController();

	virtual FString GetObjectiveDescription() const override;

	/** Hostiles within this distance of the patrol (cm) are attacked without a distress call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Patrol", meta=(ClampMin="1000.0"))
	float SensorRange = 25000.0f;

	/** Fly to distress call CallId and fight there until it closes. */
	void AnswerCall(int32 CallId);

	/** The distress call being answered, or INDEX_NONE. */
	int32 GetCallId() const { return CallId; }

	/** Whether a ship is flown by a patrol pilot. */
	static bool IsPatrolShip(const AActor* Actor);

	/** Spawn a patrol ship of ShipClass near Station (or near the origin) and start it patrolling. */
	static ASpaceship* SpawnPatrol(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station);

protected:
	virtual void TickPilot(float DeltaSeconds) override;
	virtual void TickNoTarget(float DeltaSeconds) override;
	virtual ASpaceship* FindTarget() const override;
	virtual int32 GetCombatTeam() const override;

private:
	/** Follow the call's location; drop it once the subsystem has closed it. */
	void RefreshCall();

	int32 CallId = INDEX_NONE;
	FVector CallLocation = FVector::ZeroVector;
};
