#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerRescueSubsystem.generated.h"

class AEscapePod;
class APlayerController;
class ASpaceship;
class ASpaceStation;
class ATowDrone;

/**
 * What happens when the player's ship is disabled (it is never destroyed, see
 * ASpaceship::SetWrecked):
 *
 *  1. A tow drone launches from the nearest friendly station with a docking bay,
 *     clamps onto the wreck and hauls it to a point just outside the bay.
 *  2. The pilot ejects in an escape pod (the player possesses it and watches) that
 *     flies to that station, or to a friendly trader if one is closer.
 *  3. When the pod is taken aboard, the screen fades and the pilot wakes on foot in a
 *     medical bay (EStationRoom::Medical), checked over by the medics. Aboard a trader,
 *     that's the trader's sick bay until it has ferried them to the tow's station.
 *  4. The pilot waits in the station. When the tow arrives the ship is repaired free
 *     of charge and docked; the pilot boards it through the airlock as usual. Until
 *     then the airlock, trading and outfitting terminals say the ship isn't in yet.
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

	/**
	 * Why the pilot can't leave the room they're in yet, or empty if they can: still in
	 * flight aboard the trader that picked them up.
	 */
	FString GetTransitBlockReason() const;

	/** One-line status for logs and debug UI. */
	FString GetStatus() const;

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void StartRescue();
	void TickRescue(float DeltaTime);
	/** Fade out; the pilot wakes in the medical bay once the screen is black. */
	void BeginWake();
	void WakeInMedicalBay();
	/** The tow has delivered: repair the ship and dock it with nobody aboard. */
	void DockTowedShip();
	/** No on-foot interior available: put the pilot straight back in the repaired ship (the old flow). */
	void ReuniteInShip();
	void Message(const FString& Text, float Seconds, bool bWarning = false) const;
	void Reset();

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
	/** The pilot is at the tow's station (awake in its medical bay). */
	bool bPilotAtStation = false;
	/** Seconds since the wake fade-out began; negative when not fading. */
	float WakeClock = -1.0f;
	/** The pilot is awake on foot (in a station or a trader's medical bay). */
	bool bAwake = false;
	/** The towed ship has been repaired and sent in to dock. */
	bool bShipSentToDock = false;
	float DockRetryClock = 0.0f;
	float FallbackClock = -1.0f;
};
