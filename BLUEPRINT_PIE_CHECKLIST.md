# Blueprint Checklist for PIE (Play in Editor) - Adastrea

**Goal**: Create the minimum blueprints needed to start the game in PIE and control a spaceship.

**Estimated Time**: 15-30 minutes for minimum viable setup

---

## Status Legend
- ✅ **EXISTS** - Already created in the repository
- 🔨 **NEEDS CREATION** - Must be created in Unreal Editor
- ⚙️ **NEEDS CONFIGURATION** - Exists but needs setup

---

## Phase 1: Essential Input System (CRITICAL)

### Input Actions (Already Exist ✅)
These files already exist in `Content/Input/Actions/`:
- ✅ **IA_Move** - Movement input (Vector2D)
- ✅ **IA_Look** - Camera look input (Vector2D)
- ✅ **IA_Boost** - Speed boost (Boolean)
- ✅ **IA_Fire_Primary** - Primary weapon (Boolean)
- ✅ **IA_PauseMenu** - Pause menu (Boolean)

**Location**: `Content/Input/Actions/`
**Status**: ✅ DO NOT DELETE - Already configured and ready to use

📖 **For UE5.6 Key Mapping Guide**: See [IMC_SPACESHIP_VISUAL_GUIDE.md](IMC_SPACESHIP_VISUAL_GUIDE.md) for detailed key binding configuration with visual examples.

### Input Mapping Context (Already Exists ✅)
- ✅ **IMC_Spaceship** - Maps input actions to keyboard/mouse keys

**Location**: `Content/Input/IMC_Spaceship.uasset`
**Status**: ✅ Already configured with:
  - WASD for movement
  - Mouse for camera
  - Space for boost
  - Left mouse button for fire
  - ESC for pause

### Input Config Data Asset (Already Exists ✅)
- ✅ **DA_InputConfig** - Central configuration linking all input actions

**Location**: `Content/DataAssets/Input/DA_InputConfig.uasset`
**Status**: ⚙️ **VERIFY CONFIGURATION** - Open and ensure all Input Actions are properly assigned:
  - MoveAction → IA_Move
  - LookAction → IA_Look
  - BoostAction → IA_Boost
  - FirePrimaryAction → IA_Fire_Primary
  - MenuAction → IA_PauseMenu
  - SpaceshipMappingContext → IMC_Spaceship

📖 **For UE5.6 Visual Setup Guides**:
- [DA_INPUTCONFIG_VISUAL_GUIDE.md](DA_INPUTCONFIG_VISUAL_GUIDE.md) - DA_InputConfig configuration with visual layout diagrams
- [IMC_SPACESHIP_VISUAL_GUIDE.md](IMC_SPACESHIP_VISUAL_GUIDE.md) - IMC_Spaceship key mapping configuration

---

## Phase 2: Core Game Framework (CRITICAL)

### Player Ship Blueprint (Already Exists ✅)
- ✅ **BP_PlayerShip** - The player's controllable spaceship

**Location**: `Content/Blueprints/Ships/BP_PlayerShip.uasset`
**Parent Class**: `ASpaceship` (C++)
**Status**: ⚙️ **VERIFY COMPONENTS** - Open and check:

**Required Components**:
1. Static Mesh Component (ship visual)
2. Spring Arm Component (camera boom)
3. Camera Component (player camera)
4. FloatingPawnMovement Component (inherited from Spaceship)
5. Arrow Component (projectile spawn point - optional)

**Required Variables** (should be configured):
- ShipDataAsset: Reference to `DA_Ship_PlayerScout`
- Speed and rotation rates
- Health and shield values

**Action Needed**:
1. Open `BP_PlayerShip`
2. Verify Static Mesh is assigned (use placeholder cube if no mesh)
3. Check Camera is properly positioned behind ship
4. Verify ShipDataAsset points to `DA_Ship_PlayerScout`
5. Test compile - should have no errors

### Player Controller Blueprint (Already Exists ✅)
- ✅ **BP_SpaceshipController** - Handles player input and HUD

**Location**: `Content/Blueprints/Controllers/BP_SpaceshipController.uasset`
**Parent Class**: `AdastreaPlayerController` (C++)
**Status**: ⚙️ **VERIFY EVENT GRAPH** - Open and check Event BeginPlay:

