# X4-Style Flight Controls Guide

## Overview

The Adastrea spaceship system now implements X4: Foundations-style flight controls, providing a realistic and intuitive space flight experience. This guide explains the flight model, configuration options, and how to tune the controls.

## Flight Model Characteristics

### 1. Flight Assist Mode (Enabled by Default)

**What it does:**
- Ship maintains orientation when no input is given
- Rotation is smoothly damped for natural feel
- Velocity is maintained when no movement input is given
- Ship responds to throttle setting rather than direct acceleration

**Why it matters:**
This creates the "fly-by-wire" feel of X4: Foundations where the ship computer assists the pilot in maintaining stable flight. It's easier for new players while still allowing skilled pilots to disable it for pure Newtonian physics.

**Toggle:** Press `V` key (default) or call `ToggleFlightAssist()` function

### 2. Inertia & Drift

**How it works:**
- In space, objects maintain their velocity unless acted upon by a force
- With flight assist ON: Ship maintains velocity when no input is given
- With flight assist OFF: Pure Newtonian physics - ship drifts indefinitely

**Design note:**
This is realistic space physics. Unlike atmospheric flight where air resistance slows you down, in space you keep moving until you apply counter-thrust.

### 3. Auto-Leveling

**How it works:**
- When no yaw input is given, ship automatically levels its roll to the ecliptic plane (Z-up)
- Strength controlled by `AutoLevelStrength` parameter (0 = off, 1 = instant)
- Only active when flight assist is enabled

**Why it's useful:**
Prevents the ship from slowly drifting into awkward orientations during exploration and combat. Makes it easier to maintain situational awareness.

### 4. Throttle System (Percentage-Based)

**How it works:**
- Throttle (0-100%) controls target velocity, not acceleration
- Throttle 0% = ship comes to a stop
- Throttle 50% = ship maintains half speed
- Throttle 100% = ship maintains max speed
- Ship automatically accelerates/decelerates to match throttle setting

**Controls:**
- `Mouse Wheel Up` or `=` key: Increase throttle
- `Mouse Wheel Down` or `-` key: Decrease throttle
- Increment: 10% per press (configurable via `ThrottleStep`)

**Design note:**
This is intuitive speed control where you set a target speed and the flight computer handles the acceleration curve. It's like cruise control for spaceships.

### 5. Strafe Independence

**How it works:**
- Lateral (A/D keys) and vertical (Space/Ctrl keys) strafing is independent from forward motion
- Controlled by `StrafeIndependence` parameter (0-1, default 0.8)
- RCS thrusters provide directional thrust without affecting forward velocity

**Why it matters:**
Allows complex maneuvering like strafing while maintaining forward speed. Essential for combat and docking.

### 6. Mouse Flight (X4-Style Position-Based)

**How it works:**
- Mouse cursor position on screen determines rotation direction and speed
- Distance from screen center determines rotation speed (farther = faster)
- Deadzone in center (50 pixels default) prevents accidental rotation
- Maximum effective radius (400 pixels default) for 100% rotation speed
- Each ship has a rotation rate multiplier (0.1-1.0) affecting turn speed
- Toggle between position-based and traditional mouse delta modes

**Configuration:**
- `bUseMousePositionFlight`: Enable X4-style position mode (default: true)
- `MouseDeadzoneRadius`: Deadzone from center in pixels (default: 50)
- `MouseMaxRadius`: Max effective distance for 100% rotation (default: 400)
- `RotationRateMultiplier` (in ShipDataAsset): Per-ship agility (0.1-1.0)

**Ship Rotation Rate Examples:**
- 0.1 = Very slow, heavy ships (capital ships, large freighters)
- 0.3 = Slow but stable (medium freighters, industrial ships)
- 0.5 = Medium agility (default, balanced ships)
- 0.7 = Agile (light combat ships, scouts)
- 1.0 = Maximum agility (fighters, interceptors)

