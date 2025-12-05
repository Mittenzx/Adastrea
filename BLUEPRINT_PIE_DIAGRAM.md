# Blueprint Architecture for PIE - Visual Guide

**Visual representation of how blueprints connect for spaceship gameplay**

---

## System Overview Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        UNREAL ENGINE PIE                        │
│                     (Play in Editor Session)                    │
└─────────────────────────────────────────────────────────────────┘
                                 │
                    ┌────────────┴────────────┐
                    │   BP_GameInstance       │
                    │  (Persistent Storage)   │
                    └────────────┬────────────┘
                                 │
                    ┌────────────┴────────────┐
                    │   BP_SpaceGameMode      │
                    │    (Game Rules)         │
                    │                         │
                    │  Sets:                  │
                    │  - Default Pawn         │
                    │  - Player Controller    │
                    └────────┬───────┬────────┘
                             │       │
                ┌────────────┘       └────────────┐
                │                                  │
       ┌────────▼──────────┐          ┌───────────▼──────────┐
       │  BP_PlayerShip    │◄─────────│ BP_SpaceshipController│
       │  (Pawn/Ship)      │          │  (Input Handler)      │
       │                   │          │                       │
       │  Components:      │          │  Handles:             │
       │  - Static Mesh    │          │  - Enhanced Input     │
       │  - Camera         │          │  - HUD Creation       │
       │  - Spring Arm     │          │  - Input Mapping      │
       │  - Movement       │          └───────────┬───────────┘
       └──────┬────────────┘                      │
              │                                   │
              │ References                        │ Uses
              │                                   │
       ┌──────▼────────────┐          ┌──────────▼───────────┐
       │ DA_Ship_PlayerScout│          │   DA_InputConfig     │
       │  (Ship Stats)      │          │  (Input Settings)    │
       │                    │          │                      │
       │  - Health: 1000    │          │  References:         │
       │  - Shield: 500     │          │  - IA_Move           │
       │  - Speed: 2000     │          │  - IA_Look           │
       └────────────────────┘          │  - IA_Boost          │
                                       │  - IMC_Spaceship     │
                                       └──────────┬───────────┘
                                                  │
                                       ┌──────────┴───────────┐
                                       │   Input System       │
                                       │                      │
                                       │  IA_Move   (Action)  │
                                       │  IA_Look   (Action)  │
                                       │  IA_Boost  (Action)  │
                                       │  IMC_Spaceship (Map) │
                                       └──────────────────────┘
```

---

## Input Flow Diagram

```
Player Presses Key
       │
       ▼
┌──────────────────┐
│  IMC_Spaceship   │ ─── Maps keys to actions
│  (Mapping)       │     W → IA_Move (forward)
└────────┬─────────┘     Mouse → IA_Look
         │
         ▼
┌──────────────────┐
│  IA_Move         │ ─── Input Action triggered
│  (Action)        │     Type: Vector2D
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ DA_InputConfig   │ ─── References action
└────────┬─────────┘
         │
         ▼
┌──────────────────────────┐
│ BP_SpaceshipController   │ ─── Binds action to function
│  BeginPlay:              │
│  - Get Input Subsystem   │
│  - Add IMC_Spaceship     │
│  - Bind Input Actions    │
└────────┬─────────────────┘
         │
         ▼
┌──────────────────┐
│  BP_PlayerShip   │ ─── Receives input event
│  Move Function   │     Executes movement
└──────────────────┘
         │
         ▼
    Ship Moves!
```

---

## Spawning Flow Diagram

```
PIE Session Starts
       │
       ▼
┌──────────────────┐
│   Load Level     │
│   (TestLevel)    │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Find Player     │
│  Start Actor     │
└────────┬─────────┘
         │
         ▼
┌──────────────────────┐
│  BP_SpaceGameMode    │
│  (Active in Level)   │
│                      │
│  Default Pawn Class: │
│  → BP_PlayerShip     │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Spawn BP_PlayerShip │
│  at Player Start     │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Create Controller   │
│  BP_SpaceshipController│
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Controller Possess  │
│  Pawn (Ship)         │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Initialize Input    │
│  (Enhanced Input)    │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Create HUD Widget   │
│  Add to Viewport     │
└────────┬─────────────┘
         │
         ▼
   Player Can Play!
