// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Audio/AdastreaAudioSettings.h"
#include "AudioMixSubsystem.generated.h"

class UAudioComponent;
class USoundMix;
class USoundClass;
class ASpaceStation;
class AAsteroidField;
class AAdastreaPlayerController;
class AAdastreaHUD;

/**
 * The one place that owns the game's mix state (see SOUND_PLAN.md section 3).
 *
 * Every game/PIE world gets one. Each tick it reads the local player's controller and
 * HUD state and:
 *  - pushes/pops SM_Interior while the player is on foot (engine + world low-passed and lowered);
 *  - pushes/pops SM_MenuDuck while the pause menu, trading screen or Station Editor is open;
 *  - runs the ambience beds (all transitions fade; nothing hard-cuts):
 *      Ambient.Space        2D bed, very low, ducked near stations, in fields, on foot and on the map;
 *      Ambient.StationHum   3D loop attached to each ASpaceStation (catalog attenuation does the falloff);
 *      Ambient.AsteroidCreak sparse random one-shots around the player inside an asteroid field;
 *      Ambient.MapRoomTone  2D, only while the sector map is open;
 *  - applies the player's volume settings (UAdastreaAudioSettings) as class overrides on SM_Settings.
 *
 * Sound assets are only ever reached through UAudioCatalogSubsystem event IDs; the mixes and
 * classes are loaded by soft path from /Game/Audio (always-cooked). Missing assets log once and no-op.
 */
UCLASS()
class ADASTREA_API UAudioMixSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UAudioMixSubsystem* Get(const UObject* WorldContextObject);

	// USubsystem / UWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }

	/** Set one volume slider (0..1), apply it immediately and persist it. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Mix")
	void SetCategoryVolume(EAdastreaVolumeCategory Category, float Value);

	UFUNCTION(BlueprintPure, Category = "Audio|Mix")
	float GetCategoryVolume(EAdastreaVolumeCategory Category) const;

	/** Re-apply every saved volume as SM_Settings class overrides. */
	void ApplyVolumeSettings(float FadeSeconds);

	UFUNCTION(BlueprintPure, Category = "Audio|Mix")
	bool IsInteriorMixActive() const { return bInteriorMixActive; }

	UFUNCTION(BlueprintPure, Category = "Audio|Mix")
	bool IsMenuDuckActive() const { return bMenuDuckActive; }

	/** Console: log the current mix/ambience state. */
	void LogState() const;

	// ---- Tunables (seconds / multipliers on the catalog entry volume) ----

	/** Bed crossfade time. */
	float BedFadeSeconds = 2.5f;

	/** Space bed level in each situation (multiplier; 1 = the catalog level, which is already very low). */
	float SpaceBedOpen = 1.0f;
	float SpaceBedNearStation = 0.55f;
	float SpaceBedInField = 0.6f;
	float SpaceBedOnFoot = 0.2f;
	float SpaceBedMapOpen = 0.3f;

	/** Seconds between asteroid creaks, uniform in [Min, Max]. */
	float CreakMinInterval = 8.0f;
	float CreakMaxInterval = 20.0f;

	/** Creaks play at this distance band from the listener (cm). */
	float CreakMinDistance = 1500.0f;
	float CreakMaxDistance = 6000.0f;

	/** Field "inside" test: distance to the field centre <= FieldRadius * this. */
	float FieldRadiusScale = 1.1f;

	/** Station hum components exist only inside this range (cm); falls back when the attenuation is unknown. */
	float DefaultHumRange = 150000.0f;

	/** How often (s) the station/field scan runs. */
	float ScanInterval = 1.0f;

private:
	AAdastreaPlayerController* GetLocalController() const;
	FVector GetListenerLocation(const AAdastreaPlayerController* PC) const;

	void UpdateMixes(bool bInterior, bool bMenu);
	void SetMixActive(const TCHAR* MixPath, TObjectPtr<USoundMix>& Cached, bool& bActive, bool bWant);
	USoundMix* LoadMix(const TCHAR* Path, TObjectPtr<USoundMix>& Cached) const;
	USoundClass* LoadClass(const TCHAR* Path) const;

	void UpdateStationHums(const FVector& Listener);
	void UpdateAsteroidCreaks(float DeltaTime, const FVector& Listener, bool bInField, bool bOnFoot);

	/** Fade a looping bed toward Level (0 = fade out and stop). Creates it on first use. */
	void SetBedLevel(FName EventId, TObjectPtr<UAudioComponent>& Comp, float& CurrentLevel, float Level);

	bool IsInsideAnyField(const FVector& Listener) const;
	float GetHumRange() const;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> InteriorMix;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> MenuDuckMix;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> SettingsMix;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> SpaceBed;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MapRoomTone;

	/** One looping hum per station in range (the component is owned by the station actor). */
	TMap<TWeakObjectPtr<ASpaceStation>, TWeakObjectPtr<UAudioComponent>> StationHums;

	TArray<TWeakObjectPtr<AAsteroidField>> Fields;

	/** Beds whose event couldn't be resolved (not retried). */
	TSet<FName> FailedBeds;

	bool bInteriorMixActive = false;
	bool bMenuDuckActive = false;
	bool bSettingsMixPushed = false;
	bool bStarted = false;

	float SpaceBedLevel = 0.0f;
	float MapToneLevel = 0.0f;
	float ScanTimer = 0.0f;
	float NextCreakIn = 0.0f;
	bool bWasInField = false;
	bool bWasNearStation = false;

	mutable float CachedHumRange = -1.0f;
};
