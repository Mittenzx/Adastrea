# Spaceship HUD (WBP_Spaceship) - Blueprint Implementation Guide

## Overview

This guide provides comprehensive instructions for creating the `WBP_Spaceship` HUD Widget Blueprint that displays essential flight information, targeting, and ship status during gameplay.

## What This HUD Displays

The spaceship HUD provides real-time flight information:

### Core HUD Elements
1. **Crosshair** - Weapon aim indicator (center screen)
2. **Speed & Throttle** - Current velocity and throttle percentage
3. **Health & Hull Integrity** - Ship health and structural integrity
4. **Targeting Information** - Current target details and distance
5. **Ship Status** - Basic ship info and alerts

### MVP Focus

This guide focuses on **MVP-critical features** for the Trade Simulator:
- ✅ **Speed/Throttle display** - Essential for navigation
- ✅ **Hull integrity** - Ship health (trading hazards)
- ✅ **Basic crosshair** - Aiming reference
- ⚠️ **Targeting** - Basic target info (station docking)
- ❌ **Combat features** - Deferred to post-MVP (no combat in MVP)

## Prerequisites

Before creating the HUD, ensure you have:
- ✅ Unreal Engine 5.6+ project with Adastrea source code
- ✅ Basic UMG (Unreal Motion Graphics) knowledge
- ✅ Understanding of Blueprint visual scripting
- ✅ Compiled Adastrea C++ code (includes `UAdastreaHUDWidget`)

### Required C++ Classes

The following C++ classes provide HUD functionality:
- `UAdastreaHUDWidget` - Base HUD widget class
- `ASpaceship` - Ship actor with health, speed, integrity
- `USpaceshipControlsComponent` - Control input and speed management
- `USpaceshipDataAsset` - Ship configuration data

## Step 1: Create the Widget Blueprint

### 1.1 Create Widget Blueprint

1. **Navigate to Content Browser**
   - Recommended location: `Content/UI/HUD/`
   - Create the folder structure if it doesn't exist

2. **Create Widget Blueprint**
   - Right-click in Content Browser
   - Select **User Interface → Widget Blueprint**
   - In "Pick Parent Class" dialog, search for **AdastreaHUDWidget**
   - Select `AdastreaHUDWidget` as parent class
   - Name it: `WBP_Spaceship`

### 1.2 Why Use AdastreaHUDWidget as Parent?

The `UAdastreaHUDWidget` C++ class provides:
- ✅ Pre-built C++ functions for updating HUD elements
- ✅ Automatic integration with player controller
- ✅ Performance-optimized update logic
- ✅ Blueprint-overridable events for customization
- ✅ Built-in crosshair position tracking

## Step 2: Design the HUD Layout

### 2.1 Recommended Layout Structure

Open `WBP_Spaceship` in the Widget Designer and create this hierarchy:

```
Canvas Panel (Root)
├── Crosshair_Container (Canvas Panel)
│   └── Crosshair_Image (Image) - Center screen reticle
│
├── TopLeft_Container (Vertical Box)
│   ├── ShipName_Text (Text Block)
│   ├── HullIntegrity_Container (Horizontal Box)
│   │   ├── HullIntegrity_Label (Text Block) "Hull:"
│   │   ├── HullIntegrity_Bar (Progress Bar)
│   │   └── HullIntegrity_Value (Text Block) "100%"
│   └── Shield_Container (Horizontal Box)
│       ├── Shield_Label (Text Block) "Shield:"
│       ├── Shield_Bar (Progress Bar)
│       └── Shield_Value (Text Block) "100%"
│
├── TopRight_Container (Vertical Box)
│   ├── Speed_Container (Horizontal Box)
│   │   ├── Speed_Label (Text Block) "Speed:"
│   │   └── Speed_Value (Text Block) "0 m/s"
│   ├── Throttle_Container (Horizontal Box)
│   │   ├── Throttle_Label (Text Block) "Throttle:"
│   │   ├── Throttle_Bar (Progress Bar)
│   │   └── Throttle_Value (Text Block) "0%"
│   └── MaxSpeed_Container (Horizontal Box)
│       ├── MaxSpeed_Label (Text Block) "Max:"
│       └── MaxSpeed_Value (Text Block) "1000 m/s"
│
├── Center_Container (Vertical Box)
│   └── Target_Container (Vertical Box)
│       ├── TargetName_Text (Text Block)
│       ├── TargetDistance_Text (Text Block)
│       └── TargetStatus_Text (Text Block)
│
└── BottomCenter_Container (Horizontal Box)
    ├── Alert_Text (Text Block) - For messages/warnings
    └── Credits_Text (Text Block) - Player credits (optional)
```

