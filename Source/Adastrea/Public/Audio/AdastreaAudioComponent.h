// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Audio/SoundEffectDataAsset.h"
#include "AdastreaAudioComponent.generated.h"

/**
 * Audio Component for managing sound effects on actors
 * 
 * This component handles playing sound effects from SoundEffectDataAssets
 * with automatic resource management and priority-based playback.
 * 
 * Usage:
 * 1. Add this component to any actor that needs to play sounds
 * 2. Call PlaySoundEffect with a SoundEffectDataAsset
 * 3. Component handles spatial audio, attenuation, and priority
 * 
 * Example:
 * - Add to spaceship actor
 * - Call PlaySoundEffect with DA_SFX_LaserFire when weapon fires
 * - Component manages 3D positioning and volume automatically
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class ADASTREA_API UAdastreaAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdastreaAudioComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================================================
	// Configuration
	// ========================================================================

	/** Master volume multiplier for all sounds from this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float MasterVolume;

	/** Maximum number of simultaneous sounds this component can play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings",
		meta = (ClampMin = 1, ClampMax = 32))
	int32 MaxSimultaneousSounds;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Play a sound effect from a data asset
	 * @param SoundEffect The sound effect data asset to play
	 * @param LocationOverride Optional location override for 3D sounds (uses owner location if not set)
	 * @return True if sound was played
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool PlaySoundEffect(USoundEffectDataAsset* SoundEffect, FVector LocationOverride = FVector::ZeroVector);

	/**
	 * Stop all currently playing sounds from this component
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllSounds();

	/**
	 * Stop sounds of a specific category
	 * @param Category The category of sounds to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopSoundsByCategory(ESoundEffectCategory Category);

	/**
	 * Get the current audio load (0.0 to 1.0 based on active sounds)
	 * @return Current audio load percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
	float GetCurrentAudioLoad() const;

	/**
	 * Check if a sound of the given category is currently playing
	 * @param Category The category to check
	 * @return True if any sound in this category is playing
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
	bool IsCategoryPlaying(ESoundEffectCategory Category) const;

private:
	/** Internal tracking of active audio components */
	UPROPERTY()
	TArray<UAudioComponent*> ActiveAudioComponents;

	/** Map of audio components by category for category-based operations */
	TMap<ESoundEffectCategory, TArray<UAudioComponent*>> AudioComponentsByCategory;

	/** Cleanup finished audio components */
	void CleanupFinishedAudioComponents();

	/** Get or create an audio component for playback */
	UAudioComponent* GetAvailableAudioComponent();

	/** Track an audio component with its category */
	void TrackAudioComponent(UAudioComponent* AudioComp, ESoundEffectCategory Category);

	/** Remove an audio component from tracking */
	void UntrackAudioComponent(UAudioComponent* AudioComp);
};
