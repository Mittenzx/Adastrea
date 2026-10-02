#pragma once

#include "CoreMinimal.h"
#include "AI/HostileFighterController.h"
#include "RaiderController.generated.h"

/**
 * A pirate raider: a hostile fighter flying for a gang in a raid (URaidSubsystem).
 * It doesn't hunt on its own. It flies in on the raid's target, holds off it while
 * the gang hails, attacks it only if the demand is refused, and fights back against
 * whoever shoots at it. When its own shields fail it breaks off and runs for home,
 * leaving the level once clear of the fight.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ARaiderController : public AHostileFighterController
{
	GENERATED_BODY()

public:
	ARaiderController();

	virtual FString GetObjectiveDescription() const override;

	/** Join raid RaidId for GangId. */
	void JoinRaid(int32 InRaidId, FName InGangId);

	/** Run for home now (shields failed, or the raid is over). */
	void Flee(const TCHAR* Why);

	int32 GetRaidId() const { return RaidId; }
	FName GetGangId() const { return GangId; }
	bool IsFleeing() const { return bFleeing; }

	/** Someone shot this raider: fight back for a while. */
	void NotifyAttackedBy(AActor* Attacker);

	/** Leave the level once this far from the target and the player (cm)... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Raider", meta=(ClampMin="1000.0"))
	float LeaveDistance = 300000.0f;

	/** ...or after running this long (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Raider", meta=(ClampMin="1.0"))
	float MaxFleeSeconds = 45.0f;

protected:
	virtual void TickPilot(float DeltaSeconds) override;
	virtual void TickNoTarget(float DeltaSeconds) override;
	virtual ASpaceship* FindTarget() const override;
	virtual void OnShipWrecked() override;

private:
	void TickFlee(float DeltaSeconds);
	/** Hold HoldDistance off the target, nose on it. */
	void TickHold(const ASpaceship* RaidTarget, float DeltaSeconds);
	void LeaveLevel();

	int32 RaidId = INDEX_NONE;
	FName GangId;
	bool bFleeing = false;
	float FleeSeconds = 0.0f;
	FVector FleeDirection = FVector::ForwardVector;
	TWeakObjectPtr<AActor> Aggressor;
	double AggressorTime = -1000.0;
};
