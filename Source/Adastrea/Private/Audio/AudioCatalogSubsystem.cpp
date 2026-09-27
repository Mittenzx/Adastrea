// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/AudioCatalogSubsystem.h"
#include "Audio/AudioCatalogDataAsset.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdastreaAudio, Log, All);

const TCHAR* UAudioCatalogSubsystem::CatalogPath = TEXT("/Game/Audio/DA_AudioCatalog.DA_AudioCatalog");

void UAudioCatalogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Check the package exists first so a missing catalog is one clean warning, not a load error.
	if (FPackageName::DoesPackageExist(TEXT("/Game/Audio/DA_AudioCatalog")))
	{
		Catalog = LoadObject<UAudioCatalogDataAsset>(nullptr, CatalogPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	}

	if (Catalog)
	{
		UE_LOG(LogAdastreaAudio, Log, TEXT("AudioCatalog: loaded %d events from %s"), Catalog->Entries.Num(), CatalogPath);
	}
	else
	{
		bWarnedMissingCatalog = true;
		UE_LOG(LogAdastreaAudio, Warning, TEXT("AudioCatalog: %s not found; all audio events will be silent"), CatalogPath);
	}
}

void UAudioCatalogSubsystem::Deinitialize()
{
	KeepAlive.Empty();
	Catalog = nullptr;
	Super::Deinitialize();
}

UAudioCatalogSubsystem* UAudioCatalogSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UAudioCatalogSubsystem>() : nullptr;
}

void UAudioCatalogSubsystem::WarnOnce(FName EventId, const FString& Reason)
{
	if (WarnedIds.Contains(EventId))
	{
		return;
	}
	WarnedIds.Add(EventId);
	UE_LOG(LogAdastreaAudio, Warning, TEXT("AudioCatalog: event '%s' %s (further calls are silent)"), *EventId.ToString(), *Reason);
}

float UAudioCatalogSubsystem::PickPitch(const FAudioCatalogEntry& Entry)
{
	const float Lo = FMath::Max(0.01f, static_cast<float>(FMath::Min(Entry.PitchRange.X, Entry.PitchRange.Y)));
	const float Hi = FMath::Max(Lo, static_cast<float>(FMath::Max(Entry.PitchRange.X, Entry.PitchRange.Y)));
	return FMath::IsNearlyEqual(Lo, Hi) ? Lo : FMath::FRandRange(Lo, Hi);
}

bool UAudioCatalogSubsystem::HasEvent(FName EventId) const
{
	return Catalog && Catalog->FindEntry(EventId) != nullptr;
}

bool UAudioCatalogSubsystem::ResolveEvent(FName EventId, const FAudioCatalogEntry*& OutEntry, USoundBase*& OutSound)
{
	OutEntry = nullptr;
	OutSound = nullptr;

	if (!Catalog)
	{
		WarnOnce(EventId, TEXT("ignored: no audio catalog loaded"));
		return false;
	}

	OutEntry = Catalog->FindEntry(EventId);
	if (!OutEntry)
	{
		WarnOnce(EventId, TEXT("is not in the audio catalog"));
		return false;
	}

	// Pick the base sound or one of its variations.
	const TSoftObjectPtr<USoundBase>* Pick = &OutEntry->Sound;
	const int32 NumChoices = 1 + OutEntry->Variations.Num();
	if (NumChoices > 1)
	{
		const int32 Index = FMath::RandRange(0, NumChoices - 1);
		if (Index > 0)
		{
			Pick = &OutEntry->Variations[Index - 1];
		}
	}
	if (Pick->IsNull())
	{
		Pick = &OutEntry->Sound;
	}
	if (Pick->IsNull())
	{
		WarnOnce(EventId, TEXT("has no sound assigned"));
		return false;
	}

	OutSound = Pick->LoadSynchronous();
	if (!OutSound)
	{
		WarnOnce(EventId, FString::Printf(TEXT("failed to load sound %s"), *Pick->ToString()));
		return false;
	}
	KeepAlive.Add(OutSound);
	return true;
}

