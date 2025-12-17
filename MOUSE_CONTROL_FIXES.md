# Mouse Control Fixes - Implementation Summary

## Overview
This document summarizes the changes made to fix mouse control issues and add roll controls to the spaceship flight system.

## Issues Addressed

1. **Mouse Y-axis inversion** - Mouse up/down was inverted
2. **Unwanted roll when looking up/down** - Ship would roll when moving mouse vertically
3. **Missing roll controls** - No way to manually roll the ship
4. **Lack of visual feedback** - No second crosshair to show weapon aim direction

## Changes Made

### 1. Fixed Mouse Y-Axis Inversion
**File:** `Config/DefaultInput.ini`
- Changed `LookUp` axis mapping from `Scale=-1.000000` to `Scale=1.000000`
- This ensures mouse up = pitch up, mouse down = pitch down (not inverted)

### 2. Added Roll Controls (Q and E)
**Files Modified:**
- `Config/DefaultInput.ini` - Added Roll axis mappings
- `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h` - Added RollAction and OnRollInput
- `Source/Adastrea/Ships/SpaceshipControlsComponent.cpp` - Implemented roll input handling
- `Source/Adastrea/Ships/Spaceship.h` - Added Roll() function and RollInput variable
- `Source/Adastrea/Ships/Spaceship.cpp` - Implemented Roll() with flight assist support

**Key bindings:**
- **Q** - Roll left (counter-clockwise)
- **E** - Roll right (clockwise)

### 3. Fixed Rotation Behavior
**File:** `Source/Adastrea/Ships/SpaceshipControlsComponent.cpp`
- Changed `OnLookInput_Implementation()` from using `AddActorWorldRotation()` to `AddActorLocalRotation()`
- This prevents gimbal lock and unwanted roll when the ship is at an angle
- Rotation now happens in the ship's local coordinate system

**File:** `Source/Adastrea/Ships/Spaceship.cpp`
- Updated `Roll()` function to use `AddActorLocalRotation()` for consistent behavior
- Added roll velocity damping in `ApplyFlightAssist()` for smooth roll control

### 4. Added Weapon Aim Crosshair
**Files Modified:**
- `Source/Adastrea/UI/AdastreaHUDWidget.h` - Added crosshair tracking properties and functions
- `Source/Adastrea/UI/AdastreaHUDWidget.cpp` - Implemented automatic mouse tracking

**New Features:**
- `WeaponAimPosition` - Normalized screen position (0-1 range) tracking mouse location
- `bAimCrosshairVisible` - Controls crosshair visibility
- `UpdateAimCrosshair()` - BlueprintNativeEvent for updating crosshair position
- `SetAimCrosshairVisible()` - BlueprintNativeEvent for showing/hiding crosshair
- Automatic mouse tracking in `NativeTick()` - No manual setup required

## Testing Instructions

### 1. Build the Project
```bash
# Open Adastrea.uproject in Unreal Engine 5.6
# The engine will automatically compile the C++ changes
# Or right-click Adastrea.uproject and select "Generate Visual Studio project files"
# Then build in Development Editor configuration
```

### 2. Test Mouse Controls
1. **Start PIE (Play In Editor)**
2. **Possess a spaceship** (if not already)
3. **Test mouse pitch (up/down):**
   - Move mouse up → Ship nose should pitch up
   - Move mouse down → Ship nose should pitch down
   - ✅ Should NOT be inverted anymore
   - ✅ Should NOT cause unwanted roll
4. **Test mouse yaw (left/right):**
   - Move mouse left → Ship should yaw left
   - Move mouse right → Ship should yaw right

### 3. Test Roll Controls
1. **Press Q key** → Ship should roll counter-clockwise (left)
2. **Press E key** → Ship should roll clockwise (right)
3. **Release Q/E** → Roll should smoothly stop (if flight assist enabled)

### 4. Test Weapon Aim Crosshair
1. **Open the HUD widget Blueprint** (e.g., `WBP_AdastreaHUD`)
2. **Add visual elements** to represent the weapon aim crosshair:
   - Add an Image widget for the crosshair graphic
   - Bind its position to `WeaponAimPosition` (use Canvas Panel slots)
   - Bind its visibility to `bAimCrosshairVisible`
3. **In-game:**
   - Move mouse around screen
   - Crosshair should follow mouse position
   - Shows where weapons would aim

## Blueprint Integration

### For HUD Widget (Blueprint Designer)
To implement the weapon aim crosshair visuals:

1. **Open your HUD Widget Blueprint** (based on `UAdastreaHUDWidget`)
2. **Add a Canvas Panel** (if not already present)
3. **Add an Image widget** as a child of Canvas Panel
4. **Configure the Image:**
   - Name: `WeaponAimCrosshair`
   - Select a crosshair texture/material
   - Set size (e.g., 32x32 pixels)
   - Set anchor to center (0.5, 0.5)
5. **Bind Position:**
   ```
   Event Graph:
   - On Tick or Event Construct
   - Get WeaponAimPosition (from parent class)
   - Multiply by Viewport Size
   - Set Canvas Position of WeaponAimCrosshair
   ```
6. **Bind Visibility:**
   ```
   - Bind IsVisible to bAimCrosshairVisible
   ```

### Alternative: Override in Blueprint
You can override `UpdateAimCrosshair` in Blueprint:
```
Event UpdateAimCrosshair
  - Get ScreenPosition parameter
  - Update your custom crosshair widget position
  - Add any custom visual effects (e.g., color changes, scaling)
```

## Technical Details

### Coordinate Systems
- **World Rotation** - Rotates in world space (can cause gimbal lock issues)
- **Local Rotation** - Rotates in actor's local coordinate system (prevents gimbal lock)
- We now use **Local Rotation** for all ship controls to ensure predictable behavior

### Flight Assist System
The X4-style flight assist system now handles all three rotation axes:
- **Yaw** - Damped when no mouse X input
- **Pitch** - Damped when no mouse Y input
- **Roll** - Damped when no Q/E input (NEW)

When flight assist is enabled:
- Rotations smooth out when input is released
- Ship maintains orientation without constant input
- Roll gradually returns to level when not actively rolling

### Enhanced Input System
All controls use Unreal Engine 5's Enhanced Input system:
- Input Actions are created programmatically in `SpaceshipControlsComponent`
- Key mappings are configured in `CreateInputMappingContext()`
- Modifiers (like Negate for Q key) handle input inversion
- Blueprint-friendly via `BlueprintNativeEvent` pattern

## Troubleshooting

### Mouse still feels inverted
- Check `SpaceshipControlsComponent->bInvertLookY` property in Blueprint
- Set to `false` if you want normal controls

### Roll doesn't work
- Verify Q and E keys aren't bound to other actions
- Check that `SpaceshipControlsComponent` is attached to your ship Blueprint
- Enable logging: `LogAdastreaInput` category

### Crosshair doesn't show
- Implement visual elements in HUD Widget Blueprint (see above)
- Check `bAimCrosshairVisible` is true
- Verify HUD widget is added to viewport

### Ship rolls randomly
- This was fixed by changing to `AddActorLocalRotation()`
- If still occurring, check for conflicting input bindings
- Verify roll input is only coming from Q/E keys

## Files Modified Summary

### C++ Source Files
1. `Config/DefaultInput.ini` - Input axis mappings
2. `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h` - Roll action declaration
3. `Source/Adastrea/Ships/SpaceshipControlsComponent.cpp` - Roll implementation
4. `Source/Adastrea/Ships/Spaceship.h` - Roll function and variable
5. `Source/Adastrea/Ships/Spaceship.cpp` - Roll logic and damping
6. `Source/Adastrea/UI/AdastreaHUDWidget.h` - Crosshair system declaration
7. `Source/Adastrea/UI/AdastreaHUDWidget.cpp` - Crosshair implementation

### Blueprint Integration Points
- Any `BP_Spaceship_*` Blueprint - Will automatically get roll controls
- `WBP_AdastreaHUD` or similar - Add crosshair visual implementation
- Input Mapping Context - May need regeneration in editor

## Performance Notes
- All changes are minimal and performance-friendly
- Roll damping uses same interpolation as existing pitch/yaw
- Mouse tracking happens once per frame in HUD widget (negligible cost)
- No new components or heavy computations added

## Future Enhancements
Possible improvements for later:
1. **Lead indicator** - Show where to aim for moving targets
2. **Reticle customization** - Allow players to choose crosshair style
3. **Target lock indicator** - Highlight locked targets
4. **Weapon convergence** - Show where weapons will actually hit at current distance
5. **Roll stabilization** - Add auto-leveling option for roll axis

## Credits
Implementation follows Adastrea's design patterns:
- Data-Driven with Blueprint exposure
- X4: Foundations-inspired flight model
- BlueprintNativeEvent for extensibility
- Minimal C++ changes, maximum designer freedom

---

**Last Updated:** 2025-12-17  
**Version:** 1.0  
**Author:** GitHub Copilot (with human oversight)
