# HUD System Guide

> **Complete guide to implementing and customizing the Heads-Up Display (HUD) system in Adastrea**

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Core Classes](#core-classes)
- [Creating a Custom HUD](#creating-a-custom-hud)
- [HUD Components](#hud-components)
- [Blueprint Integration](#blueprint-integration)
- [Examples](#examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

---

## Overview

The HUD system provides a flexible, data-driven interface for displaying critical game information to players during gameplay. The system is designed to be:

- **Blueprint-Friendly**: Create custom HUDs entirely in Blueprint
- **Modular**: Individual HUD elements can be updated independently
- **Performance-Conscious**: Auto-updates only when needed
- **Extensible**: Easy to add new display elements

### Key Features

- Health and shield status indicators
- Speed and navigation displays
- Target information and combat data
- Alert/message system
- Weapon status tracking
- Automatic updates from game state
- Customizable visibility and layout

---

## Quick Start

### 1. Create a HUD Widget Blueprint

1. In the Content Browser, navigate to `Content/UI/`
2. Right-click and select **User Interface > Widget Blueprint**
3. Set the parent class to **AdastreaHUDWidget**
4. Name it `WBP_MainHUD`

### 2. Design Your HUD Layout

1. Open `WBP_MainHUD`
2. Add UI elements for:
   - Health bar (Progress Bar)
   - Shield bar (Progress Bar)
   - Speed display (Text Block)
   - Target information panel
   - Alert message area

### 3. Bind to C++ Functions

In the Widget Blueprint designer:

1. **Health Bar**:
   - Bind Percent to `HealthPercent` variable
   
2. **Shield Bar**:
   - Bind Percent to `ShieldPercent` variable

3. **Speed Display**:
   - Bind Text to a function that formats `CurrentSpeedValue`

### 4. Add to Player Controller

In your Player Controller Blueprint:

```
Event BeginPlay
  → Create Widget (WBP_MainHUD)
  → Add to Viewport
  → Call InitializeHUD
```

---

## Core Classes

### UAdastreaHUDWidget

**Location**: `Source/Adastrea/UI/AdastreaHUDWidget.h`

Base class for all HUD widgets. Provides core functionality and Blueprint extension points.

**Key Properties**:
- `HealthPercent` - Current health as percentage (0.0-1.0)
- `ShieldPercent` - Current shield as percentage (0.0-1.0)
- `CurrentSpeedValue` - Current speed value
- `bHasTarget` - Whether a target is selected
- `ControlledSpaceship` - Reference to player's ship

**Key Functions**:

#### InitializeHUD()
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD")
void InitializeHUD();
```
Called when HUD is first created. Override in Blueprint to set up initial state.

#### UpdateHealth(CurrentHealth, MaxHealth)
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Status")
void UpdateHealth(float CurrentHealth, float MaxHealth);
```
Updates the health display. Automatically calculates percentage.

#### UpdateShield(CurrentShield, MaxShield)
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Status")
void UpdateShield(float CurrentShield, float MaxShield);
```
Updates the shield display. Automatically calculates percentage.

#### UpdateSpeed(CurrentSpeed, MaxSpeed)
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Navigation")
void UpdateSpeed(float CurrentSpeed, float MaxSpeed);
```
Updates the speed display with current velocity.

#### UpdateTargetInfo(TargetName, TargetDistance, bIsHostile)
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Targeting")
void UpdateTargetInfo(const FText& TargetName, float TargetDistance, bool bIsHostile);
```
Updates target information display.

#### ShowAlert(Message, Duration, bIsWarning)
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Messages")
void ShowAlert(const FText& Message, float Duration, bool bIsWarning);
```
Shows an alert message on the HUD.

---

## Creating a Custom HUD

### Step-by-Step: Complete HUD Implementation

#### 1. Create the Widget Blueprint

Create a new Widget Blueprint inheriting from `UAdastreaHUDWidget`.

#### 2. Add UI Elements

Add these essential elements to your canvas:

**Health Display**:
```
Canvas Panel
  → Progress Bar (Name: HealthBar)
    - Size: 200x20
    - Fill Color: Green → Red gradient
    - Anchor: Bottom Left
```

**Shield Display**:
```
Canvas Panel
  → Progress Bar (Name: ShieldBar)
    - Size: 200x20
    - Fill Color: Blue
    - Anchor: Bottom Left (above health)
```

**Speed Display**:
```
Canvas Panel
  → Text Block (Name: SpeedText)
    - Size: 150x30
    - Font Size: 24
    - Anchor: Bottom Center
```

**Target Panel**:
```
Canvas Panel
  → Border (Name: TargetPanel)
    → Vertical Box
      → Text Block (Name: TargetNameText)
      → Text Block (Name: TargetDistanceText)
```

**Alert Messages**:
```
Canvas Panel
  → Vertical Box (Name: AlertContainer)
    - Anchor: Top Center
```

#### 3. Implement Blueprint Logic

**Override InitializeHUD**:
```
Event InitializeHUD
  → Set Health Bar Percent = 1.0
  → Set Shield Bar Percent = 1.0
  → Set Target Panel Visibility = Hidden
```

**Override UpdateHealth**:
```
Event UpdateHealth (CurrentHealth, MaxHealth)
  → Set Health Bar Percent = HealthPercent
  → (Optional) Change color based on health level
```

**Override UpdateTargetInfo**:
```
Event UpdateTargetInfo (TargetName, TargetDistance, bIsHostile)
  → Set Target Panel Visibility = Visible
  → Set Target Name Text = TargetName
  → Set Target Distance Text = Format("{0} m", TargetDistance)
  → Set Text Color = (bIsHostile ? Red : White)
```

**Override ShowAlert**:
```
Event ShowAlert (Message, Duration, bIsWarning)
  → Create Text Block
  → Add to Alert Container
  → Set Text = Message
  → Set Color = (bIsWarning ? Yellow : White)
  → If Duration > 0:
    → Set Timer (Duration)
      → Remove Text Block
```

#### 4. Auto-Update Implementation

**Override UpdateHUDFromGameState**:
```
Event UpdateHUDFromGameState (DeltaTime)
  → Get Controlled Spaceship
  → If Valid:
    → Get Health from Spaceship
    → Call UpdateHealth
    → Get Velocity from Spaceship
    → Call UpdateSpeed
```

---

## HUD Components

### Health System

Display player ship health with visual feedback.

**Features**:
- Percentage-based display
- Color coding (green → yellow → red)
- Damage flash effects
- Low health warnings

**Implementation Example**:
```
Health Bar Material:
  - Base Color: Lerp(Red, Green, HealthPercent)
  - Emissive: Low health pulse effect
```

### Shield System

Display shield status with recharge indication.

**Features**:
- Current/max shield display
- Recharge rate indicator
- Shield down warning
- Hit effect feedback

### Target System

Display information about targeted objects.

**Features**:
- Target name and type
- Distance to target
- Hostility indicator
- Target health (if applicable)

### Navigation

Display movement and positioning information.

**Features**:
- Current speed
- Maximum speed capability
- Direction indicator
- Waypoint markers

### Alert System

Display important messages and notifications.

**Features**:
- Timed messages
- Warning/error classification
- Message queue
- Priority system

---

## Blueprint Integration

### Connecting to Game Systems

#### From Spaceship

```
In Spaceship Blueprint:
  Event TakeDamage
    → Get Player Controller
    → Get HUD Widget
    → Call UpdateHealth(CurrentHealth, MaxHealth)
```

#### From Combat System

```
In Combat Manager:
  On Target Acquired
    → Get Player HUD
    → Call UpdateTargetInfo(Target Name, Distance, Is Hostile)
  
  On Target Lost
    → Get Player HUD
    → Call ClearTargetInfo()
```

#### From Alert System

```
In Game System:
  On Important Event
    → Get Player HUD
    → Call ShowAlert("Shields down!", 3.0, true)
```

### Custom HUD Elements

Add custom elements by extending the base class:

```
In Custom HUD Blueprint:
  
  // Add custom property
  Variable: CurrentFuel (Float)
  
  // Add custom function
  Function: UpdateFuel(NewFuel)
    → Set CurrentFuel = NewFuel
    → Update Fuel Bar widget
  
  // Auto-update custom elements
  Override UpdateHUDFromGameState
    → Parent: UpdateHUDFromGameState
    → Get Ship Fuel Level
    → Call UpdateFuel
```

---

## Examples

### Example 1: Minimalist HUD

A clean, minimal HUD with essential information:

```
Features:
- Small health/shield bars in corner
- Speed as numeric value
- Target crosshair only (details on hover)
- No permanent UI clutter

Layout:
  Bottom Left: Health + Shield bars (150x15 each)
  Bottom Center: Speed (100x20)
  Center: Dynamic crosshair with target reticle
```

### Example 2: Tactical HUD

Information-dense HUD for combat scenarios:

```
Features:
- Detailed health/shield with numeric values
- Weapon status for all hardpoints
- Target panel with full details
- Radar/minimap
- Active effects display

Layout:
  Top: Alert messages
  Left: Ship status (health, shields, power)
  Right: Target information
  Bottom: Weapon status bar
  Center: Crosshair with range markers
```

### Example 3: Exploration HUD

Focus on navigation and discovery:

```
Features:
- Compass rose
- Waypoint markers
- Sector information
- Discovery notifications
- Minimal combat UI

Layout:
  Top Center: Compass + Location
  Top Right: Objective markers
  Bottom: Simplified health/shield
  Right: Discovery log
```

---

## Best Practices

### Performance

1. **Update Only When Needed**:
   ```
   Don't update every frame if values haven't changed
   Use change detection before updating UI
   ```

2. **Batch Updates**:
   ```
   Update multiple elements in one call
   Avoid individual widget updates
   ```

3. **Optimize Visibility**:
   ```
   Collapse unused HUD elements
   Don't render hidden widgets
   ```

### User Experience

1. **Clear Visual Hierarchy**:
   - Critical info: Prominent and always visible
   - Secondary info: Accessible but less prominent
   - Contextual info: Show only when relevant

2. **Color Coding**:
   - Red: Danger, low health, hostile
   - Yellow/Orange: Warning, caution
   - Green: Good status, friendly
   - Blue: Shields, energy, utility

3. **Feedback**:
   - Provide visual feedback for player actions
   - Use animations for state changes
   - Flash or pulse for urgent information

### Accessibility

1. **Configurable Opacity**:
   ```
   Add HUD opacity slider in settings
   Allow players to adjust visibility
   ```

2. **Colorblind Modes**:
   ```
   Provide alternative color schemes
   Use shapes/icons in addition to colors
   ```

3. **Scalable UI**:
   ```
   Support different resolutions
   Allow UI scaling options
   ```

---

## Troubleshooting

### HUD Not Appearing

**Symptom**: HUD widget doesn't show up

**Solutions**:
1. Check widget is added to viewport:
   ```
   In Player Controller:
   Create Widget → Add to Viewport
   ```

2. Verify Z-order:
   ```
   Ensure HUD is on top layer
   Check other UI elements aren't covering it
   ```

3. Check visibility:
   ```
   Ensure widget visibility is set to Visible
   Check parent containers aren't collapsed
   ```

### Values Not Updating

**Symptom**: Health/shields show 0% or don't update

**Solutions**:
1. Verify spaceship reference:
   ```
   Check GetControlledSpaceship returns valid reference
   Debug print the spaceship pointer
   ```

2. Check update calls:
   ```
   Ensure UpdateHealth/UpdateShield are being called
   Add debug prints in update functions
   ```

3. Verify binding:
   ```
   Check progress bar bindings are correct
   Ensure variables are marked BlueprintReadOnly
   ```

### Performance Issues

**Symptom**: FPS drops when HUD is visible

**Solutions**:
1. Reduce update frequency:
   ```
   Don't update every tick
   Use timers for periodic updates
   ```

2. Optimize widgets:
   ```
   Collapse unused elements
   Use simpler materials
   Reduce text updates
   ```

3. Profile widget performance:
   ```
   Use Unreal's Widget Reflector
   Identify expensive widgets
   Optimize or simplify
   ```

---

## Additional Resources

- **Blueprint Creation Guide**: See `BLUEPRINT_CREATION_GUIDE.md`
- **UI Best Practices**: See `CONTENT_CREATION_QUICKSTART.md`
- **Spaceship System**: See `Assets/SpaceshipSetupGuide.md`
- **Combat System**: See `Assets/CombatSystemGuide.md`

---

**Last Updated**: November 11, 2025  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
