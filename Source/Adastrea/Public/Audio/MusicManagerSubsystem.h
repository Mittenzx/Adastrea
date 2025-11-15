// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio/MusicTrackDataAsset.h"
#include "MusicManagerSubsystem.generated.h"

/**
 * Music Manager Subsystem for handling dynamic music transitions
 * 
 * This subsystem manages music playback with dynamic mixing based on
 * gameplay context (mood and intensity).
 * 
 * Usage:
 * 1. Access via UGameInstance::GetSubsystem<UMusicManagerSubsystem>()
 * 2. Call SetMusicContext to change mood/intensity
 * 3. System automatically handles transitions and crossfades
 * 
 * Example:
 * - During peaceful exploration: SetMusicContext(Peaceful, Low)
 * - When combat starts: SetMusicContext(Combat, High)
 * - System fades between tracks smoothly
 */
UCLASS()
class ADASTREA_API UMusicManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMusicManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// Configuration
	// ========================================================================

	/** Master music volume (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float MasterMusicVolume;

	/** Default crossfade time when not specified by tracks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings",
		meta = (ClampMin = 0.0, ClampMax = 10.0, UIMin = 0.0, UIMax = 10.0))
	float DefaultCrossfadeTime;

	/** Available music tracks for dynamic selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	TArray<UMusicTrackDataAsset*> MusicLibrary;

	// ========================================================================
	// State
	// ========================================================================

	/** Currently playing music track */
	UPROPERTY(BlueprintReadOnly, Category = "Music State")
	UMusicTrackDataAsset* CurrentTrack;

	/** Current music mood */
	UPROPERTY(BlueprintReadOnly, Category = "Music State")
	EMusicMood CurrentMood;

	/** Current music intensity */
	UPROPERTY(BlueprintReadOnly, Category = "Music State")
	EMusicIntensity CurrentIntensity;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Set the music context (mood and intensity) for dynamic music selection
	 * @param NewMood Desired music mood
	 * @param NewIntensity Desired music intensity
	 * @param bForceImmediate If true, changes immediately without fade
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void SetMusicContext(EMusicMood NewMood, EMusicIntensity NewIntensity, bool bForceImmediate = false);

	/**
	 * Play a specific music track immediately
	 * @param Track The track to play
	 * @param bFadeIn Whether to fade in the track
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void PlayTrack(UMusicTrackDataAsset* Track, bool bFadeIn = true);

	/**
	 * Stop the currently playing music
	 * @param bFadeOut Whether to fade out the music
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void StopMusic(bool bFadeOut = true);

	/**
	 * Pause the currently playing music
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void PauseMusic();

	/**
	 * Resume paused music
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void ResumeMusic();

	/**
	 * Set the master music volume
	 * @param NewVolume Volume value (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void SetMasterVolume(float NewVolume);

	/**
	 * Check if music is currently playing
	 * @return True if music is playing
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Music Manager")
	bool IsMusicPlaying() const;

	/**
	 * Add a track to the music library
	 * @param Track Track to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Music Manager")
	void AddTrackToLibrary(UMusicTrackDataAsset* Track);

private:
	/** Audio component for playing music */
	UPROPERTY()
	UAudioComponent* MusicAudioComponent;

	/** Audio component for crossfading */
	UPROPERTY()
	UAudioComponent* CrossfadeAudioComponent;

	/** Find the best matching track for current context */
	UMusicTrackDataAsset* FindBestTrackForContext() const;

	/** Handle crossfade between tracks */
	void CrossfadeTracks(UMusicTrackDataAsset* NewTrack);

	/** Timer handle for crossfade */
	FTimerHandle CrossfadeTimerHandle;
};
