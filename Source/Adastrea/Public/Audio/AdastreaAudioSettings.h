// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AdastreaAudioSettings.generated.h"

/** The player-facing volume sliders (pause menu). No music slider until the music session. */
UENUM(BlueprintType)
enum class EAdastreaVolumeCategory : uint8
{
	Master,
	SFX,
	UI
};

/**
 * Player volume settings, persisted in GameUserSettings.ini under
 * [/Script/Adastrea.AdastreaAudioSettings]. Loaded automatically with the CDO at startup;
 * UAudioMixSubsystem applies them as SoundMix class overrides (SM_Settings) on every world.
 *
 * Values are linear 0..1 gain multipliers on SC_Master / SC_SFX / SC_UI (children included).
 */
UCLASS(config = GameUserSettings, configdonotcheckdefaults)
class ADASTREA_API UAdastreaAudioSettings : public UObject
{
	GENERATED_BODY()

public:
	/** Mutable CDO (the single settings instance). */
	static UAdastreaAudioSettings* Get() { return GetMutableDefault<UAdastreaAudioSettings>(); }

	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UIVolume = 1.0f;

	float GetVolume(EAdastreaVolumeCategory Category) const;

	/** Clamps to 0..1 and stores (does not save; call Save()). */
	void SetVolume(EAdastreaVolumeCategory Category, float Value);

	/** Writes the values to GameUserSettings.ini. */
	void Save();

	static const TCHAR* GetCategoryLabel(EAdastreaVolumeCategory Category);
};
