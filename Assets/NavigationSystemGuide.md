# Navigation System Guide

## Overview

The Adastrea Navigation System provides comprehensive pathfinding, autopilot, and waypoint navigation for spaceships. Built on top of the existing Way/WayNetwork infrastructure, it enables both player-controlled and AI-driven navigation through space.

## System Status: ✅ COMPLETE

The Navigation System is production-ready with the following features:
- **NavigationComponent** - Full autopilot and waypoint navigation
- **3D Pathfinding** - Obstacle avoidance and route planning
- **Following Behavior** - AI ships can follow targets
- **Integration Ready** - Works with WayNetwork for long-distance routes

## Core Components

### NavigationComponent

Actor component that handles all navigation functionality for spaceships.

**Key Features:**
- Autopilot to single destination or multi-waypoint paths
- Following behavior (escort/convoy mechanics)
- 3D pathfinding with obstacle detection
- Configurable speeds and turn rates
- BlueprintNativeEvents for custom behaviors

**Add to Ship:**
```cpp
// In your ship Blueprint or C++ class
UNavigationComponent* NavComponent = CreateDefaultSubobject<UNavigationComponent>(TEXT("NavigationComponent"));
```

## Navigation Modes

The system supports four navigation modes:

### 1. Manual Mode
**Default mode** - Player has full control, no automated navigation.

```cpp
// Always in manual mode when autopilot is not active
```

### 2. Autopilot Mode
**Automated navigation** to waypoints. Ship follows path automatically.

**Activate Autopilot:**
```cpp
// Navigate to single destination
FVector Destination(100000.0f, 50000.0f, 0.0f);
NavComponent->ActivateAutopilot(Destination);

// Navigate through multiple waypoints
TArray<FNavigationWaypoint> Waypoints;
// ... add waypoints ...
NavComponent->ActivateAutopilotPath(Waypoints);
```

**Deactivate Autopilot:**
```cpp
NavComponent->DeactivateAutopilot();
```

### 3. Following Mode
**Follow another ship** at a set distance (escort, convoy).

```cpp
AActor* TargetShip = ...; // Ship to follow
float Distance = 500.0f;   // Distance to maintain (meters)
NavComponent->StartFollowing(TargetShip, Distance);

// Stop following
NavComponent->StopFollowing();
```

### 4. Docking Mode
**Approach station/dock** - Reserved for future docking implementation.

## Waypoint System

### Waypoint Structure

```cpp
FNavigationWaypoint Waypoint;
Waypoint.Location = FVector(10000, 5000, 0);        // World location
Waypoint.WaypointName = FText::FromString("Alpha"); // Optional name
Waypoint.DesiredSpeed = 800.0f;                     // Speed at waypoint (0 = use default)
Waypoint.bStopAtWaypoint = false;                   // Whether to stop
```

### Working with Waypoints

**Add Waypoint:**
```cpp
NavComponent->AddWaypoint(Waypoint);
```

**Clear All Waypoints:**
```cpp
NavComponent->ClearWaypoints();
```

**Skip to Next:**
```cpp
if (NavComponent->SkipToNextWaypoint())
{
    // Successfully skipped to next waypoint
}
```

**Query Current Waypoint:**
```cpp
FNavigationWaypoint CurrentWaypoint;
if (NavComponent->GetCurrentWaypoint(CurrentWaypoint))
{
    // CurrentWaypoint now contains active waypoint data
    float Distance = NavComponent->GetDistanceToWaypoint();
}
```

## Pathfinding

### 3D Pathfinding

Calculate a path that avoids obstacles:

```cpp
FVector Start = Ship->GetActorLocation();
FVector End = TargetLocation;
TArray<FNavigationWaypoint> Path;

if (NavComponent->FindPath3D(Start, End, Path))
{
    // Path found! Use it for autopilot
    NavComponent->ActivateAutopilotPath(Path);
}
else
{
    // No path found, handle error
    UE_LOG(LogTemp, Warning, TEXT("Could not find path to destination"));
}
```

### Check Path Clear

Test if direct line of sight exists:

```cpp
if (NavComponent->IsPathClear(Start, End))
{
    // Direct route is clear, no obstacles
}
else
{
    // Obstacles detected, use pathfinding
}
```

### Travel Time Estimation

```cpp
float EstimatedSeconds = NavComponent->CalculateTravelTime(Destination);
float EstimatedMinutes = EstimatedSeconds / 60.0f;

UE_LOG(LogTemp, Log, TEXT("ETA: %.1f minutes"), EstimatedMinutes);
```

## Configuration

### Component Properties

Set these in Blueprint editor or via C++:

