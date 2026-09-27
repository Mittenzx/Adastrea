// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Audio/ShipEngineSoundProfile.h"
#include "ShipEngineAudioComponent.generated.h"

class ASpaceship;
class UAudioComponent;
class USoundAttenuation;
class USoundBase;
class USpaceshipDataAsset;
struct FSoundAttenuationSettings;
struct FAudioCatalogEntry;

ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaEngineVoice, Log, All);

/** How a ship's engine is currently heard. */
UENUM(BlueprintType)
enum class EShipEngineVoiceMode : uint8
{
	/** Not initialised yet, or no engine sounds could be resolved. */
	Off,
	/** The ship the player is flying: non-spatial (2D), always audible. */
	Player,
	/** The player's ship while they're on foot (its interior, or a station it's docked at): 2D, low-passed and quieter. */
	PlayerMuffled,
	/** Every other ship (AI-piloted or parked): 3D, audible only very close to the hull. */
	AI
};

/**
 * The engine voice of a spaceship. ASpaceship creates one by default, so every ship class
 * and Blueprint subclass has it.
 *
 * STATIC VOICE (DeriveVoice): from the ship's USpaceshipDataAsset,
 *  - Size    = ln(hull) and ln(1 + cargo), weighted and normalised across the roster. It picks
 *              the loop family (Light/Medium/Heavy/Capital) and the pitch within that family:
 *              smaller ships sit higher. MaxSpeed is deliberately NOT used (the Freighter's is
 *              higher than the Fighter's).
 *  - Agility = log(acceleration x maneuverability), normalised. It sets the whine layer level.
 *  - FShipEngineSoundProfile on the data asset can override family/pitch/whine and any layer's sound.
 *
 * RUNTIME (TickComponent), all smoothed with interp so nothing steps:
 *  - revs:     speed / max speed crossfades the Low -> High loop and adds up to RevPitchRise pitch
 *  - throttle: body volume follows the throttle; at 0 only a faint idle hum remains
 *  - throttle changes play an Engine.SpoolUp / Engine.SpoolDown one-shot
 *  - boost:    fades in Engine.Boost, adds BoostPitchRise pitch, Engine.BoostStart on the rising edge
 *
 * PLAYER vs AI: re-evaluated every tick from the player controller, so possession changes
 * (hangar ship swap, save load, leaving the cockpit) switch modes automatically. AI engines use
 * a box attenuation around the hull (from the mesh bounds), so capital ships count from their
 * surface: full within AIFullVolumeRange of the hull, silent at AISilentRange. At most
 * MaxAIVoices AI ships play at once (UShipEngineVoiceSubsystem grants them, nearest first).
 *
 * All layers resolve through UAudioCatalogSubsystem event IDs (Engine.*), never by asset name.
 * Debug: adastrea.EngineVoiceReport (voice table for every ship data asset),
 *        adastrea.EngineVoiceDebug 1 (on-screen readout; 2 also logs twice a second).
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent), config=Game)
class ADASTREA_API UShipEngineAudioComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UShipEngineAudioComponent();

	// ====================
	// VOICE DERIVATION (roster-wide tuning; config=Game)
	// ====================

	/** Weight of log(HullStrength) in the size score. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float SizeHullWeight = 0.65f;

	/** Weight of log(1 + CargoCapacity) in the size score. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float SizeCargoWeight = 0.35f;

	/** Size score that maps to 0 (the smallest ship in the roster). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float SizeScoreMin = 4.3f;

	/** Size score that maps to 1 (the largest ship in the roster). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float SizeScoreMax = 9.5f;

	/** log(acceleration x maneuverability) that maps to agility 0. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float AgilityScoreMin = 3.9f;

	/** log(acceleration x maneuverability) that maps to agility 1. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation")
	float AgilityScoreMax = 7.5f;

	/** Base pitch of the smallest ship within a family. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation", meta=(ClampMin="0.7", ClampMax="1.4"))
	float FamilyPitchHigh = 1.04f;

	/** Base pitch of the largest ship within a family. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation", meta=(ClampMin="0.7", ClampMax="1.4"))
	float FamilyPitchLow = 0.78f;

	/** Whine = Agility ^ WhineExponent, so sluggish ships get almost none. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Derivation", meta=(ClampMin="0.5", ClampMax="4.0"))
	float WhineExponent = 1.5f;

	// ====================
	// RUNTIME SHAPING
	// ====================

	/** Extra pitch at full revs (0.25 = +25%). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float RevPitchRise = 0.25f;

	/** Extra pitch while boosting (0.10 = +10%). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float BoostPitchRise = 0.10f;

	/** Level of the idle hum at zero throttle (it fades out as the throttle opens). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime", meta=(ClampMin="0.0", ClampMax="1.0"))
	float IdleLevel = 0.35f;

	/** Level of the boost layer when fully boosting. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BoostLevel = 0.7f;

	/** Level of the spool-up/down and boost-start one-shots. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime", meta=(ClampMin="0.0", ClampMax="1.0"))
	float OneShotLevel = 0.5f;

	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float ThrottleInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float RevInterpSpeed = 1.5f;

	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float BoostInterpSpeed = 3.0f;

	/** Minimum throttle change (0..1) that plays a spool one-shot, for the player's ship. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float PlayerSpoolThreshold = 0.08f;

	/** Minimum throttle change (0..1) that plays a spool one-shot, for AI ships (their throttle moves continuously). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float AISpoolThreshold = 0.25f;

	/** Seconds between spool one-shots (player). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float PlayerSpoolCooldown = 0.4f;

	/** Seconds between spool one-shots (AI). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Runtime")
	float AISpoolCooldown = 2.0f;

	// ====================
	// PLAYER SHIP
	// ====================

	/** Overall level of the player's own engine (kept low; SC_Engine and the mix do the rest). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Player", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PlayerVolume = 0.8f;

	/** Volume scale while muffled (player on foot / inside). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Player", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MuffledVolumeScale = 0.4f;

	/** Low-pass cutoff while muffled (heard through the hull). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Player", meta=(ClampMin="100.0", ClampMax="20000.0"))
	float MuffledLowPassHz = 700.0f;

	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|Player")
	float MuffleInterpSpeed = 2.5f;

	// ====================
	// AI SHIPS
	// ====================

	/** Full volume within this distance of the hull (cm). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|AI", meta=(ClampMin="0.0"))
	float AIFullVolumeRange = 1000.0f;

	/** Silent at this distance from the hull (cm); AI engines are only for very close range. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|AI", meta=(ClampMin="100.0"))
	float AISilentRange = 5000.0f;

	/** AI engines are quieter than the player's (multiplies PlayerVolume). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|AI", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AIVolumeScale = 0.55f;

	/** At most this many AI ships' engines play at once (nearest first). */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|AI", meta=(ClampMin="0", ClampMax="8"))
	int32 MaxAIVoices = 2;

	/** Base attenuation for AI engines. Its shape, extents and falloff are replaced by the hull box + the ranges above. */
	UPROPERTY(EditAnywhere, Config, Category="Engine Voice|AI")
	TSoftObjectPtr<USoundAttenuation> AIAttenuation;

	// ====================
	// API
	// ====================

	/** Derive a ship's engine voice from its data asset (uses the class-default tuning above). */
	static FShipEngineVoice DeriveVoice(const USpaceshipDataAsset* Data);

	/** Raw (un-normalised) size score: SizeHullWeight*ln(hull) + SizeCargoWeight*ln(1+cargo). */
	static float ComputeSizeScore(float HullStrength, float CargoCapacity);

	/** Raw (un-normalised) agility score: ln(acceleration x maneuverability). */
	static float ComputeAgilityScore(float Acceleration, float Maneuverability);

	/** Catalog event ID for a family's loop, e.g. Engine.Heavy.Low. */
	static FName FamilyEventId(EShipEngineFamily Family, bool bHighRevs);

	static const TCHAR* FamilyName(EShipEngineFamily Family);

	UFUNCTION(BlueprintPure, Category="Engine Voice")
	const FShipEngineVoice& GetVoice() const { return Voice; }

	UFUNCTION(BlueprintPure, Category="Engine Voice")
	EShipEngineVoiceMode GetMode() const { return Mode; }

	/** Re-derive the voice and reload the layer sounds (e.g. after swapping the data asset). */
	UFUNCTION(BlueprintCallable, Category="Engine Voice")
	void RefreshVoice();

	/** Distance (cm) from a world point to the hull's bounding box (0 inside it). */
	float GetDistanceToHull(const FVector& WorldPoint) const;

	/** Called by UShipEngineVoiceSubsystem: whether this AI ship may use one of the MaxAIVoices slots. */
	void SetAIVoiceGranted(bool bGranted);
	bool IsAIVoiceGranted() const { return bAIVoiceGranted; }

	/** True when any layer is currently playing. */
	bool IsAudible() const { return bLayersPlaying; }

	/** One-line state for the debug readout / log. HullDistanceCm < 0 omits the distance. */
	FString GetDebugLine(float HullDistanceCm = -1.0f) const;

	ASpaceship* GetShip() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum ELayer : int32 { Low = 0, High, Whine, Boost, Idle, NumLayers };

	/** Resolve the layer sounds from the catalog / the profile's custom sounds, creating the audio components. */
	void BuildLayers();
	void DestroyLayers();

	/** Size the hull box from the ship mesh bounds and sit this component at its centre. */
	void MeasureHull();

	EShipEngineVoiceMode ResolveMode() const;
	void ApplyMode(EShipEngineVoiceMode NewMode);
	FSoundAttenuationSettings MakeAttenuation(bool bSpatial) const;

	bool ShouldLayersPlay() const;
	void StartLayers();
	void StopLayers();

	void UpdateSpoolAndBoostEvents(float Throttle01, bool bBoost, float DeltaTime);
	void PlayOneShot(FName EventId);

	const FAudioCatalogEntry* FindCatalogEntry(FName EventId) const;
	UAudioComponent* MakeAudio(const TCHAR* LayerName, USoundBase* Sound);
	/** Loop layers go through UAudioCatalogSubsystem::SpawnEventAttached (or the profile's custom sound). */
	UAudioComponent* SpawnLayer(int32 Index, FName EventId, const TSoftObjectPtr<USoundBase>& Custom);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> Layers;

	/** One-shots (spool, boost start) play through this so they share the ship's mode/attenuation. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> OneShot;

	UPROPERTY(VisibleInstanceOnly, Transient, Category="Engine Voice")
	FShipEngineVoice Voice;

	UPROPERTY(VisibleInstanceOnly, Transient, Category="Engine Voice")
	EShipEngineVoiceMode Mode = EShipEngineVoiceMode::Off;

	/** Mode changes are debounced briefly, so a one-frame unpossess during a ship swap doesn't restart the loops. */
	EShipEngineVoiceMode PendingMode = EShipEngineVoiceMode::Off;
	float PendingModeTime = 0.0f;

	/** Overall level for the current mode (player / muffled / AI), shared by loops and one-shots. */
	float MasterVolume = 0.0f;

	/** Half extents (cm, world scale) of the hull box, in this component's frame. */
	FVector HullExtent = FVector(500.0f);

	bool bAIVoiceGranted = false;
	bool bLayersPlaying = false;

	// Smoothed runtime state
	float SmoothThrottle = 0.0f;
	float SmoothRev = 0.0f;
	float SmoothBoost = 0.0f;
	float SmoothMuffle = 0.0f;

	// Last values pushed to the audio components (for the debug readout)
	float OutPitch = 1.0f;
	float OutLayerVolume[NumLayers] = {};
	float OutLowPassHz = 20000.0f;

	// Catalog entry volume/pitch per layer, kept as factors under the runtime modulation
	float LayerEntryVolume[NumLayers] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	float LayerEntryPitch[NumLayers] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	// Spool / boost edge detection
	float SpoolReferenceThrottle = 0.0f;
	float SpoolCooldownLeft = 0.0f;
	bool bWasBoosting = false;
	FName LastOneShot;
};
