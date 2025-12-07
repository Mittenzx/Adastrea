# PIE (Play In Editor) Setup Checklist

## Current Status

Your project has:
✅ **Input System**: Enhanced Input with `IMC_Spaceship` mapping context
✅ **Spaceship Code**: `ASpaceship` class with flight controls
✅ **Player Controller**: `AAdastreaPlayerController`
✅ **Game Mode**: `ATestGameMode` and `AAdastreaGameMode`
✅ **HUD Widget**: `UAdastreaHUDWidget` base class
✅ **Maps**: TestLevel.umap (editor start map)

## Problems Preventing PIE from Working

### 🔴 Problem 1: No Default Pawn Set
**Issue**: Game mode doesn't specify what pawn to spawn for the player
**Location**: `DefaultEngine.ini` line 4
**Current**: `GlobalDefaultGameMode=/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C`

### 🔴 Problem 2: Missing Blueprint Implementations
**Issue**: C++ classes exist but no Blueprint versions with visual content
**Missing**:
- `BP_PlayerShip` (exists but may not be configured)
- `BP_PlayerController` (exists but may not have input setup)
- `WBP_SpaceShipHUD` (exists but may not be assigned)

### 🔴 Problem 3: Input Not Connected
**Issue**: Enhanced Input system configured but not connected to Player Controller
**Required**: Blueprint or C++ to add Input Mapping Context in BeginPlay

### 🔴 Problem 4: HUD Not Spawned
**Issue**: No code spawns the HUD widget when game starts
**Required**: Player Controller must create and add HUD to viewport

---

## Quick Fix Steps (30 minutes)

### Step 1: Set Default Pawn in Game Mode (5 min)

Open Unreal Editor:
1. Open `Content/Blueprints/BP_TestGameMode`
2. **Class Defaults** panel:
   - **Default Pawn Class**: Select `BP_PlayerShip`
   - **Player Controller Class**: Select `BP_PlayerController`
3. Compile & Save

### Step 2: Configure Player Ship Blueprint (10 min)

Open `Content/Blueprints/Ships/BP_PlayerShip`:

1. **Components Panel**:
   - Add **Static Mesh Component** (or Skeletal Mesh)
   - Set mesh to a visible placeholder (Cube, Sphere, or imported ship model)
   - Add **Camera Component** (for player view)
   - Set Camera location: X=0, Y=0, Z=100 (behind ship)
   - Add **SpaceshipControlsComponent** (from C++ class)

2. **Class Defaults**:
   - **Auto Possess Player**: Player 0
   - **Movement Component** → Max Speed: 3000

3. Compile & Save

### Step 3: Setup Player Controller Input (10 min)

Open `Content/Blueprints/Controllers/BP_PlayerController`:

1. **Event Graph**:
   ```
   Event BeginPlay
     ↓
   [Get Enhanced Input Subsystem]
     Target: Get Player Controller → Get Local Player → Get Subsystem (Enhanced Input)
     ↓
   [Add Mapping Context]
     Context: IMC_Spaceship (select from Content/Input/)
     Priority: 0
     Options: (default)
     ↓
   [Create HUD Widget]
     Widget Class: WBP_SpaceShipHUD (select from Content/UI/HUD/)
     Owning Player: Self
     ↓
   [Add to Viewport]
     Target: Return value from Create Widget
     Z-Order: 0
   ```

2. **Class Defaults**:
   - **Show Mouse Cursor**: False (for flight)
   - **Enable Click Events**: False
   - **Input Mode**: Game Only

3. Compile & Save

### Step 4: Setup Basic HUD (5 min)

Open `Content/UI/HUD/WBP_SpaceShipHUD`:

1. **Designer**:
   - Add **Text Block** (top-left): "Speed: 0"
   - Add **Text Block** (top-right): "Health: 100"
   - Add **Text Block** (center): "CONTROLS: WASD=Move, Mouse=Look, Scroll=Speed"

2. **Graph** (optional - add later):
   - Implement `InitializeHUD` event from parent class

3. Compile & Save

### Step 5: Verify Test Level (5 min)

Open `Content/Maps/TestLevel`:

1. **World Settings** panel:
   - **Game Mode Override**: `BP_TestGameMode`

