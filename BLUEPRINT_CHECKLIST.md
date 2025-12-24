# Blueprint Creation Checklist - Adastrea

> **Quick checklist for creating required Blueprints in Unreal Engine Editor**

**New to Blueprints?** Start with [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md) for a complete beginner-friendly guide!

**For detailed instructions, see**: `BLUEPRINT_REQUIREMENTS_LIST.md`

> **✨ Status Update (November 18, 2025)**: Phase 1 is 80% complete! Most core Blueprints have been created. See the [Progress Tracking](#-progress-tracking) section for details.

---

## 🚨 Phase 1: Minimum Required (15 Blueprints)

**Goal**: Get a basic playable game running  
**Time**: ~2 hours  
**Result**: Player can spawn, move, and see UI

### Game Framework (5)
- [x] **BP_SpaceGameMode** (`Blueprints/GameModes/`) ✅
  - Parent: `AdastreaGameMode`
  - Set: Default Pawn, Player Controller
  
- [x] **BP_GameInstance** (`Blueprints/Core/`) ✅
  - Parent: `AdastreaGameInstance`
  - Set: PlayerName, CurrentCredits, SaveSlotName
  
- [x] **BP_SpaceshipController** (`Blueprints/Controllers/`) ✅
  - Parent: `AdastreaPlayerController`
  - Add: Enhanced Input setup, HUD creation
  
- [x] **BP_PlayerShip** (`Blueprints/Ships/`) ✅
  - Parent: `ASpaceship`
  - Add: Mesh, Camera, Spring Arm, Combat components
  - Set: ShipDataAsset reference
  
- [x] **WBP_SpaceshipHUD** (`UI/HUD/`) ✅
  - Parent: `UAdastreaHUDWidget`
  - Add: Ship name, speed, health bar, shield bar, crosshair
  - **Note**: Created as `WPB_SpaceShipHUD`

### UI (2)
- [x] **WBP_MainMenu** (`UI/Menus/`) ✅
  - Add: Start Game, Load, Settings, Quit buttons
  - **Note**: Created as `WPB_MainMenu`
  
- [ ] **WBP_PauseMenu** (`UI/Menus/`)
  - Add: Resume, Settings, Save, Main Menu buttons
  - **Note**: Input action `IA_PauseMenu` exists, widget pending

### Data Assets (3)
- [x] **DA_Ship_PlayerScout** (`DataAssets/Ships/`) ✅
  - Parent: `USpaceshipDataAsset`
  - Use template: `Assets/SpaceshipTemplates/PathfinderScout.yaml`
  
- [ ] **DA_Faction_PlayerAlliance** (`DataAssets/Factions/`)
  - Parent: `UFactionDataAsset`
  - Set: Name, colors, tech level
  - **Note**: `DA_Way_TradersGuild` exists (Way system)
  
- [x] **DA_InputConfig** (`DataAssets/Input/`) ✅
  - Parent: `UInputConfigDataAsset`

### Enhanced Input (5)
- [x] **IMC_Spaceship** (`Input/`) - Input Mapping Context ✅
  - Map all actions to keys
  
- [x] **IA_Move** (`Input/Actions/`) - Vector2D ✅
- [x] **IA_Look** (`Input/Actions/`) - Vector2D ✅
- [x] **IA_Boost** (`Input/Actions/`) - Boolean ✅
- [x] **IA_Fire_Primary** (`Input/Actions/`) - Boolean ✅
- [x] **IA_PauseMenu** (`Input/Actions/`) - Boolean ✅ (Bonus)

### Project Settings
- [ ] Set Default GameMode: `BP_SpaceGameMode` (requires editor verification)
- [ ] Set Game Instance: `BP_GameInstance` (requires editor verification)
- [x] Set Game Default Map: `MainMenu` ✅ (MainMenu.umap exists)
- [ ] Enable Enhanced Input System (requires editor verification)

---

## 🔶 Phase 2: High Priority (Additional 25 Blueprints)

**Goal**: Add combat, trading, and core systems  
**Time**: ~4 hours  
**Result**: Full gameplay loop functional

### Stations (5)
- [ ] **BP_SpaceStation_Main** (`Blueprints/Stations/`)
- [ ] **BP_Module_Docking** (`Blueprints/Stations/Modules/`)
- [ ] **BP_Module_Power** (`Blueprints/Stations/Modules/`)
- [ ] **BP_Module_Storage** (`Blueprints/Stations/Modules/`)
- [ ] **BP_Module_Defence** (`Blueprints/Stations/Modules/`)

### Combat (5)
- [ ] **DA_Weapon_Laser** (`DataAssets/Weapons/`)
- [ ] **DA_Weapon_Cannon** (`DataAssets/Weapons/`)
- [ ] **DA_Weapon_Missile** (`DataAssets/Weapons/`)
- [ ] **BP_Projectile_Laser** (`Blueprints/Combat/Projectiles/`)
- [ ] **BP_Projectile_Cannon** (`Blueprints/Combat/Projectiles/`)

### Ships (3)
- [ ] **DA_Ship_PlayerFighter** (`DataAssets/Ships/`)
- [ ] **DA_Ship_NPC_Fighter** (`DataAssets/Ships/`)
- [ ] **BP_Ship_NPC_Fighter** (`Blueprints/Ships/NPCs/`)

### Trading (5)
- [ ] **WBP_TradingInterface** (`UI/Trading/`)
- [ ] **DA_TradeItem_Food** (`DataAssets/Trading/Items/`)
- [ ] **DA_TradeItem_Minerals** (`DataAssets/Trading/Items/`)
- [ ] **DA_TradeItem_Technology** (`DataAssets/Trading/Items/`)
- [ ] **DA_Market_Station1** (`DataAssets/Trading/Markets/`)

### Additional Systems (7)
- [ ] **WBP_InventoryWidget** (`UI/Inventory/`)
- [ ] **DA_Faction_Merchants** (`DataAssets/Factions/`)
- [ ] **DA_Faction_Pirates** (`DataAssets/Factions/`)
- [ ] **DA_Quest_Tutorial_FirstFlight** (`DataAssets/Quests/`)
- [ ] **DA_Homeworld_EarthStation** (`DataAssets/Homeworlds/`)
- [ ] **DA_Personnel_Captain** (`DataAssets/Personnel/`)
- [ ] **DA_Personnel_Pilot** (`DataAssets/Personnel/`)

---

## 🔷 Phase 3: Complete Systems (Additional 60+ Blueprints)

**Goal**: All features fully functional  
**Time**: ~8-10 hours  
**Result**: Production-ready game

### All Station Modules (10)
- [ ] BP_Module_Habitation
- [ ] BP_Module_Command
- [ ] BP_Module_Industrial
- [ ] BP_Module_Trade
- [ ] BP_Module_Research
- [ ] BP_Module_Entertainment

### All Ship Types (8)
- [ ] DA_Ship_PlayerTrader
- [ ] DA_Ship_NPC_Trader
- [ ] DA_Ship_NPC_Hauler
- [ ] DA_Ship_NPC_Scout
- [ ] BP_Ship_NPC_Trader
- [ ] BP_Ship_NPC_Hauler
- [ ] BP_Ship_NPC_Scout

### All UI Widgets (10)
- [ ] WBP_SettingsMenu
- [ ] WBP_StationManagement
- [ ] WBP_StationEditor
- [ ] WBP_ShipCustomization
- [ ] WBP_QuestLog
- [ ] WBP_SectorMap
- [ ] WBP_CharacterCreation
- [ ] WBP_DialogueInterface
- [ ] WBP_TutorialOverlay
- [ ] WBP_AchievementNotification

### Complete Input Actions (15)
- [ ] IA_TargetNext
- [ ] IA_TargetPrevious
- [ ] IA_Fire_Secondary
- [ ] IA_Autopilot
- [ ] IA_ToggleMap
- [ ] IA_OpenInventory
- [ ] IA_OpenQuestLog
- [ ] IA_Interact
- [ ] IA_Dock
- [ ] IA_Jump
- [ ] IA_ScanTarget
- [ ] IA_CycleWeapons
- [ ] IA_ToggleShields
- [ ] IA_EmergencyWarp
- [ ] IA_Screenshot

### All Data Assets (30+)
- [ ] Complete faction set (5 more factions)
- [ ] Complete personnel roles (25 more roles)
- [ ] Complete trade items (10+ items)
- [ ] Complete weapon types (5+ weapons)
- [ ] Complete quests (10+ quests)
- [ ] Complete tutorial steps (5+ steps)
- [ ] Complete materials (8+ materials)
- [ ] Complete homeworlds (3+ homeworlds)
- [ ] Achievement definitions (10+ achievements)
- [ ] Ship upgrades (10+ upgrades)
- [ ] Audio tracks (5+ music, 10+ SFX)

### Environment & Effects (8)
- [ ] BP_Asteroid
- [ ] BP_Planet
- [ ] BP_Nebula
- [ ] BP_Explosion
- [ ] BP_EngineTrail
- [ ] BP_ShieldEffect
- [ ] BP_WarpEffect
- [ ] BP_DamageEffect

---

## 📋 Quick Test Checklist

After each phase, verify:

### Phase 1 Tests
- [ ] Game starts without errors
- [ ] Main menu displays
- [ ] Can start new game
- [ ] Player ship spawns
- [ ] WASD moves ship
- [ ] Mouse looks around
- [ ] HUD displays information
- [ ] Can pause game
- [ ] Can quit to main menu

### Phase 2 Tests
- [ ] Can approach station
- [ ] Trading UI opens
- [ ] Can buy items
- [ ] Can sell items
- [ ] Inventory updates
- [ ] Can fire weapons
- [ ] Projectiles spawn
- [ ] Damage is dealt
- [ ] NPC ships appear
- [ ] Factions have relationships

### Phase 3 Tests
- [ ] All menus functional
- [ ] Quest tracking works
- [ ] Map displays sectors
- [ ] Save/load works
- [ ] Tutorial guides player
- [ ] NPCs trade autonomously
- [ ] Station editor works
- [ ] Ship customization works
- [ ] Audio plays correctly
- [ ] All systems integrate smoothly

---

## 🎯 Recommended Order

**Week 1**: Phase 1 (Minimum Required)
- Days 1-2: Game framework and player ship
- Day 3: UI and HUD
- Day 4: Input system
- Day 5: Testing and polish

**Week 2**: Phase 2 (High Priority)
- Days 1-2: Combat system
- Days 3-4: Trading and stations
- Day 5: Testing and integration

**Week 3-4**: Phase 3 (Complete Systems)
- Week 3: All data assets and UI
- Week 4: Environment, effects, and polish

---

## 📁 Folder Setup

Before creating Blueprints, create these folders:

```
Content/
├── Blueprints/
│   ├── Core/
│   ├── GameModes/
│   ├── Controllers/
│   ├── Ships/
│   │   └── NPCs/
│   ├── Stations/
│   │   └── Modules/
│   ├── Combat/
│   │   └── Projectiles/
│   ├── Environment/
│   ├── AI/
│   └── Components/
├── DataAssets/
│   ├── Ships/
│   ├── Factions/
│   ├── Personnel/
│   ├── Trading/
│   │   ├── Items/
│   │   └── Markets/
│   ├── Weapons/
│   ├── Quests/
│   ├── Homeworlds/
│   ├── Materials/
│   ├── Input/
│   ├── Tutorials/
│   └── Audio/
├── UI/
│   ├── HUD/
│   ├── Menus/
│   ├── Trading/
│   ├── Stations/
│   ├── Inventory/
│   ├── Ships/
│   ├── Quests/
│   └── Map/
└── Input/
    └── Actions/
```

---

## 🔧 Tools to Help

### Python Scripts (Run from project root)
```bash
# Validate setup
python SetupCheck.py

# Generate YAML templates
python TemplateGenerator.py

# Import YAML to Data Assets
python Tools/YAMLtoDataAsset.py

# Procedural content generation
python ProceduralGenerators.py

# Scene population
python ScenePopulator.py
```

### Documentation
- **BLUEPRINT_REQUIREMENTS_LIST.md** - Complete detailed list
- **BLUEPRINT_CREATION_GUIDE.md** - Step-by-step instructions
- **CONTENT_CREATION_QUICKSTART.md** - 30-minute quickstart
- **Assets/[System]Guide.md** - System-specific guides

---

## ⚠️ Common Mistakes to Avoid

- ❌ Forgetting to set parent class correctly
- ❌ Not adding Enhanced Input Mapping Context in BeginPlay
- ❌ Missing Data Asset references on ships
- ❌ Not setting Default Pawn Class in Game Mode
- ❌ Forgetting to add HUD widget to viewport
- ❌ Not configuring Project Settings
- ❌ Creating Blueprints without compiling first
- ❌ Not testing after each phase

---

## 💡 Tips

- ✅ Create one Blueprint at a time and test it
- ✅ Use existing C++ classes as parents
- ✅ Reference YAML templates for data assets
- ✅ Save frequently (Ctrl+S)
- ✅ Compile after each change
- ✅ Check Output Log for errors
- ✅ Use descriptive names with prefixes (BP_, WBP_, DA_, IA_)
- ✅ Organize in proper folders
- ✅ Keep backups before major changes

---

## 📊 Progress Tracking

Track your progress:

```
Phase 1: [████████░░] 12/15 Blueprints (80% Complete)
  - Game Framework: 5/5 ✅
  - UI: 1/2 (WBP_PauseMenu pending)
  - Data Assets: 2/3 (DA_Faction_PlayerAlliance pending)
  - Enhanced Input: 5/5 ✅ (+1 bonus)
  - Project Settings: 1/4 (3 require editor verification)

Phase 2: [    ] 0/25 Blueprints  
Phase 3: [    ] 0/60+ Blueprints

Total: [██░░░░░░░░] 12/100+ Blueprints (12% Complete)

**Latest Status**: Phase 1 nearly complete! Missing only WBP_PauseMenu and DA_Faction_PlayerAlliance.
```

---

**Need help?** See README.md or open an issue on GitHub

**Last Updated**: November 18, 2025  
**Status**: Phase 1 is 80% complete (12/15 items)
