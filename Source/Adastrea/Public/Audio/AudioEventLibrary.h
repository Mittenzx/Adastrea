// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AudioEventLibrary.generated.h"

class AActor;
class UAudioComponent;
class USceneComponent;
class USpaceshipDataAsset;

/**
 * Gameplay-side front door for event sounds (flight, docking, mining, trading,
 * Station Editor, interiors, UI).
 *
 * Every call ends in UAudioCatalogSubsystem with a dotted event ID, so gameplay
 * code never names a sound asset. This layer adds what the catalog doesn't:
 *
 *  - Rate limits: MinInterval drops a repeat of the same ID fired too recently.
 *  - Secondary events (toasts, credit dings, menu open/close/hover) are deferred
 *    one frame and dropped if a primary event fired within SecondaryYieldSeconds
 *    either side. So docking plays the clamp, not the clamp plus the menu-open
 *    blip, and a sale plays Trade.Sell, not Trade.Sell plus a credits ding.
 *  - FScopedMute: silences everything fired inside it (used while a save loads,
 *    which re-docks the ship and rewrites the wallet).
 *  - adastrea.AudioEventLog 1 logs each event as it actually plays, with
 *    whether the catalog knows the ID. That log is the evidence for PIE checks.
 */
UCLASS()
class ADASTREA_API UAudioEventLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Window (seconds) in which a primary event suppresses secondary ones. */
	static constexpr double SecondaryYieldSeconds = 0.35;

	/** Non-spatial one-shot (player feedback). Returns true if it was sent to the catalog. */
	UFUNCTION(BlueprintCallable, Category="Audio|Events", meta=(WorldContext="WorldContext"))
	static bool PlayEvent2D(const UObject* WorldContext, FName EventId, float MinInterval = 0.0f);

	/** Spatial one-shot at a world location (the catalog entry carries the attenuation). */
	UFUNCTION(BlueprintCallable, Category="Audio|Events", meta=(WorldContext="WorldContext"))
	static bool PlayEventAtLocation(const UObject* WorldContext, FName EventId, FVector Location, float MinInterval = 0.0f);

	/**
	 * Low-priority 2D one-shot: plays next frame unless a primary event fired within
	 * SecondaryYieldSeconds (before or after). For toasts, credit dings, menu open/close/hover.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Events", meta=(WorldContext="WorldContext"))
	static void PlaySecondary2D(const UObject* WorldContext, FName EventId, float MinInterval = 0.0f);

	/** A looping or long-lived event on a component. Null Parent = 2D. Caller owns and stops it. */
	static UAudioComponent* SpawnEventAttached(const UObject* WorldContext, FName EventId, USceneComponent* Parent, bool bAutoPlay);

	/** Fade out and release a component made by SpawnEventAttached. Safe on null. */
	static void FadeOutAndRelease(UAudioComponent* Component, float FadeSeconds);

	/** True when Actor (or the pawn that owns it) is controlled by the local player. */
	static bool IsLocalPlayerActor(const AActor* Actor);

	/**
	 * 0 (smallest: Viper fighter) .. 1 (largest: Behemoth freighter), from log10(HullStrength + CargoCapacity)
	 * across the DA_* roster's range. Null data = 0.3 (a light ship).
	 */
	static float GetShipSizeFactor(const USpaceshipDataAsset* ShipData);

	/** Ships at or above this size factor use the heavy thruster voice (Thruster.HeavyGroan). */
	static constexpr float HeavyShipSizeFactor = 0.5f;

	/** True while a FScopedMute is alive. */
	static bool IsMuted();

	/** Silences every event fired while it lives (nests). */
	struct ADASTREA_API FScopedMute
	{
		FScopedMute();
		~FScopedMute();
		FScopedMute(const FScopedMute&) = delete;
		FScopedMute& operator=(const FScopedMute&) = delete;
	};

private:
	/** Rate limit + mute gate. Records the time when it returns true. */
	static bool PassesGate(FName EventId, float MinInterval);

	/** Log line for adastrea.AudioEventLog. */
	static void LogEvent(const UObject* WorldContext, FName EventId, const TCHAR* Mode);
};
