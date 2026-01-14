# Forward Speed Feature

## Overview

A new function `GetForwardSpeed()` has been added to the `ASpaceship` class to allow the HUD to display the ship's current forward speed.

## Implementation Details

### C++ Function

The function is declared in `Source/Adastrea/Ships/Spaceship.h` and implemented in `Source/Adastrea/Ships/Spaceship.cpp`:

```cpp
/**
 * Get current forward speed in the ship's forward direction
 * @return Current forward speed in units per second
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Flight Control")
float GetForwardSpeed() const;
```

### How It Works

The function:
1. Checks if the `MovementComponent` is valid
2. Gets the ship's forward vector using `GetActorForwardVector()`
3. Projects the velocity onto the forward vector using dot product
4. Returns the forward speed in units per second

This approach correctly calculates the speed in the direction the ship is facing, which can be:
- **Positive**: Ship is moving forward
- **Negative**: Ship is moving backward
- **Zero**: Ship is stationary or moving only laterally

## Usage in Blueprints

### In HUD Widget

To display the forward speed in your HUD:

1. **Get the Controlled Spaceship**
   - Use `GetControlledSpaceship()` function (already available in `UAdastreaHUDWidget`)
   - Cast result to `ASpaceship`

2. **Call GetForwardSpeed()**
   - This is a `BlueprintPure` function (doesn't require execution pin)
   - Returns a `float` value

3. **Format and Display**
   - Use `UpdateSpeed()` function in `UAdastreaHUDWidget`
   - Format the speed value as needed (e.g., convert to different units, round, add labels)

### Example Blueprint Logic

```
Event Tick (or UpdateHUDFromGameState)
  ↓
Get Controlled Spaceship
  ↓
Branch (Is Valid?)
  ↓ True
Get Forward Speed
  ↓
Update Speed (CurrentSpeed, MaxSpeed)
  ↓
Display in UI (Text, Progress Bar, etc.)
```

### Example Usage in Widget Blueprint

```
// In your HUD Widget Blueprint's Tick or UpdateHUDFromGameState function:

1. Get Controlled Spaceship → Spaceship Reference
2. Get Forward Speed (from Spaceship Reference) → Forward Speed (float)
3. Get Effective Max Speed (from Spaceship Reference) → Max Speed (float)
4. Update Speed (Forward Speed, Max Speed)
```

### Formatting Speed for Display

You may want to format the speed for better readability:

- **Absolute Value**: Use `Abs()` if you only want to show magnitude
- **Rounding**: Use `Round()` or `Floor()` to remove decimals
- **Units**: Divide by 100 to convert to "meters per second" if using cm/s
- **Speed Indicators**: Add visual indicators for forward/backward motion

### Integration with Existing HUD

The `UAdastreaHUDWidget` class already has an `UpdateSpeed()` function:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Navigation")
void UpdateSpeed(float CurrentSpeed, float MaxSpeed);
```

Simply call this function with:
- `CurrentSpeed`: Result from `GetForwardSpeed()`
- `MaxSpeed`: Result from `GetEffectiveMaxSpeed()`

## Benefits

1. **Real-time Feedback**: Players can see their current speed relative to max speed
2. **Flight Assist Clarity**: Shows actual velocity, not just throttle setting
3. **Navigation Aid**: Helps with docking, landing, and precise maneuvers
4. **Combat Information**: Important for dogfighting and evasive maneuvers
5. **Blueprint Accessible**: No C++ knowledge required to use in HUD

## Technical Notes

- The function is marked `BlueprintPure`, so it doesn't require an execution pin
- It's marked `const`, meaning it doesn't modify the ship's state
- Returns 0.0f if the MovementComponent is not valid (safety check)
- Uses dot product for accurate forward speed calculation
- Works correctly with all flight modes (normal, boost, travel)
- Handles negative speeds when moving backward

## Testing

To test the implementation:

1. Create a simple HUD widget with a text field
2. In the widget's Tick or Update event:
   - Get the controlled spaceship
   - Call `GetForwardSpeed()`
   - Display the result in the text field
3. In PIE (Play In Editor):
   - Possess a spaceship
   - Use throttle controls to move
   - Verify the speed display updates correctly
4. Test different scenarios:
   - Forward movement (positive speed)
   - Backward movement (negative speed)
   - Lateral strafing (speed should remain near zero)
   - Boost mode (higher speeds)
   - Travel mode (even higher speeds)

## Future Enhancements

Potential additions for the future:

- `GetLateralSpeed()`: Speed perpendicular to forward direction
- `GetVerticalSpeed()`: Speed in up/down direction
- `GetTotalSpeed()`: Magnitude of velocity (3D speed)
- Speed history tracking for acceleration display
- Target relative speed calculation