### 2.2 Layout Positioning Guidelines

#### Crosshair (Center Screen)
```
Widget: Crosshair_Image
Type: Image
Anchors: Center (0.5, 0.5)
Position: (0, 0)
Alignment: (0.5, 0.5)
Size: 32x32 pixels (or desired size)
Z-Order: High (render on top)
```

#### Top Left (Ship Status)
```
Widget: TopLeft_Container
Type: Vertical Box
Anchors: Top-Left (0, 0)
Position: (20, 20) - Offset from top-left corner
Alignment: (0, 0)
```

#### Top Right (Speed & Throttle)
```
Widget: TopRight_Container
Type: Vertical Box
Anchors: Top-Right (1, 0)
Position: (-20, 20) - Offset from top-right corner
Alignment: (1, 0) - Right-aligned
```

#### Center (Target Info)
```
Widget: Center_Container
Type: Vertical Box
Anchors: Top-Center (0.5, 0)
Position: (0, 150) - Below crosshair
Alignment: (0.5, 0) - Center-aligned
```

#### Bottom Center (Alerts)
```
Widget: BottomCenter_Container
Type: Horizontal Box
Anchors: Bottom-Center (0.5, 1)
Position: (0, -50) - Above bottom edge
Alignment: (0.5, 1) - Center-aligned
```

### 2.3 Styling Recommendations

#### Color Scheme (Sci-Fi HUD)
- **Primary Text**: Cyan/Teal `(0, 1, 1, 1)` or Light Blue `(0.5, 0.8, 1, 1)`
- **Warning Text**: Yellow/Orange `(1, 0.8, 0, 1)`
- **Critical Text**: Red `(1, 0, 0, 1)`
- **Background**: Dark with transparency `(0, 0, 0, 0.7)`
- **Progress Bars**:
  - Health (Green): `(0, 1, 0, 1)`
  - Shield (Blue): `(0, 0.5, 1, 1)`
  - Throttle (Cyan): `(0, 1, 1, 1)`

#### Font Settings
- **Font Size**: 14-18pt for labels, 20-24pt for values
- **Font**: Monospace or sci-fi style (e.g., "Roboto Mono", "Orbitron")
- **Shadow**: Enable with offset (1, 1) for readability

#### Progress Bar Settings
```
Width: 150-200 pixels
Height: 20 pixels
Fill Color: Use color scheme above
Background Color: Dark gray (0.1, 0.1, 0.1, 0.8)
Style: Flat or with subtle gradient
```

## Step 3: Make Widgets Variables

For each widget you want to update from Blueprint/C++:

1. **Select the widget** in the Designer hierarchy
2. **Check "Is Variable"** in the Details panel
3. **Name it descriptively**

### Required Widget Variables

Make these widgets variables (check "Is Variable"):

**Health/Integrity:**
- `HullIntegrity_Bar` (Progress Bar)
- `HullIntegrity_Value` (Text Block)
- `Shield_Bar` (Progress Bar)
- `Shield_Value` (Text Block)

**Speed/Throttle:**
- `Speed_Value` (Text Block)
- `Throttle_Bar` (Progress Bar)
- `Throttle_Value` (Text Block)
- `MaxSpeed_Value` (Text Block)

**Targeting:**
- `TargetName_Text` (Text Block)
- `TargetDistance_Text` (Text Block)
- `TargetStatus_Text` (Text Block)
- `Target_Container` (Vertical Box) - For show/hide

**Crosshair:**
- `Crosshair_Image` (Image)

