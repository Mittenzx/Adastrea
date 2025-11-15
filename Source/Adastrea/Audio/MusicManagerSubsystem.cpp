// Copyright Mittenzx. All Rights Reserved.

#include "Audio/MusicManagerSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UMusicManagerSubsystem::UMusicManagerSubsystem()
	: MasterMusicVolume(0.7f)
	, DefaultCrossfadeTime(3.0f)
	, CurrentTrack(nullptr)
	, CurrentMood(EMusicMood::Ambient)
	, CurrentIntensity(EMusicIntensity::Low)
	, MusicAudioComponent(nullptr)
	, CrossfadeAudioComponent(nullptr)
{
}

void UMusicManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Music audio components will be created when first needed
}

void UMusicManagerSubsystem::Deinitialize()
{
	StopMusic(false);

	if (MusicAudioComponent)
	{
		MusicAudioComponent->DestroyComponent();
		MusicAudioComponent = nullptr;
	}

	if (CrossfadeAudioComponent)
	{
		CrossfadeAudioComponent->DestroyComponent();
		CrossfadeAudioComponent = nullptr;
	}

	Super::Deinitialize();
}

void UMusicManagerSubsystem::SetMusicContext(EMusicMood NewMood, EMusicIntensity NewIntensity, bool bForceImmediate)
{
	// Update context
	CurrentMood = NewMood;
	CurrentIntensity = NewIntensity;

	// Find best matching track
	UMusicTrackDataAsset* BestTrack = FindBestTrackForContext();
	if (!BestTrack || BestTrack == CurrentTrack)
	{
		return; // No better track found or already playing the best track
	}

	// Play the new track
	if (bForceImmediate)
	{
		PlayTrack(BestTrack, false);
	}
	else
	{
		CrossfadeTracks(BestTrack);
	}
}

void UMusicManagerSubsystem::PlayTrack(UMusicTrackDataAsset* Track, bool bFadeIn)
{
	if (!Track || !Track->MusicTrack)
	{
		return;
	}

	// Create music audio component if needed
	if (!MusicAudioComponent)
	{
		MusicAudioComponent = NewObject<UAudioComponent>(this);
		if (MusicAudioComponent)
		{
			MusicAudioComponent->RegisterComponent();
			MusicAudioComponent->bAllowSpatialization = false;
			MusicAudioComponent->bIsUISound = true;
		}
	}

	if (!MusicAudioComponent)
	{
		return;
	}

	// Stop any currently playing music
	if (MusicAudioComponent->IsPlaying())
	{
		MusicAudioComponent->Stop();
	}

	// Configure and play new track
	MusicAudioComponent->SetSound(Track->MusicTrack);
	float EffectiveVolume = Track->GetEffectiveVolume(MasterMusicVolume);
	MusicAudioComponent->SetVolumeMultiplier(bFadeIn ? 0.0f : EffectiveVolume);
	MusicAudioComponent->Play();

	// Handle fade in
	if (bFadeIn)
	{
		MusicAudioComponent->FadeIn(Track->FadeInTime, EffectiveVolume);
	}

	CurrentTrack = Track;
}

void UMusicManagerSubsystem::StopMusic(bool bFadeOut)
{
	if (!MusicAudioComponent || !MusicAudioComponent->IsPlaying())
	{
		return;
	}

	if (bFadeOut && CurrentTrack)
	{
		MusicAudioComponent->FadeOut(CurrentTrack->FadeOutTime, 0.0f);
	}
	else
	{
		MusicAudioComponent->Stop();
	}

	CurrentTrack = nullptr;
}

void UMusicManagerSubsystem::PauseMusic()
{
	if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
	{
		MusicAudioComponent->SetPaused(true);
	}
}

void UMusicManagerSubsystem::ResumeMusic()
{
	if (MusicAudioComponent)
	{
		MusicAudioComponent->SetPaused(false);
	}
}

void UMusicManagerSubsystem::SetMasterVolume(float NewVolume)
{
	MasterMusicVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

	// Update currently playing track volume
	if (MusicAudioComponent && CurrentTrack)
	{
		float EffectiveVolume = CurrentTrack->GetEffectiveVolume(MasterMusicVolume);
		MusicAudioComponent->SetVolumeMultiplier(EffectiveVolume);
	}
}

bool UMusicManagerSubsystem::IsMusicPlaying() const
{
	return MusicAudioComponent && MusicAudioComponent->IsPlaying();
}

void UMusicManagerSubsystem::AddTrackToLibrary(UMusicTrackDataAsset* Track)
{
	if (Track)
	{
		MusicLibrary.AddUnique(Track);
	}
}

UMusicTrackDataAsset* UMusicManagerSubsystem::FindBestTrackForContext() const
{
	UMusicTrackDataAsset* BestTrack = nullptr;
	float BestPriority = -1.0f;

	for (UMusicTrackDataAsset* Track : MusicLibrary)
	{
		if (!Track)
		{
			continue;
		}

		// Check if track matches context
		if (Track->MatchesContext(CurrentMood, CurrentIntensity))
		{
			// Select track with highest priority
			if (Track->Priority > BestPriority)
			{
				BestTrack = Track;
				BestPriority = Track->Priority;
			}
		}
	}

	return BestTrack;
}

void UMusicManagerSubsystem::CrossfadeTracks(UMusicTrackDataAsset* NewTrack)
{
	if (!NewTrack || !NewTrack->MusicTrack)
	{
		return;
	}

	// Create crossfade component if needed
	if (!CrossfadeAudioComponent)
	{
		CrossfadeAudioComponent = NewObject<UAudioComponent>(this);
		if (CrossfadeAudioComponent)
		{
			CrossfadeAudioComponent->RegisterComponent();
			CrossfadeAudioComponent->bAllowSpatialization = false;
			CrossfadeAudioComponent->bIsUISound = true;
		}
	}

	if (!CrossfadeAudioComponent)
	{
		// Fallback to immediate play
		PlayTrack(NewTrack, true);
		return;
	}

	// Fade out current track
	if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
	{
		float FadeTime = CurrentTrack ? CurrentTrack->FadeOutTime : DefaultCrossfadeTime;
		MusicAudioComponent->FadeOut(FadeTime, 0.0f);
	}

	// Start new track on crossfade component
	CrossfadeAudioComponent->SetSound(NewTrack->MusicTrack);
	float EffectiveVolume = NewTrack->GetEffectiveVolume(MasterMusicVolume);
	CrossfadeAudioComponent->SetVolumeMultiplier(0.0f);
	CrossfadeAudioComponent->Play();
	CrossfadeAudioComponent->FadeIn(NewTrack->FadeInTime, EffectiveVolume);

	// Swap components after fade completes
	float SwapDelay = FMath::Max(NewTrack->FadeInTime, DefaultCrossfadeTime);
	GetWorld()->GetTimerManager().SetTimer(CrossfadeTimerHandle, [this, NewTrack]()
	{
		// Swap components
		UAudioComponent* Temp = MusicAudioComponent;
		MusicAudioComponent = CrossfadeAudioComponent;
		CrossfadeAudioComponent = Temp;

		if (CrossfadeAudioComponent && CrossfadeAudioComponent->IsPlaying())
		{
			CrossfadeAudioComponent->Stop();
		}

		CurrentTrack = NewTrack;
	}, SwapDelay, false);
}