**Required Setup in Event BeginPlay**:
1. Get Enhanced Input Subsystem
2. Add Mapping Context (IMC_Spaceship)
3. Create HUD Widget (WBP_SpaceshipHUD)
4. Add HUD to Viewport

**Action Needed**:
1. Open `BP_SpaceshipController`
2. Check Event Graph → Event BeginPlay
3. Ensure Enhanced Input is initialized
4. Verify HUD creation (can be added later if not present)

### Game Mode Blueprint (Already Exists ✅)
- ✅ **BP_SpaceGameMode** - Main game mode controlling game rules

**Location**: `Content/Blueprints/GameModes/BP_SpaceGameMode.uasset`
**Parent Class**: `AdastreaGameMode` (C++)
**Status**: ⚙️ **VERIFY CLASS DEFAULTS** - Open Class Defaults:

**Required Settings**:
- Default Pawn Class: **BP_PlayerShip**
- Player Controller Class: **BP_SpaceshipController**
- Game State Class: AAdastreaGameState (C++)
- HUD Class: None (using Widget-based HUD)

**Action Needed**:
1. Open `BP_SpaceGameMode`
2. Click "Class Defaults" button in toolbar
3. Set Default Pawn Class to BP_PlayerShip
4. Set Player Controller Class to BP_SpaceshipController
5. Compile and save

### Game Instance Blueprint (Already Exists ✅)
- ✅ **BP_GameInstance** - Persistent game instance for save/load

**Location**: `Content/Blueprints/Core/BP_GameInstance.uasset`
**Parent Class**: `AdastreaGameInstance` (C++)
**Status**: ✅ Should work as-is, no action needed initially

---

## Phase 3: UI/HUD (HIGH PRIORITY)

### HUD Widget (Already Exists ✅)
- ✅ **WBP_SpaceShipHUD** - Main heads-up display

**Location**: `Content/UI/HUD/WBP_SpaceShipHUD.uasset`
**Parent Class**: `UAdastreaHUDWidget` (C++)
**Status**: ⚙️ **MAY NEED DESIGN** - Check if widget has visual elements

**Minimum Required Elements**:
1. Canvas Panel (root)
2. Text - Speed indicator
3. Text - Ship name
4. Progress Bar - Health
5. Progress Bar - Shield
6. Image - Crosshair (center)

**Action Needed**:
1. Open `WBP_SpaceShipHUD`
2. If empty, add basic Canvas Panel
3. Add simple text element to verify it displays
4. Can add full UI elements later

### Main Menu Widget (Already Exists ✅)
- ✅ **WBP_MainMenu** - Main menu for game start

**Location**: `Content/UI/Menus/WBP_MainMenu.uasset`
**Parent Class**: `UUserWidget`
**Status**: ⚙️ **MAY NEED BUTTONS** - Check if menu has functional buttons

**Minimum Required Elements**:
1. Canvas Panel (root)
2. Button - Start Game (opens test level)
3. Button - Quit Game

**Action Needed**:
1. Open `WBP_MainMenu`
2. Add Start Game button with OnClicked event
3. OnClicked → Open Level ("TestLevel")
4. Can be enhanced later

---

## Phase 4: Ship Data Asset (CRITICAL)

### Player Ship Data Asset (Already Exists ✅)
- ✅ **DA_Ship_PlayerScout** - Ship stats and configuration

**Location**: `Content/DataAssets/Ships/DA_Ship_PlayerScout.uasset`
**Parent Class**: `USpaceshipDataAsset` (C++)
**Status**: ⚙️ **VERIFY STATS** - Should have default values

**Required Properties**:
- Display Name: "Player Scout"
- Description: "Starting ship for players"
- Core Stats: Health, Shield, Speed values
- Combat Stats: Weapon mounts, defense values
- Mobility Stats: Turn rates, acceleration

**Action Needed**:
1. Open `DA_Ship_PlayerScout`
2. Fill in Display Name and Description
3. Set reasonable default values for stats:
   - Max Health: 1000
   - Max Shield: 500
   - Max Speed: 2000
   - Turn Rate: 50
4. Save

---

## Phase 5: Level Setup (REQUIRED)

### Test Level Map 🔨
- 🔨 **TestLevel** - Basic playable level

