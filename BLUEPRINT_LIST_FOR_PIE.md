# Blueprint List for Starting Game in PIE and Controlling a Spaceship

**Direct answer to: "Can you create a list of blueprints I need to create in the editor related to starting the game in PIE and controlling a spaceship?"**

---

## Executive Summary

✅ **Great News**: Most blueprints already exist in your repository! You mainly need to verify configuration and create a test level.

⏱️ **Time to PIE**: 5-15 minutes if blueprints are configured, 30 minutes if starting fresh

📚 **See Also**: 
- **BLUEPRINT_QUICK_START_PIE.md** - Quick 5-minute setup guide
- **BLUEPRINT_PIE_CHECKLIST.md** - Detailed checklist with troubleshooting
- **BLUEPRINT_PIE_DIAGRAM.md** - Visual diagrams and architecture

---

## Complete Blueprint List

### ✅ ALREADY EXIST (Verify Configuration)

These blueprints are already in your repository at `Content/`:

#### 1. Input System (CRITICAL - Already Exist)
| Blueprint | Location | Status | Action Needed |
|-----------|----------|--------|---------------|
| **IA_Move** | `Input/Actions/` | ✅ Exists | None - DO NOT DELETE |
| **IA_Look** | `Input/Actions/` | ✅ Exists | None - DO NOT DELETE |
| **IA_Boost** | `Input/Actions/` | ✅ Exists | None - DO NOT DELETE |
| **IA_Fire_Primary** | `Input/Actions/` | ✅ Exists | None - DO NOT DELETE |
| **IA_PauseMenu** | `Input/Actions/` | ✅ Exists | None - DO NOT DELETE |
| **IMC_Spaceship** | `Input/` | ✅ Exists | None - DO NOT DELETE |
| **DA_InputConfig** | `DataAssets/Input/` | ✅ Exists | ⚙️ **VERIFY** all actions assigned |

**Verification Steps for DA_InputConfig**:
1. Open `Content/DataAssets/Input/DA_InputConfig`
2. Ensure these are assigned:
   - MoveAction → IA_Move
   - LookAction → IA_Look
   - BoostAction → IA_Boost
   - FirePrimaryAction → IA_Fire_Primary
   - MenuAction → IA_PauseMenu
   - SpaceshipMappingContext → IMC_Spaceship

#### 2. Game Framework (CRITICAL - Already Exist)
| Blueprint | Location | Status | Action Needed |
|-----------|----------|--------|---------------|
| **BP_GameInstance** | `Blueprints/Core/` | ✅ Exists | ✅ Ready to use |
| **BP_SpaceGameMode** | `Blueprints/GameModes/` | ✅ Exists | ⚙️ **CONFIGURE** Class Defaults |
| **BP_SpaceshipController** | `Blueprints/Controllers/` | ✅ Exists | ⚙️ **VERIFY** BeginPlay |
| **BP_PlayerShip** | `Blueprints/Ships/` | ✅ Exists | ⚙️ **CONFIGURE** Components |

**Configuration Steps**:

**BP_SpaceGameMode**:
1. Open blueprint
2. Click "Class Defaults" button
3. Set "Default Pawn Class" → BP_PlayerShip
4. Set "Player Controller Class" → BP_SpaceshipController
5. Compile and save

**BP_SpaceshipController**:
1. Open blueprint
2. Check Event Graph → Event BeginPlay
3. Ensure Enhanced Input is initialized
4. Verify IMC_Spaceship is added to Input Subsystem

**BP_PlayerShip**:
1. Open blueprint
2. Verify Components exist:
   - Static Mesh Component (with any mesh/cube)
   - Spring Arm Component
   - Camera Component
   - FloatingPawnMovement Component
3. Set "Ship Data Asset" → DA_Ship_PlayerScout
4. Compile and save

#### 3. Ship Data (CRITICAL - Already Exists)
| Blueprint | Location | Status | Action Needed |
|-----------|----------|--------|---------------|
| **DA_Ship_PlayerScout** | `DataAssets/Ships/` | ✅ Exists | ⚙️ **VERIFY** stats filled in |

