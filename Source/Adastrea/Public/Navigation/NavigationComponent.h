#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationComponent.generated.h"

/**
 * Enum for navigation modes
 */
UENUM(BlueprintType)
enum class ENavigationMode : uint8
{
    Manual          UMETA(DisplayName = "Manual"),          // Player controlled
    Autopilot       UMETA(DisplayName = "Autopilot"),       // Automated navigation
    Following       UMETA(DisplayName = "Following"),       // Following another ship
    Docking         UMETA(DisplayName = "Docking")          // Approaching station/dock
};

/**
 * Structure for waypoint data
 */
USTRUCT(BlueprintType)
struct FNavigationWaypoint
{
    GENERATED_BODY()

    /** World location of waypoint */
    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    /** Optional waypoint name */
    UPROPERTY(BlueprintReadWrite)
    FText WaypointName;

    /** Desired speed at this waypoint (0 = use default) */
    UPROPERTY(BlueprintReadWrite)
    float DesiredSpeed;

    /** Whether to stop at this waypoint */
    UPROPERTY(BlueprintReadWrite)
    bool bStopAtWaypoint;

    FNavigationWaypoint()
        : Location(FVector::ZeroVector)
        , WaypointName(FText::GetEmpty())
        , DesiredSpeed(0.0f)
        , bStopAtWaypoint(false)
    {}
};

/**
 * Component for managing spaceship navigation and autopilot
 * 
 * Handles pathfinding, waypoint navigation, autopilot, and route planning.
 * Works with WayNetwork for long-distance routes and provides local navigation.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Configure navigation parameters
 * - Use Blueprint functions for autopilot control
 * - Query for navigation status and path info
 */
UCLASS(ClassGroup=(Navigation), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UNavigationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNavigationComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Maximum navigation speed in m/s */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(ClampMin="0", ClampMax="10000"))
    float MaxNavigationSpeed;

    /** Approach speed when near waypoint (m/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(ClampMin="0", ClampMax="1000"))
    float ApproachSpeed;

    /** Distance to consider waypoint reached (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(ClampMin="1", ClampMax="1000"))
    float WaypointReachedDistance;

    /** How smoothly to turn (0-1, higher = smoother) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(ClampMin="0.1", ClampMax="1.0"))
    float TurnSmoothing;

    /** Whether to avoid obstacles automatically */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation")
    bool bAvoidObstacles;

    /** Obstacle detection range (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(EditCondition="bAvoidObstacles", ClampMin="100", ClampMax="10000"))
    float ObstacleDetectionRange;

    // ====================
    // STATE
    // ====================

    /** Current navigation mode */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation State")
    ENavigationMode CurrentMode;

    /** Current waypoint path */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation State")
    TArray<FNavigationWaypoint> WaypointPath;

    /** Index of current target waypoint */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation State")
    int32 CurrentWaypointIndex;

    /** Whether autopilot is active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation State")
    bool bAutopilotActive;

    /** Target actor to follow (if in Following mode) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation State")
    AActor* FollowTarget;

    /** Distance to maintain when following */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Navigation", meta=(ClampMin="50", ClampMax="5000"))
    float FollowDistance;

