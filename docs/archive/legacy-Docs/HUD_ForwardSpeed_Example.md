# HUD Forward Speed Integration Example

This document provides a step-by-step example of integrating the forward speed display into your HUD widget.

## Prerequisites

- A HUD Widget Blueprint (e.g., `WBP_MainHUD`)
- Basic understanding of Unreal Engine Widget Blueprints
- The spaceship should be based on `BP_Spaceship` (which inherits from `ASpaceship`)

## Step-by-Step Integration

### 1. Add UI Elements to Your HUD Widget

In your HUD Widget Blueprint's Designer view:

1. Add a **Text Block** for displaying speed
   - Name: `SpeedText`
   - Default Text: "Speed: 0"
   
2. Add a **Progress Bar** for visual speed indicator (optional)
   - Name: `SpeedProgressBar`
   - Fill Color: Green to Red gradient based on speed

3. Add a **Text Block** for speed unit label (optional)
   - Text: "m/s" or "km/h"

### 2. Create Variables in Your HUD Widget

In the Graph view, add these variables:

- `CachedSpaceship` (Type: Spaceship, Object Reference)
  - Tooltip: "Reference to the controlled spaceship"
  - Category: "Cache"

- `DisplaySpeedUnitsMultiplier` (Type: Float, Default: 0.01)
  - Tooltip: "Multiplier to convert units (0.01 = cm/s to m/s)"
  - Category: "Display Settings"
  - Editable: Yes

### 3. Override UpdateHUDFromGameState Event

If your HUD inherits from `UAdastreaHUDWidget`, override the `UpdateHUDFromGameState` event:

```
Event UpdateHUDFromGameState (DeltaTime)
  │
  ├─→ Get Controlled Spaceship
  │   └─→ [Set CachedSpaceship]
  │
  ├─→ Branch (Is Valid: CachedSpaceship)
  │   │
  │   └─→ True:
  │       │
  │       ├─→ Get Forward Speed (Target: CachedSpaceship)
  │       │   └─→ [Forward Speed Float]
  │       │
  │       ├─→ Get Effective Max Speed (Target: CachedSpaceship)
  │       │   └─→ [Max Speed Float]
  │       │
  │       ├─→ Multiply (Forward Speed × DisplaySpeedUnitsMultiplier)
  │       │   └─→ [Display Speed Float]
  │       │
  │       ├─→ Format Text
  │       │   ├─ Format: "Speed: {Speed} m/s"
  │       │   ├─ Speed: [Display Speed Float]
  │       │   └─→ Set Text (SpeedText)
  │       │
  │       └─→ Divide (Forward Speed ÷ Max Speed)
  │           └─→ Set Percent (SpeedProgressBar)
  │
  └─→ False:
      └─→ Set Text (SpeedText, "Speed: N/A")
```

### 4. Alternative: Simple Tick Event

If not using `UpdateHUDFromGameState`, you can use the Tick event:

```
Event Tick (My Geometry, In Delta Time)
  │
  ├─→ Get Player Pawn
  │   └─→ Cast to Spaceship
  │       │
  │       ├─→ Success:
  │       │   │
  │       │   ├─→ Get Forward Speed
  │       │   │   └─→ [Speed Float]
  │       │   │
  │       │   ├─→ Format as Text
  │       │   │   ├─ Format: "{0} m/s"
  │       │   │   ├─ Multiply by 0.01 (cm/s to m/s)
  │       │   │   └─→ Set Text (SpeedText)
  │       │   │
  │       │   └─→ Update Speed (Call existing function)
  │       │
  │       └─→ Failed:
  │           └─→ Do nothing or hide speed display
```

## Advanced Features

### 1. Speed with Direction Indicator

Show whether the ship is moving forward or backward:

```
Get Forward Speed → [Speed]
  │
  ├─→ Select (Condition: Speed < 0)
  │   ├─ True: "⬅ Reverse"
  │   └─ False: "➡ Forward"
  │   └─→ Set Text (DirectionIndicator)
  │
  └─→ Abs (Speed)
      └─→ Display absolute speed
```

