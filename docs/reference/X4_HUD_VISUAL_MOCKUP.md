# X4-Style HUD Visual Mockup Guide

## Overview

This document provides visual mockups and ASCII art representations of the X4-style flight HUD crosshair system to help designers implement the visual elements in Blueprint.

---

## Full HUD Layout

```
┌──────────────────────────────────────────────────────────────────┐
│                                                                  │
│  [Ship Info]                                        [Status]     │
│  Pathfinder                                         Hull: ████   │
│  Scout Class                                        Shield: ██░░ │
│                                                     Speed: 2500  │
│                                                                  │
│                                                                  │
│                                                                  │
│                          ╔═══════╗                               │
│                          ║   ░   ║  ← Deadzone Circle           │
│                          ║  ┼ ↗  ║     (subtle, optional)       │
│                          ║   ║   ║                              │
│                          ╚═══║═══╝                               │
│                              ║                                   │
│                              ║  ← Connection Line                │
│                              ║                                   │
│                              ◈  ← Mouse Cursor (white)          │
│                                                                  │
│                                                                  │
│                                                                  │
│  [Rotation Speed]                                   [Throttle]   │
│  ████████░░  80%                                    ██████░░░░   │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘

Legend:
  ┼  = Screen center (ship position)
  ↗  = Ship rotation indicator (cyan, shows turn direction)
  ◈  = Mouse cursor indicator (white, follows mouse)
  ║  = Connection line (gradient white→cyan)
  ╔═══╗ = Deadzone circle (optional visualization)
```

---

## Component Breakdown

### 1. Mouse Cursor Indicator

**Normal State (Outside Deadzone):**
```
    ◈
```
- Size: 28-32 pixels
- Color: White (255, 255, 255)
- Style: Crosshair or plus sign
- Opacity: 100%

**In Deadzone:**
```
    ⊕
```
- Size: 28-32 pixels
- Color: Light Gray (180, 180, 180)
- Style: Circle with dot (showing "neutral")
- Opacity: 60%

**High Rotation Speed:**
```
    ✦
```
- Size: 32-36 pixels (slightly larger)
- Color: Yellow-Orange (255, 200, 100)
- Style: Star burst or emphasized crosshair
- Opacity: 100%

---

### 2. Ship Rotation Direction Indicator

**Normal State:**
```
    ⟰
```
- Size: 40-48 pixels
- Color: Cyan (100, 200, 255)
- Style: Arrow or chevron pointing in rotation direction
- Opacity: 80%

**Aligned with Mouse (No Rotation Needed):**
```
    ◎
```
- Size: 40-48 pixels
- Color: Green (100, 255, 100)
- Style: Circle (target locked)
- Opacity: 80%

**High Speed Rotation:**
```
    ⟱⃗
```
- Size: 48-54 pixels (larger)
- Color: Red-Orange (255, 100, 100)
- Style: Motion blur effect or multiple arrows
- Opacity: 100%

---

### 3. Connection Line

Shows visual link between mouse cursor and ship direction.

**Short Distance (Mouse close to center):**
```
    ↗  ← Ship
    ║
    ║
    ◈  ← Mouse
```
- Thickness: 2 pixels
- Color: Solid cyan
- Style: Simple line

**Medium Distance:**
```
    ↗  ← Ship
    ║
    ║
    ║
    ║
    ◈  ← Mouse
```
- Thickness: 2-3 pixels
- Color: Gradient (cyan → white)
- Style: Solid or subtle gradient

**Long Distance (High rotation needed):**
```
    ↗  ← Ship
    ║
    ╏
    ╏
    ╏
    ◈  ← Mouse
```
- Thickness: 3 pixels
- Color: Strong gradient with glow
- Style: Dashed or pulsing

---

### 4. Deadzone Circle Visualization

**When Enabled (bShowDeadzoneCircle = true):**

```
                    Screen Center
                         │
              ┌──────────┼──────────┐
              │          │          │
              │    ╔═════╪═════╗    │
              │    ║     │     ║    │
        ──────┼────║─────┼─────║────┼──────
              │    ║     │     ║    │
              │    ╚═════╪═════╝    │
              │          │          │
              └──────────┼──────────┘
                         │
                    Inner: Deadzone
                    Outer: Max Effective Radius
```

