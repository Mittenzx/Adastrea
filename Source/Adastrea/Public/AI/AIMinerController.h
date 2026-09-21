// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIPilotController.h"
#include "AIMinerController.generated.h"

class AAsteroid;

/** What the AI miner is currently doing. */
UENUM(BlueprintType)
enum class EAIMinerState : uint8
{
	/** Find the nearest minable asteroid. */
	SeekingAsteroid,
	/** Flying out to laser range of the chosen asteroid. */
	ToAsteroid,
	/** Facing the asteroid with the mining laser firing. */
	Mining,
	/** Cargo full: flying to the station that pays best for the ore. */
	ToStation,
	/** Docked: selling the ore, then back out to the rocks. */
	Docked
};

/**
 * AI miner: fly to the nearest asteroid, mine it with the ship's UMiningLaserComponent
 * until the hold is full, sell everything at the station that pays the most for the
 * ore, then head back out. Reuses AAIPilotController's steering and docking.
 *
 * The laser does the actual work (targeting, range and aim checks, moving ore into
 * the UCargoComponent); this controller only positions and aims the ship. The laser
 * is force-enabled on possession, so any ship class can be used as a miner.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AAIMinerController : public AAIPilotController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

	/** Fraction of the laser's range to park at (the laser stops working beyond its range). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Miner", meta=(ClampMin="0.1", ClampMax="0.95"))
	float StandOffFraction = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="AI Miner")
	EAIMinerState MinerState = EAIMinerState::SeekingAsteroid;

	/** The asteroid currently being flown to or mined. */
	UPROPERTY(BlueprintReadOnly, Category="AI Miner")
	TObjectPtr<AAsteroid> TargetAsteroid;

	/** Spawn an AI miner of ShipClass near Station and start it working. */
	static ASpaceship* SpawnAIMiner(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station);

protected:
	virtual void TickPilot(float DeltaSeconds) override;

private:
	void TickSeeking();
	void TickToAsteroid(float DeltaSeconds);
	void TickMining(float DeltaSeconds);
	void TickToStation(float DeltaSeconds);
	void TickMinerDocked(float DeltaSeconds);

	bool IsMinable(const AAsteroid* Rock) const;

	/** Distance from the laser muzzle to the rock's surface (cm). */
	float SurfaceDistance(const AAsteroid* Rock) const;

	/** Stop firing, drop the lock, and go back to looking for a rock. */
	void AbandonAsteroid();

	/** Pick the station that pays the most for what's in the hold. */
	void ChooseSellStation();

	void SellAllCargo();
};
