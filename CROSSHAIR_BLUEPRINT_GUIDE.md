# Weapon Aim Crosshair - Blueprint Implementation Guide

## Quick Start: Add Crosshair to Your HUD

This guide shows how to implement the weapon aim crosshair visuals in your HUD Widget Blueprint.

## Prerequisites
- HUD Widget Blueprint based on `UAdastreaHUDWidget` (e.g., `WBP_AdastreaHUD`)
- Basic knowledge of UMG (Unreal Motion Graphics)
- A crosshair texture or material

## Step 1: Set Up the Widget Hierarchy

### In the Designer Tab:
1. **Open your HUD Widget Blueprint** (e.g., `WBP_AdastreaHUD`)
2. **Ensure you have a Canvas Panel** as root or main container
   - If not, add one: `Add Widget -> Panel -> Canvas Panel`
3. **Add an Image widget** as a child of Canvas Panel
   - `Add Widget -> Common -> Image`
   - Name it: `WeaponAimCrosshair`

### Configure the Image Widget:
```
Properties:
├─ Appearance
│  ├─ Brush
│  │  └─ Image: [Your crosshair texture, e.g., T_Crosshair_Aim]
│  └─ Tint: White (or your preferred color)
├─ Transform
│  └─ Size: 32 x 32 (or desired crosshair size)
└─ Slot (Canvas Panel Slot)
   ├─ Anchors: Center (0.5, 0.5)
   ├─ Position: (0, 0) [Will be updated dynamically]
   └─ Alignment: (0.5, 0.5) [Centers the crosshair on position]
```

## Step 2: Bind Crosshair Position (Event Graph)

### Method A: Using Event Tick (Simple)
```
Event Graph:

[Event Tick]
    |
    └─> [Get WeaponAimPosition]
           |
           └─> [Get Owning Player]
                  |
                  └─> [Get Viewport Size]
                         |
                         └─> [Break Vector2D] (WeaponAimPosition)
                                |
                                ├─> X * ViewportSize.X
                                └─> Y * ViewportSize.Y
                                        |
                                        └─> [Make Vector2D]
                                               |
                                               └─> [Set Canvas Position] (WeaponAimCrosshair)
```

### Detailed Node Setup:
1. **Event Tick** node
2. **Get WeaponAimPosition** (inherited from parent class)
   - Returns FVector2D (0-1 normalized coordinates)
3. **Get Owning Player** → **Get Viewport Size**
   - Returns X and Y viewport dimensions
4. **Break Vector2D** on WeaponAimPosition
5. **Multiply** X by Viewport Size X
6. **Multiply** Y by Viewport Size Y
7. **Make Vector2D** with the results
8. **Get WeaponAimCrosshair** (your Image widget)
9. **Set Canvas Position** with the calculated position

### Method B: Using Property Binding (Blueprint Only)
1. **Select WeaponAimCrosshair** in the Designer
2. **Click "Bind" next to Position** in the Details panel
3. **Create Binding** → New function opens
4. Implement the same logic as Method A
5. Return the calculated position

## Step 3: Bind Crosshair Visibility

### Simple Visibility Binding:
```
[Get bAimCrosshairVisible]
    |
    └─> [Select] 
           ├─ True: ESlateVisibility::Visible
           └─ False: ESlateVisibility::Collapsed
                |
                └─> [Set Visibility] (WeaponAimCrosshair)
```

### Or Property Binding:
1. **Select WeaponAimCrosshair** in Designer
2. **Bind Visibility** property
3. Return `bAimCrosshairVisible ? Visible : Collapsed`

## Step 4: Customize Appearance (Optional)

### Add Visual Feedback:
```
[Event UpdateAimCrosshair] (Override)
    |
    ├─> [Parent: UpdateAimCrosshair] (Call parent implementation)
    |
    └─> Custom Logic:
         ├─> Check if targeting enemy
         │   └─> Set Crosshair Tint (Red for hostile, Green for friendly)
         ├─> Check weapon lock status
         │   └─> Animate crosshair (Scale up/down)
         └─> Check range to target
             └─> Change crosshair style (Different texture based on range)
```

## Example Blueprint Graph (Pseudocode)

```blueprint
// Event Graph

Event Construct
{
    // Initialize crosshair
    WeaponAimCrosshair.SetVisibility(bAimCrosshairVisible ? Visible : Collapsed);
}

Event Tick (DeltaTime)
{
    // Get parent class properties
    FVector2D AimPos = GetWeaponAimPosition();
    bool bVisible = GetbAimCrosshairVisible();
    
    // Get viewport dimensions
    APlayerController* PC = GetOwningPlayer();
    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);
    
    // Convert normalized position (0-1) to screen pixels
    float ScreenX = AimPos.X * ViewportX;
    float ScreenY = AimPos.Y * ViewportY;
    
    // Update crosshair position (canvas slot)
    FVector2D ScreenPosition = MakeVector2D(ScreenX, ScreenY);
    WeaponAimCrosshair.SetCanvasPosition(ScreenPosition);
    
    // Update visibility
    WeaponAimCrosshair.SetVisibility(bVisible ? Visible : Collapsed);
}

// Optional: Override for custom behavior
Event UpdateAimCrosshair (ScreenPosition: FVector2D)
{
    // Call parent implementation first
    Parent::UpdateAimCrosshair(ScreenPosition);
    
    // Your custom crosshair effects here
    // Example: Change color based on what's under the crosshair
    if (IsTargetingEnemy())
    {
        WeaponAimCrosshair.SetColorAndOpacity(FLinearColor::Red);
    }
    else
    {
        WeaponAimCrosshair.SetColorAndOpacity(FLinearColor::White);
    }
}
```

