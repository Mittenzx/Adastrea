# X4-Style Mouse Position Flight - Implementation Guide

**Status**: ✅ Implemented  
**Date**: 2026-01-12  
**Issue**: #446 follow-up - Controls improvement  
**Branch**: `copilot/fix-controls-issue`

---

## Overview

This document describes the implementation of X4: Foundations-style mouse position flight controls for Adastrea. The system allows the ship to rotate based on mouse cursor position on screen, with rotation speed scaling based on distance from center.

## Problem Statement

**Issue #446 Feedback:**
> "I don't know if flight assist is causing the problems. I think the difference between these controls and x4 is in x4 the ship turns and faces as per where the mouse is, faster the further from the center, but then adds something else that defines each ship. Like each ship has a variable between 0 and 1 that the rotation speed of the ship is multiplied by."

**Previous System:**
- Mouse movement (delta) → Fixed rotation rate (45°/s)
- Rotation speed constant regardless of mouse position
- No per-ship rotation customization

**X4 System (Desired):**
- Mouse position on screen → Rotation direction
- Distance from center → Rotation speed multiplier
- Per-ship rotation rate multiplier (0-1 range)

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

### 3. Rotation Logic Implementation

**File**: `Source/Adastrea/Private/Ships/Spaceship.cpp`

#### Turn() Function (Yaw Rotation)

```cpp
// Get ship-specific rotation rate multiplier from data asset
float ShipRotationMultiplier = 1.0f;
if (ShipDataAsset)
{
    ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
}

// X4-style mouse position flight
if (bUseMousePositionFlight)
{
    // Get mouse position relative to screen center
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        int32 ViewportSizeX, ViewportSizeY;
        float MouseX, MouseY;
        
        PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
        PC->GetMousePosition(MouseX, MouseY);
        
        // Calculate center of screen
        float CenterX = ViewportSizeX * 0.5f;
        float CenterY = ViewportSizeY * 0.5f;
        
        // Calculate distance from center
        float DeltaX = MouseX - CenterX;
        float DeltaY = MouseY - CenterY;
        
        // Apply deadzone
        float DistanceFromCenter = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
        if (DistanceFromCenter < MouseDeadzoneRadius)
        {
            // Within deadzone, no rotation
            YawInput = 0.0f;
            RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, 
                DeltaSeconds, FlightAssistResponsiveness);
            return;
        }
        
        // Calculate rotation speed based on distance from center (beyond deadzone)
        float EffectiveDistance = DistanceFromCenter - MouseDeadzoneRadius;
        float MaxEffectiveDistance = MouseMaxRadius - MouseDeadzoneRadius;
        float DistanceRatio = FMath::Clamp(EffectiveDistance / MaxEffectiveDistance, 0.0f, 1.0f);
        
        // Calculate rotation rate: distance ratio * base turn rate * ship multiplier * sensitivity
        float RotationRate = DeltaX / DistanceFromCenter * DistanceRatio * TurnRate * 
            ShipRotationMultiplier * MouseFlightSensitivity;
        
        // Interpolate rotation velocity for smooth feel
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, RotationRate, 
            DeltaSeconds, FlightAssistResponsiveness);
        
        // Apply rotation directly to actor
        FRotator DeltaRotation = FRotator(0.0f, RotationVelocity.Yaw * DeltaSeconds, 0.0f);
        AddActorWorldRotation(DeltaRotation);
    }
}
```

**Similar changes applied to:**
- `LookUp()` - Pitch rotation (inverted DeltaY)
- `Roll()` - Ship rotation multiplier applied

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

- [ ] Compile without errors
- [ ] Mouse position affects rotation direction
- [ ] Rotation speed scales with distance from center
- [ ] Deadzone prevents rotation near center (50px)
- [ ] Different ships have different rotation speeds
- [ ] Flight assist smoothing still works
- [ ] Free look mode unaffected
- [ ] Mouse delta mode toggle works
- [ ] Performance acceptable (no FPS drops)

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
