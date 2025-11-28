# Enhanced Input Implementation Guide

## Overview

The Adastrea Enhanced Input System provides a comprehensive input configuration framework using Unreal Engine 5's Enhanced Input plugin. This guide covers the C++ implementation and Blueprint setup for spaceship controls.

## ⭐ Recommended Approach: Use C++ InputConfigDataAsset

**This is the preferred method** for setting up Enhanced Input in Adastrea. The C++ `InputConfigDataAsset` class provides centralized, type-safe input configuration that's easier to maintain than manually creating dozens of Input Actions.

**Benefits of C++ Approach:**
- ✅ Centralized configuration in one Data Asset
- ✅ Type-safe references to Input Actions
- ✅ Built-in validation functions
- ✅ Consistent with project's data-driven design philosophy
- ✅ Easier debugging and maintenance

**Alternative**: See [ENHANCED_INPUT_GUIDE.md](../ENHANCED_INPUT_GUIDE.md) for manual Blueprint-only setup (not recommended for new users).

## System Status: ✅ COMPLETE (C++ Foundation)

The Enhanced Input System C++ foundation is complete:
- **InputConfigDataAsset** - Centralized input action storage
- **EnhancedInput** module integrated
- **Basic Input Actions exist** - Some Input Actions already created in `Content/Input/`
- **Ready for use** - Create `DA_InputConfig` Data Asset and reference existing actions

## Core Components

### InputConfigDataAsset

A Data Asset that stores references to all Input Actions and Input Mapping Contexts.

**Benefits:**
- Centralized input configuration
- Easy to reference across Blueprints
- Organized by category (Movement, Combat, Navigation, etc.)
- Validation functions to ensure completeness

## Setup Steps

### 1. Create Input Actions

Input Actions define what inputs do (move, fire, look, etc.).

**Create Input Actions in Editor:**

1. **Content Browser → Right Click**
2. **Input → Input Action**
3. **Name:** `IA_Move`, `IA_Look`, `IA_FirePrimary`, etc.

**Configure Action Properties:**

**IA_Move** (Movement):
```
Value Type: Axis2D (Vector 2D)
Consume Input: Yes
Triggers: None (continuous)
Modifiers: None
```

**IA_Look** (Camera):
```
Value Type: Axis2D (Vector 2D)
Consume Input: Yes
Triggers: None (continuous)
Modifiers: 
  - Swizzle Input Axis Values (if Y-axis invert needed)
  - Scalar (for mouse sensitivity)
```

**IA_FirePrimary** (Weapon):
```
Value Type: Digital (Bool)
Consume Input: Yes
Triggers: 
  - Pressed
  - Released
Modifiers: None
```

**IA_Boost** (Boost):
```
Value Type: Digital (Bool)
Consume Input: Yes
Triggers:
  - Pressed (Started)
  - Released (Completed)
Modifiers: None
```

### 2. Create Input Mapping Context

Input Mapping Contexts bind keys to Input Actions.

**Create Mapping Context:**

1. **Content Browser → Right Click**
2. **Input → Input Mapping Context**
3. **Name:** `IMC_Spaceship`

**Add Mappings:**

Open `IMC_Spaceship` and add these mappings:

**Movement (IA_Move):**
```
W Key:
  - Action: IA_Move
  - Modifiers: Input Axis Swizzle (Order: Y)
  - Scale: 1.0

S Key:
  - Action: IA_Move
  - Modifiers: Input Axis Swizzle (Order: Y), Negate
  - Scale: 1.0

A Key:
  - Action: IA_Move
  - Modifiers: Input Axis Swizzle (Order: X), Negate
  - Scale: 1.0

D Key:
  - Action: IA_Move
  - Modifiers: Input Axis Swizzle (Order: X)
  - Scale: 1.0

Gamepad Left Thumbstick:
  - Action: IA_Move
  - Modifiers: Deadzone (0.15)
  - Scale: 1.0
```

**Look (IA_Look):**
```
Mouse XY 2D-Axis:
  - Action: IA_Look
  - Modifiers: 
    * Scalar (X: 0.5, Y: 0.5) [Sensitivity]
  - Scale: 1.0

Gamepad Right Thumbstick:
  - Action: IA_Look
  - Modifiers: Deadzone (0.25)
  - Scale: 1.0
```

