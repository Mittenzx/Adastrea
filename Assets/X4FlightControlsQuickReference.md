# X4 Flight Controls - Quick Reference

## Default Controls

| Action | Key | Description |
|--------|-----|-------------|
| **Forward** | W | Forward thrust |
| **Backward** | S | Backward thrust |
| **Strafe Left** | A | Move left |
| **Strafe Right** | D | Move right |
| **Up** | Space | Move up |
| **Down** | Left Ctrl | Move down |
| **Rotate** | Mouse | Pitch & Yaw |
| **Throttle Up** | Mouse Wheel Up / = | +10% throttle |
| **Throttle Down** | Mouse Wheel Down / - | -10% throttle |
| **Boost** | Left Shift (hold) | 2x speed boost |
| **Flight Assist** | V | Toggle assist on/off |
| **Travel Mode** | J | Toggle 5x cruise speed |

## Flight Modes

### Flight Assist ON (Default)
- ✅ Smooth rotation with damping
- ✅ Auto-leveling to ecliptic plane
- ✅ Maintains velocity when no input
- ✅ Throttle controls target speed

### Flight Assist OFF
- ⚠️ Pure Newtonian physics
- ⚠️ Ship drifts indefinitely
- ⚠️ No auto-leveling
- ⚠️ More challenging but realistic

## Quick Tuning

### Make Ship More Responsive
```
FlightAssistResponsiveness = 4.0
RotationDampingFactor = 0.7
```

### Make Ship More Stable
```
FlightAssistResponsiveness = 1.5
RotationDampingFactor = 0.95
AutoLevelStrength = 0.7
```

## Common Parameters

| Parameter | Default | What It Does |
|-----------|---------|--------------|
| `RotationDampingFactor` | 0.85 | Higher = smoother rotation |
| `AutoLevelStrength` | 0.5 | How strongly ship levels (0-1) |
| `FlightAssistResponsiveness` | 2.0 | How quickly ship responds to input |
| `ThrottleStep` | 10.0 | Throttle increment per press (%) |
| `BoostMultiplier` | 2.0 | Speed multiplier when boosting |
| `TravelModeMultiplier` | 5.0 | Speed multiplier in travel mode |
| `StrafeIndependence` | 0.8 | Strafe independence from forward motion |

## Blueprint Functions

```cpp
ToggleFlightAssist()          // Toggle flight assist on/off
ThrottleUp()                  // Increase throttle by 10%
ThrottleDown()                // Decrease throttle by 10%
SetThrottle(Percentage)       // Set throttle to specific % (0-100)
ActivateBoost()               // Activate boost mode
DeactivateBoost()             // Deactivate boost mode
ToggleTravelMode()            // Toggle travel mode
GetEffectiveMaxSpeed()        // Get current max speed with modifiers
```

## Tips

1. **For Exploration:** Use throttle mode with flight assist ON
2. **For Combat:** Disable flight assist for maximum agility
3. **For Long Travel:** Activate travel mode (J key)
4. **For Docking:** Use low throttle (10-20%) with strafe controls
5. **For Evasion:** Hold Left Shift for boost while maneuvering

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Ship too sluggish | Increase `FlightAssistResponsiveness` to 3-5 |
| Ship too twitchy | Increase `RotationDampingFactor` to 0.9-1.0 |
| Won't stop rolling | Increase `AutoLevelStrength` to 0.7-0.9 |
| Throttle not working | Check flight assist is enabled |
| Boost not working | Release and re-press Left Shift |

---

**See Full Guide:** `X4FlightControlsGuide.md`
