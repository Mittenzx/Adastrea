// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/ShipEngineAudioComponent.h"
#include "Audio/AudioCatalogSubsystem.h"
#include "Audio/AudioCatalogDataAsset.h"
#include "Audio/ShipEngineVoiceSubsystem.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundWave.h"

DEFINE_LOG_CATEGORY(LogAdastreaEngineVoice);

namespace EngineVoice
{
	static const FName WhineId(TEXT("Engine.Whine"));
	static const FName BoostId(TEXT("Engine.Boost"));
	static const FName BoostStartId(TEXT("Engine.BoostStart"));
	static const FName SpoolUpId(TEXT("Engine.SpoolUp"));
	static const FName SpoolDownId(TEXT("Engine.SpoolDown"));
	static const FName IdleId(TEXT("Engine.Idle"));

	static constexpr float MinPitch = 0.7f;
	static constexpr float MaxPitch = 1.4f;
	static constexpr float OpenLowPassHz = 20000.0f;
	static constexpr float ModeDebounceSeconds = 0.1f;
	static constexpr float LayerFadeSeconds = 0.3f;

	static const TCHAR* LayerNames[] = { TEXT("Low"), TEXT("High"), TEXT("Whine"), TEXT("Boost"), TEXT("Idle") };

	static const TCHAR* ModeName(EShipEngineVoiceMode Mode)
	{
		switch (Mode)
		{
		case EShipEngineVoiceMode::Player: return TEXT("Player");
		case EShipEngineVoiceMode::PlayerMuffled: return TEXT("Muffled");
		case EShipEngineVoiceMode::AI: return TEXT("AI");
		default: return TEXT("Off");
		}
	}
}

UShipEngineAudioComponent::UShipEngineAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// After the ship's own Tick has moved it, so speed/throttle are this frame's.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	AIAttenuation = TSoftObjectPtr<USoundAttenuation>(FSoftObjectPath(TEXT("/Game/Audio/Attenuation/ATT_AIEngine.ATT_AIEngine")));
}

// ====================
// VOICE DERIVATION
// ====================

float UShipEngineAudioComponent::ComputeSizeScore(float HullStrength, float CargoCapacity)
{
	const UShipEngineAudioComponent* Tuning = GetDefault<UShipEngineAudioComponent>();
	return Tuning->SizeHullWeight * FMath::Loge(FMath::Max(HullStrength, 1.0f))
		+ Tuning->SizeCargoWeight * FMath::Loge(1.0f + FMath::Max(CargoCapacity, 0.0f));
}

float UShipEngineAudioComponent::ComputeAgilityScore(float Acceleration, float Maneuverability)
{
	return FMath::Loge(FMath::Max(Acceleration * Maneuverability, 1.0f));
}