protected:
    /** Current velocity */
    FVector CurrentVelocity;

    /** Target velocity for autopilot */
    FVector TargetVelocity;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // AUTOPILOT CONTROL
    // ====================

    /**
     * Enable autopilot to navigate to a location
     * @param TargetLocation World location to navigate to
     * @return True if autopilot was activated
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    bool ActivateAutopilot(FVector TargetLocation);

    /**
     * Enable autopilot with multiple waypoints
     * @param Waypoints Array of waypoints to navigate through
     * @return True if autopilot was activated
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    bool ActivateAutopilotPath(const TArray<FNavigationWaypoint>& Waypoints);

    /**
     * Disable autopilot and return control to player
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    void DeactivateAutopilot();

    /**
     * Start following another ship
     * @param Target Ship to follow
     * @param Distance Distance to maintain
     * @return True if following started
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    bool StartFollowing(AActor* Target, float Distance = 500.0f);

    /**
     * Stop following target
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    void StopFollowing();

    /**
     * Add waypoint to current path
     * @param Waypoint Waypoint to add
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    void AddWaypoint(FNavigationWaypoint Waypoint);

    /**
     * Clear all waypoints
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    void ClearWaypoints();

    /**
     * Skip to next waypoint in path
     * @return True if there was a next waypoint
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    bool SkipToNextWaypoint();

    // ====================
    // PATHFINDING
    // ====================

    /**
     * Calculate 3D path to destination avoiding obstacles
     * @param Start Starting location
     * @param End Destination location
     * @param OutPath Generated waypoint path
     * @return True if path was found
     */
    UFUNCTION(BlueprintCallable, Category="Navigation|Pathfinding")
    bool FindPath3D(FVector Start, FVector End, TArray<FNavigationWaypoint>& OutPath);

    /**
     * Check if line of sight is clear to destination
     * @param Start Starting location
     * @param End Destination location
     * @return True if path is clear
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation|Pathfinding")
    bool IsPathClear(FVector Start, FVector End) const;

    /**
     * Calculate estimated travel time to destination
     * @param Destination Target location
     * @return Estimated time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation|Pathfinding")
    float CalculateTravelTime(FVector Destination) const;

    // ====================
    // QUERIES
    // ====================

    /**
     * Get current waypoint target
     * @param OutWaypoint Current waypoint data
     * @return True if there is an active waypoint
     */
    UFUNCTION(BlueprintCallable, Category="Navigation")
    bool GetCurrentWaypoint(FNavigationWaypoint& OutWaypoint) const;

    /**
     * Get distance to current waypoint
     * @return Distance in meters (0 if no waypoint)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    float GetDistanceToWaypoint() const;

    /**
     * Get distance remaining in path
     * @return Total distance to complete path in meters
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    float GetRemainingDistance() const;

    /**
     * Get estimated time to complete path
     * @return Time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    float GetEstimatedTimeToComplete() const;

    /**
     * Check if ship is on course to waypoint
     * @param AngleTolerance Maximum angle deviation in degrees
     * @return True if ship is pointing at waypoint
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    bool IsOnCourse(float AngleTolerance = 10.0f) const;

    /**
     * Get current navigation mode
     * @return Current mode
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    ENavigationMode GetNavigationMode() const;

    /**
     * Check if autopilot is active
     * @return True if autopilot is engaged
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Navigation")
    bool IsAutopilotActive() const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when waypoint is reached
     * @param Waypoint The waypoint that was reached
     * @param WaypointIndex Index in the path
     */
    UFUNCTION(BlueprintNativeEvent, Category="Navigation")
    void OnWaypointReached(const FNavigationWaypoint& Waypoint, int32 WaypointIndex);

    /**
     * Called when autopilot destination is reached
     */
    UFUNCTION(BlueprintNativeEvent, Category="Navigation")
    void OnDestinationReached();

    /**
     * Called when autopilot is activated
     */
    UFUNCTION(BlueprintNativeEvent, Category="Navigation")
    void OnAutopilotActivated();

    /**
     * Called when autopilot is deactivated
     */
    UFUNCTION(BlueprintNativeEvent, Category="Navigation")
    void OnAutopilotDeactivated();

    /**
     * Called when obstacle is detected in path
     * @param ObstacleLocation Location of detected obstacle
     */
    UFUNCTION(BlueprintNativeEvent, Category="Navigation")
    void OnObstacleDetected(FVector ObstacleLocation);

private:
    /** Update autopilot navigation */
    void UpdateAutopilot(float DeltaTime);

    /** Update following behavior */
    void UpdateFollowing(float DeltaTime);

    /** Check if current waypoint is reached */
    bool CheckWaypointReached();

    /** Advance to next waypoint */
    void AdvanceToNextWaypoint();

    /** Calculate steering to target */
    FVector CalculateSteeringToTarget(FVector TargetLocation, float DeltaTime);

    /** Detect and avoid obstacles */
    FVector DetectAndAvoidObstacles(FVector DesiredVelocity);

    /** Apply velocity to owner actor */
    void ApplyVelocity(float DeltaTime);
};
