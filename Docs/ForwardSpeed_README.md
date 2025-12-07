# Forward Speed Feature - Quick Reference

## What Was Added

A new C++ function `GetForwardSpeed()` has been added to the `ASpaceship` class to enable HUD displays of the ship's current forward speed.

## Quick Start for Designers

### Using in Blueprint HUD Widget

1. **Get the spaceship reference:**
   ```
   Get Player Pawn → Cast to Spaceship
   ```

2. **Get the forward speed:**
   ```
   Call "Get Forward Speed" on the spaceship
   ```

3. **Display the speed:**
   ```
   Format Text with the speed value → Set Text on your speed display widget
   ```

### Example Blueprint Node Chain

```
Tick Event
  → Get Player Pawn
    → Cast to Spaceship
      → Get Forward Speed
        → Multiply by 0.01 (convert cm/s to m/s)
          → Format Text: "Speed: {0} m/s"
            → Set Text (Speed Text Widget)
```

## Key Features

- ✅ **Blueprint-Callable**: Can be used directly in Blueprint widgets
- ✅ **Pure Function**: No execution pin needed, can be called anywhere
- ✅ **Safe**: Returns 0 if MovementComponent is invalid
- ✅ **Accurate**: Uses dot product for precise forward speed calculation
- ✅ **Supports All Modes**: Works with normal flight, boost, and travel mode

## Return Value

- **Positive**: Ship moving forward
- **Negative**: Ship moving backward  
- **Zero**: Ship stationary or moving only sideways

Units: Unreal units per second (typically cm/s, multiply by 0.01 for m/s)

## Files Modified

- `Source/Adastrea/Ships/Spaceship.h` - Added function declaration
- `Source/Adastrea/Ships/Spaceship.cpp` - Added function implementation

## Documentation

- **Technical Details**: See `Docs/ForwardSpeedFeature.md`
- **HUD Integration Guide**: See `Docs/HUD_ForwardSpeed_Example.md`

## Function Signature

```cpp
UFUNCTION(BlueprintCallable, BlueprintPure, Category="Flight Control")
float GetForwardSpeed() const;
```

## Integration with Existing HUD

The `UAdastreaHUDWidget` class has an `UpdateSpeed()` function you can use:

```cpp
UpdateSpeed(float CurrentSpeed, float MaxSpeed)
```

Call it with:
- `CurrentSpeed`: Value from `GetForwardSpeed()`
- `MaxSpeed`: Value from `GetEffectiveMaxSpeed()`

## Testing

To verify it works:

1. Create a simple text widget in your HUD
2. In Tick event, get forward speed and display it
3. Play in editor and possess a spaceship
4. Increase throttle and watch speed increase
5. Use boost/travel mode to see higher speeds

## Common Use Cases

1. **Speed Indicator**: Show current speed on HUD
2. **Speed Bar**: Visual progress bar of speed vs max speed
3. **Cruise Control**: Display when at target speed
4. **Landing Assist**: Show speed during docking
5. **Collision Warning**: Alert when approaching at high speed

## Notes

- The function is thread-safe and const-correct
- No performance impact (simple dot product calculation)
- Works with flight assist on or off
- Compatible with all ship types inheriting from ASpaceship

## Support

For questions or issues:
- Check the detailed documentation in `Docs/ForwardSpeedFeature.md`
- Review the example implementations in `Docs/HUD_ForwardSpeed_Example.md`
- See the Adastrea project's main documentation

---

**Created**: 2025-12-07  
**Issue**: #236 (Add spaceship forward speed in cpp for HUD)  
**Status**: ✅ Implemented and Tested
