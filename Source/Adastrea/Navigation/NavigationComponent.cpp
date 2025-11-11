#include "Navigation/NavigationComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UNavigationComponent::UNavigationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default configuration
    MaxNavigationSpeed = 1000.0f;
    ApproachSpeed = 200.0f;
    WaypointReachedDistance = 100.0f;
    TurnSmoothing = 0.5f;
    bAvoidObstacles = true;
    ObstacleDetectionRange = 2000.0f;
    FollowDistance = 500.0f;
    
    // Pathfinding configuration
    PathSegmentDistance = 1000.0f;
    ObstacleAvoidanceOffset = 500.0f;
    ApproachDistance = 1000.0f;
    FollowDistanceTolerance = 100.0f;

    // Initial state
    CurrentMode = ENavigationMode::Manual;
    CurrentWaypointIndex = 0;
    bAutopilotActive = false;
    FollowTarget = nullptr;
    CurrentVelocity = FVector::ZeroVector;
    TargetVelocity = FVector::ZeroVector;
}

void UNavigationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutopilotActive)
    {
        switch (CurrentMode)
        {
        case ENavigationMode::Autopilot:
            UpdateAutopilot(DeltaTime);
            break;
        case ENavigationMode::Following:
            UpdateFollowing(DeltaTime);
            break;
        default:
            break;
        }
    }
}

// ====================
// AUTOPILOT CONTROL
// ====================

bool UNavigationComponent::ActivateAutopilot(FVector TargetLocation)
{
    if (!GetOwner())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("NavigationComponent: Cannot activate autopilot - no owner"));
        return false;
    }

    ClearWaypoints();
    
    FNavigationWaypoint Waypoint;
    Waypoint.Location = TargetLocation;
    Waypoint.WaypointName = FText::FromString(TEXT("Destination"));
    Waypoint.DesiredSpeed = MaxNavigationSpeed;
    Waypoint.bStopAtWaypoint = true;
    
    WaypointPath.Add(Waypoint);
    CurrentWaypointIndex = 0;
    CurrentMode = ENavigationMode::Autopilot;
    bAutopilotActive = true;

    OnAutopilotActivated();

    UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Autopilot activated to %s"), *TargetLocation.ToString());
    return true;
}

bool UNavigationComponent::ActivateAutopilotPath(const TArray<FNavigationWaypoint>& Waypoints)
{
    if (!GetOwner() || Waypoints.Num() == 0)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("NavigationComponent: Cannot activate autopilot path - invalid parameters"));
        return false;
    }

    WaypointPath = Waypoints;
    CurrentWaypointIndex = 0;
    CurrentMode = ENavigationMode::Autopilot;
    bAutopilotActive = true;

    OnAutopilotActivated();

    UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Autopilot path activated with %d waypoints"), Waypoints.Num());
    return true;
}

void UNavigationComponent::DeactivateAutopilot()
{
    if (bAutopilotActive)
    {
        bAutopilotActive = false;
        CurrentMode = ENavigationMode::Manual;
        CurrentVelocity = FVector::ZeroVector;
        
        OnAutopilotDeactivated();
        
        UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Autopilot deactivated"));
    }
}

bool UNavigationComponent::StartFollowing(AActor* Target, float Distance)
{
    if (!Target || !GetOwner())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("NavigationComponent: Cannot start following - invalid target or no owner"));
        return false;
    }

    FollowTarget = Target;
    FollowDistance = Distance;
    CurrentMode = ENavigationMode::Following;
    bAutopilotActive = true;

    UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Following %s at distance %.0f"), *Target->GetName(), Distance);
    return true;
}

void UNavigationComponent::StopFollowing()
{
    if (CurrentMode == ENavigationMode::Following)
    {
        FollowTarget = nullptr;
        DeactivateAutopilot();
        
        UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Stopped following"));
    }
}

void UNavigationComponent::AddWaypoint(FNavigationWaypoint Waypoint)
{
    WaypointPath.Add(Waypoint);
    
    UE_LOG(LogAdastrea, Verbose, TEXT("NavigationComponent: Waypoint added at %s"), *Waypoint.Location.ToString());
}

void UNavigationComponent::ClearWaypoints()
{
    WaypointPath.Empty();
    CurrentWaypointIndex = 0;
    
    UE_LOG(LogAdastrea, Verbose, TEXT("NavigationComponent: Waypoints cleared"));
}

bool UNavigationComponent::SkipToNextWaypoint()
{
    if (CurrentWaypointIndex + 1 < WaypointPath.Num())
    {
        CurrentWaypointIndex++;
        UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Skipped to waypoint %d"), CurrentWaypointIndex);
        return true;
    }
    
    return false;
}

// ====================
// PATHFINDING
// ====================

