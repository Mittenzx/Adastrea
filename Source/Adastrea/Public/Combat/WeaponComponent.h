#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponDataAsset.h"
#include "WeaponComponent.generated.h"

/**
 * Component for managing weapon mounting and firing on spaceships
 * 
 * Handles weapon state, ammunition, heat management, and firing logic.
 * Designed to be attached to ship actors and configured via WeaponDataAsset.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Assign WeaponDataAsset
 * - Configure mount location and size
 * - Call Fire() from input or AI logic
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Weapon data asset defining this weapon's properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    UWeaponDataAsset* WeaponData;

    /** Mount size for this hardpoint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
    EWeaponMountSize MountSize;

    /** Relative location of weapon mount on ship */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    FVector MountLocation;

    /** Relative rotation of weapon mount on ship */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    FRotator MountRotation;

    /** Whether this is a turret (can rotate) or fixed mount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    bool bIsTurret;

    // ====================
    // STATE
    // ====================

    /** Current ammunition remaining (for projectile/missile weapons) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon State")
    int32 CurrentAmmo;

    /** Current heat level (0-100) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon State")
    float CurrentHeat;

    /** Whether weapon is currently reloading */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon State")
    bool bIsReloading;

    /** Whether weapon is overheated and cannot fire */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon State")
    bool bIsOverheated;

    /** Current target actor (if any) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon State")
    AActor* CurrentTarget;

protected:
    /** Time since last shot (for rate of fire management) */
    float TimeSinceLastShot;

    /** Time remaining for reload */
    float ReloadTimeRemaining;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // WEAPON OPERATIONS
    // ====================

    /**
     * Attempt to fire the weapon
     * @param Target Optional target actor for homing weapons
     * @return True if weapon fired successfully
     */
    UFUNCTION(BlueprintCallable, Category="Weapon")
    bool Fire(AActor* Target = nullptr);

    /**
     * Start reloading the weapon
     * @return True if reload started successfully
     */
    UFUNCTION(BlueprintCallable, Category="Weapon")
    bool StartReload();

    /**
     * Set the target for this weapon
     * @param Target Actor to target
     */
    UFUNCTION(BlueprintCallable, Category="Weapon")
    void SetTarget(AActor* Target);

    /**
     * Clear the current target
     */
    UFUNCTION(BlueprintCallable, Category="Weapon")
    void ClearTarget();

    /**
     * Check if weapon can fire
     * @return True if weapon is ready to fire
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    bool CanFire() const;

    /**
     * Check if weapon needs reload
     * @return True if weapon is out of ammo and can reload
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    bool NeedsReload() const;

    /**
     * Get the weapon's current aim direction
     * @return World space direction vector
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    FVector GetAimDirection() const;

    /**
     * Get the weapon's world position
     * @return World space position of weapon mount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    FVector GetWeaponWorldPosition() const;

    /**
     * Check if weapon can hit target at current range
     * @param Target Target actor
     * @return True if target is within effective range
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    bool IsTargetInRange(AActor* Target) const;

    /**
     * Get ammunition percentage remaining (0-1)
     * @return Ammo percentage (1.0 for energy weapons)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    float GetAmmoPercentage() const;

    /**
     * Get heat percentage (0-1)
     * @return Current heat as percentage of maximum
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon")
    float GetHeatPercentage() const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when weapon fires
     * Override in Blueprint for visual/audio effects
     * @param Target The target being fired at (may be nullptr)
     */
    UFUNCTION(BlueprintNativeEvent, Category="Weapon")
    void OnWeaponFired(AActor* Target);

    /**
     * Called when weapon starts reloading
     * Override in Blueprint for reload effects
     */
    UFUNCTION(BlueprintNativeEvent, Category="Weapon")
    void OnReloadStarted();

    /**
     * Called when weapon finishes reloading
     * Override in Blueprint for reload complete effects
     */
    UFUNCTION(BlueprintNativeEvent, Category="Weapon")
    void OnReloadCompleted();

    /**
     * Called when weapon overheats
     * Override in Blueprint for overheat effects
     */
    UFUNCTION(BlueprintNativeEvent, Category="Weapon")
    void OnWeaponOverheated();

private:
    /** Handle weapon cooling over time */
    void UpdateCooling(float DeltaTime);

    /** Handle reload timer */
    void UpdateReload(float DeltaTime);

    /** Apply damage to target */
    void ApplyDamageToTarget(AActor* Target);
};
