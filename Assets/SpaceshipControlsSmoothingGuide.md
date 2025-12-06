# Spaceship Controls Smoothing Guide

**Version:** 1.0  
**Last Updated:** December 6, 2024  
**System:** Spaceship Controls Component  
**For:** Game Designers and Developers

---

## Overview

This guide explains how to configure smooth spaceship controls in Adastrea. The smoothing system eliminates jerky mouse look and WASD movement by introducing interpolation between input and actual ship behavior.

### Key Features

- **Rotation Smoothing**: Camera rotates freely, ship follows with adjustable delay
- **Movement Smoothing**: WASD inputs are smoothly interpolated for natural feel
- **Turn Rate Integration**: Ship rotation respects turn rate while following camera
- **Designer-Friendly**: All parameters exposed in Blueprint for easy tuning
- **Performance Optimized**: Uses efficient interpolation with minimal overhead

---

## How It Works

### Camera vs Ship Rotation

Without smoothing, the ship rotates instantly with mouse movement, creating a jerky feel. With smoothing enabled:

1. **Mouse Input** → Updates camera/controller rotation instantly
2. **Ship Rotation** → Smoothly interpolates towards camera rotation over time
3. **Turn Rate** → Controlled by `RotationSmoothingSpeed` parameter

This creates a more realistic feel where the ship's physical mass and inertia are represented.

### Movement Smoothing

WASD input is smoothed to avoid instant acceleration/deceleration:

1. **Keyboard Input** → WASD provides target movement direction
2. **Interpolation** → Current movement smoothly transitions to target
3. **Natural Feel** → Ship feels like it has weight and momentum

---

## Configuration Parameters

All parameters are found on the **SpaceshipControlsComponent** in Blueprint.

### Rotation Smoothing

#### `bEnableRotationSmoothing` (bool)
- **Default:** `true`
- **Description:** Enables camera-to-ship rotation smoothing
- **When true:** Camera rotates freely, ship follows with delay
- **When false:** Ship rotates instantly with camera (legacy behavior)

#### `RotationSmoothingSpeed` (float, 0.0 - 20.0)
- **Default:** `5.0`
- **Description:** How quickly ship rotation catches up to camera
- **Lower values (1.0 - 3.0):** Slow, heavy ship feel (capital ships, freighters)
- **Medium values (4.0 - 7.0):** Balanced response (fighters, corvettes)
- **Higher values (8.0 - 20.0):** Quick response, minimal delay (interceptors)
- **0.0:** No smoothing (instant rotation)

**Recommended Values by Ship Type:**
```
- Fighter/Interceptor:     8.0 - 12.0
- Scout/Light Ship:        6.0 - 9.0
- Corvette/Frigate:        4.0 - 6.0
- Cruiser/Destroyer:       3.0 - 5.0
- Battleship/Capital:      1.5 - 3.0
- Freighter/Mining:        1.0 - 2.5
```

### Movement Smoothing

#### `bEnableMovementSmoothing` (bool)
- **Default:** `true`
- **Description:** Enables WASD movement input smoothing
- **When true:** Movement input is interpolated for smooth acceleration
- **When false:** Movement responds instantly to input (arcade feel)

#### `MovementSmoothingSpeed` (float, 0.0 - 20.0)
- **Default:** `10.0`
- **Description:** How quickly movement responds to WASD input
- **Lower values (2.0 - 5.0):** Sluggish, heavy ship feel
- **Medium values (6.0 - 12.0):** Realistic response with inertia
- **Higher values (13.0 - 20.0):** Snappy, arcade-style movement
- **0.0:** No smoothing (instant response)

**Recommended Values by Ship Type:**
```
- Fighter/Interceptor:     12.0 - 15.0
- Scout/Light Ship:        10.0 - 13.0
- Corvette/Frigate:        8.0 - 11.0
- Cruiser/Destroyer:       6.0 - 9.0
- Battleship/Capital:      3.0 - 6.0
- Freighter/Mining:        2.0 - 5.0
```

### Additional Parameters

#### `LookSensitivity` (float, 0.1 - 10.0)
- **Default:** `1.0`
- **Description:** Mouse sensitivity multiplier
- **Still affects camera rotation speed**
- **Works in conjunction with smoothing**

#### `MovementSpeed` (float, 0.1 - 10.0)
- **Default:** `1.0`
- **Description:** Base movement speed multiplier
- **Affects WASD strafe speed**
- **Applied before smoothing**

---

## Setup Instructions

### For Blueprint Users

1. **Open Your Spaceship Blueprint**
   - Locate the spaceship Blueprint in `Content/Blueprints/Ships/`
   - Or open your custom spaceship BP