**Alerts:**
- `Alert_Text` (Text Block)

**Ship Info:**
- `ShipName_Text` (Text Block)

## Step 4: Implement Blueprint Logic

### 4.1 Override Parent Class Events

The `UAdastreaHUDWidget` C++ class provides BlueprintNativeEvent functions that you should override:

#### Event Graph Overview

```
Event Construct
  ↓
[Initialize HUD]
  ↓
[Hide target info initially]

Event Tick
  ↓
[Update crosshair position]
  ↓
[Update speed/throttle from spaceship]
  ↓
[Update health/integrity from spaceship]
```

### 4.2 Event Construct (Initialization)

Override the `Event Construct` to initialize the HUD:

```blueprint
Event Construct
  ↓
Call: Initialize HUD (Parent function)
  ↓
Set: Target_Container → Visibility = Collapsed
  ↓
Set: Alert_Text → Text = "" (empty)
```

**Implementation Steps:**
1. In the **Graph** tab, find or create `Event Construct`
2. Right-click → Search "Initialize HUD"
3. Call the parent implementation
4. Set initial widget visibility states

### 4.3 Update Health Display

Override `UpdateHealth` to display hull integrity:

```blueprint
Event Update Health (Current Health, Max Health)
  ↓
Branch: Max Health > 0
  True:
    ├─> Divide: Current Health / Max Health → Health Percent
    ├─> Set: HullIntegrity_Bar → Percent = Health Percent
    ├─> Format Text: "{0} / {1}" (Current Health, Max Health)
    └─> Set: HullIntegrity_Value → Text = Formatted Text
  False:
    └─> Set: HullIntegrity_Value → Text = "N/A"
```

**Implementation Steps:**
1. In **Graph** tab, right-click → Search "Update Health"
2. Select **Override 'Update Health'**
3. Implement the logic above using Blueprint nodes:
   - **Divide** node for percentage calculation
   - **Set Percent** on Progress Bar
   - **Format Text** for displaying values
   - **Set Text** on Text Block

**Detailed Node Connections:**
```
[Event Update Health]
├─ Current Health (float) → [Divide] Input A
├─ Max Health (float) → [Divide] Input B
│
[Divide] Output
├─> [HullIntegrity_Bar] → Set Percent
│
[Format Text] "{0:0} / {1:0}"
├─ Argument 0: Current Health
├─ Argument 1: Max Health
└─> [HullIntegrity_Value] → Set Text
```

### 4.4 Update Shield Display

Override `UpdateShield` similarly:

```blueprint
Event Update Shield (Current Shield, Max Shield)
  ↓
Branch: Max Shield > 0
  True:
    ├─> Divide: Current Shield / Max Shield → Shield Percent
    ├─> Set: Shield_Bar → Percent = Shield Percent
    ├─> Format Text: "{0} / {1}" (Current Shield, Max Shield)
    └─> Set: Shield_Value → Text = Formatted Text
  False:
    ├─> Set: Shield_Bar → Visibility = Collapsed
    └─> Set: Shield_Value → Visibility = Collapsed
```

**Note:** If ship has no shields (Max Shield = 0), hide the shield display entirely.

### 4.5 Update Speed Display

Override `UpdateSpeed` to show current and max speed:

```blueprint
Event Update Speed (Current Speed, Max Speed)
  ↓
Format Text: "{0:0.0} m/s" (Current Speed)
  ↓
Set: Speed_Value → Text = Formatted Text
  ↓
Format Text: "{0:0.0} m/s" (Max Speed)
  ↓
Set: MaxSpeed_Value → Text = Formatted Text
```

**Implementation:**
```
[Event Update Speed]
├─ Current Speed (float)
│   ├─> [Format Text] "{0:0.0} m/s"
│   └─> [Speed_Value] → Set Text
│
└─ Max Speed (float)
    ├─> [Format Text] "{0:0.0} m/s"
    └─> [MaxSpeed_Value] → Set Text
```

### 4.6 Update Throttle Display (Custom Implementation)

Since throttle isn't in the base class, you'll implement this in Event Tick:

```blueprint
Event Tick (Delta Time)
  ↓
Get: Controlled Spaceship
  ↓
Branch: Is Valid?
  True:
    ├─> Get: SpaceshipControlsComponent
    ├─> Branch: Is Valid?
    │     True:
    │       ├─> Get: Current Speed (from component)
    │       ├─> Get: Max Speed (from component)
    │       ├─> Divide: Current Speed / Max Speed → Throttle Percent
    │       ├─> Multiply: Throttle Percent × 100 → Throttle Percentage
    │       ├─> Set: Throttle_Bar → Percent = Throttle Percent
    │       └─> Format Text: "{0:0}%" (Throttle Percentage)
    │           └─> Set: Throttle_Value → Text = Formatted Text
```

**Node Breakdown:**
```
[Event Tick]
  ↓
[Get Controlled Spaceship] (inherited function)
  ↓
[Is Valid?] (validation check)
  ↓
[Get Component By Class] (USpaceshipControlsComponent)
  ↓
[Get Current Speed] (from Controls Component)
  ↓
[Divide by Max Speed] → Throttle Percent (0-1)
  ↓
[Multiply by 100] → Throttle Percentage (0-100)
  ↓
[Set Percent] (Throttle_Bar) → Throttle Percent
  ↓
[Format Text] "{0:0}%" → Throttle Percentage
  ↓
[Set Text] (Throttle_Value)
```

### 4.7 Update Target Info

Override `UpdateTargetInfo` for targeting display:

```blueprint
Event Update Target Info (Target Name, Target Distance, Is Hostile)
  ↓
Set: TargetName_Text → Text = Target Name
  ↓
Format Text: "Distance: {0:0.0}m" (Target Distance)
  ↓
Set: TargetDistance_Text → Text = Formatted Text
  ↓
Branch: Is Hostile?
  True:
    ├─> Set: TargetStatus_Text → Text = "HOSTILE"
    ├─> Set: TargetStatus_Text → Color = Red (1, 0, 0, 1)
    └─> Set: Crosshair_Image → Color = Red
  False:
    ├─> Set: TargetStatus_Text → Text = "FRIENDLY"
    ├─> Set: TargetStatus_Text → Color = Green (0, 1, 0, 1)
    └─> Set: Crosshair_Image → Color = White
  ↓
Set: Target_Container → Visibility = Visible
```

Override `ClearTargetInfo` to hide targeting:

```blueprint
Event Clear Target Info
  ↓
Set: Target_Container → Visibility = Collapsed
  ↓
Set: Crosshair_Image → Color = White (default)
```

### 4.8 Crosshair Position Update

The crosshair position is handled automatically by the parent class, but you need to apply it to your widget:

```blueprint
Event Tick (continued from 4.6)
  ↓
Get: WeaponAimPosition (inherited from parent)
  ↓
Get: Owning Player → Get Viewport Size
  ↓
Break Vector2D: WeaponAimPosition
  ├─> X * Viewport Size X → Screen X
  └─> Y * Viewport Size Y → Screen Y
  ↓
Make Vector2D: (Screen X, Screen Y)
  ↓
Set: Crosshair_Image → Canvas Position = Screen Position
```

**Note:** See existing [Crosshair Blueprint Guide](CROSSHAIR_BLUEPRINT_GUIDE.md) for detailed crosshair implementation.

### 4.9 Show Alert Messages

Override `ShowAlert` for displaying warnings/messages:

```blueprint
Event Show Alert (Message, Duration, Is Warning)
  ↓
Set: Alert_Text → Text = Message
  ↓
Branch: Is Warning?
  True:
    └─> Set: Alert_Text → Color = Yellow/Orange
  False:
    └─> Set: Alert_Text → Color = White
  ↓
Set: Alert_Text → Visibility = Visible
  ↓
Branch: Duration > 0?
  True:
    └─> Set Timer: Duration seconds
        └─> On Complete: Set Alert_Text → Visibility = Collapsed
```

**Implementation Notes:**
- Use **Set Timer by Event** node for automatic hiding
- Create a custom event "Hide Alert" and call it from timer

## Step 5: Automatic HUD Updates

