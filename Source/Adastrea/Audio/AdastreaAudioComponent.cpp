// Copyright Mittenzx. All Rights Reserved.

#include "Audio/AdastreaAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UAdastreaAudioComponent::UAdastreaAudioComponent()
	: MasterVolume(1.0f)
	, MaxSimultaneousSounds(8)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Tick twice per second for cleanup
}

void UAdastreaAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	ActiveAudioComponents.Reserve(MaxSimultaneousSounds);
}

void UAdastreaAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAllSounds();
	Super::EndPlay(EndPlayReason);
}

void UAdastreaAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CleanupFinishedAudioComponents();
}

bool UAdastreaAudioComponent::PlaySoundEffect(USoundEffectDataAsset* SoundEffect, FVector LocationOverride)
{
	if (!SoundEffect || !SoundEffect->Sound)
	{
		return false;
	}

	// Check if we should play this sound based on current load
	float CurrentLoad = GetCurrentAudioLoad();
	if (!SoundEffect->ShouldPlaySound(CurrentLoad))
	{
		return false;
	}

	// Get location for 3D sounds
	FVector PlayLocation = LocationOverride.IsZero() ? GetOwner()->GetActorLocation() : LocationOverride;

	// Get or create audio component
	UAudioComponent* AudioComp = GetAvailableAudioComponent();
	if (!AudioComp)
	{
		return false;
	}

	// Configure audio component
	AudioComp->SetSound(SoundEffect->Sound);
	AudioComp->SetVolumeMultiplier(SoundEffect->Volume * MasterVolume);
	AudioComp->SetPitchMultiplier(SoundEffect->GetRandomizedPitch());

	if (SoundEffect->bIs3DSound)
	{
		AudioComp->SetWorldLocation(PlayLocation);
		AudioComp->AttenuationSettings = nullptr; // Use sound's built-in attenuation
	}
	else
	{
		AudioComp->bAllowSpatialization = false;
	}

	// Play the sound
	AudioComp->Play();

	// Track this component
	ActiveAudioComponents.AddUnique(AudioComp);

	return true;
}

void UAdastreaAudioComponent::StopAllSounds()
{
	for (UAudioComponent* AudioComp : ActiveAudioComponents)
	{
		if (AudioComp && AudioComp->IsPlaying())
		{
			AudioComp->Stop();
		}
	}
	ActiveAudioComponents.Empty();
}

void UAdastreaAudioComponent::StopSoundsByCategory(ESoundEffectCategory Category)
{
	// Note: Category tracking would require storing the SoundEffectDataAsset reference
	// For now, this is a placeholder. Full implementation would need a TMap to track categories
	UE_LOG(LogTemp, Warning, TEXT("StopSoundsByCategory called, but category filtering is not implemented. All sounds will be stopped."));
	StopAllSounds();
}

float UAdastreaAudioComponent::GetCurrentAudioLoad() const
{
	if (MaxSimultaneousSounds <= 0)
	{
		return 0.0f;
	}

	int32 ActiveCount = 0;
	for (UAudioComponent* AudioComp : ActiveAudioComponents)
	{
		if (AudioComp && AudioComp->IsPlaying())
		{
			ActiveCount++;
		}
	}

	return static_cast<float>(ActiveCount) / static_cast<float>(MaxSimultaneousSounds);
}

bool UAdastreaAudioComponent::IsCategoryPlaying(ESoundEffectCategory Category) const
{
	// Note: Would need category tracking implementation
	UE_LOG(LogTemp, Warning, TEXT("IsCategoryPlaying called for category %d, but category tracking is not implemented. Returns true if any audio is playing, regardless of category."), static_cast<int32>(Category));
	return ActiveAudioComponents.Num() > 0;
}

void UAdastreaAudioComponent::CleanupFinishedAudioComponents()
{
	for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; --i)
	{
		UAudioComponent* AudioComp = ActiveAudioComponents[i];
		if (!AudioComp || !AudioComp->IsPlaying())
		{
			ActiveAudioComponents.RemoveAt(i);
			if (AudioComp)
			{
				AudioComp->DestroyComponent();
			}
		}
	}
}

UAudioComponent* UAdastreaAudioComponent::GetAvailableAudioComponent()
{
	// First try to find a stopped component we can reuse
	for (UAudioComponent* AudioComp : ActiveAudioComponents)
	{
		if (AudioComp && !AudioComp->IsPlaying())
		{
			return AudioComp;
		}
	}

	// If we're at max capacity, don't create new ones
	if (ActiveAudioComponents.Num() >= MaxSimultaneousSounds)
	{
		return nullptr;
	}

	// Create a new audio component
	UAudioComponent* NewAudioComp = NewObject<UAudioComponent>(GetOwner());
	if (NewAudioComp)
	{
		NewAudioComp->RegisterComponent();
		NewAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	return NewAudioComp;
}