**Design note:**
This is the true X4: Foundations flight model - cursor position determines where the ship rotates to, with rotation speed scaling smoothly based on distance from center. Each ship type can have unique handling characteristics through the rotation rate multiplier.

### 7. Boost Mode

**How it works:**
- Temporary speed increase (default 2x speed)
- Activated by holding `Left Shift`
- Cannot be used while in Travel Mode
- Useful for combat maneuvering and quick repositioning

**Controls:**
- Hold `Left Shift`: Activate boost
- Release `Left Shift`: Deactivate boost

**Configuration:**
- `BoostMultiplier`: Speed multiplier when boosting (default 2.0x)

### 8. Travel Mode

**How it works:**
- High-speed cruise mode for long distances (default 5x speed)
- Automatically sets throttle to 100%
- Disables boost while active
- Toggle on/off with `J` key

**When to use:**
- Long distance travel between stations
- Exploring large star systems
- Not recommended for combat or precision maneuvering

**Configuration:**
- `TravelModeMultiplier`: Speed multiplier in travel mode (default 5.0x)

## Configuration Parameters

All parameters can be tuned in the Unreal Editor or via Blueprint:

### Flight Assist Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `bFlightAssistEnabled` | `true` | bool | Enable/disable flight assist |
| `RotationDampingFactor` | `0.85` | 0.0-1.0 | Rotation smoothing (higher = more damping) |
| `AutoLevelStrength` | `0.5` | 0.0-1.0 | Auto-level intensity (0 = off, 1 = instant) |
| `FlightAssistResponsiveness` | `2.0` | 0.1-10.0 | How quickly ship responds to input |

### Throttle Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `ThrottlePercentage` | `0.0` | 0.0-100.0 | Current throttle setting (read-only) |
| `ThrottleStep` | `10.0` | 1.0-25.0 | Throttle increment per input (%) |

### Boost Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `bBoostActive` | `false` | bool | Boost currently active (read-only) |
| `BoostMultiplier` | `2.0` | 1.0-5.0 | Speed multiplier when boosting |

### Travel Mode Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `bTravelModeActive` | `false` | bool | Travel mode active (read-only) |
| `TravelModeMultiplier` | `5.0` | 2.0-20.0 | Speed multiplier in travel mode |

### Advanced Parameters

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `StrafeIndependence` | `0.8` | 0.0-1.0 | Strafe independence from forward motion |
| `MouseFlightSensitivity` | `1.0` | 0.1-5.0 | Mouse rotation sensitivity |
| `bUseMousePositionFlight` | `true` | bool | X4-style position mode vs. delta mode |
| `MouseDeadzoneRadius` | `50.0` | 0.0-500.0 | Deadzone from center (pixels) |
| `MouseMaxRadius` | `400.0` | 100.0-2000.0 | Max distance for 100% rotation (pixels) |

### Ship-Specific Parameters (in SpaceshipDataAsset)

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `RotationRateMultiplier` | `0.5` | 0.1-1.0 | Ship-specific rotation agility |

## Default Controls

### Keyboard & Mouse

| Action | Key/Input | Description |
|--------|-----------|-------------|
| Forward Speed | `Mouse Wheel Up` or `=` | Increase forward speed (+10% throttle) |
| Forward Speed | `Mouse Wheel Down` or `-` | Decrease forward speed (-10% throttle) |
| Move Up | `W` or `Space` | Vertical thrust up |
| Move Down | `S` or `Left Ctrl` | Vertical thrust down |
| Strafe Left | `A` | Strafe left |
| Strafe Right | `D` | Strafe right |
| Rotate | `Mouse Movement` | Pitch and yaw rotation |
| Boost | `Left Shift` (hold) | Activate boost mode |
| Flight Assist | `V` | Toggle flight assist on/off |
| Travel Mode | `J` | Toggle travel mode on/off |

### Gamepad (Future)

Gamepad support can be added via Enhanced Input system by creating Input Actions for throttle and flight assist controls.

## Tuning the Flight Model

### For Arcade-Style Flight (Easy, Responsive)

