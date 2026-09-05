// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "SolarArrayModule.generated.h"

/**
 * Solar array module for space stations
 *
 * Alternative power generation using photovoltaic panels. Output depends on
 * the station's local light level (e.g. proximity to a star / distance from
 * occlusion), giving a cheaper but environment-dependent power source.
 *
 * Power Generation: -100 units (negative = generates power) at full illumination
 * Module Group: Power
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ASolarArrayModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ASolarArrayModule();

	/** Peak generation capacity at 100% illumination (positive value). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SolarArray", meta=(ClampMin="0.0"))
	float PeakOutput = 100.0f;

	/** Current illumination factor 0..1 (0 = dark, 1 = full). Station sets this each tick. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SolarArray", meta=(ClampMin="0.0", ClampMax="1.0"))
	float Illumination = 1.0f;

	/** Effective output right now = PeakOutput * Illumination * health. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="SolarArray")
	float GetEffectiveOutput() const;

	/** Whether the array contributes meaningfully (above a small threshold). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="SolarArray")
	bool IsContributing() const;
};