// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/AudioMixSubsystem.h"
#include "Audio/AudioCatalogSubsystem.h"
#include "Audio/AudioCatalogDataAsset.h"
#include "AdastreaHUD.h"
#include "Player/AdastreaPlayerController.h"
#include "Stations/SpaceStation.h"
#include "Mining/AsteroidField.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdastreaMix, Log, All);

namespace AdastreaMix
{
	// Mixes (created by Tools/create_audio_mixes.py) and classes (SoundSmith's importer).
	static const TCHAR* InteriorMixPath = TEXT("/Game/Audio/Mixes/SM_Interior.SM_Interior");
	static const TCHAR* MenuDuckMixPath = TEXT("/Game/Audio/Mixes/SM_MenuDuck.SM_MenuDuck");
	static const TCHAR* SettingsMixPath = TEXT("/Game/Audio/Mixes/SM_Settings.SM_Settings");

	static const TCHAR* MasterClassPath = TEXT("/Game/Audio/Classes/SC_Master.SC_Master");
	static const TCHAR* SFXClassPath    = TEXT("/Game/Audio/Classes/SC_SFX.SC_SFX");
	static const TCHAR* UIClassPath     = TEXT("/Game/Audio/Classes/SC_UI.SC_UI");

	static const FName SpaceBedId(TEXT("Ambient.Space"));
	static const FName StationHumId(TEXT("Ambient.StationHum"));
	static const FName AsteroidCreakId(TEXT("Ambient.AsteroidCreak"));
	static const FName MapRoomToneId(TEXT("Ambient.MapRoomTone"));

	static const TCHAR* ClassPathFor(EAdastreaVolumeCategory Category)
	{
		switch (Category)
		{
		case EAdastreaVolumeCategory::Master: return MasterClassPath;
		case EAdastreaVolumeCategory::SFX:    return SFXClassPath;
		case EAdastreaVolumeCategory::UI:     return UIClassPath;
		default:                              return nullptr;
		}
	}

	static bool ParseCategory(const FString& In, EAdastreaVolumeCategory& Out)
	{
		if (In.Equals(TEXT("Master"), ESearchCase::IgnoreCase)) { Out = EAdastreaVolumeCategory::Master; return true; }
		if (In.Equals(TEXT("SFX"), ESearchCase::IgnoreCase))    { Out = EAdastreaVolumeCategory::SFX; return true; }
		if (In.Equals(TEXT("UI"), ESearchCase::IgnoreCase))     { Out = EAdastreaVolumeCategory::UI; return true; }
		return false;
	}

	// Console helpers for PIE / packaged verification.
	static FAutoConsoleCommandWithWorld StateCmd(
		TEXT("Adastrea.AudioMix.State"),
		TEXT("Log the audio mix state (active mixes, bed levels, hums, volumes)."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UAudioMixSubsystem* Mix = World ? World->GetSubsystem<UAudioMixSubsystem>() : nullptr)
			{
				Mix->LogState();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs SetVolumeCmd(
		TEXT("Adastrea.AudioMix.SetVolume"),
		TEXT("Adastrea.AudioMix.SetVolume <Master|SFX|UI> <0-100>: set and persist a volume slider."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UAudioMixSubsystem* Mix = World ? World->GetSubsystem<UAudioMixSubsystem>() : nullptr;
			EAdastreaVolumeCategory Category;
			if (!Mix || Args.Num() < 2 || !ParseCategory(Args[0], Category))
			{
				UE_LOG(LogAdastreaMix, Warning, TEXT("Usage: Adastrea.AudioMix.SetVolume <Master|SFX|UI> <0-100>"));
				return;
			}
			Mix->SetCategoryVolume(Category, FCString::Atof(*Args[1]) / 100.0f);
		}));
}

UAudioMixSubsystem* UAudioMixSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UAudioMixSubsystem>() : nullptr;
}

