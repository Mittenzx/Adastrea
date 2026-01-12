# X4-Style Mouse Flight Controls - Visual Guide

This guide provides visual diagrams to understand the X4-style mouse position flight system.

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Screen Space                             │
│                                                              │
│                                                              │
│          ┌──────────────────────────┐                       │
│          │   Deadzone (50px)        │                       │
│          │      ┌───────┐           │                       │
│          │      │   +   │ ← Center  │                       │
│          │      └───────┘           │                       │
│          │     No Rotation          │                       │
│          └──────────────────────────┘                       │
│                                                              │
│     ┌────────────────────────────────────┐                  │
│     │  Scaling Zone (50-400px)           │                  │
│     │  Rotation speed 0% → 100%          │                  │
│     └────────────────────────────────────┘                  │
│                                                              │
│  ┌──────────────────────────────────────────────┐           │
│  │  Max Radius Zone (400px+)                    │           │
│  │  100% Rotation Speed                         │           │
│  └──────────────────────────────────────────────┘           │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

## Mouse Position to Rotation Speed

```
Rotation Speed
    100% │                    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
         │                ▓▓▓▓
         │            ▓▓▓▓
     50% │        ▓▓▓▓
         │    ▓▓▓▓
         │▓▓▓▓
      0% └────┴────┴────┴────┴────┴────┴────┴────┴───
         0   50  100 150 200 250 300 350 400+
              Distance from Center (pixels)
              
         │◄──►│           Deadzone (no rotation)
              │◄───────────►│ Scaling zone (0-100%)
                            │◄─────────►│ Max speed zone
```

## Ship Rotation Rate Multiplier Effect

```
Fighter (1.0x)        │████████████████████│ 100% rotation
Scout (0.8x)          │████████████████    │  80% rotation
Medium (0.5x)         │██████████          │  50% rotation
Freighter (0.3x)      │██████              │  30% rotation
Capital Ship (0.2x)   │████                │  20% rotation
                      └────────────────────┘
                         Same Mouse Distance
```

## Rotation Direction

```
Screen Quadrants:

    Northwest │ North │ Northeast
    Pitch Up  │       │ Pitch Up
    Yaw Left  │       │ Yaw Right
    ──────────┼───────┼──────────
    West      │   +   │   East
    Yaw Left  │       │ Yaw Right
    ──────────┼───────┼──────────
    Southwest │ South │ Southeast
    Pitch Down│       │ Pitch Down
    Yaw Left  │       │ Yaw Right


Mouse Position → Rotation:
  • Mouse right of center → Ship yaws right
  • Mouse left of center → Ship yaws left
  • Mouse above center → Ship pitches up
  • Mouse below center → Ship pitches down
  
Combined movements create diagonal rotation.
```

## Configuration Examples

### Example 1: Agile Fighter

```
Ship DataAsset:
  RotationRateMultiplier: 0.9 (very agile)
  
Spaceship Settings:
  bUseMousePositionFlight: true
  MouseDeadzoneRadius: 40.0 (smaller deadzone)
  MouseMaxRadius: 350.0 (reaches max speed sooner)
  
Result: Very responsive, quick turns
```

### Example 2: Heavy Freighter

```
Ship DataAsset:
  RotationRateMultiplier: 0.3 (slow)
  
Spaceship Settings:
  bUseMousePositionFlight: true
  MouseDeadzoneRadius: 75.0 (larger deadzone for stability)
  MouseMaxRadius: 500.0 (reaches max speed later)
  
Result: Stable, slow, deliberate turns
```

## Input Flow Diagram

```
┌──────────────┐
│ Mouse Input  │
│ (Position)   │
└──────┬───────┘
       │
       ▼
┌──────────────────────────┐
│ Get Mouse Position       │
│ Get Viewport Size        │
│ Calculate Center         │
└──────┬───────────────────┘
       │
       ▼
┌──────────────────────────┐
│ Calculate Distance       │
│ from Center              │
└──────┬───────────────────┘
       │
       ▼
┌──────────────────────────┐
│ Check Deadzone           │
│ (< 50px?)                │
└──────┬───────────────────┘
       │
   Yes │ No
       │
┌──────▼──────┐    ┌──────────────────────┐
│ Return      │    │ Calculate Distance   │
│ No Rotation │    │ Ratio (0-1)          │
└─────────────┘    └──────┬───────────────┘
                          │
                          ▼
                   ┌──────────────────────┐
                   │ Calculate Rotation   │
                   │ Rate =               │
                   │   Direction/Distance │
                   │   × DistanceRatio    │
                   │   × TurnRate         │
                   │   × ShipMultiplier   │
                   │   × Sensitivity      │
                   └──────┬───────────────┘
                          │
                          ▼
                   ┌──────────────────────┐
                   │ Interpolate Velocity │
                   │ (Smooth Damping)     │
                   └──────┬───────────────┘
                          │
                          ▼
                   ┌──────────────────────┐
                   │ Apply Rotation       │
                   │ to Ship              │
                   └──────────────────────┘
```

## Comparison: Mouse Delta vs Mouse Position

