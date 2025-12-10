# Adastrea Input Controls Reference

Quick reference for the default input controls in Adastrea.

## Spaceship Flight Controls

### Movement
| Key | Action | Description |
|-----|--------|-------------|
| **A** | Strafe Left | Move ship left (X axis) |
| **D** | Strafe Right | Move ship right (X axis) |
| **W** | Move Up | Move ship up (Z+ axis) |
| **S** | Move Down | Move ship down (Z- axis) |
| **Mouse Wheel Up** | Increase Throttle | Increase forward speed |
| **Mouse Wheel Down** | Decrease Throttle | Decrease forward speed |
| **Left Shift** | Boost | Temporary speed boost |
| **Left Ctrl** | Brake | Quick stop/slow down |

### Camera/Look
| Key | Action | Description |
|-----|--------|-------------|
| **Mouse Movement** | Look/Rotate Camera | Control camera direction |
| **Page Up** | Zoom In | Zoom camera closer |
| **Page Down** | Zoom Out | Zoom camera further |

## Combat Controls

### Weapons
| Key | Action | Description |
|-----|--------|-------------|
| **Left Mouse Button** | Fire Primary Weapon | Fire main weapon |
| **Right Mouse Button** | Fire Secondary Weapon | Fire secondary weapon |
| **Tab** | Cycle Weapon | Switch between weapons |

### Targeting
| Key | Action | Description |
|-----|--------|-------------|
| **T** | Target Next | Target next enemy |
| **Alt + T** | Target Previous | Target previous enemy |
| **R** | Target Nearest | Target closest enemy |
| **Escape** (x2) | Clear Target | Clear current target |

### Defensive Systems
| Key | Action | Description |
|-----|--------|-------------|
| **F** | Toggle Shields | Turn shields on/off |
| **Q** | Shields Forward | Redirect shields to front |
| **E** | Shields Rear | Redirect shields to rear |
| **C** | Deploy Countermeasures | Launch countermeasures |

## Navigation Controls

| Key | Action | Description |
|-----|--------|-------------|
| **Z** | Toggle Autopilot | Enable/disable autopilot |
| **V** | Match Target Speed | Match speed of target |
| **X** | Full Stop | Immediate full stop |
| **B** | Dock | Initiate docking sequence |

## UI Controls

### Menus
| Key | Action | Description |
|-----|--------|-------------|
| **Escape** | Pause Menu | Open pause menu |
| **I** | Inventory | Open inventory |
| **M** | Star Map | Open navigation map |
| **L** | Quest Log | Open quest log |
| **K** | Ship Customization | Open ship loadout |
| **O** | Trading | Open trading interface |
| **P** | Communications | Open comms |

### General
| Key | Action | Description |
|-----|--------|-------------|
| **E** | Interact | Interact with nearby object |
| **F12** | Screenshot | Take screenshot |

## Gamepad Controls (Xbox Controller)

### Movement
| Button/Stick | Action | Description |
|--------------|--------|-------------|
| **Left Stick** | Strafe | Move left/right/up/down |
| **Right Stick** | Look | Rotate camera |
| **LT (Left Trigger)** | Decrease Throttle | Reduce forward speed |
| **RT (Right Trigger)** | Increase Throttle | Increase forward speed |
| **LB (Left Bumper)** | Brake | Quick stop |
| **RB (Right Bumper)** | Boost | Speed boost |

### Combat
| Button | Action | Description |
|--------|--------|-------------|
| **A** | Fire Primary | Fire main weapon |
| **B** | Fire Secondary | Fire secondary weapon |
| **X** | Cycle Weapon | Switch weapons |
| **Y** | Target Next | Next enemy target |

### UI
| Button | Action | Description |
|--------|--------|-------------|
| **Start** | Pause Menu | Open pause menu |
| **Back/Select** | Star Map | Open map |
| **D-Pad Up** | Inventory | Open inventory |
| **D-Pad Down** | Quest Log | Open quests |
| **D-Pad Left** | Trading | Open trading |
| **D-Pad Right** | Communications | Open comms |

