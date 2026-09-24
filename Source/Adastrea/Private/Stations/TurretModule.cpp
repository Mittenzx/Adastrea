// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/TurretModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/PackageName.h"

ATurretModule::ATurretModule()
{
    TurretHeadComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretHead"));
    TurretHeadComponent->SetupAttachment(MeshComponent);

    // Resolved once (first construction = the CDO, on the game thread) and re-applied
    // on every construction: native-constructor-set subobject state is NOT copied from
    // the CDO into native-class instances. Missing head mesh = silently no head.
    static bool bResolved = false;
    static TWeakObjectPtr<UStaticMesh> CachedHeadMesh;
    static FVector CachedHeadLocation(0.0f, 0.0f, 50.0f);
    if (!bResolved && IsInGameThread())
    {
        bResolved = true;
        if (FPackageName::DoesPackageExist(TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_TurretHead_01")))
        {
            CachedHeadMesh = LoadObject<UStaticMesh>(nullptr,
                TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_TurretHead_01.SM_StationModule_TurretHead_01"),
                nullptr, LOAD_NoWarn | LOAD_Quiet);
        }
        // Mount at the base TurretHead socket LOCATION only. The exported socket
        // carries a flipped (yaw 180 / roll 180) rotation from the FBX axis
        // conversion, so snapping to the socket transform would hang the head
        // upside down; the head pivot is already its yaw axis.
        if (const UStaticMesh* BaseMesh = MeshComponent ? MeshComponent->GetStaticMesh() : nullptr)
        {
            if (const UStaticMeshSocket* Socket = BaseMesh->FindSocket(TEXT("TurretHead")))
            {
                CachedHeadLocation = Socket->RelativeLocation;
            }
        }
    }
    if (UStaticMesh* HeadMesh = CachedHeadMesh.Get())
    {
        TurretHeadComponent->SetStaticMesh(HeadMesh);
    }
    TurretHeadComponent->SetRelativeLocation(CachedHeadLocation);

    ModuleType = TEXT("Turret");
    ModulePower = 25.0f;
    ModuleGroup = EStationModuleGroup::Defence;

    DamagePerShot = 10.0f;
    EngagementRange = 12000.0f;
    FireRate = 2.0f;
}

bool ATurretModule::CanFire() const
{
    // A turret is operational while undamaged. Power availability is enforced by
    // the station combat layer (a powerless station can't provide firing energy).
    return !IsDestroyed_Implementation();
}

float ATurretModule::GetDps() const
{
    return DamagePerShot * FireRate;
}