# X4-Style HUD Crosshair System

## Overview

The X4-Style HUD Crosshair System provides visual feedback for the player's flight controls, showing exactly where the mouse cursor is, where the ship is turning, and the relationship between player input and ship response.

This document explains how to implement and customize the HUD crosshair in Blueprint.

---

## Key Concepts

### 1. Mouse Position Indicator
- **What it shows**: Where the player's mouse cursor is on screen
- **Purpose**: Primary input indication - "I want to turn here"
- **Visual**: Typically a crosshair, reticle, or dot following the mouse

### 2. Ship Rotation Direction Indicator
- **What it shows**: The direction the ship is actually rotating towards
- **Purpose**: Shows ship response to player input
- **Visual**: Arrow, chevron, or secondary indicator showing actual turn direction

### 3. Connection Line/Arrow
- **What it shows**: Visual link between mouse position and ship direction
- **Purpose**: Shows the rotation relationship and intent
- **Visual**: Line, arrow, or gradient connecting the two indicators

### 4. Deadzone Visualization
- **What it shows**: The circular area around screen center where no rotation occurs
- **Purpose**: Helps players understand the "neutral zone"
- **Visual**: Subtle circle at center with configurable radius

### 5. Rotation Speed Indicator
- **What it shows**: Current rotation speed (0-100%)
- **Purpose**: Visual feedback for how fast the ship is turning
- **Visual**: Color intensity, size variation, or progress bar

---

## Implementation Guide

### Step 1: Create HUD Widget Blueprint

1. Create a new Widget Blueprint based on `UAdastreaHUDWidget`
2. Name it `WBP_FlightHUD` or similar

### Step 2: Add Visual Elements

Add these widget components to your Canvas:

#### A. Mouse Cursor Indicator
```
Component: Image or Custom Widget
Name: MouseCursorIndicator
Properties:
  - Anchors: Center (0.5, 0.5)
  - Size: 32x32 pixels
  - Color: White (RGB 255, 255, 255, Alpha 255)
  - Visibility: Visible when bFlightCrosshairVisible = true
```

#### B. Ship Direction Indicator
```
Component: Image or Custom Widget
Name: ShipDirectionIndicator
Properties:
  - Anchors: Center (0.5, 0.5)
  - Size: 48x48 pixels (slightly larger than cursor)
  - Color: Cyan or Blue (RGB 100, 200, 255, Alpha 200)
  - Visibility: Visible when bFlightCrosshairVisible = true
```

#### C. Connection Line
```
Component: Custom Drawing (use Paint event)
or
Component: Multiple Image widgets arranged as line segments
Properties:
  - Color: Fades from white (cursor) to cyan (ship)
  - Thickness: 2-3 pixels
  - Style: Solid or dashed
```

#### D. Deadzone Circle
```
Component: Image (circle texture)
Name: DeadzoneCircle
Properties:
  - Anchors: Center (0.5, 0.5)
  - Size: Calculated from DeadzoneRadiusPercent
  - Color: White with low opacity (Alpha 30-50)
  - Visibility: Controlled by bShowDeadzoneCircle
```

#### E. Rotation Speed Bar (Optional)
```
Component: Progress Bar
Name: RotationSpeedBar
Properties:
  - Anchors: Top-Left corner or bottom of screen
  - Size: 200x10 pixels
  - Fill: Based on CurrentRotationSpeed (0-1)
  - Color: Green (low speed) to Red (high speed)
```

### Step 3: Override Blueprint Functions

#### UpdateFlightCrosshair

