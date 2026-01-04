# Mouse Sensitivity Quick Reference

## Visual Guide to Mouse Sensitivity Settings

### Before Fix (January 2026)

```
Mouse Movement:
┌─────────────────────────────────┐
│  Horizontal: ◄════════════════► │  Fast, responsive
│                                 │
│  Vertical:   ▲══════▼          │  Slow, unresponsive
└─────────────────────────────────┘

Problem: Vertical sensitivity = Horizontal sensitivity (1.0x)
Result: Vertical movement felt sluggish
```

### After Fix (January 2026)

```
Mouse Movement:
┌─────────────────────────────────┐
│  Horizontal: ◄════════════════► │  Fast, responsive
│                                 │
│  Vertical:   ▲══════════════▼  │  Fast, responsive
└─────────────────────────────────┘

Solution: Vertical sensitivity = 2.0x Horizontal
Result: Balanced, playable controls
```

## Property Overview

| Property | Controls | Default | Min | Max |
|----------|----------|---------|-----|-----|
| **LookSensitivity** | Horizontal (Yaw) | 1.0 | 0.1 | 10.0 |
| **LookSensitivityVertical** | Vertical (Pitch) | 2.0 | 0.0 | 10.0 |

**Special Case**: If `LookSensitivityVertical = 0`, uses `LookSensitivity` value

## Common Presets

### 🎮 Default (Balanced)
```
LookSensitivity:         1.0
LookSensitivityVertical: 2.0
Result: Good for most players
```

### ⚡ High Sensitivity (Combat)
```
LookSensitivity:         2.0
LookSensitivityVertical: 3.0
Result: Fast response, good for dogfighting
```

### 🎯 Low Sensitivity (Precision)
```
LookSensitivity:         0.5
LookSensitivityVertical: 1.0
Result: Precise aiming, good for trading/docking
```

### ✈️ Flight Sim Style
```
LookSensitivity:         1.0
LookSensitivityVertical: 2.0
bInvertLookY:            true
Result: Inverted pitch like a real aircraft
```

## How to Adjust in Editor

```
1. Open spaceship Blueprint (e.g., BP_PlayerShip)
2. Select SpaceshipControlsComponent
3. Find "Controls|Look" category in Details panel
4. Adjust LookSensitivity and LookSensitivityVertical
5. Save, Compile, and Test in PIE
```

## Visual Sensitivity Comparison

```
Sensitivity Value | Mouse Movement | Ship Rotation
─────────────────────────────────────────────────
    0.5           |  ──────►      |  ─►      (Slow)
    1.0           |  ──────►      |  ───►    (Normal)
    2.0           |  ──────►      |  ──────► (Fast)
    3.0           |  ──────►      |  ─────────► (Very Fast)
```

## What Changed?

**Old Behavior** (Before January 2026):
```cpp
LookValue.X *= LookSensitivity;  // Horizontal
LookValue.Y *= LookSensitivity;  // Vertical - SAME as horizontal
```

**New Behavior** (After January 2026):
```cpp
LookValue.X *= LookSensitivity;           // Horizontal
LookValue.Y *= LookSensitivityVertical;   // Vertical - SEPARATE value
```

## Testing Checklist

- [ ] Horizontal movement feels responsive
- [ ] Vertical movement feels responsive
- [ ] Both axes feel balanced (not one too fast/slow)
- [ ] Can aim at targets comfortably
- [ ] Can perform precise maneuvers (e.g., docking)
- [ ] No motion sickness or discomfort

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Vertical too slow | Increase `LookSensitivityVertical` (try 2.5 or 3.0) |
| Horizontal too slow | Increase `LookSensitivity` (try 1.5 or 2.0) |
| Everything too fast | Reduce both values by 50% |
| Y-axis feels wrong | Toggle `bInvertLookY` |
| Settings don't work | Ensure `LookSensitivityVertical` is not 0 |

## Related Files

- Implementation: `Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp`
- Header: `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h`
- Detailed Guide: `docs/setup/MOUSE_SENSITIVITY_GUIDE.md`

---

**Quick Tip**: Start with defaults (1.0 / 2.0) and adjust in 0.5 increments until comfortable.
