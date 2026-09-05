// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "HabitationModule.generated.h"

/**
 * Habitation module for space stations
 *
 * Civilian living quarters. Houses the station's permanent population
 * (residents / workforce) as opposed to military berths in the barracks.
 * Tracks capacity + current residents.
 *
 * Power Consumption: 30 units
 * Module Group: Habitation
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AHabitationModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AHabitationModule();

	/** Total resident capacity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Habitation", meta=(ClampMin="0"))
	int32 Capacity = 250;

	/** Number of residents currently living here. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Habitation", meta=(ClampMin="0"))
	int32 Residents = 0;

	/** Free residential capacity. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Habitation")
	int32 GetFreeCapacity() const;

	/** Occupancy fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Habitation")
	float GetOccupancyRatio() const;

	/** Add up to Count residents. Returns number actually housed. */
	UFUNCTION(BlueprintCallable, Category="Habitation")
	int32 AddResidents(int32 Count);

	/** Remove up to Count residents. Returns number actually removed. */
	UFUNCTION(BlueprintCallable, Category="Habitation")
	int32 RemoveResidents(int32 Count);
};