```cpp
// Speed Settings
NavComponent->MaxNavigationSpeed = 1000.0f;     // Max autopilot speed (m/s)
NavComponent->ApproachSpeed = 200.0f;           // Speed when near waypoint (m/s)

// Waypoint Detection
NavComponent->WaypointReachedDistance = 100.0f; // Distance to consider reached (meters)

// Turn Behavior
NavComponent->TurnSmoothing = 0.5f;             // 0-1, higher = smoother turns

// Obstacle Avoidance
NavComponent->bAvoidObstacles = true;           // Enable/disable avoidance
NavComponent->ObstacleDetectionRange = 2000.0f; // Detection range (meters)

// Following Behavior
NavComponent->FollowDistance = 500.0f;          // Default follow distance (meters)
```

## Query Functions

### Navigation Status

```cpp
// Check if autopilot is active
bool bIsActive = NavComponent->IsAutopilotActive();

// Get current mode
ENavigationMode Mode = NavComponent->GetNavigationMode();
// Returns: Manual, Autopilot, Following, or Docking

// Check if on course
bool bOnCourse = NavComponent->IsOnCourse(10.0f); // Within 10 degrees
```

### Distance and Time

```cpp
// Distance to current waypoint
float DistanceToWaypoint = NavComponent->GetDistanceToWaypoint();

// Total remaining distance in path
float RemainingDistance = NavComponent->GetRemainingDistance();

// Estimated time to complete path
float TimeRemaining = NavComponent->GetEstimatedTimeToComplete();
```

## Blueprint Integration

### Blueprint Events

Override these in Blueprint for custom behavior:

**OnWaypointReached**
```cpp
// Called when ship reaches a waypoint
UFUNCTION(BlueprintNativeEvent)
void OnWaypointReached(const FNavigationWaypoint& Waypoint, int32 WaypointIndex);
```

**OnDestinationReached**
```cpp
// Called when final destination is reached
UFUNCTION(BlueprintNativeEvent)
void OnDestinationReached();
```

**OnAutopilotActivated**
```cpp
// Called when autopilot starts
UFUNCTION(BlueprintNativeEvent)
void OnAutopilotActivated();
```

**OnAutopilotDeactivated**
```cpp
// Called when autopilot stops
UFUNCTION(BlueprintNativeEvent)
void OnAutopilotDeactivated();
```

**OnObstacleDetected**
```cpp
// Called when obstacle is detected in path
UFUNCTION(BlueprintNativeEvent)
void OnObstacleDetected(FVector ObstacleLocation);
```

### Example Blueprint Setup

1. **Add Component to Ship:**
   - Open ship Blueprint
   - Add Component → Navigation Component
   - Configure properties in Details panel

2. **Implement Event Override:**
   - Event Graph → Override Function → OnWaypointReached
   - Add custom logic (play sound, show message, etc.)

3. **Call Navigation Functions:**
   - Use "Activate Autopilot" node to start navigation
   - Use "Get Distance To Waypoint" for UI display
   - Use "Is Autopilot Active" for conditional logic

## Integration with WayNetwork

The Navigation System works alongside the WayNetwork infrastructure for long-distance travel:

```cpp
// Get waypoint from WayNetwork
UWayDataAsset* Way = ...; // From WayNetwork
FVector WayLocation = Way->HomeBaseID; // Get location from Way

// Navigate to Way location
NavComponent->ActivateAutopilot(WayLocation);
```

**Suggested Integration:**
1. Use WayNetwork for sector-to-sector routing
2. Use NavigationComponent for local navigation within sectors
3. Combine both for long-distance autopilot:
   - Calculate route through WayNetwork sectors
   - Create waypoints at each sector transition
   - Activate autopilot with multi-waypoint path

## AI Usage

For AI-controlled ships:

```cpp
// In AI Controller or AI Logic class
void AMyShipAI::NavigateToTarget(FVector TargetLocation)
{
    UNavigationComponent* NavComp = Ship->FindComponentByClass<UNavigationComponent>();
    if (NavComp)
    {
        // Find path avoiding obstacles
        TArray<FNavigationWaypoint> Path;
        if (NavComp->FindPath3D(Ship->GetActorLocation(), TargetLocation, Path))
        {
            NavComp->ActivateAutopilotPath(Path);
        }
    }
}

// For escort/convoy AI
void AMyEscortAI::EscortTarget(AActor* Target)
{
    UNavigationComponent* NavComp = Ship->FindComponentByClass<UNavigationComponent>();
    if (NavComp)
    {
        NavComp->StartFollowing(Target, 600.0f);
    }
}
```

## Performance Considerations

### Optimization Tips

1. **Pathfinding Frequency:**
   - Don't recalculate paths every frame
   - Use timer to recalculate every 1-2 seconds for dynamic obstacles

