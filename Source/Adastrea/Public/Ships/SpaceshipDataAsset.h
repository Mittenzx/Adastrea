#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpaceshipDataAsset.generated.h"

/**
 * Comprehensive Data Asset for storing spaceship specifications.
 * This allows designers to create spaceship configurations as Blueprint Data Assets.
 * Includes stats for combat, mobility, utility, operations, and lore.
 */
UCLASS(BlueprintType)
class ADASTREA_API USpaceshipDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ====================
    // BASIC INFO
    // ====================

    // Display name of the spaceship
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText ShipName;

    // Brief description of the spaceship
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
    FText Description;

    // Ship class/type (e.g., Scout, Gunship, Freighter)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText ShipClass;

    // Unique identifier for the ship type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FName ShipID;

    // ====================
    // CORE STATS
    // ====================

    // Maximum hull strength (health points)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="1", ClampMax="100000"))
    float HullStrength;

    // Cargo capacity in tons
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="0", ClampMax="50000"))
    float CargoCapacity;

    // Minimum crew required to operate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="1", ClampMax="10000"))
    int32 CrewRequired;

    // Maximum crew capacity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="1", ClampMax="10000"))
    int32 MaxCrew;

    // Modular points for customization
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Core Stats", meta=(ClampMin="0", ClampMax="100"))
    int32 ModularPoints;

    // ====================
    // COMBAT STATS
    // ====================

    // Armor rating (damage reduction)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="1000"))
    float ArmorRating;

    // Shield strength (additional health layer)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="100000"))
    float ShieldStrength;

    // Shield recharge rate per second
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="1000"))
    float ShieldRechargeRate;

    // Number of weapon hardpoints
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="50"))
    int32 WeaponSlots;

    // Total weapon power capacity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="10000"))
    float WeaponPowerCapacity;

    // Point defense rating (anti-missile/fighter)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat Stats", meta=(ClampMin="0", ClampMax="100"))
    float PointDefenseRating;

    // ====================
    // MOBILITY STATS
    // ====================

    // Maximum speed in m/s
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="0", ClampMax="10000"))
    float MaxSpeed;

    // Acceleration in m/s²
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="0", ClampMax="1000"))
    float Acceleration;

    // Maneuverability rating (1-10, affects turn rate)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="1", ClampMax="10"))
    int32 Maneuverability;

    // Jump range in light years
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="0", ClampMax="1000"))
    float JumpRange;

    // Fuel capacity in units
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="0", ClampMax="100000"))
    float FuelCapacity;

    // Fuel consumption rate per jump
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", meta=(ClampMin="0", ClampMax="10000"))
    float FuelConsumptionRate;

    // ====================
    // UTILITY/SUPPORT STATS
    // ====================

    // Sensor range in units
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="0", ClampMax="100000"))
    float SensorRange;

    // Sensor resolution (1-10, affects detail)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="1", ClampMax="10"))
    int32 SensorResolution;

    // Stealth rating (1-10, reduces detection)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="1", ClampMax="10"))
    int32 StealthRating;

    // Repair system efficiency (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 RepairSystemRating;

    // Science lab capabilities (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 ScienceRating;

    // Medical bay capabilities (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 MedicalRating;

    // ====================
    // OPERATIONAL STATS
    // ====================

    // Power generation capacity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="0", ClampMax="100000"))
    float PowerCapacity;

    // Life support system rating (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="1", ClampMax="10"))
    int32 LifeSupportRating;

    // Maintenance requirements (lower is better)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="1", ClampMax="10"))
    int32 MaintenanceLevel;

    // Hangar bay capacity (number of small craft)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="0", ClampMax="100"))
    int32 HangarCapacity;

    // Number of drones/probes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="0", ClampMax="1000"))
    int32 DroneCapacity;

    // AI system capabilities (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Operational Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 AISystemRating;

    // ====================
    // ADVANCED/SPECIAL STATS
    // ====================

    // Diplomacy facilities (1-10, for negotiations)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 DiplomacyRating;

    // Boarding party capabilities (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 BoardingCapability;

    // Number of customizable module slots
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced Stats", meta=(ClampMin="0", ClampMax="20"))
    int32 CustomizableSlots;

    // Electronic warfare capabilities (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 EWarRating;

    // Mining/resource extraction capabilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced Stats", meta=(ClampMin="0", ClampMax="10"))
    int32 MiningRating;

    // ====================
    // LORE/FLAVOR
    // ====================

    // Ship manufacturer
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FText Manufacturer;

    // Year built/designed
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    int32 YearBuilt;

    // Rarity tier (Common, Uncommon, Rare, Legendary, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FText RarityTier;

    // Notable features or lore notes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore", meta=(MultiLine=true))
    FText LoreNotes;

    // Primary color scheme
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FLinearColor PrimaryColor;

    // Secondary color scheme
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
    FLinearColor SecondaryColor;

    // ====================
    // Constructor
    // ====================

    USpaceshipDataAsset();

    // ====================
    // Blueprint Callable Functions
    // ====================

    /**
     * Get the overall combat effectiveness rating (0-100)
     * Calculated from armor, shields, weapons, and point defense
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    float GetCombatRating() const;

    /**
     * Get the overall mobility rating (0-100)
     * Calculated from speed, acceleration, maneuverability, and jump range
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    float GetMobilityRating() const;

    /**
     * Get the overall utility rating (0-100)
     * Calculated from sensors, repair, science, and medical systems
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    float GetUtilityRating() const;

    /**
     * Check if ship is suitable for a specific role
     * @param Role The role to check (e.g., "Combat", "Trade", "Exploration", "Mining")
     * @return True if ship is well-suited for this role
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    bool IsSuitableForRole(const FString& Role) const;

    /**
     * Get ship size category based on hull strength and crew
     * @return Size category (Fighter, Corvette, Frigate, Cruiser, Battleship, Capital)
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    FString GetSizeCategory() const;

    /**
     * Calculate operational cost per day
     * Based on crew, maintenance, and power requirements
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
    float GetOperationalCost() const;

protected:
    // ====================
    // CALCULATION CACHING (Phase 2 Optimization)
    // ====================

    /** 
     * Cached rating values - marked Transient so they're not serialized
     * 
     * Thread Safety: Data Assets are typically accessed from the game thread only.
     * If multi-threaded access is needed, external synchronization should be used.
     * Cache updates are infrequent (only on property changes) so race conditions are unlikely.
     */
    UPROPERTY(Transient)
    mutable float CachedCombatRating;

    UPROPERTY(Transient)
    mutable float CachedMobilityRating;

    UPROPERTY(Transient)
    mutable float CachedUtilityRating;

    /** Dirty flag for cache invalidation */
    UPROPERTY(Transient)
    mutable bool bRatingsCacheDirty;

    /**
     * Invalidate cached ratings (called when properties change)
     */
    void InvalidateRatingsCache();

    /**
     * Calculate and cache all ratings at once
     */
    void UpdateRatingsCache() const;

    /**
     * Internal rating calculation methods (moved from public implementations)
     */
    float CalculateCombatRatingInternal() const;
    float CalculateMobilityRatingInternal() const;
    float CalculateUtilityRatingInternal() const;

public:
#if WITH_EDITOR
    /**
     * Validate Data Asset properties
     * Called when asset is saved or validated in editor
     * Checks for:
     * - Required fields (name, class, ID)
     * - Logical constraints (crew, capacity, etc.)
     * - Stat ranges and consistency
     */
    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;

    /** Invalidate cache when properties change in editor */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
