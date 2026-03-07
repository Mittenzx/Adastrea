# DA_InputConfig Visual Setup Guide for UE5.6

**Complete visual guide for configuring DA_InputConfig Data Asset in Unreal Engine 5.6**

---

## Overview

The `DA_InputConfig` Data Asset is the central hub for the Enhanced Input system in Adastrea. It references all Input Actions and Input Mapping Contexts, making it easy to manage input configuration in one place.

**Parent Class**: `UInputConfigDataAsset` (C++)
**Location**: `Content/DataAssets/Input/DA_InputConfig.uasset`

---

## Visual Layout in UE5.6 Editor

When you open `DA_InputConfig` in the Unreal Editor, you'll see the Details panel organized into these categories:

```
┌─────────────────────────────────────────────────────────────┐
│ DA_InputConfig - Details Panel                              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│ ▼ Input Mapping                                             │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Spaceship Mapping Context        [IMC_Spaceship] ⚡│   │
│   │ Menu Mapping Context             [None]          ⚡│   │
│   │ Spaceship Context Priority        0              ▲▼│   │
│   │ Menu Context Priority             1              ▲▼│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | Movement                                  │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Move Action                      [IA_Move]        ⚡│   │
│   │ Look Action                      [IA_Look]        ⚡│   │
│   │ Boost Action                     [IA_Boost]       ⚡│   │
│   │ Brake Action                     [None]           ⚡│   │
│   │ Roll Action                      [None]           ⚡│   │
│   │ Throttle Up Action               [None]           ⚡│   │
│   │ Throttle Down Action             [None]           ⚡│   │
│   │ Toggle Flight Assist Action      [None]           ⚡│   │
│   │ Toggle Travel Mode Action        [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | Combat                                    │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Fire Primary Action              [IA_Fire_Primary]⚡│   │
│   │ Fire Secondary Action            [None]           ⚡│   │
│   │ Target Lock Action               [None]           ⚡│   │
│   │ Next Target Action               [None]           ⚡│   │
│   │ Previous Target Action           [None]           ⚡│   │
│   │ Nearest Target Action            [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | Interaction                               │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Interact Action                  [None]           ⚡│   │
│   │ Dock Action                      [None]           ⚡│   │
│   │ Board Action                     [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | Navigation                                │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Autopilot Action                 [None]           ⚡│   │
│   │ Set Waypoint Action              [None]           ⚡│   │
│   │ Clear Waypoint Action            [None]           ⚡│   │
│   │ Next Waypoint Action             [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | System                                    │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Menu Action                      [IA_PauseMenu]   ⚡│   │
│   │ Map Action                       [None]           ⚡│   │
│   │ Inventory Action                 [None]           ⚡│   │
│   │ Quest Log Action                 [None]           ⚡│   │
│   │ Screenshot Action                [None]           ⚡│   │
│   │ Station Editor Action            [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
│ ▼ Input Actions | Camera                                    │
│   ┌────────────────────────────────────────────────────┐   │
│   │ Cycle Camera Action              [None]           ⚡│   │
│   │ Zoom In Action                   [None]           ⚡│   │
│   │ Zoom Out Action                  [None]           ⚡│   │
│   └────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘

Legend:
  ⚡ = Browse/Select Asset button (lightning bolt icon)
  ▲▼ = Numeric up/down spinner
  [Name] = Currently assigned asset
  [None] = No asset assigned (needs configuration)
```

---

## Step-by-Step Configuration for Minimum PIE

### Step 1: Create the Data Asset (If Not Exists)

1. **Right-click** in Content Browser → `Content/DataAssets/Input/`
2. Select **Miscellaneous** → **Data Asset**
3. In the popup, search for: `InputConfigDataAsset`
4. Select it and click **Select**
5. Name it: `DA_InputConfig`
6. **Double-click** to open

### Step 2: Assign Input Mapping Context

In the **Input Mapping** category:

```
Spaceship Mapping Context: [Click ⚡ button]
  └─ Search: "IMC_Spaceship"
  └─ Select: Content/Input/IMC_Spaceship
  └─ Result: Shows "IMC_Spaceship" in field
```