2. **Obstacle Detection:**
   - Adjust `ObstacleDetectionRange` based on ship speed
   - Disable for ships in open space (far from obstacles)

3. **Waypoint Density:**
   - Use fewer waypoints for long-distance travel
   - More waypoints near complex obstacles

4. **Following Behavior:**
   - Set appropriate `FollowDistance` to prevent clustering
   - Use different distances for different ship types

### Recommended Settings

**Fast Ships (Fighters):**
```cpp
MaxNavigationSpeed = 1500.0f;
TurnSmoothing = 0.3f;
WaypointReachedDistance = 150.0f;
```

**Medium Ships (Freighters):**
```cpp
MaxNavigationSpeed = 800.0f;
TurnSmoothing = 0.5f;
WaypointReachedDistance = 100.0f;
```

**Large Ships (Capital Ships):**
```cpp
MaxNavigationSpeed = 400.0f;
TurnSmoothing = 0.8f;
WaypointReachedDistance = 200.0f;
```

## Troubleshooting

### Autopilot Not Working

**Problem:** Autopilot doesn't activate
**Solutions:**
- Check that waypoints are added to path
- Ensure component has valid owner actor
- Verify owner actor has location/rotation

**Problem:** Ship moves but doesn't turn
**Solutions:**
- Check `TurnSmoothing` value (should be > 0)
- Verify owner actor can be rotated
- Ensure no other systems overriding rotation

### Pathfinding Issues

**Problem:** No path found
**Solutions:**
- Check if destination is reachable
- Verify obstacle detection is working
- Try increasing pathfinding segment size
- Test with direct line-of-sight first

**Problem:** Ship hits obstacles
**Solutions:**
- Enable obstacle avoidance (`bAvoidObstacles = true`)
- Increase `ObstacleDetectionRange`
- Add more waypoints around complex geometry

### Following Behavior Issues

**Problem:** Ship doesn't follow target
**Solutions:**
- Verify target actor is valid
- Check that target is moving
- Adjust `FollowDistance` (may be too close/far)

**Problem:** Ship oscillates around follow point
**Solutions:**
- Increase `TurnSmoothing` value
- Adjust `FollowDistance` buffer zone
- Reduce `MaxNavigationSpeed`

## Future Enhancements

Potential additions to the Navigation System:

1. **Formation Flying:**
   - Multiple ships in coordinated formations
   - Configurable formation patterns

2. **Jump Drive Integration:**
   - Long-distance instant travel
   - Charge-up and cooldown mechanics

3. **Traffic Management:**
   - Collision avoidance between AI ships
   - Traffic lanes in busy sectors

4. **Dynamic Pathfinding:**
   - Real-time path recalculation
   - Adaptive obstacle avoidance

5. **Waypoint Networks:**
   - Persistent waypoint markers in world
   - Shared waypoints between players

## Code Examples

### Complete Autopilot Setup

```cpp
void AMySpaceship::SetupNavigation()
{
    // Create component
    NavComponent = CreateDefaultSubobject<UNavigationComponent>(TEXT("Navigation"));
    
    // Configure
    NavComponent->MaxNavigationSpeed = 1000.0f;
    NavComponent->bAvoidObstacles = true;
    NavComponent->ObstacleDetectionRange = 2000.0f;
}

void AMySpaceship::NavigateToLocation(FVector Destination)
{
    if (NavComponent)
    {
        // Find path
        TArray<FNavigationWaypoint> Path;
        if (NavComponent->FindPath3D(GetActorLocation(), Destination, Path))
        {
            // Activate autopilot with path
            NavComponent->ActivateAutopilotPath(Path);
            
            // Show ETA
            float ETA = NavComponent->GetEstimatedTimeToComplete();
            UE_LOG(LogTemp, Log, TEXT("Autopilot engaged. ETA: %.1f seconds"), ETA);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot find path to destination"));
        }
    }
}

void AMySpaceship::OnAutopilotDeactivated_Implementation()
{
    Super::OnAutopilotDeactivated_Implementation();
    
    // Custom behavior when autopilot stops
    UE_LOG(LogTemp, Log, TEXT("Destination reached!"));
    // Play arrival sound, show notification, etc.
}
```

## See Also

- `WayNetwork.h` - Long-distance routing infrastructure
- `Way.h` - Waypoint and sector system
- `ENHANCED_INPUT_GUIDE.md` - Input configuration
- `CODE_STYLE.md` - Coding standards

---

**Last Updated**: 2025-11-11  
**Version**: 1.0  
**Status**: Production Ready  
**Component**: NavigationComponent  
**Lines of Code**: ~865 lines (header + implementation)