**Location**: `Content/Maps/TestLevel.umap` (needs creation if missing)
**Status**: 🔨 **NEEDS CREATION OR VERIFICATION**

**Required Elements in Level**:
1. **Player Start** actor (spawn point)
2. **Directional Light** (sun/lighting)
3. **Sky Atmosphere** (space background)
4. **Sky Light** (ambient lighting)
5. (Optional) Static mesh cubes/spheres as asteroids/obstacles

**Action Needed**:
1. Check if `Content/Maps/TestLevel.umap` exists
2. If not: File → New Level → Empty Level
3. Add Player Start actor from Place Actors panel
4. Add Directional Light
5. Add Sky Atmosphere
6. Add Sky Light
7. (Optional) Add a few static mesh actors for environment
8. Save as `TestLevel` in `Content/Maps/`

### Main Menu Level 🔨
- 🔨 **MainMenu** - Level for main menu screen

**Location**: `Content/Maps/MainMenu.umap` (needs creation if missing)
**Status**: 🔨 **NEEDS CREATION** (can skip for PIE testing)

**Simple Setup**:
1. File → New Level → Empty Level
2. Add Directional Light
3. Add Sky Atmosphere
4. Save as `MainMenu` in `Content/Maps/`

**Note**: For PIE testing, you can skip this and directly play from TestLevel.

---

## Phase 6: Project Settings Configuration (CRITICAL)

After creating/verifying all blueprints, configure Project Settings:

### Maps & Modes Settings ⚙️
1. **Edit → Project Settings → Maps & Modes**
2. Set the following:

```
Game Default Map: TestLevel (or MainMenu if created)
Editor Startup Map: TestLevel
Server Default Map: TestLevel

Default Modes:
- Default GameMode: BP_SpaceGameMode
- Selected GameMode: BP_SpaceGameMode
```

### Input Settings ⚙️
1. **Edit → Project Settings → Input**
2. Verify:

```
Default Classes:
- Default Player Input Class: EnhancedPlayerInput
- Default Input Component Class: EnhancedInputComponent

Make sure "Enhanced Input" is enabled in plugins if not already.
```

### Engine Settings ⚙️
1. **Edit → Project Settings → Engine → General Settings**
2. Set:

```
Game Instance Class: BP_GameInstance
```

---

## Quick Start Testing Checklist

Once you've completed the above, test in PIE:

### Pre-Flight Checklist
- [ ] DA_InputConfig has all Input Actions assigned
- [ ] IMC_Spaceship exists and has key mappings
- [ ] BP_PlayerShip has Static Mesh and Camera components
- [ ] BP_PlayerShip references DA_Ship_PlayerScout
- [ ] BP_SpaceshipController initializes Enhanced Input in BeginPlay
- [ ] BP_SpaceGameMode sets BP_PlayerShip as Default Pawn
- [ ] TestLevel has Player Start actor
- [ ] Project Settings → Default GameMode = BP_SpaceGameMode
- [ ] Project Settings → Editor Startup Map = TestLevel

### PIE Testing Steps
1. **Open TestLevel** in Unreal Editor
2. **Click Play** button (or press Alt+P)
3. **Verify**:
   - [ ] Player ship spawns at Player Start location
   - [ ] Camera follows ship from behind
   - [ ] WASD keys move the ship (W=forward, S=backward, A/D=strafe)
   - [ ] Mouse moves camera (look around)
   - [ ] Space bar activates boost (ship goes faster)
   - [ ] Left mouse button fires weapon (if configured)
   - [ ] ESC opens pause menu (if configured)
4. **Check Output Log** for any errors

---

## Common Issues and Solutions

### Issue: Ship doesn't spawn
**Solution**:
- Check Player Start is in the level
- Verify BP_SpaceGameMode → Default Pawn Class = BP_PlayerShip
- Check Project Settings → Default GameMode = BP_SpaceGameMode

### Issue: Input doesn't work
**Solution**:
- Verify Enhanced Input is initialized in BP_SpaceshipController BeginPlay
- Check IMC_Spaceship is added to Input Subsystem
- Ensure DA_InputConfig has all actions assigned
- Verify Input Actions exist in Content/Input/Actions/