FShipEngineVoice UShipEngineAudioComponent::DeriveVoice(const USpaceshipDataAsset* Data)
{
	const UShipEngineAudioComponent* Tuning = GetDefault<UShipEngineAudioComponent>();
	FShipEngineVoice Result;
	if (!Data)
	{
		// No stats: a mid-sized, mid-agility voice.
		Result.BasePitch = 0.5f * (Tuning->FamilyPitchHigh + Tuning->FamilyPitchLow);
		return Result;
	}

	const float SizeScore = ComputeSizeScore(Data->HullStrength, Data->CargoCapacity);
	const float AgilityScore = ComputeAgilityScore(Data->Acceleration, static_cast<float>(Data->Maneuverability));
	Result.Size = FMath::Clamp((SizeScore - Tuning->SizeScoreMin) / FMath::Max(Tuning->SizeScoreMax - Tuning->SizeScoreMin, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	Result.Agility = FMath::Clamp((AgilityScore - Tuning->AgilityScoreMin) / FMath::Max(Tuning->AgilityScoreMax - Tuning->AgilityScoreMin, KINDA_SMALL_NUMBER), 0.0f, 1.0f);

	// Four equal size bands pick the family; the position inside the band picks the pitch
	// (bottom of the band = smallest ship of that family = highest pitch).
	const float Scaled = Result.Size * 4.0f;
	const int32 FamilyIndex = FMath::Clamp(FMath::FloorToInt32(Scaled), 0, 3);
	const float InBand = FMath::Clamp(Scaled - static_cast<float>(FamilyIndex), 0.0f, 1.0f);
	Result.Family = static_cast<EShipEngineFamily>(FamilyIndex);
	Result.BasePitch = FMath::Lerp(Tuning->FamilyPitchHigh, Tuning->FamilyPitchLow, InBand);
	Result.WhineAmount = FMath::Pow(Result.Agility, Tuning->WhineExponent);

	const FShipEngineSoundProfile& Profile = Data->EngineSoundProfile;
	if (Profile.bOverride)
	{
		Result.Family = Profile.Family;
		Result.BasePitch = 0.5f * (Tuning->FamilyPitchHigh + Tuning->FamilyPitchLow) + Profile.PitchOffset;
		Result.WhineAmount = FMath::Clamp(Profile.WhineAmount, 0.0f, 1.0f);
		Result.bFromOverride = true;
	}

	Result.BasePitch = FMath::Clamp(Result.BasePitch, EngineVoice::MinPitch, EngineVoice::MaxPitch);
	return Result;
}

FName UShipEngineAudioComponent::FamilyEventId(EShipEngineFamily Family, bool bHighRevs)
{
	return FName(*FString::Printf(TEXT("Engine.%s.%s"), FamilyName(Family), bHighRevs ? TEXT("High") : TEXT("Low")));
}

const TCHAR* UShipEngineAudioComponent::FamilyName(EShipEngineFamily Family)
{
	switch (Family)
	{
	case EShipEngineFamily::Light: return TEXT("Light");
	case EShipEngineFamily::Medium: return TEXT("Medium");
	case EShipEngineFamily::Heavy: return TEXT("Heavy");
	case EShipEngineFamily::Capital: return TEXT("Capital");
	default: return TEXT("Medium");
	}
}

// ====================
// LIFECYCLE
// ====================

ASpaceship* UShipEngineAudioComponent::GetShip() const
{
	return Cast<ASpaceship>(GetOwner());
}

void UShipEngineAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	MeasureHull();
	BuildLayers();

	if (const ASpaceship* Ship = GetShip())
	{
		SpoolReferenceThrottle = FMath::Clamp(Ship->ThrottlePercentage / 100.0f, 0.0f, 1.0f);
		SmoothThrottle = SpoolReferenceThrottle;
		bWasBoosting = Ship->bBoostActive;
	}

	if (UWorld* World = GetWorld())
	{
		if (UShipEngineVoiceSubsystem* Voices = World->GetSubsystem<UShipEngineVoiceSubsystem>())
		{
			Voices->Register(this);
		}
	}
}

void UShipEngineAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UShipEngineVoiceSubsystem* Voices = World->GetSubsystem<UShipEngineVoiceSubsystem>())
		{
			Voices->Unregister(this);
		}
	}
	DestroyLayers();
	Super::EndPlay(EndPlayReason);
}

void UShipEngineAudioComponent::RefreshVoice()
{
	const EShipEngineVoiceMode PreviousMode = Mode;
	BuildLayers();
	if (PreviousMode != EShipEngineVoiceMode::Off)
	{
		ApplyMode(PreviousMode);
	}
}

void UShipEngineAudioComponent::MeasureHull()
{
	ASpaceship* Ship = GetShip();
	UStaticMeshComponent* Mesh = Ship ? Ship->ShipMeshComponent.Get() : nullptr;
	if (Mesh && Mesh->GetStaticMesh())
	{
		// Sit at the centre of the mesh's local bounds, aligned with the mesh, so the AI
		// attenuation box (extents = hull + range) wraps the actual hull.
		const FBox LocalBox = Mesh->GetStaticMesh()->GetBoundingBox();
		if (GetAttachParent() != Mesh)
		{
			AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
		}
		SetRelativeLocationAndRotation(LocalBox.GetCenter(), FRotator::ZeroRotator);
		HullExtent = LocalBox.GetExtent() * Mesh->GetComponentScale().GetAbs();
	}
	else if (Ship)
	{
		FVector Origin, Extent;
		Ship->GetActorBounds(true, Origin, Extent);
		if (!Extent.IsNearlyZero())
		{
			SetWorldLocation(Origin);
			HullExtent = Extent;
		}
	}
	HullExtent = HullExtent.ComponentMax(FVector(100.0f));
}

