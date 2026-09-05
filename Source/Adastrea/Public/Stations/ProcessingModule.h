// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "ProcessingModule.generated.h"

/**
 * Processing module for space stations
 *
 * Refines raw materials (raw ore, gases, ice) into refined goods and fuel.
 * Has a throughput rate (units/sec) and an optional input buffer of raw material
 * waiting to be refined.
 *
 * Power Consumption: 100 units
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AProcessingModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AProcessingModule();

	/** Maximum raw material buffered for refinement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Processing", meta=(ClampMin="0"))
	int32 BufferCapacity = 400;

	/** Units of raw material currently buffered. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Processing", meta=(ClampMin="0"))
	int32 BufferedAmount = 0;

	/** Throughput: units refined per second while running. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Processing", meta=(ClampMin="0.1"))
	float ThroughputPerSecond = 20.0f;

	/** True when the station is actively feeding this processor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Processing")
	bool bIsRunning = false;

	/** Buffer fill fraction 0..1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Processing")
	float GetBufferRatio() const;

	/** Feed Amount of raw material in. Returns false if buffer full. */
	UFUNCTION(BlueprintCallable, Category="Processing")
	bool FeedRawMaterial(int32 Amount);

	/** Request the station processes up to Amount worth; clamps to buffered + sets running. */
	UFUNCTION(BlueprintCallable, Category="Processing")
	int32 Process(int32 Amount);
};