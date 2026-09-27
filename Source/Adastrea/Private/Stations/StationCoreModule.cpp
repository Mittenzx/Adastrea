// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/StationCoreModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Misc/ScopeLock.h"
#include "AdastreaLog.h"

namespace
{
    /**
     * Load a core hull by object path, once per path. Like the module mesh cache in
     * SpaceStationModule.cpp: the CDO (built on the game thread) resolves it, and
     * later constructions on the async loading thread only read the cache.
     */
    UStaticMesh* LoadCoreHull(const TCHAR* MeshPath)
    {
        static FCriticalSection CacheLock;
        static TMap<FString, TWeakObjectPtr<UStaticMesh>> Cache;

        {
            FScopeLock Lock(&CacheLock);
            if (const TWeakObjectPtr<UStaticMesh>* Found = Cache.Find(MeshPath))
            {
                if (Found->IsValid())
                {
                    return Found->Get();
                }
            }
        }
        if (!IsInGameThread())
        {
            return nullptr;
        }

        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
        if (!Mesh)
        {
            UE_LOG(LogAdastreaStations, Warning, TEXT("StationCoreModule: hull mesh %s not found"), MeshPath);
        }
        FScopeLock Lock(&CacheLock);
        Cache.Add(MeshPath, Mesh);
        return Mesh;
    }
}

AStationCoreModule::AStationCoreModule()
{
    ModuleType = TEXT("Core");
    ModulePower = 0.0f;
    ModuleGroup = EStationModuleGroup::Other;
    MaxModuleIntegrity = 5000.0f;
    CurrentModuleIntegrity = 5000.0f;

    // The root carries no geometry: the base class would give an unlisted class a
    // cube. The hull lives on a child so it can be recentred on the grid cell.
    MeshComponent->SetStaticMesh(nullptr);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HullComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreHull"));
    HullComponent->SetupAttachment(MeshComponent);
    HullComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

void AStationCoreModule::InitCore(const TCHAR* MeshPath, int32 InFootprintCells)
{
    FootprintCells = FMath::Max(1, InFootprintCells);

    UStaticMesh* Hull = LoadCoreHull(MeshPath);
    if (!Hull)
    {
        return;
    }
    HullComponent->SetStaticMesh(Hull);

    // Core hulls are authored to the grid (Tools/generate_station_core_meshes.py):
    // pivot on the footprint centre at module mid-height, collars on the footprint
    // boundary. So they're used as-is; only the height is measured, as a box centred
    // on the pivot that covers whichever way the hull reaches further.
    const FBox Box = Hull->GetBoundingBox();
    const float WidestSide = 2.0f * FMath::Max(Box.Max.X, FMath::Max(-Box.Min.X, FMath::Max(Box.Max.Y, -Box.Min.Y)));
    if (WidestSide > FootprintCells * GridCellSize + 1.0f)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("StationCoreModule: %s is %.0f cm wide, more than its %d-cell footprint"),
            MeshPath, WidestSide, FootprintCells);
    }
    const float HalfHeight = FMath::Max(Box.Max.Z, -Box.Min.Z);
    HeightCells = FMath::Max(1, FMath::CeilToInt(2.0f * HalfHeight / GridCellSize - KINDA_SMALL_NUMBER));
}

AStationCore_TradeHub::AStationCore_TradeHub()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_TradeHub_01.SM_Station_Core_TradeHub_01"), 4);
}

AStationCore_Agricultural::AStationCore_Agricultural()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_Agricultural_01.SM_Station_Core_Agricultural_01"), 4);
}

AStationCore_Industrial::AStationCore_Industrial()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_Industrial_01.SM_Station_Core_Industrial_01"), 3);
}

AStationCore_Research::AStationCore_Research()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_Research_01.SM_Station_Core_Research_01"), 3);
}

AStationCore_Luxury::AStationCore_Luxury()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_Luxury_01.SM_Station_Core_Luxury_01"), 3);
}

AStationCore_BlackMarket::AStationCore_BlackMarket()
{
    InitCore(TEXT("/AdastreaShips/Meshes/Station/SM_Station_Core_BlackMarket_01.SM_Station_Core_BlackMarket_01"), 3);
}
