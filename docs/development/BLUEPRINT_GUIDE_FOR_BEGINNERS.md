# Blueprint Guide for Complete Beginners - Adastrea

> **A comprehensive, beginner-friendly guide to creating every Blueprint you need in Adastrea**

**Last Updated**: November 18, 2025  
**For**: Adastrea 1.0.0-alpha  
**Engine**: Unreal Engine 5.6  
**Difficulty**: Complete Beginner (No Blueprint experience required)

---

## 📖 Table of Contents

1. [What Are Blueprints?](#what-are-blueprints)
2. [Blueprint Basics](#blueprint-basics)
3. [Complete Blueprint List by Priority](#complete-blueprint-list-by-priority)
4. [Detailed Blueprint Creation Instructions](#detailed-blueprint-creation-instructions)
5. [Testing Your Blueprints](#testing-your-blueprints)
6. [Troubleshooting Common Issues](#troubleshooting-common-issues)
7. [Next Steps](#next-steps)

---

## What Are Blueprints?

### Simple Explanation

Think of Blueprints as **visual templates** for game objects. Instead of writing code, you connect nodes (like building blocks) to create game logic. In Adastrea:

- **Blueprint Actors** = Things that exist in the game world (ships, stations, planets)
- **Blueprint Widgets** = UI elements (menus, HUD, inventory screens)
- **Blueprint Data Assets** = Configuration files (ship stats, Way guild info, weapon specs)
- **Blueprint Components** = Parts of actors (weapons, health, navigation)

### Why Blueprints Matter

The C++ code in Adastrea provides the **foundation** (the engine, physics, core systems), but Blueprints let you:
- Create specific ship types without coding
- Design UI without touching C++
- Configure game content quickly
- Test and iterate rapidly

**Key Point**: You don't need to understand the C++ code to create content in Adastrea. Blueprints handle everything!

---

## Blueprint Basics

### How to Create a Blueprint

1. **Open Unreal Engine** and load `Adastrea.uproject`
2. **Open Content Browser** (bottom panel in editor)
3. **Right-click** in the Content Browser
4. **Select the Blueprint type** you want to create
5. **Choose a parent class** (explained below)
6. **Name it** using proper conventions (explained below)
7. **Double-click** to open and edit

### Blueprint Types You'll Create

| Type | What It Is | Example |
|------|------------|---------|
| **Blueprint Class** | An actor or object in the game | BP_PlayerShip |
| **Widget Blueprint** | UI element | WBP_MainMenu |
| **Data Asset** | Configuration data | DA_Ship_PlayerScout |
| **Input Action** | Player input (keys, buttons) | IA_Move |
| **Input Mapping Context** | Maps inputs to actions | IMC_Spaceship |

### Naming Conventions (VERY IMPORTANT!)

Always use these prefixes so you can find things easily:

| Prefix | Meaning | Example |
|--------|---------|---------|
| `BP_` | Blueprint Actor | BP_PlayerShip |
| `WBP_` | Widget Blueprint | WBP_SpaceshipHUD |
| `DA_` | Data Asset | DA_Ship_Fighter |
| `IA_` | Input Action | IA_Fire_Primary |
| `IMC_` | Input Mapping Context | IMC_Spaceship |

### Parent Classes (What Are They?)

When you create a Blueprint, you choose a **parent class**. This is like choosing what "type" of thing you're making:

- **Want to make a ship?** Parent = `ASpaceship` (our C++ class)
- **Want to make a station?** Parent = `ASpaceStation`
- **Want to make a menu?** Parent = `UUserWidget`
- **Want to make ship stats?** Parent = `USpaceshipDataAsset`

The parent class gives your Blueprint built-in functionality. You just fill in the details!

---

## Complete Blueprint List by Priority

### 🚨 PHASE 1: Critical - Game Won't Run Without These (15 Blueprints)

**Time Required**: ~2 hours  
**Goal**: Get a basic playable game with a flying ship

#### Game Framework (5 Blueprints)

1. **BP_SpaceGameMode**
   - **Type**: Blueprint Class
   - **Parent**: `AdastreaGameMode`
   - **Location**: `Content/Blueprints/GameModes/`
   - **What it does**: Controls the game rules, spawns the player
   - **What you set**:
     - Default Pawn Class → `BP_PlayerShip`
     - Player Controller Class → `BP_SpaceshipController`

2. **BP_GameInstance**
   - **Type**: Blueprint Class
   - **Parent**: `AdastreaGameInstance`
   - **Location**: `Content/Blueprints/Core/`
   - **What it does**: Manages save/load, persistent data
   - **What you set**:
     - PlayerName (String) = "Commander"
     - CurrentCredits (Integer) = 1000
     - SaveSlotName (String) = "SaveSlot1"

3. **BP_SpaceshipController**
   - **Type**: Blueprint Class
   - **Parent**: `AdastreaPlayerController`
   - **Location**: `Content/Blueprints/Controllers/`
   - **What it does**: Handles player input and camera
   - **What you add**: Enhanced Input setup in BeginPlay event

4. **BP_PlayerShip**
   - **Type**: Blueprint Class
   - **Parent**: `ASpaceship`
   - **Location**: `Content/Blueprints/Ships/`
   - **What it does**: The player's controllable spaceship
   - **Components to add**:
     - Static Mesh Component (ship visual)
     - Spring Arm Component (camera boom)
     - Camera Component (player view)
     - Arrow Component (weapon spawn point)

5. **WBP_SpaceshipHUD**
   - **Type**: Widget Blueprint
   - **Parent**: `UAdastreaHUDWidget`
   - **Location**: `Content/UI/HUD/`
   - **What it does**: Shows ship health, speed, shields on screen
   - **UI Elements to add**:
     - Text: Ship name
     - Text: Speed display
     - Progress Bar: Health
     - Progress Bar: Shields
     - Image: Crosshair

#### Enhanced Input System (1-2 Blueprints Needed)

**⚠️ GOOD NEWS**: Most Input Actions already exist! The game uses C++ `InputConfigDataAsset` for centralized input management.

**What Already Exists** (in `Content/Input/`):
- ✅ `IMC_Spaceship` - Input Mapping Context
- ✅ `IA_Move` - Movement input
- ✅ `IA_Look` - Camera look input
- ✅ `IA_Boost` - Speed boost input
- ✅ `IA_Fire_Primary` - Primary weapon input
- ✅ `IA_PauseMenu` - Pause menu input

**What You Need to Create**:

6. **DA_InputConfig** (Most Important)
   - **Type**: Data Asset
   - **Parent**: `UInputConfigDataAsset` (C++ class)
   - **Location**: `Content/DataAssets/Input/`
   - **What it does**: References all Input Actions and Mapping Contexts in one place
   - **What you set**: Assign the existing Input Actions and IMC_Spaceship
   - **Priority**: ⚠️ **Create this first!**

7. **Additional Input Actions** (Only if needed)
   - Create more Input Actions in `Content/Input/Actions/` if you need additional controls
   - Examples: `IA_Fire_Secondary`, `IA_TargetNext`, `IA_Autopilot`
   - See [Assets/EnhancedInputImplementation.md](Assets/EnhancedInputImplementation.md) for full list

#### Data Assets (2 Blueprints)

8. **DA_Ship_PlayerScout**
    - **Type**: Data Asset
    - **Parent**: `USpaceshipDataAsset`
    - **Location**: `Content/DataAssets/Ships/`
    - **What it does**: Defines your starting ship's stats
    - **What you set**: (Use template from `Assets/SpaceshipTemplates/PathfinderScout.yaml`)
      - DisplayName = "Pathfinder Scout"
      - MaxHealth = 1000
      - MaxShield = 500
      - ForwardSpeed = 500
      - Maneuverability = 70
      - (many more stats available)

9. **DA_Way_TradersGuild**
    - **Type**: Data Asset
    - **Parent**: `UWayDataAsset` (Way system - specialized guilds)
    - **Location**: `Content/DataAssets/Ways/`
    - **What it does**: Your starting guild/Way configuration
    - **What you set**:
      - WayName = "Traders Guild"
      - Specialization = "Trading"
      - Colors (guild color scheme)

#### UI (1 Blueprint)

10. **WBP_MainMenu**
    - **Type**: Widget Blueprint
    - **Parent**: `UUserWidget`
    - **Location**: `Content/UI/Menus/`
    - **What it does**: Main menu when game starts
    - **UI Elements to add**:
      - Button: Start Game
      - Button: Load Game
      - Button: Settings
      - Button: Quit Game

**✅ After Phase 1, you should have**: A flying ship with HUD that you can control!

---

### 🔶 PHASE 2: High Priority - Core Gameplay (25 Additional Blueprints)

**Time Required**: ~4 hours  
**Goal**: Add combat, trading, stations, and NPCs

#### Space Stations (5 Blueprints)

11. **BP_SpaceStation_Main**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceStation`
    - **Location**: `Content/Blueprints/Stations/`
    - **What it does**: Main station you can dock at
    - **What you set**: Owning Way (optional), initial modules

12. **BP_Module_Docking**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceStationModule`
    - **Location**: `Content/Blueprints/Stations/Modules/`
    - **What it does**: Docking port for ships
    - **Visual**: Connection point for ships

13. **BP_Module_Power**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceStationModule`
    - **Location**: `Content/Blueprints/Stations/Modules/`
    - **What it does**: Generates power for station
    - **Visual**: Generator or reactor appearance

14. **BP_Module_Storage**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceStationModule`
    - **Location**: `Content/Blueprints/Stations/Modules/`
    - **What it does**: Cargo storage area
    - **Visual**: Warehouse module

15. **BP_Module_Defence**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceStationModule`
    - **Location**: `Content/Blueprints/Stations/Modules/`
    - **What it does**: Weapons and shields
    - **Visual**: Turrets or shield generators

#### Combat System (5 Blueprints)

21. **DA_Weapon_Laser**
    - **Type**: Data Asset
    - **Parent**: `UWeaponDataAsset`
    - **Location**: `Content/DataAssets/Weapons/`
    - **What it does**: Laser weapon stats
    - **What you set**:
      - WeaponName = "Pulse Laser"
      - Damage = 50
      - FireRate = 0.5
      - Range = 2000
      - ProjectileSpeed = 3000

22. **DA_Weapon_Cannon**
    - **Type**: Data Asset
    - **Parent**: `UWeaponDataAsset`
    - **Location**: `Content/DataAssets/Weapons/`
    - **What it does**: Cannon weapon stats
    - **What you set**:
      - WeaponName = "Mass Driver"
      - Damage = 150
      - FireRate = 1.5
      - Range = 3000

23. **DA_Weapon_Missile**
    - **Type**: Data Asset
    - **Parent**: `UWeaponDataAsset`
    - **Location**: `Content/DataAssets/Weapons/`
    - **What it does**: Missile launcher stats
    - **What you set**:
      - WeaponName = "Seeker Missile"
      - Damage = 300
      - FireRate = 3.0
      - IsHoming = true

24. **BP_Projectile_Laser**
    - **Type**: Blueprint Class
    - **Parent**: `AActor`
    - **Location**: `Content/Blueprints/Combat/Projectiles/`
    - **What it does**: Visual projectile for lasers
    - **Components**: Particle effect, collision

25. **BP_Projectile_Cannon**
    - **Type**: Blueprint Class
    - **Parent**: `AActor`
    - **Location**: `Content/Blueprints/Combat/Projectiles/`
    - **What it does**: Visual projectile for cannons
    - **Components**: Mesh, collision, trail effect

#### More Ships (3 Blueprints)

26. **DA_Ship_PlayerFighter**
    - **Type**: Data Asset
    - **Parent**: `USpaceshipDataAsset`
    - **Location**: `Content/DataAssets/Ships/`
    - **What it does**: Combat-focused ship upgrade
    - **Higher stats**: Weapons, shields, armor

27. **DA_Ship_NPC_Fighter**
    - **Type**: Data Asset
    - **Parent**: `USpaceshipDataAsset`
    - **Location**: `Content/DataAssets/Ships/`
    - **What it does**: Enemy fighter stats

28. **BP_Ship_NPC_Fighter**
    - **Type**: Blueprint Class
    - **Parent**: `ASpaceship`
    - **Location**: `Content/Blueprints/Ships/NPCs/`
    - **What it does**: AI-controlled enemy ship
    - **Uses**: DA_Ship_NPC_Fighter for stats

#### Trading System (5 Blueprints)

29. **WBP_TradingUI**
    - **Type**: Widget Blueprint
    - **Parent**: `UTradingInterfaceWidget`
    - **Location**: `Content/UI/Trading/`
    - **What it does**: UI for buying/selling goods
    - **UI Elements**: Item list, buy/sell buttons, price display

30. **DA_TradeItem_Food**
    - **Type**: Data Asset
    - **Parent**: `UTradeItemDataAsset`
    - **Location**: `Content/DataAssets/Trading/Items/`
    - **What it does**: Food commodity
    - **What you set**:
      - ItemName = "Food Supplies"
      - BasePrice = 50
      - VolumPerUnit = 1
      - Category = "Consumables"

31. **DA_TradeItem_Minerals**
    - **Type**: Data Asset
    - **Parent**: `UTradeItemDataAsset`
    - **Location**: `Content/DataAssets/Trading/Items/`
    - **What it does**: Mining resources
    - **Higher volume/weight**: Industrial goods

32. **DA_TradeItem_Technology**
    - **Type**: Data Asset
    - **Parent**: `UTradeItemDataAsset`
    - **Location**: `Content/DataAssets/Trading/Items/`
    - **What it does**: Tech equipment
    - **Higher value**: Advanced goods

33. **DA_Market_Station1**
    - **Type**: Data Asset
    - **Parent**: `UMarketDataAsset`
    - **Location**: `Content/DataAssets/Trading/Markets/`
    - **What it does**: Market configuration for a station
    - **What you set**: Available items, buy/sell prices

#### Additional Systems (7 Blueprints)

34. **WBP_InventoryWidget**
    - **Type**: Widget Blueprint
    - **Parent**: `UInventoryWidget`
    - **Location**: `Content/UI/Inventory/`
    - **What it does**: Shows player's cargo and items

35. **DA_Way_Merchants**
    - **Type**: Data Asset
    - **Parent**: `UWayDataAsset` (Way system)
    - **Location**: `Content/DataAssets/Ways/`
    - **What it does**: Trading guild/Way
    - **Traits**: Economic bonuses, trade focus

36. **DA_Way_Freelancers**
    - **Type**: Data Asset
    - **Parent**: `UWayDataAsset` (Way system)
    - **Location**: `Content/DataAssets/Ways/`
    - **What it does**: Independent operators guild/Way
    - **Traits**: Flexible roles, no allegiances

37. **DA_Quest_Tutorial_FirstFlight**
    - **Type**: Data Asset
    - **Parent**: `UQuestDataAsset`
    - **Location**: `Content/DataAssets/Quests/`
    - **What it does**: Tutorial quest
    - **Objectives**: "Learn to fly", "Dock at station"

38. **DA_Homeworld_EarthStation**
    - **Type**: Data Asset
    - **Parent**: `UHomeworldDataAsset`
    - **Location**: `Content/DataAssets/Homeworlds/`
    - **What it does**: Starting location
    - **What you set**: Starting location, credits, ship

39. **DA_Personnel_Captain**
    - **Type**: Data Asset
    - **Parent**: `UPersonnelDataAsset`
    - **Location**: `Content/DataAssets/Personnel/`
    - **What it does**: Ship captain crew member
    - **Skills**: Leadership, tactical

40. **DA_Personnel_Pilot**
    - **Type**: Data Asset
    - **Parent**: `UPersonnelDataAsset`
    - **Location**: `Content/DataAssets/Personnel/`
    - **What it does**: Pilot crew member
    - **Skills**: Navigation, maneuvering

**✅ After Phase 2, you should have**: Combat, trading, stations, and a full gameplay loop!

---

### 🔷 PHASE 3: Complete Systems (60+ Additional Blueprints)

**Time Required**: ~8-10 hours  
**Goal**: Polish and complete all game systems

#### All Station Modules (6 more - total 10)

41-46. **BP_Module_[Type]**
    - **Types**: Habitation, Command, Industrial, Trade, Research, Entertainment
    - **Location**: `Content/Blueprints/Stations/Modules/`
    - **Each module**: Unique visual, specific function

#### More Ship Variants (5 more)

47. **DA_Ship_PlayerTrader** - Cargo-focused
48. **DA_Ship_NPC_Trader** - AI trader
49. **DA_Ship_NPC_Hauler** - Large cargo ship
50. **DA_Ship_NPC_Scout** - Fast explorer
51. **BP_Ship_NPC_Trader** - AI trader actor

#### Complete UI Set (10 widgets)

52. **WBP_PauseMenu** - In-game pause
53. **WBP_SettingsMenu** - Game settings
54. **WBP_StationManagement** - Station overview
55. **WBP_StationEditor** - Build modules
56. **WBP_ShipCustomization** - Customize ship
57. **WBP_QuestLog** - Track missions
58. **WBP_SectorMap** - Navigation map
59. **WBP_CharacterCreation** - New game setup
60. **WBP_DialogueInterface** - NPC conversations
61. **WBP_TutorialOverlay** - Tutorial popups

#### Complete Input Actions (10 more - total 15)

62-71. **IA_[Action]**
    - TargetNext, TargetPrevious, Fire_Secondary
    - Autopilot, ToggleMap, OpenInventory
    - OpenQuestLog, Interact, Dock, Jump

#### More Data Assets (30+)

72-76. **5 More Ways** - Diverse guild/specialization types (Explorers, Mercenaries, Miners, Scientists)
77-96. **20 More Personnel Roles** - Complete crew roster
97-106. **10 More Trade Items** - Full economy
107-116. **10 More Quests** - Story content
117-121. **5 More Weapons** - Weapon variety
122-129. **8 Materials** - Crafting resources
130-139. **10 Tutorials** - Learning content

#### Environment & Effects (8)

140. **BP_Asteroid** - Space hazard
141. **BP_Planet** - Planet actor
142. **BP_Nebula** - Visual effect
143. **BP_Explosion** - Combat effect
144. **BP_EngineTrail** - Ship engine VFX
145. **BP_ShieldEffect** - Shield VFX
146. **BP_WarpEffect** - Jump effect
147. **BP_DamageEffect** - Hit feedback

**✅ After Phase 3, you should have**: A complete, production-ready game!

---

## Detailed Blueprint Creation Instructions

### How to Create a Blueprint Class (Actors)

**Example**: Creating BP_PlayerShip

1. **Navigate to folder**:
   - Open Content Browser
   - Navigate to `Content/Blueprints/Ships/`
   - Create folder if it doesn't exist (Right-click → New Folder)

2. **Create Blueprint**:
   - Right-click in Content Browser
   - Select `Blueprint Class`
   - Window opens: "Pick Parent Class"

3. **Choose Parent Class**:
   - Click on `All Classes` (top-right)
   - Search for `Spaceship`
   - Click on `Spaceship` class
   - Click `Select` button

4. **Name it**:
   - Type: `BP_PlayerShip`
   - Press Enter

5. **Open Blueprint**:
   - Double-click `BP_PlayerShip` icon
   - Blueprint Editor opens

6. **Add Components**:
   - Look at left panel: "Components"
   - Click `+ Add` button
   - Search and add:
     - `Static Mesh Component`
     - `Spring Arm Component`
     - `Camera Component`
   
7. **Configure Components**:
   - Click each component in list
   - Look at right panel: "Details"
   - For Static Mesh:
     - Find "Static Mesh" property
     - Click dropdown, select a mesh (or use basic cube for testing)
     - Set Location: (0, 0, 0)
     - Set Rotation: (0, -90, 0) to point forward
     - Set Scale: (2.0, 1.0, 0.5) for ship shape
   - For Spring Arm:
     - Set Target Arm Length: 800
     - Enable "Use Pawn Control Rotation"
   - For Camera:
     - Drag onto Spring Arm to attach it
     - No special settings needed

8. **Add Variables** (if needed):
   - Look at left panel: "My Blueprint"
   - Click `+ Variable` button
   - Name it (e.g., "ForwardSpeed")
   - Click variable, look at Details panel
   - Set Variable Type: Float
   - Set Default Value: 500.0
   - Set Category: "Movement"

9. **Compile and Save**:
   - Click `Compile` button (top toolbar)
   - Green checkmark = success!
   - Click `Save` button
   - Close Blueprint Editor

### How to Create a Widget Blueprint (UI)

**Example**: Creating WBP_MainMenu

1. **Navigate and Create**:
   - Go to `Content/UI/Menus/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_MainMenu`

2. **Open Widget Editor**:
   - Double-click widget
   - Designer view opens (visual editor)

3. **Design Layout**:
   - Left panel: "Palette" (available UI elements)
   - Drag elements from Palette to canvas:
     - Canvas Panel (root - already there)
     - Vertical Box (drag onto canvas)
     - Text blocks (for titles)
     - Buttons (for menu options)
   
4. **Configure Button**:
   - Click button in hierarchy
   - Details panel on right:
     - Set Size To Content: ✓
     - Padding: 10 on all sides
   - Expand button in hierarchy
   - Click Text inside button
     - Set Text: "Start Game"
     - Set Font Size: 24
     - Set Justification: Center

5. **Position Elements**:
   - Select Vertical Box
   - Details → Anchors: Center
   - Position: (0, 0) relative to center
   - Size: (400, 600)
   - Set Vertical Alignment: Center

6. **Add Button Logic**:
   - Click "Graph" tab (top-right, next to Designer)
   - In left panel, find your button under Variables
   - Right-click button → Add Event → On Clicked
   - Nodes appear in Event Graph
   - Drag from On Clicked pin
   - Search: "Open Level"
   - Connect nodes
   - Set Level Name: "TestLevel"

7. **Compile and Save**:
   - Click Compile
   - Click Save
   - Close editor

### How to Create a Data Asset

**Example**: Creating DA_Ship_PlayerScout

1. **Navigate and Create**:
   - Go to `Content/DataAssets/Ships/`
   - Right-click → Miscellaneous → Data Asset

2. **Choose Data Asset Class**:
   - Window opens
   - Search: "SpaceshipDataAsset"
   - Click `SpaceshipDataAsset`
   - Click Select

3. **Name it**:
   - Type: `DA_Ship_PlayerScout`
   - Press Enter

4. **Open and Configure**:
   - Double-click data asset
   - Details panel shows ALL properties
   - Fill in values:

**Basic Info Section**:
```
Display Name: "Pathfinder Scout"
Description: "Fast, lightweight exploration vessel"
Manufacturer: "Solaris Shipyards"
Ship Class: "Scout"
```

**Core Stats Section**:
```
Max Health: 1000.0
Max Shield Capacity: 500.0
Shield Recharge Rate: 25.0
Hull Armor Rating: 50.0
Mass: 15000.0
```

**Combat Stats Section**:
```
Weapon Hardpoints: 2
Max Weapon Power: 500.0
Targeting System Range: 3000.0
Point Defense Rating: 30.0
```

**Mobility Stats Section**:
```
Max Forward Speed: 500.0
Max Reverse Speed: 200.0
Max Strafe Speed: 300.0
Acceleration Rate: 100.0
Max Turn Rate (Yaw): 50.0
Max Pitch Rate: 40.0
Max Roll Rate: 60.0
Maneuverability Rating: 70.0
```

**Pro Tip**: Use the YAML templates in `Assets/SpaceshipTemplates/` to copy values!

5. **Save**: Click Save button

### How to Create Input Actions

**Example**: Creating IA_Move

1. **Navigate**: Go to `Content/Input/Actions/`

2. **Create**:
   - Right-click → Input → Input Action
   - Name: `IA_Move`

3. **Configure**:
   - Double-click to open
   - Details panel:
     - Value Type: Axis2D (Vector2D)
   - Save

4. **Repeat** for other actions:
   - `IA_Look` - Axis2D
   - `IA_Boost` - Digital (bool)
   - `IA_Fire_Primary` - Digital (bool)

### How to Create Input Mapping Context

**Example**: Creating IMC_Spaceship

1. **Create**:
   - Go to `Content/Input/`
   - Right-click → Input → Input Mapping Context
   - Name: `IMC_Spaceship`

2. **Open and Map**:
   - Double-click to open
   - Click `+ Mappings` button (bottom)
   - For each mapping:
     - Click dropdown, select Input Action (e.g., IA_Move)
     - Click `+` next to it to add a key
     - Click `Keyboard` or `Mouse` icon
     - Press the key you want (e.g., W for forward)
     - For axes (Move, Look), add multiple keys:

**IA_Move Mappings**:
```
W key → Modifiers: Swizzle Input Axis Values (X=0, Y=1)
S key → Modifiers: Swizzle Input Axis Values (X=0, Y=-1), Negate
A key → Modifiers: Swizzle Input Axis Values (X=-1, Y=0), Negate
D key → Modifiers: Swizzle Input Axis Values (X=1, Y=0)
```

**IA_Look Mappings**:
```
Mouse X → No modifiers needed
Mouse Y → Modifier: Negate (invert Y axis)
```

**IA_Boost Mapping**:
```
Left Shift → No modifiers
```

**IA_Fire_Primary Mapping**:
```
Left Mouse Button → No modifiers
```

3. **Save**: Click Save

---

## Testing Your Blueprints

### Phase 1 Testing (After First 15 Blueprints)

1. **Set Project Settings**:
   - Edit → Project Settings
   - Maps & Modes:
     - Default GameMode: `BP_SpaceGameMode`
     - Editor Startup Map: Your test level
     - Game Default Map: `MainMenu`
   - Input:
     - Default Player Input Class: EnhancedPlayerInput
     - Default Input Component Class: EnhancedInputComponent
   - Engine - General Settings:
     - Game Instance Class: `BP_GameInstance`

2. **Create a Test Level**:
   - File → New Level → Empty Level
   - Save as: `Content/Maps/TestLevel`
   - Add to level:
     - Directional Light
     - Sky Atmosphere
     - Sky Light
     - Player Start (drag into scene)

3. **Test in Editor**:
   - Click Play button (Alt+P)
   - You should:
     - Spawn in your ship
     - See HUD on screen
     - Move with WASD
     - Look with mouse
     - Boost with Shift

4. **If it doesn't work**, check:
   - BP_SpaceGameMode has Default Pawn Class set
   - BP_SpaceshipController adds Input Mapping Context in BeginPlay
   - IMC_Spaceship has all key mappings
   - Player Start is in the level
   - Output Log for errors (Window → Developer Tools → Output Log)

### Phase 2 Testing (After Combat/Trading)

1. **Place Station in Level**:
   - Drag BP_SpaceStation_Main into level
   - Position it in view of Player Start

2. **Test Combat**:
   - Place BP_Ship_NPC_Fighter in level
   - Start game
   - Fire weapon (Left Mouse Button)
   - Projectile should spawn and move

3. **Test Trading**:
   - Fly near station
   - Open trading UI (need to implement interaction first)
   - Should see trading interface

### Phase 3 Testing (Complete Game)

1. **Full Playthrough**:
   - Start game from main menu
   - Create character
   - Complete tutorial quest
   - Dock at station
   - Trade goods
   - Accept mission
   - Combat encounter
   - Return to station

2. **Test All UI**:
   - Open every menu
   - Click every button
   - Check for errors

3. **Test Save/Load**:
   - Play for a bit
   - Save game
   - Quit
   - Reload save
   - Verify state is restored

---

## Troubleshooting Common Issues

### "Blueprint won't compile"

**Problem**: Red X on compile button, errors listed

**Solutions**:
1. **Read the error message** - it tells you what's wrong
2. **Missing parent class?**
   - Delete Blueprint and recreate with correct parent
3. **Missing variable connection?**
   - Check Event Graph, red nodes = problems
   - Right-click red node → "Refresh Nodes"
4. **Circular dependency?**
   - Blueprint A references Blueprint B, which references A
   - Break the circular reference

### "Input doesn't work"

**Problem**: Keys don't move ship

**Solutions**:
1. **Enhanced Input not initialized?**
   - Open BP_SpaceshipController
   - Event Graph → Event BeginPlay
   - Must have nodes to add IMC_Spaceship to player
   
2. **Input actions not mapped?**
   - Open IMC_Spaceship
   - Verify all actions have key mappings
   
3. **Project Settings not configured?**
   - Edit → Project Settings → Input
   - Verify Enhanced Input is enabled

4. **Check Output Log**:
   - Window → Developer Tools → Output Log
   - Look for "Input Action not found" errors

### "Ship doesn't spawn"

**Problem**: Game starts but no ship visible

**Solutions**:
1. **Game Mode not set?**
   - Edit → Project Settings → Maps & Modes
   - Set Default GameMode: BP_SpaceGameMode
   
2. **Default Pawn not set?**
   - Open BP_SpaceGameMode
   - Class Defaults → Default Pawn Class: BP_PlayerShip
   
3. **No Player Start in level?**
   - Add Player Start actor to level
   - Position it where you want to spawn
   
4. **Spawn collision issue?**
   - Move Player Start to open space
   - Check ship collision settings

### "HUD doesn't show"

**Problem**: Game works but no UI visible

**Solutions**:
1. **Widget not added to viewport?**
   - Open BP_SpaceshipController
   - Event BeginPlay should have:
     - Create Widget → WBP_SpaceshipHUD
     - Add to Viewport
   
2. **Widget has no content?**
   - Open WBP_SpaceshipHUD
   - Check Designer tab
   - Add UI elements if empty
   
3. **Widget visibility off?**
   - Select widget root
   - Details → Visibility: Visible
   
4. **Z-order issue?**
   - Add to Viewport → Z-Order: 0

### "Data Asset values don't work"

**Problem**: Ship has wrong stats

**Solutions**:
1. **Data Asset not assigned?**
   - Open BP_PlayerShip
   - Look for ShipDataAsset variable
   - Assign DA_Ship_PlayerScout
   
2. **Using default values?**
   - C++ code might use defaults if no Data Asset
   - Always assign a Data Asset reference
   
3. **Wrong Data Asset?**
   - Check you assigned the right one
   - Open Data Asset to verify values

### "Game crashes on start"

**Problem**: Unreal Editor crashes or freezes

**Solutions**:
1. **Check Output Log** before crash
   - Scroll up to see last error
   - Usually tells you what's wrong
   
2. **Missing references?**
   - Blueprint references a deleted asset
   - Fix: Remove the reference or recreate asset
   
3. **Infinite loop?**
   - Event Graph has a loop with no exit
   - Fix: Add delay or break condition
   
4. **Memory issue?**
   - Too many objects spawning
   - Fix: Limit spawn rates, add object pooling

### "Blueprint is slow/laggy"

**Problem**: Game runs slow when Blueprint runs

**Solutions**:
1. **Event Tick overuse?**
   - Don't do heavy calculations every frame
   - Use timers instead: Set Timer by Event
   
2. **Too many nodes?**
   - Move complex logic to functions
   - Or consider C++ for that system
   
3. **No caching?**
   - Don't call GetAllActorsOfClass every frame
   - Cache references in BeginPlay

---

## Next Steps

### After Creating Blueprints

1. **Create Content**:
   - Import 3D models for ships
   - Create materials for visuals
   - Add particle effects
   - Import sound effects

2. **Design Levels**:
   - Build space environments
   - Place stations and POIs
   - Add asteroid fields
   - Create jump gates

3. **Balance Gameplay**:
   - Adjust ship stats
   - Tune weapon damage
   - Balance economy prices
   - Test difficulty curve

4. **Add Polish**:
   - Improve UI visuals
   - Add transition animations
   - Enhance VFX
   - Implement audio system

### Learning Resources

**Unreal Engine Documentation**:
- [Blueprint Visual Scripting](https://docs.unrealengine.com/5.6/en-US/blueprints-visual-scripting-in-unreal-engine/)
- [Enhanced Input](https://docs.unrealengine.com/5.6/en-US/enhanced-input-in-unreal-engine/)
- [UMG UI Designer](https://docs.unrealengine.com/5.6/en-US/umg-ui-designer-for-unreal-engine/)

**Adastrea Documentation**:
- `CONTENT_CREATION_QUICKSTART.md` - 30-minute quick start
- `Assets/DesignerOnboarding.md` - Designer onboarding guide
- `Assets/[System]Guide.md` - System-specific guides
- `BLUEPRINT_CREATION_GUIDE.md` - Step-by-step Blueprint guide

**Templates**:
- `Assets/SpaceshipTemplates/*.yaml` - Ship templates
- `Assets/WaySystemGuide.md` - Way system (guilds) configuration
- `Assets/PersonnelTemplates/*.yaml` - Crew templates

**Tools**:
- `YAMLtoDataAsset.py` - Import YAML to Data Assets
- `ProceduralGenerators.py` - Generate content procedurally
- `SetupCheck.py` - Validate your setup

---

## Quick Reference Card

### Blueprint Creation Cheat Sheet

```
CREATE BLUEPRINT CLASS:
1. Right-click → Blueprint Class
2. Pick parent (ASpaceship, ASpaceStation, etc.)
3. Name with BP_ prefix
4. Add components in Components panel
5. Set variables in My Blueprint panel
6. Add logic in Event Graph
7. Compile and Save

CREATE WIDGET:
1. Right-click → User Interface → Widget Blueprint
2. Name with WBP_ prefix
3. Design UI in Designer tab
4. Add logic in Graph tab
5. Compile and Save

CREATE DATA ASSET:
1. Right-click → Miscellaneous → Data Asset
2. Pick parent (USpaceshipDataAsset, etc.)
3. Name with DA_ prefix
4. Fill in all properties
5. Save

CREATE INPUT ACTION:
1. Right-click → Input → Input Action
2. Name with IA_ prefix
3. Set value type (Digital, Axis2D, etc.)
4. Save

CREATE INPUT MAPPING:
1. Right-click → Input → Input Mapping Context
2. Name with IMC_ prefix
3. Add mappings for each action
4. Assign keys
5. Save
```

### Essential Hotkeys

```
Alt+P          - Play in Editor
Esc            - Stop Playing
Ctrl+S         - Save
Ctrl+Space     - Content Browser search
F             - Focus on selected object
Ctrl+D         - Duplicate
Delete         - Delete selected
Ctrl+Z         - Undo
Ctrl+Y         - Redo
```

---

## Summary

You now have a complete guide to every Blueprint you need to create in Adastrea! Here's the progression:

1. **Start with Phase 1** (15 Blueprints, 2 hours)
   - Get a basic flying ship
   - Test immediately

2. **Add Phase 2** (25 more, 4 hours)
   - Add combat and trading
   - Full gameplay loop

3. **Complete Phase 3** (60+ more, 8-10 hours)
   - Polish and complete
   - Production-ready game

**Total**: ~100+ Blueprints, ~14-16 hours

**Remember**:
- ✅ Test after each phase
- ✅ Use proper naming conventions
- ✅ Save frequently
- ✅ Check Output Log for errors
- ✅ Reference the YAML templates
- ✅ Ask for help if stuck (see README.md)

**Good luck, and happy Blueprint creating! 🚀**

---

**Related Documentation**:
- `BLUEPRINT_REQUIREMENTS_LIST.md` - Detailed Blueprint specifications
- `BLUEPRINT_CHECKLIST.md` - Phase-by-phase checklist
- `BLUEPRINT_CREATION_GUIDE.md` - Advanced creation guide
- `BLUEPRINT_QUICK_REFERENCE.md` - One-page visual reference
- `CONTENT_CREATION_QUICKSTART.md` - 30-minute quick start

**Last Updated**: November 18, 2025  
**Maintainer**: Adastrea Development Team
