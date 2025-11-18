# Blueprint Requirements List - Adastrea Editor

> **Comprehensive list of user-created Blueprints needed in the Unreal Engine Editor to make Adastrea functional and playable**

**Last Updated**: November 16, 2025  
**For Version**: 1.0.0-alpha  
**Target Engine**: Unreal Engine 5.6

**⭐ New to Blueprints?** Start with [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - it explains everything from scratch!

---

## Table of Contents

1. [Essential Blueprints (Required for Basic Gameplay)](#essential-blueprints-required-for-basic-gameplay)
2. [Core System Blueprints](#core-system-blueprints)
3. [Data Asset Blueprints](#data-asset-blueprints)
4. [UI Widget Blueprints](#ui-widget-blueprints)
5. [Actor Blueprints](#actor-blueprints)
6. [Component Blueprints](#component-blueprints)
7. [Input Blueprints](#input-blueprints)
8. [AI and Logic Blueprints](#ai-and-logic-blueprints)
9. [Quest and Progression Blueprints](#quest-and-progression-blueprints)
10. [Optional Enhancement Blueprints](#optional-enhancement-blueprints)
11. [Quick Reference Table](#quick-reference-table)
12. [Testing Checklist](#testing-checklist)

---

## Essential Blueprints (Required for Basic Gameplay)

These Blueprints are **absolutely required** to get a playable version of Adastrea running in the editor.

### 1. Game Framework Blueprints

#### BP_SpaceGameMode
- **Location**: `Content/Blueprints/GameModes/BP_SpaceGameMode.uasset`
- **Parent Class**: `AdastreaGameMode` (C++)
- **Purpose**: Main game mode that controls game rules, spawning, and player setup
- **Required Settings**:
  - Default Pawn Class: `BP_PlayerShip`
  - Player Controller Class: `BP_SpaceshipController`
  - Game State Class: `AAdastreaGameState` (C++)
  - Player State Class: `PlayerState` (default)
- **Priority**: ⚠️ **CRITICAL** - Game won't start without this

#### BP_GameInstance
- **Location**: `Content/Blueprints/Core/BP_GameInstance.uasset`
- **Parent Class**: `AdastreaGameInstance` (C++)
- **Purpose**: Persistent game instance that manages save/load, global state, and faction diplomacy
- **Required Variables**:
  - PlayerName: String
  - CurrentCredits: Integer
  - SaveSlotName: String
- **Priority**: ⚠️ **CRITICAL** - Needed for game persistence

#### BP_SpaceshipController
- **Location**: `Content/Blueprints/Controllers/BP_SpaceshipController.uasset`
- **Parent Class**: `AdastreaPlayerController` (C++)
- **Purpose**: Player controller that handles input, camera, and HUD
- **Required Setup**:
  - Enhanced Input initialization in BeginPlay
  - HUD widget creation and viewport addition
- **Priority**: ⚠️ **CRITICAL** - Player control won't work without this

### 2. Player Ship Blueprint

#### BP_PlayerShip
- **Location**: `Content/Blueprints/Ships/BP_PlayerShip.uasset`
- **Parent Class**: `ASpaceship` (C++)
- **Purpose**: The player's controllable spaceship
- **Required Components**:
  - Static Mesh Component (ship visual)
  - Spring Arm Component (camera boom)
  - Camera Component (player view)
  - Arrow Component (projectile spawn point)
  - Combat Components (WeaponComponent, TargetingComponent, CombatHealthComponent)
  - NavigationComponent (autopilot)
- **Required Variables**:
  - ShipDataAsset: Reference to `USpaceshipDataAsset`
  - ForwardSpeed, StrafeSpeed, BoostMultiplier
  - YawRate, PitchRate, RollRate
- **Priority**: ⚠️ **CRITICAL** - This is the player's ship

### 3. Basic Environment

#### BP_PlayerStart (Can use Engine Default)
- **Location**: Place in each map
- **Parent Class**: `PlayerStart`
- **Purpose**: Spawn point for player
- **Priority**: ⚠️ **CRITICAL** - Player needs somewhere to spawn

---

## Core System Blueprints

### Space Station Blueprints

#### BP_SpaceStation_Main
- **Location**: `Content/Blueprints/Stations/BP_SpaceStation_Main.uasset`
- **Parent Class**: `ASpaceStation` (C++)
- **Purpose**: Main space station actor with module attachment system
- **Required Setup**:
  - Faction Data Asset reference
  - Initial module list
- **Priority**: 🔶 **HIGH** - Core station functionality

#### BP_SpaceStationModule_[Type]
Create one Blueprint for each module type:
- **BP_Module_Docking** - Docking ports for ships
- **BP_Module_Power** - Power generation
- **BP_Module_Storage** - Cargo storage
- **BP_Module_Defence** - Weapons/shields
- **BP_Module_Habitation** - Living quarters
- **BP_Module_Command** - Command center
- **BP_Module_Industrial** - Manufacturing
- **BP_Module_Trade** - Trading hub
- **BP_Module_Research** - Research facilities
- **BP_Module_Entertainment** - Recreational areas

- **Location**: `Content/Blueprints/Stations/Modules/`
- **Parent Class**: `ASpaceStationModule` (C++)
- **Purpose**: Individual station modules that can be attached
- **Priority**: 🔶 **HIGH** - Needed for station construction

### Ship Blueprints

#### BP_Ship_NPC_[Type]
Create NPC ship variants:
- **BP_Ship_NPC_Fighter** - Combat ship
- **BP_Ship_NPC_Trader** - Trading vessel
- **BP_Ship_NPC_Hauler** - Cargo transport
- **BP_Ship_NPC_Scout** - Exploration ship

- **Location**: `Content/Blueprints/Ships/NPCs/`
- **Parent Class**: `ASpaceship` (C++)
- **Purpose**: AI-controlled ships for encounters
- **Required Components**:
  - AITraderComponent (for traders)
  - Combat components
  - NavigationComponent
- **Priority**: 🔷 **MEDIUM** - Needed for dynamic world

---

## Data Asset Blueprints

### Spaceship Data Assets

#### DA_Ship_[Name]
Create at least these ship templates:
- **DA_Ship_PlayerScout** - Player starting ship
- **DA_Ship_PlayerFighter** - Combat upgrade
- **DA_Ship_PlayerTrader** - Trading vessel
- **DA_Ship_NPC_Fighter** - Enemy fighter
- **DA_Ship_NPC_Trader** - AI trader

- **Location**: `Content/DataAssets/Ships/`
- **Parent Class**: `USpaceshipDataAsset` (C++)
- **Purpose**: Ship statistics and configuration
- **Priority**: ⚠️ **CRITICAL** - Ships need stats
- **Templates Available**: `Assets/SpaceshipTemplates/*.yaml`

### Faction Data Assets

#### DA_Faction_[Name]
Create at least 3-5 factions:
- **DA_Faction_PlayerAlliance** - Player's starting faction
- **DA_Faction_Merchants** - Trading guild
- **DA_Faction_Military** - Military force
- **DA_Faction_Pirates** - Hostile faction
- **DA_Faction_Independent** - Neutral entities

- **Location**: `Content/DataAssets/Factions/`
- **Parent Class**: `UFactionDataAsset` (C++)
- **Purpose**: Faction configuration, traits, relationships
- **Priority**: 🔶 **HIGH** - Needed for diplomacy system
- **Templates Available**: `Assets/FactionSetupGuide.md`

### Personnel Data Assets

#### DA_Personnel_[Role]
Create key personnel roles:
- **DA_Personnel_Captain** - Ship captain
- **DA_Personnel_Pilot** - Pilot
- **DA_Personnel_Engineer** - Engineering officer
- **DA_Personnel_Medical** - Medical officer
- **DA_Personnel_Security** - Security chief
- **DA_Personnel_Trader** - Trading specialist

- **Location**: `Content/DataAssets/Personnel/`
- **Parent Class**: `UPersonnelDataAsset` (C++)
- **Purpose**: Crew member specifications
- **Priority**: 🔷 **MEDIUM** - Crew management system
- **Templates Available**: `Assets/PersonnelTemplates/*.yaml`

### Trading Data Assets

#### DA_TradeItem_[Type]
Create trade items:
- **DA_TradeItem_Food** - Food commodities
- **DA_TradeItem_Minerals** - Raw minerals
- **DA_TradeItem_Technology** - Tech goods
- **DA_TradeItem_Weapons** - Weapons
- **DA_TradeItem_Luxury** - Luxury goods

- **Location**: `Content/DataAssets/Trading/Items/`
- **Parent Class**: `UTradeItemDataAsset` (C++)
- **Purpose**: Tradeable commodities
- **Priority**: 🔷 **MEDIUM** - Trading economy

#### DA_Market_[Location]
- **Location**: `Content/DataAssets/Trading/Markets/`
- **Parent Class**: `UMarketDataAsset` (C++)
- **Purpose**: Market configurations for stations
- **Priority**: 🔷 **MEDIUM** - Trading system

### Material Data Assets

#### DA_Material_[Type]
- **Location**: `Content/DataAssets/Materials/`
- **Parent Class**: `UMaterialDataAsset` (C++)
- **Purpose**: Raw materials and refined goods
- **Priority**: 🔷 **MEDIUM** - Crafting and station building

### Weapon Data Assets

#### DA_Weapon_[Type]
Create weapon types:
- **DA_Weapon_Laser** - Energy weapon
- **DA_Weapon_Cannon** - Projectile weapon
- **DA_Weapon_Missile** - Missile launcher
- **DA_Weapon_Torpedo** - Torpedo launcher

- **Location**: `Content/DataAssets/Weapons/`
- **Parent Class**: `UWeaponDataAsset` (C++)
- **Purpose**: Weapon specifications
- **Priority**: 🔶 **HIGH** - Combat system

### Quest Data Assets

#### DA_Quest_[Name]
Create example quests:
- **DA_Quest_Tutorial_FirstFlight** - Tutorial quest
- **DA_Quest_Delivery_CargoPod** - Delivery quest
- **DA_Quest_Combat_Pirates** - Combat quest
- **DA_Quest_Exploration_Sector** - Exploration quest

- **Location**: `Content/DataAssets/Quests/`
- **Parent Class**: `UQuestDataAsset` (C++)
- **Purpose**: Quest definitions
- **Priority**: 🔷 **MEDIUM** - Quest system

### Homeworld Data Assets

#### DA_Homeworld_[Name]
Create starting locations:
- **DA_Homeworld_EarthStation** - Human starting point
- **DA_Homeworld_ColonyAlpha** - Colony world
- **DA_Homeworld_TradingHub** - Trade station start

- **Location**: `Content/DataAssets/Homeworlds/`
- **Parent Class**: `UHomeworldDataAsset` (C++)
- **Purpose**: Player starting locations
- **Priority**: 🔶 **HIGH** - Character creation

### Way System Data Assets

#### DA_Way_[Name]
Create Ways (guilds):
- **DA_Way_Traders** - Trading guild
- **DA_Way_Explorers** - Explorer's guild
- **DA_Way_Mercenaries** - Mercenary guild

- **Location**: `Content/DataAssets/Ways/`
- **Parent Class**: Implementation depends on Way system
- **Purpose**: Guild configurations
- **Priority**: 🔷 **MEDIUM** - Way system

---

## UI Widget Blueprints

### HUD Widgets

#### WBP_SpaceshipHUD
- **Location**: `Content/UI/HUD/WBP_SpaceshipHUD.uasset`
- **Parent Class**: `UAdastreaHUDWidget` (C++)
- **Purpose**: Main heads-up display showing ship status, speed, health, shields, target info
- **Required Elements**:
  - Ship name text
  - Speed indicator
  - Health bar
  - Shield bar
  - Crosshair
  - Target info panel
  - Objective display
- **Priority**: ⚠️ **CRITICAL** - Player needs UI feedback

### Menu Widgets

#### WBP_MainMenu
- **Location**: `Content/UI/Menus/WBP_MainMenu.uasset`
- **Parent Class**: `UUserWidget`
- **Purpose**: Main menu for game start
- **Required Elements**:
  - Start Game button
  - Load Game button
  - Settings button
  - Quit button
- **Priority**: ⚠️ **CRITICAL** - Entry point to game

#### WBP_PauseMenu
- **Location**: `Content/UI/Menus/WBP_PauseMenu.uasset`
- **Parent Class**: `UUserWidget`
- **Purpose**: In-game pause menu
- **Required Elements**:
  - Resume button
  - Settings button
  - Save Game button
  - Main Menu button
- **Priority**: 🔶 **HIGH** - Player needs to pause

#### WBP_SettingsMenu
- **Location**: `Content/UI/Menus/WBP_SettingsMenu.uasset`
- **Parent Class**: `UUserWidget`
- **Purpose**: Game settings configuration
- **Priority**: 🔷 **MEDIUM** - Quality of life

### Trading UI

#### WBP_TradingInterface
- **Location**: `Content/UI/Trading/WBP_TradingInterface.uasset`
- **Parent Class**: `UTradingInterfaceWidget` (C++)
- **Purpose**: Trading interface for buying/selling goods
- **Priority**: 🔶 **HIGH** - Trading system

### Station Management UI

#### WBP_StationManagement
- **Location**: `Content/UI/Stations/WBP_StationManagement.uasset`
- **Parent Class**: `UStationManagementWidget` (C++)
- **Purpose**: Station overview and management
- **Priority**: 🔶 **HIGH** - Station interaction

#### WBP_StationEditor
- **Location**: `Content/UI/Stations/WBP_StationEditor.uasset`
- **Parent Class**: From `StationEditor` module
- **Purpose**: In-game station editor for module placement
- **Priority**: 🔷 **MEDIUM** - Station customization
- **Reference**: `Content/Blueprints/STATION_EDITOR_README.md`

### Inventory UI

#### WBP_InventoryWidget
- **Location**: `Content/UI/Inventory/WBP_InventoryWidget.uasset`
- **Parent Class**: `UInventoryWidget` (C++)
- **Purpose**: Player inventory display and management
- **Priority**: 🔶 **HIGH** - Item management

### Ship Customization UI

#### WBP_ShipCustomization
- **Location**: `Content/UI/Ships/WBP_ShipCustomization.uasset`
- **Parent Class**: `UShipCustomizationWidget` (C++)
- **Purpose**: Ship loadout and visual customization
- **Priority**: 🔷 **MEDIUM** - Ship personalization

### Quest UI

#### WBP_QuestLog
- **Location**: `Content/UI/Quests/WBP_QuestLog.uasset`
- **Parent Class**: `UUserWidget`
- **Purpose**: Quest tracking and objectives display
- **Priority**: 🔷 **MEDIUM** - Quest system

### Map UI

#### WBP_SectorMap
- **Location**: `Content/UI/Map/WBP_SectorMap.uasset`
- **Parent Class**: `USectorMapWidget` (C++)
- **Purpose**: Navigation map showing sectors and points of interest
- **Priority**: 🔷 **MEDIUM** - Navigation aid

---

## Actor Blueprints

### Environmental Actors

#### BP_Asteroid
- **Location**: `Content/Blueprints/Environment/BP_Asteroid.uasset`
- **Parent Class**: `AActor`
- **Purpose**: Asteroid obstacles in space
- **Priority**: 🔷 **MEDIUM** - Environmental hazards

#### BP_Planet
- **Location**: `Content/Blueprints/Environment/BP_Planet.uasset`
- **Parent Class**: `APlanet` (C++)
- **Purpose**: Planet actors
- **Priority**: 🔷 **MEDIUM** - World building

#### BP_Nebula
- **Location**: `Content/Blueprints/Environment/BP_Nebula.uasset`
- **Parent Class**: `AActor`
- **Purpose**: Nebula visual effects
- **Priority**: 🟢 **LOW** - Visual polish

### Combat Actors

#### BP_Projectile_[Type]
Create projectile variants:
- **BP_Projectile_Laser** - Laser bolt
- **BP_Projectile_Cannon** - Cannon shell
- **BP_Projectile_Missile** - Guided missile

- **Location**: `Content/Blueprints/Combat/Projectiles/`
- **Parent Class**: `AActor`
- **Purpose**: Weapon projectiles
- **Priority**: 🔶 **HIGH** - Combat system

#### BP_Explosion
- **Location**: `Content/Blueprints/Effects/BP_Explosion.uasset`
- **Parent Class**: `AActor`
- **Purpose**: Explosion visual effect
- **Priority**: 🔷 **MEDIUM** - Visual feedback

### Interactive Actors

#### BP_CargoContainer
- **Location**: `Content/Blueprints/Interactables/BP_CargoContainer.uasset`
- **Parent Class**: `AActor`
- **Purpose**: Cargo containers that can be collected
- **Priority**: 🔷 **MEDIUM** - Trading system

#### BP_Waypoint
- **Location**: `Content/Blueprints/Navigation/BP_Waypoint.uasset`
- **Parent Class**: `AActor`
- **Purpose**: Navigation waypoint markers
- **Priority**: 🔷 **MEDIUM** - Navigation system

---

## Component Blueprints

Most components are C++ classes, but you may want to create Blueprint child components for custom behavior:

#### BP_CustomWeaponComponent
- **Location**: `Content/Blueprints/Components/BP_CustomWeaponComponent.uasset`
- **Parent Class**: `UWeaponComponent` (C++)
- **Purpose**: Custom weapon behavior
- **Priority**: 🟢 **LOW** - Optional customization

---

## Input Blueprints

### Enhanced Input Assets

#### IMC_Spaceship
- **Location**: `Content/Input/IMC_Spaceship.uasset`
- **Type**: Input Mapping Context
- **Purpose**: Maps input actions to keys for spaceship control
- **Required Mappings**:
  - IA_Move → WASD keys
  - IA_Look → Mouse movement
  - IA_Boost → Left Shift
  - IA_Fire_Primary → Left Mouse Button
  - IA_Fire_Secondary → Right Mouse Button
  - IA_TargetNext → Tab
  - IA_TargetPrevious → Shift+Tab
  - IA_Autopilot → Z
- **Priority**: ⚠️ **CRITICAL** - Input won't work without this

#### IA_[ActionName]
Create Input Actions:
- **IA_Move** - Movement (Vector2D)
- **IA_Look** - Camera look (Vector2D)
- **IA_Boost** - Speed boost (Boolean)
- **IA_Fire_Primary** - Primary weapon (Boolean)
- **IA_Fire_Secondary** - Secondary weapon (Boolean)
- **IA_TargetNext** - Next target (Boolean)
- **IA_TargetPrevious** - Previous target (Boolean)
- **IA_Autopilot** - Toggle autopilot (Boolean)
- **IA_OpenInventory** - Open inventory (Boolean)
- **IA_OpenMap** - Open map (Boolean)
- **IA_PauseMenu** - Pause game (Boolean)

- **Location**: `Content/Input/Actions/`
- **Type**: Input Action
- **Priority**: ⚠️ **CRITICAL** - Input system

#### DA_InputConfig
- **Location**: `Content/DataAssets/Input/DA_InputConfig.uasset`
- **Parent Class**: `UInputConfigDataAsset` (C++)
- **Purpose**: Central input configuration
- **Priority**: ⚠️ **CRITICAL** - Enhanced Input system

---

## AI and Logic Blueprints

### AI Logic

#### BP_FactionLogic_[FactionName]
- **Location**: `Content/Blueprints/AI/Factions/`
- **Parent Class**: `UFactionLogic` (C++)
- **Purpose**: Custom AI logic for specific factions
- **Priority**: 🔷 **MEDIUM** - Advanced AI behavior

#### BP_PersonnelLogic_[Personality]
- **Location**: `Content/Blueprints/AI/Personnel/`
- **Parent Class**: `UPersonnelLogic` (C++)
- **Purpose**: Custom AI logic for personnel
- **Priority**: 🔷 **MEDIUM** - Crew interactions

### AI Controllers

#### BP_NPCShipController
- **Location**: `Content/Blueprints/AI/BP_NPCShipController.uasset`
- **Parent Class**: `AAIController`
- **Purpose**: AI controller for NPC ships
- **Priority**: 🔷 **MEDIUM** - NPC behavior

---

## Quest and Progression Blueprints

### Tutorial Blueprints

#### DA_TutorialStep_[Name]
Create tutorial steps:
- **DA_TutorialStep_Welcome** - Welcome message
- **DA_TutorialStep_Movement** - Movement tutorial
- **DA_TutorialStep_Combat** - Combat tutorial
- **DA_TutorialStep_Trading** - Trading tutorial

- **Location**: `Content/DataAssets/Tutorials/`
- **Parent Class**: `UTutorialStepDataAsset` (C++)
- **Purpose**: Tutorial step definitions
- **Priority**: 🔷 **MEDIUM** - New player experience

### Achievement Blueprints

#### DA_Achievement_[Name]
- **Location**: `Content/DataAssets/Achievements/`
- **Parent Class**: `UAchievementDataAsset` (C++)
- **Purpose**: Achievement definitions
- **Priority**: 🟢 **LOW** - Progression tracking

---

## Optional Enhancement Blueprints

### Audio

#### DA_MusicTrack_[Name]
- **Location**: `Content/DataAssets/Audio/Music/`
- **Parent Class**: `UMusicTrackDataAsset` (C++)
- **Purpose**: Music track configurations
- **Priority**: 🟢 **LOW** - Audio system

#### DA_SoundEffect_[Name]
- **Location**: `Content/DataAssets/Audio/SFX/`
- **Parent Class**: `USoundEffectDataAsset` (C++)
- **Purpose**: Sound effect configurations
- **Priority**: 🟢 **LOW** - Audio system

### Ship Upgrades

#### DA_ShipUpgrade_[Type]
- **Location**: `Content/DataAssets/Upgrades/`
- **Parent Class**: `UShipUpgradeDataAsset` (C++)
- **Purpose**: Ship upgrade definitions
- **Priority**: 🟢 **LOW** - Progression system

### Visual Effects

#### BP_EngineTrail
- **Location**: `Content/Blueprints/Effects/BP_EngineTrail.uasset`
- **Parent Class**: `AActor` or Niagara System
- **Purpose**: Ship engine visual effect
- **Priority**: 🟢 **LOW** - Visual polish

---

## Quick Reference Table

### Minimum Required Blueprints (15 Total)

| Blueprint | Parent Class | Location | Why Critical |
|-----------|-------------|----------|--------------|
| BP_SpaceGameMode | AdastreaGameMode | Blueprints/GameModes/ | Game won't start |
| BP_GameInstance | AdastreaGameInstance | Blueprints/Core/ | Save/load system |
| BP_SpaceshipController | AdastreaPlayerController | Blueprints/Controllers/ | Player control |
| BP_PlayerShip | ASpaceship | Blueprints/Ships/ | Player's ship |
| WBP_SpaceshipHUD | UAdastreaHUDWidget | UI/HUD/ | Player feedback |
| WBP_MainMenu | UUserWidget | UI/Menus/ | Game entry |
| DA_Ship_PlayerScout | USpaceshipDataAsset | DataAssets/Ships/ | Ship stats |
| DA_Faction_PlayerAlliance | UFactionDataAsset | DataAssets/Factions/ | Faction system |
| IMC_Spaceship | Input Mapping Context | Input/ | Input mapping |
| IA_Move | Input Action | Input/Actions/ | Movement input |
| IA_Look | Input Action | Input/Actions/ | Camera input |
| IA_Boost | Input Action | Input/Actions/ | Boost input |
| IA_Fire_Primary | Input Action | Input/Actions/ | Combat input |
| IA_PauseMenu | Input Action | Input/Actions/ | Pause input |
| DA_InputConfig | UInputConfigDataAsset | DataAssets/Input/ | Input config |

### High Priority Blueprints (25 Total)

Add to minimum required:
- Station blueprints (BP_SpaceStation_Main + 3 module types)
- Combat blueprints (3 weapon types, 2 projectile types)
- Trading UI (WBP_TradingInterface)
- Inventory UI (WBP_InventoryWidget)
- Additional ship data assets (2-3 variants)
- Additional faction data assets (2-3 factions)
- Quest data assets (2-3 quests)
- NPC ship blueprint (1 basic type)

### Complete Blueprints (~100+ Total)

Includes all systems fully functional:
- All UI widgets (15+)
- All data assets for all systems (50+)
- All ship variants (10+)
- All station modules (10+)
- All NPC types (5+)
- All environmental actors (5+)
- Complete input actions (15+)

---

## Testing Checklist

### Phase 1: Critical Systems (Use Minimum Required)
- [ ] Game starts and loads main menu
- [ ] Can start new game from menu
- [ ] Player ship spawns in level
- [ ] WASD controls ship movement
- [ ] Mouse controls camera
- [ ] HUD displays ship information
- [ ] Can pause game
- [ ] Can quit game

### Phase 2: Core Gameplay (Use High Priority)
- [ ] Can dock at space station
- [ ] Trading interface opens and functions
- [ ] Can buy/sell goods
- [ ] Inventory system works
- [ ] Can fire weapons
- [ ] Combat damage system works
- [ ] Can see faction relationships
- [ ] Multiple ship types available

### Phase 3: Complete Features (Use Complete Set)
- [ ] All UI menus functional
- [ ] Quest system tracks objectives
- [ ] Tutorial system guides player
- [ ] NPC ships patrol and trade
- [ ] Station modules can be added/removed
- [ ] Save/load system works
- [ ] All factions have unique traits
- [ ] Audio plays correctly
- [ ] Visual effects display
- [ ] Achievement tracking works

---

## Project Settings Configuration

After creating Blueprints, configure these settings in **Edit → Project Settings**:

### Maps & Modes
```
Default Maps:
- Editor Startup Map: [Your test level]
- Game Default Map: MainMenu
- Transition Map: None

Default Modes:
- Default GameMode: BP_SpaceGameMode
- Global Default Game Mode: BP_SpaceGameMode
```

### Input
```
Default Classes:
- Default Player Input Class: EnhancedPlayerInput
- Default Input Component Class: EnhancedInputComponent

Input Settings:
- Enable Enhanced Input System: ✓ Enabled
```

### Engine
```
Game Instance Class: BP_GameInstance
```

---

## Folder Structure

Recommended organization:

```
Content/
├── Blueprints/
│   ├── Core/
│   │   └── BP_GameInstance
│   ├── GameModes/
│   │   └── BP_SpaceGameMode
│   ├── Controllers/
│   │   ├── BP_SpaceshipController
│   │   └── BP_NPCShipController
│   ├── Ships/
│   │   ├── BP_PlayerShip
│   │   └── NPCs/
│   │       ├── BP_Ship_NPC_Fighter
│   │       └── BP_Ship_NPC_Trader
│   ├── Stations/
│   │   ├── BP_SpaceStation_Main
│   │   └── Modules/
│   │       ├── BP_Module_Docking
│   │       ├── BP_Module_Power
│   │       └── [Other modules]
│   ├── Combat/
│   │   └── Projectiles/
│   │       ├── BP_Projectile_Laser
│   │       └── BP_Projectile_Cannon
│   ├── Environment/
│   │   ├── BP_Asteroid
│   │   └── BP_Planet
│   ├── AI/
│   │   ├── Factions/
│   │   └── Personnel/
│   └── Components/
│
├── DataAssets/
│   ├── Ships/
│   │   └── DA_Ship_*.uasset
│   ├── Factions/
│   │   └── DA_Faction_*.uasset
│   ├── Personnel/
│   │   └── DA_Personnel_*.uasset
│   ├── Trading/
│   │   ├── Items/
│   │   │   └── DA_TradeItem_*.uasset
│   │   └── Markets/
│   │       └── DA_Market_*.uasset
│   ├── Weapons/
│   │   └── DA_Weapon_*.uasset
│   ├── Quests/
│   │   └── DA_Quest_*.uasset
│   ├── Homeworlds/
│   │   └── DA_Homeworld_*.uasset
│   ├── Materials/
│   │   └── DA_Material_*.uasset
│   ├── Input/
│   │   └── DA_InputConfig.uasset
│   ├── Tutorials/
│   │   └── DA_TutorialStep_*.uasset
│   └── Audio/
│       ├── Music/
│       └── SFX/
│
├── UI/
│   ├── HUD/
│   │   └── WBP_SpaceshipHUD
│   ├── Menus/
│   │   ├── WBP_MainMenu
│   │   ├── WBP_PauseMenu
│   │   └── WBP_SettingsMenu
│   ├── Trading/
│   │   └── WBP_TradingInterface
│   ├── Stations/
│   │   ├── WBP_StationManagement
│   │   └── WBP_StationEditor
│   ├── Inventory/
│   │   └── WBP_InventoryWidget
│   ├── Ships/
│   │   └── WBP_ShipCustomization
│   ├── Quests/
│   │   └── WBP_QuestLog
│   └── Map/
│       └── WBP_SectorMap
│
└── Input/
    ├── IMC_Spaceship.uasset
    └── Actions/
        ├── IA_Move.uasset
        ├── IA_Look.uasset
        └── [Other actions]
```

---

## Additional Resources

### Documentation References

- **BLUEPRINT_CREATION_GUIDE.md** - Step-by-step Blueprint creation
- **CONTENT_CREATION_QUICKSTART.md** - 30-minute playable guide
- **Assets/BlueprintWorkflowTemplates.md** - Blueprint workflow patterns
- **Assets/DesignerOnboarding.md** - Designer onboarding guide
- **Assets/[System]Guide.md** - System-specific documentation

### Templates

- **Assets/SpaceshipTemplates/** - YAML ship templates
- **Assets/FactionSetupGuide.md** - Faction configuration
- **Assets/PersonnelTemplates/** - Personnel YAML templates
- **Assets/TradingTemplates/** - Trading system templates

### Automation Tools

- **YAMLtoDataAsset.py** - Import YAML to Data Assets
- **ProceduralGenerators.py** - Generate content
- **TemplateGenerator.py** - Generate YAML templates
- **SetupCheck.py** - Validate setup

---

## Troubleshooting Common Issues

### Blueprint Won't Compile
- Check parent class is correct
- Verify all required components are added
- Look for missing variable references
- Check for circular dependencies

### Input Not Working
- Verify IMC is added in PlayerController BeginPlay
- Check Input Actions are created
- Ensure mappings are correct in IMC
- Verify Enhanced Input subsystem is enabled in Project Settings

### Ship Doesn't Spawn
- Check Game Mode Default Pawn Class is set
- Verify Player Start is in level
- Check spawn collision settings
- Look at Output Log for errors

### HUD Not Visible
- Verify widget is added to viewport in PlayerController
- Check widget visibility settings
- Ensure Z-order is correct
- Check if widget has content

### Data Asset References Broken
- Re-assign Data Asset references
- Check Data Asset exists at expected path
- Verify Data Asset parent class is correct
- Look for renamed or moved assets

---

## Priority Legend

- ⚠️ **CRITICAL** - Absolutely required for game to run
- 🔶 **HIGH** - Required for core gameplay features
- 🔷 **MEDIUM** - Important for full functionality
- 🟢 **LOW** - Enhancement and polish

---

## Version History

- **1.0** (November 16, 2025) - Initial comprehensive blueprint list

---

## Contributing

Found missing Blueprints or have suggestions? Please:
1. Update this document
2. Test your additions
3. Submit a pull request
4. Update CHANGELOG.md

---

**For questions or help**: See README.md or open an issue on GitHub