## Advanced: Dynamic Crosshair Styles

### Create Multiple Crosshair Images:
```
Widget Hierarchy:
Canvas Panel
└─ Overlay (WeaponCrosshairOverlay)
   ├─ Image (Crosshair_Default)    - Default crosshair
   ├─ Image (Crosshair_Locked)     - When target locked
   ├─ Image (Crosshair_Hostile)    - When targeting enemy
   └─ Image (Crosshair_OutOfRange) - When target too far
```

### Switch Between Styles:
```
Function: UpdateCrosshairStyle
{
    // Hide all
    Crosshair_Default.SetVisibility(Collapsed);
    Crosshair_Locked.SetVisibility(Collapsed);
    Crosshair_Hostile.SetVisibility(Collapsed);
    Crosshair_OutOfRange.SetVisibility(Collapsed);
    
    // Show appropriate one
    Switch(CurrentTargetState)
    {
        case None: Crosshair_Default.SetVisibility(Visible);
        case Locked: Crosshair_Locked.SetVisibility(Visible);
        case Hostile: Crosshair_Hostile.SetVisibility(Visible);
        case OutOfRange: Crosshair_OutOfRange.SetVisibility(Visible);
    }
    
    // Position ALL of them (they're overlaid)
    FVector2D Position = GetScreenPosition();
    WeaponCrosshairOverlay.SetCanvasPosition(Position);
}
```

## Crosshair Asset Recommendations

### Create Crosshair Textures:
1. **Size:** 64x64 or 128x128 pixels (power of 2)
2. **Format:** PNG with alpha channel for transparency
3. **Style:** Simple, clear, high contrast
4. **Examples:**
   - Circle with dot in center
   - Cross/plus sign
   - Brackets ([ ])
   - Chevron/arrow shape

### Import Settings:
```
Texture Properties:
├─ Compression Settings: UserInterface2D (BC7)
├─ Texture Group: UI
├─ MipMaps: Disabled
├─ sRGB: Enabled
└─ Alpha Channel: Used for transparency
```

## Testing Checklist

- [ ] Crosshair appears on screen
- [ ] Crosshair follows mouse movement smoothly
- [ ] Crosshair position is accurate (centered on mouse)
- [ ] Crosshair is visible when `bAimCrosshairVisible = true`
- [ ] Crosshair hides when `bAimCrosshairVisible = false`
- [ ] Crosshair stays on screen (doesn't go off-canvas)
- [ ] No performance issues (check with `stat fps`)
- [ ] Works at different screen resolutions
- [ ] Crosshair is properly centered (anchor and alignment)

## Troubleshooting

### Crosshair not visible
- Check `bAimCrosshairVisible` is true
- Verify Image widget has a valid texture/material
- Check widget hierarchy (is it behind other widgets?)
- Verify Canvas Panel is large enough

### Crosshair in wrong position
- Check anchor settings (should be 0.5, 0.5 for center)
- Verify alignment (should be 0.5, 0.5 to center on position)
- Debug: Print `WeaponAimPosition` values (should be 0-1 range)
- Debug: Print calculated screen position

### Crosshair jumps or stutters
- Ensure Event Tick is used (not Event Construct)
- Check for blueprint compilation errors
- Verify no conflicting position updates

### Crosshair not following mouse exactly
- Mouse position is captured correctly (C++ side handles this)
- If offset, adjust alignment/anchor settings
- Consider crosshair texture center point

## Performance Tips

- Use **property binding** sparingly (can be expensive)
- Prefer **Event Tick** for dynamic updates
- Cache widget references (don't search hierarchy each frame)
- Use simple textures (complex materials can slow UI)
- Consider **Invalidation Boxes** for static HUD elements

## Additional Features to Implement

Once basic crosshair works, consider adding:
1. **Spread indicator** - Show weapon accuracy cone
2. **Recoil animation** - Crosshair expands when firing
3. **Hit markers** - Flash when dealing damage
4. **Lead indicator** - Show where to aim for moving targets
5. **Range rings** - Show effective weapon range
6. **Charge indicator** - For charge-up weapons
7. **Ammo counter** - Near the crosshair

---

## Quick Copy-Paste Blueprint Code (Text Format)

For quick implementation, use this Blueprint text graph:

```
Event Tick
→ Get WeaponAimPosition (Self)
→ Break Vector 2D (ReturnValue)
→ Get Owning Player (Self)
→ Get Viewport Size (ReturnValue)
→ X * X (Vector2D.X, ViewportSize.X)
→ Y * Y (Vector2D.Y, ViewportSize.Y)
→ Make Vector 2D (X, Y)
→ Get WeaponAimCrosshair (Self)
→ Set Canvas Position (Target=WeaponAimCrosshair, Position=MakeVector2D)
```

You can copy this into a text file and import as Blueprint nodes (Unreal Engine 5+ feature).

---

**Last Updated:** 2025-12-17  
**Difficulty:** Beginner-Friendly  
**Time to Implement:** 10-15 minutes  
**Maintained by:** Adastrea Development Team