bool UAudioMixSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UAudioMixSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	bStarted = true;
	ScanTimer = 0.0f; // scan on the first tick
	NextCreakIn = FMath::FRandRange(2.0f, CreakMinInterval);

	// Saved sliders (GameUserSettings.ini) apply from the first frame.
	ApplyVolumeSettings(0.0f);

	const UAdastreaAudioSettings* Settings = UAdastreaAudioSettings::Get();
	UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: started in %s (volumes Master=%.0f%% SFX=%.0f%% UI=%.0f%%)"),
		*InWorld.GetName(), Settings->MasterVolume * 100.0f, Settings->SFXVolume * 100.0f, Settings->UIVolume * 100.0f);
}

void UAudioMixSubsystem::Deinitialize()
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (bInteriorMixActive && InteriorMix) { UGameplayStatics::PopSoundMixModifier(World, InteriorMix); }
		if (bMenuDuckActive && MenuDuckMix)    { UGameplayStatics::PopSoundMixModifier(World, MenuDuckMix); }
		if (bSettingsMixPushed && SettingsMix) { UGameplayStatics::PopSoundMixModifier(World, SettingsMix); }
	}
	bInteriorMixActive = bMenuDuckActive = bSettingsMixPushed = false;

	if (SpaceBed)    { SpaceBed->Stop(); }
	if (MapRoomTone) { MapRoomTone->Stop(); }
	for (auto& Pair : StationHums)
	{
		if (UAudioComponent* Hum = Pair.Value.Get())
		{
			Hum->Stop();
		}
	}
	StationHums.Reset();
	bStarted = false;
	Super::Deinitialize();
}

TStatId UAudioMixSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAudioMixSubsystem, STATGROUP_Tickables);
}

AAdastreaPlayerController* UAudioMixSubsystem::GetLocalController() const
{
	UWorld* World = GetWorld();
	return World ? Cast<AAdastreaPlayerController>(World->GetFirstPlayerController()) : nullptr;
}

FVector UAudioMixSubsystem::GetListenerLocation(const AAdastreaPlayerController* PC) const
{
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraLocation();
	}
	if (PC && PC->GetPawn())
	{
		return PC->GetPawn()->GetActorLocation();
	}
	return FVector::ZeroVector;
}

void UAudioMixSubsystem::Tick(float DeltaTime)
{
	if (!bStarted)
	{
		return;
	}
	AAdastreaPlayerController* PC = GetLocalController();
	if (!PC)
	{
		return;
	}
	const AAdastreaHUD* HUD = Cast<AAdastreaHUD>(PC->GetHUD());

	const bool bOnFoot = PC->IsOnFoot() || PC->IsWalkingStation();
	const bool bMenu = (HUD && (HUD->bShowPauseMenu || HUD->bShowTradeScreen))
		|| PC->IsMainMenuOpen() || PC->IsTradingOpen() || PC->IsStationEditorOpen();
	const bool bMap = HUD && HUD->bShowMap;

	UpdateMixes(bOnFoot, bMenu);

	const FVector Listener = GetListenerLocation(PC);

	ScanTimer -= DeltaTime;
	if (ScanTimer <= 0.0f)
	{
		ScanTimer = ScanInterval;
		UpdateStationHums(Listener);

		Fields.Reset();
		for (TActorIterator<AAsteroidField> It(GetWorld()); It; ++It)
		{
			Fields.Add(*It);
		}
	}

	const bool bInField = !bOnFoot && IsInsideAnyField(Listener);

	bool bNearStation = false;
	const float NearRange = GetHumRange() * 0.5f;
	for (const auto& Pair : StationHums)
	{
		const ASpaceStation* Station = Pair.Key.Get();
		if (Station && FVector::DistSquared(Station->GetActorLocation(), Listener) < FMath::Square(NearRange))
		{
			bNearStation = true;
			break;
		}
	}

	if (bInField != bWasInField)
	{
		UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: %s asteroid field"), bInField ? TEXT("entered") : TEXT("left"));
		bWasInField = bInField;
		if (bInField)
		{
			NextCreakIn = FMath::FRandRange(2.0f, CreakMinInterval);
		}
	}
	if (bNearStation != bWasNearStation)
	{
		UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: %s station proximity"), bNearStation ? TEXT("entered") : TEXT("left"));
		bWasNearStation = bNearStation;
	}

	// Space bed: the quietest applicable situation wins, and every change is a fade.
	float SpaceLevel = SpaceBedOpen;
	if (bNearStation) { SpaceLevel = FMath::Min(SpaceLevel, SpaceBedNearStation); }
	if (bInField)     { SpaceLevel = FMath::Min(SpaceLevel, SpaceBedInField); }
	if (bOnFoot)      { SpaceLevel = FMath::Min(SpaceLevel, SpaceBedOnFoot); }
	if (bMap)         { SpaceLevel = FMath::Min(SpaceLevel, SpaceBedMapOpen); }
	SetBedLevel(AdastreaMix::SpaceBedId, SpaceBed, SpaceBedLevel, SpaceLevel);

	SetBedLevel(AdastreaMix::MapRoomToneId, MapRoomTone, MapToneLevel, bMap ? 1.0f : 0.0f);

	UpdateAsteroidCreaks(DeltaTime, Listener, bInField, bOnFoot);
}