2. **Level**:
   - Add **Directional Light** (if missing)
   - Add **Sky Sphere** or **Sky Atmosphere** (if missing)
   - Add **Post Process Volume** (Unbound = true)

3. **Save Map**

---

## Testing PIE

1. Open `TestLevel` map
2. Press **PIE button** (Play ▶ in toolbar) or press `Alt+P`
3. **Expected**:
   - You spawn in a spaceship
   - HUD shows text
   - WASD moves the ship
   - Mouse rotates view
   - Mouse wheel adjusts speed

4. **If Nothing Happens**:
   - Check **Output Log** for errors
   - Verify BP_PlayerShip has visible mesh
   - Verify Camera component is present
   - Check Input Mapping Context is added

---

## Advanced: Alternative Quick Setup

If you want to test IMMEDIATELY without Blueprints:

### C++ Quick Test (Programmer Path)

Edit `Config/DefaultEngine.ini`:

```ini
[/Script/Engine.Engine]
GameDefaultMap=/Game/Maps/TestLevel
EditorStartupMap=/Game/Maps/TestLevel

[/Script/EngineSettings.GameMapsSettings]
GlobalDefaultGameMode=/Script/Adastrea.AdastreaGameMode
```

This uses the C++ game mode directly, but you still need:
1. Set `DefaultSpaceshipClass` in game mode Blueprint
2. Add visible mesh to spaceship Blueprint

---

## Common Issues & Solutions

### Issue: "No player start found"
**Solution**: Add a **Player Start** actor to TestLevel map

### Issue: "Can't move or nothing happens"
**Solution**: 
- Verify `IMC_Spaceship` has key mappings configured
- Check Input Actions exist in `Content/Input/Actions/`
- Verify BP_PlayerController adds mapping context

### Issue: "No HUD visible"
**Solution**: 
- Check `WBP_SpaceShipHUD` has visual elements
- Verify BP_PlayerController creates and adds widget to viewport
- Check widget visibility is not set to Hidden

### Issue: "Ship spawns but I can't see it"
**Solution**: 
- Add a visible Static Mesh to BP_PlayerShip
- Set Camera component location behind/above ship
- Check mesh materials are not fully transparent

### Issue: "Mouse doesn't rotate view"
**Solution**: 
- Verify `IA_Look` Input Action is bound to Mouse XY in `IMC_Spaceship`
- Check SpaceshipControlsComponent is added to BP_PlayerShip
- Verify Mouse Capture mode is set in Player Controller

---

## Files to Check in Unreal Editor

1. ✅ `Content/Input/IMC_Spaceship` - Input Mapping Context
2. ✅ `Content/Input/Actions/` - Input Actions (IA_Move, IA_Look, etc.)
3. ⚠️ `Content/Blueprints/BP_TestGameMode` - Needs Default Pawn set
4. ⚠️ `Content/Blueprints/Ships/BP_PlayerShip` - Needs mesh & camera
5. ⚠️ `Content/Blueprints/Controllers/BP_PlayerController` - Needs input setup
6. ⚠️ `Content/UI/HUD/WBP_SpaceShipHUD` - Needs basic UI elements
7. ✅ `Content/Maps/TestLevel` - Exists but verify Game Mode Override

---

## Next Steps After PIE Works

Once you can fly around:

1. **Improve HUD**: Add real-time speed, health, target info
2. **Add Weapons**: Bind Fire input, add weapon effects
3. **Add Environment**: Space stations, planets, asteroids
4. **Add Combat**: Enemy ships, damage system
5. **Add Trading**: Docking, markets, cargo
6. **Polish Input**: Adjust sensitivity, add gamepad support

---

## Reference Documentation

- `ENHANCED_INPUT_GUIDE.md` - Complete input setup guide
- `CONTENT_CREATION_QUICKSTART.md` - Full content creation workflow
- `BLUEPRINT_CREATION_GUIDE.md` - Detailed Blueprint instructions
- `Assets/EnhancedInputImplementation.md` - C++ input implementation

---

**Generated**: 2025-11-29  
**Status**: Active troubleshooting guide  
**Priority**: Critical - prevents gameplay testing