**Visual Result**:
```
┌─────────────────────────────────────────────┐
│ Input Mapping                               │
│ Spaceship Mapping Context  [IMC_Spaceship]⚡│  ✅ Assigned
│ Menu Mapping Context       [None]         ⚡│  ⚠️ Optional
│ Spaceship Context Priority  0            ▲▼│  ✅ Keep at 0
│ Menu Context Priority       1            ▲▼│  ✅ Keep at 1
└─────────────────────────────────────────────┘
```

### Step 3: Assign Movement Input Actions

In the **Input Actions | Movement** category:

#### Move Action
```
Move Action: [Click ⚡ button]
  └─ Search: "IA_Move"
  └─ Select: Content/Input/Actions/IA_Move
  └─ Result: Shows "IA_Move" in field
```

#### Look Action
```
Look Action: [Click ⚡ button]
  └─ Search: "IA_Look"
  └─ Select: Content/Input/Actions/IA_Look
  └─ Result: Shows "IA_Look" in field
```

#### Boost Action
```
Boost Action: [Click ⚡ button]
  └─ Search: "IA_Boost"
  └─ Select: Content/Input/Actions/IA_Boost
  └─ Result: Shows "IA_Boost" in field
```

**Visual Result**:
```
┌──────────────────────────────────────────────┐
│ Input Actions | Movement                     │
│ Move Action                [IA_Move]        ⚡│  ✅ REQUIRED
│ Look Action                [IA_Look]        ⚡│  ✅ REQUIRED
│ Boost Action               [IA_Boost]       ⚡│  ✅ REQUIRED
│ Brake Action               [None]           ⚡│  ⚠️ Optional
│ Roll Action                [None]           ⚡│  ⚠️ Optional
│ Throttle Up Action         [None]           ⚡│  ⚠️ Optional
│ Throttle Down Action       [None]           ⚡│  ⚠️ Optional
│ Toggle Flight Assist Action[None]           ⚡│  ⚠️ Optional
│ Toggle Travel Mode Action  [None]           ⚡│  ⚠️ Optional
└──────────────────────────────────────────────┘
```

### Step 4: Assign Combat Input Actions

In the **Input Actions | Combat** category:

#### Fire Primary Action
```
Fire Primary Action: [Click ⚡ button]
  └─ Search: "IA_Fire_Primary"
  └─ Select: Content/Input/Actions/IA_Fire_Primary
  └─ Result: Shows "IA_Fire_Primary" in field
```

**Visual Result**:
```
┌──────────────────────────────────────────────┐
│ Input Actions | Combat                       │
│ Fire Primary Action        [IA_Fire_Primary]⚡│  ✅ REQUIRED
│ Fire Secondary Action      [None]           ⚡│  ⚠️ Optional
│ Target Lock Action         [None]           ⚡│  ⚠️ Optional
│ Next Target Action         [None]           ⚡│  ⚠️ Optional
│ Previous Target Action     [None]           ⚡│  ⚠️ Optional
│ Nearest Target Action      [None]           ⚡│  ⚠️ Optional
└──────────────────────────────────────────────┘
```

### Step 5: Assign System Input Actions

In the **Input Actions | System** category:

#### Menu Action
```
Menu Action: [Click ⚡ button]
  └─ Search: "IA_PauseMenu"
  └─ Select: Content/Input/Actions/IA_PauseMenu
  └─ Result: Shows "IA_PauseMenu" in field
```

**Visual Result**:
```
┌──────────────────────────────────────────────┐
│ Input Actions | System                       │
│ Menu Action                [IA_PauseMenu]   ⚡│  ✅ REQUIRED
│ Map Action                 [None]           ⚡│  ⚠️ Optional
│ Inventory Action           [None]           ⚡│  ⚠️ Optional
│ Quest Log Action           [None]           ⚡│  ⚠️ Optional
│ Screenshot Action          [None]           ⚡│  ⚠️ Optional
│ Station Editor Action      [None]           ⚡│  ⚠️ Optional
└──────────────────────────────────────────────┘
```

### Step 6: Save

1. Click **Save** button in toolbar (💾 icon)
2. Close the editor window

---

## Minimum Required Configuration Summary

For basic PIE gameplay, you MUST assign these 6 items:

| Category | Property | Required Asset | Status |
|----------|----------|----------------|--------|
| Input Mapping | Spaceship Mapping Context | IMC_Spaceship | ✅ CRITICAL |
| Movement | Move Action | IA_Move | ✅ CRITICAL |
| Movement | Look Action | IA_Look | ✅ CRITICAL |
| Movement | Boost Action | IA_Boost | ✅ CRITICAL |
| Combat | Fire Primary Action | IA_Fire_Primary | ✅ CRITICAL |
| System | Menu Action | IA_PauseMenu | ✅ CRITICAL |

