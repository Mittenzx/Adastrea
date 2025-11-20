#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponDataAsset.h"
#include "CombatHealthComponent.generated.h"

/**
 * Enum for shield facing directions
 */
UENUM(BlueprintType)
enum class EShieldFacing : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Aft         UMETA(DisplayName = "Aft"),
    Port        UMETA(DisplayName = "Port"),
    Starboard   UMETA(DisplayName = "Starboard"),
    Omnidirectional UMETA(DisplayName = "Omnidirectional")
};

/**
 * Struct for shield facing data
 */
USTRUCT(BlueprintType)
struct FShieldFacingData
{
    GENERATED_BODY()

    /** Current shield strength for this facing */
    UPROPERTY(BlueprintReadOnly)
    float CurrentStrength;

    /** Maximum shield strength for this facing */
    UPROPERTY(BlueprintReadOnly)
    float MaxStrength;

    /** Recharge delay remaining */
    UPROPERTY(BlueprintReadOnly)
    float RechargeDelay;

    FShieldFacingData()
        : CurrentStrength(0.0f)
        , MaxStrength(0.0f)
        , RechargeDelay(0.0f)
    {}
};

/**
 * Component for managing health, shields, armor, and damage for spaceships
 * 
 * Handles hull integrity, shield facings, damage calculation, armor penetration,
 * and destruction logic. Integrates with SpaceshipDataAsset for configuration.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Configure max health and shield values
 * - Receives damage through Unreal's damage system
 * - Broadcasts events for UI and gameplay responses
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UCombatHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatHealthComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Maximum hull strength (health) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health", meta=(ClampMin="1", ClampMax="100000"))
    float MaxHullStrength;

    /** Maximum shield strength (total across all facings) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(ClampMin="0", ClampMax="100000"))
    float MaxShieldStrength;

    /** Shield recharge rate per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(ClampMin="0", ClampMax="1000"))
    float ShieldRechargeRate;

    /** Delay before shields start recharging after taking damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(ClampMin="0", ClampMax="30"))
    float ShieldRechargeDelay;

    /** Armor rating (reduces damage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Armor", meta=(ClampMin="0", ClampMax="1000"))
    float ArmorRating;

    /** Use directional shields (4 facings) instead of omnidirectional */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield")
    bool bUseDirectionalShields;

    /** Percentage of shield strength per facing (total should equal 100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(EditCondition="bUseDirectionalShields", ClampMin="0", ClampMax="100"))
    float ForwardShieldPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(EditCondition="bUseDirectionalShields", ClampMin="0", ClampMax="100"))
    float AftShieldPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(EditCondition="bUseDirectionalShields", ClampMin="0", ClampMax="100"))
    float PortShieldPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield", meta=(EditCondition="bUseDirectionalShields", ClampMin="0", ClampMax="100"))
    float StarboardShieldPercentage;

    // ====================
    // STATE
    // ====================

    /** Current hull strength */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health State")
    float CurrentHullStrength;

    /** Current shield strength (omnidirectional) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield State")
    float CurrentShieldStrength;

    /** Shield facings data (if using directional shields) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield State")
    TMap<EShieldFacing, FShieldFacingData> ShieldFacings;

    /** Whether ship is destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health State")
    bool bIsDestroyed;

    /** Whether shields are overloaded */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield State")
    bool bShieldsOverloaded;