### 2. Color-Coded Speed Display

Change text color based on speed:

```
Get Forward Speed → [Speed]
Get Effective Max Speed → [MaxSpeed]
Divide → [Percentage]
  │
  ├─→ Select Color:
  │   ├─ 0-33%: Green (safe)
  │   ├─ 34-66%: Yellow (moderate)
  │   └─ 67-100%: Red (high speed)
  │
  └─→ Set Color and Opacity (SpeedText)
```

### 3. Smooth Speed Display (Anti-Jitter)

Use interpolation for smoother visual updates:

```
Variable: PreviousDisplaySpeed (Float)

Get Forward Speed → [RawSpeed]
  │
  ├─→ FInterp To (PreviousDisplaySpeed → RawSpeed, DeltaTime, InterpSpeed: 5.0)
  │   └─→ [Smoothed Speed]
  │
  ├─→ Set PreviousDisplaySpeed (Smoothed Speed)
  │
  └─→ Display Smoothed Speed
```

### 4. Multiple Speed Units

Allow players to choose their preferred units:

```
Variable: SpeedUnitsEnum (Enum: MetersPerSecond, KilometersPerHour, MilesPerHour)

Get Forward Speed → [Speed in cm/s]
  │
  └─→ Switch on SpeedUnitsEnum:
      ├─ MetersPerSecond: Multiply by 0.01
      ├─ KilometersPerHour: Multiply by 0.036
      └─ MilesPerHour: Multiply by 0.0224
      └─→ Display with appropriate unit label
```

## Testing Your Implementation

1. **In Editor (PIE)**:
   - Play in editor
   - Possess a spaceship
   - Verify the speed display appears
   
2. **Test Scenarios**:
   - Increase throttle - speed should increase
   - Decrease throttle - speed should decrease
   - Zero throttle - speed should gradually reach zero (with flight assist)
   - Lateral strafe - forward speed should remain stable
   - Boost mode - speed should show higher values
   - Travel mode - speed should show maximum values

3. **Edge Cases**:
   - No spaceship possessed - should show "N/A" or hide display
   - Spaceship destroyed - should handle gracefully
   - Quick possession changes - should update correctly

## Common Issues and Solutions

### Issue: Speed shows as 0 or doesn't update
**Solution**: Verify the spaceship has a MovementComponent and is actually moving.

### Issue: Speed value is too large
**Solution**: Apply the unit conversion multiplier (e.g., 0.01 for cm/s to m/s).

### Issue: Speed jitters/flickers
**Solution**: Use interpolation (FInterpTo) to smooth the display.

### Issue: Speed shows negative values
**Solution**: This is correct when moving backward. Use `Abs()` if you only want magnitude.

### Issue: HUD doesn't find spaceship
**Solution**: Ensure you're casting to the correct spaceship class and that the player is possessing it.

## Performance Considerations

- The `GetForwardSpeed()` function is lightweight (just a dot product)
- It's marked `BlueprintPure`, so it can be called multiple times per frame efficiently
- Consider updating the HUD at a lower frequency (e.g., 30 Hz instead of 60 Hz) for very complex HUDs
- Cache the spaceship reference instead of getting it every frame

## Example Blueprint Code Summary

**Minimal Implementation**:
```
Tick Event
  → Get Player Pawn
    → Cast to Spaceship
      → Get Forward Speed
        → Multiply by 0.01
          → Format Text: "Speed: {0} m/s"
            → Set Text (SpeedText)
```

**Full Implementation** (with all features):
```
UpdateHUDFromGameState
  → Get Controlled Spaceship
    → Is Valid?
      → Get Forward Speed
        → Smooth (FInterpTo)
          → Convert Units
            → Format with Direction
              → Color Code
                → Update Progress Bar
                  → Set Text
```

## Additional Resources

- See `Docs/ForwardSpeedFeature.md` for technical details
- Check `UAdastreaHUDWidget` class for existing HUD functions
- Review `ASpaceship` class for all available flight control functions
- Examine existing speed displays in other games for inspiration (X4: Foundations, Elite Dangerous, Star Citizen)
