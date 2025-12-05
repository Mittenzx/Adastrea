# Quick Start: Blueprints for PIE (Play in Editor)

**⚡ Fast track to flying a spaceship in Adastrea**

---

## TL;DR - What You Need

✅ **Good News**: Most blueprints already exist! You just need to verify configuration.

### Essential Files (Already Created ✅)

| File | Location | Status |
|------|----------|--------|
| **Input Actions** | `Content/Input/Actions/` | ✅ Ready |
| **Input Mapping** | `Content/Input/IMC_Spaceship.uasset` | ✅ Ready |
| **Input Config** | `Content/DataAssets/Input/DA_InputConfig.uasset` | ⚙️ Verify |
| **Player Ship** | `Content/Blueprints/Ships/BP_PlayerShip.uasset` | ⚙️ Verify |
| **Controller** | `Content/Blueprints/Controllers/BP_SpaceshipController.uasset` | ⚙️ Verify |
| **Game Mode** | `Content/Blueprints/GameModes/BP_SpaceGameMode.uasset` | ⚙️ Verify |
| **Ship Data** | `Content/DataAssets/Ships/DA_Ship_PlayerScout.uasset` | ⚙️ Verify |

---

## 5-Minute Setup Checklist

### ✅ Step 1: Verify Input Configuration (2 min)

1. Open `Content/DataAssets/Input/DA_InputConfig`
2. Check these are assigned:
   - MoveAction → `IA_Move`
   - LookAction → `IA_Look`
   - BoostAction → `IA_Boost`
   - FirePrimaryAction → `IA_Fire_Primary`
   - SpaceshipMappingContext → `IMC_Spaceship`
3. Save if you made changes

📖 **Need help?** See [DA_INPUTCONFIG_VISUAL_GUIDE.md](DA_INPUTCONFIG_VISUAL_GUIDE.md) for detailed UE5.6 configuration guide with visual layouts.

### ✅ Step 2: Configure Game Mode (1 min)

1. Open `Content/Blueprints/GameModes/BP_SpaceGameMode`
2. Click **Class Defaults** button
3. Set these:
   - Default Pawn Class → **BP_PlayerShip**
   - Player Controller Class → **BP_SpaceshipController**
4. Compile and save

### ✅ Step 3: Verify Player Ship (1 min)

1. Open `Content/Blueprints/Ships/BP_PlayerShip`
2. Check **Components** panel has:
   - ✅ Static Mesh Component (with any mesh assigned)
   - ✅ Spring Arm Component
   - ✅ Camera Component
3. Check **Details** panel:
   - Ship Data Asset → `DA_Ship_PlayerScout`
4. Compile and save

### ✅ Step 4: Create Test Level (1 min)

**If TestLevel already exists**: Skip to Step 5

**If TestLevel missing**:
1. File → New Level → Empty Level
2. Add from **Place Actors** panel:
   - **Player Start** (drag into viewport)
   - **Directional Light**
   - **Sky Atmosphere**
3. Save as `Content/Maps/TestLevel`

### ✅ Step 5: Configure Project Settings (< 1 min)

1. Edit → Project Settings
2. **Maps & Modes**:
   - Default GameMode → `BP_SpaceGameMode`
   - Editor Startup Map → `TestLevel`
3. **Input**:
   - Ensure Enhanced Input is enabled

---

## 🚀 Test Flight!

1. Open **TestLevel** in editor
2. Press **Play** button (or Alt+P)
3. **Controls**:
   - **W** - Forward
   - **S** - Backward
   - **A** - Strafe left
   - **D** - Strafe right
   - **Mouse** - Look around
   - **Space** - Boost
   - **Left Click** - Fire weapon (if configured)
   - **ESC** - Pause menu

---

## ❌ Troubleshooting

### Ship doesn't spawn?
- Check **Player Start** is in TestLevel
- Verify Game Mode → Default Pawn Class = `BP_PlayerShip`

### Input doesn't work?
- Open `BP_SpaceshipController`
- Check Event BeginPlay initializes Enhanced Input
- Verify `IMC_Spaceship` is added to Input Subsystem

### Camera is inside ship?
- Open `BP_PlayerShip`
- Select **Spring Arm** component
- Increase **Target Arm Length** to 800-1000

### Ship doesn't move?
- Check `BP_PlayerShip` has **FloatingPawnMovement** component
- Verify input bindings are connected
- Look at **Output Log** for errors

---

## 📚 Next Steps

Once basic PIE works:

1. **Add HUD**: Open `Content/UI/HUD/WBP_SpaceShipHUD` and add UI elements
2. **Add Combat**: Create projectile blueprint and weapon effects
3. **Add Environment**: Place asteroids, space stations in level
4. **Add AI Ships**: Create NPC ship blueprints with basic AI
5. **Add Quests**: Create quest data assets and tracking UI

See **BLUEPRINT_PIE_CHECKLIST.md** for detailed next steps.

---

## 📖 Full Documentation

For comprehensive information:
- **BLUEPRINT_PIE_CHECKLIST.md** - Complete PIE setup guide
- **DA_INPUTCONFIG_VISUAL_GUIDE.md** - Visual guide for DA_InputConfig in UE5.6
- **BLUEPRINT_REQUIREMENTS_LIST.md** - All blueprints for full game
- **BLUEPRINT_CREATION_GUIDE.md** - Step-by-step creation instructions
- **CONTENT_CREATION_QUICKSTART.md** - 30-minute playable prototype

---

**🎮 Have fun flying! Remember: Start simple, iterate often.**
