// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/TurretModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/PackageName.h"
#include "AI/HostileFighterController.h"
#include "Combat/CombatProjectileSubsystem.h"
#include "Combat/CombatTeams.h"
#include "Combat/ShipWeaponComponent.h"
#include "Ships/Spaceship.h"
#include "EngineUtils.h"

namespace StationTurret
{
    /** Seconds between target re-picks. */
    constexpr float RetargetInterval = 0.5f;
    constexpr float BoltLength = 900.0f;
}

ATurretModule::ATurretModule()
{
    PrimaryActorTick.bCanEverTick = true;

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
        // Mount at the base TurretHead socket LOCATION only; the head pivot is already
        // its yaw axis. (Since the #508 re-export the sockets import at identity
        // rotation; before that they came in yaw 180 / roll 180 and would have hung the
        // head upside down.)
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

    DamagePerShot = 20.0f;
    EngagementRange = 20000.0f;
    FireRate = 3.0f;
}

FVector ATurretModule::GetHeadPivot() const
{
    if (TurretHeadComponent && TurretHeadComponent->GetStaticMesh())
    {
        return TurretHeadComponent->GetSocketLocation(TEXT("PitchAxis"));
    }
    return GetActorLocation() + GetActorUpVector() * 150.0f;
}

AActor* ATurretModule::FindTarget() const
{
    const FVector Pivot = GetHeadPivot();
    AActor* Best = nullptr;
    float BestDistSq = FMath::Square(EngagementRange);
    for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
    {
        ASpaceship* Ship = *It;
        if (Ship->IsWrecked() || Ship->IsHidden() || !AHostileFighterController::IsHostileShip(Ship))
        {
            continue;
        }
        const float DistSq = FVector::DistSquared(Pivot, Ship->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Best = Ship;
        }
    }
    return Best;
}

void ATurretModule::Fire(const FVector& Direction)
{
    UCombatProjectileSubsystem* Combat = UCombatProjectileSubsystem::Get(this);
    if (!Combat)
    {
        return;
    }
    FVector Muzzle = GetHeadPivot() + Direction * 200.0f;
    if (TurretHeadComponent && TurretHeadComponent->GetStaticMesh())
    {
        Muzzle = TurretHeadComponent->GetSocketLocation(bLeftBarrel ? TEXT("Muzzle_L") : TEXT("Muzzle_R"));
    }
    bLeftBarrel = !bLeftBarrel;

    const FVector Dir = FMath::VRandCone(Direction, FMath::DegreesToRadians(SpreadDegrees));
    Combat->FireBolt(Muzzle, Dir * BoltSpeed, DamagePerShot, EngagementRange * 1.2f / BoltSpeed,
        StationTurret::BoltLength, this, CombatTeam::Civil);
    ++ShotsFired;
}

void ATurretModule::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    Cooldown = FMath::Max(Cooldown - DeltaSeconds, 0.0f);
    if (!CanFire())
    {
        CurrentTarget = nullptr;
        return;
    }

    // Re-pick now and then (and at once when the target is wrecked or gone).
    RetargetClock -= DeltaSeconds;
    const ASpaceship* TargetShip = Cast<ASpaceship>(CurrentTarget.Get());
    if (RetargetClock <= 0.0f || !TargetShip || TargetShip->IsWrecked())
    {
        RetargetClock = StationTurret::RetargetInterval;
        CurrentTarget = FindTarget();
        TargetShip = Cast<ASpaceship>(CurrentTarget.Get());
    }
    if (!TargetShip)
    {
        return;
    }

    // Aim at the lead point; turrets don't move, so only the target's velocity counts.
    const FVector Pivot = GetHeadPivot();
    FVector Lead = TargetShip->GetActorLocation();
    UShipWeaponComponent::ComputeLeadPoint(Pivot, FVector::ZeroVector, TargetShip->GetActorLocation(), TargetShip->GetVelocity(), BoltSpeed, Lead);
    const FVector WantDir = (Lead - Pivot).GetSafeNormal();

    // Slew the head (yaw about the base's up axis, pitch within limits).
    FVector AimDir = WantDir;
    if (TurretHeadComponent && MeshComponent)
    {
        const FVector LocalDir = MeshComponent->GetComponentTransform().InverseTransformVectorNoScale(WantDir);
        FRotator Want = LocalDir.Rotation();
        Want.Pitch = FMath::Clamp(Want.Pitch, MinPitch, MaxPitch);
        Want.Roll = 0.0f;
        const FRotator Now = TurretHeadComponent->GetRelativeRotation();
        TurretHeadComponent->SetRelativeRotation(FMath::RInterpConstantTo(Now, Want, DeltaSeconds, TurnRate));
        AimDir = TurretHeadComponent->GetForwardVector();
    }

    const float OffDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(AimDir, WantDir), -1.0f, 1.0f)));
    if (OffDeg > FireConeDegrees)
    {
        return;
    }
    while (Cooldown <= 0.0f)
    {
        Fire(AimDir);
        Cooldown += 1.0f / FireRate;
    }
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