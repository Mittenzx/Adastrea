// Copyright Mittenzx. All Rights Reserved.

#include "Audio/SoundEffectDataAsset.h"

USoundEffectDataAsset::USoundEffectDataAsset()
	: Volume(0.8f)
	, Pitch(1.0f)
	, PitchVariation(0.05f)
	, bIsLooping(false)
	, Priority(0.5f)
	, bIs3DSound(true)
	, MaxDistance(10000.0f)
	, AttenuationDistance(1000.0f)
{
	DisplayName = FText::FromString("New Sound Effect");
	Description = FText::FromString("Configure this sound effect's properties");
	Category = ESoundEffectCategory::Ambient;
}

float USoundEffectDataAsset::GetRandomizedPitch() const
{
	if (PitchVariation > 0.0f)
	{
		float RandomVariation = FMath::FRandRange(-PitchVariation, PitchVariation);
		return FMath::Clamp(Pitch + RandomVariation, 0.5f, 2.0f);
	}
	return Pitch;
}

bool USoundEffectDataAsset::ShouldPlaySound(float CurrentAudioLoad) const
{
	// Higher priority sounds are more likely to play under load
	// If audio load is low, play all sounds
	if (CurrentAudioLoad < 0.7f)
	{
		return true;
	}

	// Under heavy load, use priority to determine if sound should play
	float Threshold = FMath::Lerp(0.3f, 1.0f, Priority);
	return (1.0f - CurrentAudioLoad) >= Threshold;
}
