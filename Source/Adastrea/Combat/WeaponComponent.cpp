#include "Combat/WeaponComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UWeaponComponent::UWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_DuringPhysics;

    // Default configuration
    WeaponData = nullptr;
    MountSize = EWeaponMountSize::Small;
    MountLocation = FVector::ZeroVector;
    MountRotation = FRotator::ZeroRotator;
    bIsTurret = false;

    // Default state
    CurrentAmmo = 0;
    CurrentHeat = 0.0f;
    bIsReloading = false;
    bIsOverheated = false;
    CurrentTarget = nullptr;
    TimeSinceLastShot = 0.0f;
    ReloadTimeRemaining = 0.0f;
}

void UWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize ammunition from weapon data
    if (WeaponData)
    {
        CurrentAmmo = WeaponData->AmmoCapacity;
    }
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update timers
    TimeSinceLastShot += DeltaTime;

    // Handle cooling
    UpdateCooling(DeltaTime);

    // Handle reload
    UpdateReload(DeltaTime);
}

bool UWeaponComponent::Fire(AActor* Target)
{
    // Validate weapon data
    if (!WeaponData)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("WeaponComponent: Cannot fire - no WeaponData assigned"));
        return false;
    }

    // Check if can fire
    if (!CanFire())
    {
        return false;
    }

    // Check rate of fire
    const float FireInterval = 1.0f / WeaponData->RateOfFire;
    if (TimeSinceLastShot < FireInterval)
    {
        return false;
    }

    // Consume ammunition for projectile/missile weapons
    if (WeaponData->AmmoCapacity > 0)
    {
        if (CurrentAmmo <= 0)
        {
            StartReload();
            return false;
        }
        CurrentAmmo--;
    }

    // Generate heat
    CurrentHeat += WeaponData->HeatGeneration;
    if (CurrentHeat >= 100.0f)
    {
        CurrentHeat = 100.0f;
        bIsOverheated = true;
        OnWeaponOverheated();
    }

    // Reset fire timer
    TimeSinceLastShot = 0.0f;

    // Set target if provided
    if (Target)
    {
        CurrentTarget = Target;
    }

    // Apply damage to target
    if (CurrentTarget)
    {
        ApplyDamageToTarget(CurrentTarget);
    }

    // Trigger fire event
    OnWeaponFired(CurrentTarget);

    return true;
}

bool UWeaponComponent::StartReload()
{
    if (!WeaponData)
    {
        return false;
    }

    // Energy weapons don't reload
    if (WeaponData->AmmoCapacity <= 0)
    {
        return false;
    }

    // Already reloading
    if (bIsReloading)
    {
        return false;
    }

    // Already at full ammo
    if (CurrentAmmo >= WeaponData->AmmoCapacity)
    {
        return false;
    }

    // Start reload
    bIsReloading = true;
    ReloadTimeRemaining = WeaponData->ReloadTime;
    OnReloadStarted();

    return true;
}

void UWeaponComponent::SetTarget(AActor* Target)
{
    CurrentTarget = Target;
}

void UWeaponComponent::ClearTarget()
{
    CurrentTarget = nullptr;
}

bool UWeaponComponent::CanFire() const
{
    if (!WeaponData)
    {
        return false;
    }

    // Cannot fire while reloading
    if (bIsReloading)
    {
        return false;
    }

    // Cannot fire when overheated
    if (bIsOverheated)
    {
        return false;
    }

    // Check ammunition for projectile/missile weapons
    if (WeaponData->AmmoCapacity > 0 && CurrentAmmo <= 0)
    {
        return false;
    }

    return true;
}

bool UWeaponComponent::NeedsReload() const
{
    if (!WeaponData)
    {
        return false;
    }

    // Energy weapons don't need reload
    if (WeaponData->AmmoCapacity <= 0)
    {
        return false;
    }

    return CurrentAmmo <= 0 && !bIsReloading;
}

FVector UWeaponComponent::GetAimDirection() const
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return FVector::ForwardVector;
    }

    // Get weapon world rotation
    const FRotator WorldRotation = Owner->GetActorRotation() + MountRotation;
    return WorldRotation.Vector();
}

FVector UWeaponComponent::GetWeaponWorldPosition() const
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    // Transform mount location to world space
    return Owner->GetActorTransform().TransformPosition(MountLocation);
}

bool UWeaponComponent::IsTargetInRange(AActor* Target) const
{
    if (!Target || !WeaponData)
    {
        return false;
    }

    const FVector WeaponPos = GetWeaponWorldPosition();
    const FVector TargetPos = Target->GetActorLocation();
    const float Distance = FVector::Dist(WeaponPos, TargetPos);

    return Distance <= WeaponData->MaxRange;
}

