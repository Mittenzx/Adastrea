// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "FuelDepotModule.generated.h"

/**
 * Fuel depot module for space stations
 *
 * Stores refined fuel that visiting ships (and the player) can refuel from.
 * Tracks a fuel reserve in litres so refuelling can consume a real resource
 * rather than being free.
 *
 * Power Consumption: 15 units
 * Module Group: Storage
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AFuelDepotModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AFuelDepotModule();

	/** Maximum fuel reserve in litres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FuelDepot", meta=(ClampMin="0"))
	float FuelCapacity = 5000.0f;

	/** Current fuel reserve in litres. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FuelDepot", meta=(ClampMin="0"))
	float FuelLevel = 5000.0f;

	/** Fuel remaining, clamped >= 0. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FuelDepot")
	float GetFuelLevel() const;

	/** Fuel fill fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FuelDepot")
	float GetFuelRatio() const;

	/** Whether at least Amount litres can be drawn. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FuelDepot")
	bool HasFuelAvailable(float Amount) const;

	/** Add fuel (restock). Returns false if it would overflow. */
	UFUNCTION(BlueprintCallable, Category="FuelDepot")
	bool AddFuel(float Amount);

	/** Draw fuel up to Amount. Returns actual litres withdrawn. */
	UFUNCTION(BlueprintCallable, Category="FuelDepot")
	float DrawFuel(float Amount);
};