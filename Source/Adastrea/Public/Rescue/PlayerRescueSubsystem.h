#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerRescueSubsystem.generated.h"

class AEscapePod;
class APlayerController;
class ASpaceship;
class ASpaceStation;
class ATowDrone;
class ACameraActor;

/**
 * What happens when the player's ship is disabled (it is never destroyed, see
 * ASpaceship::SetWrecked):
 *
 *  1. A tow drone launches from the nearest friendly station with a docking bay,
 *     clamps onto the wreck and hauls it to a point just outside the bay.
 *  2. The pilot ejects in an escape pod (the player possesses it and watches) that
 *     flies to that station, or to a friendly trader if one is closer. A trader that
 *     takes the pod aboard ferries the pilot to the tow's station.
 *  3. Once the pilot is at the station, the camera watches the tow come in. When both
 *     are there, the ship is repaired free of charge, the pilot re-boards it, and it
 *     docks through the normal docking flow.
 *
 * With no friendly station anywhere, the ship is repaired where it lies after
 * FallbackRepairSeconds.
 */
UCLASS()
class ADASTREA_API UPlayerRescueSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UPlayerRescueSubsystem* Get(const UObject* WorldContext);

	/** The player's ship was just disabled; the rescue starts next tick. */
	void BeginRescue(ASpaceship* Wreck, APlayerController* PC);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Rescue")
	bool IsRescueUnderway() const { return Wreck.IsValid(); }

	/** One-line status for logs and debug UI. */
	FString GetStatus() const;

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void StartRescue();
	void TickRescue(float DeltaTime);
	void Reunite();
	void Message(const FString& Text, float Seconds, bool bWarning = false) const;
	void Reset();

	/** Where the camera watching the tow should be: beside the wreck, looking past it at the station. */
	static FTransform GetTowShot(const ASpaceship* Ship, const AActor* Bay);

	/** Nearest station with a docking bay to Location, or null. */
	ASpaceStation* FindNearestFriendlyStation(const FVector& Location) const;
	/** Nearest trader in flight (a civil AI pilot that can ferry) to Location within MaxDistance, or null. */
	ASpaceship* FindPickupShip(const FVector& Location, float MaxDistance) const;

	TWeakObjectPtr<ASpaceship> Wreck;
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<ASpaceStation> Station;
	TWeakObjectPtr<ATowDrone> Tow;
	TWeakObjectPtr<AEscapePod> Pod;
	TWeakObjectPtr<ASpaceship> PickupShip;

	bool bPending = false;
	bool bFerryOrdered = false;
	bool bPilotAtStation = false;
	/** The camera has switched to watching the tow come in. */
	bool bWatchingTow = false;
	float FallbackClock = -1.0f;

	/** Camera that watches the tow come in once the pilot is at the station. */
	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> WatchCamera;
};