### 5.1 Override UpdateHUDFromGameState

The parent class provides `UpdateHUDFromGameState` that's called every frame. Override it to poll ship data:

```blueprint
Event Update HUD From Game State (Delta Time)
  ↓
Call: Parent → Update HUD From Game State (Delta Time)
  ↓
Get: Controlled Spaceship
  ↓
Branch: Is Valid?
  True:
    ├─> Get: Current Hull Integrity
    ├─> Get: Max Hull Integrity
    ├─> Call: Update Health (Current, Max)
    │
    ├─> Get: Shield Strength (from Data Asset or component)
    ├─> Get: Max Shield Strength
    ├─> Call: Update Shield (Current, Max)
    │
    ├─> Get: Velocity → Get Vector Length → Current Speed
    ├─> Get: Max Speed (from Data Asset)
    └─> Call: Update Speed (Current Speed, Max Speed)
```

**Performance Note:** This function runs every frame. Keep it lightweight. Consider updating less frequently using a timer if performance is a concern.

### 5.2 Optimization: Throttled Updates

For better performance, update HUD elements at different rates:

```blueprint
Event Tick (Delta Time)
  ↓
[Accumulator Pattern]
├─ Health Update Timer += Delta Time
├─ Speed Update Timer += Delta Time
└─ Target Update Timer += Delta Time

Branch: Health Update Timer > 0.1 (update every 100ms)
  True:
    ├─> Update Health Display
    └─> Reset: Health Update Timer = 0

Branch: Speed Update Timer > 0.05 (update every 50ms)
  True:
    ├─> Update Speed/Throttle Display
    └─> Reset: Speed Update Timer = 0

Branch: Target Update Timer > 0.2 (update every 200ms)
  True:
    ├─> Update Target Display
    └─> Reset: Target Update Timer = 0
```

## Step 6: Testing the HUD

### 6.1 Add HUD to Player Controller

1. **Open Player Controller Blueprint** (e.g., `BP_AdastreaPlayerController`)
2. Find **HUD Widget Class** property
3. Set it to your `WBP_Spaceship`
4. **Compile and Save**

### 6.2 Test Checklist

Create a test scenario and verify:

**Visual Appearance:**
- [ ] HUD appears when playing as spaceship
- [ ] All widgets are properly positioned
- [ ] Text is readable and styled correctly
- [ ] Progress bars display correctly
- [ ] Crosshair is visible and centered

**Health/Integrity:**
- [ ] Hull integrity bar fills correctly (0-100%)
- [ ] Hull values display accurately (current/max)
- [ ] Shield bar displays when ship has shields
- [ ] Shield values update correctly

**Speed/Throttle:**
- [ ] Current speed updates in real-time
- [ ] Max speed displays from ship data asset
- [ ] Throttle percentage shows correct value (0-100%)
- [ ] Throttle bar fills proportionally

**Targeting:**
- [ ] Target info appears when selecting target
- [ ] Target name displays correctly
- [ ] Distance to target updates
- [ ] Hostile/friendly status shows with correct color
- [ ] Crosshair changes color based on target
- [ ] Target info hides when clearing target

**Crosshair:**
- [ ] Crosshair follows mouse position
- [ ] Crosshair stays within screen bounds
- [ ] Crosshair color changes with target status

**Performance:**
- [ ] No frame rate drops
- [ ] Smooth updates (no stuttering)
- [ ] Memory usage stable

### 6.3 Debug Techniques

#### Print Current Values

Add **Print String** nodes to debug:

```blueprint
Get: Current Hull Integrity
  ↓
Print String: "Hull: {0}" (duration: 0.1s)
```

#### Visual Debug Overlays

Temporarily add visible backgrounds to containers to verify positioning:

```
Container → Background Color = (1, 0, 0, 0.3) [Red semi-transparent]
```

#### Log Output

Use **Print to Log** instead of screen prints:

```blueprint
Print String: "HUD Update: Speed = {0}", Log = True, Duration = 0
```

Check Output Log in Unreal Editor (Window → Developer Tools → Output Log).

## Step 7: Polish and Visual Effects

### 7.1 Add Animations

