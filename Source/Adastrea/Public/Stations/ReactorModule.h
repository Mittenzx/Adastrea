// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ReactorModule.generated.h"

/**
 * Power reactor module for space stations
 *
 * Main power generation facility using fusion reactor technology.
 * Generates substantial power output to support station operations.
 *
 * The reactor has a rated generation capacity (MaxPowerOutput). When it takes
 * structural damage its effective output degrades proportionally, until it is
 * destroyed (output 0). Toggle online/offline via SetReactorOnline to shut down
 * a reactor without destroying it (e.g. for maintenance or a black-market market).
 *
 * Power Generation: -500 units (negative = generates power)
 * Module Group: Power
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AReactorModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AReactorModule();

	/** Rated power generation capacity (positive value; station sums it as generation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reactor", meta=(ClampMin="0.0"))
	float MaxPowerOutput = 500.0f;

	/** Whether the reactor is producing power right now (damage/destruction forces it off). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reactor")
	bool bReactorOnline = true;

	/** Whether reactors can be toggled offline by gameplay (crew/black-market events). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reactor")
	bool bCanShutdown = true;

	/** Enabled/disable (manually) or via station power management. */
	UFUNCTION(BlueprintCallable, Category="Reactor")
	void SetReactorOnline(bool bOnline);

	/** Effective power currently generated (degrades with damage, 0 when offline/destroyed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reactor")
	float GetCurrentPowerOutput() const;

	/** True if the reactor is undamaged enough to produce full rated output. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reactor")
	bool IsProducingFullOutput() const;
};