**All other properties are optional** and can be assigned later as you expand gameplay features.

---

## Complete Configuration Example

Here's what a fully configured DA_InputConfig looks like:

```
DA_InputConfig (Fully Configured)
├─ Input Mapping
│  ├─ Spaceship Mapping Context: IMC_Spaceship ✅
│  ├─ Menu Mapping Context: IMC_Menu ✅
│  ├─ Spaceship Context Priority: 0
│  └─ Menu Context Priority: 1
│
├─ Movement Actions
│  ├─ Move Action: IA_Move ✅
│  ├─ Look Action: IA_Look ✅
│  ├─ Boost Action: IA_Boost ✅
│  ├─ Brake Action: IA_Brake ✅
│  ├─ Roll Action: IA_Roll ✅
│  ├─ Throttle Up: IA_ThrottleUp ✅
│  ├─ Throttle Down: IA_ThrottleDown ✅
│  ├─ Toggle Flight Assist: IA_ToggleFlightAssist ✅
│  └─ Toggle Travel Mode: IA_ToggleTravelMode ✅
│
├─ Combat Actions
│  ├─ Fire Primary: IA_Fire_Primary ✅
│  ├─ Fire Secondary: IA_Fire_Secondary ✅
│  ├─ Target Lock: IA_TargetLock ✅
│  ├─ Next Target: IA_NextTarget ✅
│  ├─ Previous Target: IA_PreviousTarget ✅
│  └─ Nearest Target: IA_NearestTarget ✅
│
├─ Interaction Actions
│  ├─ Interact: IA_Interact ✅
│  ├─ Dock: IA_Dock ✅
│  └─ Board: IA_Board ✅
│
├─ Navigation Actions
│  ├─ Autopilot: IA_Autopilot ✅
│  ├─ Set Waypoint: IA_SetWaypoint ✅
│  ├─ Clear Waypoint: IA_ClearWaypoint ✅
│  └─ Next Waypoint: IA_NextWaypoint ✅
│
├─ System Actions
│  ├─ Menu: IA_PauseMenu ✅
│  ├─ Map: IA_Map ✅
│  ├─ Inventory: IA_Inventory ✅
│  ├─ Quest Log: IA_QuestLog ✅
│  ├─ Screenshot: IA_Screenshot ✅
│  └─ Station Editor: IA_StationEditor ✅
│
└─ Camera Actions
   ├─ Cycle Camera: IA_CycleCamera ✅
   ├─ Zoom In: IA_ZoomIn ✅
   └─ Zoom Out: IA_ZoomOut ✅
```

---

## How to Find Input Actions

If you can't find an Input Action when clicking the ⚡ button:

### Method 1: Search by Name
1. Click the ⚡ button next to the field
2. In the asset picker popup, use the **Search** box
3. Type: `IA_` (all Input Actions start with this prefix)
4. Browse the filtered list
5. Select the desired action

### Method 2: Browse Content Folder
1. Click the ⚡ button next to the field
2. In the asset picker popup, click **View Options**
3. Enable **Show Folders**
4. Navigate to: `Content/Input/Actions/`
5. Select the desired action

### Method 3: Content Browser Drag & Drop
1. Open **Content Browser**
2. Navigate to: `Content/Input/Actions/`
3. **Drag** the desired IA_* asset
4. **Drop** it onto the corresponding field in DA_InputConfig Details panel

---

## Verification Checklist

After configuring DA_InputConfig, verify:

- [ ] ✅ Spaceship Mapping Context is assigned (IMC_Spaceship)
- [ ] ✅ Move Action is assigned (IA_Move)
- [ ] ✅ Look Action is assigned (IA_Look)
- [ ] ✅ Boost Action is assigned (IA_Boost)
- [ ] ✅ Fire Primary Action is assigned (IA_Fire_Primary)
- [ ] ✅ Menu Action is assigned (IA_PauseMenu)
- [ ] ✅ Data Asset is saved (💾)
- [ ] ✅ No error messages in Output Log

---

## Common Issues and Solutions

### Issue: Can't find IA_* assets in asset picker

