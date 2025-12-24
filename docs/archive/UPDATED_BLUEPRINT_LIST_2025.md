# Updated Blueprint List for Adastrea Game - December 2025

**Last Updated**: December 15, 2025  
**For Version**: 1.0.0-alpha  
**Target Engine**: Unreal Engine 5.6

> **Project Status**: The C++ codebase is 85-90% complete with 22 major systems implemented. This document lists all the Blueprints and Data Assets you need to create in the Unreal Editor to make the game playable.

---

## Table of Contents

1. [Quick Start - Minimum Playable (15 items)](#quick-start---minimum-playable-15-items)
2. [Core Game Framework (10 items)](#core-game-framework-10-items)
3. [Player Ship System (8 items)](#player-ship-system-8-items)
4. [Space Station System (25 items)](#space-station-system-25-items)
5. [Combat System (20 items)](#combat-system-20-items)
6. [Navigation & Exploration (15 items)](#navigation--exploration-15-items)
7. [Trading & Economy (18 items)](#trading--economy-18-items)
8. [Faction & Diplomacy (12 items)](#faction--diplomacy-12-items)
9. [Personnel & Crew (15 items)](#personnel--crew-15-items)
10. [Way System (Guilds) (10 items)](#way-system-guilds-10-items)
11. [Quest & Tutorial System (12 items)](#quest--tutorial-system-12-items)
12. [UI Widgets (30 items)](#ui-widgets-30-items)
13. [Audio System (10 items)](#audio-system-10-items)
14. [Input System (8 items)](#input-system-8-items)
15. [Maps & Environments (10 items)](#maps--environments-10-items)
16. [Total Summary & Priority Matrix](#total-summary--priority-matrix)

---

## Quick Start - Minimum Playable (15 items)

These are the absolute minimum blueprints needed to fly a ship and test basic gameplay.

### Game Framework
1. **BP_SpaceGameMode** - Main game mode
   - Parent: `AdastreaGameMode` (C++)
   - Location: `Content/Blueprints/GameModes/`
   - Set Default Pawn: BP_PlayerShip
   - Set Player Controller: BP_SpaceshipController

2. **BP_GameInstance** - Persistent game state
   - Parent: `AdastreaGameInstance` (C++)
   - Location: `Content/Blueprints/Core/`

3. **BP_SpaceshipController** - Player controller
   - Parent: `AdastreaPlayerController` (C++)
   - Location: `Content/Blueprints/Controllers/`
   - Initialize Enhanced Input in BeginPlay

### Player Ship
4. **BP_PlayerShip** - Player's spaceship
   - Parent: `ASpaceship` (C++)
   - Location: `Content/Blueprints/Ships/`
   - Add: Static Mesh, Spring Arm, Camera, FloatingPawnMovement

5. **DA_Ship_PlayerScout** - Ship stats
   - Parent: `USpaceshipDataAsset` (C++)
   - Location: `Content/DataAssets/Ships/`
   - Template: `Assets/SpaceshipTemplates/PathfinderScout.yaml`

### Input System
6. **DA_InputConfig** - Input configuration
   - Parent: `UInputConfigDataAsset` (C++)
   - Location: `Content/DataAssets/Input/`
   - Assign all Input Actions and Mapping Contexts

7. **IMC_Spaceship** - Input Mapping Context (Already exists)
   - Location: `Content/Input/`

8. **IA_Move** - Movement input (Already exists)
9. **IA_Look** - Camera input (Already exists)
10. **IA_Boost** - Boost input (Already exists)
11. **IA_Fire_Primary** - Primary weapon (Already exists)

### UI & Environment
12. **WBP_SpaceshipHUD** - Basic HUD
    - Parent: `UAdastreaHUDWidget` (C++)
    - Location: `Content/UI/HUD/`

13. **TestLevel.umap** - Test map
    - Location: `Content/Maps/`
    - Add: Player Start, Directional Light, Sky Atmosphere

14. **WBP_MainMenu** - Main menu
    - Parent: `UUserWidget`
    - Location: `Content/UI/Menus/`

15. **DA_Faction_Player** - Player's starting faction
    - Parent: `UFactionDataAsset` (C++)
    - Location: `Content/DataAssets/Factions/`

**Time to Create**: 30-60 minutes  
**Result**: Flyable spaceship in 3D space

---

## Core Game Framework (10 items)

### Game Modes & Controllers
1. **BP_SpaceGameMode** (see Quick Start)
2. **BP_TestGameMode** - Testing game mode
   - Parent: `AdastreaGameMode`
   - Location: `Content/Blueprints/GameModes/`
   - For automated testing

3. **BP_SpaceshipController** (see Quick Start)
4. **BP_MenuController** - Menu controller
   - Parent: `APlayerController`
   - Location: `Content/Blueprints/Controllers/`

### Game State & Instance
5. **BP_GameInstance** (see Quick Start)
6. **BP_GameState** - Game state (optional)
   - Parent: `AAdastreaGameState` (C++)
   - Location: `Content/Blueprints/Core/`

### Save System
7. **BP_SaveGame** - Save game object
   - Parent: `USaveGame`
   - Location: `Content/Blueprints/Core/`

### Player Components
8. **BP_PlayerProgression** - Player progression (optional)
   - Parent: `UPlayerProgressionComponent` (C++)
   - Can be added directly to PlayerController

9. **BP_Verse** - Way system reputation (optional)
   - Parent: `UVerse` (C++)
   - Player component for guild reputation

10. **BP_PlayerUnlock** - Unlock system (optional)
    - Parent: `UPlayerUnlockComponent` (C++)
    - Track unlocked ships, modules, etc.

---

## Player Ship System (8 items)

### Core Ship Blueprints
1. **BP_PlayerShip** (see Quick Start)
2. **BP_PlayerShip_Fighter** - Combat variant
   - Parent: `ASpaceship`
   - Location: `Content/Blueprints/Ships/Player/`

3. **BP_PlayerShip_Trader** - Trading variant
   - Parent: `ASpaceship`
   - Location: `Content/Blueprints/Ships/Player/`

### Ship Data Assets (Use YAML Templates)
4. **DA_Ship_PlayerScout** (see Quick Start)
   - Template: `Assets/SpaceshipTemplates/PathfinderScout.yaml`

5. **DA_Ship_PlayerFighter** - Combat ship
   - Parent: `USpaceshipDataAsset`
   - Location: `Content/DataAssets/Ships/`
   - Template: `Assets/SpaceshipTemplates/WarhammerGunship.yaml`

6. **DA_Ship_PlayerTrader** - Cargo hauler
   - Parent: `USpaceshipDataAsset`
   - Location: `Content/DataAssets/Ships/`
   - Template: `Assets/SpaceshipTemplates/MerchantTrader.yaml`

### Ship Components
7. **BP_ShipInterior** - Ship interior space (optional)
   - Parent: `USpaceshipInterior` (C++)
   - Location: `Content/Blueprints/Ships/Interiors/`

8. **BP_ShipControlConsole** - Control console (optional)
   - Parent: `AShipControlConsole` (C++)
   - Location: `Content/Blueprints/Ships/`

**Documentation**: `Assets/SpaceshipDataAssetGuide.md`

---

## Space Station System (25 items)

### Core Station Blueprints
1. **BP_SpaceStation** - Main station actor
   - Parent: `ASpaceStation` (C++)
   - Location: `Content/Blueprints/Stations/`

2. **BP_SpaceStation_Trading** - Trading hub variant
3. **BP_SpaceStation_Military** - Military base variant
4. **BP_SpaceStation_Research** - Research station variant

### Station Modules (20+ modules)

All inherit from `ASpaceStationModule` (C++)  
Location: `Content/Blueprints/Stations/Modules/`

#### Docking Group
5. **BP_Module_DockingBay** - Parent: `ADockingBayModule`
6. **BP_Module_DockingPort** - Parent: `ADockingPortModule`

#### Power Group
7. **BP_Module_Reactor** - Parent: `AReactorModule`
8. **BP_Module_SolarArray** - Parent: `ASolarArrayModule`

#### Storage Group
9. **BP_Module_CargoBay** - Parent: `ACargoBayModule`
10. **BP_Module_FuelDepot** - Parent: `AFuelDepotModule`

#### Defence Group
11. **BP_Module_Turret** - Parent: `ATurretModule`
12. **BP_Module_ShieldGenerator** - Parent: `AShieldGeneratorModule`

#### Habitation Group
13. **BP_Module_Habitation** - Parent: `AHabitationModule`
14. **BP_Module_Barracks** - Parent: `ABarracksModule`

#### Command Group
15. **BP_Module_CommandCenter** - Parent: `ASpaceStationModule` (generic)

#### Industrial Group
16. **BP_Module_Fabrication** - Parent: `AFabricationModule`
17. **BP_Module_Processing** - Parent: `AProcessingModule`

#### Trade Group
18. **BP_Module_Marketplace** - Parent: `AMarketplaceModule`

#### Research Group
19. **BP_Module_ScienceLab** - Parent: `AScienceLabModule`

#### Infrastructure Group
20. **BP_Module_Corridor** - Parent: `ACorridorModule`
21. **BP_Module_LifeSupport** - Parent: `ASpaceStationModule` (generic)

#### Entertainment Group
22. **BP_Module_Recreation** - Parent: `ASpaceStationModule` (generic)

### Station Editor
23. **WBP_StationEditor** - Station editor UI
    - Parent: `UStationEditorWidget` (C++)
    - Location: `Content/UI/Stations/`

24. **DA_StationModuleCatalog** - Module catalog
    - Parent: `UStationModuleCatalog` (C++)
    - Location: `Content/DataAssets/Stations/`

25. **BP_StationBuildPreview** - Build preview (optional)
    - Parent: `AStationBuildPreview` (C++)
    - Location: `Content/Blueprints/Stations/`

**Documentation**: `Assets/StationEditorSystemGuide.md`

---

## Combat System (20 items)

### Weapons
1. **DA_Weapon_Laser** - Energy weapon
   - Parent: `UWeaponDataAsset` (C++)
   - Location: `Content/DataAssets/Weapons/`

2. **DA_Weapon_Cannon** - Projectile weapon
3. **DA_Weapon_Missile** - Missile launcher
4. **DA_Weapon_Torpedo** - Torpedo launcher
5. **DA_Weapon_Railgun** - Kinetic weapon
6. **DA_Weapon_BeamWeapon** - Continuous beam

### Projectiles
7. **BP_Projectile_Laser** - Laser bolt
   - Parent: `AActor`
   - Location: `Content/Blueprints/Combat/Projectiles/`

8. **BP_Projectile_Cannon** - Cannon shell
9. **BP_Projectile_Missile** - Guided missile
10. **BP_Projectile_Torpedo** - Heavy torpedo

### Visual Effects
11. **DA_CombatVFX_MuzzleFlash** - Muzzle flash effect
    - Parent: `UCombatVFXDataAsset` (C++)
    - Location: `Content/DataAssets/Combat/VFX/`

12. **DA_CombatVFX_Explosion** - Explosion effect
13. **DA_ImpactEffect_Hull** - Hull impact effect
    - Parent: `UImpactEffectDataAsset` (C++)
14. **DA_ImpactEffect_Shield** - Shield impact effect

### NPC Ships
15. **BP_Ship_NPC_Fighter** - Enemy fighter
    - Parent: `ASpaceship`
    - Location: `Content/Blueprints/Ships/NPCs/`

16. **BP_Ship_NPC_Interceptor** - Fast interceptor
17. **BP_Ship_NPC_Gunship** - Heavy gunship
18. **BP_Ship_NPC_Trader** - AI trader ship

### Ship Data for NPCs
19. **DA_Ship_NPCFighter** - Fighter stats
    - Parent: `USpaceshipDataAsset`
    - Location: `Content/DataAssets/Ships/NPCs/`

20. **DA_Ship_NPCTrader** - Trader stats

**Documentation**: `Assets/CombatSystemGuide.md`

---

## Navigation & Exploration (15 items)

### Exploration Data Assets
1. **DA_Anomaly_Spatial** - Space anomaly
   - Parent: `UAnomalyDataAsset` (C++)
   - Location: `Content/DataAssets/Exploration/`

2. **DA_Anomaly_Temporal** - Temporal anomaly
3. **DA_Anomaly_Energy** - Energy anomaly

4. **DA_Scanner_Basic** - Basic scanner
   - Parent: `UScannerDataAsset` (C++)
   - Location: `Content/DataAssets/Exploration/`

5. **DA_Scanner_Advanced** - Advanced scanner

6. **DA_Discovery_Planet** - Planet discovery
   - Parent: `UDiscoveryDataAsset` (C++)
   - Location: `Content/DataAssets/Exploration/`

7. **DA_Discovery_Artifact** - Artifact discovery
8. **DA_Discovery_Derelict** - Derelict ship discovery

### Environment Actors
9. **BP_Asteroid** - Asteroid obstacle
   - Parent: `AActor`
   - Location: `Content/Blueprints/Environment/`

10. **BP_Nebula** - Nebula visual effect
11. **BP_Planet** - Planet actor
    - Parent: `APlanet` (C++)

12. **BP_StarField** - Star field background
13. **BP_Waypoint** - Navigation waypoint

### Map System
14. **WBP_SectorMap** - Sector map UI
    - Parent: `USectorMapWidget` (C++)
    - Location: `Content/UI/Map/`

15. **WBP_UniverseMap** - Universe map UI
    - Parent: `UUniverseMapWidget` (C++)

**Documentation**: `Assets/MapSystemGuide.md`, `Assets/NavigationSystemGuide.md`

---

## Trading & Economy (18 items)

### Trade Items
1. **DA_TradeItem_Food** - Food commodities
   - Parent: `UTradeItemDataAsset` (C++)
   - Location: `Content/DataAssets/Trading/Items/`
   - Template: `Assets/TradingTemplates/`

2. **DA_TradeItem_Water** - Water
3. **DA_TradeItem_Minerals** - Raw minerals
4. **DA_TradeItem_Metal** - Refined metal
5. **DA_TradeItem_Technology** - Tech goods
6. **DA_TradeItem_Weapons** - Weapons
7. **DA_TradeItem_Medicine** - Medical supplies
8. **DA_TradeItem_Luxury** - Luxury goods
9. **DA_TradeItem_Fuel** - Fuel
10. **DA_TradeItem_Electronics** - Electronics

### Markets
11. **DA_Market_Station1** - Station market
    - Parent: `UMarketDataAsset` (C++)
    - Location: `Content/DataAssets/Trading/Markets/`

12. **DA_Market_Station2** - Another market
13. **DA_Market_Station3** - Trade hub market

### Contracts
14. **DA_Contract_Delivery** - Delivery mission
    - Parent: `UTradeContractDataAsset` (C++)
    - Location: `Content/DataAssets/Trading/Contracts/`

15. **DA_Contract_BulkTransport** - Bulk transport
16. **DA_Contract_UrgentDelivery** - Time-sensitive

### Materials
17. **DA_Material_Iron** - Iron resource
    - Parent: `UMaterialDataAsset` (C++)
    - Location: `Content/DataAssets/Materials/`

18. **DA_Material_Titanium** - Titanium resource

**Documentation**: `Assets/TradingSystemGuide.md`

---

## Faction & Diplomacy (12 items)

### Player/Friendly Factions
1. **DA_Faction_Player** (see Quick Start)
   - Template: `Assets/FactionSetupGuide.md`

2. **DA_Faction_Alliance** - Allied faction
   - Parent: `UFactionDataAsset` (C++)
   - Location: `Content/DataAssets/Factions/`

3. **DA_Faction_Merchants** - Trading guild

### Neutral Factions
4. **DA_Faction_Independent** - Independent stations
5. **DA_Faction_Miners** - Mining consortium
6. **DA_Faction_Scientists** - Research collective

### Hostile Factions
7. **DA_Faction_Pirates** - Pirate faction
8. **DA_Faction_Rebels** - Rebel faction
9. **DA_Faction_Military** - Hostile military

### Homeworlds
10. **DA_Homeworld_Earth** - Earth starting point
    - Parent: `UHomeworldDataAsset` (C++)
    - Location: `Content/DataAssets/Homeworlds/`

11. **DA_Homeworld_Mars** - Mars colony
12. **DA_Homeworld_AlphaStation** - Space station start

**Documentation**: `Assets/BlueprintFactionAssetGuide.md`, `Assets/HomeworldSystemGuide.md`

---

## Personnel & Crew (15 items)

### Command Roles
1. **DA_Personnel_Captain** - Ship captain
   - Parent: `UPersonnelDataAsset` (C++)
   - Location: `Content/DataAssets/Personnel/`
   - Template: `Assets/PersonnelTemplates/Captain.yaml`

2. **DA_Personnel_XO** - Executive officer

### Flight Crew
3. **DA_Personnel_Pilot** - Pilot
   - Template: `Assets/PersonnelTemplates/Pilot.yaml`

4. **DA_Personnel_Navigator** - Navigator

### Engineering
5. **DA_Personnel_Engineer** - Chief engineer
   - Template: `Assets/PersonnelTemplates/Engineer.yaml`

6. **DA_Personnel_Technician** - Technician

### Combat Crew
7. **DA_Personnel_Gunner** - Weapons officer
   - Template: `Assets/PersonnelTemplates/Gunner.yaml`

8. **DA_Personnel_Security** - Security officer

### Medical & Science
9. **DA_Personnel_Medical** - Medical officer
   - Template: `Assets/PersonnelTemplates/MedicalOfficer.yaml`

10. **DA_Personnel_Scientist** - Science officer

### Support Crew
11. **DA_Personnel_Quartermaster** - Supply officer
12. **DA_Personnel_Cook** - Chef
13. **DA_Personnel_Crew** - Generic crew
    - Template: `Assets/PersonnelTemplates/CrewMember_Generic.yaml`

### AI Logic
14. **BP_PersonnelLogic_Standard** - Standard AI
    - Parent: `UPersonnelLogic` (C++)
    - Location: `Content/Blueprints/AI/Personnel/`

15. **BP_FactionLogic_Standard** - Faction AI
    - Parent: `UFactionLogic` (C++)
    - Location: `Content/Blueprints/AI/Factions/`

**Documentation**: `Assets/PersonnelSystemGuide.md`, `Assets/PersonnelAISetupGuide.md`

---

## Way System (Guilds) (10 items)

### Way Data Assets
1. **DA_Way_Engineers** - Engineering guild
   - Parent: `UWayDataAsset` (C++)
   - Location: `Content/DataAssets/Ways/`
   - Template: `Assets/WayNetworkTemplates/`

2. **DA_Way_Traders** - Trading guild
3. **DA_Way_Explorers** - Explorer's guild
4. **DA_Way_Mercenaries** - Mercenary guild
5. **DA_Way_Scientists** - Scientist guild

### Way Networks
6. **DA_Network_CraftsmensCompact** - Engineering alliance
   - Parent: `UWayNetworkDataAsset` (C++)
   - Location: `Content/DataAssets/Ways/Networks/`
   - Template: `Assets/WayNetworkTemplates/CraftsmensCompact.yaml`

7. **DA_Network_MerchantCoalition** - Trade alliance
   - Template: `Assets/WayNetworkTemplates/MerchantCoalition.yaml`

8. **DA_Network_FrontierAlliance** - Explorer alliance
   - Template: `Assets/WayNetworkTemplates/FrontierAlliance.yaml`

9. **DA_Network_HonorCircle** - Warrior alliance
   - Template: `Assets/WayNetworkTemplates/HonorCircle.yaml`

10. **DA_Network_ScholarsSyndicate** - Research alliance
    - Template: `Assets/WayNetworkTemplates/ScholarsSyndicate.yaml`

**Documentation**: `Assets/WaySystemGuide.md`, `Assets/WayNetworkGuide.md`

---

## Quest & Tutorial System (12 items)

### Tutorial Steps
1. **DA_Tutorial_Welcome** - Welcome message
   - Parent: `UTutorialStepDataAsset` (C++)
   - Location: `Content/DataAssets/Tutorials/`

2. **DA_Tutorial_Movement** - Movement tutorial
3. **DA_Tutorial_Combat** - Combat tutorial
4. **DA_Tutorial_Trading** - Trading tutorial
5. **DA_Tutorial_Navigation** - Navigation tutorial

### Quests
6. **DA_Quest_Tutorial** - Tutorial quest
   - Parent: `UQuestDataAsset` (C++)
   - Location: `Content/DataAssets/Quests/`

7. **DA_Quest_FirstDelivery** - First delivery mission
8. **DA_Quest_CombatIntro** - First combat mission
9. **DA_Quest_ExplorationIntro** - First exploration
10. **DA_Quest_TradeRoute** - Establish trade route

### Achievements
11. **DA_Achievement_FirstFlight** - First flight achievement
    - Parent: `UAchievementDataAsset` (C++)
    - Location: `Content/DataAssets/Achievements/`

12. **DA_Achievement_FirstKill** - First combat victory

**Documentation**: `Assets/QuestSystemGuide.md`, `Assets/TutorialSystemGuide.md`

---

## UI Widgets (30 items)

### HUD System
1. **WBP_SpaceshipHUD** (see Quick Start)
2. **WBP_CombatHUD** - Combat-focused HUD
   - Parent: `UAdastreaHUDWidget`
   - Location: `Content/UI/HUD/`

3. **WBP_NavigationHUD** - Navigation overlay

### Menus
4. **WBP_MainMenu** (see Quick Start)
5. **WBP_PauseMenu** - Pause menu
   - Parent: `UUserWidget`
   - Location: `Content/UI/Menus/`

6. **WBP_SettingsMenu** - Settings menu
7. **WBP_LoadingScreen** - Loading screen
8. **WBP_CharacterCreation** - Character creator

### Ship Status
9. **WBP_ShipStatus** - Ship status screen
   - Parent: `UShipStatusWidget` (C++)
   - Location: `Content/UI/Ship/`

10. **WBP_ShipCustomization** - Ship customization
    - Parent: `UShipCustomizationWidget` (C++)

11. **WBP_CrewManagement** - Crew management UI

### Inventory & Trading
12. **WBP_Inventory** - Inventory screen
    - Parent: `UInventoryWidget` (C++)
    - Location: `Content/UI/Inventory/`

13. **WBP_TradingInterface** - Trading UI
    - Parent: `UTradingInterfaceWidget` (C++)
    - Location: `Content/UI/Trading/`

14. **WBP_CargoManagement** - Cargo screen

### Station Management
15. **WBP_StationManagement** - Station overview
    - Parent: `UStationManagementWidget` (C++)
    - Location: `Content/UI/Stations/`

16. **WBP_StationEditor** (covered in Station System)
17. **WBP_DockingInterface** - Docking UI

### Map & Navigation
18. **WBP_SectorMap** (covered in Navigation)
19. **WBP_UniverseMap** (covered in Navigation)
20. **WBP_Minimap** - Tactical minimap

### Quest & Progression
21. **WBP_QuestLog** - Quest tracker
    - Parent: `UUserWidget`
    - Location: `Content/UI/Quests/`

22. **WBP_ObjectiveTracker** - Objective overlay
23. **WBP_ProgressionScreen** - Player progression
24. **WBP_AchievementNotification** - Achievement popup

### Communication
25. **WBP_Comms** - Communication interface
26. **WBP_DialogueBox** - Dialogue system
27. **WBP_Notification** - Notification popup

### Combat UI
28. **WBP_TargetingInfo** - Target info panel
29. **WBP_WeaponStatus** - Weapon status display
30. **WBP_DamageIndicator** - Damage direction indicator

**Documentation**: `Assets/ShipStatusScreenSetup.md`

---

## Audio System (10 items)

### Music Tracks
1. **DA_Music_MainTheme** - Main theme
   - Parent: `UMusicTrackDataAsset` (C++)
   - Location: `Content/DataAssets/Audio/Music/`

2. **DA_Music_Combat** - Combat music
3. **DA_Music_Exploration** - Exploration music
4. **DA_Music_Trading** - Trading hub music
5. **DA_Music_Menu** - Menu music

### Sound Effects
6. **DA_SFX_WeaponFire** - Weapon firing sound
   - Parent: `USoundEffectDataAsset` (C++)
   - Location: `Content/DataAssets/Audio/SFX/`

7. **DA_SFX_Explosion** - Explosion sound
8. **DA_SFX_EngineLoop** - Engine sound
9. **DA_SFX_ShieldHit** - Shield impact
10. **DA_SFX_UIClick** - UI interaction sound

**Documentation**: `Assets/AudioSystemGuide.md`

---

## Input System (8 items)

### Already Exist (Verify Configuration)
1. **DA_InputConfig** (see Quick Start)
2. **IMC_Spaceship** (Already exists)
3. **IA_Move** (Already exists)
4. **IA_Look** (Already exists)
5. **IA_Boost** (Already exists)
6. **IA_Fire_Primary** (Already exists)
7. **IA_PauseMenu** (Already exists)

### Additional Input Actions (Create if needed)
8. **IA_Fire_Secondary** - Secondary weapon
   - Type: Input Action
   - Location: `Content/Input/Actions/`

9. **IA_TargetNext** - Next target
10. **IA_TargetPrevious** - Previous target
11. **IA_Autopilot** - Toggle autopilot
12. **IA_OpenInventory** - Open inventory
13. **IA_OpenMap** - Open map
14. **IA_OpenComms** - Open comms

**Note**: Most Input Actions already exist. See `InputConfigDataAsset.h` for full list of 30+ actions.

**Documentation**: `Assets/EnhancedInputImplementation.md`

---

## Maps & Environments (10 items)

### Core Maps
1. **TestLevel.umap** (see Quick Start)
2. **MainMenu.umap** - Main menu scene
   - Location: `Content/Maps/`

3. **TestMode.umap** - Comprehensive test map
   - Location: `Content/Maps/`
   - Specification: `Content/Maps/TestMode.umap.txt`

### Gameplay Maps
4. **SectorAlpha.umap** - Starting sector
5. **SectorBeta.umap** - Second sector
6. **StationInterior_Trading.umap** - Trading station interior
7. **StationInterior_Military.umap** - Military station interior

### Special Maps
8. **TutorialMap.umap** - Tutorial level
9. **CombatArena.umap** - Combat test arena
10. **EmptySpace.umap** - Blank test environment

**Documentation**: `Docs/EmptyMapSetup.md`

---

## Total Summary & Priority Matrix

### By Priority

#### ⚠️ CRITICAL - Must Create First (15 items)
- Game Framework: BP_SpaceGameMode, BP_GameInstance, BP_SpaceshipController
- Player Ship: BP_PlayerShip, DA_Ship_PlayerScout
- Input: DA_InputConfig (and verify existing Input Actions)
- UI: WBP_SpaceshipHUD, WBP_MainMenu
- Environment: TestLevel.umap
- Faction: DA_Faction_Player

#### 🔶 HIGH - Create Soon (40 items)
- Station system: BP_SpaceStation + 10 core modules
- Combat: 5 weapons, 3 projectiles, 3 NPC ships
- Trading: 5 trade items, 2 markets
- Factions: 3 additional factions
- Personnel: 5 core crew roles
- UI: Inventory, Trading interface, Station management

#### 🔷 MEDIUM - Expand Gameplay (60 items)
- All station modules (25 total)
- Complete trading items (10+)
- Complete personnel roles (15+)
- Way system (10 items)
- Quest system (5 quests)
- Additional ship variants
- More UI widgets

#### 🟢 LOW - Polish & Enhancement (60+ items)
- Tutorial steps
- Achievements
- Advanced UI widgets
- Audio assets
- Visual effects
- Additional maps
- Procedural content

### Total Count by System

| System | Data Assets | Blueprints | UI Widgets | Total |
|--------|-------------|------------|------------|-------|
| Core Framework | 1 | 7 | 2 | 10 |
| Ships | 6 | 8 | 2 | 16 |
| Stations | 1 | 24 | 2 | 27 |
| Combat | 14 | 10 | 3 | 27 |
| Navigation | 8 | 5 | 2 | 15 |
| Trading | 16 | 0 | 2 | 18 |
| Factions | 12 | 2 | 0 | 14 |
| Personnel | 13 | 2 | 1 | 16 |
| Ways | 10 | 0 | 0 | 10 |
| Quests | 12 | 0 | 2 | 14 |
| UI | 0 | 0 | 30 | 30 |
| Audio | 10 | 0 | 0 | 10 |
| Input | 1 | 0 | 0 | 1 (+ existing) |
| Maps | 0 | 0 | 0 | 10 |
| **TOTAL** | **104** | **58** | **46** | **208** |

---

## Project Configuration

After creating blueprints, configure these in **Edit → Project Settings**:

### Maps & Modes
```
Default GameMode: BP_SpaceGameMode
Editor Startup Map: TestLevel
Game Default Map: MainMenu
```

### Input
```
Default Player Input Class: EnhancedPlayerInput
Default Input Component Class: EnhancedInputComponent
Enhanced Input plugin: ✓ Enabled
```

### Engine
```
Game Instance Class: BP_GameInstance
```

---

## Creation Workflow

### Recommended Order

#### Week 1: Minimum Playable
1. Create Quick Start items (15 items)
2. Configure Project Settings
3. Test flying in TestLevel
4. **Result**: Flyable spaceship

#### Week 2: Combat & NPCs
1. Create combat weapons (5 items)
2. Create projectiles (4 items)
3. Create NPC ships (4 items)
4. **Result**: Basic combat gameplay

#### Week 3: Stations & Trading
1. Create BP_SpaceStation
2. Create 5 core modules
3. Create 5 trade items
4. Create trading UI
5. **Result**: Docking and trading

#### Week 4: Content Expansion
1. Create more factions (5+)
2. Create personnel (10+)
3. Create quests (5+)
4. Create UI widgets (10+)
5. **Result**: Full gameplay loop

---

## Using YAML Templates

Most Data Assets have YAML templates in `Assets/` directory:

- **Ships**: `Assets/SpaceshipTemplates/*.yaml`
- **Factions**: `Assets/FactionSetupGuide.md`
- **Personnel**: `Assets/PersonnelTemplates/*.yaml`
- **Trading**: `Assets/TradingTemplates/*.yaml`
- **Ways**: `Assets/WayNetworkTemplates/*.yaml`

**Import Tool**: Use `YAMLtoDataAsset.py` to convert YAML to Data Assets

---

## Automation Tools

Speed up content creation with these Python tools:

- **ProceduralGenerators.py** - Generate ships, NPCs, quests, systems
- **YAMLtoDataAsset.py** - Import YAML as Data Assets
- **TemplateGenerator.py** - Generate YAML templates
- **DataAssetBatchCreator.py** - Batch create Data Assets

See `PROCEDURAL_GENERATION_QUICKSTART.md` for details.

---

## Testing Checklist

After creating blueprints:

### Phase 1: Basic Movement
- [ ] Ship spawns
- [ ] WASD movement works
- [ ] Mouse look works
- [ ] Boost works
- [ ] HUD displays

### Phase 2: Combat
- [ ] Can fire weapons
- [ ] Projectiles spawn
- [ ] Damage applies
- [ ] NPC ships spawn

### Phase 3: Economy
- [ ] Can dock at stations
- [ ] Trading interface opens
- [ ] Can buy/sell items
- [ ] Prices update

### Phase 4: Systems Integration
- [ ] Quests track progress
- [ ] Faction relationships work
- [ ] Crew management works
- [ ] Save/load works

---

## Common Issues & Solutions

### Blueprint Won't Compile
- Check parent class is correct
- Verify all required components exist
- Look for missing variable references

### Input Not Working
- Verify DA_InputConfig has all actions assigned
- Check Enhanced Input is enabled in Project Settings
- Verify IMC_Spaceship is added in PlayerController BeginPlay

### Ship Doesn't Spawn
- Check Game Mode Default Pawn Class
- Verify Player Start is in level
- Check Output Log for errors

### Data Asset References Broken
- Re-assign Data Asset references in Blueprint
- Check file paths match documentation
- Verify Data Asset parent class

---

## Documentation References

### Quick Start Guides
- `BLUEPRINT_QUICK_START_PIE.md` - 5-minute setup
- `BLUEPRINT_LIST_FOR_PIE.md` - PIE-focused list
- `CONTENT_CREATION_QUICKSTART.md` - 30-minute playable

### Complete Guides
- `BLUEPRINT_REQUIREMENTS_LIST.md` - Original comprehensive list
- `BLUEPRINT_GUIDE_FOR_BEGINNERS.md` - Beginner-friendly guide
- `BLUEPRINT_CREATION_GUIDE.md` - Step-by-step creation

### System Documentation
- `Assets/SpaceshipDataAssetGuide.md` - Complete ship system
- `Assets/StationEditorSystemGuide.md` - Station system
- `Assets/TradingSystemGuide.md` - Trading system
- `Assets/PersonnelSystemGuide.md` - Personnel system
- `Assets/WaySystemGuide.md` - Way/guild system
- And 20+ more in `Assets/` directory

---

## What's New in This List

### December 2025 Updates

1. **Reorganized by System** - Blueprints grouped by game system
2. **Priority Matrix** - Clear priority levels for each item
3. **C++ Class References** - Shows parent C++ class for each blueprint
4. **Module Count** - Comprehensive station module list (25 items)
5. **Way System** - Added guild/network system (10 items)
6. **Exploration System** - Added exploration data assets (8 items)
7. **Audio System** - Added audio data assets (10 items)
8. **Total Count** - 208 total items to create
9. **Creation Workflow** - Week-by-week creation plan
10. **YAML Templates** - References to all available templates

### Differences from Original List

- **More Comprehensive**: 208 items vs 100+ estimated
- **Better Organization**: Grouped by system, not asset type
- **C++ Integration**: Shows C++ parent classes
- **Practical Workflow**: Week-by-week creation plan
- **Updated Status**: Reflects December 2025 codebase
- **Station Modules**: Complete list of all 25+ modules
- **Way System**: New guild system added
- **Exploration**: New exploration system added

---

## Questions or Help?

- **For Designers**: See `Assets/DesignerOnboarding.md`
- **For Programmers**: See `CONTRIBUTING.md` and `ARCHITECTURE.md`
- **For Quick Start**: See `BLUEPRINT_QUICK_START_PIE.md`
- **For All Docs**: See `DOCUMENTATION_INDEX.md`

---

**Good luck creating content! 🚀**

The C++ systems are ready - now it's time to bring them to life in the editor!