**Visual Properties:**
- Inner Circle (Deadzone): 
  - Diameter: 100-200 pixels (default 50px radius × 2)
  - Color: White with 30-40 alpha
  - Style: Dashed or dotted line
  
- Outer Circle (Max Radius - Optional):
  - Diameter: 800 pixels (default 400px radius × 2)
  - Color: White with 15-20 alpha
  - Style: Very subtle dashed line

---

## State Diagrams

### State 1: Mouse in Deadzone (No Rotation)

```
┌──────────────────────────────────────┐
│                                      │
│                                      │
│            ╔═══════╗                 │
│            ║   ⊕   ║  ← Mouse (gray) │
│            ║   ┼   ║  ← Ship (center)│
│            ╚═══════╝                 │
│                                      │
│  Status: IN DEADZONE                 │
│  Rotation: 0%                        │
│                                      │
└──────────────────────────────────────┘
```
- Mouse and ship indicators overlapped at center
- Gray color indicates no input
- No connection line needed

---

### State 2: Mouse Outside Deadzone (Slow Rotation)

```
┌──────────────────────────────────────┐
│                                      │
│                                      │
│            ╔═══════╗                 │
│            ║   ↗   ║  ← Ship (cyan)  │
│            ║   ┼   ║                 │
│            ╚═══║═══╝                 │
│                ║                     │
│                ║  ← Line (subtle)    │
│                ◈  ← Mouse (white)    │
│                                      │
│  Status: ROTATING                    │
│  Rotation: 25%                       │
│                                      │
└──────────────────────────────────────┘
```
- Ship indicator shows rotation direction
- Thin connection line
- White mouse cursor

---

### State 3: Mouse at Edge (High Speed Rotation)

```
┌──────────────────────────────────────┐
│                                      │
│            ╔═══════╗                 │
│            ║   ⟱⃗  ║  ← Ship (red)   │
│            ║   ┼   ║                 │
│            ╚═══╏═══╝                 │
│                ╏                     │
│                ╏  ← Line (bright)    │
│                ╏                     │
│                ╏                     │
│                ╏                     │
│                ✦  ← Mouse (yellow)   │
│                                      │
│  Status: FAST ROTATION               │
│  Rotation: 95%                       │
│                                      │
└──────────────────────────────────────┘
```
- Bright, thick connection line
- Yellow mouse cursor (high speed)
- Red/orange ship indicator (high speed)
- Larger indicator sizes

---

## Color Transitions

### Rotation Speed → Color Mapping

```
Rotation Speed: 0% ────────────────────────────────> 100%
                │                                     │
Mouse Color:    Light Gray → White → Yellow → Orange
                (180,180,180) (255,255,255) (255,200,100)
                
Ship Color:     Center → Cyan → Orange → Red
                (100,200,255) (255,150,100) (255,100,100)

Line Color:     None → Subtle → Normal → Bright
                (invisible) (50% alpha) (100% alpha) (glow)
```

### Visual Feedback Curve

```
Rotation Speed
    ^
100%│                                    ████  High (Red)
    │                            ████████
 75%│                    ████████           Medium (Orange)
    │            ████████
 50%│    ████████                           Low (Cyan)
    │████
  0%├────────────────────────────────────>
    Center  Deadzone  Near    Mid    Far   Distance from Center
            Edge      Edge    Range  Edge
```

---

## Layout Examples for Different Screen Sizes

### 1920x1080 (Full HD)

```
Deadzone Radius: 50 pixels
Max Radius: 400 pixels
Cursor Size: 32 pixels
Ship Indicator: 48 pixels

┌─────────────────────── 1920px ────────────────────────┐
│                                                       │
│                                                       │ 1080px
│                     ╔══════╗                          │
│                     ║  ↗   ║                          │
│                     ║  ┼   ║                          │
│                     ╚══╬═══╝                          │
│                        ║                              │
│                        ◈                              │
│                                                       │
└───────────────────────────────────────────────────────┘
```

### 2560x1440 (2K)