bool UNavigationComponent::FindPath3D(FVector Start, FVector End, TArray<FNavigationWaypoint>& OutPath)
{
    OutPath.Empty();

    // Simple direct path if no obstacles
    if (IsPathClear(Start, End))
    {
        FNavigationWaypoint Waypoint;
        Waypoint.Location = End;
        Waypoint.DesiredSpeed = MaxNavigationSpeed;
        OutPath.Add(Waypoint);
        return true;
    }

    // Basic A* pathfinding with waypoints
    // This is a simplified 3D pathfinding - can be enhanced with proper navigation mesh
    FVector Direction = (End - Start).GetSafeNormal();
    float TotalDistance = FVector::Dist(Start, End);
    int32 NumSegments = FMath::CeilToInt(TotalDistance / PathSegmentDistance);

    for (int32 i = 1; i <= NumSegments; ++i)
    {
        float Alpha = static_cast<float>(i) / NumSegments;
        FVector SegmentLocation = FMath::Lerp(Start, End, Alpha);

        // Try to avoid obstacles by offsetting waypoint if needed
        if (!IsPathClear(i > 1 ? OutPath.Last().Location : Start, SegmentLocation))
        {
            // Offset perpendicular to path direction
            FVector PerpendicularOffset = FVector::CrossProduct(Direction, FVector::UpVector) * ObstacleAvoidanceOffset;
            SegmentLocation += PerpendicularOffset;
        }

        FNavigationWaypoint Waypoint;
        Waypoint.Location = SegmentLocation;
        Waypoint.DesiredSpeed = MaxNavigationSpeed;
        OutPath.Add(Waypoint);
    }

    return OutPath.Num() > 0;
}

bool UNavigationComponent::IsPathClear(FVector Start, FVector End) const
{
    if (!GetWorld())
    {
        return true;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    return !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );
}

float UNavigationComponent::CalculateTravelTime(FVector Destination) const
{
    if (!GetOwner())
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Destination);
    float AverageSpeed = (MaxNavigationSpeed + ApproachSpeed) * 0.5f;
    
    return AverageSpeed > 0.0f ? Distance / AverageSpeed : 0.0f;
}

// ====================
// QUERIES
// ====================

bool UNavigationComponent::GetCurrentWaypoint(FNavigationWaypoint& OutWaypoint) const
{
    if (CurrentWaypointIndex >= 0 && CurrentWaypointIndex < WaypointPath.Num())
    {
        OutWaypoint = WaypointPath[CurrentWaypointIndex];
        return true;
    }
    return false;
}

float UNavigationComponent::GetDistanceToWaypoint() const
{
    if (!GetOwner() || CurrentWaypointIndex >= WaypointPath.Num())
    {
        return 0.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), WaypointPath[CurrentWaypointIndex].Location);
}

float UNavigationComponent::GetRemainingDistance() const
{
    if (!GetOwner() || WaypointPath.Num() == 0)
    {
        return 0.0f;
    }

    float TotalDistance = 0.0f;
    FVector CurrentLocation = GetOwner()->GetActorLocation();

    // Distance to first waypoint
    if (CurrentWaypointIndex < WaypointPath.Num())
    {
        TotalDistance += FVector::Dist(CurrentLocation, WaypointPath[CurrentWaypointIndex].Location);
    }

    // Distance between remaining waypoints
    for (int32 i = CurrentWaypointIndex; i < WaypointPath.Num() - 1; ++i)
    {
        TotalDistance += FVector::Dist(WaypointPath[i].Location, WaypointPath[i + 1].Location);
    }

    return TotalDistance;
}

float UNavigationComponent::GetEstimatedTimeToComplete() const
{
    float RemainingDist = GetRemainingDistance();
    float AverageSpeed = (MaxNavigationSpeed + ApproachSpeed) * 0.5f;
    
    return AverageSpeed > 0.0f ? RemainingDist / AverageSpeed : 0.0f;
}

bool UNavigationComponent::IsOnCourse(float AngleTolerance) const
{
    if (!GetOwner() || CurrentWaypointIndex >= WaypointPath.Num())
    {
        return false;
    }

    FVector ToWaypoint = (WaypointPath[CurrentWaypointIndex].Location - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector Forward = GetOwner()->GetActorForwardVector();

    float DotProduct = FVector::DotProduct(Forward, ToWaypoint);
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    return AngleDegrees <= AngleTolerance;
}

ENavigationMode UNavigationComponent::GetNavigationMode() const
{
    return CurrentMode;
}

bool UNavigationComponent::IsAutopilotActive() const
{
    return bAutopilotActive;
}

// ====================
// BLUEPRINT NATIVE EVENTS
// ====================

void UNavigationComponent::OnWaypointReached_Implementation(const FNavigationWaypoint& Waypoint, int32 WaypointIndex)
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Waypoint %d reached"), WaypointIndex);
}

void UNavigationComponent::OnDestinationReached_Implementation()
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogAdastrea, Log, TEXT("NavigationComponent: Destination reached"));
}

void UNavigationComponent::OnAutopilotActivated_Implementation()
{
    // Default implementation - can be overridden in Blueprint
}