2. **Select SpaceshipControlsComponent**
   - In the Components panel, select the SpaceshipControlsComponent
   - If not present, add it: Add Component → SpaceshipControlsComponent

3. **Configure Rotation Smoothing**
   - In the Details panel, find "Controls|Look" category
   - Set `bEnableRotationSmoothing` to `true`
   - Adjust `RotationSmoothingSpeed` (start with `5.0`)
   - Set `LookSensitivity` to desired mouse sensitivity

4. **Configure Movement Smoothing**
   - Find "Controls|Movement" category
   - Set `bEnableMovementSmoothing` to `true`
   - Adjust `MovementSmoothingSpeed` (start with `10.0`)
   - Set `MovementSpeed` to base strafe speed

5. **Test and Tune**
   - Use Play In Editor (PIE) to test controls
   - Adjust smoothing speeds to achieve desired feel
   - Different ship types should have different values

### For C++ Users

```cpp
// In your spaceship or game mode class
USpaceshipControlsComponent* ControlsComp = Spaceship->FindComponentByClass<USpaceshipControlsComponent>();
if (ControlsComp)
{
    // Enable smoothing
    ControlsComp->bEnableRotationSmoothing = true;
    ControlsComp->RotationSmoothingSpeed = 5.0f;
    
    ControlsComp->bEnableMovementSmoothing = true;
    ControlsComp->MovementSmoothingSpeed = 10.0f;
    
    // Adjust sensitivity
    ControlsComp->SetLookSensitivity(1.5f);
    ControlsComp->SetMovementSpeed(2.0f);
}
```

---

## Advanced Usage

### Ship Type Presets

Create Data Assets or Blueprint functions to apply preset configurations:

**Fighter Preset:**
```
RotationSmoothingSpeed: 10.0
MovementSmoothingSpeed: 14.0
LookSensitivity: 1.2
MovementSpeed: 1.5
```

**Capital Ship Preset:**
```
RotationSmoothingSpeed: 2.0
MovementSmoothingSpeed: 4.0
LookSensitivity: 0.7
MovementSpeed: 0.8
```

### Dynamic Adjustment

You can adjust smoothing at runtime based on:
- Ship damage (slower response when damaged)
- Power levels (faster response with boosters)
- Player skill level (more/less assistance)

```cpp
// Blueprint example
void ASpaceship::OnShipDamaged(float DamageAmount)
{
    USpaceshipControlsComponent* Controls = FindComponentByClass<USpaceshipControlsComponent>();
    if (Controls)
    {
        // Reduce responsiveness when damaged
        float DamagePercent = CurrentHull / MaxHull;
        Controls->RotationSmoothingSpeed = FMath::Lerp(2.0f, 10.0f, DamagePercent);
    }
}
```

### Integration with Flight Assist

The smoothing system works alongside the X4-style flight assist system in `ASpaceship`:

- **Flight Assist ON + Smoothing ON**: Most realistic, smooth flight model
- **Flight Assist ON + Smoothing OFF**: Assisted but instant response
- **Flight Assist OFF + Smoothing ON**: Newtonian physics with smooth controls
- **Flight Assist OFF + Smoothing OFF**: Pure arcade/manual control

---

## Technical Details

### Implementation Overview

The smoothing system is implemented in `USpaceshipControlsComponent`:

1. **Input Handling**: Raw input captured in `HandleMove()` and `HandleLook()`
2. **Interpolation**: Uses `FMath::Vector2DInterpTo()` and `FMath::RInterpTo()`
3. **Tick Update**: `TickComponent()` applies smoothed rotation to ship actor
4. **Framerate Independent**: Uses delta time for consistent behavior

### Performance

- **CPU Cost**: Minimal (2-3 interpolation operations per frame)
- **Memory**: 4 additional floats per component (~16 bytes)
- **Tick Cost**: Only active when controls enabled
- **Optimized**: Early returns prevent unnecessary calculations

### Compatibility

- Works with Enhanced Input System
- Compatible with legacy input system
- Integrates with X4-style flight controls
- Blueprint-overridable for custom behavior

---

## Troubleshooting

### Problem: Ship still feels jerky

**Solution:**
- Increase `RotationSmoothingSpeed` for rotation issues
- Increase `MovementSmoothingSpeed` for movement issues
- Check if smoothing is enabled (`bEnableRotationSmoothing` / `bEnableMovementSmoothing`)

### Problem: Ship responds too slowly

**Solution:**
- Decrease smoothing speed values
- Ensure `LookSensitivity` and `MovementSpeed` are not too low
- Consider if ship type should feel sluggish (capital ships)