// ---------------------------------------------------------------------------
// Mixes
// ---------------------------------------------------------------------------

USoundMix* UAudioMixSubsystem::LoadMix(const TCHAR* Path, TObjectPtr<USoundMix>& Cached) const
{
	if (!Cached)
	{
		Cached = LoadObject<USoundMix>(nullptr, Path, nullptr, LOAD_NoWarn);
		if (!Cached)
		{
			static TSet<FString> Warned;
			if (!Warned.Contains(Path))
			{
				Warned.Add(Path);
				UE_LOG(LogAdastreaMix, Warning, TEXT("AudioMix: sound mix %s is missing (run Tools/create_audio_mixes.py)"), Path);
			}
		}
	}
	return Cached;
}

USoundClass* UAudioMixSubsystem::LoadClass(const TCHAR* Path) const
{
	USoundClass* Class = LoadObject<USoundClass>(nullptr, Path, nullptr, LOAD_NoWarn);
	if (!Class)
	{
		static TSet<FString> Warned;
		if (!Warned.Contains(Path))
		{
			Warned.Add(Path);
			UE_LOG(LogAdastreaMix, Warning, TEXT("AudioMix: sound class %s is missing"), Path);
		}
	}
	return Class;
}

void UAudioMixSubsystem::SetMixActive(const TCHAR* MixPath, TObjectPtr<USoundMix>& Cached, bool& bActive, bool bWant)
{
	if (bActive == bWant)
	{
		return;
	}
	USoundMix* Mix = LoadMix(MixPath, Cached);
	bActive = bWant;
	if (!Mix)
	{
		return;
	}
	if (bWant)
	{
		UGameplayStatics::PushSoundMixModifier(GetWorld(), Mix);
	}
	else
	{
		UGameplayStatics::PopSoundMixModifier(GetWorld(), Mix);
	}
	UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: %s %s"), bWant ? TEXT("push") : TEXT("pop"), *Mix->GetName());
}

void UAudioMixSubsystem::UpdateMixes(bool bInterior, bool bMenu)
{
	SetMixActive(AdastreaMix::InteriorMixPath, InteriorMix, bInteriorMixActive, bInterior);
	SetMixActive(AdastreaMix::MenuDuckMixPath, MenuDuckMix, bMenuDuckActive, bMenu);
}

// ---------------------------------------------------------------------------
// Volume settings
// ---------------------------------------------------------------------------

void UAudioMixSubsystem::ApplyVolumeSettings(float FadeSeconds)
{
	UWorld* World = GetWorld();
	USoundMix* Mix = LoadMix(AdastreaMix::SettingsMixPath, SettingsMix);
	if (!World || !Mix)
	{
		return;
	}
	if (!bSettingsMixPushed)
	{
		UGameplayStatics::PushSoundMixModifier(World, Mix);
		bSettingsMixPushed = true;
	}

	const UAdastreaAudioSettings* Settings = UAdastreaAudioSettings::Get();
	for (EAdastreaVolumeCategory Category : { EAdastreaVolumeCategory::Master, EAdastreaVolumeCategory::SFX, EAdastreaVolumeCategory::UI })
	{
		if (USoundClass* Class = LoadClass(AdastreaMix::ClassPathFor(Category)))
		{
			UGameplayStatics::SetSoundMixClassOverride(World, Mix, Class, Settings->GetVolume(Category), 1.0f, FadeSeconds, /*bApplyToChildren*/ true);
		}
	}
}

