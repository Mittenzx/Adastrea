// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/AdastreaAudioSettings.h"
#include "Misc/ConfigCacheIni.h"

float UAdastreaAudioSettings::GetVolume(EAdastreaVolumeCategory Category) const
{
	switch (Category)
	{
	case EAdastreaVolumeCategory::Master: return MasterVolume;
	case EAdastreaVolumeCategory::SFX:    return SFXVolume;
	case EAdastreaVolumeCategory::UI:     return UIVolume;
	default:                              return 1.0f;
	}
}

void UAdastreaAudioSettings::SetVolume(EAdastreaVolumeCategory Category, float Value)
{
	const float Clamped = FMath::Clamp(Value, 0.0f, 1.0f);
	switch (Category)
	{
	case EAdastreaVolumeCategory::Master: MasterVolume = Clamped; break;
	case EAdastreaVolumeCategory::SFX:    SFXVolume = Clamped; break;
	case EAdastreaVolumeCategory::UI:     UIVolume = Clamped; break;
	default: break;
	}
}

void UAdastreaAudioSettings::Save()
{
	// config=GameUserSettings resolves to GGameUserSettingsIni (Saved/Config/<Platform>/GameUserSettings.ini).
	SaveConfig();
	if (GConfig)
	{
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

const TCHAR* UAdastreaAudioSettings::GetCategoryLabel(EAdastreaVolumeCategory Category)
{
	switch (Category)
	{
	case EAdastreaVolumeCategory::Master: return TEXT("Master");
	case EAdastreaVolumeCategory::SFX:    return TEXT("SFX");
	case EAdastreaVolumeCategory::UI:     return TEXT("UI");
	default:                              return TEXT("?");
	}
}
