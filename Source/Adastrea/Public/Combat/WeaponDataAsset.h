#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

/**
 * Enum defining different weapon types
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Energy      UMETA(DisplayName = "Energy Weapon"),
    Projectile  UMETA(DisplayName = "Projectile Weapon"),
    Missile     UMETA(DisplayName = "Missile Launcher"),
    Torpedo     UMETA(DisplayName = "Torpedo Launcher"),
    Beam        UMETA(DisplayName = "Beam Weapon"),
    Plasma      UMETA(DisplayName = "Plasma Weapon")
};

/**
 * Enum defining damage types
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Kinetic     UMETA(DisplayName = "Kinetic"),
    Energy      UMETA(DisplayName = "Energy"),
    Explosive   UMETA(DisplayName = "Explosive"),
    Thermal     UMETA(DisplayName = "Thermal"),
    EMP         UMETA(DisplayName = "Electromagnetic Pulse")
};

/**
 * Enum defining weapon mount sizes
 */
UENUM(BlueprintType)
enum class EWeaponMountSize : uint8
{
    Small       UMETA(DisplayName = "Small"),
    Medium      UMETA(DisplayName = "Medium"),
    Large       UMETA(DisplayName = "Large"),
    Capital     UMETA(DisplayName = "Capital")
};

/**
 * Data Asset for weapon configurations
 * 
 * Defines all properties for a weapon type including damage, range, power requirements,
 * and firing characteristics. Allows designers to create weapon configurations as
 * Blueprint Data Assets without C++ knowledge.
 * 
 * Usage:
 * - Create a Blueprint based on this class
 * - Configure weapon properties in editor
 * - Assign to WeaponComponent on ships
 */
UCLASS(BlueprintType)
class ADASTREA_API UWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ====================
    // BASIC INFO
    // ====================

    /** Display name of the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText WeaponName;

    /** Brief description of the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
    FText Description;

    /** Unique identifier for the weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FName WeaponID;

    /** Type of weapon (energy, projectile, missile, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    EWeaponType WeaponType;

    /** Type of damage dealt */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    EDamageType DamageType;

    /** Required mount size */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    EWeaponMountSize MountSize;

    // ====================
    // DAMAGE STATS
    // ====================

    /** Base damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0", ClampMax="10000"))
    float BaseDamage;

    /** Armor penetration rating (0-100, affects damage vs armored targets) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0", ClampMax="100"))
    float ArmorPenetration;

    /** Shield penetration rating (0-100, affects damage vs shields) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0", ClampMax="100"))
    float ShieldPenetration;

    /** Critical hit chance (0-1, probability of critical hit) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0", ClampMax="1.0"))
    float CriticalChance;

    /** Critical hit damage multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="1.0", ClampMax="10.0"))
    float CriticalMultiplier;

    // ====================
    // FIRING CHARACTERISTICS
    // ====================

    /** Rate of fire (shots per second) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="0.1", ClampMax="100"))
    float RateOfFire;

    /** Maximum effective range in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="100", ClampMax="100000"))
    float MaxRange;

    /** Optimal range for maximum accuracy in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="100", ClampMax="100000"))
    float OptimalRange;

    /** Projectile/beam speed in m/s (0 for instant hit) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="0", ClampMax="10000"))
    float ProjectileSpeed;

    /** Tracking speed for turrets (degrees per second, 0 for fixed mount) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="0", ClampMax="360"))
    float TrackingSpeed;

    /** Accuracy rating (0-100, affects hit chance) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Firing", meta=(ClampMin="0", ClampMax="100"))
    float Accuracy;

    // ====================
    // RESOURCE REQUIREMENTS
    // ====================

    /** Power consumption per shot */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resources", meta=(ClampMin="0", ClampMax="10000"))
    float PowerPerShot;

    /** Ammunition capacity (0 for energy weapons) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resources", meta=(ClampMin="0", ClampMax="10000"))
    int32 AmmoCapacity;

    /** Reload time in seconds (0 for energy weapons) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resources", meta=(ClampMin="0", ClampMax="60"))
    float ReloadTime;

    /** Heat generation per shot (affects overheat mechanics) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resources", meta=(ClampMin="0", ClampMax="100"))
    float HeatGeneration;

    /** Cooling rate (heat removed per second) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Resources", meta=(ClampMin="0", ClampMax="100"))
    float CoolingRate;

    // ====================
    // SPECIAL PROPERTIES
    // ====================

    /** Can target missiles and fighters (point defense capability) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special")
    bool bPointDefenseCapable;

    /** Area of effect radius for explosive weapons (0 for single target) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special", meta=(ClampMin="0", ClampMax="1000"))
    float AOERadius;

    /** EMP effect strength (0-100, chance to disable systems) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special", meta=(ClampMin="0", ClampMax="100"))
    float EMPStrength;

    /** Homing capability for missiles (0 = no homing, 10 = perfect tracking) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special", meta=(ClampMin="0", ClampMax="10"))
    int32 HomingRating;

    // ====================
    // LORE/FLAVOR
    // ====================

    /** Weapon manufacturer */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FText Manufacturer;

    /** Tech level required to use */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore", meta=(ClampMin="1", ClampMax="10"))
    int32 TechLevel;

    /** Rarity tier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FText RarityTier;

    // ====================
    // Constructor
    // ====================

    UWeaponDataAsset();

    // ====================
    // Blueprint Callable Functions
    // ====================

    /**
     * Get the damage per second output
     * @return Damage per second (BaseDamage * RateOfFire)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon Stats")
    float GetDPS() const;

    /**
     * Calculate effective damage against a target with given armor/shield
     * @param ArmorRating Target's armor rating
     * @param ShieldStrength Target's current shield strength
     * @return Effective damage after penetration calculations
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon Stats")
    float GetEffectiveDamage(float ArmorRating, float ShieldStrength) const;

    /**
     * Get time to kill a target with given health
     * @param TargetHealth Total health of target
     * @return Time in seconds to destroy target
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon Stats")
    float GetTimeToKill(float TargetHealth) const;

    /**
     * Check if weapon is suitable for a specific role
     * @param Role The role to check (e.g., "AntiShip", "AntiMissile", "AntiArmor")
     * @return True if weapon is well-suited for this role
     */
    UFUNCTION(BlueprintCallable, Category="Weapon Stats")
    bool IsSuitableForRole(const FString& Role) const;

#if WITH_EDITOR
    /**
     * Validate weapon data asset properties
     * Checks for required fields and logical constraints
     */
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
