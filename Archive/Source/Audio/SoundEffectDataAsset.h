// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "SoundEffectDataAsset.generated.h"

/**
 * Sound Effect Category for organizing audio assets
 */
UENUM(BlueprintType)
enum class ESoundEffectCategory : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Engine UMETA(DisplayName = "Engine"),
	UI UMETA(DisplayName = "UI"),
	Ambient UMETA(DisplayName = "Ambient"),
	Voice UMETA(DisplayName = "Voice"),
	Environmental UMETA(DisplayName = "Environmental"),
	Mechanical UMETA(DisplayName = "Mechanical"),
	Alert UMETA(DisplayName = "Alert")
};

/**
 * Data Asset for defining sound effects in Adastrea
 * 
 * This class provides a designer-friendly way to configure sound effects
 * with volume, pitch variation, attenuation, and categorization.
 * 
 * Usage:
 * 1. Create a Blueprint based on this class in the Content Browser
 * 2. Configure sound properties and assign audio files
 * 3. Reference this Data Asset from audio components or Blueprint logic
 * 4. Use Blueprint functions to play sounds with variations
 * 
 * Example:
 * - Create DA_SFX_LaserFire for weapon sounds
 * - Set category to Combat, volume to 0.8, pitch variation to 0.1
 * - Reference from weapon component to play during firing
 */
UCLASS(BlueprintType)
class ADASTREA_API USoundEffectDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	USoundEffectDataAsset();

	// ========================================================================
	// Basic Info
	// ========================================================================

	/** Display name for this sound effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	FText DisplayName;

	/** Description of when and how this sound should be used */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info", meta = (MultiLine = true))
	FText Description;

	/** Category for organizing sound effects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	ESoundEffectCategory Category;

	// ========================================================================
	// Audio Settings
	// ========================================================================

	/** The actual sound asset to play */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	USoundBase* Sound;

	/** Base volume multiplier (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings", 
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float Volume;

	/** Base pitch multiplier (0.5 to 2.0, where 1.0 is normal) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings",
		meta = (ClampMin = 0.5, ClampMax = 2.0, UIMin = 0.5, UIMax = 2.0))
	float Pitch;

	/** Random pitch variation range (+/- value) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings",
		meta = (ClampMin = 0.0, ClampMax = 0.5, UIMin = 0.0, UIMax = 0.5))
	float PitchVariation;

	/** Whether this sound should loop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
	bool bIsLooping;

	/** Priority of this sound (higher priority sounds are more likely to play when resources are limited) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float Priority;

	// ========================================================================
	// Spatial Settings
	// ========================================================================

	/** Whether this sound is 3D (spatialized) or 2D (non-spatialized) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Settings")
	bool bIs3DSound;

	/** Maximum distance at which this sound can be heard (for 3D sounds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Settings",
		meta = (EditCondition = "bIs3DSound", ClampMin = 100.0, ClampMax = 100000.0))
	float MaxDistance;

	/** Distance at which sound starts to attenuate (for 3D sounds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Settings",
		meta = (EditCondition = "bIs3DSound", ClampMin = 0.0, ClampMax = 50000.0))
	float AttenuationDistance;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Get the final pitch value with random variation applied
	 * @return Randomized pitch value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sound Effect")
	float GetRandomizedPitch() const;

	/**
	 * Check if this sound should be played based on priority and current audio load
	 * @param CurrentAudioLoad Current number of active sounds (0.0 to 1.0)
	 * @return True if sound should be played
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sound Effect")
	bool ShouldPlaySound(float CurrentAudioLoad) const;
};