USoundBase* UAudioCatalogSubsystem::GetSound(FName EventId)
{
	const FAudioCatalogEntry* Entry = nullptr;
	USoundBase* Sound = nullptr;
	return ResolveEvent(EventId, Entry, Sound) ? Sound : nullptr;
}

void UAudioCatalogSubsystem::PlayEvent2D(FName EventId)
{
	const FAudioCatalogEntry* Entry = nullptr;
	USoundBase* Sound = nullptr;
	if (!ResolveEvent(EventId, Entry, Sound))
	{
		return;
	}
	if (Entry->bLooping)
	{
		WarnOnce(EventId, TEXT("is a loop; use SpawnEventAttached so it can be stopped"));
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// CreateSound2D (instead of PlaySound2D) so the catalog's sound class can be applied.
	if (UAudioComponent* Comp = UGameplayStatics::CreateSound2D(World, Sound, Entry->Volume, PickPitch(*Entry), 0.0f, nullptr, false, /*bAutoDestroy*/ true))
	{
		if (USoundClass* SoundClass = Entry->SoundClass.LoadSynchronous())
		{
			Comp->SoundClassOverride = SoundClass;
		}
		Comp->Play();
	}
}

void UAudioCatalogSubsystem::PlayEventAtLocation(FName EventId, FVector Location)
{
	const FAudioCatalogEntry* Entry = nullptr;
	USoundBase* Sound = nullptr;
	if (!ResolveEvent(EventId, Entry, Sound))
	{
		return;
	}
	if (Entry->bLooping)
	{
		WarnOnce(EventId, TEXT("is a loop; use SpawnEventAttached so it can be stopped"));
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USoundAttenuation* Attenuation = Entry->Attenuation.LoadSynchronous();
	if (UAudioComponent* Comp = UGameplayStatics::SpawnSoundAtLocation(World, Sound, Location, FRotator::ZeroRotator,
		Entry->Volume, PickPitch(*Entry), 0.0f, Attenuation, nullptr, /*bAutoDestroy*/ true))
	{
		if (USoundClass* SoundClass = Entry->SoundClass.LoadSynchronous())
		{
			Comp->SoundClassOverride = SoundClass;
		}
	}
}

UAudioComponent* UAudioCatalogSubsystem::SpawnEventAttached(FName EventId, USceneComponent* Parent, bool bAutoPlay)
{
	const FAudioCatalogEntry* Entry = nullptr;
	USoundBase* Sound = nullptr;
	if (!ResolveEvent(EventId, Entry, Sound))
	{
		return nullptr;
	}

	UWorld* World = Parent ? Parent->GetWorld() : GetWorld();
	if (!World)
	{
		return nullptr;
	}

	USoundClass* SoundClass = Entry->SoundClass.LoadSynchronous();
	UAudioComponent* Comp = nullptr;

	if (!Parent)
	{
		// Non-spatial: a persistent 2D component the caller owns.
		Comp = UGameplayStatics::CreateSound2D(World, Sound, Entry->Volume, PickPitch(*Entry), 0.0f, nullptr, false, /*bAutoDestroy*/ false);
		if (!Comp)
		{
			return nullptr;
		}
	}
	else
	{
		AActor* Owner = Parent->GetOwner();
		UObject* Outer = Owner ? static_cast<UObject*>(Owner) : static_cast<UObject*>(Parent);
		Comp = NewObject<UAudioComponent>(Outer, NAME_None, RF_Transient);
		Comp->bAutoActivate = false;
		Comp->bAutoDestroy = false;
		Comp->bStopWhenOwnerDestroyed = true;
		Comp->SetSound(Sound);
		Comp->SetVolumeMultiplier(Entry->Volume);
		Comp->SetPitchMultiplier(PickPitch(*Entry));
		if (USoundAttenuation* Attenuation = Entry->Attenuation.LoadSynchronous())
		{
			Comp->AttenuationSettings = Attenuation;
		}
		Comp->SetupAttachment(Parent);
		if (Owner)
		{
			Comp->RegisterComponent();
		}
		else
		{
			Comp->RegisterComponentWithWorld(World);
		}
	}

	if (SoundClass)
	{
		Comp->SoundClassOverride = SoundClass;
	}
	if (bAutoPlay)
	{
		Comp->Play();
	}
	return Comp;
}