```
Deadzone Radius: 67 pixels (scaled)
Max Radius: 533 pixels (scaled)
Cursor Size: 42 pixels (scaled)
Ship Indicator: 64 pixels (scaled)

┌──────────────────────── 2560px ───────────────────────┐
│                                                       │
│                                                       │ 1440px
│                      ╔════════╗                       │
│                      ║   ↗    ║                       │
│                      ║   ┼    ║                       │
│                      ╚═══╬════╝                       │
│                          ║                            │
│                          ◈                            │
│                                                       │
└───────────────────────────────────────────────────────┘
```

### 3840x2160 (4K)

```
Deadzone Radius: 100 pixels (scaled)
Max Radius: 800 pixels (scaled)
Cursor Size: 64 pixels (scaled)
Ship Indicator: 96 pixels (scaled)

┌──────────────────────── 3840px ───────────────────────┐
│                                                       │
│                                                       │
│                                                       │ 2160px
│                       ╔═════════╗                     │
│                       ║    ↗    ║                     │
│                       ║    ┼    ║                     │
│                       ╚════╬════╝                     │
│                            ║                          │
│                            ◈                          │
│                                                       │
│                                                       │
└───────────────────────────────────────────────────────┘
```

---

## Animation Sequences

### Entering Deadzone Animation

```
Frame 1:  ↗ ║ ◈   (Rotating, outside deadzone)
          
Frame 2:  ↗ ║ ◈   (Approaching center)
           ╚╝
           
Frame 3:  ↗ ⊕     (Mouse entering deadzone)
          ┼       (Ship centering)
          
Frame 4:  ⊕┼      (Overlapped, gray, no rotation)
```
Duration: 0.2 seconds with ease-out curve

---

### Exiting Deadzone Animation

```
Frame 1:  ⊕┼      (In deadzone, overlapped)

Frame 2:  ↗       (Ship indicator appears)
          ┼║
           ◈      (Mouse moving out)
           
Frame 3:  ↗       (Indicators separating)
          ┼║
           ║◈
           
Frame 4:  ↗       (Full separation, active)
          ┼║
           ║
           ║◈
```
Duration: 0.2 seconds with ease-out curve

---

### High Speed Pulse Animation

When rotation speed > 80%, add pulsing effect:

```
Beat 1 (0.0s):  ✦ (large, bright)
Beat 2 (0.1s):  ◈ (normal)
Beat 3 (0.2s):  ✦ (large, bright)
Beat 4 (0.3s):  ◈ (normal)
```
Pulse frequency: 3-4 times per second
Scale range: 1.0x to 1.2x

---

## Widget Hierarchy in Blueprint

Recommended structure for Canvas Panel:

```
Canvas Panel (Root)
├── Background Elements
│   └── Deadzone Circle (Image)
│       - Anchors: Center
│       - Z-Order: -10
│
├── Connection Line (Custom Drawing or Multiple Images)
│   - Z-Order: 0
│
├── Ship Direction Indicator (Image)
│   - Anchors: Center
│   - Z-Order: 5
│
├── Mouse Cursor Indicator (Image)
│   - Anchors: Center
│   - Z-Order: 10
│
└── Debug Info (Text Block - Optional)
    - Rotation Speed: XX%
    - Mouse Position: (X, Y)
    - In Deadzone: Yes/No
```

---

## Asset Requirements

### Texture Assets Needed

1. **Mouse Cursor Indicators:**
   - `T_Crosshair_Normal.png` (32x32) - White crosshair
   - `T_Crosshair_Deadzone.png` (32x32) - Circle with dot
   - `T_Crosshair_HighSpeed.png` (36x36) - Star burst

2. **Ship Direction Indicators:**
   - `T_ShipDirection_Arrow.png` (48x48) - Cyan arrow/chevron
   - `T_ShipDirection_Locked.png` (48x48) - Green circle
   - `T_ShipDirection_Fast.png` (54x54) - Red motion blur

3. **Deadzone Circle:**
   - `T_Circle_Dashed.png` (512x512) - Transparent with dashed outline
   - `T_Circle_Solid.png` (512x512) - Transparent with solid outline

4. **Connection Line:**
   - `T_Line_Gradient.png` (4x64) - Vertical gradient strip
   - Or use custom drawing in Blueprint Paint event