Create widget animations for polish:

#### Health Warning Animation (Low Health)

1. **Create Animation** in Animations tab: `Anim_HealthWarning`
2. **Add Track**: HullIntegrity_Bar → Color and Opacity
3. **Keyframes**:
   - 0.0s: Green (0, 1, 0, 1)
   - 0.5s: Red (1, 0, 0, 1)
   - 1.0s: Green (0, 1, 0, 1)
4. **Set Loop**: True

Play animation when health < 25%:

```blueprint
Event Update Health (Current Health, Max Health)
  ↓
Calculate: Health Percent = Current Health / Max Health
  ↓
Branch: Health Percent < 0.25?
  True:
    └─> Play Animation: Anim_HealthWarning
  False:
    └─> Stop Animation: Anim_HealthWarning
```

#### Alert Fade In/Out

1. **Create Animation**: `Anim_AlertFadeIn`
   - Opacity: 0 → 1 over 0.2s
2. **Create Animation**: `Anim_AlertFadeOut`
   - Opacity: 1 → 0 over 0.5s

```blueprint
Event Show Alert (Message, Duration, Is Warning)
  ↓
[Set text and visibility]
  ↓
Play Animation: Anim_AlertFadeIn
  ↓
Set Timer: Duration seconds
  ↓
On Complete:
  └─> Play Animation: Anim_AlertFadeOut
      └─> On Finished: Set Visibility = Collapsed
```

### 7.2 Add Sound Effects

Add audio feedback for events:

```blueprint
Event Update Health (Current Health, Max Health)
  ↓
[Calculate health percent]
  ↓
Branch: Health Percent < 0.25 AND Previous Health >= 0.25?
  True:
    └─> Play Sound 2D: Snd_HealthWarning
```

**Recommended Sound Events:**
- Low health warning
- Shield down warning
- Target acquired
- Target lost
- Speed limit reached

### 7.3 Advanced Crosshair Feedback

Dynamic crosshair based on game state:

```blueprint
Function: Update Crosshair State
  ↓
Get: Controlled Spaceship
  ↓
Get: Controls Component → Get Current Speed
  ↓
Branch: Current Speed > Max Speed * 0.9?
  True:
    └─> Set: Crosshair_Image → Tint = Yellow (warning)
  False:
    └─> Set: Crosshair_Image → Tint = White (normal)
  ↓
Branch: Has Target?
  True:
    ├─> Branch: Is Hostile?
    │     True: Red tint
    │     False: Green tint
```

### 7.4 Screen-Space Effects

Add post-process effects for immersion (optional):

- Vignette when health is low
- Screen shake when taking damage
- Color grading based on ship status

**Blueprint Implementation:**
```blueprint
Event Update Health (Current Health, Max Health)
  ↓
Calculate: Health Percent
  ↓
Branch: Health Percent < 0.25?
  True:
    └─> Get Player Camera Manager
        └─> Set Post Process Blend Weight = 0.5
```

## Advanced Customization

### Multiple HUD Modes

Implement HUD mode switching:

```blueprint
Enum: EHUDMode
  - Minimal (only crosshair and speed)
  - Standard (all elements)
  - Detailed (additional stats)
```

```blueprint
Function: Set HUD Mode (Mode)
  ↓
Switch on Mode:
  ├─ Minimal:
  │   └─> Hide all except crosshair and speed
  ├─ Standard:
  │   └─> Show core elements
  └─ Detailed:
      └─> Show all elements + extra stats
```

Bind to input (e.g., "H" key):

```
Input Action: IA_ToggleHUD
  ↓
Cycle through HUD modes
```

### HUD Scaling

Add HUD scale setting:

```blueprint
Function: Set HUD Scale (Scale)
  ↓
Set: Root Canvas → RenderTransform → Scale = (Scale, Scale)
```

Clamp scale: 0.5 to 2.0

### Color Blind Modes

Provide alternate color schemes:

```blueprint
Function: Set Color Blind Mode (Mode)
  ↓
Switch on Mode:
  ├─ Normal: Standard colors
  ├─ Deuteranopia: Green → Blue/Yellow
  ├─ Protanopia: Red → Blue/Brown
  └─ Tritanopia: Blue → Red/Green
```