```blueprint
Event: UpdateFlightCrosshair
Inputs:
  - MouseScreenPosition (Vector2D): 0-1 normalized screen position
  - ShipRotationDirection (Vector2D): 0-1 normalized screen position
  - RotationSpeed (Float): 0-1 rotation speed
  - bInDeadzone (Boolean): Whether mouse is in deadzone

Logic:
1. Get viewport size (Get Viewport Size node)
2. Convert MouseScreenPosition to pixel coordinates:
   - PixelX = MouseScreenPosition.X * ViewportWidth
   - PixelY = MouseScreenPosition.Y * ViewportHeight
3. Set MouseCursorIndicator position:
   - SetRenderTranslation(PixelX - HalfIndicatorSize, PixelY - HalfIndicatorSize)
4. Convert ShipRotationDirection to pixel coordinates (same formula)
5. Set ShipDirectionIndicator position
6. Update connection line between the two positions
7. If bInDeadzone:
   - Set MouseCursorIndicator color to gray or dim white
   - Set ShipDirectionIndicator to center position
8. Else:
   - Set MouseCursorIndicator color based on RotationSpeed
   - Interpolate colors (white -> yellow -> red as speed increases)
9. Update rotation speed visual feedback
```

#### UpdateDeadzoneVisualization

```blueprint
Event: UpdateDeadzoneVisualization
Inputs:
  - DeadzoneRadiusPercent (Float): 0-1 inner deadzone radius
  - MaxRadiusPercent (Float): 0-1 max effective radius

Logic:
1. Get viewport size
2. Calculate deadzone diameter in pixels:
   - Diameter = DeadzoneRadiusPercent * ViewportWidth
3. Set DeadzoneCircle size:
   - SetWidgetSize(Diameter, Diameter)
4. Optionally draw outer circle at MaxRadiusPercent
5. Center both circles on screen
```

#### SetFlightCrosshairVisible

```blueprint
Event: SetFlightCrosshairVisible
Input: bVisible (Boolean)

Logic:
1. Set visibility of all crosshair elements:
   - MouseCursorIndicator: Visible if bVisible
   - ShipDirectionIndicator: Visible if bVisible
   - ConnectionLine: Visible if bVisible
2. Store bFlightCrosshairVisible = bVisible
```

---

## Visual Design Guidelines

### Color Scheme (X4-Inspired)

```
Mouse Cursor:
  - Neutral/Deadzone: RGB(180, 180, 180) - Light Gray
  - Active/Rotating: RGB(255, 255, 255) - White
  - High Speed: RGB(255, 200, 100) - Yellow-Orange

Ship Direction:
  - Default: RGB(100, 200, 255) - Cyan
  - High Speed: RGB(255, 100, 100) - Red
  - Aligned with mouse: RGB(100, 255, 100) - Green

Connection Line:
  - Gradient from Mouse color to Ship color
  - Opacity based on distance between them

Deadzone Circle:
  - RGB(255, 255, 255, 50) - Subtle white with low opacity
  - Dashed or dotted style for minimal distraction
```

### Size Recommendations

```
Mouse Cursor: 28-32 pixels (small, precise)
Ship Direction: 40-48 pixels (larger, more visible)
Connection Line: 2-3 pixels thick
Deadzone Circle: 100-200 pixels diameter (adjust for screen size)
Rotation Speed Bar: 200x8 pixels
```

### Animation and Smoothing

```
Position Updates:
  - Lerp/Interpolate positions over 0.05-0.1 seconds for smooth movement
  - Use "Ease Out" curves for natural deceleration

Color Transitions:
  - Lerp colors over 0.2 seconds
  - Avoid sudden flashing

Size Scaling:
  - Scale indicators based on rotation speed (pulse effect)
  - Range: 0.9x to 1.1x of base size
```

---

## Configuration Properties

These properties are exposed from the Spaceship class and HUD:

### Spaceship Properties (Affect Crosshair Behavior)

```cpp
bUseMousePositionFlight (Boolean, default: true)
  - Enables X4-style position-based flight
  - When false, crosshair uses delta mode (traditional mouse look)

MouseDeadzoneRadius (Float, default: 50.0 pixels)
  - Size of center deadzone where no rotation occurs
  - Larger = more stable, less precise
  - Smaller = more responsive, potentially twitchy

MouseMaxRadius (Float, default: 400.0 pixels)
  - Distance from center for 100% rotation speed
  - Larger = slower sensitivity, more control
  - Smaller = faster sensitivity, more responsive

MouseFlightSensitivity (Float, default: 1.0)
  - Global sensitivity multiplier
  - Range: 0.1 (very slow) to 5.0 (very fast)
```

### HUD Properties

