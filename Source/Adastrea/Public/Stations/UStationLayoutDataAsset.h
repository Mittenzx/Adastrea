// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Stations/StationModuleTypes.h"
#include "UStationLayoutDataAsset.generated.h"

// Forward declarations
class ASpaceStationModule;

/** A single module entry in a station layout, at grid coordinates. */
USTRUCT(BlueprintType)
struct FStationLayoutModule
{
	GENERATED_BODY()

	/** Unique module instance id within the layout (e.g. "M1"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	FString ModuleID;

	/** Crafting-tree ItemID of the module (matches Stations/*.h class name, e.g.
	 *  "CorridorModule", "ReactorModule", "WeaponsLabModule" ...). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	FName ItemID;

	/** Grid cell position (x, y, z) in units of GridSpacing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	FIntVector GridPos = FIntVector::ZeroValue;

	/** Yaw rotation in degrees (0/90/180/270). Re-orients the module's connection
	 *  faces (a SolarArray only connects through its W face, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	int32 Rotation = 0;

	/** Exactly one module in a layout is the core (station anchor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	bool IsCore = false;

	FStationLayoutModule() {}
	FStationLayoutModule(const FString& InID, FName InItem, FIntVector InPos,
		int32 InRot, bool InCore)
		: ModuleID(InID), ItemID(InItem), GridPos(InPos), Rotation(InRot), IsCore(InCore) {}
};

/**
 * A data asset describing a space-station layout, mirroring the machine-readable
 * StationLayout / ExampleStationLayout.json authored by the Station Builder agent.
 *
 * The builder produces these (plan mode) and ASpaceStation consumes them to spawn
 * the real modules (BuildFromLayout).
 *
 * Usage (see STATION_BUILDER.md):
 *   UStationLayoutDataAsset* Layout = LoadUAsset<UStationLayoutDataAsset>(...);
 *   ASpaceStation* Station = SpawnActor<ASpaceStation>(...);
 *   Station->BuildFromLayout(Layout);   // spawns each module at GridPos*Spacing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UStationLayoutDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Display name for the designed station. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
	FString StationName;

	/** Build plot footprint (x, y, z) in unreal-cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
	FVector PlotSize = FVector(10000.0f, 10000.0f, 10000.0f);

	/** Distance between adjacent grid nodes, in unreal-cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout", meta=(ClampMin=10.0f))
	float GridSpacing = 100.0f;

	/** The modules to spawn. Order matters only for core selection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
	TArray<FStationLayoutModule> Modules;

	/** Total build cost (sum of each module's crafted OutputValue). Filled by
	 *  the builder; informational only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Economy")
	int64 TotalBuildCost = 0;

	/** Crew budget: berths provided vs crew required. Filled by the builder. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Economy")
	int32 CrewBerths = 0;

	/** Crew budget: required crew; positive margin = enough berths. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Economy")
	int32 CrewRequired = 0;

	/** True if this layout passed validation (connected, powered, dockable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
	bool IsValid = false;
};