## Integration with Game Systems

### Ship Status Widget Integration

Link to detailed ship status:

```blueprint
Input Action: IA_ShipStatus (e.g., "Tab" key)
  ↓
Get Player Controller → Cast to AdastreaPlayerController
  ↓
Toggle Ship Status (opens WBP_ShipStatus)
```

Add button to HUD for ship status (optional).

### Trading UI Integration

When docked at station:

```blueprint
Event: On Docked At Station
  ↓
Set: HUD Visibility = Hidden or Dimmed
  ↓
Show: Trading UI
```

When undocking:

```blueprint
Event: On Undocked
  ↓
Set: HUD Visibility = Visible
  ↓
Hide: Trading UI
```

### Save/Load Preferences

Save HUD preferences:

```blueprint
Function: Save HUD Settings
  ↓
Get: Game Instance → Get Save Game
  ↓
Set: HUDScale, HUDMode, ColorScheme
  ↓
Save Game to Slot
```

Load on startup:

```blueprint
Event Begin Play
  ↓
Load HUD Settings
  ↓
Apply: HUDScale, HUDMode, ColorScheme
```

## Troubleshooting

### HUD Not Appearing

**Check:**
- [ ] Player Controller has `WBP_Spaceship` set as HUD Widget Class
- [ ] Controlled pawn is a Spaceship (not another pawn type)
- [ ] Widget is added to viewport (automatic in `UAdastreaHUDWidget`)

**Debug:**
```blueprint
Event Begin Play
  ↓
Print: "HUD Created"
```

If not printing, HUD widget isn't being created.

### Values Not Updating

**Check:**
- [ ] `UpdateHUDFromGameState` is overridden and implemented
- [ ] `GetControlledSpaceship()` returns valid spaceship
- [ ] Ship has valid Data Asset assigned

**Debug:**
```blueprint
Event Update HUD From Game State
  ↓
Get: Controlled Spaceship
  ↓
Print: "Spaceship: {0}" (Is Valid?)
  ↓
Print: "Hull: {0} / {1}" (Current, Max)
```

### Crosshair Position Wrong

**Check:**
- [ ] Crosshair anchors are Center (0.5, 0.5)
- [ ] Crosshair alignment is (0.5, 0.5)
- [ ] Crosshair size is set correctly
- [ ] Position calculation uses viewport size

**Fix:**
```
Designer → Crosshair_Image:
Anchors: Min (0.5, 0.5), Max (0.5, 0.5)
Alignment: (0.5, 0.5)
```

### Performance Issues