const FAudioCatalogEntry* UShipEngineAudioComponent::FindCatalogEntry(FName EventId) const
{
	const UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
	const UAudioCatalogDataAsset* Data = Catalog ? Catalog->GetCatalog() : nullptr;
	return Data ? Data->FindEntry(EventId) : nullptr;
}

UAudioComponent* UShipEngineAudioComponent::MakeAudio(const TCHAR* LayerName, USoundBase* Sound)
{
	AActor* Owner = GetOwner();
	const FName Name = MakeUniqueObjectName(Owner, UAudioComponent::StaticClass(), FName(*FString::Printf(TEXT("EngineVoice_%s"), LayerName)));
	UAudioComponent* Audio = NewObject<UAudioComponent>(Owner, Name, RF_Transient);
	Audio->bAutoActivate = false;
	Audio->bAutoDestroy = false;
	Audio->bStopWhenOwnerDestroyed = true;
	Audio->SetupAttachment(this);
	Audio->SetSound(Sound);
	Audio->RegisterComponent();
	return Audio;
}

UAudioComponent* UShipEngineAudioComponent::SpawnLayer(int32 Index, FName EventId, const TSoftObjectPtr<USoundBase>& Custom)
{
	LayerEntryVolume[Index] = 1.0f;
	LayerEntryPitch[Index] = 1.0f;
	const FAudioCatalogEntry* Entry = FindCatalogEntry(EventId);

	UAudioComponent* Audio = nullptr;
	if (!Custom.IsNull())
	{
		// A per-ship replacement sound; still routed through the catalog entry's sound class.
		if (USoundBase* CustomSound = Custom.LoadSynchronous())
		{
			Audio = MakeAudio(EngineVoice::LayerNames[Index], CustomSound);
			if (Entry)
			{
				Audio->SoundClassOverride = Entry->SoundClass.LoadSynchronous();
			}
		}
		else
		{
			UE_LOG(LogAdastreaEngineVoice, Warning, TEXT("%s: custom sound %s failed to load; using %s"),
				*GetNameSafe(GetOwner()), *Custom.ToString(), *EventId.ToString());
		}
	}
	if (!Audio)
	{
		UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
		Audio = Catalog ? Catalog->SpawnEventAttached(EventId, this, /*bAutoPlay*/ false) : nullptr;
		if (Audio)
		{
			// The catalog applied the entry's volume/pitch; keep them as factors under our modulation.
			LayerEntryVolume[Index] = Audio->VolumeMultiplier;
			LayerEntryPitch[Index] = Audio->PitchMultiplier;
		}
	}
	if (Audio)
	{
		Audio->SetVolumeMultiplier(0.0f);
		if (const USoundWave* Wave = Cast<USoundWave>(Audio->Sound))
		{
			if (!Wave->bLooping)
			{
				UE_LOG(LogAdastreaEngineVoice, Warning, TEXT("%s: %s layer sound %s is not set to loop"),
					*GetNameSafe(GetOwner()), EngineVoice::LayerNames[Index], *Wave->GetName());
			}
		}
	}
	return Audio;
}

void UShipEngineAudioComponent::BuildLayers()
{
	DestroyLayers();

	ASpaceship* Ship = GetShip();
	AActor* Owner = GetOwner();
	if (!Ship || !Owner)
	{
		return;
	}

	const USpaceshipDataAsset* Data = Ship->ShipDataAsset;
	Voice = DeriveVoice(Data);
	const FShipEngineSoundProfile EmptyProfile;
	const FShipEngineSoundProfile& Profile = Data ? Data->EngineSoundProfile : EmptyProfile;

	const FName LayerIds[NumLayers] = {
		FamilyEventId(Voice.Family, false),
		FamilyEventId(Voice.Family, true),
		EngineVoice::WhineId,
		EngineVoice::BoostId,
		EngineVoice::IdleId };
	const TSoftObjectPtr<USoundBase>* LayerCustom[NumLayers] = {
		&Profile.LowRevsSound, &Profile.HighRevsSound, &Profile.WhineSound, &Profile.BoostSound, &Profile.IdleSound };

	int32 Resolved = 0;
	Layers.SetNum(NumLayers);
	for (int32 Index = 0; Index < NumLayers; ++Index)
	{
		Layers[Index] = SpawnLayer(Index, LayerIds[Index], *LayerCustom[Index]);
		Resolved += Layers[Index] ? 1 : 0;
	}
	OneShot = MakeAudio(TEXT("OneShot"), nullptr);

	if (Resolved == 0)
	{
		UE_LOG(LogAdastreaEngineVoice, Warning, TEXT("%s: no engine sounds resolved (is the audio catalog loaded?); engine is silent"), *Owner->GetName());
	}
	UE_LOG(LogAdastreaEngineVoice, Log, TEXT("%s: voice %s pitch %.2f whine %.2f (size %.2f, agility %.2f%s), %d/%d layers"),
		*Owner->GetName(), FamilyName(Voice.Family), Voice.BasePitch, Voice.WhineAmount, Voice.Size, Voice.Agility,
		Voice.bFromOverride ? TEXT(", override") : TEXT(""), Resolved, static_cast<int32>(NumLayers));

	Mode = EShipEngineVoiceMode::Off;
	bLayersPlaying = false;
}

