#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
// TODO: Combat system archived - EDamageType will be reimplemented in MVP
// #include "Combat/WeaponDataAsset.h"
#include "IDamageable.generated.h"

// Forward declaration for damage type enum
// TODO: This will be moved back to WeaponDataAsset when combat system is reimplemented
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Kinetic     UMETA(DisplayName = "Kinetic"),
    Energy      UMETA(DisplayName = "Energy"),
    Explosive   UMETA(DisplayName = "Explosive"),
    Thermal     UMETA(DisplayName = "Thermal"),
    EMP         UMETA(DisplayName = "EMP")
};

/**
 * UInterface class for IDamageable
 * Required by Unreal's reflection system
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can receive damage
 * 
 * Implement this interface on:
 * - Spaceships (ASpaceship)
 * - Space stations (ASpaceStation)
 * - Station modules (ASpaceStationModule)
 * - Any destructible objects
 * 
 * Benefits:
 * - Unified damage handling across all damageable entities
 * - Easy to add new damageable types without modifying weapon code
 * - Blueprint-friendly with full visual scripting support
 * - Polymorphic damage application
 * 
 * Usage Example (C++):
 * @code
 * if (Target->Implements<UDamageable>())
 * {
 *     IDamageable* DamageableTarget = Cast<IDamageable>(Target);
 *     float DamageDealt = DamageableTarget->Execute_ApplyDamage(
 *         Target, Damage, DamageType, Instigator, DamageCauser
 *     );
 * }
 * @endcode
 * 
 * Usage Example (Blueprint):
 * - Use "Does Implement Interface" node to check
 * - Call interface methods directly on any actor implementing IDamageable
 */
class ADASTREA_API IDamageable
{
    GENERATED_BODY()

public:
    /**
     * Apply damage to this actor
     * 
     * This is the primary method for dealing damage. Implementations should:
     * - Apply damage to shields first (if present)
     * - Apply remaining damage to hull
     * - Consider armor penetration and damage type
     * - Trigger death/destruction if health reaches zero
     * - Broadcast damage events for AI/UI
     * 
     * @param Damage Amount of raw damage to apply
     * @param DamageType Type of damage (Kinetic, Energy, Explosive, Thermal, EMP)
     * @param Instigator Actor that initiated the damage (the ship firing the weapon)
     * @param DamageCauser Direct cause of damage (projectile, explosion effect, etc.)
     * @return Actual damage applied after armor/shields (for feedback/logging)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float ApplyDamage(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser);

    /**
     * Check if this actor can currently take damage
     * 
     * Implementations should return false if:
     * - Actor is already destroyed
     * - Actor is invulnerable (quest protection, etc.)
     * - Actor is in a safe zone
     * - Actor is currently docked and protected
     * 
     * @return True if the actor can take damage, false otherwise
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool CanTakeDamage() const;

    /**
     * Get current health as a percentage (0.0 to 1.0)
     * 
     * Useful for:
     * - UI health bars
     * - AI threat assessment
     * - Determining retreat thresholds
     * - Visual damage effects
     * 
     * @return Health percentage from 0.0 (dead) to 1.0 (full health)
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float GetHealthPercentage() const;

    /**
     * Check if this actor is destroyed or dead
     * 
     * Should return true if:
     * - Health has reached zero
     * - Actor is pending destruction
     * - Actor has been critically damaged
     * 
     * @return True if destroyed, false if still operational
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    bool IsDestroyed() const;

    /**
     * Get maximum health value
     * 
     * Used for damage calculation percentages and UI display
     * 
     * @return Maximum possible health value
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float GetMaxHealth() const;

    /**
     * Get current health value
     * 
     * Used for damage calculation and status display
     * 
     * @return Current health value
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Damage")
    float GetCurrentHealth() const;
};
