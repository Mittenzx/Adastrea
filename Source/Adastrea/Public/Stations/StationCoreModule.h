// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Stations/SpaceStationModule.h"
#include "StationCoreModule.generated.h"

/**
 * The anchor module every station is built outward from ("first placed = core").
 *
 * Each station archetype has its own core subclass with a unique hull mesh, so
 * stations are told apart by their core while the rest of the station is ordinary
 * catalog modules attached through the Station Editor's grid rules.
 *
 * A core occupies a square FootprintCells x FootprintCells block of the editor's
 * build grid. Its hull (SM_Station_Core_<Type>_01, Tools/generate_station_core_meshes.py)
 * is authored to exactly that block with a connection collar on each side face, so
 * attached modules' collars meet it on the boundary. Cores are not in the module
 * catalog, so players can't build or remove one.
 */
UCLASS(Abstract)
class ADASTREA_API AStationCoreModule : public ASpaceStationModule
{
    GENERATED_BODY()

public:
    AStationCoreModule();

    /** Build-grid cell size (cm). Matches UStationGridSystem::GridSize's default. */
    static constexpr float GridCellSize = 400.0f;

    /** Side length of the core's square footprint, in build-grid cells. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module|Core")
    int32 GetFootprintCells() const { return FootprintCells; }

    /** Footprint in build-grid cells: FootprintCells square, HeightCells tall. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module|Core")
    FIntVector GetFootprint() const { return FIntVector(FootprintCells, FootprintCells, HeightCells); }

protected:
    /**
     * Give this core its hull. Call from each archetype's constructor.
     * @param MeshPath Object path of the hull StaticMesh
     * @param InFootprintCells Side length of the square footprint in grid cells
     */
    void InitCore(const TCHAR* MeshPath, int32 InFootprintCells);

    /** Side length of the core's square footprint, in build-grid cells. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module|Core")
    int32 FootprintCells = 3;

    /** Height of the hull, in build-grid cells (rounded up), as a box centred on the pivot. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module|Core")
    int32 HeightCells = 1;

    /** The archetype's hull, authored to the footprint with its pivot on the module origin. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> HullComponent;
};

/** Trade hub: stepped octagonal trade tower over a wide concourse. */
UCLASS()
class ADASTREA_API AStationCore_TradeHub : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_TradeHub();
};

/** Agricultural: greenhouse ring on spokes round a domed hydroponics hub. */
UCLASS()
class ADASTREA_API AStationCore_Agricultural : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_Agricultural();
};

/** Industrial: refinery block with smelter stacks and an ore hopper. */
UCLASS()
class ADASTREA_API AStationCore_Industrial : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_Industrial();
};

/** Research: lab drum with a big sensor dish and an observatory sphere. */
UCLASS()
class ADASTREA_API AStationCore_Research : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_Research();
};

/** Luxury: octagonal gallery under a ribbed glass spire. */
UCLASS()
class ADASTREA_API AStationCore_Luxury : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_Luxury();
};

/** Black market: asteroid with scavenged habs and a hangar bolted on. */
UCLASS()
class ADASTREA_API AStationCore_BlackMarket : public AStationCoreModule
{
    GENERATED_BODY()
public:
    AStationCore_BlackMarket();
};
