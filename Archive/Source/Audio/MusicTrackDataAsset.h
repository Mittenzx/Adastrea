// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "MusicTrackDataAsset.generated.h"

/**
 * Music mood types for dynamic music selection
 */
UENUM(BlueprintType)
enum class EMusicMood : uint8
{
	Peaceful UMETA(DisplayName = "Peaceful"),
	Tense UMETA(DisplayName = "Tense"),
	Combat UMETA(DisplayName = "Combat"),
	Exploration UMETA(DisplayName = "Exploration"),
	Dramatic UMETA(DisplayName = "Dramatic"),
	Victory UMETA(DisplayName = "Victory"),
	Defeat UMETA(DisplayName = "Defeat"),
	Ambient UMETA(DisplayName = "Ambient")
};

/**
 * Music intensity level for dynamic mixing
 */
UENUM(BlueprintType)
enum class EMusicIntensity : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High")
};

/**
 * Data Asset for defining music tracks with dynamic mixing support
 * 
 * This class provides a designer-friendly way to configure music tracks
 * that can be dynamically mixed based on gameplay context.
 * 
 * Usage:
 * 1. Create a Blueprint based on this class
 * 2. Configure music properties and assign audio files
 * 3. Set mood and intensity for dynamic selection
 * 4. Reference from music manager to handle transitions
 * 
 * Example:
 * - Create DA_Music_CombatHigh for intense combat
 * - Set mood to Combat, intensity to High
 * - Configure fade times for smooth transitions
 */
UCLASS(BlueprintType)
class ADASTREA_API UMusicTrackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UMusicTrackDataAsset();

	// ========================================================================
	// Basic Info
	// ========================================================================

	/** Display name for this music track */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	FText DisplayName;

	/** Description of when this music should play */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info", meta = (MultiLine = true))
	FText Description;

	// ========================================================================
	// Music Settings
	// ========================================================================

	/** The music audio asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Settings")
	USoundBase* MusicTrack;

	/** Base volume for this track (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Settings",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float Volume;

	/** Mood classification for dynamic selection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Settings")
	EMusicMood Mood;

	/** Intensity level for layered music system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Settings")
	EMusicIntensity Intensity;

	// ========================================================================
	// Transition Settings
	// ========================================================================

	/** Time to fade in when starting this track (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition Settings",
		meta = (ClampMin = 0.0, ClampMax = 10.0, UIMin = 0.0, UIMax = 10.0))
	float FadeInTime;

	/** Time to fade out when ending this track (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition Settings",
		meta = (ClampMin = 0.0, ClampMax = 10.0, UIMin = 0.0, UIMax = 10.0))
	float FadeOutTime;

	/** Whether this track can smoothly crossfade with others */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition Settings")
	bool bAllowCrossfade;

	/** Priority for track selection (higher priority plays first) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition Settings",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float Priority;

	// ========================================================================
	// Layering Settings
	// ========================================================================

	/** Whether this track is a layer that can be mixed with base tracks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layering Settings")
	bool bIsLayer;

	/** Base track this layer should play with (if bIsLayer is true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layering Settings",
		meta = (EditCondition = "bIsLayer"))
	UMusicTrackDataAsset* BaseTrack;

	/** Volume multiplier when this layer is active (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layering Settings",
		meta = (EditCondition = "bIsLayer", ClampMin = 0.0, ClampMax = 1.0))
	float LayerVolume;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Check if this track matches the desired mood and intensity
	 * @param DesiredMood Target mood to match
	 * @param DesiredIntensity Target intensity to match
	 * @return True if this track is a good match
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Music Track")
	bool MatchesContext(EMusicMood DesiredMood, EMusicIntensity DesiredIntensity) const;

	/**
	 * Get the effective volume for this track considering all modifiers
	 * @param MasterVolume Master music volume setting
	 * @return Final volume value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Music Track")
	float GetEffectiveVolume(float MasterVolume) const;
};
