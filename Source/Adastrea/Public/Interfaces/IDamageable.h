#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
// NOTE: Combat system archived - EDamageType will be reimplemented in MVP for hazards
// Full combat system (weapons, projectiles) is POST-MVP
// #include "Combat/WeaponDataAsset.h"
#include "IDamageable.generated.h"

// Forward declaration for damage type enum
// MVP USAGE: Environmental hazards only (no combat weapons)
// - Kinetic: Docking accidents, collisions
// - Energy: Solar radiation, station system failures
// - Thermal: Engine overheating, star proximity
// POST-MVP: Full weapon system implementation
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
 * MVP SCOPE (Trade Simulator):
 * - Environmental hazards (docking accidents, radiation, collisions)
 * - Basic ship durability for trading risks
 * - No weapons or active combat
 * 
 * POST-MVP SCOPE:
 * - Full combat system with weapons and projectiles
 * - NPC pirate encounters
 * - Station defense systems
 * 
 * Implement this interface on:
 * - Spaceships (ASpaceship) - MVP: Environmental damage only
 * - Space stations (ASpaceStation) - POST-MVP: Combat implementation
 * - Station modules (ASpaceStationModule) - POST-MVP: Combat implementation
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
 * // MVP: Apply environmental damage
 * if (Target->Implements<UDamageable>())
 * {
 *     IDamageable* DamageableTarget = Cast<IDamageable>(Target);
 *     float DamageDealt = DamageableTarget->Execute_ApplyDamage(
 *         Target, 10.0f, EDamageType::Thermal, nullptr, nullptr
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
     * MVP USAGE (Trade Simulator):
     * - Environmental hazards only (docking accidents, radiation, collisions)
     * - Instigator/DamageCauser can be nullptr for environmental damage
     * - Basic shield/hull damage calculation
     * 
     * POST-MVP USAGE:
     * - Full weapon system implementation
     * - Instigator = attacking ship, DamageCauser = projectile/weapon
     * - Complex armor penetration calculations
     * 
     * This is the primary method for dealing damage. Implementations should:
     * - Apply damage to shields first (if present)
     * - Apply remaining damage to hull
     * - Consider armor penetration and damage type
     * - Trigger death/destruction if health reaches zero
     * - Broadcast damage events for AI/UI
     * 
     * @param Damage Amount of raw damage to apply
     * @param DamageType Type of damage (MVP: Kinetic, Thermal, Energy for hazards)
     * @param Instigator Actor that initiated the damage (MVP: Can be nullptr for environment)
     * @param DamageCauser Direct cause of damage (MVP: Can be nullptr for environment)
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
