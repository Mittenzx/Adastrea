#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatTestDirector.generated.h"

class ASpaceship;

/**
 * Runs the CombatTest level: sends waves of hostile fighters at the player. (A disabled
 * player is rescued by UPlayerRescueSubsystem: pod to the station, tow for the ship.)
 *
 * Waves come in ahead of the player at SpawnDistance; the next wave follows
 * WaveDelay seconds after the last hostile of the previous one is gone.
 * Console: HostileWaves 0/1 (pause/resume), SpawnHostiles [N], ClearHostiles.
 */
UCLASS()
class ADASTREA_API ACombatTestDirector : public AActor
{
	GENERATED_BODY()

public:
	ACombatTestDirector();

	/** Ship the hostiles fly (defaults to BP_Ship_Fighter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test")
	TSubclassOf<ASpaceship> HostileShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test")
	bool bAutoWaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test", meta=(ClampMin="1", ClampMax="12"))
	int32 WaveSize;

	/** Seconds after play starts before the first wave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test", meta=(ClampMin="0.0"))
	float FirstWaveDelay;

	/** Seconds between a wave being cleared and the next arriving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test", meta=(ClampMin="0.0"))
	float WaveDelay;

	/** How far from the player waves arrive (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat Test", meta=(ClampMin="1000.0"))
	float SpawnDistance;

	/** Spawn Count hostiles ahead of the player. Returns how many spawned. */
	static int32 SpawnHostilesNearPlayer(UWorld* World, int32 Count, float Distance, TSubclassOf<ASpaceship> ShipClass = nullptr);

	/** Remove every hostile ship and its pilot. */
	static int32 ClearHostiles(UWorld* World);

	static int32 CountHostiles(UWorld* World);

	/** The level's director, if it has one. */
	static ACombatTestDirector* Find(UWorld* World);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	float WaveClock;
	int32 WavesSent;
};
