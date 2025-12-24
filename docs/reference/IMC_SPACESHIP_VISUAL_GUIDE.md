# IMC_Spaceship Visual Setup Guide for UE5.6

**Complete visual guide for configuring IMC_Spaceship (Input Mapping Context) in Unreal Engine 5.6**

---

## Overview

The `IMC_Spaceship` (Input Mapping Context) is the central mapping system that connects physical keyboard/mouse/gamepad inputs to Input Actions in Adastrea. It defines which keys trigger which actions and how they're processed.

**Type**: Input Mapping Context  
**Location**: `Content/Input/IMC_Spaceship.uasset`  
**Purpose**: Maps hardware inputs (keys, mouse, gamepad) to logical Input Actions

---

## Visual Layout in UE5.6 Editor

When you open `IMC_Spaceship` in the Unreal Editor, you'll see a list-based interface:

```
┌─────────────────────────────────────────────────────────────────┐
│ IMC_Spaceship - Input Mapping Context                          │
├─────────────────────────────────────────────────────────────────┤
│ Toolbar: [+ Add] [Import] [Export] [Search...]                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ Mappings (list of key → action bindings):                      │
│                                                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ 🔽 IA_Move (Input Action)                                  │ │
│ │    ├─ [W Key]          Modifiers: Swizzle(Y), Scale(1.0)  │ │
│ │    ├─ [S Key]          Modifiers: Swizzle(Y), Negate      │ │
│ │    ├─ [A Key]          Modifiers: Swizzle(X), Negate      │ │
│ │    ├─ [D Key]          Modifiers: Swizzle(X), Scale(1.0)  │ │
│ │    └─ [Gamepad Left]   Modifiers: Deadzone(0.15)          │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ 🔽 IA_Look (Input Action)                                  │ │
│ │    ├─ [Mouse XY]       Modifiers: Scalar(0.5, 0.5)        │ │
│ │    └─ [Gamepad Right]  Modifiers: Deadzone(0.25)          │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ 🔽 IA_Boost (Input Action)                                 │ │
│ │    └─ [Space Key]                                          │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ 🔽 IA_Fire_Primary (Input Action)                          │ │
│ │    └─ [Left Mouse Button]                                  │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌────────────────────────────────────────────────────────────┐ │
│ │ 🔽 IA_PauseMenu (Input Action)                             │ │
│ │    └─ [Escape Key]                                         │ │
│ └────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ... (additional mappings) ...                                   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

Legend:
  🔽 = Collapsible section (click to expand/collapse)
  [Key] = Hardware input (keyboard, mouse, gamepad)
  Modifiers = Transform how input is processed
```

---

## Understanding the Interface

### Main Components

1. **Mappings List**: Shows all Input Action → Hardware Key assignments
2. **Action Groups**: Each Input Action can have multiple hardware inputs mapped to it
3. **Modifiers**: Optional transformations applied to input values
4. **Player Mappable Keys**: Allows players to rebind keys at runtime (optional)

### Reading a Mapping Entry

```
┌──────────────────────────────────────────────────────────┐
│ 🔽 IA_Move (Input Action)                ◄── Action Name│
│    ├─ [W Key] ◄── Hardware Input                        │
│    │   Modifiers: ◄── Processing                        │
│    │   - Swizzle Input Axis (Order: Y) ◄── Modifier 1   │
│    │   - Scale (1.0)                   ◄── Modifier 2   │
│    │                                                     │
│    ├─ [S Key]                                           │
│    │   Modifiers:                                       │
│    │   - Swizzle Input Axis (Order: Y)                 │
│    │   - Negate                        ◄── Reverses val│
│    │   - Scale (1.0)                                    │
└──────────────────────────────────────────────────────────┘
```

---

## Step-by-Step Configuration for Minimum PIE

### Step 1: Open IMC_Spaceship

1. **Content Browser** → Navigate to `Content/Input/`
2. **Double-click** `IMC_Spaceship.uasset`
3. The Input Mapping Context editor opens

### Step 2: Add Movement Mapping (IA_Move)

#### Add W Key (Forward)

1. Click **[+ Add]** button at top
2. **Action** dropdown → Select `IA_Move`
3. **Key** dropdown → Search: "W" → Select **W Key**
4. Click **[+ Add Modifier]** next to the W key entry
   - Select **Swizzle Input Axis Values**
   - Set **Order**: `YXZ` (Y first)