### Mouse Delta Mode (Traditional)
```
Mouse Movement → Constant Rotation Rate
┌─────┐
│ →   │ Small mouse movement  → Slow rotation
└─────┘

┌─────┐
│ →→→ │ Large mouse movement  → Fast rotation
└─────┘

Problem: No intuitive control, hard to predict
```

### Mouse Position Mode (X4-Style)
```
Mouse Position → Distance-Based Rotation
┌────────────────┐
│      +→        │ Near center → Slow rotation
└────────────────┘

┌────────────────┐
│      +     →   │ Far from center → Fast rotation
└────────────────┘

Benefit: Predictable, intuitive, natural feel
```

## Performance Characteristics

### CPU Cost per Frame

```
Low Cost:  Mouse delta mode (traditional)
           └─ 1-2 floating point operations

Medium Cost: Mouse position mode
             └─ Viewport size query
             └─ Mouse position query
             └─ Distance calculation (sqrt)
             └─ Ratio calculation
             └─ Interpolation
             Total: ~10-15 operations per frame

Note: Negligible impact on modern hardware
      Tested: <0.1ms on mid-range CPU
```

## Tuning Workflow

```
1. Set Ship Role
   ↓
2. Choose Base RotationRateMultiplier
   • Fighter: 0.8-1.0
   • Medium: 0.5-0.7
   • Heavy:  0.2-0.4
   ↓
3. Test in Editor
   ↓
4. Adjust Based on Feel
   ← Too slow?     → Increase multiplier
   ← Too fast?     → Decrease multiplier
   ← Too twitchy?  → Increase deadzone
   ← Not responsive? → Decrease max radius
   ↓
5. Fine-tune Global Settings
   • FlightAssistResponsiveness
   • RotationDampingFactor
   ↓
6. Validate with Different Players
```

## Common Scenarios

### Scenario 1: Precision Docking

```
Problem: Need fine control near docking port

Solution:
  MouseDeadzoneRadius: 75-100px (larger)
  MouseMaxRadius: 500px (slower scaling)
  
Result: Very stable, precise movements near center
```

### Scenario 2: Combat Dogfighting

```
Problem: Need quick response for target tracking

Solution:
  MouseDeadzoneRadius: 30-40px (smaller)
  MouseMaxRadius: 300px (faster scaling)
  RotationRateMultiplier: 0.8-1.0
  
Result: Highly responsive, agile combat
```

### Scenario 3: Exploration/Trading

```
Problem: Need comfortable long-term flying

Solution:
  MouseDeadzoneRadius: 50px (default)
  MouseMaxRadius: 400px (default)
  RotationRateMultiplier: 0.5
  
Result: Balanced, comfortable for extended play
```

## Troubleshooting Visual Guide

### Problem: Ship won't rotate

```
Check:
1. Mouse position
   ┌────────────────┐
   │      +         │ ← Inside deadzone?
   │       ×        │    Move mouse further out
   └────────────────┘

2. bUseMousePositionFlight = true?
3. Flight assist enabled?
4. Ship has valid DataAsset?
```

### Problem: Rotation too sensitive

```
Before:
┌────────────────┐
│      +→        │ Small movement = Fast rotation
└────────────────┘

After:
┌────────────────┐
│      +   →     │ Increase MouseMaxRadius
└────────────────┘  or decrease Sensitivity
```

### Problem: Different ships feel identical

```
Check DataAssets:
Ship A: RotationRateMultiplier = 0.5  ✓
Ship B: RotationRateMultiplier = 0.5  ✗ (should differ)

Fix:
Ship A (Fighter):  RotationRateMultiplier = 0.9  ✓
Ship B (Freighter): RotationRateMultiplier = 0.3  ✓
```

---

## Quick Reference Card

```
╔═══════════════════════════════════════════════════════╗
║        X4-STYLE MOUSE FLIGHT - QUICK REFERENCE        ║
╠═══════════════════════════════════════════════════════╣
║ Controls:                                             ║
║   • Move mouse from center = Ship rotates             ║
║   • Further from center = Faster rotation             ║
║   • Near center (50px) = No rotation                  ║
║                                                       ║
║ Ship Tuning (in DataAsset):                          ║
║   Fighter:       RotationRateMultiplier = 0.9        ║
║   Medium:        RotationRateMultiplier = 0.5        ║
║   Heavy:         RotationRateMultiplier = 0.3        ║
║                                                       ║
║ Global Tuning (in Spaceship):                        ║
║   Deadzone:      MouseDeadzoneRadius = 50px          ║
║   Max Radius:    MouseMaxRadius = 400px              ║
║   Mode Toggle:   bUseMousePositionFlight = true      ║
║                                                       ║
║ Troubleshooting:                                      ║
║   Too sensitive?    → Increase MouseMaxRadius        ║
║   Not sensitive?    → Decrease MouseMaxRadius        ║
║   Won't rotate?     → Move mouse outside deadzone    ║
║   Ships same feel?  → Set different multipliers      ║
╚═══════════════════════════════════════════════════════╝
```

---

**See Also:**
- `Assets/X4FlightControlsGuide.md` - Complete implementation guide
- `docs/development/X4_MOUSE_FLIGHT_IMPLEMENTATION.md` - Technical details
