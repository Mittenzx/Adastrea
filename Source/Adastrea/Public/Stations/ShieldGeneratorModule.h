// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ShieldGeneratorModule.generated.h"

/**
 * Shield generator module for space stations
 *
 * Projects an energy shield that absorbs incoming damage before it reaches the
 * station's structure. Charging costs power; the station recharges the bubble
 * up to MaxShieldStrength over time using its power surplus.
 *
 * Power Consumption: 200 units
 * Module Group: Defence
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AShieldGeneratorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AShieldGeneratorModule();

	/** Maximum shield hit points this generator projects. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ShieldGenerator", meta=(ClampMin="0.0"))
	float MaxShieldStrength = 4000.0f;

	/** Current shield hit points. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ShieldGenerator", meta=(ClampMin="0.0"))
	float CurrentShieldStrength = 4000.0f;

	/** Shield points regenerated per second while the station has power. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ShieldGenerator", meta=(ClampMin="0.0"))
	float RechargePerSecond = 25.0f;

	/** Whether the bubble is up (consumes power). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ShieldGenerator")
	bool bShieldsActive = true;

	/** Shield fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ShieldGenerator")
	float GetShieldRatio() const;

	/** Absorb incoming damage; returns the damage that passed through to the hull. */
	UFUNCTION(BlueprintCallable, Category="ShieldGenerator")
	float AbsorbDamage(float IncomingDamage);

	/** Recharge the bubble by this tick's power allocation; clamps to max. */
	UFUNCTION(BlueprintCallable, Category="ShieldGenerator")
	void RechargeShield(float PowerAllocated);
};