5. Click **[+ Add Modifier]** again
   - Select **Scalar**
   - Set value: `1.0`

**Visual Result**:
```
🔽 IA_Move
   ├─ [W Key]
   │   Modifiers:
   │   - Swizzle Input Axis (Order: Y)
   │   - Scale (1.0)
```

#### Add S Key (Backward)

1. Click **[+ Add]** under the IA_Move group
2. **Key** → **S Key**
3. Add modifiers:
   - **Swizzle Input Axis Values** (Order: YXZ)
   - **Negate** (reverses direction)
   - **Scalar** (1.0)

**Visual Result**:
```
🔽 IA_Move
   ├─ [W Key] - Forward
   ├─ [S Key] - Backward
```

#### Add A Key (Strafe Left)

1. Click **[+ Add]** under IA_Move
2. **Key** → **A Key**
3. Add modifiers:
   - **Swizzle Input Axis Values** (Order: XYZ) ← X first
   - **Negate** (left is negative)
   - **Scalar** (1.0)

#### Add D Key (Strafe Right)

1. Click **[+ Add]** under IA_Move
2. **Key** → **D Key**
3. Add modifiers:
   - **Swizzle Input Axis Values** (Order: XYZ)
   - **Scalar** (1.0)

**Complete IA_Move Visual Result**:
```
┌────────────────────────────────────────────────┐
│ 🔽 IA_Move (Axis2D)                           │
│    ├─ [W Key] ...................... Forward   │
│    │   • Swizzle(Y), Scale(1.0)               │
│    ├─ [S Key] ...................... Backward  │
│    │   • Swizzle(Y), Negate, Scale(1.0)       │
│    ├─ [A Key] ...................... Left      │
│    │   • Swizzle(X), Negate, Scale(1.0)       │
│    └─ [D Key] ...................... Right     │
│        • Swizzle(X), Scale(1.0)               │
└────────────────────────────────────────────────┘
```

### Step 3: Add Camera Mapping (IA_Look)

#### Add Mouse Movement

1. Click **[+ Add]** button
2. **Action** → `IA_Look`
3. **Key** → Search: "Mouse XY" → Select **Mouse XY 2D-Axis**
4. Add modifier:
   - **Scalar**
   - X: `0.5` (horizontal sensitivity)
   - Y: `0.5` (vertical sensitivity)

**Visual Result**:
```
┌────────────────────────────────────────────────┐
│ 🔽 IA_Look (Axis2D)                           │
│    └─ [Mouse XY 2D-Axis]                      │
│        • Scalar (X: 0.5, Y: 0.5)              │
└────────────────────────────────────────────────┘
```

### Step 4: Add Boost Mapping (IA_Boost)

1. Click **[+ Add]**
2. **Action** → `IA_Boost`
3. **Key** → **Space Bar**
4. No modifiers needed (digital input)

**Visual Result**:
```
┌────────────────────────────────────────────────┐
│ 🔽 IA_Boost (Digital)                         │
│    └─ [Space Bar]                             │
└────────────────────────────────────────────────┘
```

### Step 5: Add Combat Mapping (IA_Fire_Primary)

1. Click **[+ Add]**
2. **Action** → `IA_Fire_Primary`
3. **Key** → **Left Mouse Button**
4. No modifiers needed

**Visual Result**:
```
┌────────────────────────────────────────────────┐
│ 🔽 IA_Fire_Primary (Digital)                  │
│    └─ [Left Mouse Button]                     │
└────────────────────────────────────────────────┘
```

### Step 6: Add Menu Mapping (IA_PauseMenu)

1. Click **[+ Add]**
2. **Action** → `IA_PauseMenu`
3. **Key** → **Escape Key**
4. No modifiers needed

**Visual Result**:
```
┌────────────────────────────────────────────────┐
│ 🔽 IA_PauseMenu (Digital)                     │
│    └─ [Escape Key]                            │
└────────────────────────────────────────────────┘
```

### Step 7: Save

1. Click **Save** button (💾) in toolbar
2. Close the editor

---

## Minimum Required Configuration Summary

For basic PIE gameplay, configure these 6 mappings:

| Input Action | Key(s) | Modifiers | Purpose |
|-------------|--------|-----------|---------|
| **IA_Move** | W, S, A, D | Swizzle, Negate, Scalar | Movement |
| **IA_Look** | Mouse XY | Scalar (0.5, 0.5) | Camera |
| **IA_Boost** | Space | None | Speed boost |
| **IA_Fire_Primary** | Left Mouse Button | None | Shoot |
| **IA_PauseMenu** | Escape | None | Pause |

**Total mappings**: 6 Input Actions with 8 hardware keys

---

## Complete Configuration Example

Here's what a fully configured IMC_Spaceship looks like with all recommended mappings:

```
IMC_Spaceship (Complete)
│
├─ 🔽 IA_Move (Movement)
│  ├─ W Key (Forward)
│  ├─ S Key (Backward)
│  ├─ A Key (Left)
│  ├─ D Key (Right)
│  └─ Gamepad Left Thumbstick
│
├─ 🔽 IA_Look (Camera)
│  ├─ Mouse XY 2D-Axis
│  └─ Gamepad Right Thumbstick
│
├─ 🔽 IA_Boost (Movement)
│  ├─ Space Bar
│  └─ Gamepad Left Shoulder Button
│
├─ 🔽 IA_Brake (Movement)
│  ├─ Left Ctrl
│  └─ Gamepad Right Shoulder Button
│
├─ 🔽 IA_Roll (Movement)
│  ├─ Q Key (Roll Left)
│  └─ E Key (Roll Right)
│
├─ 🔽 IA_Fire_Primary (Combat)
│  ├─ Left Mouse Button
│  └─ Gamepad Right Trigger
│
├─ 🔽 IA_Fire_Secondary (Combat)
│  ├─ Right Mouse Button
│  └─ Gamepad Left Trigger
│
├─ 🔽 IA_TargetLock (Combat)
│  ├─ Tab Key
│  └─ Gamepad Y Button
│
├─ 🔽 IA_NextTarget (Combat)
│  └─ T Key
│
├─ 🔽 IA_PreviousTarget (Combat)
│  └─ Shift + T
│
├─ 🔽 IA_NearestTarget (Combat)
│  └─ R Key
│
├─ 🔽 IA_Interact (Interaction)
│  ├─ F Key
│  └─ Gamepad A Button
│
├─ 🔽 IA_Dock (Interaction)
│  └─ L Key
│
├─ 🔽 IA_Board (Interaction)
│  └─ B Key
│
├─ 🔽 IA_Autopilot (Navigation)
│  └─ P Key
│
├─ 🔽 IA_SetWaypoint (Navigation)
│  └─ G Key
│
├─ 🔽 IA_ClearWaypoint (Navigation)
│  └─ H Key
│
├─ 🔽 IA_NextWaypoint (Navigation)
│  └─ N Key
│
├─ 🔽 IA_PauseMenu (System)
│  ├─ Escape Key
│  └─ Gamepad Start Button
│
├─ 🔽 IA_Map (System)
│  └─ M Key
│
├─ 🔽 IA_Inventory (System)
│  └─ I Key
│
├─ 🔽 IA_QuestLog (System)
│  └─ J Key
│
├─ 🔽 IA_Screenshot (System)
│  └─ F12 Key
│
├─ 🔽 IA_StationEditor (System)
│  └─ M Key (when in spaceship)
│
├─ 🔽 IA_CycleCamera (Camera)
│  └─ V Key
│
├─ 🔽 IA_ZoomIn (Camera)
│  └─ Mouse Wheel Up
│
└─ 🔽 IA_ZoomOut (Camera)
   └─ Mouse Wheel Down
```

---

## Understanding Modifiers

### Common Modifiers

#### 1. Swizzle Input Axis Values
**Purpose**: Rearranges input axes (X, Y, Z order)  
**Use Case**: Convert horizontal key press to forward movement

```
Example: W Key for forward movement
- Input: W pressed = 1D value
- Swizzle (Order: Y): Assigns to Y-axis
- Result: (0, 1) on Vector2D for IA_Move
```

#### 2. Negate
**Purpose**: Reverses the sign of input value  
**Use Case**: Make S go backward, A go left

```
Example: S Key for backward
- Input: S pressed = 1.0
- Swizzle to Y-axis = (0, 1)
- Negate = (0, -1)
- Result: Backward movement
```

#### 3. Scalar
**Purpose**: Multiplies input value  
**Use Case**: Adjust sensitivity, scale movement

```
Example: Mouse sensitivity
- Input: Mouse moved X units
- Scalar (0.5): Halves the value
- Result: Slower camera rotation
```