void UShipEngineAudioComponent::DestroyLayers()
{
	for (UAudioComponent* Audio : Layers)
	{
		if (Audio)
		{
			Audio->Stop();
			Audio->DestroyComponent();
		}
	}
	Layers.Reset();
	if (OneShot)
	{
		OneShot->Stop();
		OneShot->DestroyComponent();
		OneShot = nullptr;
	}
	bLayersPlaying = false;
}

// ====================
// MODE (player / muffled / AI)
// ====================

EShipEngineVoiceMode UShipEngineAudioComponent::ResolveMode() const
{
	const ASpaceship* Ship = GetShip();
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (Ship && Ship->IsWrecked())
	{
		return EShipEngineVoiceMode::Off;   // dead engines
	}
	if (Ship && PC)
	{
		if (PC->GetPawn() == Ship)
		{
			return EShipEngineVoiceMode::Player;
		}
		if (const AAdastreaPlayerController* AdastreaPC = Cast<AAdastreaPlayerController>(PC))
		{
			if (AdastreaPC->GetShipLeftOnFoot() == Ship)
			{
				return EShipEngineVoiceMode::PlayerMuffled;
			}
		}
	}
	return EShipEngineVoiceMode::AI;
}

FSoundAttenuationSettings UShipEngineAudioComponent::MakeAttenuation(bool bSpatial) const
{
	FSoundAttenuationSettings Settings;
	if (!bSpatial)
	{
		// The player's own engine: no distance falloff, no panning. (The on-foot avatar walks
		// interiors far from the ship, so distance must not matter.)
		Settings.bAttenuate = false;
		Settings.bSpatialize = false;
		Settings.bAttenuateWithLPF = false;
		return Settings;
	}

	if (const USoundAttenuation* Base = AIAttenuation.LoadSynchronous())
	{
		Settings = Base->Attenuation;
	}
	else
	{
		static bool bWarned = false;
		if (!bWarned)
		{
			bWarned = true;
			UE_LOG(LogAdastreaEngineVoice, Warning, TEXT("AI engine attenuation %s not found; using built-in linear falloff"), *AIAttenuation.ToString());
		}
	}

	// Range counts from the hull, not the ship's centre: a box around the mesh bounds,
	// full volume within AIFullVolumeRange of it, silent at AISilentRange.
	Settings.bAttenuate = true;
	Settings.bSpatialize = true;
	Settings.AttenuationShape = EAttenuationShape::Box;
	Settings.AttenuationShapeExtents = HullExtent + FVector(AIFullVolumeRange);
	Settings.FalloffDistance = FMath::Max(AISilentRange - AIFullVolumeRange, 1.0f);
	return Settings;
}

