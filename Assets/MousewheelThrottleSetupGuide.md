# Mousewheel Throttle Control Setup Guide for Unreal Engine 5.6

## Overview

This guide provides step-by-step instructions for mapping the mousewheel scroll to increase and decrease spaceship throttle in Adastrea. The throttle system uses X4: Foundations-style flight controls where the throttle percentage (0-100%) controls the ship's target velocity.

**What You'll Learn:**
- How to configure mousewheel input for throttle control in UE 5.6
- Both Enhanced Input System approach (recommended) and Legacy Input approach
- Blueprint and C++ implementation methods
- Troubleshooting common issues

**Prerequisites:**
- Unreal Engine 5.6 installed
- Adastrea project opened in editor
- Basic familiarity with Unreal Engine's input system

## Table of Contents

1. [Understanding the Throttle System](#understanding-the-throttle-system)
2. [Method 1: Enhanced Input System (Recommended)](#method-1-enhanced-input-system-recommended)
3. [Method 2: Legacy Input System](#method-2-legacy-input-system)
4. [Blueprint Implementation](#blueprint-implementation)
5. [C++ Implementation](#cpp-implementation)
6. [Testing Your Setup](#testing-your-setup)
7. [Customization Options](#customization-options)
8. [Troubleshooting](#troubleshooting)

---

## Understanding the Throttle System

### How Throttle Works in Adastrea

The throttle system in Adastrea uses a percentage-based model (0-100%) that controls the ship's target velocity:

- **Throttle 0%**: Ship comes to a complete stop
- **Throttle 50%**: Ship maintains half of maximum speed
- **Throttle 100%**: Ship maintains maximum speed

The flight computer automatically handles acceleration and deceleration to match the throttle setting, providing smooth and intuitive speed control.

### Default Configuration

- **ThrottlePercentage**: Current throttle (0-100%) - Read-only, displayed in HUD
- **ThrottleStep**: Increment/decrement amount per input (default: 10%)
- **Mouse Wheel Up**: Increase throttle by ThrottleStep
- **Mouse Wheel Down**: Decrease throttle by ThrottleStep

### Key Features

✅ Smooth acceleration/deceleration to target speed  
✅ Independent from strafe and vertical movement  
✅ Works with Flight Assist mode  
✅ Visual feedback via HUD display  
✅ Configurable increment size

---

## Method 1: Enhanced Input System (Recommended)

The Enhanced Input System is the modern approach used in Unreal Engine 5.6 and provides better flexibility, context switching, and customization.

### Step 1: Verify Enhanced Input Plugin

1. Open **Edit → Plugins**
2. Search for "Enhanced Input"
3. Ensure the plugin is **Enabled** (should already be enabled in Adastrea)
4. Restart the editor if you just enabled it

### Step 2: Locate or Create Input Actions

The Adastrea project already includes Input Actions for throttle control:

1. Open **Content Browser**
2. Navigate to **Content/Input/** folder
3. Look for these Input Actions:
   - `IA_ThrottleUp` (or create if missing)
   - `IA_ThrottleDown` (or create if missing)

**If Input Actions don't exist, create them:**

1. **Right-click in Content Browser → Input → Input Action**
2. **Name**: `IA_ThrottleUp`
3. **Open the asset** and configure:
   - **Value Type**: Digital (Bool)
   - **Consume Input**: Yes
   - **Triggers**: Pressed

4. Repeat for `IA_ThrottleDown`

### Step 3: Configure Input Mapping Context

The Input Mapping Context defines which keys/inputs trigger which Input Actions.

1. **Locate or Create Mapping Context:**
   - Navigate to **Content/Input/**
   - Find `IMC_Spaceship` (or create: **Right-click → Input → Input Mapping Context**)

2. **Open `IMC_Spaceship`**

3. **Add Mousewheel Mappings:**

   **For Throttle Up (Mouse Wheel Up):**
   - Click **+ (Add Mapping)** button
   - **Action**: Select `IA_ThrottleUp`
   - **Key**: Mouse Wheel Up (or search "Mouse Wheel Axis")
   - **Modifiers**: None
   - **Triggers**: Default (Pressed)

   **For Throttle Down (Mouse Wheel Down):**
   - Click **+ (Add Mapping)** button
   - **Action**: Select `IA_ThrottleDown`
   - **Key**: Mouse Wheel Down (or search "Mouse Wheel Axis")
   - **Modifiers**: None
   - **Triggers**: Default (Pressed)

4. **Save the Input Mapping Context**

### Step 4: Update Input Config Data Asset

The `InputConfigDataAsset` centralizes all input configuration:

1. **Navigate to Content/Input/**
2. **Find or Create** `DA_InputConfig_Default` (Blueprint based on `InputConfigDataAsset`)
3. **Open** the Data Asset
4. **Assign Input Actions:**
   - Locate **Movement** category
   - Set **Throttle Up Action** = `IA_ThrottleUp`
   - Set **Throttle Down Action** = `IA_ThrottleDown`
5. **Save the Data Asset**

### Step 5: Bind Input Actions in Spaceship Blueprint

Now we need to make the spaceship respond to these Input Actions.

**Option A: Blueprint Binding (Recommended for Designers)**

1. **Open your Spaceship Blueprint** (e.g., `BP_PlayerShip`)
2. **Go to Event Graph**
3. **Add Enhanced Input Action Events:**

   **For Throttle Up:**
   ```
   Event Graph:
   
   [IA_ThrottleUp (Enhanced Input Action)]
     ↓
   [Started]
     ↓
   [Throttle Up] (Call Function on Self)
   ```

   **For Throttle Down:**
   ```
   [IA_ThrottleDown (Enhanced Input Action)]
     ↓
   [Started]
     ↓
   [Throttle Down] (Call Function on Self)
   ```

4. **Compile and Save** the Blueprint

**Option B: C++ Binding (For Programmers)**

See [C++ Implementation](#cpp-implementation) section below.

### Step 6: Ensure Mapping Context is Added to Player Controller

The Input Mapping Context must be added to the Enhanced Input subsystem at runtime:

1. **Open your Player Controller Blueprint** (e.g., `BP_AdastreaPlayerController`)
2. **Find Event BeginPlay** (or create it)
3. **Add Mapping Context:**

   ```
   Event BeginPlay
     ↓
   [Get Enhanced Input Local Player Subsystem]
     Player Controller: Self
     ↓
   [Add Mapping Context]
     Mapping Context: IMC_Spaceship
     Priority: 0
     Options: (default)
   ```

4. **Compile and Save**

---

## Method 2: Legacy Input System

If you prefer to use the legacy input system (used in older Unreal Engine versions), follow these steps.

**Note:** The Enhanced Input System is recommended for UE 5.6, but legacy input is still supported for backward compatibility.

### Step 1: Open DefaultInput.ini

1. Navigate to **Config/DefaultInput.ini** in your project folder
2. Open in a text editor

### Step 2: Add Mousewheel Bindings

Add these lines under the `[/Script/Engine.InputSettings]` section:

```ini
[/Script/Engine.InputSettings]

; Throttle Control - Mousewheel
+ActionMappings=(ActionName="ThrottleUp",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MouseWheelUp)
+ActionMappings=(ActionName="ThrottleDown",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MouseWheelDown)

; Alternative: Use = and - keys as well
+ActionMappings=(ActionName="ThrottleUp",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Equals)
+ActionMappings=(ActionName="ThrottleDown",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Hyphen)
```

### Step 3: Bind in Spaceship Class

The `ASpaceship` class already includes legacy input bindings in `SetupPlayerInputComponent`:

```cpp
// In Spaceship.cpp - SetupPlayerInputComponent
PlayerInputComponent->BindAction(TEXT("ThrottleUp"), IE_Pressed, this, &ASpaceship::ThrottleUp);
PlayerInputComponent->BindAction(TEXT("ThrottleDown"), IE_Pressed, this, &ASpaceship::ThrottleDown);
```

This is already implemented, so no additional code is needed!

### Step 4: Restart Editor

- Close and reopen Unreal Editor for input changes to take effect

---

## Blueprint Implementation

### Complete Blueprint Example

Here's a complete Blueprint setup for throttle control using Enhanced Input:

#### In Spaceship Blueprint (BP_PlayerShip):

**Event Graph:**

```
┌─────────────────────────────────────────────────────────────┐
│  Enhanced Input Action: IA_ThrottleUp                        │
│    ↓                                                          │
│  [Started Trigger]                                            │
│    ↓                                                          │
│  [Get Throttle Percentage] (from Self)                       │
│    ↓                                                          │
│  [Add (Float + Float)]                                        │
│    A: Throttle Percentage                                     │
│    B: Throttle Step (Get from Self, default 10.0)            │
│    ↓                                                          │
│  [Clamp (Float)]                                              │
│    Value: Result of Add                                       │
│    Min: 0.0                                                   │
│    Max: 100.0                                                 │
│    ↓                                                          │
│  [Set Throttle Percentage] (on Self)                         │
│    Value: Clamped Result                                      │
│    ↓                                                          │
│  [Print String] (Optional - for debugging)                   │
│    String: "Throttle: {ThrottlePercentage}%"                 │
└─────────────────────────────────────────────────────────────┘
```

**Alternative: Call Built-in Function**

Since `ASpaceship` already has `ThrottleUp()` and `ThrottleDown()` functions:

```
┌─────────────────────────────────────────────────┐
│  Enhanced Input Action: IA_ThrottleUp           │
│    ↓                                             │
│  [Started]                                       │
│    ↓                                             │
│  [Throttle Up] (Function on Self)               │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│  Enhanced Input Action: IA_ThrottleDown         │
│    ↓                                             │
│  [Started]                                       │
│    ↓                                             │
│  [Throttle Down] (Function on Self)             │
└─────────────────────────────────────────────────┘
```

This is the simplest and recommended approach!

### Adding Visual Feedback

To display throttle percentage on screen:

```
┌─────────────────────────────────────────────────┐
│  Event Tick                                      │
│    ↓                                             │
│  [Get Throttle Percentage] (from Self)          │
│    ↓                                             │
│  [Format Text]                                   │
│    Format: "Throttle: {0}%"                      │
│    Value: Throttle Percentage                    │
│    ↓                                             │
│  [Set Text] (on HUD Text Block)                 │
└─────────────────────────────────────────────────┘
```

---

## C++ Implementation

For C++ programmers who want direct control over throttle input binding.

### Using Enhanced Input in C++

**In Spaceship.h:**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Spaceship.generated.h"

// Forward declarations
class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;

UCLASS()
class ADASTREA_API ASpaceship : public APawn
{
    GENERATED_BODY()

public:
    // Input Actions (assign in Blueprint or C++)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    UInputAction* ThrottleUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    UInputAction* ThrottleDownAction;

protected:
    // Input handlers
    void HandleThrottleUp(const FInputActionValue& Value);
    void HandleThrottleDown(const FInputActionValue& Value);

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
```

**In Spaceship.cpp:**

```cpp
#include "Spaceship.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/InputConfigDataAsset.h"

void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Cast to Enhanced Input Component
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind throttle actions
        if (ThrottleUpAction)
        {
            EnhancedInput->BindAction(
                ThrottleUpAction,
                ETriggerEvent::Started,
                this,
                &ASpaceship::HandleThrottleUp
            );
        }

        if (ThrottleDownAction)
        {
            EnhancedInput->BindAction(
                ThrottleDownAction,
                ETriggerEvent::Started,
                this,
                &ASpaceship::HandleThrottleDown
            );
        }
    }
}

void ASpaceship::HandleThrottleUp(const FInputActionValue& Value)
{
    // Call the existing ThrottleUp function
    ThrottleUp();
    
    // Optional: Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Throttle Up: %f%%"), ThrottlePercentage);
}

void ASpaceship::HandleThrottleDown(const FInputActionValue& Value)
{
    // Call the existing ThrottleDown function
    ThrottleDown();
    
    // Optional: Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Throttle Down: %f%%"), ThrottlePercentage);
}
```

### Alternative: Using InputConfigDataAsset in C++

**Load Input Actions from Data Asset:**

```cpp
void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Load input config
    UInputConfigDataAsset* InputConfig = LoadObject<UInputConfigDataAsset>(
        nullptr,
        TEXT("/Game/Input/DA_InputConfig_Default")
    );

    if (InputConfig && Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

        // Bind throttle up
        EnhancedInput->BindAction(
            InputConfig->ThrottleUpAction,
            ETriggerEvent::Started,
            this,
            &ASpaceship::ThrottleUp
        );

        // Bind throttle down
        EnhancedInput->BindAction(
            InputConfig->ThrottleDownAction,
            ETriggerEvent::Started,
            this,
            &ASpaceship::ThrottleDown
        );
    }
}
```

This approach uses the centralized `InputConfigDataAsset` for consistency.

---

## Testing Your Setup

### In-Editor Testing (PIE - Play In Editor)

1. **Open your test level** (or create a simple test map)
2. **Place your spaceship Blueprint** in the level
3. **Set the spaceship as the Default Pawn:**
   - Open **World Settings**
   - Under **Game Mode**, set **Default Pawn Class** to your spaceship Blueprint
4. **Press Play (Alt+P)**
5. **Test throttle control:**
   - Scroll **Mouse Wheel Up** → Throttle should increase
   - Scroll **Mouse Wheel Down** → Throttle should decrease
6. **Verify visual feedback** (if implemented):
   - Check HUD displays current throttle percentage
   - Ship speed should change accordingly

### Console Commands for Debugging

While in PIE, press **~ (tilde)** to open console and use these commands:

```
// Show Enhanced Input debug info
showdebug enhancedinput

// Enable input logging
log LogInput Verbose

// Display all active mapping contexts
```

### Verification Checklist

- [ ] Mouse wheel up increases throttle percentage
- [ ] Mouse wheel down decreases throttle percentage
- [ ] Throttle clamps to 0-100% range
- [ ] Ship speed changes match throttle setting
- [ ] No console errors or warnings
- [ ] Throttle persists when releasing mouse wheel
- [ ] Alternative keys (= and -) work if configured

---

## Customization Options

### Adjusting Throttle Increment Size

Change how much throttle changes per scroll:

**In Blueprint:**
1. Open spaceship Blueprint
2. Select the spaceship actor
3. In **Details Panel**, find **Flight Control → Throttle**
4. Change **Throttle Step** value:
   - `5.0` = Small increments (more precise)
   - `10.0` = Default (balanced)
   - `25.0` = Large increments (faster changes)

**In C++:**
```cpp
// In Spaceship constructor
ThrottleStep = 5.0f;  // 5% per scroll
```

### Adding Gamepad Support

To add gamepad support for throttle:

1. **Open `IMC_Spaceship`**
2. **Add gamepad mappings:**
   - **D-Pad Up** → `IA_ThrottleUp`
   - **D-Pad Down** → `IA_ThrottleDown`
   - Or use **Right Trigger** and **Left Trigger** for analog throttle

### Inverting Mousewheel Direction

If you prefer mouse wheel up to decrease throttle:

**In Input Mapping Context:**
1. Open `IMC_Spaceship`
2. Swap the key assignments:
   - **Mouse Wheel Up** → `IA_ThrottleDown`
   - **Mouse Wheel Down** → `IA_ThrottleUp`

### Setting Max Throttle Instantly

Add a "full throttle" button:

**New Input Action: IA_FullThrottle**

**Blueprint:**
```
[IA_FullThrottle (Enhanced Input Action)]
  ↓
[Started]
  ↓
[Set Throttle] (Function on Self)
  Percentage: 100.0
```

**Add to IMC_Spaceship:**
- **Space Bar (Hold)** → `IA_FullThrottle`

### Smooth Throttle Interpolation

For smoother throttle changes, modify the update function:

**C++ (in UpdateThrottleVelocity):**
```cpp
// Smooth throttle interpolation
float TargetThrottle = ThrottlePercentage / 100.0f;
float CurrentThrottle = CurrentVelocity.Size() / MovementComponent->MaxSpeed;
float SmoothedThrottle = FMath::FInterpTo(CurrentThrottle, TargetThrottle, DeltaTime, 2.0f);

// Apply smoothed throttle
// ... rest of throttle velocity code
```

---

## Troubleshooting

### Problem: Mousewheel doesn't change throttle

**Solution:**

1. **Check Input Mapping Context is added:**
   - Verify Player Controller adds `IMC_Spaceship` in BeginPlay
   - Use console command `showdebug enhancedinput` to see active contexts

2. **Verify Input Actions are assigned:**
   - Check `DA_InputConfig_Default` has `ThrottleUpAction` and `ThrottleDownAction` set
   - Ensure Input Actions exist in Content/Input/ folder

3. **Check Blueprint bindings:**
   - Open spaceship Blueprint Event Graph
   - Verify `IA_ThrottleUp` and `IA_ThrottleDown` events exist
   - Check they call `ThrottleUp` and `ThrottleDown` functions

4. **Confirm Enhanced Input plugin is enabled:**
   - Edit → Plugins → Search "Enhanced Input" → Should be enabled
   - Restart editor if you just enabled it

5. **Test with keyboard alternative:**
   - Try `=` and `-` keys if configured
   - If keyboard works but mousewheel doesn't, issue is specific to mousewheel mapping

### Problem: Throttle changes too much/too little per scroll

**Solution:**

- Adjust **ThrottleStep** in spaceship Blueprint
- Default is 10%, try 5% for finer control or 20% for coarser control

### Problem: Throttle resets to 0% unexpectedly

**Solution:**

1. **Check for conflicting input:**
   - Another action might be resetting throttle
   - Look for `SetThrottle(0)` calls in Blueprint

2. **Verify Flight Assist is enabled:**
   - Throttle system only works when Flight Assist is ON
   - Press `V` key to toggle Flight Assist (default binding)

3. **Check travel mode is not interfering:**
   - Travel mode automatically sets throttle to 100%
   - Press `J` key to toggle Travel Mode (default binding)

### Problem: Ship doesn't move when throttle increases

**Solution:**

1. **Verify Movement Component exists:**
   - Check spaceship has `UFloatingPawnMovement` component
   - Ensure it's not disabled

2. **Check MaxSpeed is not zero:**
   - Default should be 3000.0
   - Check spaceship's **DefaultMaxSpeed** property

3. **Ensure ship is possessed by player:**
   - Ship must be controlled by PlayerController
   - Check Default Pawn Class is set correctly

### Problem: Console shows "Input Action not found" error

**Solution:**

1. **Input Action asset is missing:**
   - Create `IA_ThrottleUp` and `IA_ThrottleDown` in Content/Input/
   - Follow steps in [Method 1: Step 2](#step-2-locate-or-create-input-actions)

2. **Input Action reference is broken:**
   - Re-assign Input Actions in `DA_InputConfig_Default`
   - Check asset paths are correct

### Problem: Mousewheel scrolls UI instead of controlling throttle

**Solution:**

1. **UI is capturing input:**
   - Ensure spaceship input has higher priority
   - In Player Controller, set `IMC_Spaceship` priority to higher value (e.g., 1 or 2)

2. **Set UI Input Mode correctly:**
   - When UI is open, switch to UI-only input mode
   - When closing UI, restore Game-only input mode

---

## Advanced Topics

### Multiple Input Contexts

Switch between different input contexts based on game state:

**Example: Combat Mode vs. Exploration Mode**

```cpp
// In Player Controller
void SwitchToCombatMode()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Remove exploration context
        Subsystem->RemoveMappingContext(ExplorationContext);
        
        // Add combat context (might have different throttle sensitivity)
        Subsystem->AddMappingContext(CombatContext, 1);
    }
}
```

### Analog Throttle (Gamepad Triggers)

For gamepad triggers providing analog input:

**Create IA_ThrottleAxis (Axis1D Input Action)**

**Bind to Right Trigger:**
- Value range: 0.0 to 1.0
- Maps directly to 0% to 100% throttle

**Blueprint:**
```
[IA_ThrottleAxis (Enhanced Input Action)]
  ↓
[Triggered]
  ↓
[Get Action Value (Axis 1D)]
  ↓
[Multiply by 100] (convert to percentage)
  ↓
[Set Throttle] (Function on Self)
```

### Combining with Auto-Throttle/Cruise Control

Create a "cruise control" feature:

```
[IA_SetCruiseControl (Enhanced Input Action)]
  ↓
[Started]
  ↓
[Branch: Is Cruise Control Active?]
    True → [Set Throttle] 0.0 → [Set Cruise Control Active] False
    False → [Set Cruise Control Active] True (remembers current throttle)
```

When active, throttle inputs are ignored and throttle is locked.

---

## See Also

### Related Adastrea Guides

- **[X4FlightControlsGuide.md](X4FlightControlsGuide.md)** - Complete X4-style flight control documentation
- **[EnhancedInputImplementation.md](EnhancedInputImplementation.md)** - Enhanced Input System setup guide
- **[ShipControlGuide.md](ShipControlGuide.md)** - Ship interior control system
- **[DA_InputConfig Visual Guide](../DA_INPUTCONFIG_VISUAL_GUIDE.md)** - Visual setup guide for Input Config Data Asset
- **[Input System Clarification](../INPUT_SYSTEM_CLARIFICATION.md)** - Quick clarification on Input Actions vs Blueprint graphs
- **[X4FlightControlsQuickReference.md](X4FlightControlsQuickReference.md)** - Quick reference for flight controls
- **[Player Controls Workflow](../Docs/PLAYER_CONTROLS_WORKFLOW.md)** - Complete player controls workflow

### Source Code Reference

- **[InputConfigDataAsset.h](../Source/Adastrea/Public/Input/InputConfigDataAsset.h)** - Input configuration class
- **[Spaceship.h](../Source/Adastrea/Ships/Spaceship.h)** - Spaceship class with throttle implementation
- **[Spaceship.cpp](../Source/Adastrea/Ships/Spaceship.cpp)** - Throttle function implementations

### External Resources

- [Unreal Engine 5.6 Enhanced Input Documentation](https://docs.unrealengine.com/5.6/en-US/enhanced-input-in-unreal-engine/)
- [Input Action Asset Reference](https://docs.unrealengine.com/5.6/en-US/input-actions-in-unreal-engine/)
- [Input Mapping Context Reference](https://docs.unrealengine.com/5.6/en-US/input-mapping-contexts-in-unreal-engine/)

---

## Summary

You've learned how to configure mousewheel scroll for throttle control in Unreal Engine 5.6. Key takeaways:

✅ **Enhanced Input System** is the recommended approach for UE 5.6  
✅ **Mouse Wheel Up/Down** increases/decreases throttle percentage  
✅ **ThrottleStep** controls how much throttle changes per scroll  
✅ **Blueprint or C++** implementation both work seamlessly  
✅ **InputConfigDataAsset** centralizes all input configuration  

The throttle system integrates with the X4-style flight controls, providing smooth and intuitive speed management for space flight gameplay.

---

**Last Updated**: 2025-12-05  
**Unreal Engine Version**: 5.6  
**Status**: Complete  
**Author**: Adastrea Development Team
