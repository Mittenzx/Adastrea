# Adastrea - Unreal Engine Editor Implementation Guide

**Version:** 1.0  
**Last Updated:** 2025-11-11  
**Engine Version:** Unreal Engine 5.6

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites and Setup](#prerequisites-and-setup)
3. [Editor Interface Overview](#editor-interface-overview)
4. [Data Asset Creation](#data-asset-creation)
5. [Blueprint Implementation](#blueprint-implementation)
6. [Widget and UI Creation](#widget-and-ui-creation)
7. [Input System Setup](#input-system-setup)
8. [Material and Visual Effects](#material-and-visual-effects)
9. [Level Design and Scene Setup](#level-design-and-scene-setup)
10. [Station Editor System](#station-editor-system)
11. [Testing and Debugging](#testing-and-debugging)
12. [Editor Utilities and Tools](#editor-utilities-and-tools)
13. [Common Workflows](#common-workflows)
14. [Troubleshooting](#troubleshooting)

---

## Overview

This guide focuses on **editor-specific implementation** for Adastrea, covering everything that must be done within the Unreal Engine editor rather than in external tools or code editors. This includes:

- Creating and configuring Data Assets
- Implementing Blueprints for gameplay systems
- Designing UI widgets with UMG (Unreal Motion Graphics)
- Setting up Enhanced Input mappings
- Creating materials and visual effects
- Building and populating game levels
- Testing and debugging in-editor

### What This Guide Covers

✅ **Blueprint visual scripting** for all game systems  
✅ **Data Asset configuration** without C++ knowledge  
✅ **UMG Widget creation** for UI/HUD  
✅ **Enhanced Input system** setup  
✅ **Material editor** workflows  
✅ **Level design** and scene population  
✅ **In-editor testing** and debugging  
✅ **Editor customizations** and utilities

### What This Guide Doesn't Cover

❌ C++ code implementation (see [ARCHITECTURE.md](ARCHITECTURE.md))  
❌ External scripting (see [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md))  
❌ Version control workflows (see [CONTRIBUTING.md](CONTRIBUTING.md))  
❌ Build and compilation (see [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md))

---

## Prerequisites and Setup

### Required Software

- **Unreal Engine 5.6** installed and configured
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++ compilation
- **Git** for version control

### Initial Project Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Mittenzx/Adastrea.git
   cd Adastrea
   ```

2. **Run setup validation:**
   ```bash
   # Linux/Mac/WSL
   ./SetupCheck.sh
   
   # Or cross-platform Python
   python SetupCheck.py
   ```

3. **Generate project files:**
   - Right-click `Adastrea.uproject`
   - Select "Generate Visual Studio project files"

4. **Build the project:**
   - Open `Adastrea.sln` in Visual Studio
   - Build in "Development Editor" configuration
   - Or open `Adastrea.uproject` directly in Unreal Engine to compile

5. **Launch the editor:**
   - Open `Adastrea.uproject` in Unreal Engine
   - Wait for shader compilation to complete (first launch takes time)

### Recommended Editor Settings

**Edit → Editor Preferences:**

- **General → Loading & Saving:**
  - ✅ Enable "Auto Save"
  - Set "Auto Save Frequency" to 10 minutes

- **General → Appearance:**
  - Choose your preferred color theme
  - Enable "Display Engine Version"

- **Content Browser:**
  - ✅ Show C++ Classes
  - ✅ Show Plugin Content
  - Set "Thumbnail Size" to medium

- **Level Editor → Play:**
  - Set "Play Mode" to "Selected Viewport"
  - ✅ Enable "Play in Editor" keyboard shortcut (Alt+P)

**Edit → Project Settings:**

- **Engine → Input:**
  - Default Input Class: `EnhancedPlayerInput`
  - Default Input Component Class: `EnhancedInputComponent`

- **Project → Maps & Modes:**
  - Editor Startup Map: Your test level
  - Game Default Map: Main menu level
  - Default GameMode: `BP_SpaceGameMode` (once created)

---

## Editor Interface Overview

### Essential Windows and Panels

#### Content Browser
**Location:** Bottom panel (default)  
**Purpose:** Asset management and organization

**Key Features:**
- View all game content (Blueprints, Data Assets, Materials, etc.)
- Create new assets with right-click menu
- Search and filter assets
- Organize content into folders

**Important Folders:**
```
Content/
├── Blueprints/      # Blueprint classes
├── DataAssets/      # Data Asset configurations
├── Maps/            # Game levels
├── Materials/       # Materials and material instances
├── UI/              # Widget Blueprints
└── Input/           # Input Actions and Mapping Contexts
```

#### Details Panel
**Location:** Right side (default)  
**Purpose:** Property editing for selected objects

**Usage:**
- Edit properties of selected actors in viewport
- Configure Blueprint variables and components
- Set Data Asset properties
- Adjust material parameters

#### Viewport
**Location:** Center panel  
**Purpose:** Visual level editing and scene composition

**Controls:**
- Right-click + WASD: Navigate
- Left-click: Select actors
- Middle-mouse drag: Pan
- Mouse wheel: Zoom
- F: Focus on selected object

#### Outliner
**Location:** Right side (default)  
**Purpose:** Hierarchical view of all actors in current level

**Features:**
- View all actors in scene
- Search for specific actors
- Group and organize actors
- Quick access to actor properties

#### Blueprint Editor
**Location:** Opens in new window  
**Purpose:** Visual scripting interface

**Key Tabs:**
- **Viewport:** 3D preview of Blueprint components
- **Construction Script:** Runs when object is placed in level
- **Event Graph:** Main logic and gameplay events
- **My Blueprint:** Variables, functions, and components list
- **Class Defaults:** Default property values

---

## Data Asset Creation

Data Assets are the **primary way** to configure game content without C++ code. All major systems use Data Assets.

### General Data Asset Workflow

1. **Create the Data Asset:**
   - Content Browser → Right-click
   - Miscellaneous → Data Asset
   - Select the appropriate Data Asset class

2. **Name it properly:**
   - Follow naming convention: `DA_[Type]_[Name]`
   - Example: `DA_Ship_PathfinderScout`

3. **Configure properties:**
   - Open the Data Asset
   - Fill in all required fields
   - Use tooltips for guidance

4. **Save and test:**
   - Save the asset
   - Reference it in Blueprints
   - Test in-game

### Spaceship Data Assets

**Location:** `Content/DataAssets/Ships/`  
**Class:** `SpaceshipDataAsset`  
**Purpose:** Define complete ship specifications

**Step-by-Step:**

1. **Create:** Right-click → Data Asset → `SpaceshipDataAsset`
2. **Name:** `DA_Ship_YourShipName`
3. **Configure sections:**

```
Basic Info:
  Ship Name: "Your Ship Display Name"
  Ship ID: "unique_ship_id"
  Ship Class: "Scout" / "Fighter" / "Trader" / etc.
  Description: Brief ship description
  Manufacturer: Company or faction name

Core Stats:
  Hull Strength: 100-10000 (durability)
  Cargo Capacity: 0-1000 (tons)
  Crew Required: 1-100 (personnel count)
  Modular Points: 0-20 (customization slots)

Combat Stats:
  Armor Rating: 0-1000 (damage reduction)
  Shield Strength: 0-5000 (shield capacity)
  Shield Regen Rate: 0-100 (per second)
  Weapon Slots: 0-20 (number of hardpoints)
  Point Defense Systems: 0-10 (anti-missile)

Mobility Stats:
  Max Speed: 100-2000 (m/s)
  Acceleration: 10-200 (m/s²)
  Maneuverability Rating: 1-10 (turning)
  Jump Range: 0-100 (light-years)
  Fuel Capacity: 0-10000 (units)

Utility Stats:
  Sensor Range: 0-100 (km)
  Stealth Rating: 0-10 (detection avoidance)
  Repair Rate: 0-100 (per minute)
  Science Labs: 0-5 (research capability)
  Medical Facilities: 0-5 (crew health)

Operational Stats:
  Power Output: 0-10000 (watts)
  Life Support Capacity: 0-1000 (days)
  Maintenance Cost: 0-10000 (credits/day)
  Hangars: 0-10 (small craft storage)
  Drone Capacity: 0-50 (drones)
  AI Core Level: 0-10 (automation)

Advanced Stats:
  Diplomacy Bonus: 0-10 (negotiation)
  Boarding Capacity: 0-100 (marines)
  Customization Slots: 0-20 (upgrades)
  Electronic Warfare: 0-10 (hacking)
  Mining Capability: 0-10 (resource extraction)

Lore/Flavor:
  Year Built: YYYY
  Rarity Tier: Common/Uncommon/Rare/Epic/Legendary
  Primary Color: RGB color picker
  Secondary Color: RGB color picker
  Lore Notes: Rich text description
```

4. **Save** (Ctrl+S)

**Templates:** Use YAML files in `Assets/SpaceshipTemplates/` as reference

### Faction Data Assets

**Location:** `Content/DataAssets/Factions/`  
**Class:** `FactionDataAsset`  
**Purpose:** Define political organizations

**Step-by-Step:**

1. **Create:** Right-click → Data Asset → `FactionDataAsset`
2. **Name:** `DA_Faction_YourFactionName`
3. **Configure:**

```
Basic Info:
  Faction Name: "Display Name"
  Faction ID: "unique_faction_id"
  Description: Faction background and goals
  Primary Color: Faction color 1
  Secondary Color: Faction color 2
  Emblem/Logo: Texture reference

Attributes:
  Technology Level: 1-10
  Military Strength: 1-10
  Economic Power: 1-10
  Political Influence: 1-10

Traits:
  Add traits: Click "+" button
  For each trait:
    - Trait ID: "trait_name"
    - Trait Name: "Display Name"
    - Description: What it does
    - Category: Military/Economic/Scientific/Diplomatic/Special
    - Modifier Type: Flat/Percentage
    - Modifier Value: Numerical value

Relationships:
  Add relationships: Click "+" button
  For each relationship:
    - Other Faction: Reference to another FactionDataAsset
    - Relationship Value: -100 (hostile) to +100 (allied)
    - Is Allied: Boolean
    - Is At War: Boolean
    - Trade Modifier: -100% to +100%
```

4. **Save** (Ctrl+S)

**Templates:** Use examples in `Assets/FactionSetupGuide.md`

---

## Blueprint Implementation

Blueprints are the visual scripting system in Unreal Engine. All game logic can be implemented without writing C++ code.

### Creating Blueprint Classes

#### Player Ship Blueprint

**Purpose:** Main player-controlled spaceship

**Creation Steps:**

1. **Create Blueprint:**
   - Content Browser → `Content/Blueprints/Ships/`
   - Right-click → Blueprint Class
   - Select parent class: `Spaceship` (C++ class)
   - Name: `BP_PlayerShip`

2. **Configure Components (Viewport tab):**

```
Components Hierarchy:
├─ Root (inherited)
├─ ShipMesh (Static Mesh Component)
│   └─ Mesh: Choose ship model or placeholder cube
│   └─ Scale: (2.0, 1.0, 0.5) for basic ship shape
│   └─ Rotation: (0, -90, 0) to point forward
│   └─ Collision: BlockAll
│
├─ CameraBoom (Spring Arm Component)
│   └─ Socket Offset: (0, 0, 100) above ship
│   └─ Target Arm Length: 800
│   └─ Use Pawn Control Rotation: true
│   └─ Parent: ShipMesh
│   │
│   └─ FollowCamera (Camera Component)
│       └─ Field of View: 90
│       └─ Parent: CameraBoom
│
└─ ProjectileSpawn (Arrow Component)
    └─ Location: (200, 0, 0) front of ship
    └─ Used for weapon spawn point
```

3. **Add Variables (My Blueprint panel):**

```
Movement:
  ForwardSpeed (Float): 500.0
  StrafeSpeed (Float): 300.0
  BoostMultiplier (Float): 2.0
  bIsBoosting (Boolean): false

Rotation:
  YawRate (Float): 50.0
  PitchRate (Float): 40.0
  RollRate (Float): 30.0

Combat:
  MaxHealth (Float): 1000.0
  CurrentHealth (Float): 1000.0
  MaxShield (Float): 500.0
  CurrentShield (Float): 500.0
  ShieldRegenRate (Float): 25.0

References:
  ShipDataAsset (SpaceshipDataAsset): Object Reference
  HUDWidget (WBP_SpaceshipHUD): Object Reference
```

4. **Compile and Save** (Ctrl+S)

#### Game Mode Blueprint

**Purpose:** Manages game rules, player spawning, and session state

1. **Create:**
   - `Content/Blueprints/GameModes/`
   - Parent class: `AdastreaGameMode` (C++ class)
   - Name: `BP_SpaceGameMode`

2. **Class Defaults:**
```
Classes:
  Default Pawn Class: BP_PlayerShip
  Player Controller Class: BP_SpaceshipController
  HUD Class: None (using widgets)
```

3. **Compile and Save**

---

## Widget and UI Creation

UMG (Unreal Motion Graphics) is used to create all UI elements.

### Main HUD Widget

**Purpose:** In-game heads-up display showing ship status

**Creation Steps:**

1. **Create Widget:**
   - `Content/UI/HUD/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_SpaceshipHUD`

2. **Designer Tab - Widget Hierarchy:**

```
Canvas Panel (Root)
  ├─ Overlay_Top
  │   ├─ Text_ShipName
  │   │   └─ Position: (20, 20)
  │   │   └─ Font Size: 18
  │   │
  │   └─ Text_Speed
  │       └─ Anchors: Top-Right
  │       └─ Font Size: 16
  │
  ├─ Overlay_Center
  │   └─ Image_Crosshair
  │       └─ Anchors: Center
  │       └─ Size: 32x32
  │
  └─ Overlay_Bottom
      ├─ ProgressBar_Health
      │   └─ Size: (300, 20)
      │   └─ Fill Color: Red
      │
      └─ ProgressBar_Shield
          └─ Size: (300, 20)
          └─ Fill Color: Blue
```

3. **Compile and Save**

---

## Input System Setup

Adastrea uses **Enhanced Input** system for flexible input mapping.

### Creating Input Actions

**Location:** `Content/Input/Actions/`

1. **Create Input Action:**
   - Right-click → Input → Input Action
   - Name: `IA_Move`
   - Value Type: `Axis2D` (Vector2D)

2. **Create more actions:**
   - `IA_Look` (Axis2D)
   - `IA_Boost` (Digital Bool)
   - `IA_Fire_Primary` (Digital Bool)
   - `IA_Interact` (Digital Bool)

### Creating Input Mapping Context

**Location:** `Content/Input/`

1. **Create:**
   - Right-click → Input → Input Mapping Context
   - Name: `IMC_Spaceship`

2. **Add mappings:**
   - Map WASD to IA_Move
   - Map Mouse to IA_Look
   - Map Shift to IA_Boost
   - Map Left Mouse Button to IA_Fire_Primary

---

## Material and Visual Effects

### Creating Master Materials

1. **Create Material:**
   - `Content/Materials/Master/`
   - Right-click → Material
   - Name: `M_ShipHull_Master`

2. **Add Parameters:**
   - Scalar Parameter: "Metallic" (0.9)
   - Scalar Parameter: "Roughness" (0.4)
   - Vector Parameter: "BaseColor"

3. **Create Material Instance:**
   - Right-click Master Material
   - Create Material Instance
   - Name: `MI_ShipHull_Blue`

---

## Level Design and Scene Setup

### Creating a Test Level

1. **New Level:**
   - File → New Level → Empty Level
   - Save as: `Content/Maps/TestLevel`

2. **Add Essential Actors:**
   - Directional Light (Sun)
   - Sky Light
   - Sky Atmosphere
   - Player Start

3. **Configure Lighting and Post Process**

---

## Station Editor System

### Overview

The Station Editor allows players to build and customize space stations in-game using a modular system.

**Full Documentation:** See `Content/Blueprints/STATION_EDITOR_README.md`

---

## Testing and Debugging

### In-Editor Play Testing

- Click **Play** button (Alt+P)
- **F8** - Eject from player
- **` (Backtick)** - Open console
- **F11** - Toggle fullscreen

### Blueprint Debugging

- Right-click node → Add Breakpoint
- Use Print String for quick debugging
- Watch variables during play

---

## Editor Utilities and Tools

### Content Browser Tools

- **Asset Actions → Audit:** Check asset references
- **Reference Viewer:** See dependencies
- **Collections:** Group related assets

### Python Scripting in Editor

- **Tools → Python → Execute Python Script**
- Use `AdastreaAssetValidator.py`
- Use `ScenePopulator.py`

---

## Common Workflows

### Complete Ship Creation Workflow

1. Create Data Asset with stats
2. Create Blueprint extending Spaceship
3. Add components and visuals
4. Reference Data Asset
5. Implement controls
6. Create HUD
7. Test

### Complete Faction Creation Workflow

1. Create Faction Data Asset
2. Configure traits and relationships
3. Create associated assets (ships, stations)
4. Test integration

---

## Troubleshooting

### Common Issues

#### Blueprint Compilation Errors
- Check for red nodes
- Verify all connections
- Try "Refresh All Nodes"

#### Input Not Working
- Verify Mapping Context is added
- Check Input Actions configuration
- Ensure Player Controller is set

#### Widget Not Displaying
- Verify "Add to Viewport" is called
- Check Z-order
- Verify anchors and position

---

## Additional Resources

### Essential Documentation Files

- **[BLUEPRINT_CREATION_GUIDE.md](BLUEPRINT_CREATION_GUIDE.md)** - Blueprint tutorials
- **[CONTENT_CREATION_QUICKSTART.md](CONTENT_CREATION_QUICKSTART.md)** - Quick start
- **[Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)** - Designer onboarding
- **[SCENE_POPULATION_GUIDE.md](SCENE_POPULATION_GUIDE.md)** - Scene setup

### System-Specific Guides

- **Spaceships:** `Assets/SpaceshipDataAssetGuide.md`
- **Factions:** `Assets/BlueprintFactionAssetGuide.md`
- **Personnel:** `Assets/PersonnelSystemGuide.md`
- **Trading:** `Assets/TradingSystemGuide.md`
- **AI:** `Assets/AIDesignerWorkflow.md`
- **Way System:** `Assets/WaySystemGuide.md`

---

## Conclusion

This guide covers the essential editor workflows for implementing game systems in Adastrea. Remember:

- **Data Assets first** - Configure game content without code
- **Blueprint visual scripting** - Implement gameplay logic visually
- **UMG for UI** - Design all interfaces in the editor
- **Enhanced Input** - Flexible input mapping
- **Test frequently** - Use in-editor play testing

For more detailed information on specific systems, refer to the individual guides in the `Assets/` directory.

**Happy developing!** 🚀✨

---

**Need Help?**
- Check the troubleshooting section
- Review system-specific guides
- Ask in GitHub Discussions
- Report issues on GitHub

**Contributing:**
If you find errors or have suggestions for this guide, please submit a pull request or open an issue.