```

---

## Component Hierarchy (BP_PlayerShip)

```
BP_PlayerShip (Actor)
│
├─ RootComponent (Scene)
│
├─ ShipMesh (StaticMeshComponent)
│  └─ Visual representation of ship
│
├─ CameraBoom (SpringArmComponent)
│  │  - Attached to ShipMesh
│  │  - Target Arm Length: 800
│  │
│  └─ FollowCamera (CameraComponent)
│     └─ Player's view camera
│
├─ ProjectileSpawn (ArrowComponent)
│  └─ Spawn point for weapons
│
├─ FloatingPawnMovement (Component)
│  └─ Handles 3D space movement
│
├─ WeaponComponent (Optional)
│  └─ Weapon system
│
├─ TargetingComponent (Optional)
│  └─ Target tracking
│
└─ CombatHealthComponent (Optional)
   └─ Health/shield management
```

---

## Data Asset References

```
┌────────────────────────┐
│   BP_PlayerShip        │
└───────────┬────────────┘
            │
            │ References
            ▼
┌────────────────────────┐
│  DA_Ship_PlayerScout   │  ◄── USpaceshipDataAsset (C++)
└────────────────────────┘
            │
            │ Contains
            ▼
┌────────────────────────┐
│  Ship Configuration    │
│  - Display Name        │
│  - Description         │
│  - Core Stats:         │
│    • Health: 1000      │
│    • Shield: 500       │
│    • Speed: 2000       │
│  - Combat Stats        │
│  - Mobility Stats      │
│  - Utility Stats       │
└────────────────────────┘
```

---

## Input Configuration Chain

```
┌──────────────────────┐
│  DA_InputConfig      │  ◄── Created in Editor
└──────────┬───────────┘
           │ References
           ▼
┌──────────────────────┐
│  Input Actions       │  ◄── Already exist
│  - IA_Move           │
│  - IA_Look           │
│  - IA_Boost          │
│  - IA_Fire_Primary   │
│  - IA_PauseMenu      │
└──────────┬───────────┘
           │
           │ Used by
           ▼
┌──────────────────────┐
│  IMC_Spaceship       │  ◄── Already exists
│  (Mapping Context)   │
│                      │
│  Maps:               │
│  W/S → IA_Move (Y)   │
│  A/D → IA_Move (X)   │
│  Mouse → IA_Look     │
│  Space → IA_Boost    │
│  LMB → IA_Fire       │
└──────────────────────┘
```

---

## Level Setup Diagram

```
TestLevel (Map)
│
├─ World Settings
│  └─ GameMode Override: BP_SpaceGameMode
│
├─ Lighting
│  ├─ Directional Light (Sun)
│  ├─ Sky Atmosphere
│  └─ Sky Light
│
├─ Player Start
│  └─ Spawn point (X, Y, Z coordinates)
│
└─ Environment (Optional)
   ├─ Static Mesh (Asteroid 1)
   ├─ Static Mesh (Asteroid 2)
   └─ Static Mesh (Platform)
```

---

## HUD System Diagram

```
BP_SpaceshipController
│
└─ Event BeginPlay
   │
   └─ Create Widget
      │
      ▼
┌────────────────────┐
│ WBP_SpaceShipHUD   │  ◄── UUserWidget
└────────┬───────────┘
         │
         │ Add to Viewport
         ▼
┌────────────────────┐
│  Player's Screen   │
│                    │
│  ┌──────────────┐  │
│  │ Ship: Scout  │  │  ◄── Ship name
│  └──────────────┘  │
│                    │
│         ┼          │  ◄── Crosshair
│                    │
│  ▓▓▓▓░░░░  Health  │  ◄── Health bar
│  ▓▓▓▓▓░░░  Shield  │  ◄── Shield bar
└────────────────────┘
```

---

## Project Settings Configuration

```
Edit → Project Settings
│
├─ Maps & Modes
│  ├─ Default GameMode: BP_SpaceGameMode
│  ├─ Editor Startup Map: TestLevel
│  └─ Game Default Map: TestLevel
│
├─ Input
│  ├─ Default Player Input: EnhancedPlayerInput
│  └─ Default Input Component: EnhancedInputComponent
│
└─ Engine → General Settings
   └─ Game Instance Class: BP_GameInstance
```

---

## Execution Order (PIE Start)

```
1. PIE Button Pressed
         ↓
2. Load TestLevel.umap
         ↓
3. Create BP_GameInstance (persistent)
         ↓
4. Spawn BP_SpaceGameMode (rules)
         ↓
5. Find Player Start actor
         ↓
