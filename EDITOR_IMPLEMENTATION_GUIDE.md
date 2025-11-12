# Adastrea - Unreal Engine Editor Implementation Guide

**Version:** 1.1  
**Last Updated:** 2025-11-12  
**Engine Version:** Unreal Engine 5.6

> **Important Note:** This guide now categorizes tasks by implementation method:
> - 🧑 **Unreal Editor User**: Tasks requiring manual interaction in the editor
> - 🤖 **Remote Control API**: Tasks that can be automated via Unreal's Remote Control API
> - 🔀 **Hybrid**: Tasks that benefit from both approaches

## Table of Contents

1. [Overview](#overview)
2. [Remote Control API Introduction](#remote-control-api-introduction)
3. [Prerequisites and Setup](#prerequisites-and-setup)
4. [Editor Interface Overview](#editor-interface-overview)
5. [Data Asset Creation](#data-asset-creation)
6. [Blueprint Implementation](#blueprint-implementation)
7. [Widget and UI Creation](#widget-and-ui-creation)
8. [Input System Setup](#input-system-setup)
9. [Material and Visual Effects](#material-and-visual-effects)
10. [Level Design and Scene Setup](#level-design-and-scene-setup)
11. [Station Editor System](#station-editor-system)
12. [Testing and Debugging](#testing-and-debugging)
13. [Editor Utilities and Tools](#editor-utilities-and-tools)
14. [Common Workflows](#common-workflows)
15. [Troubleshooting](#troubleshooting)

---

## Overview

This guide focuses on **editor-specific implementation** for Adastrea, covering tasks that can be done within the Unreal Engine editor. This guide now distinguishes between tasks that require manual editor interaction versus those that can be automated using the **Unreal Engine Remote Control API**.

### Task Categories

#### 🧑 **Unreal Editor User Tasks**
Tasks that require human creativity, decision-making, or visual design:
- Creating new Blueprint classes and logic
- Visual level design and artistic composition
- UI/UX design and widget layout
- Material artistic design and shader graph creation
- Initial project setup and configuration
- Manual testing and debugging
- Design decisions and content creation

#### 🤖 **Remote Control API Tasks**
Tasks that can be automated through HTTP/WebSocket API:
- Modifying existing Data Asset properties
- Triggering Blueprint functions and events
- Batch updating material parameters
- Spawning and manipulating actors in levels
- Automating repetitive property changes
- Remote testing and validation
- Integration with external tools and pipelines

#### 🔀 **Hybrid Tasks**
Tasks that benefit from both approaches:
- Data Asset creation (manual design, automated population)
- Blueprint implementation (manual logic, automated testing)
- Widget creation (manual design, data-driven content)
- Scene setup (manual composition, procedural population)

### What This Guide Covers

✅ **Blueprint visual scripting** for all game systems (🧑)  
✅ **Data Asset configuration** without C++ knowledge (🔀)  
✅ **UMG Widget creation** for UI/HUD (🔀)  
✅ **Enhanced Input system** setup (🧑/🤖)  
✅ **Material editor** workflows (🔀)  
✅ **Level design** and scene population (🔀)  
✅ **In-editor testing** and debugging (🔀)  
✅ **Editor customizations** and utilities (🧑)  
✅ **Remote Control API** integration (🤖)

### What This Guide Doesn't Cover

❌ C++ code implementation (see [ARCHITECTURE.md](ARCHITECTURE.md))  
❌ External scripting (see [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md))  
❌ Version control workflows (see [CONTRIBUTING.md](CONTRIBUTING.md))  
❌ Build and compilation (see [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md))

---

## Remote Control API Introduction

### What is the Remote Control API?

The **Unreal Engine Remote Control API** is a powerful system that allows external applications, scripts, and tools to interact with the Unreal Editor or running games through HTTP and WebSocket connections. It provides programmatic access to properties, functions, and events that are exposed to Blueprints and Python.

### Key Capabilities

**Property Manipulation** 🤖
- Read and write any Blueprint-exposed property
- Batch update multiple properties simultaneously
- Monitor property changes in real-time via WebSockets

**Function Execution** 🤖
- Call any BlueprintCallable function remotely
- Execute Editor Utility Widget functions
- Trigger gameplay events and animations

**Asset Management** 🔀
- Create and modify Data Assets programmatically
- Batch process assets with Editor Utility Widgets
- Import/export asset data via Python integration

**Scene Automation** 🤖
- Spawn and manipulate actors remotely
- Adjust lighting, materials, and effects in real-time
- Automate level population and scene setup

**Live Production** 🤖
- Real-time parameter control for virtual production
- Integration with hardware controllers (Stream Deck, MIDI)
- Web-based control panels for live events

### When to Use Remote Control API

**✅ Use Remote Control API for:**
- Automating repetitive tasks (batch property updates)
- Integration with CI/CD pipelines
- External tool integration (web dashboards, mobile apps)
- Live production control (broadcasts, virtual events)
- Automated testing and validation
- Data-driven content workflows
- Remote collaboration tools

**❌ Don't Use Remote Control API for:**
- Initial creative design work
- Complex Blueprint logic creation
- Visual artistic composition
- New system architecture design
- Learning the editor interface
- One-time manual configuration

### Setting Up Remote Control API

#### 1. Enable the Plugin 🧑

In Unreal Editor:
1. **Edit → Plugins**
2. Search for "Remote Control"
3. Check **Remote Control Web Interface**
4. Check **Remote Control API**
5. Restart the editor

#### 2. Create a Remote Control Preset 🧑

1. **Window → Remote Control → Remote Control Panel**
2. Click **+ Preset** to create new preset
3. Name it (e.g., "Adastrea_Automation")
4. Drag properties, actors, or functions into the preset
5. Save the preset

#### 3. Start the Web Server 🧑/🤖

**Manual Start:**
- Click **Start Server** in Remote Control Panel
- Default: `http://localhost:30010`

**Auto-Start on Launch:**
- Edit → Project Settings → Plugins → Remote Control Web Interface
- Enable **Start Server on Launch**

#### 4. Test Connection 🤖

```bash
# Health check
curl http://localhost:30010/remote/health

# List presets
curl http://localhost:30010/remote/presets

# Get preset details
curl http://localhost:30010/remote/preset/[preset-name]
```

### Remote Control API Examples

#### Example 1: Update Data Asset Property 🤖

```bash
# Update ship speed via HTTP POST
curl -X PUT http://localhost:30010/remote/object/property \
  -H "Content-Type: application/json" \
  -d '{
    "objectPath": "/Game/DataAssets/Ships/DA_Ship_Fighter.DA_Ship_Fighter",
    "propertyName": "MaxSpeed",
    "propertyValue": 1500
  }'
```

#### Example 2: Call Blueprint Function 🤖

```bash
# Trigger a Blueprint event
curl -X PUT http://localhost:30010/remote/preset/Adastrea_Automation/function/SpawnTestShip \
  -H "Content-Type: application/json" \
  -d '{
    "parameters": {
      "Location": {"X": 0, "Y": 0, "Z": 500},
      "ShipType": "Fighter"
    }
  }'
```

#### Example 3: Batch Update Materials 🤖

```python
import requests

# Batch update multiple material parameters
base_url = "http://localhost:30010/remote/batch"
materials = ["M_ShipHull_01", "M_ShipHull_02", "M_ShipHull_03"]

batch_requests = [
    {
        "RequestId": i,
        "URL": "/remote/object/property",
        "Verb": "PUT",
        "Body": {
            "objectPath": f"/Game/Materials/{mat}.{mat}",
            "propertyName": "BaseColor",
            "propertyValue": {"R": 0.2, "G": 0.5, "B": 0.8}
        }
    }
    for i, mat in enumerate(materials)
]

response = requests.put(base_url, json={"Requests": batch_requests})
print(response.json())
```

### Security Considerations ⚠️

**Important**: The Remote Control API should only be used in secure environments:
- Run on **localhost** or **secure LAN/VPN**
- Never expose to public internet
- Use for development, testing, and controlled production only
- Consider authentication in production environments

### Further Resources

- [Official Remote Control Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-for-unreal-engine)
- [Remote Control API HTTP Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-api-http-reference-for-unreal-engine)
- [Remote Control Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-quick-start-for-unreal-engine)

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

### General Data Asset Workflow (🔀 Hybrid)

#### Initial Creation (🧑 Manual - Editor User)

1. **Create the Data Asset:**
   - Content Browser → Right-click
   - Miscellaneous → Data Asset
   - Select the appropriate Data Asset class

2. **Name it properly:**
   - Follow naming convention: `DA_[Type]_[Name]`
   - Example: `DA_Ship_PathfinderScout`

3. **Configure initial structure:**
   - Open the Data Asset
   - Set up basic structure and categories
   - Define relationships and references

#### Property Configuration (🤖 Can Be Automated)

Once created, properties can be modified via Remote Control API:

**Manual Configuration (🧑):**
- Open the Data Asset in editor
- Fill in all required fields
- Use tooltips for guidance
- Save the asset

**Automated Configuration (🤖 Remote Control API):**
```python
# Example: Update Data Asset properties via API
import requests

def update_data_asset(asset_path, properties):
    url = "http://localhost:30010/remote/object/property"
    
    for prop_name, prop_value in properties.items():
        payload = {
            "objectPath": asset_path,
            "propertyName": prop_name,
            "propertyValue": prop_value
        }
        response = requests.put(url, json=payload)
        print(f"Updated {prop_name}: {response.status_code}")

# Update ship stats
update_data_asset(
    "/Game/DataAssets/Ships/DA_Ship_Fighter.DA_Ship_Fighter",
    {
        "MaxSpeed": 1500,
        "HullStrength": 5000,
        "ShieldStrength": 2500,
        "ArmorRating": 300
    }
)
```

**Batch Creation (🤖 Python + Editor Utility Widgets):**
```python
# Use Unreal Python to create multiple Data Assets
import unreal

def create_ship_data_assets(ship_configs):
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    for config in ship_configs:
        # Create new Data Asset
        asset_path = f"/Game/DataAssets/Ships/{config['name']}"
        asset = asset_tools.create_asset(
            config['name'],
            "/Game/DataAssets/Ships",
            unreal.SpaceshipDataAsset,
            unreal.DataAssetFactory()
        )
        
        # Set properties
        if asset:
            asset.set_editor_property("ship_name", config['display_name'])
            asset.set_editor_property("max_speed", config['speed'])
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
```

#### Testing and Validation (🔀 Hybrid)

**Manual Testing (🧑):**
- Reference asset in Blueprints
- Test in-game with Play In Editor (PIE)
- Visual verification

**Automated Testing (🤖 Remote Control API):**
- Query asset properties via API
- Validate against schema
- Run automated test suites

### Spaceship Data Assets (🔀 Hybrid)

**Location:** `Content/DataAssets/Ships/`  
**Class:** `SpaceshipDataAsset`  
**Purpose:** Define complete ship specifications

#### Manual Creation Process (🧑 Editor User)

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

#### Automated Property Updates (🤖 Remote Control API)

**Via HTTP API:**
```bash
# Update multiple ship properties at once
curl -X PUT http://localhost:30010/remote/batch \
  -H "Content-Type: application/json" \
  -d '{
    "Requests": [
      {
        "RequestId": 1,
        "URL": "/remote/object/property",
        "Verb": "PUT",
        "Body": {
          "objectPath": "/Game/DataAssets/Ships/DA_Ship_Fighter.DA_Ship_Fighter",
          "propertyName": "MaxSpeed",
          "propertyValue": 1800
        }
      },
      {
        "RequestId": 2,
        "URL": "/remote/object/property",
        "Verb": "PUT",
        "Body": {
          "objectPath": "/Game/DataAssets/Ships/DA_Ship_Fighter.DA_Ship_Fighter",
          "propertyName": "HullStrength",
          "propertyValue": 6000
        }
      }
    ]
  }'
```

**Via Python Script:**
```python
# Batch import ship data from YAML
import unreal
import yaml

def import_ship_from_yaml(yaml_path, asset_name):
    with open(yaml_path, 'r') as f:
        ship_data = yaml.safe_load(f)
    
    # Load existing asset or create new
    asset_path = f"/Game/DataAssets/Ships/{asset_name}"
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    
    if asset:
        # Update properties from YAML
        for section, properties in ship_data.items():
            for prop, value in properties.items():
                asset.set_editor_property(prop, value)
        
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        print(f"Updated {asset_name} from {yaml_path}")
```

**Use Cases for Automation:**
- Balancing passes (update stats across all ships)
- Importing from spreadsheets or external databases
- A/B testing different configurations
- Procedural ship generation
- CI/CD integration for content validation

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

### Creating Blueprint Classes (🧑 Editor User Required)

**Note:** Blueprint **creation** and **logic design** require manual work in the editor. However, once created, Blueprint **function execution** and **property modification** can be automated via Remote Control API.

#### Blueprint Creation: Manual Only (🧑)
- Designing Blueprint node graphs
- Creating event graphs and functions
- Setting up component hierarchies
- Defining Blueprint logic flow
- Visual scripting and connections

#### Blueprint Automation: Remote Control API (🤖)
- Calling Blueprint functions remotely
- Modifying exposed properties
- Triggering Blueprint events
- Spawning Blueprint actors
- Testing Blueprint behavior

#### Player Ship Blueprint (🧑 Manual Creation)

**Purpose:** Main player-controlled spaceship

**Creation Steps (🧑 Editor User):**

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

#### Runtime Interaction (🤖 Remote Control API)

Once the Blueprint is created, you can interact with it remotely:

**Call Blueprint Functions:**
```bash
# Call a custom Blueprint function
curl -X PUT http://localhost:30010/remote/object/call \
  -H "Content-Type: application/json" \
  -d '{
    "objectPath": "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C",
    "functionName": "SetBoostEnabled",
    "parameters": {
      "bEnabled": true
    }
  }'
```

**Modify Blueprint Properties:**
```python
# Update ship properties at runtime
import requests

def update_ship_property(property_name, value):
    url = "http://localhost:30010/remote/object/property"
    payload = {
        "objectPath": "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C",
        "propertyName": property_name,
        "propertyValue": value,
        "generateTransaction": True
    }
    return requests.put(url, json=payload)

# Example: Modify ship stats during testing
update_ship_property("MaxHealth", 2000)
update_ship_property("ForwardSpeed", 750)
update_ship_property("bIsBoosting", False)
```

**Spawn Blueprint Actors:**
```bash
# Spawn a ship in the level via API
curl -X POST http://localhost:30010/remote/object/call \
  -H "Content-Type: application/json" \
  -d '{
    "objectPath": "/Script/Engine.GameplayStatics",
    "functionName": "BeginDeferredActorSpawnFromClass",
    "parameters": {
      "WorldContextObject": "/Game/Maps/TestLevel",
      "ActorClass": "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C",
      "SpawnTransform": {
        "Translation": {"X": 0, "Y": 0, "Z": 500},
        "Rotation": {"Pitch": 0, "Yaw": 0, "Roll": 0},
        "Scale3D": {"X": 1, "Y": 1, "Z": 1}
      }
    }
  }'
```

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

### Widget Creation Approaches (🔀 Hybrid)

#### Manual Design (🧑 Editor User)
Widget **layout**, **visual design**, and **hierarchy** require manual creation:
- Designing widget layouts and composition
- Creating visual hierarchy
- Setting up widget bindings
- Configuring animations and transitions
- Artistic styling and theming

#### Data-Driven Content (🤖 Can Be Automated)
Widget **content** and **properties** can be driven by data:
- Text content updates
- Image asset swapping
- Dynamic data binding
- Property value changes
- Programmatic visibility control

### Main HUD Widget (🔀 Hybrid)

**Purpose:** In-game heads-up display showing ship status

#### Initial Creation (🧑 Manual - Editor User)

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

#### Data-Driven Updates (🤖 Remote Control API)

Once widget is created, update content programmatically:

**Update Widget Properties:**
```python
# Update HUD elements via Remote Control API
import requests

def update_hud_element(widget_name, property_name, value):
    url = "http://localhost:30010/remote/object/property"
    payload = {
        "objectPath": f"/Game/UI/HUD/WBP_SpaceshipHUD.WBP_SpaceshipHUD_C:{widget_name}",
        "propertyName": property_name,
        "propertyValue": value
    }
    return requests.put(url, json=payload)

# Update ship status display
update_hud_element("Text_Speed", "Text", "Speed: 1500 m/s")
update_hud_element("ProgressBar_Health", "Percent", 0.75)
update_hud_element("ProgressBar_Shield", "Percent", 0.50)
```

**Data-Driven UMG Approach:**
```cpp
// In Blueprint or C++, bind widget properties to data
// Remote Control API can then update the underlying data

// Create data-driven UI binding
UPROPERTY(BlueprintReadWrite, Category="HUD")
float CurrentHealthPercent = 1.0f;

UPROPERTY(BlueprintReadWrite, Category="HUD")
float CurrentShieldPercent = 1.0f;

// Widget automatically updates when these properties change
// Remote Control API updates the properties, not the widget directly
```

**Benefits of Data-Driven Approach:**
- Widgets update automatically when data changes
- Single source of truth for game state
- Easier testing and automation
- Better separation of concerns
- Version control friendly (data in JSON/CSV, not in widgets)

---

## Input System Setup (🔀 Hybrid)

Adastrea uses **Enhanced Input** system for flexible input mapping.

### Creating Input Actions (🧑 Manual - Editor User)

**Location:** `Content/Input/Actions/`

**Initial Setup (🧑 Required):**

1. **Create Input Action:**
   - Right-click → Input → Input Action
   - Name: `IA_Move`
   - Value Type: `Axis2D` (Vector2D)

2. **Create more actions:**
   - `IA_Look` (Axis2D)
   - `IA_Boost` (Digital Bool)
   - `IA_Fire_Primary` (Digital Bool)
   - `IA_Interact` (Digital Bool)

### Creating Input Mapping Context (🧑 Manual)

**Location:** `Content/Input/`

1. **Create:**
   - Right-click → Input → Input Mapping Context
   - Name: `IMC_Spaceship`

2. **Add mappings:**
   - Map WASD to IA_Move
   - Map Mouse to IA_Look
   - Map Shift to IA_Boost
   - Map Left Mouse Button to IA_Fire_Primary

### Runtime Input Modification (🤖 Remote Control API)

Once input actions and mappings are created, you can modify them at runtime:

**Simulate Input Events:**
```python
# Trigger input actions programmatically for automated testing
import requests

def simulate_input_action(action_name, value):
    url = "http://localhost:30010/remote/object/call"
    payload = {
        "objectPath": "/Script/Engine.PlayerController",
        "functionName": "EnhancedInputAction",
        "parameters": {
            "ActionName": action_name,
            "ActionValue": value
        }
    }
    return requests.post(url, json=payload)

# Automated testing: simulate boost press
simulate_input_action("IA_Boost", True)

# Automated testing: simulate movement
simulate_input_action("IA_Move", {"X": 1.0, "Y": 0.0})
```

**Configure Input Mappings via Data Asset:**
```python
# Use InputConfigDataAsset for data-driven input configuration
# Remote Control API can update the Data Asset properties

def update_input_config(config_property, value):
    url = "http://localhost:30010/remote/object/property"
    payload = {
        "objectPath": "/Game/Input/DA_InputConfig.DA_InputConfig",
        "propertyName": config_property,
        "propertyValue": value
    }
    return requests.put(url, json=payload)

# Example: Adjust input sensitivity
update_input_config("LookSensitivity", 2.5)
update_input_config("MovementSpeed", 1.2)
```

**Use Cases for Input Automation:**
- Automated gameplay testing
- Input recording and playback
- Difficulty adjustment (sensitivity tuning)
- Accessibility feature testing
- CI/CD integration testing

---

## Material and Visual Effects (🔀 Hybrid)

### Creating Master Materials (🧑 Manual - Editor User)

**Initial Creation Requires Manual Work:**

1. **Create Material:**
   - `Content/Materials/Master/`
   - Right-click → Material
   - Name: `M_ShipHull_Master`

2. **Add Parameters (🧑 Manual Graph Design):**
   - Scalar Parameter: "Metallic" (0.9)
   - Scalar Parameter: "Roughness" (0.4)
   - Vector Parameter: "BaseColor"

3. **Create Material Instance:**
   - Right-click Master Material
   - Create Material Instance
   - Name: `MI_ShipHull_Blue`

### Material Parameter Automation (🤖 Remote Control API)

Once materials are created with parameters, values can be modified remotely:

**Update Material Instance Parameters:**
```python
# Modify material parameters at runtime
import requests

def update_material_parameter(material_path, param_name, param_value):
    url = "http://localhost:30010/remote/object/property"
    payload = {
        "objectPath": material_path,
        "propertyName": param_name,
        "propertyValue": param_value
    }
    return requests.put(url, json=payload)

# Change ship hull color
update_material_parameter(
    "/Game/Materials/Instances/MI_ShipHull_Blue.MI_ShipHull_Blue",
    "BaseColor",
    {"R": 0.1, "G": 0.3, "B": 0.8, "A": 1.0}
)

# Adjust metallic value
update_material_parameter(
    "/Game/Materials/Instances/MI_ShipHull_Blue.MI_ShipHull_Blue",
    "Metallic",
    0.95
)
```

**Batch Update Multiple Materials:**
```python
# Update materials across all ships for consistent look
import requests

def batch_update_ship_materials(materials, updates):
    url = "http://localhost:30010/remote/batch"
    
    requests_list = []
    request_id = 0
    
    for material in materials:
        for param_name, param_value in updates.items():
            requests_list.append({
                "RequestId": request_id,
                "URL": "/remote/object/property",
                "Verb": "PUT",
                "Body": {
                    "objectPath": material,
                    "propertyName": param_name,
                    "propertyValue": param_value
                }
            })
            request_id += 1
    
    return requests.put(url, json={"Requests": requests_list})

# Example: Update all faction ship colors
faction_materials = [
    "/Game/Materials/Instances/MI_ShipHull_Faction1.MI_ShipHull_Faction1",
    "/Game/Materials/Instances/MI_ShipHull_Faction2.MI_ShipHull_Faction2",
    "/Game/Materials/Instances/MI_ShipHull_Faction3.MI_ShipHull_Faction3"
]

batch_update_ship_materials(faction_materials, {
    "Roughness": 0.5,
    "Metallic": 0.9
})
```

**Live Material Editing for Virtual Production:**
```bash
# Real-time material adjustments during filming/recording
curl -X PUT http://localhost:30010/remote/object/property \
  -H "Content-Type: application/json" \
  -d '{
    "objectPath": "/Game/Materials/Instances/MI_StationLights.MI_StationLights",
    "propertyName": "EmissiveStrength",
    "propertyValue": 15.0
  }'
```

**Use Cases for Material Automation:**
- Dynamic faction color schemes
- Day/night cycles and lighting adjustments
- Damage states (scorched, damaged materials)
- Live production control (broadcast, cinematics)
- A/B testing visual styles
- Procedural material variation
- Performance optimization (LOD material switching)

---

## Level Design and Scene Setup (🔀 Hybrid)

### Creating a Test Level (🧑 Manual Initial Setup)

**Initial Level Creation (🧑 Editor User):**

1. **New Level:**
   - File → New Level → Empty Level
   - Save as: `Content/Maps/TestLevel`

2. **Add Essential Actors:**
   - Directional Light (Sun)
   - Sky Light
   - Sky Atmosphere
   - Player Start

3. **Configure Lighting and Post Process**

### Scene Population and Automation (🤖 Remote Control API)

Once a level exists, actors can be spawned and manipulated remotely:

**Spawn Actors Programmatically:**
```python
# Populate scene with actors via Remote Control API
import requests

def spawn_actor(actor_class, location, rotation=(0, 0, 0), scale=(1, 1, 1)):
    url = "http://localhost:30010/remote/object/call"
    payload = {
        "objectPath": "/Script/Engine.GameplayStatics",
        "functionName": "BeginDeferredActorSpawnFromClass",
        "parameters": {
            "WorldContextObject": "/Game/Maps/TestLevel",
            "ActorClass": actor_class,
            "SpawnTransform": {
                "Translation": {"X": location[0], "Y": location[1], "Z": location[2]},
                "Rotation": {"Pitch": rotation[0], "Yaw": rotation[1], "Roll": rotation[2]},
                "Scale3D": {"X": scale[0], "Y": scale[1], "Z": scale[2]}
            }
        }
    }
    return requests.post(url, json=payload)

# Example: Populate test scene with ships
ship_positions = [
    (0, 0, 500),
    (1000, 0, 500),
    (-1000, 0, 500),
    (0, 1000, 500)
]

for pos in ship_positions:
    spawn_actor(
        "/Game/Blueprints/Ships/BP_EnemyShip.BP_EnemyShip_C",
        pos
    )
```

**Modify Actor Properties:**
```python
# Adjust lighting and atmosphere settings
import requests

def update_actor_property(actor_path, property_name, value):
    url = "http://localhost:30010/remote/object/property"
    payload = {
        "objectPath": actor_path,
        "propertyName": property_name,
        "propertyValue": value
    }
    return requests.put(url, json=payload)

# Adjust directional light (sun)
update_actor_property(
    "/Game/Maps/TestLevel.TestLevel:PersistentLevel.DirectionalLight_0",
    "Intensity",
    8.0
)

update_actor_property(
    "/Game/Maps/TestLevel.TestLevel:PersistentLevel.DirectionalLight_0",
    "LightColor",
    {"R": 255, "G": 240, "B": 220}
)

# Adjust post process volume
update_actor_property(
    "/Game/Maps/TestLevel.TestLevel:PersistentLevel.PostProcessVolume_0",
    "Settings.BloomIntensity",
    0.8
)
```

**Procedural Scene Population:**
```python
# Use existing Python tools with Remote Control integration
import unreal
import random

def procedural_populate_asteroids(count, bounds):
    """Procedurally spawn asteroids in a region"""
    for i in range(count):
        x = random.uniform(-bounds, bounds)
        y = random.uniform(-bounds, bounds)
        z = random.uniform(-bounds/2, bounds/2)
        
        scale = random.uniform(0.5, 3.0)
        rotation_y = random.uniform(0, 360)
        
        spawn_actor(
            "/Game/Blueprints/Environment/BP_Asteroid.BP_Asteroid_C",
            (x, y, z),
            rotation=(0, rotation_y, 0),
            scale=(scale, scale, scale)
        )

# Populate test area with 50 asteroids
procedural_populate_asteroids(50, 5000)
```

**Integration with Existing Tools:**

Adastrea includes scene population scripts that can be extended with Remote Control API:
- `ScenePopulator.py` - Basic scene population
- `InteriorLayoutPopulator.py` - Station interior generation
- `ProceduralGenerators.py` - Procedural content generation

**Use Cases for Scene Automation:**
- Automated test scenario setup
- Procedural level generation
- Dynamic event spawning
- Performance testing (spawn many actors)
- Cinematics and cutscene setup
- Live production scene management
- A/B testing different layouts

---

## Station Editor System

### Overview

The Station Editor allows players to build and customize space stations in-game using a modular system.

**Full Documentation:** See `Content/Blueprints/STATION_EDITOR_README.md`

---

## Testing and Debugging (🔀 Hybrid)

### In-Editor Play Testing (🧑 Manual)

**Manual Testing (🧑 Editor User):**
- Click **Play** button (Alt+P)
- **F8** - Eject from player
- **` (Backtick)** - Open console
- **F11** - Toggle fullscreen

### Blueprint Debugging (🧑 Manual)

**Manual Debugging (🧑 Editor User):**
- Right-click node → Add Breakpoint
- Use Print String for quick debugging
- Watch variables during play

### Automated Testing (🤖 Remote Control API)

**Remote Test Execution:**
```python
# Automated test suite using Remote Control API
import requests
import time

class UnrealTestRunner:
    def __init__(self, base_url="http://localhost:30010"):
        self.base_url = base_url
    
    def start_play_session(self):
        """Start PIE (Play In Editor) session"""
        url = f"{self.base_url}/remote/object/call"
        payload = {
            "objectPath": "/Script/UnrealEd.EditorLevelLibrary",
            "functionName": "EditorPlaySimulate"
        }
        return requests.post(url, json=payload)
    
    def stop_play_session(self):
        """Stop PIE session"""
        url = f"{self.base_url}/remote/object/call"
        payload = {
            "objectPath": "/Script/UnrealEd.EditorLevelLibrary",
            "functionName": "EditorEndPlay"
        }
        return requests.post(url, json=payload)
    
    def get_property_value(self, object_path, property_name):
        """Read property value for validation"""
        url = f"{self.base_url}/remote/object/property"
        params = {
            "objectPath": object_path,
            "propertyName": property_name,
            "access": "READ_ACCESS"
        }
        return requests.get(url, params=params)
    
    def call_test_function(self, object_path, function_name, parameters=None):
        """Execute test function"""
        url = f"{self.base_url}/remote/object/call"
        payload = {
            "objectPath": object_path,
            "functionName": function_name,
            "parameters": parameters or {}
        }
        return requests.post(url, json=payload)

# Example test suite
def test_ship_movement():
    runner = UnrealTestRunner()
    
    # Start test session
    runner.start_play_session()
    time.sleep(2)  # Wait for level to load
    
    # Test: Set ship speed
    ship_path = "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C"
    response = runner.call_test_function(
        ship_path,
        "SetForwardSpeed",
        {"Speed": 1000.0}
    )
    
    # Validate: Check speed was set
    speed = runner.get_property_value(ship_path, "ForwardSpeed")
    assert speed.json()["PropertyValue"] == 1000.0, "Speed not set correctly"
    
    # Stop test session
    runner.stop_play_session()
    
    print("✓ Ship movement test passed")

# Run automated tests
test_ship_movement()
```

**Property Validation:**
```python
# Automated validation of Data Asset properties
def validate_ship_stats(ship_asset_path):
    """Validate ship stats meet game balance requirements"""
    runner = UnrealTestRunner()
    
    # Define validation rules
    rules = {
        "MaxSpeed": {"min": 100, "max": 2000},
        "HullStrength": {"min": 100, "max": 10000},
        "ArmorRating": {"min": 0, "max": 1000},
        "WeaponSlots": {"min": 0, "max": 20}
    }
    
    results = []
    for prop, limits in rules.items():
        response = runner.get_property_value(ship_asset_path, prop)
        value = response.json()["PropertyValue"]
        
        if limits["min"] <= value <= limits["max"]:
            results.append(f"✓ {prop}: {value} (valid)")
        else:
            results.append(f"✗ {prop}: {value} (out of range)")
    
    return results

# Validate all ships
ship_assets = [
    "/Game/DataAssets/Ships/DA_Ship_Fighter.DA_Ship_Fighter",
    "/Game/DataAssets/Ships/DA_Ship_Scout.DA_Ship_Scout",
    "/Game/DataAssets/Ships/DA_Ship_Trader.DA_Ship_Trader"
]

for ship in ship_assets:
    print(f"\nValidating {ship}:")
    for result in validate_ship_stats(ship):
        print(f"  {result}")
```

**Integration with CI/CD:**
```yaml
# Example GitHub Actions workflow
name: Unreal Automated Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - name: Checkout
        uses: actions/checkout@v2
      
      - name: Start Unreal Editor with Remote Control
        run: |
          Start-Process "UnrealEditor.exe" `
            -ArgumentList "${{ github.workspace }}/Adastrea.uproject" `
            -PassThru
      
      - name: Wait for editor startup
        run: Start-Sleep -Seconds 30
      
      - name: Run automated tests
        run: python tests/automated_tests.py
      
      - name: Generate test report
        run: python tests/generate_report.py
```

**Use Cases for Automated Testing:**
- Continuous integration testing
- Regression testing after changes
- Performance benchmarking
- Data validation (assets, configurations)
- Gameplay mechanics validation
- Balance testing across multiple configurations
- Smoke tests before builds

---

## Editor Utilities and Tools (🔀 Hybrid)

### Content Browser Tools (🧑 Manual)

**Built-in Editor Tools (🧑):**
- **Asset Actions → Audit:** Check asset references
- **Reference Viewer:** See dependencies
- **Collections:** Group related assets

### Python Scripting in Editor (🔀 Hybrid)

**Manual Execution (🧑):**
- **Tools → Python → Execute Python Script**
- Use `AdastreaAssetValidator.py`
- Use `ScenePopulator.py`

**Remote Execution (🤖 Remote Control API):**
```python
# Execute Python scripts remotely
import requests

def execute_editor_script(script_path):
    """Execute Python script in Unreal Editor"""
    url = "http://localhost:30010/remote/object/call"
    payload = {
        "objectPath": "/Script/PythonScriptPlugin.PythonScriptLibrary",
        "functionName": "ExecutePythonScript",
        "parameters": {
            "PythonScript": script_path
        }
    }
    return requests.post(url, json=payload)

# Execute asset validator remotely
execute_editor_script("/Game/Scripts/AdastreaAssetValidator.py")
```

### Editor Utility Widgets (🔀 Hybrid)

**Creation (🧑 Manual):**
- Create custom editor tools with UMG
- Design automation workflows
- Build batch processing interfaces

**Remote Invocation (🤖 API):**
```python
# Trigger Editor Utility Widget functions remotely
def run_editor_utility(widget_path, function_name, params=None):
    """Execute Editor Utility Widget function"""
    url = "http://localhost:30010/remote/object/call"
    payload = {
        "objectPath": widget_path,
        "functionName": function_name,
        "parameters": params or {}
    }
    return requests.post(url, json=payload)

# Example: Batch asset import via EUW
run_editor_utility(
    "/Game/EditorUtilities/EUW_BatchImporter.EUW_BatchImporter",
    "ImportFromYAML",
    {"SourceDirectory": "/Game/YAML/Ships"}
)
```

### Automated Asset Management (🤖 Remote Control API)

**Batch Asset Operations:**
```python
# Remote asset validation and reporting
import requests
from pathlib import Path

class RemoteAssetManager:
    def __init__(self, base_url="http://localhost:30010"):
        self.base_url = base_url
    
    def get_all_assets_of_type(self, asset_class):
        """Query all assets of a specific type"""
        url = f"{self.base_url}/remote/search"
        params = {
            "Query": f"Class={asset_class}",
            "Limit": 1000
        }
        return requests.get(url, params=params)
    
    def validate_asset(self, asset_path):
        """Validate individual asset"""
        url = f"{self.base_url}/remote/object/call"
        payload = {
            "objectPath": "/Script/AdastreaEditor.AssetValidator",
            "functionName": "ValidateAsset",
            "parameters": {"AssetPath": asset_path}
        }
        return requests.post(url, json=payload)
    
    def generate_report(self, asset_type):
        """Generate validation report for asset type"""
        assets = self.get_all_assets_of_type(asset_type)
        results = []
        
        for asset in assets.json()["Assets"]:
            validation = self.validate_asset(asset["Path"])
            results.append({
                "asset": asset["Path"],
                "valid": validation.json()["IsValid"],
                "errors": validation.json().get("Errors", [])
            })
        
        return results

# Example usage
manager = RemoteAssetManager()

# Validate all spaceship data assets
print("Validating Spaceship Data Assets...")
report = manager.generate_report("SpaceshipDataAsset")

for result in report:
    status = "✓" if result["valid"] else "✗"
    print(f"{status} {result['asset']}")
    if result["errors"]:
        for error in result["errors"]:
            print(f"  - {error}")
```

### Integration with Existing Adastrea Tools

**Combining Manual and Automated Workflows:**

1. **Asset Validation (🔀 Hybrid):**
   - `AdastreaAssetValidator.py` - Manual or automated execution
   - Remote Control API - Automated validation in CI/CD

2. **Scene Population (🔀 Hybrid):**
   - `ScenePopulator.py` - Initial setup manually
   - Remote Control API - Automated scene modifications

3. **Procedural Generation (🤖 Mostly Automated):**
   - `ProceduralGenerators.py` - Can be triggered remotely
   - Remote Control API - Parameter adjustment on the fly

4. **YAML Import (🔀 Hybrid):**
   - `YAMLtoDataAsset.py` - Initial setup manually
   - Remote Control API - Batch updates after import

5. **Testing Automation (🤖 Automated):**
   - `SmokeTest.py` - Automated with Remote Control
   - `ScreenshotTester.py` - Visual regression via API
   - `AutomationRunner.py` - Full CI/CD integration

**Example: Combined Workflow**
```python
# Complete automated content pipeline
import subprocess

# 1. Generate YAML from template
subprocess.run(["python", "TemplateGenerator.py", "--type", "ship", "--count", 10])

# 2. Import YAML to Data Assets (manual or automated)
subprocess.run(["python", "YAMLtoDataAsset.py", "--batch", "ships"])

# 3. Validate imported assets (Remote Control API)
manager = RemoteAssetManager()
validation_report = manager.generate_report("SpaceshipDataAsset")

# 4. Update properties based on balance rules (Remote Control API)
for asset in validation_report:
    if asset["valid"]:
        # Apply balance adjustments
        pass

# 5. Run automated tests
subprocess.run(["python", "SmokeTest.py", "--asset-type", "ships"])

print("✓ Content pipeline complete!")
```

---

## Common Workflows

### Complete Ship Creation Workflow (🔀 Hybrid)

#### Manual Design Phase (🧑 Editor User)
1. **Create Data Asset structure** (manual)
   - Define ship archetype and class
   - Set up basic properties structure
   
2. **Create Blueprint extending Spaceship** (manual)
   - Design component hierarchy
   - Set up visual mesh
   - Configure collision
   
3. **Implement controls and logic** (manual)
   - Create input handling
   - Design movement behavior
   - Add special abilities

4. **Create HUD** (manual)
   - Design widget layout
   - Set up data bindings
   - Style UI elements

#### Automated Population Phase (🤖 Remote Control API)
5. **Populate Data Asset properties** (automated)
   ```python
   # Import stats from balance spreadsheet
   import pandas as pd
   
   balance_data = pd.read_csv("ship_balance.csv")
   
   for _, ship in balance_data.iterrows():
       update_data_asset(
           f"/Game/DataAssets/Ships/{ship['AssetName']}",
           {
               "MaxSpeed": ship["Speed"],
               "HullStrength": ship["Hull"],
               "ShieldStrength": ship["Shield"],
               "WeaponSlots": ship["Weapons"]
           }
       )
   ```

6. **Batch test all variants** (automated)
   ```python
   # Automated testing of all ship configurations
   test_runner = UnrealTestRunner()
   
   ships = get_all_ship_assets()
   for ship in ships:
       test_ship_performance(ship)
       test_ship_balance(ship)
       validate_ship_data(ship)
   ```

7. **Generate documentation** (automated)
   ```python
   # Auto-generate ship documentation from Data Assets
   generate_ship_wiki(ships)
   ```

### Complete Faction Creation Workflow (🔀 Hybrid)

#### Manual Design Phase (🧑 Editor User)
1. **Create Faction Data Asset** (manual)
   - Design faction identity and theme
   - Define core traits
   - Set initial relationships

2. **Create associated assets** (manual)
   - Design faction-specific ships
   - Create faction stations
   - Design UI themes

#### Automated Configuration Phase (🤖 Remote Control API)
3. **Configure traits and relationships** (automated)
   ```python
   # Import faction data from game design documents
   import yaml
   
   with open("faction_config.yaml") as f:
       factions = yaml.safe_load(f)
   
   for faction in factions:
       asset_path = f"/Game/DataAssets/Factions/{faction['id']}"
       
       # Update faction properties
       update_data_asset(asset_path, {
           "FactionName": faction["name"],
           "TechnologyLevel": faction["tech"],
           "MilitaryStrength": faction["military"],
           "EconomicPower": faction["economy"]
       })
       
       # Configure relationships
       for relationship in faction["relationships"]:
           add_faction_relationship(
               asset_path,
               relationship["target"],
               relationship["value"]
           )
   ```

4. **Balance test factions** (automated)
   ```python
   # Validate faction balance
   for faction in get_all_factions():
       test_faction_economy(faction)
       test_faction_military(faction)
       validate_faction_relationships(faction)
   ```

5. **Generate faction content** (automated)
   ```python
   # Procedurally generate faction ships, stations, NPCs
   for faction in factions:
       generate_faction_fleet(faction, count=10)
       generate_faction_stations(faction, count=5)
       populate_faction_npcs(faction)
   ```

### Data-Driven Content Workflow (🤖 Mostly Automated)

**Ideal workflow for large-scale content:**

```python
# 1. Design in external tools (spreadsheet, database)
# Content/Data/ship_designs.csv

# 2. Import and create assets
def import_content_batch(csv_path, asset_type):
    data = pd.read_csv(csv_path)
    
    for _, row in data.iterrows():
        # Create or update Data Asset
        asset_path = f"/Game/DataAssets/{asset_type}/{row['ID']}"
        
        # Map CSV columns to asset properties
        properties = {col: row[col] for col in data.columns}
        
        update_or_create_asset(asset_path, properties)

# 3. Validate all content
validate_all_assets(asset_type)

# 4. Run automated tests
run_test_suite(asset_type)

# 5. Generate documentation
generate_wiki_pages(asset_type)

# 6. Deploy to build
trigger_build_pipeline()
```

**Benefits of Data-Driven Approach:**
- ✅ Designers work in familiar tools (Excel, Google Sheets)
- ✅ Version control friendly (CSV, YAML)
- ✅ Easy collaboration and review
- ✅ Automated validation catches errors early
- ✅ CI/CD integration for quality assurance
- ✅ Rapid iteration and balance adjustments
- ✅ Consistent documentation generation

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

This guide covers the essential editor workflows for implementing game systems in Adastrea, now enhanced with Remote Control API automation capabilities.

### Key Takeaways

#### 🧑 Manual Work (Editor User Required)

**Creative Design Tasks:**
- Initial Blueprint creation and logic design
- Visual level design and artistic composition
- UI/UX widget layout and design
- Material shader graph creation
- System architecture and design decisions
- Initial asset structure setup

**When Manual Work is Best:**
- First-time system design
- Creative and artistic decisions
- Learning the editor and systems
- Complex visual composition
- One-off configurations

#### 🤖 Automated Work (Remote Control API)

**Automation Opportunities:**
- Batch Data Asset property updates
- Material parameter adjustments
- Scene population and actor spawning
- Property validation and testing
- CI/CD integration
- Live production control
- Performance benchmarking

**When Automation is Best:**
- Repetitive tasks at scale
- Content pipeline integration
- Balance adjustments across many assets
- Automated testing and validation
- Live production scenarios
- Data-driven content workflows

#### 🔀 Hybrid Approach (Best of Both Worlds)

**Recommended Workflow:**
1. **Design manually** - Create structure, logic, and visual design
2. **Populate with data** - Use Remote Control API for property updates
3. **Automate testing** - Validate with automated test suites
4. **Iterate rapidly** - Adjust values via API, test immediately
5. **Document automatically** - Generate docs from asset data

### Implementation Strategy

**For New Features:**
1. 🧑 Design system architecture manually
2. 🧑 Create initial Blueprint/Data Asset templates
3. 🤖 Set up Remote Control preset for the system
4. 🤖 Create automation scripts for common operations
5. 🔀 Test with both manual and automated methods
6. 🤖 Integrate into CI/CD pipeline

**For Content Creation:**
1. 🧑 Design content templates manually
2. 🤖 Import bulk data from spreadsheets/databases
3. 🤖 Validate all content automatically
4. 🔀 Manually review and adjust as needed
5. 🤖 Run automated tests
6. 🤖 Generate documentation

**For Maintenance:**
1. 🤖 Use Remote Control API for balance adjustments
2. 🤖 Automated regression testing after changes
3. 🧑 Manual review of critical changes
4. 🤖 Batch updates for consistency
5. 🤖 Performance profiling and optimization

### Quick Reference Summary

| Task | Manual (🧑) | Automated (🤖) | Best Approach |
|------|-------------|----------------|---------------|
| Blueprint Creation | ✅ Required | ❌ Not possible | 🧑 Manual |
| Blueprint Function Calls | ❌ N/A | ✅ Possible | 🤖 Automated |
| Data Asset Creation | ✅ Required | ⚠️ Via Python | 🧑 Manual |
| Data Asset Properties | ✅ Possible | ✅ Recommended | 🔀 Hybrid |
| Widget Layout | ✅ Required | ❌ Not possible | 🧑 Manual |
| Widget Content | ✅ Possible | ✅ Recommended | 🔀 Hybrid |
| Material Graph | ✅ Required | ❌ Not possible | 🧑 Manual |
| Material Parameters | ✅ Possible | ✅ Recommended | 🔀 Hybrid |
| Level Creation | ✅ Required | ⚠️ Limited | 🧑 Manual |
| Actor Spawning | ✅ Possible | ✅ Recommended | 🤖 Automated |
| Input Action Creation | ✅ Required | ❌ Not possible | 🧑 Manual |
| Input Testing | ✅ Possible | ✅ Recommended | 🤖 Automated |
| Testing & Validation | ✅ Possible | ✅ Recommended | 🔀 Hybrid |

### Best Practices

**Design for Automation:**
- Expose properties you might need to change remotely
- Use Data Assets for all configurable content
- Create Remote Control presets early
- Document automation opportunities
- Build editor utilities for common tasks

**Maintain Manual Control:**
- Keep creative decisions manual
- Don't over-automate complex workflows
- Review automated changes periodically
- Maintain editor skills for troubleshooting
- Balance efficiency with understanding

**Integrate Smoothly:**
- Start with manual workflows
- Identify repetitive tasks
- Automate incrementally
- Test automation thoroughly
- Document both approaches

### Resources for Success

**Adastrea-Specific:**
- **Data Assets first** - Configure game content without code
- **Blueprint visual scripting** - Implement gameplay logic visually
- **UMG for UI** - Design all interfaces in the editor
- **Enhanced Input** - Flexible input mapping
- **Remote Control API** - Automate repetitive tasks
- **Test frequently** - Use both manual and automated testing

**Further Learning:**
- [Unreal Remote Control Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-control-for-unreal-engine)
- [Editor Utility Widgets Tutorial](https://dev.epicgames.com/documentation/en-us/unreal-engine/editor-utility-widgets-in-unreal-engine)
- [Python Scripting in Unreal](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api-reference-for-unreal-engine)

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
