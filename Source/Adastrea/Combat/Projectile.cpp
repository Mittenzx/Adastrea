// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create collision component
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    RootComponent = CollisionComponent;

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create projectile movement component
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->SetUpdatedComponent(CollisionComponent);
    ProjectileMovement->InitialSpeed = 3000.0f;
    ProjectileMovement->MaxSpeed = 3000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity for space combat
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();

    // Set up lifetime timer
    if (Lifetime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AProjectile::DestroyProjectile, Lifetime, false);
    }
}

void AProjectile::Initialize(const FVector& Direction, float Speed)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
    }
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Apply damage if we hit something damageable
    if (OtherActor && OtherActor != GetOwner())
    {
        UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, GetOwner(), nullptr);
    }

    // Destroy the projectile
    DestroyProjectile();
}

void AProjectile::DestroyProjectile()
{
    // Clear the timer if it's still active
    if (LifetimeTimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
    }

    // Return to pool instead of destroying
    // This will be handled by the ProjectilePoolComponent
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
    }
}