**Combat Actions:**
```
Left Mouse Button → IA_FirePrimary
Right Mouse Button → IA_FireSecondary
Tab Key → IA_TargetLock
T Key → IA_NextTarget
R Key (Hold) → IA_NearestTarget
```

**Navigation:**
```
F Key → IA_Autopilot
G Key → IA_SetWaypoint
H Key → IA_ClearWaypoint
N Key → IA_NextWaypoint
```

**System:**
```
Escape Key → IA_Menu
M Key → IA_StationEditor (opens station editor when in spaceship mode)
Tab Key → IA_Map
I Key → IA_Inventory
J Key → IA_QuestLog
F12 Key → IA_Screenshot
```

**Camera:**
```
V Key → IA_CycleCamera
Mouse Wheel Up → IA_ZoomIn
Mouse Wheel Down → IA_ZoomOut
```

### 3. Create Input Config Data Asset

**Create the Data Asset:**

1. **Content Browser → Right Click**
2. **Blueprint Class → InputConfigDataAsset**
3. **Name:** `DA_InputConfig_Default`

**Assign Input Actions and Contexts:**

Open `DA_InputConfig_Default` and assign all the Input Actions and Mapping Contexts you created:

```cpp
// Input Mapping Contexts
SpaceshipMappingContext = IMC_Spaceship
MenuMappingContext = IMC_Menu (create separately)
SpaceshipContextPriority = 0
MenuContextPriority = 1

// Movement Actions
MoveAction = IA_Move
LookAction = IA_Look
BoostAction = IA_Boost
BrakeAction = IA_Brake
RollAction = IA_Roll

// Combat Actions
FirePrimaryAction = IA_FirePrimary
FireSecondaryAction = IA_FireSecondary
TargetLockAction = IA_TargetLock
NextTargetAction = IA_NextTarget
PreviousTargetAction = IA_PreviousTarget
NearestTargetAction = IA_NearestTarget

// ... assign all other actions ...
```

### 4. Setup Player Controller

**In Player Controller Blueprint:**

**BeginPlay Event:**
```blueprint
Event BeginPlay
  ↓
[Get Enhanced Input Local Player Subsystem]
  Player Controller: Self
  ↓
[Add Mapping Context]
  Mapping Context: InputConfig->SpaceshipMappingContext
  Priority: InputConfig->SpaceshipContextPriority
  Options: (default)
```

**Alternative C++ Setup:**
```cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Load input config
    UInputConfigDataAsset* InputConfig = LoadObject<UInputConfigDataAsset>(
        nullptr,
        TEXT("/Game/Input/DA_InputConfig_Default")
    );
    
    if (InputConfig)
    {
        // Add spaceship mapping context
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(
                InputConfig->GetSpaceshipMappingContext(),
                InputConfig->SpaceshipContextPriority
            );
        }
    }
}
```

### 5. Bind Input Actions in Ship Blueprint

**Event Graph:**

**Movement:**
```blueprint
[IA_Move (Enhanced Input Action)]
  ↓
[Get Action Value (Vector2D)]
  ↓
[Break Vector2D] → X, Y
  ↓
[X] → [Multiply] → Strafe Speed → [Add Movement Input] → Right Vector
[Y] → [Multiply] → Forward Speed → [Add Movement Input] → Forward Vector
```

**Look (Camera):**
```blueprint
[IA_Look (Enhanced Input Action)]
  ↓
[Get Action Value (Vector2D)]
  ↓
[Break Vector2D] → X, Y
  ↓
[X] → [Add Controller Yaw Input]
[Y] → [Add Controller Pitch Input]
```

**Fire Primary:**
```blueprint
[IA_FirePrimary (Enhanced Input Action)]
  ↓
Branch on [Triggered]
  True → [Get WeaponComponent] → [Fire]
  False → (weapon stops firing if held)
```

**Boost:**
```blueprint
[IA_Boost (Enhanced Input Action)]
  ↓
[Started] → Set MaxSpeed = 1500 (boosted)
[Completed] → Set MaxSpeed = 800 (normal)
```

**Target Lock:**
```blueprint
[IA_TargetLock (Enhanced Input Action)]
  ↓
[Triggered] → [Get TargetingComponent] → [SelectNearestTarget]
```