void UAudioMixSubsystem::SetCategoryVolume(EAdastreaVolumeCategory Category, float Value)
{
	UAdastreaAudioSettings* Settings = UAdastreaAudioSettings::Get();
	Settings->SetVolume(Category, Value);
	Settings->Save();
	ApplyVolumeSettings(0.1f);
	UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: %s volume -> %.0f%% (saved)"),
		UAdastreaAudioSettings::GetCategoryLabel(Category), Settings->GetVolume(Category) * 100.0f);
}

float UAudioMixSubsystem::GetCategoryVolume(EAdastreaVolumeCategory Category) const
{
	return UAdastreaAudioSettings::Get()->GetVolume(Category);
}

// ---------------------------------------------------------------------------
// Ambience beds
// ---------------------------------------------------------------------------

void UAudioMixSubsystem::SetBedLevel(FName EventId, TObjectPtr<UAudioComponent>& Comp, float& CurrentLevel, float Level)
{
	if (FMath::IsNearlyEqual(Level, CurrentLevel, 0.01f) && (Level <= 0.0f || (Comp && Comp->IsPlaying())))
	{
		return;
	}

	if (!Comp)
	{
		if (Level <= 0.0f || FailedBeds.Contains(EventId))
		{
			CurrentLevel = Level;
			return;
		}
		UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
		Comp = Catalog ? Catalog->SpawnEventAttached(EventId, nullptr, false) : nullptr;
		if (!Comp)
		{
			// The catalog already logged why; don't retry every frame.
			FailedBeds.Add(EventId);
			CurrentLevel = Level;
			return;
		}
	}

	if (Level <= 0.001f)
	{
		Comp->FadeOut(BedFadeSeconds, 0.0f);
	}
	else if (!Comp->IsPlaying())
	{
		Comp->FadeIn(BedFadeSeconds, Level);
	}
	else
	{
		Comp->AdjustVolume(BedFadeSeconds, Level);
	}
	UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: bed %s %.2f -> %.2f (%.1fs fade)"), *EventId.ToString(), CurrentLevel, Level, BedFadeSeconds);
	CurrentLevel = Level;
}

float UAudioMixSubsystem::GetHumRange() const
{
	if (CachedHumRange > 0.0f)
	{
		return CachedHumRange;
	}
	CachedHumRange = DefaultHumRange;

	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
	const UAudioCatalogDataAsset* Data = Catalog ? Catalog->GetCatalog() : nullptr;
	const FAudioCatalogEntry* Entry = Data ? Data->FindEntry(AdastreaMix::StationHumId) : nullptr;
	if (Entry)
	{
		const USoundAttenuation* Attenuation = Entry->Attenuation.LoadSynchronous();
		if (!Attenuation)
		{
			if (const USoundBase* Sound = Entry->Sound.LoadSynchronous())
			{
				Attenuation = Sound->AttenuationSettings;
			}
		}
		if (Attenuation && Attenuation->Attenuation.bAttenuate)
		{
			// Keep the component a little past the point where the attenuation reaches zero,
			// so the despawn is always inaudible.
			CachedHumRange = FMath::Max(Attenuation->Attenuation.GetMaxDimension() * 1.1f, 1000.0f);
		}
	}
	UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: station hum range %.0f cm"), CachedHumRange);
	return CachedHumRange;
}

