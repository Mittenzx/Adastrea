// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioCatalogSubsystem.generated.h"

class UAudioCatalogDataAsset;
class UAudioComponent;
class USceneComponent;
class USoundBase;
struct FAudioCatalogEntry;

/**
 * Plays audio by event ID through /Game/Audio/DA_AudioCatalog.
 *
 * Safe by design: an unknown event ID, a missing catalog, or a sound that
 * fails to load logs one warning per ID and then silently no-ops. It never
 * crashes, including in headless (-nullrhi, -nosound) or packaged runs.
 *
 * One-shots: PlayEvent2D / PlayEventAtLocation.
 * Loops (engines, hums, beds): SpawnEventAttached, which returns a component the
 * caller owns and can Stop / FadeOut / SetPitchMultiplier. Looping entries passed
 * to the one-shot functions are refused (logged once) because nothing could stop them.
 */
UCLASS()
class ADASTREA_API UAudioCatalogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Asset path of the project catalog. */
	static const TCHAR* CatalogPath;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Convenience accessor from any world context. May return nullptr. */
	static UAudioCatalogSubsystem* Get(const UObject* WorldContextObject);

	/** Plays a non-spatial one-shot (UI, player-only feedback). */
	UFUNCTION(BlueprintCallable, Category = "Audio|Catalog")
	void PlayEvent2D(FName EventId);

	/** Plays a spatial one-shot at a world location, using the entry's attenuation. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Catalog")
	void PlayEventAtLocation(FName EventId, FVector Location);

	/**
	 * Creates an audio component for the event attached to Parent (3D, entry attenuation).
	 * If Parent is null the component is 2D (non-spatial). The component is not auto-destroyed;
	 * the caller keeps it and stops it. Returns nullptr when the event can't be resolved.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Catalog")
	UAudioComponent* SpawnEventAttached(FName EventId, USceneComponent* Parent, bool bAutoPlay);

	/** Returns the (loaded) sound for an event, or nullptr. With variations, a random one. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Catalog")
	USoundBase* GetSound(FName EventId);

	/** True when the catalog has an entry for EventId. */
	UFUNCTION(BlueprintPure, Category = "Audio|Catalog")
	bool HasEvent(FName EventId) const;

	/** The loaded catalog (nullptr if /Game/Audio/DA_AudioCatalog is missing). */
	UFUNCTION(BlueprintPure, Category = "Audio|Catalog")
	UAudioCatalogDataAsset* GetCatalog() const { return Catalog; }

private:
	/** Finds the entry and loads its sound; logs once per ID and returns false on failure. */
	bool ResolveEvent(FName EventId, const FAudioCatalogEntry*& OutEntry, USoundBase*& OutSound);

	/** Logs a warning the first time a given ID fails for a given reason. */
	void WarnOnce(FName EventId, const FString& Reason);

	/** Random pitch in the entry's range. */
	static float PickPitch(const FAudioCatalogEntry& Entry);

	UPROPERTY(Transient)
	TObjectPtr<UAudioCatalogDataAsset> Catalog = nullptr;

	/** Keeps every sound resolved so far loaded. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<USoundBase>> KeepAlive;

	/** IDs already warned about (so logs stay one line per ID). */
	TSet<FName> WarnedIds;

	bool bWarnedMissingCatalog = false;
};