void UShipEngineAudioComponent::ApplyMode(EShipEngineVoiceMode NewMode)
{
	const bool bWasSpatial = (Mode == EShipEngineVoiceMode::AI);
	const bool bSpatial = (NewMode == EShipEngineVoiceMode::AI);
	const bool bSpatialChanged = (Mode == EShipEngineVoiceMode::Off) || (bWasSpatial != bSpatial);

	UE_LOG(LogAdastreaEngineVoice, Log, TEXT("%s: engine voice %s -> %s"),
		*GetNameSafe(GetOwner()), EngineVoice::ModeName(Mode), EngineVoice::ModeName(NewMode));
	Mode = NewMode;
	PendingMode = NewMode;
	PendingModeTime = 0.0f;

	if (!bSpatialChanged)
	{
		// Player <-> Muffled: same 2D setup; the low-pass and volume glide in TickComponent.
		return;
	}

	// 2D <-> 3D can't be switched on a playing sound cleanly: stop, reconfigure, and let
	// TickComponent fade the layers back in if this mode should be heard.
	if (bLayersPlaying)
	{
		for (UAudioComponent* Audio : Layers)
		{
			if (Audio)
			{
				Audio->Stop();
			}
		}
		bLayersPlaying = false;
	}

	const FSoundAttenuationSettings Attenuation = MakeAttenuation(bSpatial);
	auto Configure = [&](UAudioComponent* Audio)
	{
		if (!Audio)
		{
			return;
		}
		Audio->bAllowSpatialization = bSpatial;
		Audio->AdjustAttenuation(Attenuation);
		Audio->SetLowPassFilterEnabled(!bSpatial);
		Audio->SetLowPassFilterFrequency(EngineVoice::OpenLowPassHz);
	};
	for (UAudioComponent* Audio : Layers)
	{
		Configure(Audio);
	}
	Configure(OneShot);

	if (!bSpatial)
	{
		bAIVoiceGranted = false;
		SmoothMuffle = (NewMode == EShipEngineVoiceMode::PlayerMuffled) ? 1.0f : 0.0f;
	}
}

void UShipEngineAudioComponent::SetAIVoiceGranted(bool bGranted)
{
	if (bAIVoiceGranted != bGranted)
	{
		bAIVoiceGranted = bGranted;
		UE_LOG(LogAdastreaEngineVoice, Verbose, TEXT("%s: AI engine voice %s"), *GetNameSafe(GetOwner()), bGranted ? TEXT("granted") : TEXT("released"));
	}
}

bool UShipEngineAudioComponent::ShouldLayersPlay() const
{
	switch (Mode)
	{
	case EShipEngineVoiceMode::Player:
	case EShipEngineVoiceMode::PlayerMuffled:
		return true;
	case EShipEngineVoiceMode::AI:
		return bAIVoiceGranted;
	default:
		return false;
	}
}

void UShipEngineAudioComponent::StartLayers()
{
	for (UAudioComponent* Audio : Layers)
	{
		if (Audio)
		{
			Audio->FadeIn(EngineVoice::LayerFadeSeconds, 1.0f);
		}
	}
	bLayersPlaying = true;
}

void UShipEngineAudioComponent::StopLayers()
{
	for (UAudioComponent* Audio : Layers)
	{
		if (Audio)
		{
			Audio->FadeOut(EngineVoice::LayerFadeSeconds, 0.0f);
		}
	}
	bLayersPlaying = false;
}

float UShipEngineAudioComponent::GetDistanceToHull(const FVector& WorldPoint) const
{
	const FVector Local = GetComponentTransform().InverseTransformPositionNoScale(WorldPoint);
	const FVector Clamped(
		FMath::Clamp(Local.X, -HullExtent.X, HullExtent.X),
		FMath::Clamp(Local.Y, -HullExtent.Y, HullExtent.Y),
		FMath::Clamp(Local.Z, -HullExtent.Z, HullExtent.Z));
	return FVector::Dist(Local, Clamped);
}

// ====================
// RUNTIME
// ====================

void UShipEngineAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ASpaceship* Ship = GetShip();
	if (!Ship || Layers.Num() == 0)
	{
		return;
	}

	// --- Mode (debounced) ---
	const EShipEngineVoiceMode Desired = ResolveMode();
	if (Desired != Mode)
	{
		if (Desired != PendingMode)
		{
			PendingMode = Desired;
			PendingModeTime = 0.0f;
		}
		PendingModeTime += DeltaTime;
		if (Mode == EShipEngineVoiceMode::Off || PendingModeTime >= EngineVoice::ModeDebounceSeconds)
		{
			ApplyMode(Desired);
		}
	}
	else
	{
		PendingMode = Mode;
		PendingModeTime = 0.0f;
	}

	// --- Flight state -> smoothed drivers ---
	const float Throttle01 = FMath::Clamp(Ship->ThrottlePercentage / 100.0f, 0.0f, 1.0f);
	const bool bBoost = Ship->bBoostActive;
	float RevMaxSpeed = Ship->GetEffectiveMaxSpeed();
	if (bBoost && Ship->BoostMultiplier > KINDA_SMALL_NUMBER)
	{
		// Revs are measured against the un-boosted max, so boosting pins them at the top.
		RevMaxSpeed /= Ship->BoostMultiplier;
	}
	const float Speed01 = FMath::Clamp(Ship->GetCurrentSpeed() / FMath::Max(RevMaxSpeed, 1.0f), 0.0f, 1.0f);

	SmoothThrottle = FMath::FInterpTo(SmoothThrottle, Throttle01, DeltaTime, ThrottleInterpSpeed);
	SmoothRev = FMath::FInterpTo(SmoothRev, Speed01, DeltaTime, RevInterpSpeed);
	SmoothBoost = FMath::FInterpTo(SmoothBoost, bBoost ? 1.0f : 0.0f, DeltaTime, BoostInterpSpeed);
	SmoothMuffle = FMath::FInterpTo(SmoothMuffle, Mode == EShipEngineVoiceMode::PlayerMuffled ? 1.0f : 0.0f, DeltaTime, MuffleInterpSpeed);

	switch (Mode)
	{
	case EShipEngineVoiceMode::AI:
		MasterVolume = PlayerVolume * AIVolumeScale;
		break;
	case EShipEngineVoiceMode::Player:
	case EShipEngineVoiceMode::PlayerMuffled:
		MasterVolume = PlayerVolume * FMath::Lerp(1.0f, MuffledVolumeScale, SmoothMuffle);
		break;
	default:
		MasterVolume = 0.0f;
		break;
	}

	// --- Start / stop ---
	const bool bShouldPlay = ShouldLayersPlay();
	if (bShouldPlay && !bLayersPlaying)
	{
		StartLayers();
	}
	else if (!bShouldPlay && bLayersPlaying)
	{
		StopLayers();
	}

	UpdateSpoolAndBoostEvents(Throttle01, bBoost, DeltaTime);

	if (!bLayersPlaying)
	{
		return;
	}

	// --- Levels ---
	// Body follows the throttle; the speed fraction crossfades low -> high revs (equal power).
	const float Body = FMath::Pow(SmoothThrottle, 0.8f);
	const float RevAngle = SmoothRev * HALF_PI;
	OutLayerVolume[Low] = Body * FMath::Cos(RevAngle);
	OutLayerVolume[High] = Body * FMath::Sin(RevAngle);
	OutLayerVolume[Whine] = Voice.WhineAmount * (0.2f + 0.8f * SmoothThrottle) * (0.6f + 0.4f * SmoothRev);
	OutLayerVolume[Boost] = BoostLevel * SmoothBoost;
	// A faint idle hum is always there (never fully silent), receding as the throttle opens.
	OutLayerVolume[Idle] = IdleLevel * (1.0f - 0.8f * SmoothThrottle);

	OutPitch = FMath::Clamp(Voice.BasePitch * (1.0f + RevPitchRise * SmoothRev + BoostPitchRise * SmoothBoost),
		EngineVoice::MinPitch, EngineVoice::MaxPitch);
	const float IdlePitch = FMath::Clamp(Voice.BasePitch, EngineVoice::MinPitch, EngineVoice::MaxPitch);

	// Muffle glides in log-frequency so it sounds even.
	OutLowPassHz = FMath::Exp(FMath::Lerp(FMath::Loge(EngineVoice::OpenLowPassHz), FMath::Loge(FMath::Max(MuffledLowPassHz, 20.0f)), SmoothMuffle));
	const bool bUseLowPass = (Mode != EShipEngineVoiceMode::AI);

	for (int32 Index = 0; Index < NumLayers; ++Index)
	{
		UAudioComponent* Audio = Layers.IsValidIndex(Index) ? Layers[Index].Get() : nullptr;
		if (!Audio)
		{
			continue;
		}
		Audio->SetVolumeMultiplier(FMath::Max(OutLayerVolume[Index] * MasterVolume * LayerEntryVolume[Index], 0.0f));
		Audio->SetPitchMultiplier((Index == Idle ? IdlePitch : OutPitch) * LayerEntryPitch[Index]);
		if (bUseLowPass)
		{
			Audio->SetLowPassFilterFrequency(OutLowPassHz);
		}
	}
	if (OneShot && bUseLowPass)
	{
		OneShot->SetLowPassFilterFrequency(OutLowPassHz);
	}
}