```cpp
bFlightCrosshairVisible (Boolean, default: true)
  - Master visibility toggle for entire crosshair system

bShowDeadzoneCircle (Boolean, default: false)
  - Whether to show deadzone visualization
  - Useful for new players learning the system

FlightMousePosition (Vector2D)
  - Current mouse position in screen space (0-1)
  - Read-only, updated automatically

ShipRotationIndicator (Vector2D)
  - Current ship rotation direction in screen space (0-1)
  - Read-only, updated automatically

CurrentRotationSpeed (Float, 0-1)
  - Normalized rotation speed
  - Read-only, updated automatically

bMouseInDeadzone (Boolean)
  - Whether mouse is currently in deadzone
  - Read-only, updated automatically
```

---

## Usage Examples

### Example 1: Basic Crosshair

Minimal implementation showing just mouse and ship direction:

```
Blueprint Graph:
1. UpdateFlightCrosshair event
2. Get MouseCursorIndicator widget
3. Set Position from MouseScreenPosition
4. Get ShipDirectionIndicator widget
5. Set Position from ShipRotationDirection
6. Set Opacity based on RotationSpeed
```

### Example 2: Full Featured with Deadzone

Complete implementation with all features:

```
Blueprint Graph:
1. UpdateFlightCrosshair event
2. Branch on bInDeadzone:
   True: 
     - Dim MouseCursor
     - Center ShipDirection
     - Hide ConnectionLine
   False:
     - Full opacity MouseCursor
     - Update ShipDirection position
     - Draw ConnectionLine
3. Update color based on RotationSpeed
4. UpdateDeadzoneVisualization event
5. Resize DeadzoneCircle based on parameters
```

### Example 3: Advanced with Rotation Prediction

Shows predicted rotation arc:

```
Blueprint Graph:
1. Calculate rotation velocity vector
2. Project future rotation direction (0.5 seconds ahead)
3. Draw arc from current direction to predicted direction
4. Color arc based on rotation speed
5. Add rotation rate indicators (degrees/second)
```

---

## Testing Checklist

- [ ] Crosshair visible when flying ship
- [ ] Mouse cursor indicator follows mouse accurately
- [ ] Ship direction indicator shows rotation intent
- [ ] Connection line draws between indicators
- [ ] Deadzone circle visible when enabled
- [ ] Colors change based on rotation speed
- [ ] Crosshair hides in deadzone appropriately
- [ ] Smooth interpolation (no jittering)
- [ ] Performance (60 FPS maintained)
- [ ] Works at different resolutions (1920x1080, 2560x1440, 3840x2160)

---

## Troubleshooting

### Crosshair Not Visible

**Possible Causes:**
1. `bFlightCrosshairVisible = false`
2. Widget not added to viewport
3. Z-order too low (behind other widgets)
4. Opacity set to 0

**Solutions:**
- Check property value in Spaceship Blueprint
- Verify widget is created and added in PlayerController
- Adjust Z-Order to be on top
- Check material/image opacity settings

### Indicators Not Following Mouse/Ship

**Possible Causes:**
1. UpdateFlightCrosshair not being called
2. Position calculation incorrect
3. Widget anchors not set to center
4. Canvas scale mode incorrect

**Solutions:**
- Add debug print nodes to verify function is called
- Log MouseScreenPosition values
- Set anchors to 0.5, 0.5 (center)
- Use "Scale to Fit" canvas mode

### Deadzone Circle Wrong Size

**Possible Causes:**
1. DeadzoneRadiusPercent not calculated correctly
2. Viewport size not updated
3. Circle texture aspect ratio incorrect

**Solutions:**
- Verify radius calculation: `Radius / (ViewportWidth * 0.5)`
- Get fresh viewport size each frame
- Use square texture (1:1 aspect ratio)

### Performance Issues

**Possible Causes:**
1. Too many draw calls in Paint event
2. Creating new widgets each frame
3. Complex line rendering

**Solutions:**
- Cache widget references
- Reuse existing widgets, update positions only
- Simplify line rendering (use simple images instead of custom drawing)
- Update every 2-3 frames instead of every frame

---

## Advanced Customization

