# Enhanced Input Configuration Guide

This document explains how to set up Enhanced Input for the spaceship controls.

## Input Actions

### IA_Move (Input Action)
**Value Type**: Vector2D (Axis2D)
**Purpose**: Forward/backward and left/right movement

**Blueprint Setup**:
```
Name: IA_Move
Value Type: Axis2D
Trigger: Down
Modifiers: None
```

### IA_Look (Input Action)
**Value Type**: Vector2D (Axis2D)
**Purpose**: Pitch and yaw rotation

**Blueprint Setup**:
```
Name: IA_Look
Value Type: Axis2D
Trigger: Down
Modifiers: Swizzle Input Axis Values (for inverting Y if needed)
```

### IA_Boost (Input Action)
**Value Type**: Digital (Button)
**Purpose**: Activate boost/afterburner

**Blueprint Setup**:
```
Name: IA_Boost
Value Type: Digital (Bool)
Trigger: Pressed & Released
```

### IA_Fire_Primary (Input Action)
**Value Type**: Digital (Button)
**Purpose**: Fire primary weapons

### IA_Fire_Secondary (Input Action)
**Value Type**: Digital (Button)
**Purpose**: Fire secondary weapons/missiles

### IA_TargetLock (Input Action)
**Value Type**: Digital (Button)
**Purpose**: Lock onto target

### IA_Interact (Input Action)
**Value Type**: Digital (Button)
**Purpose**: Interact with stations/objects

## Input Mapping Context

### IMC_Spaceship (Input Mapping Context)

**Mapping for IA_Move**:
- W: (Y: 1.0) - Forward
- S: (Y: -1.0) - Backward
- A: (X: -1.0) - Left
- D: (X: 1.0) - Right
- Gamepad Left Thumbstick: (X, Y)

**Mapping for IA_Look**:
- Mouse XY: (X, Y) with Sensitivity modifier
- Gamepad Right Thumbstick: (X, Y)

**Mapping for IA_Boost**:
- Left Shift: Trigger on Pressed
- Gamepad Left Trigger: Trigger on value > 0.5

**Mapping for IA_Fire_Primary**:
- Left Mouse Button: Trigger on Pressed
- Gamepad Right Trigger: Trigger on value > 0.5

**Mapping for IA_Fire_Secondary**:
- Right Mouse Button: Trigger on Pressed
- Gamepad Right Shoulder: Trigger on Pressed

**Mapping for IA_TargetLock**:
- Tab: Trigger on Pressed
- Gamepad Face Button Left (X/Square): Trigger on Pressed

**Mapping for IA_Interact**:
- E: Trigger on Pressed
- Gamepad Face Button Bottom (A/Cross): Trigger on Pressed

## Applying Input in Blueprints

### In BP_SpaceshipController (BeginPlay)

```blueprint
Event BeginPlay
  ↓
Get Player Controller (Cast to APlayerController)
  ↓
Get Enhanced Input Local Player Subsystem
  ↓
Add Mapping Context
    Context: IMC_Spaceship
    Priority: 0
    Options: Default
```

### In BP_PlayerShip (Event Graph)

**Movement Input**:
```blueprint
Enhanced Input Action (IA_Move)
  ↓
Action Value (Get 2D Vector)
  ↓
Split Vector (X, Y)
  ↓
[X] Multiply by Strafe Speed (300)
  ↓ Add Actor Local Offset (Right Vector * X)
  
[Y] Multiply by Forward Speed (500)
  ↓ Add Actor Local Offset (Forward Vector * Y)
```

**Rotation Input**:
```blueprint
Enhanced Input Action (IA_Look)
  ↓
Action Value (Get 2D Vector)
  ↓
Split Vector (X for Yaw, Y for Pitch)
  ↓
[X] Multiply by Yaw Rate (50) 
  ↓ Add Actor Local Rotation (Yaw)
  
[Y] Multiply by Pitch Rate (50)
  ↓ Add Actor Local Rotation (Pitch)
```

**Boost Input**:
```blueprint
Enhanced Input Action (IA_Boost)
  ↓
Triggered: Set Max Speed to 1000 (boosted)
  ↓
Completed: Set Max Speed to 500 (normal)
```

**Fire Input**:
```blueprint
Enhanced Input Action (IA_Fire_Primary)
  ↓
Triggered: Call FireWeapon() function
  ↓
Spawn Projectile at Weapon Muzzle
```

## DefaultInput.ini Configuration

The following should already be in `Config/DefaultInput.ini`:

```ini
[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
```

## Mouse Sensitivity Settings

Add to BP_SpaceshipController or create a settings widget:

```blueprint
Mouse Sensitivity Variable (Float, default 0.5)
  ↓
Applied as Scalar modifier in IA_Look mapping
```

## Gamepad Deadzone

Configure in Input Mapping Context:
- Right Thumbstick: Deadzone 0.25
- Left Thumbstick: Deadzone 0.15
- Triggers: Deadzone 0.1

## Input Buffering

For responsive controls, enable in Project Settings:
- `bEnableMouseSmoothing`: False (for precise aim)
- `bEnableFOVScaling`: True (for consistent sensitivity)
- `bViewAccelerationEnabled`: False (for predictable camera)

## Debugging Input

### Check if Input is Received:
1. In ship Blueprint, add Print String to Input Action events
2. Play in editor
3. Press keys and verify messages appear

### Check if Mapping Context is Active:
1. Console command: `showdebug enhancedinput`
2. Verify IMC_Spaceship is listed with Priority 0

## Advanced: Multiple Input Contexts

You can stack contexts for different states:

**IMC_Spaceship** (Priority 0): Base flight controls
**IMC_MenuOpen** (Priority 1): UI navigation, blocks movement
**IMC_Targeting** (Priority 0): Additional targeting controls

Toggle contexts on/off as needed:
```blueprint
Remove Mapping Context (IMC_Spaceship)
Add Mapping Context (IMC_MenuOpen, Priority 1)
```

## Platform-Specific Input

### PC Controls Summary:
- WASD: Movement
- Mouse: Camera/Aim
- LMB: Fire Primary
- RMB: Fire Secondary
- Shift: Boost
- E: Interact
- Tab: Target Lock
- Esc: Pause Menu

### Gamepad Controls Summary:
- Left Stick: Movement
- Right Stick: Camera
- RT: Fire Primary
- LT: Boost
- RB: Fire Secondary
- A: Interact
- X: Target Lock
- Start: Pause Menu

## See Also
- [Unreal Engine Enhanced Input Documentation](https://docs.unrealengine.com/5.3/en-US/enhanced-input-in-unreal-engine/)
- `CONTENT_CREATION_QUICKSTART.md` - Content creation guide
- `CODE_STYLE.md` - Blueprint naming conventions
