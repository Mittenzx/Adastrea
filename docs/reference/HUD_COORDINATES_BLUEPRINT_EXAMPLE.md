# HUD Coordinates Blueprint Example

This document shows a simple Blueprint implementation for displaying coordinates in the HUD.

## Visual Mockup

```
┌─────────────────────────────────────────────┐
│  HUD Widget (WBP_HUD_Main)                  │
│                                             │
│  ┌──────────────────────┐                  │
│  │ Position: X=1234.5   │ ← Text Block     │
│  │          Y=6789.0    │                  │
│  │          Z=345.2     │                  │
│  └──────────────────────┘                  │
│                                             │
└─────────────────────────────────────────────┘
```

## Simple Blueprint Implementation

### Option 1: Property Binding (Easiest)

**Widget Hierarchy:**
```
Canvas Panel
  └─ Text Block (Name: "CoordinatesText")
       └─ Text Binding: Custom
```

**Text Binding Blueprint:**
```
Get Current Coordinates
  ↓
Break Vector (Split into X, Y, Z)
  ↓
Format Text
  Format: "Position:\nX={0:0.1}\nY={1:0.1}\nZ={2:0.1}"
  Arguments: X, Y, Z
  ↓
Return FText
```

### Option 2: Event-Driven Update (More Control)

**Override UpdateCoordinates Event:**

```blueprint
Event: UpdateCoordinates (Position)
  ↓
Branch: Position != Previous Position
  True:
    ↓
    Store Position as "Previous Position"
    ↓
    Break Vector (Position)
      ↓ X, Y, Z
    Format Text
      ↓
    CoordinatesText -> Set Text
```

## Styling Recommendations

### Text Properties
- **Font**: Roboto or similar modern sans-serif
- **Size**: 14-16 for readability
- **Color**: White (#FFFFFF) or Light Blue (#00CCFF)
- **Outline**: Dark shadow for contrast against any background
- **Opacity**: 80-90% to not distract

### Positioning
- **Top-Left**: Traditional for debug info
- **Top-Right**: Less intrusive
- **Bottom-Left**: Near speed/status indicators
- **HUD Panel**: Part of navigation cluster

## Advanced Example: Navigation Panel

```
┌─────────────────────────────────┐
│  Navigation Panel               │
├─────────────────────────────────┤
│  📍 Position                    │
│     X: 1234.5 m                 │
│     Y: 6789.0 m                 │
│     Z: 345.2 m                  │
│                                 │
│  🎯 Target: Station Alpha       │
│     Distance: 5432.1 m          │
│     Bearing: 045°               │
│                                 │
│  ⚡ Speed: 150 m/s              │
└─────────────────────────────────┘
```

### Widget Structure
```
Vertical Box (Navigation Panel)
  ├─ Text Block: "📍 Position"
  ├─ Horizontal Box
  │   ├─ Text Block: "   X:"
  │   └─ Text Block: {CoordX} (Bound)
  ├─ Horizontal Box
  │   ├─ Text Block: "   Y:"
  │   └─ Text Block: {CoordY} (Bound)
  ├─ Horizontal Box
  │   ├─ Text Block: "   Z:"
  │   └─ Text Block: {CoordZ} (Bound)
  └─ (Other navigation info...)
```

## Blueprint Code Examples

### Format as Meters (from Unreal Units)

```blueprint
Event: UpdateCoordinates (Position)
  ↓
Break Vector (Position) → X, Y, Z
  ↓
Divide: X / 100.0 → X_Meters
Divide: Y / 100.0 → Y_Meters
Divide: Z / 100.0 → Z_Meters
  ↓
Format Text ("X: {0:0.1}m, Y: {1:0.1}m, Z: {2:0.1}m")
  ↓
Set Text
```

### Show Distance from Origin

```blueprint
Event: UpdateCoordinates (Position)
  ↓
Vector Length (Position) → Distance
  ↓
Divide: Distance / 100.0 → Distance_Meters
  ↓
Format Text ("Distance from Origin: {0:0.1}m", Distance_Meters)
  ↓
Set Text
```

### Compact Display

```blueprint
Event: UpdateCoordinates (Position)
  ↓
Break Vector → X, Y, Z
  ↓
Round to Int: X, Y, Z
  ↓
Format Text ("[{0}, {1}, {2}]", X, Y, Z)
  ↓
Set Text
```

## Testing in Editor

1. **Open WBP_HUD_Main** (or your HUD widget)
2. **Add Text Block** for coordinates
3. **PIE (Play in Editor)** with spaceship
4. **Fly around** and verify coordinates update
5. **Check performance** in Stat Widget/Stat Unit

## Performance Considerations

### Every Frame Updates
- The C++ side updates coordinates every frame
- Blueprint binding also evaluates every frame
- Consider throttling text updates if needed

### Optimization Example
```blueprint
Event: Update HUD From Game State (Delta Time)
  ↓
Add to Timer: UpdateTimer + DeltaTime
  ↓
Branch: UpdateTimer >= 0.1 (update every 100ms)
  True:
    ↓
    Reset UpdateTimer to 0
    ↓
    Update Coordinate Text
    ↓
  False:
    (Skip update this frame)
```

## Common Patterns

### Toggle Visibility
```blueprint
Key Press: F1
  ↓
Toggle: CoordinatesText Visibility
```

### Different Formats Based on Context
```blueprint
Is In Combat?
  True: Show compact format [X,Y,Z]
  False: Show detailed format with labels
```

### Highlight on Change
```blueprint
Position Changed?
  True:
    Set Color: Yellow
    Wait 0.5s
    Set Color: White
```

---

**Related Files:**
- C++ Header: `Source/Adastrea/Public/UI/AdastreaHUDWidget.h`
- C++ Implementation: `Source/Adastrea/Private/UI/AdastreaHUDWidget.cpp`
- Usage Guide: [HUD_COORDINATES_USAGE.md](HUD_COORDINATES_USAGE.md)

**Blueprint Assets:**
- Example: `Content/UI/HUD/WBP_HUD_Main.uasset`
- Widget Component: Based on `UAdastreaHUDWidget`
