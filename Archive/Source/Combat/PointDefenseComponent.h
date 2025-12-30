#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponComponent.h"
#include "PointDefenseComponent.generated.h"

/**
 * Struct for tracking incoming threats
 */
USTRUCT(BlueprintType)
struct FIncomingThreat
{
    GENERATED_BODY()

    /** Threatening actor (missile, torpedo, fighter, etc.) */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> ThreatActor;

    /** Distance to threat */
    UPROPERTY(BlueprintReadOnly)
    float Distance;

    /** Time until impact (estimated) */
    UPROPERTY(BlueprintReadOnly)
    float TimeToImpact;

    /** Threat priority (0-100, higher = more dangerous) */
    UPROPERTY(BlueprintReadOnly)
    float Priority;

    /** Time this threat was detected */
    UPROPERTY(BlueprintReadOnly)
    float DetectionTime;

    /** Whether this threat is currently being engaged */
    UPROPERTY(BlueprintReadOnly)
    bool bIsEngaged;

    FIncomingThreat()
        : ThreatActor(nullptr)
        , Distance(0.0f)
        , TimeToImpact(0.0f)
        , Priority(0.0f)
        , DetectionTime(0.0f)
        , bIsEngaged(false)
    {}
};

/**
 * Component for automated point defense against missiles, torpedoes, and small craft
 * 
 * Automatically detects and engages incoming threats using point defense weapons.
 * Prioritizes threats based on distance, velocity, and danger level.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Assign point defense capable weapon components
 * - Configure threat detection range and priorities
 * - System operates automatically when threats detected
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPointDefenseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPointDefenseComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Maximum range for threat detection */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense", meta=(ClampMin="100", ClampMax="50000"))
    float DetectionRange;

    /** How often to scan for threats (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense", meta=(ClampMin="0.1", ClampMax="2.0"))
    float ScanInterval;

    /** Tags to identify missile/torpedo threats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense")
    TArray<FName> MissileTags;

    /** Tags to identify fighter/small craft threats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense")
    TArray<FName> FighterTags;

    /** Maximum number of simultaneous targets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense", meta=(ClampMin="1", ClampMax="20"))
    int32 MaxSimultaneousTargets;

    /** Automatically engage detected threats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense")
    bool bAutoEngage;

    /** Prioritize missiles over fighters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense")
    bool bPrioritizeMissiles;

    // ====================
    // STATE
    // ====================

    /** Assigned point defense weapons */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Point Defense State")
    TArray<TObjectPtr<UWeaponComponent>> PointDefenseWeapons;

    /** Currently tracked threats */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Point Defense State")
    TArray<FIncomingThreat> TrackedThreats;

    /** Currently engaged threats */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Point Defense State")
    TArray<AActor*> EngagedThreats;

protected:
    /** Time since last threat scan */
    float TimeSinceLastScan;

    /** Weapon assignment map (weapon -> threat) */
    UPROPERTY()
    TMap<UWeaponComponent*, AActor*> WeaponAssignments;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // POINT DEFENSE OPERATIONS
    // ====================

    /**
     * Activate point defense system
     */
    virtual void Activate(bool bReset = false) override;

    /**
     * Deactivate point defense system
     */
    virtual void Deactivate() override;

    /**
     * Manually add a weapon to point defense
     * @param Weapon Weapon component to add
     * @return True if weapon was added successfully
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    bool AddPointDefenseWeapon(UWeaponComponent* Weapon);

    /**
     * Remove a weapon from point defense
     * @param Weapon Weapon component to remove
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    void RemovePointDefenseWeapon(UWeaponComponent* Weapon);

    /**
     * Manually engage a specific threat
     * @param Threat Actor to engage
     * @return True if threat was engaged
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    bool EngageThreat(AActor* Threat);

    /**
     * Disengage from a specific threat
     * @param Threat Actor to disengage from
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    void DisengageThreat(AActor* Threat);

    /**
     * Clear all engaged threats
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    void ClearAllThreats();

    // ====================
    // QUERIES
    // ====================

    /**
     * Get number of tracked threats
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Point Defense")
    int32 GetThreatCount() const;

    /**
     * Get number of engaged threats
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Point Defense")
    int32 GetEngagedThreatCount() const;

    /**
     * Get highest priority threat
     * @return Most dangerous unengaged threat (nullptr if none)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Point Defense")
    AActor* GetHighestPriorityThreat() const;

    /**
     * Calculate interception probability for a threat
     * @param Threat The threat actor
     * @return Probability of successful interception (0-1)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Point Defense")
    float CalculateInterceptionProbability(AActor* Threat) const;

    /**
     * Get threat information
     * @param Threat The threat actor
     * @param OutThreatInfo Output threat information
     * @return True if threat is tracked
     */
    UFUNCTION(BlueprintCallable, Category="Point Defense")
    bool GetThreatInfo(AActor* Threat, FIncomingThreat& OutThreatInfo) const;

    /**
     * Check if actor is a valid point defense target
     * @param Actor Actor to check
     * @return True if actor can be targeted by point defense
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Point Defense")
    bool IsValidThreat(AActor* Actor) const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when a new threat is detected
     * @param Threat The newly detected threat
     */
    UFUNCTION(BlueprintNativeEvent, Category="Point Defense")
    void OnThreatDetected(AActor* Threat);

    /**
     * Called when engaging a threat
     * @param Threat The threat being engaged
     * @param Weapon The weapon assigned to this threat
     */
    UFUNCTION(BlueprintNativeEvent, Category="Point Defense")
    void OnThreatEngaged(AActor* Threat, UWeaponComponent* Weapon);

    /**
     * Called when a threat is destroyed or neutralized
     * @param Threat The neutralized threat
     * @param bDestroyed Whether threat was destroyed (false = evaded)
     */
    UFUNCTION(BlueprintNativeEvent, Category="Point Defense")
    void OnThreatNeutralized(AActor* Threat, bool bDestroyed);

    /**
     * Calculate threat priority
     * Override in Blueprint for custom prioritization
     * @param Threat The threat to evaluate
     * @return Priority value (0-100, higher = more dangerous)
     */
    UFUNCTION(BlueprintNativeEvent, Category="Point Defense")
    float CalculateThreatPriority(AActor* Threat) const;

private:
    /** Scan for incoming threats */
    void ScanForThreats();

    /** Update threat tracking */
    void UpdateThreats();

    /** Assign weapons to threats */
    void AssignWeaponsToThreats();

    /** Update weapon firing */
    void UpdateWeaponFiring(float DeltaTime);

    /** Check if weapon can engage threat */
    bool CanWeaponEngageThreat(UWeaponComponent* Weapon, AActor* Threat) const;

    /** Calculate time to impact for threat */
    float CalculateTimeToImpact(AActor* Threat) const;
};
