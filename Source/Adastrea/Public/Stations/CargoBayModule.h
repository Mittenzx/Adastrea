// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "CargoBayModule.generated.h"

/**
 * Cargo bay module for space stations
 *
 * Provides volume for storing goods the station buys/trades. Tracks used vs
 * free capacity so the economy layer can refuse trades that would overflow
 * storage (a market can't buy your whole hold if it's full).
 *
 * Power Consumption: 5 units
 * Module Group: Storage
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACargoBayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ACargoBayModule();

	/** Total storage volume in units. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CargoBay", meta=(ClampMin="0"))
	int32 StorageCapacity = 800;

	/** Currently used volume in units. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CargoBay", meta=(ClampMin="0"))
	int32 StoredAmount = 0;

	/** Free volume left (capacity - stored). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CargoBay")
	int32 GetFreeStorage() const;

	/** Fill fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CargoBay")
	float GetFillRatio() const;

	/** Whether the bay has room for Amount more units. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CargoBay")
	bool HasSpaceFor(int32 Amount) const;

	/** Reserve Amount units (e.g. storage a station buys from the player). Returns false if no room. */
	UFUNCTION(BlueprintCallable, Category="CargoBay")
	bool AddStorage(int32 Amount);

	/** Release Amount units (e.g. a station sells goods). Returns false if not stored. */
	UFUNCTION(BlueprintCallable, Category="CargoBay")
	bool RemoveStorage(int32 Amount);
};