6. Spawn BP_PlayerShip at Player Start
         ↓
7. Create BP_SpaceshipController
         ↓
8. Controller→Possess(Ship)
         ↓
9. Controller→BeginPlay
    - Initialize Enhanced Input
    - Add IMC_Spaceship mapping
    - Create HUD Widget
    - Add HUD to viewport
         ↓
10. Ship→BeginPlay
    - Load DA_Ship_PlayerScout
    - Initialize components
    - Set up movement
         ↓
11. Input System Active
         ↓
12. Player Can Play! ✅
```

---

## File Dependency Graph

```
Required for PIE:

Content/
│
├─ Blueprints/
│  ├─ Core/
│  │  └─ BP_GameInstance.uasset ───┐
│  │                                │
│  ├─ GameModes/                    │
│  │  └─ BP_SpaceGameMode.uasset ──┼─── References ───┐
│  │                                │                  │
│  ├─ Controllers/                  │                  │
│  │  └─ BP_SpaceshipController ───┼───┐              │
│  │                                │   │              │
│  └─ Ships/                        │   │              │
│     └─ BP_PlayerShip.uasset ─────┼───┼───┐          │
│                                   │   │   │          │
├─ DataAssets/                      │   │   │          │
│  ├─ Input/                        │   │   │          │
│  │  └─ DA_InputConfig.uasset ────┼───┼───┼───┐      │
│  │                                │   │   │   │      │
│  └─ Ships/                        │   │   │   │      │
│     └─ DA_Ship_PlayerScout ──────┘   │   │   │      │
│                                       │   │   │      │
├─ Input/                               │   │   │      │
│  ├─ IMC_Spaceship.uasset ────────────┼───┼───┼───┐  │
│  │                                    │   │   │   │  │
│  └─ Actions/                          │   │   │   │  │
│     ├─ IA_Move.uasset ───────────────┼───┼───┘   │  │
│     ├─ IA_Look.uasset ───────────────┼───┘       │  │
│     ├─ IA_Boost.uasset ──────────────┘           │  │
│     ├─ IA_Fire_Primary.uasset ───────────────────┘  │
│     └─ IA_PauseMenu.uasset ─────────────────────────┘
│
├─ Maps/
│  └─ TestLevel.umap
│
└─ UI/
   └─ HUD/
      └─ WBP_SpaceShipHUD.uasset
```

---

## Minimal Working Set

**Absolute minimum to fly a ship in PIE:**

```
✅ Input System:
   - Content/Input/Actions/IA_Move.uasset
   - Content/Input/Actions/IA_Look.uasset
   - Content/Input/IMC_Spaceship.uasset
   - Content/DataAssets/Input/DA_InputConfig.uasset

✅ Game Framework:
   - Content/Blueprints/GameModes/BP_SpaceGameMode.uasset
   - Content/Blueprints/Controllers/BP_SpaceshipController.uasset
   - Content/Blueprints/Ships/BP_PlayerShip.uasset

✅ Data:
   - Content/DataAssets/Ships/DA_Ship_PlayerScout.uasset

✅ Level:
   - Content/Maps/TestLevel.umap (with Player Start)

✅ Project Settings:
   - Default GameMode = BP_SpaceGameMode
   - Enhanced Input enabled
```

**Everything else is optional for basic PIE!**

---

## Next Level Features

Once basic PIE works, add these in order:

```
Priority 1: HUD
   └─ WBP_SpaceShipHUD with UI elements

Priority 2: Combat
   ├─ BP_Projectile_Laser
   ├─ Weapon firing system
   └─ Damage system

Priority 3: Environment
   ├─ Asteroid actors
   ├─ Space station
   └─ Planet actors

Priority 4: AI
   ├─ BP_Ship_NPC_Fighter
   ├─ AI Controller
   └─ Patrol behavior

Priority 5: Economy
   ├─ Trading UI
   ├─ Trade items
   └─ Market system
```

---

**📖 For detailed instructions, see:**
- BLUEPRINT_PIE_CHECKLIST.md (complete checklist)
- BLUEPRINT_QUICK_START_PIE.md (5-minute setup)
- DA_INPUTCONFIG_VISUAL_GUIDE.md (UE5.6 input configuration guide)
- IMC_SPACESHIP_VISUAL_GUIDE.md (UE5.6 key mapping guide)
- BLUEPRINT_REQUIREMENTS_LIST.md (all blueprints)
