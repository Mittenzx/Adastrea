// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class ADASTREA_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();

    /** Initialize the projectile with direction and speed */
    void Initialize(const FVector& Direction, float Speed);

    /** Reset projectile to its initial pooled state */
    void ResetToPoolState();

    /** Called when projectile hits something */
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
    virtual void BeginPlay() override;

    /** Movement component for the projectile */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UProjectileMovementComponent* ProjectileMovement;

    /** Collision component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    /** Visual representation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    /** Damage to apply on hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage = 10.0f;

    /** Lifetime of the projectile in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Lifetime = 5.0f;

private:
    /** Timer handle for lifetime */
    FTimerHandle LifetimeTimerHandle;

    /** Destroy the projectile */
    void DestroyProjectile();
};