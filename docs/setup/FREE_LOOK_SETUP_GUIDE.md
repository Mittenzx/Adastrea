# Free Look Camera Setup Guide

This guide explains how to set up the free look camera feature in Unreal Engine after the C++ code changes have been compiled.

## Overview

The free look camera allows players to look around independently from ship rotation by holding down the right mouse button. This is useful for surveying the environment while maintaining the ship's current heading. Additionally, double-clicking the right mouse button instantly returns the camera to the normal forward-facing position.

## Prerequisites

- Adastrea project compiled with the latest free look camera code
- Unreal Engine 5.6+ Editor

## Step-by-Step Setup

### 1. Create the Free Look Input Action

1. Open the Unreal Editor with the Adastrea project
2. Navigate to `Content/Input/` folder
3. Right-click in the Content Browser and select:
   - `Input` → `Input Action`
4. Name it `IA_FreeLook`
5. Open `IA_FreeLook` and configure:
   - **Value Type**: `Axis2D` (for mouse X and Y movement)
   - Leave other settings at default

### 2. Update the Spaceship Input Mapping Context

1. Open `Content/Input/IMC_Spaceship` (Input Mapping Context)
2. Add a new mapping:
   - **Action**: `IA_FreeLook` (the action you just created)
   - **Key**: `Right Mouse Button`
   - **Trigger Type**: Use default (trigger on continuous input)
   - **Modifiers**: 
     - Add `Negate` modifiers if needed for inverted Y axis

### 3. (Optional) Rebind Secondary Weapon Fire

Since Right Mouse Button is now used for free look, you may want to rebind the secondary weapon:

1. Open `Content/Input/IMC_Spaceship`
2. Find the `IA_FireSecondary` mapping
3. Change the key binding from `Right Mouse Button` to:
   - **Recommended**: `Middle Mouse Button` (mouse wheel click)
   - **Alternative**: `Mouse Button 4` (side button) or another key like `F` or `R`

### 4. Update the Input Config Data Asset

1. Navigate to where your `DA_InputConfig` Blueprint Data Asset is located (typically in `Content/Input/` or `Content/Blueprints/`)
2. Open `DA_InputConfig`
3. In the **Input Actions | Camera** section:
   - Set **Free Look Action** to `IA_FreeLook`
4. Save the Data Asset

### 5. Update Blueprint Reference (If Applicable)

If your spaceship Blueprint (`BP_PlayerShip` or similar) has a direct reference to input actions:

1. Open your player spaceship Blueprint
2. Find the **Input** section in the Details panel
3. Set **Free Look Action** to `IA_FreeLook`
4. Compile and save

### 6. Test the Feature

1. Start Play-in-Editor (PIE)
2. Take control of a spaceship
3. **Hold Right Mouse Button** and move the mouse
   - The camera should rotate independently from the ship
   - The ship should maintain its heading
4. **Release Right Mouse Button**
   - The camera should smoothly return to following the ship's forward direction
5. **Double-Click Right Mouse Button**
   - The camera should immediately snap back to the ship's forward direction
   - This provides a quick way to reset the camera view
6. Test normal flight controls to ensure they still work correctly

## Configuration Options

You can adjust the free look behavior in the spaceship Blueprint or C++ class:

### In Blueprint (Recommended)
1. Open your spaceship Blueprint
2. Find these properties under **Camera | Free Look**:
   - **Free Look Sensitivity**: Controls how fast the camera rotates (default: 1.5)
   - **Double Click Threshold**: Time window for double-click detection in seconds (default: 0.3)
   
3. Find these properties under **Camera | Settings**:
   - **Camera Distance**: Distance of camera from ship (default: 800)
   - **Camera Lag Speed**: How smoothly the camera follows position (default: 10)
   - **Camera Rotation Lag Speed**: How smoothly the camera follows rotation - prevents snap (default: 10)

### Camera Spring Arm Settings
The Spring Arm component can be further configured:
- **Target Arm Length**: Zoom distance (controlled by `CameraDistance`)
- **Camera Lag Speed**: Smoothness of camera position movement
- **Camera Rotation Lag Speed**: Smoothness of camera rotation - prevents camera snap when ship rotates

## Troubleshooting

### Camera doesn't move during free look
- Check that `IA_FreeLook` is properly assigned in `DA_InputConfig`
- Verify the Input Mapping Context has the right mouse button bound
- Check console for input binding logs (search for "FreeLookAction")

### Ship rotates instead of camera
- Ensure the `Look()` function properly checks `bFreeLookActive`
- Verify the input action binding is correct in the mapping context

### Camera snaps instead of smoothly following ship rotation
- Increase `Camera Rotation Lag Speed` in the spaceship settings
- Ensure `bEnableCameraRotationLag` is enabled on the Spring Arm component

### Camera snaps instead of smoothly returning from free look
- Increase `Camera Lag Speed` in the spaceship settings
- Ensure `bEnableCameraRotationLag` is enabled on the Spring Arm component

### Secondary weapon fires during free look
- Rebind `IA_FireSecondary` to a different key (see Step 3 above)
- Alternatively, modify weapon firing logic to check `bFreeLookActive`

## Advanced: Mouse Sensitivity Scaling

To add separate sensitivity for free look vs. normal flight:

1. In the spaceship Blueprint or C++:
   - Use `Free Look Sensitivity` for free look camera movement
   - Use `Mouse Flight Sensitivity` for ship rotation
   
2. Default values:
   - `MouseFlightSensitivity`: 1.0 (for ship rotation)
   - `FreeLookSensitivity`: 1.5 (for free look camera)

## Integration with Existing Systems

The free look feature automatically integrates with:
- **Flight Assist**: Works independently from flight assist mode
- **Ship Controls**: WASD/throttle controls remain fully functional during free look
- **Combat**: Left mouse button (primary weapon) still works during free look

## Known Limitations

1. **Secondary Weapon**: Right mouse button can no longer be used for secondary weapon fire
   - **Solution**: Rebind to middle mouse button or another key
   
2. **UI Interaction**: Free look is disabled when in UI mode (station editor, menus, etc.)

3. **Controller Support**: Free look currently uses mouse input only
   - **Future**: Right stick click could be used for controller free look toggle

## Additional Notes

- Free look automatically resets when switching pawns or entering interiors
- The camera pitch is clamped to ±89° to prevent gimbal lock
- Free look state is not persisted between sessions (always starts disabled)

---

**Last Updated**: 2025-12-11  
**Version**: 1.0  
**Related Files**: 
- `Source/Adastrea/Ships/Spaceship.h`
- `Source/Adastrea/Ships/Spaceship.cpp`
- `Source/Adastrea/Public/Input/InputConfigDataAsset.h`
