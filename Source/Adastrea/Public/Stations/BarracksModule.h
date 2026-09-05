// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "BarracksModule.generated.h"

/**
 * Barracks module for space stations
 *
 * Crew quarters / military housing. Hosts station personnel (defence crews,
 * station crew). Tracks occupied vs total berths so a station can show its
 * available population / staffing headroom.
 *
 * Power Consumption: 20 units
 * Module Group: Habitation
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ABarracksModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ABarracksModule();

	/** Total crew berths. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Barracks", meta=(ClampMin="0"))
	int32 Capacity = 120;

	/** Crew currently quartered. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Barracks", meta=(ClampMin="0"))
	int32 Occupancy = 0;

	/** Free berths. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Barracks")
	int32 GetFreeBerths() const;

	/** Occupancy fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Barracks")
	float GetOccupancyRatio() const;

	/** Quarter up to Count crew. Returns number actually housed. */
	UFUNCTION(BlueprintCallable, Category="Barracks")
	int32 HouseCrew(int32 Count);

	/** Release up to Count crew. Returns number actually released. */
	UFUNCTION(BlueprintCallable, Category="Barracks")
	int32 ReleaseCrew(int32 Count);
};