# Content Creation Guide - Quick Start to Playability

This guide will help you create the minimum content needed to make Adastrea playable in ~30 minutes.

## Prerequisites

- Unreal Engine 5.3 installed
- Adastrea.uproject opened in UE5
- Basic familiarity with Unreal Editor

## Phase 1: Create Your First Level (5 minutes)

### Step 1: Create Main Menu Level

1. **File → New Level → Empty Level**
2. **Save as**: `Content/Maps/MainMenu`
3. **Add lighting**:
   - Drag `Directional Light` into scene
   - Drag `Sky Atmosphere` into scene
   - Drag `Sky Light` into scene
4. **Add basic camera**:
   - Place `Camera Actor` in scene
   - Set as starting view camera

### Step 2: Create Test Level

1. **File → New Level → Empty Level**
2. **Save as**: `Content/Maps/TestLevel`
3. **Add space environment**:
   - Add `Directional Light` (represents sun)
   - Add `Sky Atmosphere`
   - Add `Exponential Height Fog` (for space atmosphere)
   - Set fog to very low density for space feel
4. **Add test geometry**:
   - Add a few `Static Mesh` actors as asteroids/obstacles
   - Use basic cubes/spheres scaled to different sizes
5. **Add Player Start**:
   - Drag `Player Start` actor into scene
   - Position at your desired spawn point

## Phase 2: Create Player Ship Blueprint (10 minutes)

### Step 1: Create Ship Blueprint

1. **Content Browser → Right-click → Blueprint Class**
2. **Select parent**: `Spaceship` (your C++ class)
3. **Name**: `BP_PlayerShip`
4. **Location**: `Content/Blueprints/Ships/`

### Step 2: Configure Ship Visual

1. **Open** `BP_PlayerShip`
2. **Add Component**: `Static Mesh`
3. **For now, use**: Basic shape (Cube/Cone) as placeholder
   - Rotate to point forward
   - Scale to ship size (e.g., 2, 1, 0.5)
4. **Add Component**: `Camera`
   - Position behind and above ship
   - Adjust for third-person view
5. **Add Component**: `Spring Arm`
   - Attach camera to spring arm
   - Set arm length (500-1000)

### Step 3: Add Basic Movement

1. **Event Graph**:
   - Create `Enhanced Input Action` nodes
   - Connect to ship movement functions
   
2. **Simple movement setup**:
```
Event Tick
  → Get Input Axis Value (Forward/Back)
  → Multiply by Speed (500)
  → Add Actor Local Offset (Forward Vector)
```

3. **Simple rotation**:
```
Event Tick
  → Get Input Axis Value (Yaw)
  → Multiply by Turn Rate (50)
  → Add Actor Local Rotation
```

### Step 4: Compile and Save

## Phase 3: Create Player Controller (5 minutes)

### Step 1: Create Controller Blueprint

1. **Content Browser → Blueprint Class**
2. **Parent**: `PlayerController`
3. **Name**: `BP_SpaceshipController`
4. **Location**: `Content/Blueprints/Controllers/`

### Step 2: Configure Input

1. **Open** `BP_SpaceshipController`
2. **Class Defaults**:
   - Check "Show Mouse Cursor" (for UI interaction)
   - Set Input Mode to "Game and UI"

## Phase 4: Create Game Mode (5 minutes)

### Step 1: Create GameMode Blueprint

1. **Content Browser → Blueprint Class**
2. **Parent**: `AdastreaGameMode` (your C++ class)
3. **Name**: `BP_SpaceGameMode`
4. **Location**: `Content/Blueprints/GameModes/`

### Step 2: Configure Game Mode

1. **Open** `BP_SpaceGameMode`
2. **Class Defaults**:
   - **Default Pawn Class**: `BP_PlayerShip`
   - **Player Controller Class**: `BP_SpaceshipController`

### Step 3: Set as Project Default

1. **Edit → Project Settings**
2. **Maps & Modes**:
   - **Default GameMode**: `BP_SpaceGameMode`
   - **Editor Startup Map**: `TestLevel`
   - **Game Default Map**: `MainMenu`

## Phase 5: Create Basic HUD (5 minutes)

### Step 1: Create HUD Widget

1. **Content Browser → Right-click → User Interface → Widget Blueprint**
2. **Name**: `WBP_SpaceshipHUD`
3. **Location**: `Content/UI/HUD/`

### Step 2: Design Simple HUD

1. **Add Canvas Panel** (root)
2. **Add Text blocks**:
   - Ship Name (top-left)
   - Speed indicator (top-right)
   - Health/Shield bars (bottom)