## Input Action Reference (for Developers)

These are the Enhanced Input Actions created by InputSystemGenerator.py:

### Core Movement Actions
- `IA_Move` (Vector2D) - Strafe left/right
- `IA_VerticalMove` (Float) - Move up/down on Z axis
- `IA_Throttle` (Float) - Forward/backward throttle control
- `IA_Look` (Vector2D) - Camera rotation
- `IA_Boost` (Boolean) - Speed boost
- `IA_Brake` (Boolean) - Brake/slow down

### Combat Actions
- `IA_Fire_Primary` (Boolean) - Primary weapon
- `IA_Fire_Secondary` (Boolean) - Secondary weapon
- `IA_CycleWeapon` (Boolean) - Cycle weapons
- `IA_TargetNext` (Boolean) - Next target
- `IA_TargetPrevious` (Boolean) - Previous target
- `IA_TargetNearest` (Boolean) - Nearest target
- `IA_ClearTarget` (Boolean) - Clear target

### Ship System Actions
- `IA_ShieldsToggle` (Boolean) - Toggle shields
- `IA_ShieldsFront` (Boolean) - Shields forward
- `IA_ShieldsRear` (Boolean) - Shields rear
- `IA_CountermeasuresDeploy` (Boolean) - Deploy countermeasures

### Navigation Actions
- `IA_Autopilot` (Boolean) - Toggle autopilot
- `IA_MatchSpeed` (Boolean) - Match target speed
- `IA_FullStop` (Boolean) - Full stop
- `IA_Dock` (Boolean) - Dock with station

### UI Actions
- `IA_PauseMenu` (Boolean) - Pause menu
- `IA_OpenInventory` (Boolean) - Open inventory
- `IA_OpenMap` (Boolean) - Open star map
- `IA_OpenQuestLog` (Boolean) - Open quest log
- `IA_OpenShipCustomization` (Boolean) - Open ship customization
- `IA_OpenTrading` (Boolean) - Open trading
- `IA_OpenComms` (Boolean) - Open communications

### Camera Actions
- `IA_CameraZoomIn` (Boolean) - Zoom camera in
- `IA_CameraZoomOut` (Boolean) - Zoom camera out
- `IA_CameraReset` (Boolean) - Reset camera

### Misc Actions
- `IA_Interact` (Boolean) - Interact with object
- `IA_Screenshot` (Boolean) - Take screenshot

## Input Mapping Contexts

Three mapping contexts are created:

1. **IMC_Spaceship** - Active during spaceship flight
   - Contains all movement, combat, and navigation controls
   - Priority: 1 (highest)

2. **IMC_Menu** - Active when menus are open
   - Contains UI navigation controls
   - Priority: 2

3. **IMC_Station** - Active when docked at a station
   - Contains station interaction controls
   - Priority: 1

## Customization

Players can customize controls in:
- Main Menu → Settings → Controls
- Pause Menu → Settings → Controls

Developers can modify key bindings by:
1. Opening the Input Mapping Context in Content Browser
2. Modifying key assignments for each Input Action
3. Saving the asset

## Notes

- **Mouse Wheel** is reserved for throttle control (not camera zoom)
- **Camera Zoom** uses Page Up/Down keys
- **W/S** control vertical movement (Z axis), NOT forward/backward
- **Forward/Backward** is controlled by mouse wheel throttle
- All controls are fully rebindable through the Enhanced Input system

## Version History

- **1.1** (December 10, 2025) - Fixed key bindings: Mouse wheel for throttle, W/S for Z axis
- **1.0** (December 10, 2025) - Initial controls reference

---

For implementation details, see:
- `InputSystemGenerator.py` - Script that generates these input actions
- `CONTENT_GENERATION_GUIDE.md` - Guide for generating input system
- `ENHANCED_INPUT_GUIDE.md` - Manual setup guide for Enhanced Input