float UWeaponComponent::GetAmmoPercentage() const
{
    if (!WeaponData || WeaponData->AmmoCapacity <= 0)
    {
        return 1.0f;  // Energy weapons always at "full"
    }

    return static_cast<float>(CurrentAmmo) / static_cast<float>(WeaponData->AmmoCapacity);
}

float UWeaponComponent::GetHeatPercentage() const
{
    return FMath::Clamp(CurrentHeat / 100.0f, 0.0f, 1.0f);
}

void UWeaponComponent::OnWeaponFired_Implementation(AActor* Target)
{
    // Default implementation - override in Blueprint for effects
    UE_LOG(LogAdastrea, Log, TEXT("Weapon fired at %s"), Target ? *Target->GetName() : TEXT("no target"));
}

void UWeaponComponent::OnReloadStarted_Implementation()
{
    // Default implementation - override in Blueprint for effects
    UE_LOG(LogAdastrea, Log, TEXT("Weapon reload started"));
}

void UWeaponComponent::OnReloadCompleted_Implementation()
{
    // Default implementation - override in Blueprint for effects
    UE_LOG(LogAdastrea, Log, TEXT("Weapon reload completed"));
}

void UWeaponComponent::OnWeaponOverheated_Implementation()
{
    // Default implementation - override in Blueprint for effects
    UE_LOG(LogAdastrea, Warning, TEXT("Weapon overheated"));
}

void UWeaponComponent::UpdateCooling(float DeltaTime)
{
    if (!WeaponData)
    {
        return;
    }

    // Cool down weapon
    if (CurrentHeat > 0.0f)
    {
        CurrentHeat -= WeaponData->CoolingRate * DeltaTime;
        CurrentHeat = FMath::Max(CurrentHeat, 0.0f);

        // Clear overheat flag when cooled below threshold
        if (bIsOverheated && CurrentHeat < 50.0f)
        {
            bIsOverheated = false;
        }
    }
}

void UWeaponComponent::UpdateReload(float DeltaTime)
{
    if (!bIsReloading)
    {
        return;
    }

    ReloadTimeRemaining -= DeltaTime;

    if (ReloadTimeRemaining <= 0.0f)
    {
        // Reload complete
        bIsReloading = false;
        ReloadTimeRemaining = 0.0f;

        if (WeaponData)
        {
            CurrentAmmo = WeaponData->AmmoCapacity;
        }

        OnReloadCompleted();
    }
}

void UWeaponComponent::ApplyDamageToTarget(AActor* Target)
{
    if (!Target || !WeaponData)
    {
        return;
    }

    // Calculate base damage
    float DamageAmount = WeaponData->BaseDamage;

    // Apply critical hit
    const float RandomValue = FMath::FRand();
    if (RandomValue < WeaponData->CriticalChance)
    {
        DamageAmount *= WeaponData->CriticalMultiplier;
        UE_LOG(LogAdastrea, Log, TEXT("Critical hit! Damage: %.1f"), DamageAmount);
    }

    // Apply distance-based accuracy falloff
    const FVector WeaponPos = GetWeaponWorldPosition();
    const FVector TargetPos = Target->GetActorLocation();
    const float Distance = FVector::Dist(WeaponPos, TargetPos);

    // Accuracy decreases beyond optimal range
    float AccuracyModifier = 1.0f;
    if (Distance > WeaponData->OptimalRange)
    {
        const float RangeFactor = (Distance - WeaponData->OptimalRange) / (WeaponData->MaxRange - WeaponData->OptimalRange);
        AccuracyModifier = FMath::Lerp(1.0f, 0.5f, FMath::Clamp(RangeFactor, 0.0f, 1.0f));
    }

    // Apply accuracy check
    const float HitChance = (WeaponData->Accuracy / 100.0f) * AccuracyModifier;
    if (FMath::FRand() > HitChance)
    {
        UE_LOG(LogAdastrea, Log, TEXT("Weapon missed target"));
        return;
    }

    // Apply damage through Unreal's damage system
    UGameplayStatics::ApplyDamage(
        Target,
        DamageAmount,
        GetOwner() ? GetOwner()->GetInstigatorController() : nullptr,
        GetOwner(),
        UDamageType::StaticClass()
    );

    UE_LOG(LogAdastrea, Log, TEXT("Weapon dealt %.1f damage to %s"), DamageAmount, *Target->GetName());
}