**Solution 1**: Check they exist
```
Content Browser → Navigate to Content/Input/Actions/
Look for: IA_Move.uasset, IA_Look.uasset, etc.
```

**Solution 2**: If missing, create them
```
1. Right-click in Content/Input/Actions/
2. Input → Input Action
3. Name: IA_Move (or whichever is missing)
4. Configure action type (e.g., Vector2D for Move)
```

### Issue: Assignment doesn't "stick" / reverts to None

**Solution**: Ensure asset is in correct content folder
```
✅ Correct: Content/Input/Actions/IA_Move.uasset
❌ Wrong: Plugins/.../IA_Move.uasset
❌ Wrong: TempContent/IA_Move.uasset
```

### Issue: "Input Action is not valid" error in Output Log

**Solution**: Check Input Action configuration
```
1. Open the Input Action (e.g., IA_Move)
2. Verify "Value Type" is set correctly:
   - IA_Move: Axis2D (Vector2D)
   - IA_Look: Axis2D (Vector2D)
   - IA_Boost: Digital (bool)
   - IA_Fire_Primary: Digital (bool)
   - IA_PauseMenu: Digital (bool)
```

### Issue: Keys don't respond in PIE

**Solution**: Verify Input Mapping Context (IMC_Spaceship)
```
1. Open Content/Input/IMC_Spaceship
2. Check mappings exist:
   - IA_Move → W, A, S, D keys
   - IA_Look → Mouse X, Mouse Y
   - IA_Boost → Space bar
   - IA_Fire_Primary → Left Mouse Button
   - IA_PauseMenu → Escape key
```

---

## Differences from UE5.5 and Earlier

### UE5.6 Changes:
- ✅ Data Asset interface is the same
- ✅ Property organization unchanged
- ✅ Asset picker works the same way
- ✅ All functionality preserved

### No Breaking Changes:
If you have DA_InputConfig from UE5.5, it will work in UE5.6 without modification.

---

## Blueprint Usage Example

Once configured, DA_InputConfig is used in C++ or Blueprint like this:

### Blueprint (in BP_SpaceshipController):

```
Event BeginPlay
  ↓
Get Enhanced Input Subsystem
  ↓
Load DA_InputConfig
  (Asset Reference: /Game/DataAssets/Input/DA_InputConfig)
  ↓
Get Spaceship Mapping Context
  ↓
Add Mapping Context
  Context: [Result from above]
  Priority: 0
```

### C++ (in ASpaceshipController.cpp):

```cpp
void ASpaceshipController::BeginPlay()
{
    Super::BeginPlay();

    // Load DA_InputConfig
    UInputConfigDataAsset* InputConfig = LoadObject<UInputConfigDataAsset>(
        nullptr,
        TEXT("/Game/DataAssets/Input/DA_InputConfig.DA_InputConfig")
    );

    if (InputConfig && InputConfig->GetSpaceshipMappingContext())
    {
        // Get Enhanced Input subsystem
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            // Add mapping context
            Subsystem->AddMappingContext(
                InputConfig->GetSpaceshipMappingContext(),
                InputConfig->SpaceshipContextPriority
            );
        }
    }
}
```

---

## Quick Reference Card

### Minimal Setup (5 assignments):
1. Input Mapping → Spaceship Mapping Context → **IMC_Spaceship**
2. Movement → Move Action → **IA_Move**
3. Movement → Look Action → **IA_Look**
4. Movement → Boost Action → **IA_Boost**
5. Combat → Fire Primary Action → **IA_Fire_Primary**
6. System → Menu Action → **IA_PauseMenu**

### Time to Configure: 2-3 minutes

### Location: `Content/DataAssets/Input/DA_InputConfig.uasset`

### Parent Class: `UInputConfigDataAsset`

---

## Additional Resources

- **IMC_Spaceship Key Mapping Guide**: [IMC_SPACESHIP_VISUAL_GUIDE.md](IMC_SPACESHIP_VISUAL_GUIDE.md) - Visual guide for key bindings
- **Input System Guide**: `Assets/EnhancedInputImplementation.md`
- **Input Actions Reference**: `Content/Input/Actions/`
- **Mapping Context**: `Content/Input/IMC_Spaceship.uasset`
- **C++ Source**: `Source/Adastrea/Public/Input/InputConfigDataAsset.h`

---

**Need help?** Check the Output Log in Unreal Editor for specific error messages related to input configuration.
