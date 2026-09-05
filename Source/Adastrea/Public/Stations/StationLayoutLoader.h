// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stations/UStationLayoutDataAsset.h"
#include "StationLayoutLoader.generated.h"

/**
 * Loads machine-readable station-layout JSON (as authored by the Station
 * Builder agent, e.g. Content/Data/ExampleStationLayout.json) into a
 * UStationLayoutDataAsset at runtime, mirroring UCraftingTreeLoader.
 *
 * The builder (plan mode) publishes a StationLayout; this loader lets the game
 * read it back and spawn the station via ASpaceStation::BuildFromLayout().
 */
UCLASS(BlueprintType)
class ADASTREA_API UStationLayoutLoader : public UObject
{
	GENERATED_BODY()

public:
	/** Relative path (under Content/) to a station-layout JSON. */
	static const TCHAR* GetLayoutPath() { return TEXT("Data/ExampleStationLayout.json"); }

	/**
	 * Parse a station-layout JSON and fill the given asset. Returns the number
	 * of modules loaded, or 0 on failure.
	 * @param Layout Target asset to fill (modules cleared first)
	 */
	UFUNCTION(BlueprintCallable, Category="Station|Builder")
	int32 LoadLayout(UStationLayoutDataAsset* Layout);

	/**
	 * Static convenience: spawn a real station from an asset by instantiating
	 * the matching ASpaceStationModule subclass for each entry at GridPos*Spacing.
	 * Implemented on ASpaceStation as BuildFromLayout() (see SpaceStation.h).
	 */
	UFUNCTION(BlueprintCallable, Category="Station|Builder")
	static void PlaceModules(UStationLayoutDataAsset* Layout, AActor* SpawnedStationModules, float SpacingNull);
};