```cpp
RotationDampingFactor = 0.9;      // High damping for smooth rotation
AutoLevelStrength = 0.8;          // Strong auto-leveling
FlightAssistResponsiveness = 5.0; // Very responsive
ThrottleStep = 20.0;              // Large throttle increments
MouseDeadzoneRadius = 100.0;      // Larger deadzone for stability
bUseMousePositionFlight = true;   // X4-style position mode
```

**Ship Setup:**
- Set `RotationRateMultiplier = 0.7-1.0` for agile feel

### For Simulation-Style Flight (Realistic, Challenging)

```cpp
RotationDampingFactor = 0.5;      // Low damping for more inertia
AutoLevelStrength = 0.2;          // Weak auto-leveling
FlightAssistResponsiveness = 1.0; // Slower response
ThrottleStep = 5.0;               // Fine throttle control
MouseDeadzoneRadius = 30.0;       // Smaller deadzone for precision
bUseMousePositionFlight = false;  // Traditional mouse delta mode
```

**Ship Setup:**
- Set `RotationRateMultiplier = 0.3-0.5` for heavier, realistic feel

### For Combat-Focused Flight (Agile, Precise)

```cpp
RotationDampingFactor = 0.7;      // Moderate damping
AutoLevelStrength = 0.3;          // Some auto-leveling
FlightAssistResponsiveness = 3.0; // Quick response
BoostMultiplier = 2.5;            // Strong boost for dodging
MouseDeadzoneRadius = 50.0;       // Balanced deadzone
bUseMousePositionFlight = true;   // X4-style for quick targeting
```

**Ship Setup:**
- Fighters: `RotationRateMultiplier = 0.9-1.0`
- Medium combat: `RotationRateMultiplier = 0.6-0.8`
- Heavy combat: `RotationRateMultiplier = 0.4-0.5`

## Blueprint Integration

All flight control functions are `BlueprintCallable` and can be used in Blueprint scripts:

### Example: Custom Throttle Control

```blueprint
[Event Graph]
  
  [Key Press: Numpad 0]
    ↓
  [Set Throttle] → 0% (Full Stop)

  [Key Press: Numpad 5]
    ↓
  [Set Throttle] → 50% (Cruise Speed)

  [Key Press: Numpad 10]  (doesn't exist - use Numpad Period for full throttle)
    ↓
  [Set Throttle] → 100% (Max Speed)
```

### Example: Auto Boost in Combat

```blueprint
[Event Tick]
  ↓
[Get Targeting Component] → [Has Target?]
  ↓ (True)
[Is Within Range?] (Distance < 500)
  ↓ (True)
[Activate Boost]
  ↓ (False)
[Deactivate Boost]
```

### Example: Flight Assist Toggle Notification

```blueprint
[Toggle Flight Assist] → [Called]
  ↓
[Branch] → bFlightAssistEnabled?
  ↓ (True)
[Print String] → "Flight Assist: ON"
  ↓ (False)
[Print String] → "Flight Assist: OFF"
```

## Advanced Topics

### Inertia Preservation

When flight assist is disabled, the ship preserves its velocity vector. This means:
- Ship continues moving in the same direction at the same speed
- Rotation doesn't affect velocity (you can spin while moving straight)
- You must apply counter-thrust to change direction or stop

This is pure Newtonian physics - realistic but challenging for new players.

### Rotation Velocity System

The ship uses a rotation velocity system for smooth rotation:
- Input changes are interpolated over time
- No sudden jerky movements
- Rotation "feels" like a real spacecraft with mass

### Throttle vs. Direct Input

The system supports two input modes:

**Throttle Mode (X4-style):**
- Set forward velocity with mouse wheel throttle
- W/S controls vertical movement (up/down)
- A/D provides lateral strafing
- Good for long-distance travel and exploration

**Direct Input Mode (Traditional):**
- Mouse wheel directly controls forward speed
- W/S for vertical thrust
- Good for combat and precision maneuvering

The throttle system (mouse wheel) controls forward speed, while W/S/A/D handle vertical and lateral strafing.

