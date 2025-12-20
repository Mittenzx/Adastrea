// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// Forward declarations
class AProjectile;

/**
 * Object pooling component for projectiles to reduce GC pressure and improve performance.
 * Manages a pool of reusable projectile actors to avoid frequent spawn/destroy operations.
 *
 * Usage:
 * 1. Add to weapon or combat system actor
 * 2. Configure pool size and projectile class
 * 3. Call AcquireProjectile() instead of spawning
 * 4. Call ReturnProjectile() when projectile is done
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UProjectilePoolComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UProjectilePoolComponent();

    /** Initialize the pool with the configured settings */
    virtual void InitializeComponent() override;

    /** Clean up pool when component is destroyed */
    virtual void UninitializeComponent() override;

    //================================================================================
    // Pool Configuration
    //================================================================================

    /** Initial number of projectiles to create in the pool */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pool Configuration",
        meta=(ClampMin="10", ClampMax="1000"))
    int32 InitialPoolSize = 50;

    /** Maximum number of projectiles allowed in the pool */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pool Configuration",
        meta=(ClampMin="50", ClampMax="2000"))
    int32 MaxPoolSize = 200;

    /** Projectile class to instantiate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pool Configuration")
    TSubclassOf<AProjectile> ProjectileClass;

    //================================================================================
    // Pool Management
    //================================================================================

    /**
     * Get a projectile from the pool or create new if needed
     * @return A ready-to-use projectile, or nullptr if pool is exhausted
     */
    UFUNCTION(BlueprintCallable, Category="Projectile Pool")
    AProjectile* AcquireProjectile();

    /**
     * Return a projectile to the pool for reuse
     * @param Projectile The projectile to return
     */
    UFUNCTION(BlueprintCallable, Category="Projectile Pool")
    void ReturnProjectile(AProjectile* Projectile);

    /**
     * Get current pool statistics
     * @param OutActiveCount Number of active projectiles
     * @param OutPooledCount Number of pooled projectiles
     * @param OutTotalCount Total projectiles created
     */
    UFUNCTION(BlueprintCallable, Category="Projectile Pool")
    void GetPoolStats(int32& OutActiveCount, int32& OutPooledCount, int32& OutTotalCount) const;

    /**
     * Pre-warm the pool by creating initial projectiles
     */
    UFUNCTION(BlueprintCallable, Category="Projectile Pool")
    void WarmupPool();

private:
    /** Pool of inactive projectiles ready for reuse */
    UPROPERTY()
    TArray<AProjectile*> PooledProjectiles;

    /** Currently active projectiles */
    UPROPERTY()
    TArray<AProjectile*> ActiveProjectiles;

    /** Total projectiles created (for statistics) */
    int32 TotalCreatedProjectiles;

    /** Initialize the projectile pool */
    void InitializePool();

    /** Create a new projectile and add it to the pool */
    AProjectile* CreateProjectile();

    /** Reset a projectile to its default state */
    void ResetProjectile(AProjectile* Projectile);
};

#include "ProjectilePoolComponent.generated.h"