3. **Add images**:
   - Crosshair (center)
   - Minimap placeholder (bottom-right)

### Step 3: Add to Viewport

1. **Open** `BP_SpaceshipController`
2. **Event BeginPlay**:
```
Create Widget (WBP_SpaceshipHUD)
  → Add to Viewport
```

## Phase 6: Create Example Data Assets (5 minutes)

### Step 1: Create Ship Data Asset

1. **Content Browser → Right-click → Miscellaneous → Data Asset**
2. **Select**: `SpaceshipDataAsset`
3. **Name**: `DA_Ship_PlayerScout`
4. **Location**: `Content/DataAssets/Ships/`

### Step 2: Configure Ship Stats

Use the YAML templates from `Assets/SpaceshipTemplates/PathfinderScout.yaml` as reference:

1. **Basic Info**:
   - Ship Name: "Scout Ship Alpha"
   - Ship Class: "Scout"
   - Ship ID: "player_scout_01"

2. **Core Stats**:
   - Hull Strength: 1000
   - Max Speed: 800
   - Maneuverability: 8
   - Cargo Capacity: 50

3. **Combat Stats**:
   - Armor Rating: 100
   - Shield Strength: 500
   - Weapon Slots: 2

### Step 3: Create Faction Data Asset

1. **Create**: `FactionDataAsset`
2. **Name**: `DA_Faction_PlayerAlliance`
3. **Location**: `Content/DataAssets/Factions/`
4. **Configure**: Use `Assets/FactionSetupGuide.md` templates

## Phase 7: Set Up Input (3 minutes)

### Step 1: Create Input Actions

1. **Content Browser → Input → Input Action**
2. **Create**:
   - `IA_Move` (Vector2D)
   - `IA_Boost` (Boolean)
   - `IA_Fire` (Boolean)

### Step 2: Create Input Mapping Context

1. **Content Browser → Input → Input Mapping Context**
2. **Name**: `IMC_Spaceship`
3. **Add mappings**:
   - Move: WASD keys
   - Boost: Left Shift
   - Fire: Left Mouse Button

### Step 3: Apply to Controller

1. **BP_SpaceshipController → BeginPlay**:
```
Get Player Controller
  → Enhanced Input Local Player Subsystem
  → Add Mapping Context (IMC_Spaceship, Priority 0)
```

## Testing Your Playable Game

1. **Save All**
2. **Click Play** (Alt+P)
3. **You should**:
   - Spawn in TestLevel at Player Start
   - See your ship mesh
   - Control ship with WASD
   - See basic HUD

## Next Steps for Full Playability

### Visual Improvements
- Import or create actual ship models
- Add materials and textures
- Create particle effects for engines
- Add lighting and post-processing

### Gameplay Enhancements
- Implement shooting mechanics
- Add enemy ships (using AI classes)
- Create objectives/missions
- Add station interaction

### UI/UX
- Create main menu
- Add pause menu
- Implement settings menu
- Create inventory/trading UI

### Audio
- Add ship engine sounds
- Add weapon sounds
- Add background music
- Add UI feedback sounds

## Troubleshooting

### Ship doesn't move
- Check Input Mapping Context is added
- Verify Input Actions are bound
- Check ship has collision enabled

### Can't see ship
- Verify Static Mesh is assigned
- Check camera position/rotation
- Ensure lighting in level

### HUD doesn't show
- Verify widget is added to viewport
- Check widget visibility settings
- Ensure Game Mode is set correctly

## Quick Reference: Folder Structure

```
Content/
├── Blueprints/
│   ├── Ships/
│   │   └── BP_PlayerShip
│   ├── Controllers/
│   │   └── BP_SpaceshipController
│   └── GameModes/
│       └── BP_SpaceGameMode
├── DataAssets/
│   ├── Ships/
│   │   └── DA_Ship_PlayerScout
│   └── Factions/
│       └── DA_Faction_PlayerAlliance
├── Maps/
│   ├── MainMenu
│   └── TestLevel
├── UI/
│   └── HUD/
│       └── WBP_SpaceshipHUD
└── Input/
    ├── IA_Move
    ├── IA_Boost
    ├── IA_Fire
    └── IMC_Spaceship
```

## Resources

- **C++ Classes**: All in `Source/Adastrea/`
- **Templates**: YAML files in `Assets/*/Templates/`
- **Documentation**: See `Assets/*Guide.md` files
- **Examples**: Check `Assets/*Examples.md`

---

**Time to Playability**: ~30 minutes following this guide
**Result**: Controllable ship in space environment with basic HUD
**Next**: Add gameplay, visuals, and polish