**Verification Steps**:
1. Open `Content/DataAssets/Ships/DA_Ship_PlayerScout`
2. Fill in if empty:
   - Display Name: "Player Scout"
   - Max Health: 1000
   - Max Shield: 500
   - Max Speed: 2000
   - Turn Rate: 50

#### 4. UI (HIGH PRIORITY - Already Exist)
| Blueprint | Location | Status | Action Needed |
|-----------|----------|--------|---------------|
| **WPB_SpaceShipHUD** | `UI/HUD/` | ✅ Exists | 🔧 May need UI elements |
| **WPB_MainMenu** | `UI/Menus/` | ✅ Exists | 🔧 May need buttons |

**Note**: HUD and menu can be enhanced later. Basic PIE works without fully designed UI.

---

### 🔨 NEEDS CREATION

Only ONE thing typically needs creation:

#### Test Level (REQUIRED - Usually Needs Creation)
| Item | Location | Status | Priority |
|------|----------|--------|----------|
| **TestLevel.umap** | `Content/Maps/` | 🔨 **CREATE** | ⚠️ CRITICAL |

**Creation Steps**:
1. File → New Level → Empty Level
2. Add from Place Actors panel:
   - **Player Start** actor (spawn point)
   - **Directional Light** (lighting)
   - **Sky Atmosphere** (space background)
   - **Sky Light** (ambient light)
3. (Optional) Add Static Mesh cubes/spheres for environment
4. Save as `Content/Maps/TestLevel`

**Time**: 2-3 minutes

---

### ⚙️ PROJECT SETTINGS (REQUIRED)

Not blueprints, but essential configuration:

#### Maps & Modes
```
Edit → Project Settings → Maps & Modes

Settings to change:
- Default GameMode: BP_SpaceGameMode
- Editor Startup Map: TestLevel
- Game Default Map: TestLevel
```

#### Input Settings
```
Edit → Project Settings → Input

Verify:
- Default Player Input Class: EnhancedPlayerInput
- Default Input Component Class: EnhancedInputComponent
- Enhanced Input plugin enabled
```

#### Game Instance
```
Edit → Project Settings → Engine → General Settings

Setting:
- Game Instance Class: BP_GameInstance
```

**Time**: 1-2 minutes

---

## Minimal Blueprint Count

### To Answer Your Question Directly:

**Blueprints that exist and need verification: 8**
1. DA_InputConfig (Input configuration)
2. BP_GameInstance (Game instance)
3. BP_SpaceGameMode (Game mode)
4. BP_SpaceshipController (Player controller)
5. BP_PlayerShip (Player spaceship)
6. DA_Ship_PlayerScout (Ship stats)
7. WPB_SpaceShipHUD (HUD widget - optional for basic PIE)
8. WPB_MainMenu (Main menu - optional for basic PIE)

**Blueprints to create: 1**
1. TestLevel.umap (Level/map)

**Input assets (not blueprints, but required): 6**
1. IA_Move (already exists)
2. IA_Look (already exists)
3. IA_Boost (already exists)
4. IA_Fire_Primary (already exists)
5. IA_PauseMenu (already exists)
6. IMC_Spaceship (already exists)

**Total time to set up**: 5-15 minutes

---

## Step-by-Step Quick Start

### Phase 1: Verify Input (2 min)
1. ✅ Open `DA_InputConfig`
2. ✅ Check all Input Actions are assigned
3. ✅ Save

### Phase 2: Configure Game Mode (1 min)
1. ✅ Open `BP_SpaceGameMode`
2. ✅ Set Default Pawn = BP_PlayerShip
3. ✅ Set Player Controller = BP_SpaceshipController
4. ✅ Compile and save

### Phase 3: Configure Ship (2 min)
1. ✅ Open `BP_PlayerShip`
2. ✅ Verify components (Mesh, Camera, Spring Arm)
3. ✅ Set Ship Data Asset = DA_Ship_PlayerScout
4. ✅ Compile and save

### Phase 4: Create Level (2 min)
1. ✅ File → New Level → Empty Level
2. ✅ Add Player Start
3. ✅ Add Directional Light, Sky Atmosphere, Sky Light
4. ✅ Save as TestLevel

