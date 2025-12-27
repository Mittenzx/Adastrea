#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

/**
 * Struct for storing target information
 */
USTRUCT(BlueprintType)
struct FTargetInfo
{
    GENERATED_BODY()

    /** Target actor */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> Target;

    /** Distance to target */
    UPROPERTY(BlueprintReadOnly)
    float Distance;

    /** Angle to target from forward direction */
    UPROPERTY(BlueprintReadOnly)
    float Angle;

    /** Threat level (0-100, higher = more dangerous) */
    UPROPERTY(BlueprintReadOnly)
    float ThreatLevel;

    /** Time this target was first detected */
    UPROPERTY(BlueprintReadOnly)
    float DetectionTime;

    FTargetInfo()
        : Target(nullptr)
        , Distance(0.0f)
        , Angle(0.0f)
        , ThreatLevel(0.0f)
        , DetectionTime(0.0f)
    {}
};

/**
 * Component for managing targeting and fire control on spaceships
 * 
 * Handles target detection, selection, lock-on, lead calculation, and
 * automatic target prioritization for combat systems.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Configure sensor range and targeting parameters
 * - Use Blueprint functions for target selection
 * - Query for lead indicators and firing solutions
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UTargetingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTargetingComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Maximum targeting range in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="100", ClampMax="100000"))
    float MaxTargetingRange;

    /** Maximum angle for auto-targeting (degrees from forward) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0", ClampMax="180"))
    float MaxTargetingAngle;

    /** How often to scan for targets (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0.1", ClampMax="5.0"))
    float ScanInterval;

    /** Auto-select nearest target when current target is lost */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
    bool bAutoRetarget;

    /** Prioritize targets by threat level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
    bool bPrioritizeByThreat;

    /** Tags to identify valid targets (empty = all actors) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Targeting")
    TArray<FName> TargetTags;

    // ====================
    // STATE
    // ====================

    /** Currently locked target */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Targeting State")
    TWeakObjectPtr<AActor> CurrentTarget;

    /** All detected targets within range */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Targeting State")
    TArray<FTargetInfo> DetectedTargets;

    /** Whether target lock is active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Targeting State")
    bool bIsLocked;

protected:
    /** Time since last scan */
    float TimeSinceLastScan;

    /** Time when current target was locked */
    float LockTime;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // TARGET SELECTION
    // ====================

    /**
     * Manually select a target
     * @param Target Actor to target
     * @return True if target was successfully locked
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    bool SelectTarget(AActor* Target);

    /**
     * Select next target in detected targets list
     * @return The new target (nullptr if no targets)
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    AActor* SelectNextTarget();

    /**
     * Select previous target in detected targets list
     * @return The new target (nullptr if no targets)
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    AActor* SelectPreviousTarget();

    /**
     * Select nearest target
     * @return The nearest target (nullptr if no targets)
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    AActor* SelectNearestTarget();

    /**
     * Select highest threat target
     * @return The most threatening target (nullptr if no targets)
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    AActor* SelectHighestThreatTarget();

    /**
     * Clear current target
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    void ClearTarget();

    // ====================
    // TARGET QUERIES
    // ====================

    /**
     * Get distance to current target
     * @return Distance in meters (0 if no target)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    float GetDistanceToTarget() const;

    /**
     * Get angle to current target from forward direction
     * @return Angle in degrees (0 if no target)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    float GetAngleToTarget() const;

    /**
     * Check if current target is within firing arc
     * @param ArcAngle Maximum angle from forward (degrees)
     * @return True if target is within arc
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    bool IsTargetInFiringArc(float ArcAngle = 45.0f) const;

    /**
     * Calculate lead indicator position for moving target
     * @param ProjectileSpeed Speed of projectile in m/s
     * @return World position to aim at for interception
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    FVector CalculateLeadPosition(float ProjectileSpeed) const;

    /**
     * Get target info for current target
     * @param OutTargetInfo Output target information
     * @return True if current target is valid
     */
    UFUNCTION(BlueprintCallable, Category="Targeting")
    bool GetCurrentTargetInfo(FTargetInfo& OutTargetInfo) const;

    /**
     * Get all detected targets sorted by distance
     * @return Array of target information
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    TArray<FTargetInfo> GetDetectedTargetsSortedByDistance() const;

    /**
     * Get all detected targets sorted by threat
     * @return Array of target information
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    TArray<FTargetInfo> GetDetectedTargetsSortedByThreat() const;

    /**
     * Get number of detected targets
     * @return Count of targets in range
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Targeting")
    int32 GetDetectedTargetCount() const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when a new target is locked
     * Override in Blueprint for UI updates or audio
     * @param Target The newly locked target
     */
    UFUNCTION(BlueprintNativeEvent, Category="Targeting")
    void OnTargetLocked(AActor* Target);

    /**
     * Called when target lock is lost
     * Override in Blueprint for UI updates or audio
     * @param Reason Why the target was lost (OutOfRange, Destroyed, Manual)
     */
    UFUNCTION(BlueprintNativeEvent, Category="Targeting")
    void OnTargetLost(const FString& Reason);

    /**
     * Calculate threat level for a target
     * Override in Blueprint for custom threat calculation
     * @param Target The target to evaluate
     * @return Threat level (0-100)
     */
    UFUNCTION(BlueprintNativeEvent, Category="Targeting")
    float CalculateThreatLevel(AActor* Target) const;

private:
    /** Scan for targets in range */
    void ScanForTargets();

    /** Update current target state */
    void UpdateCurrentTarget();

    /** Check if actor is a valid target */
    bool IsValidTarget(AActor* Target) const;

    /** Calculate angle to target from forward direction */
    float CalculateAngleToActor(AActor* Target) const;
};
