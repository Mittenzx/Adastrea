// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShipEngineSoundProfile.generated.h"

class USoundBase;

/**
 * Engine loop family. Each family is its own set of generated loops (low revs, high revs),
 * so a ship only has to pitch-shift its family's loops by a small amount (about 0.7x-1.4x).
 * The catalog event IDs are Engine.<Family>.Low and Engine.<Family>.High.
 */
UENUM(BlueprintType)
enum class EShipEngineFamily : uint8
{
	Light,		// fighters, couriers: ~110-140 Hz body, bright turbine whine
	Medium,		// corvettes, scouts: ~70-90 Hz
	Heavy,		// freighters, cruisers: ~45-60 Hz with a slow throb
	Capital		// battleships, carriers: ~30-40 Hz sub rumble, almost no whine
};

/**
 * Optional per-ship engine sound override on USpaceshipDataAsset.
 *
 * Left at its defaults (bOverride false, no sounds) the ship's engine voice is derived
 * from its stats by UShipEngineAudioComponent (size -> family + pitch, agility -> whine).
 * Set bOverride to pick the family, pitch offset and whine by hand. The custom sounds
 * are independent of bOverride: any that are set replace that layer's catalog sound.
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FShipEngineSoundProfile
{
	GENERATED_BODY()

	/** Use Family / PitchOffset / WhineAmount below instead of the values derived from the ship's stats. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound")
	bool bOverride = false;

	/** Loop family (only used when bOverride is set). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound", meta=(EditCondition="bOverride"))
	EShipEngineFamily Family = EShipEngineFamily::Medium;

	/** Added to the family's centre pitch (1.0 = the loop as generated). Only used when bOverride is set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound", meta=(EditCondition="bOverride", ClampMin="-0.3", ClampMax="0.3"))
	float PitchOffset = 0.0f;

	/** Turbine whine level, 0 = none, 1 = full (only used when bOverride is set). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound", meta=(EditCondition="bOverride", ClampMin="0.0", ClampMax="1.0"))
	float WhineAmount = 0.5f;

	/** Optional replacement for the low-revs loop (else the catalog's Engine.<Family>.Low). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound|Custom Sounds")
	TSoftObjectPtr<USoundBase> LowRevsSound;

	/** Optional replacement for the high-revs loop (else the catalog's Engine.<Family>.High). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound|Custom Sounds")
	TSoftObjectPtr<USoundBase> HighRevsSound;

	/** Optional replacement for the whine layer (else Engine.Whine). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound|Custom Sounds")
	TSoftObjectPtr<USoundBase> WhineSound;

	/** Optional replacement for the boost layer (else Engine.Boost). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound|Custom Sounds")
	TSoftObjectPtr<USoundBase> BoostSound;

	/** Optional replacement for the idle hum (else Engine.Idle). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Sound|Custom Sounds")
	TSoftObjectPtr<USoundBase> IdleSound;
};

/** A ship's resolved engine voice: the static part that doesn't change with throttle/speed. */
USTRUCT(BlueprintType)
struct ADASTREA_API FShipEngineVoice
{
	GENERATED_BODY()

	/** Normalised size across the roster (0 = smallest, 1 = largest), from hull + cargo. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	float Size = 0.5f;

	/** Normalised agility across the roster (0 = sluggish, 1 = nimble), from acceleration x maneuverability. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	float Agility = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	EShipEngineFamily Family = EShipEngineFamily::Medium;

	/** Pitch multiplier at idle revs, no boost. Smaller ships in a family sit higher. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	float BasePitch = 1.0f;

	/** Whine layer level 0..1. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	float WhineAmount = 0.5f;

	/** True when the ship's FShipEngineSoundProfile supplied family/pitch/whine. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine Voice")
	bool bFromOverride = false;
};