protected:
    /** Time since last damage for shield recharge */
    float TimeSinceLastDamage;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // DAMAGE HANDLING
    // ====================

    /**
     * Apply damage to this ship
     * Called automatically by Unreal's damage system
     * @param DamageAmount Amount of raw damage
     * @param DamageEvent Information about damage type and source
     * @param EventInstigator Controller that caused damage
     * @param DamageCauser Actor that caused damage
     */
    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float DamageAmount, const class UDamageType* DamageType, 
                             class AController* InstigatedBy, AActor* DamageCauser);

    /**
     * Apply typed damage with armor/shield penetration
     * @param BaseDamage Base damage amount
     * @param DamageType Type of damage (affects penetration)
     * @param ArmorPenetration Armor penetration rating (0-100)
     * @param ShieldPenetration Shield penetration rating (0-100)
     * @param HitDirection Direction damage came from (for directional shields)
     * @param DamageCauser Actor that caused damage
     * @return Actual damage dealt
     */
    UFUNCTION(BlueprintCallable, Category="Combat Health")
    float ApplyTypedDamage(float BaseDamage, EDamageType DamageType, float ArmorPenetration, 
                          float ShieldPenetration, FVector HitDirection, AActor* DamageCauser);

    /**
     * Restore hull strength
     * @param Amount Amount to heal
     * @return Actual amount healed
     */
    UFUNCTION(BlueprintCallable, Category="Combat Health")
    float RestoreHull(float Amount);

    /**
     * Restore shield strength
     * @param Amount Amount to restore
     * @param Facing Specific facing to restore (Omnidirectional for all)
     * @return Actual amount restored
     */
    UFUNCTION(BlueprintCallable, Category="Combat Health")
    float RestoreShields(float Amount, EShieldFacing Facing = EShieldFacing::Omnidirectional);

    /**
     * Destroy the ship
     */
    UFUNCTION(BlueprintCallable, Category="Combat Health")
    void DestroyShip();

    // ====================
    // QUERIES
    // ====================

    /**
     * Get hull strength percentage (0-1)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    float GetHullPercentage() const;

    /**
     * Get shield strength percentage (0-1)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    float GetShieldPercentage() const;

    /**
     * Get shield strength for specific facing
     * @param Facing Shield facing to query
     * @return Shield percentage (0-1)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    float GetShieldFacingPercentage(EShieldFacing Facing) const;

    /**
     * Check if shields are active
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    bool HasActiveShields() const;

    /**
     * Check if ship is critically damaged (below 25% hull)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    bool IsCriticallyDamaged() const;

    /**
     * Determine which shield facing is hit based on direction
     * @param HitDirection Direction of incoming damage
     * @return Shield facing that was hit
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    EShieldFacing GetHitFacing(FVector HitDirection) const;

    /**
     * Get shield facing data for a specific facing
     * @param Facing Shield facing to query
     * @return Shield facing data
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    FShieldFacingData GetShieldFacingData(EShieldFacing Facing) const;

    /**
     * Check if shields are currently recharging
     * @return true if shields are recharging
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat Health")
    bool IsRecharging() const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when hull takes damage
     * @param DamageAmount Amount of damage dealt to hull
     * @param DamageCauser Actor that caused damage
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnHullDamaged(float DamageAmount, AActor* DamageCauser);

    /**
     * Called when shields take damage
     * @param DamageAmount Amount of damage dealt to shields
     * @param Facing Shield facing that was hit
     * @param DamageCauser Actor that caused damage
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnShieldDamaged(float DamageAmount, EShieldFacing Facing, AActor* DamageCauser);

    /**
     * Called when shields are depleted
     * @param Facing Shield facing that was depleted
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnShieldsDepleted(EShieldFacing Facing);

    /**
     * Called when shields finish recharging
     * @param Facing Shield facing that recharged
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnShieldsRecharged(EShieldFacing Facing);

    /**
     * Called when ship reaches critical damage threshold
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnCriticalDamage();

    /**
     * Called when ship is destroyed
     * @param Killer Controller that destroyed this ship
     */
    UFUNCTION(BlueprintNativeEvent, Category="Combat Health")
    void OnShipDestroyed(AController* Killer);

private:
    /** Update shield recharge */
    void UpdateShieldRecharge(float DeltaTime);

    /** Initialize shield facings */
    void InitializeShieldFacings();

    /** Calculate damage after armor */
    float CalculateArmorReduction(float Damage, float ArmorPenetration) const;

    /** Apply damage to shield facing */
    float ApplyDamageToShieldFacing(EShieldFacing Facing, float Damage, float ShieldPenetration);

    /** Track if critical damage event was already fired */
    bool bCriticalDamageEventFired;
};
