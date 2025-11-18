# Blueprint Creation Step-by-Step Guide

Complete instructions for creating all essential Blueprints to make the game playable.

**⭐ New to Blueprints?** Consider starting with [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - it's designed for complete beginners and explains everything from the ground up!

## Table of Contents

1. [Player Ship Blueprint](#player-ship-blueprint)
2. [Player Controller Blueprint](#player-controller-blueprint)
3. [Game Mode Blueprint](#game-mode-blueprint)
4. [HUD Widget Blueprint](#hud-widget-blueprint)
5. [Main Menu Widget](#main-menu-widget)
6. [Game Instance Blueprint](#game-instance-blueprint)

---

## Player Ship Blueprint

**File**: `Content/Blueprints/Ships/BP_PlayerShip`  
**Parent Class**: `Spaceship` (C++ class)

### Component Setup

#### 1. Static Mesh Component
```
Component: Static Mesh
Name: ShipMesh
Parent: RootComponent

Settings:
- Static Mesh: SM_Placeholder_Ship (or basic cube for testing)
- Scale: (2.0, 1.0, 0.5)
- Rotation: (0, -90, 0) to point forward
- Collision: BlockAll
- Simulate Physics: false
```

#### 2. Spring Arm Component
```
Component: Spring Arm
Name: CameraBoom
Parent: ShipMesh

Settings:
- Socket Offset: (0, 0, 100) - above ship
- Target Arm Length: 800
- Use Pawn Control Rotation: true
- Inherit Pitch: true
- Inherit Yaw: true
- Inherit Roll: false
```

#### 3. Camera Component
```
Component: Camera
Name: FollowCamera
Parent: CameraBoom

Settings:
- Field of View: 90
- Use Pawn Control Rotation: false (camera follows spring arm)
```

#### 4. Arrow Component (Optional)
```
Component: Arrow
Name: ProjectileSpawn
Parent: ShipMesh

Settings:
- Location: (200, 0, 0) - front of ship
- Use for spawning projectiles
```

### Variables

```cpp
// Movement
ForwardSpeed: Float = 500.0
StrafeSpeed: Float = 300.0
BoostMultiplier: Float = 2.0
bIsBoosting: Boolean = false

// Rotation
YawRate: Float = 50.0
PitchRate: Float = 40.0
RollRate: Float = 30.0

// Combat
MaxHealth: Float = 1000.0
CurrentHealth: Float = 1000.0
MaxShield: Float = 500.0
CurrentShield: Float = 500.0
ShieldRegenRate: Float = 25.0

// References
ShipDataAsset: SpaceshipDataAsset (Object Reference)
```

### Event Graph - Movement

#### Event: Enhanced Input Move
```
Enhanced Input Action: IA_Move
  Event: Triggered
    ↓
  Get Action Value → Vector2D
    ↓
  Break Vector2D (X, Y)
    ↓
  [Forward/Back (Y)]
    Multiply → (bIsBoosting ? ForwardSpeed * BoostMultiplier : ForwardSpeed)
    Multiply → Delta Seconds
    ↓
    Get Actor Forward Vector
    Multiply → Result
    ↓
    Add Actor World Offset
    
  [Left/Right (X)]
    Multiply → StrafeSpeed
    Multiply → Delta Seconds
    ↓
    Get Actor Right Vector
    Multiply → Result
    ↓
    Add Actor World Offset
```

#### Event: Enhanced Input Look
```
Enhanced Input Action: IA_Look
  Event: Triggered
    ↓
  Get Action Value → Vector2D
    ↓
  Break Vector2D (X for Yaw, Y for Pitch)
    ↓
  [Yaw]
    Multiply → YawRate
    Multiply → Delta Seconds
    ↓
    Make Rotator (0, Result, 0)
    ↓
    Add Actor Local Rotation
    
  [Pitch]
    Multiply → PitchRate
    Multiply → Delta Seconds
    ↓
    Make Rotator (Result, 0, 0)
    ↓
    Add Actor Local Rotation
```

#### Event: Enhanced Input Boost
```
Enhanced Input Action: IA_Boost
  Event: Started
    ↓
  Set bIsBoosting = true
  
  Event: Completed
    ↓
  Set bIsBoosting = false
```

### Event Graph - Combat

#### Event: Enhanced Input Fire Primary
```
Enhanced Input Action: IA_Fire_Primary
  Event: Triggered
    ↓
  Spawn Actor from Class
    Class: BP_Projectile
    Transform: ProjectileSpawn World Transform
    ↓
  Play Sound at Location (LaserFireSound)
```

### Event Graph - Health System

#### Event: Tick
```
Event Tick
  ↓
  [Shield Regeneration]
  CurrentShield < MaxShield?
    ↓ Yes
    CurrentShield + (ShieldRegenRate * Delta Seconds)
    ↓
    Clamp (0, MaxShield)
    ↓
    Set CurrentShield
```

#### Custom Event: TakeDamage
```
Input: DamageAmount (Float)
  ↓
  [Shield First]
  DamageAmount > CurrentShield?
    ↓ Yes
    Overflow = DamageAmount - CurrentShield
    CurrentShield = 0
    CurrentHealth -= Overflow
    ↓ No
    CurrentShield -= DamageAmount
    ↓
  CurrentHealth <= 0?
    ↓ Yes
    Call: DestroyShip()
    ↓ No
    Update HUD
```

### Functions

#### Function: GetShipStats
```
Return: SpaceshipDataAsset
  ↓
  Return: ShipDataAsset
```

#### Function: DestroyShip
```
  Spawn Emitter at Location (ExplosionParticle)
  ↓
  Play Sound at Location (ExplosionSound)
  ↓
  Destroy Actor (self)
```

---

## Player Controller Blueprint

**File**: `Content/Blueprints/Controllers/BP_SpaceshipController`  
**Parent Class**: `PlayerController`

### Event Graph

#### Event: BeginPlay
```
Event BeginPlay
  ↓
  [Set up Enhanced Input]
  Get Player Controller → Cast to APlayerController
    ↓
  Get Subsystem (Enhanced Input Local Player Subsystem)
    ↓
  Add Mapping Context
    Context: IMC_Spaceship
    Priority: 0
    Options: Default
    ↓
  [Create HUD]
  Create Widget
    Class: WBP_SpaceshipHUD
    Owning Player: Self
    ↓
  Add to Viewport
    Z-Order: 0
```

### Class Defaults

```
Auto Manage Active Camera Target: true
Show Mouse Cursor: false
Enable Click Events: false
Enable Mouse Over Events: false
```

---

## Game Mode Blueprint

**File**: `Content/Blueprints/GameModes/BP_SpaceGameMode`  
**Parent Class**: `AdastreaGameMode` (C++ class)

### Class Defaults

```
Default Pawn Class: BP_PlayerShip
Player Controller Class: BP_SpaceshipController
HUD Class: None (using Widget)
Game State Class: GameState (default)
Player State Class: PlayerState (default)
Spectator Class: SpectatorPawn (default)
```

### Variables

```
CurrentMissionObjective: Text = ""
Score: Integer = 0
EnemiesDestroyed: Integer = 0
```

### Event Graph

#### Event: BeginPlay
```
Event BeginPlay
  ↓
  Print String ("Game Started")
  ↓
  Set CurrentMissionObjective = "Explore the sector"
```

---

## HUD Widget Blueprint

**File**: `Content/UI/HUD/WBP_SpaceshipHUD`  
**Type**: Widget Blueprint

### Widget Hierarchy

```
Canvas Panel (Root)
├── Overlay (Top)
│   ├── Text - Ship Name (Top-Left)
│   ├── Text - Speed (Top-Right)
│   └── Text - Objective (Top-Center)
├── Overlay (Center)
│   └── Image - Crosshair
└── Overlay (Bottom)
    ├── ProgressBar - Health (Bottom-Left)
    ├── ProgressBar - Shield (Below Health)
    └── Text - Status Messages (Bottom-Center)
```

### Widget Components Setup

#### Ship Name Text
```
Position: (20, 20)
Size: (300, 30)
Font Size: 18
Color: White
Text: "Bound to Ship Name variable"
```

#### Speed Text
```
Anchors: Top-Right
Position: (Screen Width - 150, 20)
Size: (130, 30)
Font Size: 16
Color: Cyan
Text: "Bound to Speed variable"
```

#### Crosshair Image
```
Anchors: Center
Position: Center
Size: (32, 32)
Image: T_Crosshair (simple cross texture)
Tint: White with 50% opacity
```

#### Health Bar
```
Position: (20, Screen Height - 100)
Size: (300, 20)
Fill Color: Red
Background Color: Dark Gray
Progress: Bound to CurrentHealth / MaxHealth
```

#### Shield Bar
```
Position: (20, Screen Height - 70)
Size: (300, 20)
Fill Color: Blue
Background Color: Dark Gray
Progress: Bound to CurrentShield / MaxShield
```

### Event Graph - Update

#### Event: Tick
```
Event Tick
  ↓
  [Get Player Ship]
  Get Owning Player Pawn → Cast to BP_PlayerShip
    ↓ Valid?
    [Update Speed]
    Get Velocity → Vector Length
    ↓
    Format Text: "{0} m/s"
    ↓
    Set Speed Text
    
    [Update Health]
    Get CurrentHealth / MaxHealth
    ↓
    Set Health Progress Bar Percent
    
    [Update Shield]
    Get CurrentShield / MaxShield
    ↓
    Set Shield Progress Bar Percent
```

---

## Main Menu Widget

**File**: `Content/UI/Menus/WBP_MainMenu`  
**Type**: Widget Blueprint

### Widget Hierarchy

```
Canvas Panel (Root)
├── Image - Background
├── Vertical Box (Center)
│   ├── Text - Title "Adastrea"
│   ├── Spacer
│   ├── Button - Start Game
│   │   └── Text - "Start Game"
│   ├── Spacer
│   ├── Button - Settings
│   │   └── Text - "Settings"
│   ├── Spacer
│   ├── Button - Quit
│   │   └── Text - "Quit"
│   └── Spacer
└── Text - Version (Bottom-Right)
```

### Event Graph

#### Button: Start Game - OnClicked
```
OnClicked
  ↓
  Open Level
    Level Name: "TestLevel"
    ↓
  Remove from Parent (this widget)
```

#### Button: Quit - OnClicked
```
OnClicked
  ↓
  Quit Game
```

---

## Game Instance Blueprint

**File**: `Content/Blueprints/Core/BP_GameInstance`  
**Parent Class**: `GameInstance`

### Variables

```
PlayerName: String = "Commander"
PlayerShipData: SpaceshipDataAsset = DA_Ship_PlayerScout
CurrentCredits: Integer = 1000
TotalPlayTime: Float = 0.0
SaveSlotName: String = "SaveSlot1"
```

### Functions

#### Function: SaveGame
```
Create Save Game Object
  Class: BP_SaveGame
  ↓
  Set Variables:
    - PlayerName
    - CurrentCredits
    - PlayerShipData
  ↓
  Save Game to Slot
    Slot Name: SaveSlotName
    User Index: 0
```

#### Function: LoadGame
```
Load Game from Slot
  Slot Name: SaveSlotName
  User Index: 0
  ↓
  Cast to BP_SaveGame
    ↓ Valid?
    Get Variables and set to Game Instance
```

---

## Project Settings Configuration

After creating these Blueprints, configure in **Edit → Project Settings**:

### Maps & Modes
```
Default Maps:
- Game Default Map: MainMenu
- Editor Startup Map: TestLevel

Default Modes:
- Default GameMode: BP_SpaceGameMode
```

### Input
```
Default Classes:
- Default Player Input Class: EnhancedPlayerInput
- Default Input Component Class: EnhancedInputComponent
```

---

## Testing Checklist

After creating all Blueprints:

- [ ] BP_PlayerShip compiles without errors
- [ ] Enhanced Input actions bind correctly
- [ ] Ship moves with WASD keys
- [ ] Camera follows ship smoothly
- [ ] HUD displays and updates
- [ ] Main menu opens on game start
- [ ] Start Game button loads test level
- [ ] Game Mode spawns player ship

---

## Troubleshooting

### Ship doesn't spawn
- Check Game Mode → Default Pawn Class is set to BP_PlayerShip
- Verify Player Start is in the level
- Check spawn collision settings

### Input not working
- Verify Enhanced Input subsystem is initialized
- Check IMC_Spaceship is added in BeginPlay
- Ensure Input Actions exist and are properly mapped

### HUD not visible
- Check widget is added to viewport
- Verify Z-order is correct
- Check widget visibility settings

---

## Next Steps

1. Create particle effects for ship engine
2. Add weapon projectile Blueprint
3. Create enemy ship AI
4. Add sound effects
5. Create more levels and missions

See `CONTENT_CREATION_QUICKSTART.md` for the complete workflow.