## Troubleshooting

### Mouse Position Not Affecting Rotation

**Solution:**
- Check `bUseMousePositionFlight` is enabled (default: true)
- Verify mouse is outside deadzone (`MouseDeadzoneRadius = 50` pixels)
- Ensure flight assist is enabled (`bFlightAssistEnabled = true`)
- Check ship has valid `RotationRateMultiplier` in DataAsset (default: 0.5)

### Rotation Too Sensitive/Not Sensitive Enough

**Solution:**
- Adjust `MouseFlightSensitivity` (default: 1.0)
- Modify `MouseMaxRadius` (smaller = more sensitive, larger = less sensitive)
- Adjust ship's `RotationRateMultiplier` (0.1 = slow, 1.0 = fast)

### Ship Rotates Even When Mouse Near Center

**Solution:**
- Increase `MouseDeadzoneRadius` (try 75-100 pixels)
- Check if mouse delta mode is active (`bUseMousePositionFlight = false`)

### Different Ships Feel the Same

**Solution:**
- Set different `RotationRateMultiplier` values in each ship's DataAsset
- Fighters: 0.8-1.0, Medium ships: 0.5-0.7, Heavy ships: 0.2-0.4
- Ensure DataAsset is assigned to ship in Blueprint

### Auto-Leveling Too Strong/Weak

**Solution:**
- Adjust `AutoLevelStrength` (0.0 = off, 1.0 = instant)
- Typical range: 0.3-0.7 for balanced feel

### Throttle Not Working

**Check:**
1. Flight assist is enabled (`bFlightAssistEnabled = true`)
2. Throttle input is bound correctly in `DefaultInput.ini`
3. Movement component is valid
4. `UpdateThrottleVelocity` is being called in `Tick`

### Boost/Travel Mode Not Working

**Check:**
1. Input bindings are correct
2. Movement component max speed is being updated
3. Boost and travel mode are mutually exclusive (can't use both at once)

## Performance Considerations

The X4-style flight system uses `Tick` for physics calculations. To optimize:

1. **Conditional Ticking:** Disable tick when ship is stationary or unpossessed
2. **Interpolation Steps:** Reduce interpolation steps for distant ships (LOD)
3. **Physics Substeps:** Use fixed timestep for deterministic physics

Example optimization:
```cpp
void ASpaceship::BeginPlay()
{
    Super::BeginPlay();
    
    // Disable tick initially, enable when possessed
    PrimaryActorTick.bCanEverTick = false;
}

void ASpaceship::BeginControl(APlayerController* PC, APawn* ExternalPawn)
{
    Super::BeginControl(PC, ExternalPawn);
    
    // Enable tick when player takes control
    PrimaryActorTick.bCanEverTick = true;
}

void ASpaceship::EndControl(APlayerController* PC)
{
    Super::EndControl(PC);
    
    // Disable tick when player leaves
    PrimaryActorTick.bCanEverTick = false;
}
```

## Future Enhancements

Potential improvements to the flight system:

1. **Inertial Dampening Levels:** Multiple flight assist modes (off, low, medium, high)
2. **Brake Mode:** Dedicated brake button for emergency stops
3. **Roll Control:** Q/E keys for manual roll control
4. **Flight Modes:** Combat mode (agile), cruise mode (stable), precision mode (slow)
5. **Energy Management:** Boost consumes energy/fuel
6. **G-Force Effects:** Camera shake and blur during high-G maneuvers
7. **Thruster Damage:** Directional thrust reduction when thrusters are damaged

## See Also

- `Spaceship.h` - Flight control class definition
- `Spaceship.cpp` - Flight control implementation
- `InputConfigDataAsset.h` - Enhanced Input action definitions
- `DefaultInput.ini` - Legacy input bindings
- `ENHANCED_INPUT_GUIDE.md` - Enhanced Input setup guide

---

**Last Updated:** 2025-11-20  
**Version:** 1.0  
**Status:** Complete  
**Implemented By:** X4-style flight control system