void UAudioMixSubsystem::UpdateStationHums(const FVector& Listener)
{
	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
	if (!Catalog)
	{
		return;
	}
	const float Range = GetHumRange();
	const float SpawnSq = FMath::Square(Range);
	const float StopSq = FMath::Square(Range * 1.15f);

	for (auto It = StationHums.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !It.Value().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		ASpaceStation* Station = *It;
		if (!IsValid(Station) || !Station->GetRootComponent())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Station->GetActorLocation(), Listener);
		TWeakObjectPtr<UAudioComponent>* Existing = StationHums.Find(Station);
		UAudioComponent* Hum = Existing ? Existing->Get() : nullptr;

		if (DistSq <= SpawnSq)
		{
			if (!Hum)
			{
				Hum = Catalog->SpawnEventAttached(AdastreaMix::StationHumId, Station->GetRootComponent(), false);
				if (!Hum)
				{
					return; // missing event: already logged once by the catalog
				}
				StationHums.Add(Station, Hum);
			}
			if (!Hum->IsPlaying())
			{
				Hum->FadeIn(BedFadeSeconds, 1.0f);
				UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: station hum fade in (%s, %.0f m)"), *Station->GetName(), FMath::Sqrt(DistSq) / 100.0f);
			}
		}
		else if (Hum && Hum->IsPlaying() && DistSq > StopSq)
		{
			Hum->FadeOut(BedFadeSeconds, 0.0f);
			UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: station hum fade out (%s, %.0f m)"), *Station->GetName(), FMath::Sqrt(DistSq) / 100.0f);
		}
	}
}

bool UAudioMixSubsystem::IsInsideAnyField(const FVector& Listener) const
{
	for (const TWeakObjectPtr<AAsteroidField>& Weak : Fields)
	{
		const AAsteroidField* Field = Weak.Get();
		if (Field && FVector::DistSquared(Field->GetActorLocation(), Listener) <= FMath::Square(Field->FieldRadius * FieldRadiusScale))
		{
			return true;
		}
	}
	return false;
}

void UAudioMixSubsystem::UpdateAsteroidCreaks(float DeltaTime, const FVector& Listener, bool bInField, bool bOnFoot)
{
	if (!bInField || bOnFoot)
	{
		return;
	}
	NextCreakIn -= DeltaTime;
	if (NextCreakIn > 0.0f)
	{
		return;
	}
	NextCreakIn = FMath::FRandRange(CreakMinInterval, CreakMaxInterval);

	if (UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this))
	{
		const FVector Where = Listener + FMath::VRand() * FMath::FRandRange(CreakMinDistance, CreakMaxDistance);
		Catalog->PlayEventAtLocation(AdastreaMix::AsteroidCreakId, Where);
		UE_LOG(LogAdastreaMix, Log, TEXT("AudioMix: asteroid creak at %.0f m (next in %.1fs)"),
			FVector::Dist(Where, Listener) / 100.0f, NextCreakIn);
	}
}

void UAudioMixSubsystem::LogState() const
{
	const UAdastreaAudioSettings* Settings = UAdastreaAudioSettings::Get();
	int32 PlayingHums = 0;
	for (const auto& Pair : StationHums)
	{
		if (const UAudioComponent* Hum = Pair.Value.Get())
		{
			PlayingHums += Hum->IsPlaying() ? 1 : 0;
		}
	}
	UE_LOG(LogAdastreaMix, Log,
		TEXT("AudioMix state: Interior=%d MenuDuck=%d Settings=%d | SpaceBed=%.2f (playing=%d) MapTone=%.2f (playing=%d) | hums %d/%d playing, range %.0f cm | InField=%d NearStation=%d | Master=%.0f%% SFX=%.0f%% UI=%.0f%%"),
		bInteriorMixActive, bMenuDuckActive, bSettingsMixPushed,
		SpaceBedLevel, SpaceBed ? SpaceBed->IsPlaying() : 0,
		MapToneLevel, MapRoomTone ? MapRoomTone->IsPlaying() : 0,
		PlayingHums, StationHums.Num(), CachedHumRange,
		bWasInField, bWasNearStation,
		Settings->MasterVolume * 100.0f, Settings->SFXVolume * 100.0f, Settings->UIVolume * 100.0f);
}
