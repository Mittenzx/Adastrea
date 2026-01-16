# X4-Style Mouse Position Flight - Implementation Guide

**Status**: ✅ Implemented (Fixed 2026-01-15)  
**Date**: 2026-01-12 (Initial), 2026-01-15 (Corrected)  
**Issue**: #446 follow-up - Controls improvement  
**Branch**: `copilot/fix-x4-mouse-flight-guide`

---

## Overview

This document describes the implementation of X4: Foundations-style mouse position flight controls for Adastrea. The system allows the ship to rotate **continuously** based on mouse cursor position on screen, with rotation speed scaling based on distance from center.

## Problem Statement (Original - 2026-01-12)

**Issue #446 Feedback:**
> "I don't know if flight assist is causing the problems. I think the difference between these controls and x4 is in x4 the ship turns and faces as per where the mouse is, faster the further from the center, but then adds something else that defines each ship. Like each ship has a variable between 0 and 1 that the rotation speed of the ship is multiplied by."

## Problem Statement (Correction - 2026-01-15)

**GitHub Issue Feedback:**
> "X4 style controls actually work differently. if the mouse is not in the deadzone the ship rotates based on the location and distance from the centre. if i move the mouse the ship moves but the ship stops if i stop moving the mouse. If this was the same as x4 the ship rotates dependant on the position of the mouse cursor, not using the direction i move the mouse. so i can leave the mouse still not in the center and the ship will rotate in that direction constantly until i move the mouse back into the deadzone"

**Root Cause**: The initial implementation (2026-01-12) checked mouse position only when `Turn()` or `LookUp()` was called (i.e., when mouse was moving). This meant the ship only rotated when the mouse was actively moving, not when the cursor was held in a position outside the deadzone.

**Previous System (Wrong):**
- Mouse position checked only in `Turn()`/`LookUp()` functions
- These functions only called when mouse **moves**
- Rotation stopped when mouse movement stopped
- Behavior: "Mouse delta with distance scaling" (not true X4-style)

**X4 System (Desired - Now Implemented):**
- Mouse position checked **every frame** in `Tick()`
- Cursor position determines rotation direction and speed
- Rotation continues as long as cursor is outside deadzone
- Behavior: "Cursor position → continuous rotation" (true X4-style)

## Implementation

### 1. Data Asset Changes

**File**: `Source/Adastrea/Public/Ships/SpaceshipDataAsset.h`

Added `RotationRateMultiplier` property to SpaceshipDataAsset:

```cpp
// Rotation rate multiplier (0-1, affects how quickly ship can rotate)
// 0.1 = Very slow, heavy ships (capital ships, freighters)
// 0.5 = Medium agility (most ships)
// 1.0 = Maximum agility (fighters, scouts)
// This is multiplied with base turn rate for final rotation speed
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mobility Stats", 
    meta=(ClampMin="0.1", ClampMax="1.0"))
float RotationRateMultiplier;
```

**Location**: Under "Mobility Stats" category alongside Maneuverability  
**Default Value**: 0.5 (medium agility)  
**Range**: 0.1 (very slow) to 1.0 (maximum agility)

### 2. Spaceship Class Changes

**File**: `Source/Adastrea/Public/Ships/Spaceship.h`

Added three new properties:

```cpp
/** Enable X4-style mouse position flight (cursor distance from center controls rotation speed) */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced")
bool bUseMousePositionFlight;

/** Deadzone radius for mouse position flight (pixels from screen center where no rotation occurs) */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", 
    meta=(ClampMin="0.0", ClampMax="500.0"))
float MouseDeadzoneRadius;

/** Maximum effective radius for mouse position flight (pixels from center at which rotation speed is 100%) */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", 
    meta=(ClampMin="100.0", ClampMax="2000.0"))
float MouseMaxRadius;
```

**Defaults:**
- `bUseMousePositionFlight = true` (X4-style enabled by default)
- `MouseDeadzoneRadius = 50.0f` (50 pixels from center)
- `MouseMaxRadius = 400.0f` (400 pixels = 100% rotation speed)

### 3. Rotation Logic Implementation (Corrected - 2026-01-15)

**File**: `Source/Adastrea/Private/Ships/Spaceship.cpp`

#### Key Change: Moved to Tick() Function

The critical fix was moving mouse position checking from `Turn()`/`LookUp()` (which only fire on mouse movement) to a new method `UpdateMousePositionFlight()` called every frame from `Tick()`.

#### New Method: UpdateMousePositionFlight()

