# HUD Coordinate Display - Usage Guide

> **Quick guide for displaying player coordinates in the HUD**

---

## Overview

The HUD widget now automatically tracks and updates the player's 3D coordinates (X, Y, Z position) in world space. This feature is useful for navigation and debugging.

## Implementation Details

### C++ Side (Automatic)

The coordinates are automatically updated in the HUD's `UpdateHUDFromGameState_Implementation` function:

```cpp
// Automatically called every frame
void UAdastreaHUDWidget::UpdateHUDFromGameState_Implementation(float DeltaTime)
{
    ControlledSpaceship = GetControlledSpaceship();
    
    if (ControlledSpaceship)
    {
        FVector CurrentPosition = ControlledSpaceship->GetActorLocation();
        UpdateCoordinates(CurrentPosition);
    }
}
```

### Available Properties

**Blueprint Read-Only Property:**
- `CurrentCoordinates` (FVector) - The player's current position (X, Y, Z)

### Blueprint Usage

#### Method 1: Direct Access to Property

```
1. Create Widget Blueprint based on UAdastreaHUDWidget
2. Add Text Block for coordinate display
3. Bind Text property to:
   - Get CurrentCoordinates
   - Break Vector (X, Y, Z)
   - Format Text: "X: {X}, Y: {Y}, Z: {Z}"
```

#### Method 2: Override UpdateCoordinates Event

```
1. Create Widget Blueprint based on UAdastreaHUDWidget
2. Override "UpdateCoordinates" event
3. Format the Position vector as desired
4. Update your text widgets with formatted values
```

**Example Blueprint:**
```
Event UpdateCoordinates (Position)
    ↓
Break Vector (Position) → X, Y, Z
    ↓
Format Text ("X: {0:0.0}, Y: {1:0.0}, Z: {2:0.0}", X, Y, Z)
    ↓
Set Text: Coordinates_Text
```

## Display Formats

### Standard Format (with decimal places)
```
X: 1234.5, Y: 6789.0, Z: 345.2
```

### Minimal Format (integers only)
```
[1234, 6789, 345]
```

### Verbose Format
```
Position: X=1234.5m, Y=6789.0m, Z=345.2m
```

### Distance from Origin
```blueprint
Get CurrentCoordinates
    ↓
Vector Length → Distance
    ↓
Format Text ("Distance: {0:0.1}m", Distance)
```

## Blueprint Functions

### UpdateCoordinates (BlueprintNativeEvent)
**Category:** HUD|Navigation  
**Parameters:** Position (FVector)  
**Description:** Called automatically when player coordinates change

Override this in Blueprint to customize coordinate display formatting.

## Example Widget Setup

### Simple Text Display

1. **Add Text Block** to HUD widget canvas
2. **Position** in top-left corner (or preferred location)
3. **Bind Text Property**:
   - Click "Bind" next to Text
   - Create Binding
   - Get CurrentCoordinates
   - Format as desired
   - Return formatted FText

### Advanced Display with Icon

1. **Add Horizontal Box**
2. **Add Image** (navigation icon)
3. **Add Text Block** for X coordinate
4. **Add Text Block** for Y coordinate
5. **Add Text Block** for Z coordinate
6. Style with colors/fonts as needed

## Tips

### Performance
- The coordinate update is automatic and runs every frame
- Consider updating text less frequently if performance is a concern
- Can check if value changed before updating text widget

### Formatting
- Use `FText::Format` for localization support
- Round to 1-2 decimal places for readability
- Consider using abbreviated notation for very large values

### Units
- Coordinates are in Unreal Units (cm)
- To display in meters: divide by 100
- To display in kilometers: divide by 100,000

## Common Issues

### Coordinates Not Updating
- Ensure player is in a spaceship (not null)
- Check that HUD widget is added to viewport
- Verify UpdateHUDFromGameState is being called

### Coordinates are Zero
- Player might not have spawned yet
- Spaceship might not be possessed by player
- Check that GetControlledSpaceship returns valid reference

## Integration with Other Systems

### With Minimap
- Use CurrentCoordinates to position player marker on minimap
- Convert world coordinates to minimap coordinates

### With Waypoints
- Calculate distance to waypoints using CurrentCoordinates
- Display direction arrows based on relative position

### With Navigation
- Show coordinates of selected destination
- Display distance remaining to target

---

## Example Blueprint Implementations

### Compact Display (Top-Right Corner)

```
Text Block Properties:
- Font Size: 12
- Color: White with 80% opacity
- Shadow: Small drop shadow for readability

Binding:
Get CurrentCoordinates
  → Break Vector
  → Format: "[{0:0}, {1:0}, {2:0}]"
```

### Detailed Navigation Panel

```
Vertical Box:
  - Text: "Position"
  - Text: "X: {X:0.1}m"
  - Text: "Y: {Y:0.1}m"
  - Text: "Z: {Z:0.1}m"
  - Text: "Distance from Origin: {Distance:0.1}m"
```

---

**Related Documentation:**
- [HUD System Guide](../archive/pre-mvp-systems/assets-guides/HUDSystemGuide.md)
- [HUD Quick Reference](../archive/pre-mvp-systems/assets-guides/HUD_QUICK_REFERENCE.md)
- [AdastreaHUDWidget C++ Reference](../../Source/Adastrea/Public/UI/AdastreaHUDWidget.h)

**Last Updated:** 2026-01-29  
**Feature Added:** PR #[number]