### Issue: Ship spawns but camera is inside ship
**Solution**:
- Open BP_PlayerShip
- Select Camera Component
- Adjust position (move backward and up from ship)
- Or adjust Spring Arm length (increase to 600-1000)

### Issue: Ship doesn't move
**Solution**:
- Check Input binding in BP_PlayerShip or C++ code
- Verify Enhanced Input Actions are triggering (use Print String to debug)
- Check FloatingPawnMovement component is present

### Issue: Compilation errors
**Solution**:
- Check all parent classes are correct (C++ classes must exist)
- Verify all referenced assets exist
- Look at Output Log for specific error messages
- Try deleting Intermediate and Saved folders, then regenerate project

---

## Minimum Viable Product (MVP) Summary

**To get spaceship control working in PIE, you need**:

### Must Have (Already Exist ✅):
1. ✅ Input Actions (IA_Move, IA_Look, IA_Boost, IA_Fire_Primary)
2. ✅ Input Mapping Context (IMC_Spaceship)
3. ✅ DA_InputConfig Data Asset
4. ✅ BP_PlayerShip Blueprint
5. ✅ BP_SpaceshipController Blueprint
6. ✅ BP_SpaceGameMode Blueprint
7. ✅ BP_GameInstance Blueprint
8. ✅ DA_Ship_PlayerScout Data Asset

### May Need Creation:
1. 🔨 TestLevel map (if missing)
2. ⚙️ HUD Widget content (if empty)

### Must Configure:
1. ⚙️ DA_InputConfig - Assign all Input Actions
2. ⚙️ BP_PlayerShip - Assign ShipDataAsset, configure components
3. ⚙️ BP_SpaceshipController - Initialize Enhanced Input in BeginPlay
4. ⚙️ BP_SpaceGameMode - Set Default Pawn and Controller classes
5. ⚙️ TestLevel - Add Player Start and lighting
6. ⚙️ Project Settings - Set Game Mode and Input classes

---

## Next Steps After Basic PIE Works

Once you have basic movement working:

### Phase A: Add Combat
1. Create projectile Blueprint (BP_Projectile_Laser)
2. Add weapon firing logic to BP_PlayerShip
3. Create target dummy actors
4. Add damage system

### Phase B: Add HUD
1. Enhance WBP_SpaceShipHUD with full UI elements
2. Bind health/shield bars to ship values
3. Add speed indicator
4. Add crosshair and target reticle

### Phase C: Add Enemy Ships
1. Create BP_Ship_NPC_Fighter Blueprint
2. Add basic AI controller
3. Create patrol behavior
4. Add combat AI

### Phase D: Add Trading System
1. Create space station Blueprint
2. Add docking functionality
3. Create trading UI widget
4. Add trade item Data Assets

### Phase E: Add Quests
1. Create quest Data Assets
2. Add quest UI widget
3. Create quest tracking system
4. Add objective markers

---

## Additional Resources

### Documentation
- **BLUEPRINT_REQUIREMENTS_LIST.md** - Complete blueprint list for full game
- **BLUEPRINT_CREATION_GUIDE.md** - Step-by-step blueprint creation
- **CONTENT_CREATION_QUICKSTART.md** - 30-minute playable guide
- **CURRENT_STATUS.md** - Realistic project status assessment
- **Assets/EnhancedInputImplementation.md** - Input system details

### Templates
- **Assets/SpaceshipTemplates/** - YAML ship templates
- **Assets/BlueprintWorkflowTemplates.md** - Blueprint patterns

### Automation
- **SetupCheck.py** - Validate project setup
- **SmokeTest.py** - Automated testing

---

## Final Notes

**Good News**: Most essential blueprints already exist in your repository! You mainly need to:
1. ✅ Verify they're configured correctly
2. 🔨 Create a test level if missing
3. ⚙️ Configure Project Settings

**Time Estimate**:
- If blueprints are already configured: 5-10 minutes (just test level + settings)
- If blueprints need configuration: 15-20 minutes
- If starting from scratch: 30-45 minutes

**Priority Focus**: Get PIE working first with basic movement, then iterate and add features.

---

## Version History
- **1.0** (December 5, 2025) - Initial PIE-focused checklist

---

**Need help?** Check the Output Log in Unreal Editor for specific error messages, or review the detailed guides in the Assets/ folder.