```cpp
void ASpaceship::UpdateMousePositionFlight(float DeltaTime)
{
    /**
     * Called every frame when bUseMousePositionFlight is true.
     * Continuously checks mouse cursor position and rotates ship accordingly.
     * 
     * Key Behavior:
     * - If mouse is right of center → Ship continuously yaws right
     * - If mouse is left of center → Ship continuously yaws left
     * - Distance from center controls rotation speed
     * - Mouse can remain stationary and ship will continue rotating
     */

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    // Get mouse position and screen dimensions
    int32 ViewportSizeX, ViewportSizeY;
    float MouseX, MouseY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    PC->GetMousePosition(MouseX, MouseY);
    
    // Calculate center and distance
    float CenterX = ViewportSizeX * 0.5f;
    float CenterY = ViewportSizeY * 0.5f;
    float DeltaX = MouseX - CenterX;
    float DeltaY = MouseY - CenterY;
    float DistanceFromCenter = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
    
    // Check deadzone
    if (DistanceFromCenter < MouseDeadzoneRadius)
    {
        // Within deadzone, smoothly stop rotation
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, 
            DeltaTime, FlightAssistResponsiveness);
        RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, 0.0f, 
            DeltaTime, FlightAssistResponsiveness);
        return;
    }
    
    // Calculate rotation rates for both yaw and pitch
    float EffectiveDistance = DistanceFromCenter - MouseDeadzoneRadius;
    float MaxEffectiveDistance = MouseMaxRadius - MouseDeadzoneRadius;
    float DistanceRatio = FMath::Clamp(EffectiveDistance / MaxEffectiveDistance, 0.0f, 1.0f);
    
    float DirectionX = DeltaX / FMath::Max(DistanceFromCenter, 0.1f);
    float DirectionY = -DeltaY / FMath::Max(DistanceFromCenter, 0.1f); // Inverted for natural pitch
    
    float ShipRotationMultiplier = ShipDataAsset ? ShipDataAsset->RotationRateMultiplier : 1.0f;
    
    float YawRotationRate = DirectionX * DistanceRatio * TurnRate * 
        ShipRotationMultiplier * MouseFlightSensitivity;
    float PitchRotationRate = DirectionY * DistanceRatio * TurnRate * 
        ShipRotationMultiplier * MouseFlightSensitivity;
    
    // Smoothly interpolate rotation velocity
    RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, YawRotationRate, 
        DeltaTime, FlightAssistResponsiveness);
    RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, PitchRotationRate, 
        DeltaTime, FlightAssistResponsiveness);
    
    // Apply rotation to ship
    FRotator DeltaRotation = FRotator(RotationVelocity.Pitch * DeltaTime, 
        RotationVelocity.Yaw * DeltaTime, 0.0f);
    AddActorWorldRotation(DeltaRotation);
}
```

#### Updated Tick() Function

```cpp
void ASpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!GetController()) return;
    
    // NEW: Update mouse position flight every frame
    if (bUseMousePositionFlight && bFlightAssistEnabled)
    {
        UpdateMousePositionFlight(DeltaTime);
    }
    
    // Rest of flight physics...
    if (bFlightAssistEnabled)
    {
        ApplyFlightAssist(DeltaTime);
        UpdateThrottleVelocity(DeltaTime);
    }
    
    // Auto-leveling...
}
```

#### Modified Turn() and LookUp() Functions

These now **only** handle mouse delta mode (when `bUseMousePositionFlight = false`):

```cpp
void ASpaceship::Turn(float Value)
{
    YawInput = Value;
    
    if (bFlightAssistEnabled && !bUseMousePositionFlight)
    {
        // Mouse delta mode only
        // Mouse position mode handled in UpdateMousePositionFlight()
        float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, RotationRate, 
            DeltaTime, FlightAssistResponsiveness);
        FRotator DeltaRotation = FRotator(0.0f, RotationVelocity.Yaw * DeltaTime, 0.0f);
        AddActorWorldRotation(DeltaRotation);
    }
    // ... (flight assist disabled code)
}

// LookUp() has similar changes
```

## Mathematical Model

### Rotation Speed Formula

```
RotationSpeed = (Direction / Distance) × DistanceRatio × TurnRate × ShipMultiplier × Sensitivity

Where:
  Direction = MouseX - CenterX (for yaw) or -(MouseY - CenterY) (for pitch)
  Distance = sqrt((MouseX - CenterX)² + (MouseY - CenterY)²)
  DistanceRatio = Clamp((Distance - Deadzone) / (MaxRadius - Deadzone), 0, 1)
  TurnRate = Base rotation rate (45°/s default)
  ShipMultiplier = RotationRateMultiplier from DataAsset (0.1-1.0)
  Sensitivity = MouseFlightSensitivity (1.0 default)
```

### Deadzone System

```
If Distance < MouseDeadzoneRadius (50px):
  RotationSpeed = 0 (no rotation)
  
If Distance >= MouseDeadzoneRadius:
  EffectiveDistance = Distance - MouseDeadzoneRadius
  DistanceRatio = EffectiveDistance / (MouseMaxRadius - MouseDeadzoneRadius)
  RotationSpeed = Formula above
  
If Distance >= MouseMaxRadius (400px):
  DistanceRatio = 1.0 (100% rotation speed)
```

## Usage Examples

### Example 1: Agile Fighter Ship

**DataAsset Configuration:**
```
RotationRateMultiplier: 0.9
MaxSpeed: 1200 m/s
Acceleration: 200 m/s²
Maneuverability: 9
```

**Result**: Very responsive, quick turns, excellent for combat and dodging.

### Example 2: Medium Freighter

**DataAsset Configuration:**
```
RotationRateMultiplier: 0.5
MaxSpeed: 600 m/s
Acceleration: 80 m/s²
Maneuverability: 5
```