void UShipEngineAudioComponent::UpdateSpoolAndBoostEvents(float Throttle01, bool bBoost, float DeltaTime)
{
	SpoolCooldownLeft = FMath::Max(SpoolCooldownLeft - DeltaTime, 0.0f);

	const bool bAI = (Mode == EShipEngineVoiceMode::AI);
	const float Threshold = bAI ? AISpoolThreshold : PlayerSpoolThreshold;
	const float Delta = Throttle01 - SpoolReferenceThrottle;
	if (FMath::Abs(Delta) >= Threshold)
	{
		if (bLayersPlaying && SpoolCooldownLeft <= 0.0f)
		{
			PlayOneShot(Delta > 0.0f ? EngineVoice::SpoolUpId : EngineVoice::SpoolDownId);
			SpoolCooldownLeft = bAI ? AISpoolCooldown : PlayerSpoolCooldown;
		}
		SpoolReferenceThrottle = Throttle01;
	}

	if (bBoost && !bWasBoosting && bLayersPlaying)
	{
		PlayOneShot(EngineVoice::BoostStartId);
	}
	bWasBoosting = bBoost;
}

void UShipEngineAudioComponent::PlayOneShot(FName EventId)
{
	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(this);
	const FAudioCatalogEntry* Entry = FindCatalogEntry(EventId);
	USoundBase* Sound = Catalog ? Catalog->GetSound(EventId) : nullptr;
	if (!OneShot || !Sound || !Entry)
	{
		return;
	}
	const float PitchLo = FMath::Max(0.01f, static_cast<float>(FMath::Min(Entry->PitchRange.X, Entry->PitchRange.Y)));
	const float PitchHi = FMath::Max(PitchLo, static_cast<float>(FMath::Max(Entry->PitchRange.X, Entry->PitchRange.Y)));
	const float EntryPitch = FMath::FRandRange(PitchLo, PitchHi);
	OneShot->SetSound(Sound);
	OneShot->SoundClassOverride = Entry->SoundClass.LoadSynchronous();
	OneShot->SetPitchMultiplier(FMath::Clamp(Voice.BasePitch, EngineVoice::MinPitch, EngineVoice::MaxPitch) * EntryPitch);
	OneShot->SetVolumeMultiplier(OneShotLevel * MasterVolume * Entry->Volume);
	OneShot->Play();
	LastOneShot = EventId;
	UE_LOG(LogAdastreaEngineVoice, Verbose, TEXT("%s: one-shot %s"), *GetNameSafe(GetOwner()), *EventId.ToString());
}

FString UShipEngineAudioComponent::GetDebugLine(float HullDistanceCm) const
{
	const ASpaceship* Ship = GetShip();
	const float Throttle = Ship ? Ship->ThrottlePercentage : 0.0f;
	const float Speed = Ship ? Ship->GetCurrentSpeed() : 0.0f;
	const bool bBoost = Ship && Ship->bBoostActive;
	FString Line = FString::Printf(
		TEXT("%s [%s%s] %s base %.2f whine %.2f | thr %3.0f%% spd %5.0f boost %d | rev %.2f pitch %.3f | vol L %.2f H %.2f W %.2f B %.2f I %.2f x%.2f | lpf %5.0f"),
		*GetNameSafe(GetOwner()), EngineVoice::ModeName(Mode), bLayersPlaying ? TEXT("") : TEXT(", silent"),
		FamilyName(Voice.Family), Voice.BasePitch, Voice.WhineAmount,
		Throttle, Speed, bBoost ? 1 : 0,
		SmoothRev, OutPitch,
		OutLayerVolume[Low], OutLayerVolume[High], OutLayerVolume[Whine], OutLayerVolume[Boost], OutLayerVolume[Idle], MasterVolume,
		Mode == EShipEngineVoiceMode::AI ? EngineVoice::OpenLowPassHz : OutLowPassHz);
	if (HullDistanceCm >= 0.0f)
	{
		Line += FString::Printf(TEXT(" | hull %.1f m"), HullDistanceCm / 100.0f);
	}
	if (!LastOneShot.IsNone())
	{
		Line += FString::Printf(TEXT(" | last %s"), *LastOneShot.ToString());
	}
	return Line;
}