**Autopilot:**
```blueprint
[IA_Autopilot (Enhanced Input Action)]
  ↓
[Triggered] → [Get NavigationComponent]
  ↓
  Branch on [IsAutopilotActive]
    True → [DeactivateAutopilot]
    False → [ActivateAutopilot] (to cursor location or waypoint)
```

**Station Editor (M Key):**
```blueprint
[IA_StationEditor (Enhanced Input Action)]
  ↓
[Triggered] → [Get PlayerController] → [Cast to AdastreaPlayerController]
  ↓
[ToggleStationEditor]
  ↓
(This broadcasts OnStationEditorToggle event - bind to this to show/hide UI)
```

Alternative using the event directly in Player Controller Blueprint:
```blueprint
Event BeginPlay
  ↓
[Bind Event to OnStationEditorToggle]
  ↓
  Event → [Toggle Station Editor Widget Visibility]
           → [Create/Remove Widget] or [Set Visibility]
```

## C++ Input Action Binding

**Alternative to Blueprint binding:**

```cpp
void AMySpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Cast to Enhanced Input Component
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Load input config
        UInputConfigDataAsset* InputConfig = LoadInputConfig();
        
        if (InputConfig)
        {
            // Bind movement
            EnhancedInput->BindAction(
                InputConfig->MoveAction,
                ETriggerEvent::Triggered,
                this,
                &AMySpaceship::Move
            );
            
            // Bind look
            EnhancedInput->BindAction(
                InputConfig->LookAction,
                ETriggerEvent::Triggered,
                this,
                &AMySpaceship::Look
            );
            
            // Bind fire
            EnhancedInput->BindAction(
                InputConfig->FirePrimaryAction,
                ETriggerEvent::Started,
                this,
                &AMySpaceship::FirePrimary
            );
            
            // Bind boost
            EnhancedInput->BindAction(
                InputConfig->BoostAction,
                ETriggerEvent::Started,
                this,
                &AMySpaceship::BoostStart
            );
            
            EnhancedInput->BindAction(
                InputConfig->BoostAction,
                ETriggerEvent::Completed,
                this,
                &AMySpaceship::BoostEnd
            );
            
            // ... bind more actions ...
        }
    }
}

void AMySpaceship::Move(const FInputActionValue& Value)
{
    // Get 2D axis value
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    // Add movement
    AddMovementInput(GetActorRightVector(), MovementVector.X);
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
}

void AMySpaceship::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void AMySpaceship::FirePrimary(const FInputActionValue& Value)
{
    if (WeaponComponent)
    {
        WeaponComponent->Fire();
    }
}
```

## Input Contexts Switching

### Switch to Menu Context

```cpp
void AMyPlayerController::OpenMenu()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Remove spaceship context
        Subsystem->RemoveMappingContext(InputConfig->GetSpaceshipMappingContext());
        
        // Add menu context
        Subsystem->AddMappingContext(
            InputConfig->GetMenuMappingContext(),
            InputConfig->MenuContextPriority
        );
    }
}

void AMyPlayerController::CloseMenu()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Remove menu context
        Subsystem->RemoveMappingContext(InputConfig->GetMenuMappingContext());
        
        // Re-add spaceship context
        Subsystem->AddMappingContext(
            InputConfig->GetSpaceshipMappingContext(),
            InputConfig->SpaceshipContextPriority
        );
    }
}
```

## Input Settings Configuration

### Project Settings

**Edit → Project Settings → Enhanced Input:**

```
Default Player Input Class: EnhancedPlayerInput
Default Input Component Class: EnhancedInputComponent
```

### DefaultInput.ini

Add to `Config/DefaultInput.ini`:

```ini
[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent

bEnableMouseSmoothing=False
bEnableFOVScaling=True
bViewAccelerationEnabled=False
```

## Validation

### Check Input Config

```cpp
void ValidateInputConfig()
{
    UInputConfigDataAsset* InputConfig = LoadInputConfig();
    
    if (InputConfig && InputConfig->IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Input config is valid"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Input config validation failed!"));
        
        if (InputConfig)
        {
            TArray<FString> Unassigned = InputConfig->GetUnassignedActions();
            for (const FString& Action : Unassigned)
            {
                UE_LOG(LogTemp, Error, TEXT("Missing: %s"), *Action);
            }
        }
    }
}
```