**Result**: Balanced handling, suitable for trading and general purpose.

### Example 3: Heavy Capital Ship

**DataAsset Configuration:**
```
RotationRateMultiplier: 0.2
MaxSpeed: 300 m/s
Acceleration: 30 m/s²
Maneuverability: 2
```

**Result**: Slow, deliberate turns, feels massive and powerful.

## Tuning Guidelines

### For Different Ship Roles

| Ship Role | Rotation Rate | Max Speed | Feel |
|-----------|--------------|-----------|------|
| Fighter | 0.8-1.0 | 1000-1500 | Agile, responsive |
| Scout | 0.7-0.9 | 800-1200 | Quick, maneuverable |
| Corvette | 0.6-0.8 | 600-1000 | Balanced agility |
| Frigate | 0.5-0.7 | 500-800 | Medium weight |
| Cruiser | 0.4-0.6 | 400-700 | Heavy but capable |
| Battleship | 0.3-0.5 | 300-600 | Slow, powerful |
| Freighter | 0.4-0.6 | 400-700 | Cargo-focused |
| Capital Ship | 0.1-0.3 | 200-400 | Massive, slow |

### Global Parameters

**Arcade Feel (Easy, Responsive):**
```
bUseMousePositionFlight: true
MouseDeadzoneRadius: 100.0
MouseMaxRadius: 300.0
FlightAssistResponsiveness: 5.0
RotationDampingFactor: 0.9
```

**Simulation Feel (Realistic, Challenging):**
```
bUseMousePositionFlight: false (mouse delta mode)
FlightAssistResponsiveness: 1.0
RotationDampingFactor: 0.5
```

**Combat Feel (Balanced):**
```
bUseMousePositionFlight: true
MouseDeadzoneRadius: 50.0
MouseMaxRadius: 400.0
FlightAssistResponsiveness: 3.0
RotationDampingFactor: 0.7
```

## Backward Compatibility

**Mouse Delta Mode Still Available:**
- Set `bUseMousePositionFlight = false` to use traditional mouse delta mode
- All existing flight assist features work unchanged
- Players can toggle between modes

**Existing Ships:**
- All ships get default `RotationRateMultiplier = 0.5`
- No breaking changes to existing DataAssets
- Ships without custom values use sensible default

## Testing Checklist

### Core Functionality
- [ ] Compile without errors
- [ ] **CRITICAL**: Ship rotates continuously when mouse cursor is held outside deadzone (not moving)
- [ ] **CRITICAL**: Ship stops rotating when cursor moves back into deadzone
- [ ] Mouse position affects rotation direction (left→yaw left, right→yaw right, up→pitch up, down→pitch down)
- [ ] Rotation speed scales with distance from center (0% at deadzone, 100% at max radius)
- [ ] Deadzone prevents rotation near center (within 50px)

### Ship-Specific Behavior
- [ ] Different ships have different rotation speeds based on RotationRateMultiplier
- [ ] Fighter (0.9 multiplier) rotates faster than freighter (0.3 multiplier)
- [ ] Ship multiplier affects both yaw and pitch equally

### Integration
- [ ] Flight assist smoothing still works
- [ ] Free look mode unaffected (camera independent rotation)
- [ ] Mouse delta mode toggle works (bUseMousePositionFlight = false)
- [ ] Flight assist can be toggled without breaking mouse position mode

### Performance
- [ ] Performance acceptable (no FPS drops)
- [ ] Mouse position checked every frame doesn't cause stuttering
- [ ] Multiple ships with mouse position flight don't cause issues

## Known Limitations

1. **Requires PlayerController**: Mouse position flight only works when ship is possessed by a player controller (not AI)
2. **Screen Space Only**: Rotation is based on 2D screen space, not 3D world space
3. **No Roll Control**: Current implementation doesn't use mouse position for roll (still uses Q/E keys if implemented)

## Future Enhancements

1. **Mouse Cursor Visibility**: Option to show/hide cursor during flight
2. **Visual Feedback**: On-screen indicator showing mouse position and deadzone
3. **Per-Axis Tuning**: Separate rotation multipliers for pitch, yaw, and roll
4. **Dynamic Adjustment**: Rotation rate affected by ship damage or power levels
5. **Input Curves**: Non-linear scaling for finer control near center

## References

- **Original Issue**: #446 - Controls feedback
- **X4: Foundations**: Flight model inspiration
- **Documentation**: 
  - `Assets/X4FlightControlsGuide.md` - Complete guide
  - `Assets/X4FlightControlsQuickReference.md` - Quick reference
- **Implementation Files**:
  - `Source/Adastrea/Public/Ships/SpaceshipDataAsset.h`
  - `Source/Adastrea/Public/Ships/Spaceship.h`
  - `Source/Adastrea/Private/Ships/Spaceship.cpp`
  - `Source/Adastrea/Private/Ships/SpaceshipDataAsset.cpp`

---

**Last Updated**: 2026-01-12  
**Implemented by**: GitHub Copilot Agent  
**Status**: ✅ Ready for Testing