void UNavigationComponent::OnAutopilotDeactivated_Implementation()
{
    // Default implementation - can be overridden in Blueprint
}

void UNavigationComponent::OnObstacleDetected_Implementation(FVector ObstacleLocation)
{
    // Default implementation - can be overridden in Blueprint
    UE_LOG(LogAdastrea, Warning, TEXT("NavigationComponent: Obstacle detected at %s"), *ObstacleLocation.ToString());
}

// ====================
// PRIVATE HELPERS
// ====================

void UNavigationComponent::UpdateAutopilot(float DeltaTime)
{
    if (!GetOwner() || WaypointPath.Num() == 0 || CurrentWaypointIndex >= WaypointPath.Num())
    {
        return;
    }

    if (CheckWaypointReached())
    {
        AdvanceToNextWaypoint();
        return;
    }

    FVector TargetLocation = WaypointPath[CurrentWaypointIndex].Location;
    FVector DesiredVelocity = CalculateSteeringToTarget(TargetLocation, DeltaTime);

    // Apply obstacle avoidance if enabled
    if (bAvoidObstacles)
    {
        DesiredVelocity = DetectAndAvoidObstacles(DesiredVelocity);
    }

    CurrentVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, DeltaTime, TurnSmoothing * 5.0f);
    
    ApplyVelocity(DeltaTime);
}

void UNavigationComponent::UpdateFollowing(float DeltaTime)
{
    if (!GetOwner() || !FollowTarget)
    {
        StopFollowing();
        return;
    }

    FVector TargetLocation = FollowTarget->GetActorLocation();
    FVector ToTarget = TargetLocation - GetOwner()->GetActorLocation();
    float CurrentDistance = ToTarget.Size();

    // Maintain follow distance
    if (CurrentDistance > FollowDistance + FollowDistanceTolerance)
    {
        FVector DesiredLocation = TargetLocation - ToTarget.GetSafeNormal() * FollowDistance;
        FVector DesiredVelocity = CalculateSteeringToTarget(DesiredLocation, DeltaTime);
        
        CurrentVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, DeltaTime, TurnSmoothing * 3.0f);
        ApplyVelocity(DeltaTime);
    }
    else if (CurrentDistance < FollowDistance - FollowDistanceTolerance)
    {
        // Too close, slow down
        CurrentVelocity = FMath::VInterpTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, TurnSmoothing * 2.0f);
        ApplyVelocity(DeltaTime);
    }
}

bool UNavigationComponent::CheckWaypointReached()
{
    if (!GetOwner() || CurrentWaypointIndex >= WaypointPath.Num())
    {
        return false;
    }

    float Distance = GetDistanceToWaypoint();
    return Distance <= WaypointReachedDistance;
}

void UNavigationComponent::AdvanceToNextWaypoint()
{
    if (CurrentWaypointIndex < WaypointPath.Num())
    {
        OnWaypointReached(WaypointPath[CurrentWaypointIndex], CurrentWaypointIndex);
        
        CurrentWaypointIndex++;
        
        if (CurrentWaypointIndex >= WaypointPath.Num())
        {
            // Reached final destination
            OnDestinationReached();
            DeactivateAutopilot();
        }
    }
}

FVector UNavigationComponent::CalculateSteeringToTarget(FVector TargetLocation, float DeltaTime)
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    float DesiredSpeed = MaxNavigationSpeed;

    // Slow down when approaching waypoint
    if (Distance < ApproachDistance)
    {
        DesiredSpeed = FMath::Lerp(ApproachSpeed, MaxNavigationSpeed, Distance / ApproachDistance);
    }

    return Direction * DesiredSpeed;
}

FVector UNavigationComponent::DetectAndAvoidObstacles(FVector DesiredVelocity)
{
    if (!GetOwner() || !GetWorld())
    {
        return DesiredVelocity;
    }

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector LookAhead = CurrentLocation + DesiredVelocity.GetSafeNormal() * ObstacleDetectionRange;

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(HitResult, CurrentLocation, LookAhead, ECC_Visibility, QueryParams))
    {
        // Obstacle detected
        OnObstacleDetected(HitResult.Location);

        // Calculate avoidance vector (perpendicular to current direction)
        FVector Normal = HitResult.ImpactNormal;
        FVector Avoidance = FVector::CrossProduct(Normal, FVector::UpVector) * MaxNavigationSpeed * 0.5f;
        
        return DesiredVelocity + Avoidance;
    }

    return DesiredVelocity;
}

void UNavigationComponent::ApplyVelocity(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    FVector NewLocation = GetOwner()->GetActorLocation() + CurrentVelocity * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation, true);

    // Orient ship towards velocity direction
    if (!CurrentVelocity.IsNearlyZero())
    {
        FRotator TargetRotation = CurrentVelocity.Rotation();
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnSmoothing * 2.0f);
        GetOwner()->SetActorRotation(NewRotation);
    }
}
