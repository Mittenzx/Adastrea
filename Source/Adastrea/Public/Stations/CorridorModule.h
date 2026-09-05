// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "CorridorModule.generated.h"

/**
 * Corridor module for space stations
 *
 * Structural/connection module linking other modules together. Provides a
 * traffic throughput ceiling and counts how many modules it connects, which is
 * used to keep a station's module network contiguous and traversable.
 *
 * Power Consumption: 2 units
 * Module Group: Connection
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACorridorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ACorridorModule();

	/** Number of other modules this corridor connects to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corridor", meta=(ClampMin="0"))
	int32 ConnectedModuleCount = 0;

	/** Maximum concurrent traffic (people/goods) this corridor can carry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corridor", meta=(ClampMin="1"))
	int32 MaxTraffic = 80;

	/** Current traffic load, clamped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corridor", meta=(ClampMin="0"))
	int32 CurrentTraffic = 0;

	/** Whether the corridor is a dead-end (only one connection). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Corridor")
	bool IsIsolated() const;

	/** Traffic load fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Corridor")
	float GetTrafficRatio() const;
};