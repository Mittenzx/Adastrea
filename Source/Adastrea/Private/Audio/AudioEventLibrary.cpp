// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/AudioEventLibrary.h"
#include "Audio/AudioCatalogSubsystem.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Ships/SpaceshipDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogAdastreaAudioEvents, Log, All);

namespace AdastreaAudioEvents
{
	static TAutoConsoleVariable<int32> CVarAudioEventLog(
		TEXT("adastrea.AudioEventLog"),
		0,
		TEXT("Log every gameplay audio event as it plays (ID, mode, whether the catalog has it). 0 = off (default), 1 = on."),
		ECVF_Default);

	/** Last time each event ID actually played (FPlatformTime seconds, so it keeps counting while paused). */
	static TMap<FName, double> LastPlayed;

	/** Last time any primary (non-secondary) event played. */
	static double LastPrimaryTime = -1.0e9;

	/** Live FScopedMute count. */
	static int32 MuteDepth = 0;

	// Roster range for the size factor, from the real DA_* ship data assets (hull + cargo):
	// DA_Fighter_ViperInterceptor ~630 is the smallest, DA_Transport_BehemothFreighter ~28000
	// the largest. With HeavyShipSizeFactor 0.5 the split falls at ~4100: Fighter, Mk1,
	// Patrol, Corvette, Frigate, Starliner and Science puff; Excavator, Gunship, Cruiser,
	// Salvage, Carrier, Merchant, Sovereign and the colony/freight haulers groan.
	static constexpr float SizeLogMin = 2.77815f; // log10(600)
	static constexpr float SizeLogMax = 4.44716f; // log10(28000)
}

bool UAudioEventLibrary::IsMuted()
{
	return AdastreaAudioEvents::MuteDepth > 0;
}

UAudioEventLibrary::FScopedMute::FScopedMute()
{
	++AdastreaAudioEvents::MuteDepth;
}

UAudioEventLibrary::FScopedMute::~FScopedMute()
{
	AdastreaAudioEvents::MuteDepth = FMath::Max(0, AdastreaAudioEvents::MuteDepth - 1);
}

bool UAudioEventLibrary::PassesGate(FName EventId, float MinInterval)
{
	if (EventId.IsNone() || IsMuted())
	{
		return false;
	}
	const double Now = FPlatformTime::Seconds();
	if (MinInterval > 0.0f)
	{
		if (const double* Last = AdastreaAudioEvents::LastPlayed.Find(EventId))
		{
			if (Now - *Last < MinInterval)
			{
				return false;
			}
		}
	}
	AdastreaAudioEvents::LastPlayed.Add(EventId, Now);
	return true;
}

void UAudioEventLibrary::LogEvent(const UObject* WorldContext, FName EventId, const TCHAR* Mode)
{
	if (AdastreaAudioEvents::CVarAudioEventLog.GetValueOnGameThread() <= 0)
	{
		return;
	}
	const UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(WorldContext);
	const TCHAR* Known = !Catalog ? TEXT("NO-SUBSYSTEM") : (Catalog->HasEvent(EventId) ? TEXT("ok") : TEXT("MISSING"));
	UE_LOG(LogAdastreaAudioEvents, Log, TEXT("AudioEvent %-24s [%s] catalog=%s from=%s"),
		*EventId.ToString(), Mode, Known, *GetNameSafe(WorldContext));
}

bool UAudioEventLibrary::PlayEvent2D(const UObject* WorldContext, FName EventId, float MinInterval)
{
	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(WorldContext);
	if (!Catalog || !PassesGate(EventId, MinInterval))
	{
		return false;
	}
	AdastreaAudioEvents::LastPrimaryTime = FPlatformTime::Seconds();
	LogEvent(WorldContext, EventId, TEXT("2D"));
	Catalog->PlayEvent2D(EventId);
	return true;
}

bool UAudioEventLibrary::PlayEventAtLocation(const UObject* WorldContext, FName EventId, FVector Location, float MinInterval)
{
	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(WorldContext);
	if (!Catalog || !PassesGate(EventId, MinInterval))
	{
		return false;
	}
	AdastreaAudioEvents::LastPrimaryTime = FPlatformTime::Seconds();
	LogEvent(WorldContext, EventId, TEXT("3D"));
	Catalog->PlayEventAtLocation(EventId, Location);
	return true;
}

void UAudioEventLibrary::PlaySecondary2D(const UObject* WorldContext, FName EventId, float MinInterval)
{
	if (!WorldContext || EventId.IsNone() || IsMuted())
	{
		return;
	}

	// Defer one frame so a primary event fired later in this same frame (the
	// clamp after the menu opened, Trade.Sell after the credits changed) still wins.
	// The core ticker runs while the game is paused, unlike world timers.
	const TWeakObjectPtr<const UObject> WeakContext(WorldContext);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[WeakContext, EventId, MinInterval](float) -> bool
		{
			const UObject* Context = WeakContext.Get();
			if (!Context || IsMuted())
			{
				return false;
			}
			if (FPlatformTime::Seconds() - AdastreaAudioEvents::LastPrimaryTime < SecondaryYieldSeconds)
			{
				return false;
			}
			UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(Context);
			if (!Catalog || !PassesGate(EventId, MinInterval))
			{
				return false;
			}
			LogEvent(Context, EventId, TEXT("2D secondary"));
			Catalog->PlayEvent2D(EventId);
			return false; // one-shot
		}));
}

UAudioComponent* UAudioEventLibrary::SpawnEventAttached(const UObject* WorldContext, FName EventId, USceneComponent* Parent, bool bAutoPlay)
{
	if (EventId.IsNone())
	{
		return nullptr;
	}
	UAudioCatalogSubsystem* Catalog = UAudioCatalogSubsystem::Get(WorldContext);
	if (!Catalog)
	{
		return nullptr;
	}
	LogEvent(WorldContext, EventId, Parent ? TEXT("attached") : TEXT("2D loop"));
	return Catalog->SpawnEventAttached(EventId, Parent, bAutoPlay);
}

void UAudioEventLibrary::FadeOutAndRelease(UAudioComponent* Component, float FadeSeconds)
{
	if (!IsValid(Component))
	{
		return;
	}
	// Let the fade finish, then the component cleans itself up.
	Component->bAutoDestroy = true;
	if (Component->IsPlaying() && FadeSeconds > 0.0f)
	{
		Component->FadeOut(FadeSeconds, 0.0f);
	}
	else
	{
		Component->Stop();
		Component->DestroyComponent();
	}
}

bool UAudioEventLibrary::IsLocalPlayerActor(const AActor* Actor)
{
	// Walk up the owner chain to the pawn (components' owners, spawned helpers).
	const AActor* Current = Actor;
	for (int32 Guard = 0; Current && Guard < 8; ++Guard)
	{
		if (const APawn* Pawn = Cast<APawn>(Current))
		{
			return Pawn->IsPlayerControlled() && Pawn->IsLocallyControlled();
		}
		Current = Current->GetOwner();
	}
	return false;
}

float UAudioEventLibrary::GetShipSizeFactor(const USpaceshipDataAsset* ShipData)
{
	if (!ShipData)
	{
		return 0.3f;
	}
	const float Mass = FMath::Max(ShipData->HullStrength + ShipData->CargoCapacity, 1.0f);
	const float T = (FMath::LogX(10.0f, Mass) - AdastreaAudioEvents::SizeLogMin)
		/ (AdastreaAudioEvents::SizeLogMax - AdastreaAudioEvents::SizeLogMin);
	return FMath::Clamp(T, 0.0f, 1.0f);
}