### Phase 5: Project Settings (1 min)
1. ✅ Set Default GameMode = BP_SpaceGameMode
2. ✅ Set Editor Startup Map = TestLevel
3. ✅ Verify Enhanced Input is enabled

### Phase 6: Test (< 1 min)
1. ✅ Open TestLevel
2. ✅ Press Play button
3. ✅ Fly with WASD, mouse, and Space!

---

## Controls After PIE Start

Once in game:
- **W** - Forward
- **S** - Backward
- **A** - Strafe left
- **D** - Strafe right
- **Mouse** - Look around (camera)
- **Space** - Boost (faster speed)
- **Left Click** - Fire primary weapon (if configured)
- **ESC** - Pause menu (if configured)

---

## Troubleshooting Checklist

If PIE doesn't work:

### Ship Doesn't Spawn
- [ ] Player Start is in TestLevel?
- [ ] BP_SpaceGameMode → Default Pawn Class = BP_PlayerShip?
- [ ] Project Settings → Default GameMode = BP_SpaceGameMode?

### Input Doesn't Work
- [ ] DA_InputConfig has all actions assigned?
- [ ] BP_SpaceshipController initializes Enhanced Input in BeginPlay?
- [ ] IMC_Spaceship added to Input Subsystem?
- [ ] Enhanced Input plugin enabled in Project Settings?

### Camera Issues
- [ ] BP_PlayerShip has Camera Component?
- [ ] Camera attached to Spring Arm?
- [ ] Spring Arm Target Arm Length = 800-1000?

### Ship Doesn't Move
- [ ] BP_PlayerShip has FloatingPawnMovement component?
- [ ] Input bindings connected?
- [ ] Check Output Log for errors?

---

## What You DON'T Need to Create

❌ You DO NOT need to create:
- New Input Actions (already exist)
- Input Mapping Context (already exists)
- Player Controller (already exists)
- Player Ship blueprint (already exists)
- Game Mode blueprint (already exists)
- Game Instance (already exists)
- Ship Data Asset (already exists)

✅ You ONLY need to:
- Verify existing blueprints are configured correctly
- Create TestLevel (if missing)
- Configure Project Settings

---

## Next Steps After Basic PIE Works

Once you can fly the ship:

### Immediate Next Steps:
1. **Add HUD elements** - Open WPB_SpaceShipHUD and add health/shield bars
2. **Add combat** - Create projectile blueprint and weapon firing
3. **Add environment** - Place asteroids and space stations
4. **Add enemy ships** - Create NPC ship blueprints

### Future Expansions:
5. Add trading system (stations, markets, goods)
6. Add faction system (relationships, diplomacy)
7. Add quest system (missions, objectives)
8. Add progression (unlocks, upgrades)

---

## Documentation Cross-Reference

**Quick Guides**:
- **BLUEPRINT_QUICK_START_PIE.md** - 5-minute setup (this is the fastest guide)
- **BLUEPRINT_PIE_CHECKLIST.md** - Complete detailed checklist
- **BLUEPRINT_PIE_DIAGRAM.md** - Visual diagrams and flowcharts

**Complete References**:
- **BLUEPRINT_REQUIREMENTS_LIST.md** - All blueprints for full game (100+)
- **BLUEPRINT_CREATION_GUIDE.md** - Step-by-step creation instructions
- **CONTENT_CREATION_QUICKSTART.md** - 30-minute playable prototype
- **ZZ_ESSENTIAL_BLUEPRINTS_ONLY.md** - Minimal C++ approach

**System Documentation**:
- **Assets/EnhancedInputImplementation.md** - Enhanced Input system details
- **Assets/SpaceshipTemplates/** - Ship configuration templates
- **Assets/BlueprintWorkflowTemplates.md** - Blueprint patterns

---

## Summary

**To answer your original question**:

You need to work with **9 blueprints/assets** total:
- **8 already exist** (just need verification/configuration)
- **1 needs creation** (TestLevel map)

**Time investment**: 5-15 minutes for basic PIE, 30 minutes for polished experience

**Complexity**: LOW - Most work is configuration, not creation

**Result**: Flyable spaceship in 3D space with full control!

---

**Good luck and happy flying! 🚀**

For questions or issues, check the troubleshooting section or review the detailed guides listed above.