#### 4. Deadzone
**Purpose**: Ignores small input values  
**Use Case**: Gamepad stick drift prevention

```
Example: Gamepad Left Stick
- Deadzone (0.15): Ignore values < 0.15
- Result: No movement from stick drift
```

---

## Adding Gamepad Support

### Gamepad Left Thumbstick (Movement)

1. Under **IA_Move**, click **[+ Add]**
2. **Key** → Search: "Gamepad Left" → **Gamepad Left Thumbstick 2D-Axis**
3. Add modifier:
   - **Deadzone** (0.15 - 0.25 recommended)

### Gamepad Right Thumbstick (Camera)

1. Under **IA_Look**, click **[+ Add]**
2. **Key** → **Gamepad Right Thumbstick 2D-Axis**
3. Add modifiers:
   - **Deadzone** (0.25)
   - **Scalar** (2.0, 2.0) - gamepad needs higher sensitivity

### Gamepad Buttons

```
A Button (Xbox) / X Button (PS) → IA_Interact
B Button (Xbox) / Circle (PS) → IA_Board
X Button (Xbox) / Square (PS) → (unused)
Y Button (Xbox) / Triangle (PS) → IA_TargetLock

Left Shoulder → IA_PreviousTarget
Right Shoulder → IA_NextTarget
Left Trigger → IA_Brake
Right Trigger → IA_Boost

D-Pad → (optional navigation)
Start Button → IA_PauseMenu
Select Button → IA_Map
```

---

## Troubleshooting

### Issue: Keys don't respond in PIE

**Solution 1**: Check Input Action exists
```
1. Open Content/Input/Actions/
2. Verify IA_Move.uasset, IA_Look.uasset, etc. exist
3. If missing, create them (Right-click → Input → Input Action)
```

**Solution 2**: Check mapping is saved
```
1. Open IMC_Spaceship
2. Verify mappings appear in list
3. Click Save button
```

**Solution 3**: Check DA_InputConfig references IMC_Spaceship
```
1. Open Content/DataAssets/Input/DA_InputConfig
2. Verify SpaceshipMappingContext = IMC_Spaceship
3. See DA_INPUTCONFIG_VISUAL_GUIDE.md for details
```

### Issue: Movement goes in wrong direction

**Solution**: Check Swizzle and Negate modifiers
```
W Key should be: Swizzle(Y), Scale(1.0) - Forward
S Key should be: Swizzle(Y), Negate - Backward
A Key should be: Swizzle(X), Negate - Left
D Key should be: Swizzle(X), Scale(1.0) - Right
```

### Issue: Mouse sensitivity too high/low

**Solution**: Adjust Scalar modifier on Mouse XY
```
Too sensitive: Lower value (0.3, 0.3)
Too slow: Increase value (0.7, 0.7)
Default: (0.5, 0.5)
```

### Issue: Gamepad stick drifts

**Solution**: Increase Deadzone modifier
```
Light drift: Deadzone (0.15)
Medium drift: Deadzone (0.25)
Heavy drift: Deadzone (0.35)
```

### Issue: Can't find Input Action in dropdown

**Solution**: Create the Input Action first
```
1. Content Browser → Content/Input/Actions/
2. Right-click → Input → Input Action
3. Name: IA_[ActionName]
4. Configure Value Type:
   - Movement/Camera: Axis2D (Vector2D)
   - Buttons: Digital (bool)
5. Save and return to IMC_Spaceship
```

---

## Advanced Features

### Player Mappable Keys

Allow players to rebind keys at runtime:

1. In IMC_Spaceship, select a mapping
2. Check **Player Mappable**
3. Set **Display Name**: "Forward", "Fire", etc.
4. Set **Display Category**: "Movement", "Combat", etc.

Players can then rebind keys through an in-game settings menu.

### Chorded Actions (Key Combinations)

Create key combos like Shift+W:

1. Add primary key (W)
2. Click **[+ Chorded Action]** on that mapping
3. Add modifier key (Shift)
4. Result: W only triggers when Shift is held

### Triggered Events

Configure when actions trigger:

- **Down**: Key pressed
- **Pressed**: After initial press delay
- **Released**: Key released
- **Hold**: Key held for duration
- **Tap**: Quick press and release
- **Pulse**: Repeats while held

Set in Input Action asset (IA_*), not in mapping context.