### Problem: Ship rotation doesn't match camera

**Solution:**
- This is expected behavior with smoothing enabled
- Reduce `RotationSmoothingSpeed` for less delay
- Set `bEnableRotationSmoothing = false` for instant rotation

### Problem: Controls feel floaty

**Solution:**
- Increase smoothing speeds for snappier response
- Check that base speed values aren't too high
- Adjust flight assist parameters in `ASpaceship` class

---

## Examples

### Example 1: Fighter Ship Configuration

```
Component: SpaceshipControlsComponent
├── Controls|Look
│   ├── bEnableRotationSmoothing: true
│   ├── RotationSmoothingSpeed: 10.0
│   ├── LookSensitivity: 1.3
│   └── bInvertLookY: false
└── Controls|Movement
    ├── bEnableMovementSmoothing: true
    ├── MovementSmoothingSpeed: 14.0
    └── MovementSpeed: 1.5
```

**Result:** Responsive, agile fighter with quick turning and smooth controls

### Example 2: Freighter Configuration

```
Component: SpaceshipControlsComponent
├── Controls|Look
│   ├── bEnableRotationSmoothing: true
│   ├── RotationSmoothingSpeed: 2.0
│   ├── LookSensitivity: 0.8
│   └── bInvertLookY: false
└── Controls|Movement
    ├── bEnableMovementSmoothing: true
    ├── MovementSmoothingSpeed: 4.0
    └── MovementSpeed: 0.7
```

**Result:** Heavy, sluggish freighter with realistic inertia

### Example 3: Arcade Mode (No Smoothing)

```
Component: SpaceshipControlsComponent
├── Controls|Look
│   ├── bEnableRotationSmoothing: false
│   ├── LookSensitivity: 1.5
│   └── bInvertLookY: false
└── Controls|Movement
    ├── bEnableMovementSmoothing: false
    └── MovementSpeed: 2.0
```

**Result:** Instant response, arcade-style controls

---

## Best Practices

### Design Guidelines

1. **Match Ship Size**: Larger ships should have slower smoothing
2. **Test Early**: Configure smoothing during ship creation
3. **Player Options**: Consider exposing sensitivity as player settings
4. **Consistent Feel**: Use similar values across same ship class

### Tuning Process

1. Start with default values (5.0 rotation, 10.0 movement)
2. Test in-game with typical scenarios
3. Adjust rotation speed first, then movement
4. Fine-tune sensitivity multipliers last
5. Get player feedback on control feel

### Common Mistakes

❌ **Don't:** Set smoothing speeds too low (< 1.0)
✅ **Do:** Use 1.0+ for all ship types

❌ **Don't:** Use same values for all ship types
✅ **Do:** Customize per ship class/size

❌ **Don't:** Forget to enable smoothing booleans
✅ **Do:** Check `bEnableRotationSmoothing` and `bEnableMovementSmoothing`

---

## Related Systems

- **SpaceshipControlsComponent**: Main controls component
- **ASpaceship**: Base ship class with flight assist
- **Enhanced Input System**: Input handling architecture
- **SpaceshipDataAsset**: Ship configuration data
- **FloatingPawnMovement**: Physics and movement

---

## Additional Resources

- **SpaceshipDataAssetGuide.md**: Ship configuration
- **SpaceshipDesignerWorkflow.md**: Creating new ships
- **EnhancedInputImplementation.md**: Input system details
- **ARCHITECTURE.md**: System architecture overview

---

## FAQ

**Q: Should I always enable smoothing?**  
A: For most players, yes. It provides a more realistic feel. Consider adding a toggle in settings for arcade mode fans.

**Q: What's the difference between rotation smoothing and flight assist?**  
A: Rotation smoothing controls how ship follows camera. Flight assist controls velocity/rotation damping in space physics.

**Q: Can I animate the transition between smoothing modes?**  
A: Yes! You can interpolate the smoothing speed values themselves for dynamic effects.

**Q: Does this work with gamepad/controller input?**  
A: Yes, smoothing applies to all input sources (mouse, gamepad, keyboard).

**Q: Will this affect AI-controlled ships?**  
A: No, smoothing only applies when `SpaceshipControlsComponent` is active with player input.

---

## Version History

**v1.0** (December 6, 2024)
- Initial implementation
- Added rotation and movement smoothing
- Full Blueprint exposure
- Integration with existing flight controls

---

## Support

For questions or issues with spaceship controls smoothing:
1. Check this guide and related documentation
2. Review example ship configurations
3. Test with different parameter values
4. Submit issues to project repository

---

**End of Guide**
