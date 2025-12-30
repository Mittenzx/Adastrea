// Copyright Mittenzx. All Rights Reserved.

#include "Audio/MusicTrackDataAsset.h"

UMusicTrackDataAsset::UMusicTrackDataAsset()
	: Volume(0.7f)
	, Mood(EMusicMood::Ambient)
	, Intensity(EMusicIntensity::Low)
	, FadeInTime(2.0f)
	, FadeOutTime(2.0f)
	, bAllowCrossfade(true)
	, Priority(0.5f)
	, bIsLayer(false)
	, BaseTrack(nullptr)
	, LayerVolume(0.5f)
{
	DisplayName = FText::FromString("New Music Track");
	Description = FText::FromString("Configure this music track's properties");
}

bool UMusicTrackDataAsset::MatchesContext(EMusicMood DesiredMood, EMusicIntensity DesiredIntensity) const
{
	// Exact mood match is required
	if (Mood != DesiredMood)
	{
		return false;
	}

	// Intensity can be close (within one level)
	int32 IntensityDiff = FMath::Abs(static_cast<int32>(Intensity) - static_cast<int32>(DesiredIntensity));
	return IntensityDiff <= 1;
}

float UMusicTrackDataAsset::GetEffectiveVolume(float MasterVolume) const
{
	float EffectiveVolume = Volume * MasterVolume;
	
	if (bIsLayer)
	{
		EffectiveVolume *= LayerVolume;
	}

	return FMath::Clamp(EffectiveVolume, 0.0f, 1.0f);
}