---

## Testing Your Configuration

### In-Editor Testing

1. **Open TestLevel** in editor
2. Press **Play** button (Alt+P)
3. **Test each mapping**:
   - W/A/S/D - Ship moves correctly
   - Mouse - Camera rotates
   - Space - Boost activates
   - Left Click - Weapon fires
   - Escape - Menu opens
4. Check **Output Log** for input errors

### Debug Input

Enable input visualization:

1. In PIE, press **~** (tilde) to open console
2. Type: `showdebug enhancedinput`
3. See active Input Actions and their values on screen

---

## Quick Reference Card

### Minimum Setup (6 mappings):
```
IA_Move:
  └─ W, S, A, D (with Swizzle and Negate)

IA_Look:
  └─ Mouse XY (with Scalar 0.5)

IA_Boost:
  └─ Space

IA_Fire_Primary:
  └─ Left Mouse Button

IA_PauseMenu:
  └─ Escape
```

### Time to Configure: 5-10 minutes

### Location: `Content/Input/IMC_Spaceship.uasset`

### Type: Input Mapping Context (UInputMappingContext)

---

## Integration with DA_InputConfig

IMC_Spaceship works together with DA_InputConfig:

```
DA_InputConfig
├─ References IMC_Spaceship (SpaceshipMappingContext property)
└─ References Input Actions (IA_Move, IA_Look, etc.)

IMC_Spaceship
├─ Maps Keys → Input Actions
└─ Defines Modifiers (Swizzle, Negate, Scalar, etc.)

Flow:
1. Player presses W key
2. IMC_Spaceship converts to IA_Move with modifiers
3. BP_SpaceshipController binds IA_Move (via DA_InputConfig)
4. BP_PlayerShip receives Move event
5. Ship moves forward
```

**See Also**: [DA_INPUTCONFIG_VISUAL_GUIDE.md](DA_INPUTCONFIG_VISUAL_GUIDE.md)

---

## Differences from UE5.5 and Earlier

### UE5.6 Changes:
- ✅ Interface is similar to UE5.5
- ✅ Modifier system unchanged
- ✅ All functionality preserved
- ⚠️ Enhanced Input plugin must be enabled (usually automatic)

### No Breaking Changes:
IMC_Spaceship from UE5.5 works in UE5.6 without modification.

---

## Example: Creating Complete Movement Mapping

**Goal**: Set up WASD movement with proper axis swizzling

### Visual Steps:

```
Step 1: Add IA_Move action group
[+ Add] → Action: IA_Move

Step 2: Add W Key (Forward)
├─ Key: W
├─ [+ Add Modifier] → Swizzle Input Axis Values
│  └─ Order: YXZ (Y first)
└─ [+ Add Modifier] → Scalar
   └─ Value: 1.0

Step 3: Add S Key (Backward)
├─ Key: S
├─ [+ Add Modifier] → Swizzle Input Axis Values
│  └─ Order: YXZ
├─ [+ Add Modifier] → Negate
└─ [+ Add Modifier] → Scalar
   └─ Value: 1.0

Step 4: Add A Key (Left)
├─ Key: A
├─ [+ Add Modifier] → Swizzle Input Axis Values
│  └─ Order: XYZ (X first)
├─ [+ Add Modifier] → Negate
└─ [+ Add Modifier] → Scalar
   └─ Value: 1.0

Step 5: Add D Key (Right)
├─ Key: D
├─ [+ Add Modifier] → Swizzle Input Axis Values
│  └─ Order: XYZ
└─ [+ Add Modifier] → Scalar
   └─ Value: 1.0

Result: WASD creates 2D movement vector
- W = (0, 1) - Forward
- S = (0, -1) - Backward
- A = (-1, 0) - Left
- D = (1, 0) - Right
- W+D = (1, 1) - Forward-Right diagonal
```

---

## Additional Resources

- **DA_InputConfig Guide**: [DA_INPUTCONFIG_VISUAL_GUIDE.md](DA_INPUTCONFIG_VISUAL_GUIDE.md)
- **Input System Overview**: `Assets/EnhancedInputImplementation.md`
- **Input Actions**: `Content/Input/Actions/`
- **Blueprint Setup**: `BLUEPRINT_PIE_CHECKLIST.md`

---

**Need help?** Check the Output Log in Unreal Editor for input-related error messages and warnings.