### Custom Crosshair Styles

Create different crosshair appearances for different ships or preferences:

**Fighter Style:**
- Small, precise crosshair
- Bright colors (white/yellow)
- Fast updates
- Minimal deadzone visualization

**Capital Ship Style:**
- Large, prominent indicators
- Slower animation
- Thicker connection lines
- Always show deadzone

**Stealth Style:**
- Dim, minimal indicators
- Blue/purple colors
- Fades when not moving
- No deadzone circle

### Rotation Prediction Arc

Show where the ship will be pointing 0.5 seconds from now:

```
1. Get current rotation velocity from ship
2. Calculate future rotation: CurrentRotation + (RotationVelocity * 0.5)
3. Project to screen space
4. Draw arc from current direction to predicted direction
5. Color based on certainty (green = accurate, yellow = approximate)
```

### Target Lead Indicator

When targeting an enemy, show where to aim:

```
1. Get target velocity
2. Calculate projectile travel time
3. Predict target position at intercept
4. Draw lead indicator at predicted position
5. Color changes when in optimal firing range
```

---

## API Reference

### C++ Functions (AdastreaHUDWidget)

```cpp
// Update the complete flight crosshair system
void UpdateFlightCrosshair(
    FVector2D MouseScreenPosition,     // Mouse position (0-1)
    FVector2D ShipRotationDirection,   // Ship direction (0-1)
    float RotationSpeed,                // Speed (0-1)
    bool bInDeadzone                    // Deadzone flag
);

// Show/hide flight crosshair
void SetFlightCrosshairVisible(bool bVisible);

// Update deadzone visualization
void UpdateDeadzoneVisualization(
    float DeadzoneRadiusPercent,  // Inner radius (0-1)
    float MaxRadiusPercent        // Outer radius (0-1)
);

// Show/hide deadzone circle
void ShowDeadzoneVisualization(bool bShow);
```

### Blueprint Events

All functions above have corresponding Blueprint events that can be overridden in your Widget Blueprint.

---

## Performance Optimization

### Update Frequency

```blueprint
// Instead of updating every frame:
Custom Event: UpdateCrosshairThrottled
Add to Event Graph:

1. Get World Delta Seconds
2. Add to AccumulatedTime variable
3. Branch: If AccumulatedTime > 0.033 (30 FPS update rate)
4. Call UpdateFlightCrosshair
5. Reset AccumulatedTime
```

### Object Pooling

```blueprint
// Cache widget references on construct:
Event Construct:
1. Find MouseCursorIndicator widget
2. Store in variable
3. Find ShipDirectionIndicator widget
4. Store in variable
5. Find all other widgets
6. Never search for them again
```

---

## Accessibility Considerations

### Colorblind Modes

Provide alternative color schemes:
- **Protanopia (Red-blind)**: Use blue/yellow instead of red/green
- **Deuteranopia (Green-blind)**: Use blue/orange instead of red/green
- **Tritanopia (Blue-blind)**: Use red/green instead of blue/yellow

### High Contrast Mode

- Increase indicator sizes
- Use solid colors (no gradients)
- Thicker lines
- Higher opacity

### Motion Sensitivity

- Reduce animation speed
- Disable pulsing effects
- Static indicators option

---

## Future Enhancements

Potential additions to the system:

1. **Haptic Feedback**: Controller rumble when exiting deadzone
2. **Audio Cues**: Sound effect when rotation speed increases
3. **3D Projection**: Project crosshair onto nearby objects
4. **Customizable Skins**: Player-selectable crosshair appearances
5. **Training Mode**: Highlighted deadzone with tutorial prompts
6. **Recording/Replay**: Save and analyze flight patterns

---

## See Also

- `X4FlightControlsGuide.md` - Complete flight controls documentation
- `X4FlightControlsQuickReference.md` - Quick reference card
- `HUDSystemGuide.md` - General HUD system documentation
- `Spaceship.h` - Spaceship C++ implementation
- `AdastreaHUDWidget.h` - HUD widget C++ implementation

---

**Last Updated**: January 13, 2026  
**Version**: 1.0  
**Author**: Adastrea Development Team
