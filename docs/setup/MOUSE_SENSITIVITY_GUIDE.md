# Mouse Sensitivity Configuration Guide

## Overview

This guide explains how to configure mouse sensitivity for spaceship controls in Adastrea. As of January 2026, separate horizontal and vertical sensitivity settings are available to provide better control feel.

## Quick Reference

| Property | Default | Range | Purpose |
|----------|---------|-------|---------|
| `LookSensitivity` | 1.0 | 0.1 - 10.0 | Horizontal mouse movement (yaw/left-right) |
| `LookSensitivityVertical` | 2.0 | 0.0 - 10.0 | Vertical mouse movement (pitch/up-down) |
| `bInvertLookY` | false | true/false | Invert vertical mouse axis |

## Why Separate Sensitivities?

Players reported that vertical mouse movement felt significantly less responsive than horizontal movement. The separate `LookSensitivityVertical` property allows fine-tuning of pitch sensitivity independently from yaw sensitivity.

**Default Configuration:**
- Horizontal sensitivity: 1.0x
- Vertical sensitivity: 2.0x (2x more sensitive than horizontal)

This provides a more responsive feel for vertical mouse movement while maintaining comfortable horizontal control.

## Configuration in Blueprint Editor

### For Ship Blueprints (BP_PlayerShip, etc.)

1. Open your spaceship Blueprint (e.g., `BP_PlayerShip`)
2. Select the **SpaceshipControlsComponent** in the Components panel
3. In the Details panel, locate the **Controls|Look** category
4. Adjust the following properties:

#### LookSensitivity (Horizontal)
- **Purpose**: Controls how fast the ship turns left/right when moving mouse horizontally
- **Recommended Range**: 0.5 - 2.0
- **Lower values**: Slower, more precise control
- **Higher values**: Faster, more responsive control

#### LookSensitivityVertical (Pitch)
- **Purpose**: Controls how fast the ship pitches up/down when moving mouse vertically
- **Recommended Range**: 1.0 - 3.0
- **Special**: If set to 0, falls back to using `LookSensitivity` value
- **Lower values**: Slower vertical movement
- **Higher values**: Faster vertical movement

#### bInvertLookY
- **Purpose**: Reverses vertical mouse axis (some players prefer inverted controls)
- **Checked**: Moving mouse up pitches down (like flight simulator)
- **Unchecked**: Moving mouse up pitches up (standard FPS)

### Example Configurations

#### Default (Balanced)
```
LookSensitivity: 1.0
LookSensitivityVertical: 2.0
bInvertLookY: false
```

#### High Sensitivity (Fast Response)
```
LookSensitivity: 2.0
LookSensitivityVertical: 3.0
bInvertLookY: false
```

#### Low Sensitivity (Precise Control)
```
LookSensitivity: 0.5
LookSensitivityVertical: 1.0
bInvertLookY: false
```

#### Flight Sim Style (Inverted Y)
```
LookSensitivity: 1.0
LookSensitivityVertical: 2.0
bInvertLookY: true
```

## Testing Your Changes

1. **Save your Blueprint** after adjusting settings
2. **Compile the Blueprint** (click the Compile button)
3. **Launch PIE** (Play In Editor) by pressing the Play button
4. **Test mouse movement** in all directions:
   - Move mouse left/right - check horizontal responsiveness
   - Move mouse up/down - check vertical responsiveness
   - Try combat maneuvers or precision movements
5. **Adjust as needed** and repeat

### What to Look For

- **Too Sensitive**: Ship moves too quickly, hard to aim precisely
- **Too Slow**: Ship feels sluggish, takes too long to respond
- **Unbalanced**: One axis feels significantly different from the other
- **Perfect**: Comfortable, responsive, easy to control

## Per-Player Settings (Future)

**Note**: Currently these settings are per-ship in Blueprints. In a future update, we plan to add per-player settings that can be adjusted in-game through a settings menu.

## Technical Details

### How It Works

When the player moves the mouse, the input system:

1. Captures raw mouse movement as `FVector2D` (X = horizontal, Y = vertical)
2. Applies horizontal sensitivity: `X *= LookSensitivity`
3. Applies vertical sensitivity: `Y *= LookSensitivityVertical` (or `LookSensitivity` if vertical is 0)
4. Applies Y inversion if `bInvertLookY` is true
5. Sends processed input to ship rotation logic

### Source Code

The sensitivity system is implemented in `SpaceshipControlsComponent`:

**Header**: `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h`
- Property declarations
- Setter functions

**Implementation**: `Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp`
- `HandleLook()` function applies sensitivity
- Default values set in constructor

## Troubleshooting

### Vertical movement still feels wrong
- Try increasing `LookSensitivityVertical` to 2.5 or 3.0
- Ensure `LookSensitivityVertical` is not 0 (which would use horizontal value)
- Check if `bInvertLookY` is set correctly for your preference

### Can't aim precisely
- Reduce both sensitivity values
- Consider using different sensitivities for different ship types (fighters = higher, freighters = lower)

### Mouse feels inconsistent
- Make sure both values are set (not leaving vertical at 0 accidentally)
- Check that frame rate is stable (sensitivity is frame-rate independent)
- Verify no other input modifiers are conflicting

### Settings don't apply
1. Ensure you saved the Blueprint after changes
2. Compile the Blueprint
3. Restart PIE (stop and start again)
4. Check you're modifying the correct Blueprint instance

## Related Documentation

- **Enhanced Input Guide**: `docs/development/ENHANCED_INPUT_GUIDE.md`
- **Player Controls Reference**: `docs/reference/INPUT_CONTROLS_REFERENCE.md`
- **Spaceship Controls Component**: See header comments in `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h`

## Feedback

If you have suggestions for default sensitivity values or additional control options, please open an issue on GitHub with:
- Your preferred sensitivity settings
- Ship type you're flying
- Control scheme (mouse/keyboard, gamepad)
- What feels good or bad about the current defaults

---

**Last Updated**: 2026-01-04  
**Version**: 1.0  
**Related Issue**: Mouse controls vertical sensitivity too low