**Optimize:**
- [ ] Use throttled updates (update less frequently)
- [ ] Avoid expensive operations in Event Tick
- [ ] Cache widget references (don't search each frame)
- [ ] Use Invalidation Boxes for static elements

**Profile:**
```
Console Command: stat fps
Console Command: stat ui
```

Check for high Widget Tick time.

### Text Not Visible

**Check:**
- [ ] Text color has alpha > 0 (not transparent)
- [ ] Font size is large enough (14-24pt recommended)
- [ ] Text is not behind other widgets (Z-order)
- [ ] Parent container is visible

**Fix:**
```
Text Block → Appearance → Color and Opacity → Alpha = 1.0
Text Block → Font → Size = 18
```

## Performance Best Practices

### Optimization Checklist

- [ ] Update health/shield every 100ms, not every frame
- [ ] Update speed/throttle every 50ms
- [ ] Update target info every 200ms
- [ ] Cache widget references in variables
- [ ] Use Invalidation Boxes for static elements
- [ ] Minimize Blueprint complexity in Event Tick
- [ ] Profile with `stat fps` and `stat ui`

### Memory Optimization

- [ ] Don't create widgets dynamically in Tick
- [ ] Reuse widgets (show/hide instead of create/destroy)
- [ ] Use texture atlases for HUD elements
- [ ] Compress textures appropriately (UI compression)

### Blueprint Performance

- [ ] Minimize nodes in frequently-called events
- [ ] Avoid string operations in Tick
- [ ] Cache expensive calculations
- [ ] Use **Select** nodes instead of branches when possible
- [ ] Collapse complex logic into functions

## Example HUD Configurations

### Minimal HUD (MVP Focus)

For Trade Simulator MVP, focus on essentials:

```
Elements:
✅ Crosshair (basic)
✅ Speed (current/max)
✅ Throttle (percentage bar)
✅ Hull integrity (bar + percentage)
❌ Shields (no shields in MVP)
❌ Weapons (no combat in MVP)
⚠️ Target info (station names only)
```

### Standard HUD (Post-MVP)

After MVP validation:

```
Elements:
✅ All MVP elements
✅ Shields
✅ Weapons/ammo
✅ Advanced targeting
✅ Radar/minimap
✅ Quest indicators
```

### Detailed HUD (Full Release)

Complete feature set:

```
Elements:
✅ All Standard elements
✅ System status (power, fuel, etc.)
✅ Crew status
✅ Faction relations
✅ Navigation waypoints
✅ Sensor contacts
```

## Reference Files

### C++ Class References

- **`UAdastreaHUDWidget`** - Base HUD class
  - Location: `Source/Adastrea/Public/UI/AdastreaHUDWidget.h`
  - Functions: `UpdateHealth`, `UpdateShield`, `UpdateSpeed`, `UpdateTargetInfo`, etc.

- **`ASpaceship`** - Ship actor
  - Location: `Source/Adastrea/Public/Ships/Spaceship.h`
  - Properties: `CurrentHullIntegrity`, `MaxHullIntegrity`, velocity, etc.

- **`USpaceshipControlsComponent`** - Controls component
  - Location: `Source/Adastrea/Public/Ships/SpaceshipControlsComponent.h`
  - Properties: `CurrentSpeed`, `MovementSpeed`, throttle settings

- **`USpaceshipDataAsset`** - Ship configuration
  - Location: `Source/Adastrea/Public/Ships/SpaceshipDataAsset.h`
  - Properties: All ship stats (max speed, health, shields, etc.)

### Documentation References

- [Crosshair Blueprint Guide](CROSSHAIR_BLUEPRINT_GUIDE.md) - Detailed crosshair implementation
- [Ship Status Widget Usage Guide](../../Source/Adastrea/Public/UI/ShipStatusWidget_Usage.md) - Detailed ship info display
- [Trading UI Blueprint Guide](../mvp/TRADING_UI_BLUEPRINT_GUIDE.md) - Trading interface implementation
- [Blueprint Implementation Guide](../mvp/BLUEPRINT_IMPLEMENTATION_GUIDE.md) - General Blueprint patterns

### Asset References

**Textures:**
- Crosshair: `Content/Textures/UI/T_Crosshair_*`
- Icons: `Content/Textures/UI/Icons/`

**Fonts:**
- UI Font: `Content/Fonts/UI_Font`
- Monospace Font: `Content/Fonts/RobotoMono`

**Sounds:**
- Alert sounds: `Content/Audio/UI/`

## Conclusion

You now have a comprehensive guide to create the `WBP_Spaceship` HUD Blueprint with:

✅ **Crosshair** - Weapon aim indicator  
✅ **Speed & Throttle** - Navigation info  
✅ **Health & Hull Integrity** - Ship status  
✅ **Targeting** - Target information  
✅ **Alerts** - Messages and warnings  
✅ **Polish** - Animations and effects  

### Next Steps

1. **Create the Widget Blueprint** following Step 1
2. **Design the layout** using the recommended structure
3. **Implement Blueprint logic** for each HUD element
4. **Test thoroughly** with the checklist
5. **Polish and optimize** based on playtesting

### Getting Help

If you encounter issues:
- Check the Troubleshooting section
- Review C++ class headers for function documentation
- Examine existing UI widgets for reference patterns
- Test in isolation (disable complex logic to narrow down issues)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-03  
**Target UE Version:** 5.6+  
**Difficulty:** Intermediate  
**Estimated Implementation Time:** 2-4 hours  
**Maintained by:** Adastrea Development Team
