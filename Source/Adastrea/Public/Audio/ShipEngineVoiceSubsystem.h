// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ShipEngineVoiceSubsystem.generated.h"

class UShipEngineAudioComponent;

/**
 * World-level arbitration for ship engine voices.
 *
 * - AI voice limit: a few times a second, ranks every AI-mode UShipEngineAudioComponent by
 *   distance from the listener to its hull and grants the nearest MaxAIVoices (within
 *   AISilentRange) permission to play. Everything else stays stopped, so distant fleets cost nothing.
 * - Debug readout: `adastrea.EngineVoiceDebug 1` draws one line per audible engine plus the
 *   nearest AI hull distance; `2` also logs those lines twice a second.
 * - `adastrea.EngineVoiceReport` logs the derived voice for every USpaceshipDataAsset.
 */
UCLASS()
class ADASTREA_API UShipEngineVoiceSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void Register(UShipEngineAudioComponent* Engine);
	void Unregister(UShipEngineAudioComponent* Engine);

	/** Log the voice table (ship, size, agility, family, base pitch, whine) for every ship data asset. */
	static void LogVoiceReport();

	// UTickableWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	void ArbitrateAIVoices();
	void DrawDebug(float DeltaTime);

	bool GetListener(FVector& OutLocation) const;

	TArray<TWeakObjectPtr<UShipEngineAudioComponent>> Engines;

	float ArbitrationTimer = 0.0f;
	float DebugLogTimer = 0.0f;
};
