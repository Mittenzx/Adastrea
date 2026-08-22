// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Combat/ProjectilePoolComponent.h"
#include "Combat/Projectile.h"
#include "Engine/World.h"
#include "AdastreaLog.h"

UProjectilePoolComponent::UProjectilePoolComponent()
    : InitialPoolSize(50)
    , MaxPoolSize(200)
    , ProjectileClass(nullptr)
    , TotalCreatedProjectiles(0)
{
    PrimaryComponentTick.bCanEverTick = false; // Pool doesn't need ticking
}

void UProjectilePoolComponent::InitializeComponent()
{
    Super::InitializeComponent();

    // Initialize pool if projectile class is set
    if (ProjectileClass)
    {
        InitializePool();
    }
    else
    {
        UE_LOG(LogAdastrea, Warning, TEXT("ProjectilePoolComponent: No ProjectileClass set - pool will not be initialized"));
    }
}

void UProjectilePoolComponent::UninitializeComponent()
{
    // Clean up all projectiles
    for (AProjectile* Projectile : PooledProjectiles)
    {
        if (IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    for (AProjectile* Projectile : ActiveProjectiles)
    {
        if (IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    PooledProjectiles.Empty();
    ActiveProjectiles.Empty();

    Super::UninitializeComponent();
}

AProjectile* UProjectilePoolComponent::AcquireProjectile()
{
    if (!ProjectileClass)
    {
        UE_LOG(LogAdastrea, Error, TEXT("ProjectilePoolComponent: Cannot acquire projectile - no ProjectileClass set"));
        return nullptr;
    }

    AProjectile* Projectile = nullptr;

    // Try to get from pool first
    if (PooledProjectiles.Num() > 0)
    {
        Projectile = PooledProjectiles.Pop();
    }
    else
    {
        // Create new projectile if pool is empty and we haven't hit max
        if (TotalCreatedProjectiles < MaxPoolSize)
        {
            Projectile = CreateProjectile();
        }
        else
        {
            UE_LOG(LogAdastrea, Warning, TEXT("ProjectilePoolComponent: Pool exhausted (MaxPoolSize: %d)"), MaxPoolSize);
            return nullptr;
        }
    }

    if (Projectile)
    {
        // Reset projectile state
        ResetProjectile(Projectile);

        // Make visible and collidable
        Projectile->SetActorHiddenInGame(false);
        Projectile->SetActorEnableCollision(true);

        // Add to active list
        ActiveProjectiles.Add(Projectile);

        UE_LOG(LogAdastrea, Verbose, TEXT("ProjectilePoolComponent: Acquired projectile %s"), *Projectile->GetName());
    }

    return Projectile;
}

void UProjectilePoolComponent::ReturnProjectile(AProjectile* Projectile)
{
    if (!IsValid(Projectile))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("ProjectilePoolComponent: Attempted to return invalid projectile"));
        return;
    }

    // Remove from active list
    ActiveProjectiles.Remove(Projectile);

    // Reset projectile
    ResetProjectile(Projectile);

    // Hide and disable collision
    Projectile->SetActorHiddenInGame(true);
    Projectile->SetActorEnableCollision(false);

    // Move to a safe location (below world)
    Projectile->SetActorLocation(FVector(0, 0, -10000));

    // Add back to pool
    PooledProjectiles.Add(Projectile);

    UE_LOG(LogAdastrea, Verbose, TEXT("ProjectilePoolComponent: Returned projectile %s to pool"), *Projectile->GetName());
}

void UProjectilePoolComponent::GetPoolStats(int32& OutActiveCount, int32& OutPooledCount, int32& OutTotalCount) const
{
    OutActiveCount = ActiveProjectiles.Num();
    OutPooledCount = PooledProjectiles.Num();
    OutTotalCount = TotalCreatedProjectiles;
}

void UProjectilePoolComponent::WarmupPool()
{
    if (!ProjectileClass)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("ProjectilePoolComponent: Cannot warmup pool - no ProjectileClass set"));
        return;
    }

    UE_LOG(LogAdastrea, Log, TEXT("ProjectilePoolComponent: Warming up pool with %d projectiles"), InitialPoolSize);

    for (int32 i = PooledProjectiles.Num(); i < InitialPoolSize && TotalCreatedProjectiles < MaxPoolSize; ++i)
    {
        AProjectile* Projectile = CreateProjectile();
        if (Projectile)
        {
            PooledProjectiles.Add(Projectile);
        }
    }

    UE_LOG(LogAdastrea, Log, TEXT("ProjectilePoolComponent: Pool warmup complete. Pooled: %d, Total Created: %d"),
        PooledProjectiles.Num(), TotalCreatedProjectiles);
}

void UProjectilePoolComponent::InitializePool()
{
    if (!ProjectileClass)
    {
        return;
    }

    UE_LOG(LogAdastrea, Log, TEXT("ProjectilePoolComponent: Initializing pool with %d projectiles"), InitialPoolSize);

    // Pre-create initial projectiles
    for (int32 i = 0; i < InitialPoolSize && TotalCreatedProjectiles < MaxPoolSize; ++i)
    {
        AProjectile* Projectile = CreateProjectile();
        if (Projectile)
        {
            PooledProjectiles.Add(Projectile);
        }
    }

    UE_LOG(LogAdastrea, Log, TEXT("ProjectilePoolComponent: Pool initialized. Created %d projectiles"), PooledProjectiles.Num());
}

AProjectile* UProjectilePoolComponent::CreateProjectile()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogAdastrea, Error, TEXT("ProjectilePoolComponent: Cannot create projectile - no valid world"));
        return nullptr;
    }

    // Spawn projectile at safe location
    FVector SpawnLocation = FVector(0, 0, -10000); // Below world
    FRotator SpawnRotation = FRotator::ZeroRotator;

    AProjectile* Projectile = World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
    if (Projectile)
    {
        // Configure for pooling
        Projectile->SetActorHiddenInGame(true);
        Projectile->SetActorEnableCollision(false);

        TotalCreatedProjectiles++;

        UE_LOG(LogAdastrea, Verbose, TEXT("ProjectilePoolComponent: Created projectile %s"), *Projectile->GetName());
    }
    else
    {
        UE_LOG(LogAdastrea, Error, TEXT("ProjectilePoolComponent: Failed to create projectile"));
    }

    return Projectile;
}

void UProjectilePoolComponent::ResetProjectile(AProjectile* Projectile)
{
    if (!IsValid(Projectile))
    {
        return;
    }

    // Reset projectile to default state
    // This would need to be customized based on your projectile implementation
    Projectile->ResetToPoolState();

    // Reset common properties
    Projectile->SetActorRotation(FRotator::ZeroRotator);
    Projectile->SetActorScale3D(FVector::OneVector);
}