## Debugging Input

### Console Commands

```
showdebug enhancedinput - Shows active mapping contexts and actions
```

### Blueprint Debugging

Add Print String nodes to input event handlers to verify they're being called.

## Common Input Actions Reference

### Complete Action List

**Movement:**
- `IA_Move` - WASD/Left Stick movement
- `IA_Look` - Mouse/Right Stick look
- `IA_Boost` - Shift/LT afterburner
- `IA_Brake` - S (hold) / RT slow down
- `IA_Roll` - Q/E roll left/right

**Combat:**
- `IA_FirePrimary` - LMB/RT fire primary weapon
- `IA_FireSecondary` - RMB/RB fire missiles
- `IA_TargetLock` - Tab/X lock target
- `IA_NextTarget` - T/D-pad Right cycle targets
- `IA_PreviousTarget` - Shift+T/D-pad Left previous target
- `IA_NearestTarget` - R (hold) nearest enemy

**Interaction:**
- `IA_Interact` - E/A interact with objects
- `IA_Dock` - F/Y dock with station
- `IA_Board` - B/B board/capture ship

**Navigation:**
- `IA_Autopilot` - F toggle autopilot
- `IA_SetWaypoint` - G set waypoint
- `IA_ClearWaypoint` - H clear waypoints
- `IA_NextWaypoint` - N skip waypoint

**System:**
- `IA_Menu` - ESC/Start pause menu
- `IA_StationEditor` - M open station editor (only when in spaceship mode)
- `IA_Map` - Tab/Back open map
- `IA_Inventory` - I open inventory
- `IA_QuestLog` - J open quests
- `IA_Screenshot` - F12 take screenshot

**Camera:**
- `IA_CycleCamera` - V cycle camera views
- `IA_ZoomIn` - Mouse Wheel Up zoom in
- `IA_ZoomOut` - Mouse Wheel Down zoom out

## Customization

### Player Settings

Create a settings menu to allow players to rebind keys:

```cpp
void UInputSettingsWidget::RebindAction(UInputAction* Action, FKey NewKey)
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Get player mappable key settings
        // Modify key binding
        // Save to player profile
    }
}
```

### Sensitivity Settings

```cpp
void UGameSettingsWidget::SetMouseSensitivity(float Sensitivity)
{
    // Update scalar modifier on Look action
    // Range: 0.1 - 2.0
}
```

## Best Practices

1. **Use Input Config Data Asset:**
   - Single source of truth for all input
   - Easy to update and maintain
   - Validation built-in

2. **Organize by Category:**
   - Group related actions
   - Use consistent naming
   - Document in tooltips

3. **Use Appropriate Value Types:**
   - Axis2D for movement/look
   - Digital (Bool) for buttons
   - Axis1D for triggers/analog inputs

4. **Configure Triggers Properly:**
   - Pressed for immediate response
   - Started/Completed for state changes
   - Triggered for continuous input

5. **Add Deadzones:**
   - Gamepad thumbsticks: 0.15-0.25
   - Triggers: 0.1
   - Prevents drift and unintended input

## Troubleshooting

### Input Not Working

**Check:**
1. Mapping context is added to subsystem
2. Input Action is assigned in Input Config
3. Mapping exists in Input Mapping Context
4. Enhanced Input plugin is enabled
5. Project settings use Enhanced Input classes

### Wrong Context Active

**Check:**
1. Context priority (higher = more priority)
2. Context wasn't removed when it should be
3. Multiple contexts fighting for same input

### Sensitivity Issues

**Check:**
1. Scalar modifier values in mapping context
2. Mouse settings in Project Settings
3. Look action configuration

## See Also

- `InputConfigDataAsset.h` - Input configuration system
- `ENHANCED_INPUT_GUIDE.md` - Original input guide
- `NavigationSystemGuide.md` - Autopilot integration
- [Unreal Engine Enhanced Input Documentation](https://docs.unrealengine.com/5.3/en-US/enhanced-input-in-unreal-engine/)

---

**Last Updated**: 2025-11-11  
**Version**: 1.0  
**Status**: C++ Foundation Complete  
**Component**: InputConfigDataAsset  
**Lines of Code**: ~240 lines (header + implementation)