### Material Setup

**Mouse Cursor Material:**
```
Base Color: Parameter (controlled by Blueprint)
Opacity: 1.0
Blend Mode: Translucent
```

**Ship Direction Material:**
```
Base Color: Parameter (Cyan default)
Opacity: 0.8
Blend Mode: Translucent
Emissive: Multiply Base Color × 1.5 (glow effect)
```

**Deadzone Circle Material:**
```
Base Color: White
Opacity: Parameter (0.3 default)
Blend Mode: Translucent
```

---

## Implementation Checklist

- [ ] Create all texture assets (crosshairs, arrows, circles)
- [ ] Create materials for each indicator type
- [ ] Set up Canvas Panel hierarchy in Widget Blueprint
- [ ] Implement `UpdateFlightCrosshair` event
  - [ ] Convert screen positions to pixel coordinates
  - [ ] Update widget positions with SetRenderTranslation
  - [ ] Update colors based on rotation speed
  - [ ] Handle deadzone visual state
- [ ] Implement `UpdateDeadzoneVisualization` event
  - [ ] Resize deadzone circle
  - [ ] Update opacity based on state
- [ ] Add smooth interpolation for position changes
- [ ] Add color transition animations
- [ ] Test at different screen resolutions
- [ ] Add debug text display (optional)
- [ ] Optimize for 60 FPS performance

---

## Testing Visual Mockup

### Test Scenario 1: Mouse Movement
```
Start: Mouse at center
Action: Move mouse right slowly
Expected:
  Frame 1: ⊕┼         (gray, center)
  Frame 2: ↗┼ ║       (separating)
          ◈
  Frame 3: ↗┼  ║      (further apart)
           ◈
  Frame 4: ↗┼   ║     (max separation)
            ◈
```

### Test Scenario 2: Rapid Return to Center
```
Start: Mouse at edge (fast rotation)
Action: Snap mouse back to center
Expected:
  Frame 1: ⟱⃗┼    ║    (red, high speed)
              ✦
  Frame 2: ↗┼   ║     (slowing down, color fading)
             ◈
  Frame 3: ↗┼ ║       (almost centered)
           ◈
  Frame 4: ⊕┼         (deadzone, gray)
```

---

## Comparison with X4: Foundations

### X4 Original:
- Mouse cursor is hidden (replaced by flight cursor)
- Flight cursor shows rotation direction
- Subtle trail effect
- No explicit deadzone visualization
- Color: Cyan/blue theme

### Adastrea Implementation:
- Mouse cursor indicator (visible)
- Ship direction indicator (separate)
- Connection line (shows relationship)
- Optional deadzone circle
- Customizable colors
- More explicit feedback for learning

**Why Different?**
Our implementation is more educational and explicit, helping players understand the flight model. Once comfortable, players can hide extra indicators and get closer to X4's minimal style.

---

## Future Enhancements

### Rotation Prediction Arc (Advanced)
```
┌──────────────────────────────────────┐
│                                      │
│            ╔═══════╗                 │
│            ║   ⟰═══╗  ← Predicted   │
│            ║   ┼   ║    rotation    │
│            ╚═══║═══╝    arc         │
│                ║                     │
│                ◈                     │
│                                      │
└──────────────────────────────────────┘
```
Shows where ship will be pointing 0.5 seconds from now.

### Target Lead Indicator (Combat)
```
┌──────────────────────────────────────┐
│                ⊗  ← Target           │
│                ║                     │
│                ⊕  ← Lead point       │
│                                      │
│            ╔═══════╗                 │
│            ║   ↗   ║                 │
│            ║   ┼   ║                 │
│            ╚═══════╝                 │
│                                      │
└──────────────────────────────────────┘
```
Shows where to aim to hit moving target.

---

## See Also

- `X4_HUD_CROSSHAIR_SYSTEM.md` - Complete technical documentation
- `X4FlightControlsGuide.md` - Flight controls documentation
- `HUDSystemGuide.md` - General HUD system
- `Spaceship.h` - C++ implementation

---

**Last Updated**: January 13, 2026  
**Version**: 1.0  
**Author**: Adastrea Development Team
