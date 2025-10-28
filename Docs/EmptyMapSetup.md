# Empty Map Setup Guide - Adastrea

## Overview
This guide provides step-by-step instructions for creating and configuring an empty 200km x 200km space map in Unreal Engine 5.5+ for the Adastrea project. These maps serve as the foundation for procedural generation, manual content placement, and sector-based open world design.

---

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Creating a New Empty Map](#creating-a-new-empty-map)
3. [Adding Space Sector Markers](#adding-space-sector-markers)
4. [Configuring World Settings](#configuring-world-settings)
5. [Automatic Player Spaceship Spawning](#automatic-player-spaceship-spawning)
6. [Setting Up Lighting](#setting-up-lighting)
7. [Adding Background Elements](#adding-background-elements)
8. [Testing Your Map](#testing-your-map)
9. [Best Practices](#best-practices)
10. [Next Steps](#next-steps)

---

## Prerequisites

Before starting, ensure you have:
- Unreal Engine 5.5 or later installed
- Adastrea project compiled successfully
- Basic familiarity with Unreal Engine editor
- Understanding of level design concepts

---

## Creating a New Empty Map

### Step 1: Create the Level

1. **Open Unreal Engine** and load the Adastrea project
2. In the **Content Browser**, navigate to `Content/Maps/`
   - If the Maps folder doesn't exist, create it: Right-click → New Folder → "Maps"
3. Right-click in the Maps folder
4. Select **Level** from the context menu
5. Choose **Empty Level** (not "Default" or "VR")
6. Name your level: `Map_Sector_[YourSectorName]`
   - Examples: `Map_Sector_AlphaCentauri`, `Map_Sector_AsteroidBelt`

### Step 2: Initial Map Setup

1. **Open your new map** by double-clicking it
2. The viewport should show a completely empty void
3. Save the map immediately: **File → Save Current Level**

---

## Adding Space Sector Markers

### Step 1: Place the Sector Actor

1. In the **Place Actors** panel (Window → Place Actors if not visible):
   - Search for "SpaceSectorMap"
   - Or navigate to **All Classes** and find `ASpaceSectorMap`
2. **Drag and drop** the SpaceSectorMap actor into the viewport
3. The actor will appear with a billboard icon for easy identification

### Step 2: Position the Sector

1. **Select** the SpaceSectorMap actor in the outliner or viewport
2. In the **Details** panel, set the **Location**:
   - X: 0
   - Y: 0
   - Z: 0
   - This centers the sector at world origin for easier math
3. **Note**: The sector extends 100km (10,000,000 units) in all directions from its center

### Step 3: Configure Sector Properties

1. With the SpaceSectorMap actor selected, find the **Sector Info** category
2. Set the **Sector Name**:
   - Enter a descriptive name (e.g., "Alpha Centauri Core", "Trading Hub Nexus")
   - This helps identify the sector in multi-sector layouts
3. Set the **Description** (optional):
   - Add notes about planned content
   - Document special features or themes
   - Example: "Main trading hub with 3 stations and asteroid mining operations"

### Step 4: Verify Sector Bounds

1. **Enable visualization helpers** (optional):
   - In the viewport, click **Show → Volumes**
   - This helps visualize the 200km x 200km boundaries
2. **Test helper functions** in Blueprint:
   - Create a temporary Blueprint actor
   - Call `GetSectorBounds()` to verify boundaries
   - Call `GetRandomPositionInSector()` to test random placement

---

## Configuring World Settings

### Step 1: Access World Settings

1. Open the **World Settings** panel:
   - **Window → World Settings**
   - Or click **Settings → World Settings** in the toolbar

### Step 2: Set World Bounds

1. In World Settings, find **World** category
2. Set **Kill Z** to a very low value (e.g., -50,000,000)
   - Prevents actors from being destroyed if they fall
   - Important for space environments with no "ground"
3. Enable **Enable World Bounds Checks**: False
   - Allows objects to exist beyond default boundaries
   - Essential for large 200km sectors

### Step 3: Configure Physics

1. In the **Physics** category:
2. Set **Default Gravity Z** to 0.0
   - Space has no gravity by default
   - Can be overridden per-object or zone as needed
3. Consider setting **World Bounds Extent** to match sector size:
   - X: 10000000 (100km)
   - Y: 10000000 (100km)
   - Z: 10000000 (100km)

### Step 4: Game Mode Configuration

1. In the **Game Mode** category:
2. Set **GameMode Override** to `AAdastreaGameMode` (or your custom game mode)
3. Configure **Default Pawn Class** as needed for space flight

---

## Automatic Player Spaceship Spawning

**New Feature**: Adastrea now automatically spawns the player in a spaceship when a map with a SpaceSectorMap loads!

### Overview

When using `AAdastreaGameMode` (or a Blueprint derived from it), the game will automatically:
1. Find the SpaceSectorMap actor in your level
2. Spawn a player spaceship at the sector center or a random position
3. Possess the spaceship with the PlayerController

This eliminates the need for manual player start placement and ensures the player always begins inside the sector bounds.

### Configuration (C++ GameMode)

The auto-spawn feature is controlled by three properties in the GameMode:

**In World Settings → Game Mode Override → Selected GameMode Details:**

1. **Default Spaceship Class** (`TSubclassOf<ASpaceship>`)
   - Set this to your spaceship Blueprint class
   - Example: `BP_Spaceship_Starter` or `BP_Ship_Fighter`
   - **Required**: Auto-spawn will not work if this is not set

2. **Auto Spawn Player Ship** (`bool`)
   - Default: `true`
   - Toggle to enable/disable automatic spawning
   - Set to `false` if you want to manually control player spawn

3. **Spawn At Center** (`bool`)
   - Default: `false` (random position)
   - If `true`: Spawns at exact sector center
   - If `false`: Spawns at random position within sector bounds
   - Random positioning provides variety on each play session

### Setup Steps (C++)

1. **Create a Spaceship Blueprint**:
   - Create a Blueprint based on `ASpaceship` class
   - Configure the ship's mesh, components, and properties
   - Save as `BP_PlayerShip` (or your preferred name)

2. **Configure GameMode**:
   - In World Settings, set GameMode Override to `AAdastreaGameMode`
   - Expand the GameMode details in World Settings
   - Find the **Player Spawn** category
   - Set **Default Spaceship Class** to your spaceship Blueprint
   - Verify **Auto Spawn Player Ship** is checked (enabled)
   - Choose spawn location type with **Spawn At Center** checkbox

3. **Place SpaceSectorMap**:
   - Add a SpaceSectorMap actor to your level (see [Adding Space Sector Markers](#adding-space-sector-markers))
   - Position it at (0, 0, 0) for consistent spawning

4. **Test**:
   - Click Play in the editor
   - You should spawn inside your spaceship
   - Check the Output Log for spawn confirmation messages

### Blueprint Alternative (For Designers)

If you prefer to use Blueprint instead of the C++ GameMode, you can create your own spawn logic:

**Create a Blueprint based on AAdastreaGameMode:**

1. **Create GameMode Blueprint**:
   - Content Browser → Right-click → Blueprint Class
   - Parent Class: `AAdastreaGameMode`
   - Name: `BP_MyGameMode`

2. **Override BeginPlay Event**:
   ```
   Event BeginPlay
       ↓
   [Optional] Call Parent BeginPlay (if you want auto-spawn)
       ↓
   [Your custom logic here]
   ```

3. **Manual Spawn Logic** (if disabling auto-spawn):
   ```
   Event BeginPlay
       ↓
   Get All Actors of Class → ASpaceSectorMap
       ↓
   Get (index 0) → Cast to SpaceSectorMap
       ↓
   Get Sector Center (or Get Random Position In Sector)
       ↓
   Spawn Actor from Class → [Your Spaceship Blueprint]
       ↓  ↓
   Location   Rotation (0, 0, 0)
       ↓
   Get Player Controller (Index 0)
       ↓
   Possess → [Spawned Spaceship]
   ```

4. **Set in World Settings**:
   - World Settings → GameMode Override → `BP_MyGameMode`

### Logging and Debugging

The auto-spawn feature logs its actions to the Output Log with the `LogAdastrea` category:

- **Success**: `"AdastreaGameMode: Successfully spawned player spaceship at [location]"`
- **No Spaceship Class**: `"Cannot auto-spawn - DefaultSpaceshipClass is not set"`
- **No Sector Map**: `"No SpaceSectorMap found in level"`
- **Possession**: `"Player controller possessed spaceship"`

**Tip**: Enable verbose logging for troubleshooting:
```
Log LogAdastrea Verbose
```

### Common Issues

**Issue**: Player doesn't spawn in a spaceship
- **Solution 1**: Verify DefaultSpaceshipClass is set in GameMode details
- **Solution 2**: Check that SpaceSectorMap actor exists in the level
- **Solution 3**: Ensure Auto Spawn Player Ship is enabled (checked)

**Issue**: Player spawns at world origin (0,0,0) instead of in sector
- **Solution**: Verify SpaceSectorMap is properly placed in the level
- **Solution**: Check Output Log for error messages about sector map

**Issue**: Want to spawn at specific location, not random
- **Solution**: Enable "Spawn At Center" checkbox in GameMode
- **Solution**: Or override SpawnPlayerSpaceship() in Blueprint/C++ for custom logic

**Issue**: Want to disable auto-spawn entirely
- **Solution**: Uncheck "Auto Spawn Player Ship" in GameMode settings
- **Solution**: Implement your own spawn logic in Blueprint

### Advanced: Custom Spawn Logic

For advanced users who want full control over spawning:

**Override in C++**:
```cpp
void AMyCustomGameMode::SpawnPlayerSpaceship()
{
    // Your custom spawn logic here
    // You can call Super::SpawnPlayerSpaceship() to use default behavior
    // Or implement completely custom logic
}
```

**Override in Blueprint**:
1. Create Blueprint based on AAdastreaGameMode
2. Override `SpawnPlayerSpaceship` function
3. Add your custom logic
4. Optionally call Parent function for default behavior

---

## Setting Up Lighting

### Step 1: Add Directional Light (Sun)

1. In **Place Actors** panel, search for "Directional Light"
2. Drag it into the level
3. Configure the light:
   - **Intensity**: 3.0 - 5.0 (adjust for desired brightness)
   - **Light Color**: Slightly warm white (RGB: 255, 250, 245)
   - **Rotation**: Angle to simulate distant sun
   - **Cast Shadows**: True
   - **Mobility**: Stationary or Static

### Step 2: Add Skylight

1. Search for "Sky Light" in Place Actors
2. Add it to the level
3. Configure:
   - **Intensity**: 0.5 - 1.0
   - **Source Type**: SLS Captured Scene (or use HDRI)
   - **Mobility**: Stationary or Static
4. **Important**: Click "Recapture Scene" after adding objects

### Step 3: Add Exponential Height Fog (Optional)

1. For atmospheric effects or nebula-like fog:
2. Add **Exponential Height Fog** actor
3. Configure for space environment:
   - **Fog Density**: 0.001 - 0.01 (very low for space)
   - **Fog Height Falloff**: 0.0 (uniform density)
   - **Fog Inscattering Color**: Deep blue or purple for nebula

### Step 4: Post Process Volume

1. Add a **Post Process Volume** to control visual quality
2. Set **Infinite Extent (Unbound)**: True
3. Configure post-process settings:
   - **Bloom Intensity**: 0.5 - 1.0 (for star glow)
   - **Auto Exposure**: Adjust for space darkness
   - **Tone Mapper**: Filmic or ACES

---

## Adding Background Elements

### Step 1: Skybox / Space Background

1. **Option A: Use Sky Atmosphere**
   - Add Sky Atmosphere actor for procedural space/planet atmospheres
   - Configure for deep space (low density, dark colors)

2. **Option B: Static Skybox**
   - Create or import a space skybox texture (cube map or sphere)
   - Create Material for skybox using texture
   - Add Sky Sphere Blueprint and assign material

3. **Option C: Simple Solid Color**
   - Set viewport background to solid black
   - Simplest option for prototyping

### Step 2: Distant Stars

1. **Particle System** (optional):
   - Create particle system for distant stars
   - Use very small, bright particles
   - Scatter across skybox distance

2. **Mesh-Based Stars** (recommended for performance):
   - Create simple star field static mesh
   - Use unlit material with emissive texture
   - Place at skybox distance (e.g., 10,000,000 units from center)

### Step 3: Nebula or Background Features

1. Create volumetric clouds/nebula (optional):
   - Use Volumetric Cloud actor with custom materials
   - Set density very low for distant nebula effect
   - Adjust colors for visual interest

2. Add distant celestial objects:
   - Distant planets (as static meshes with materials)
   - Moons or other landmarks
   - Keep them far enough to maintain 200km sector scale

---

## Testing Your Map

### Step 1: Basic Navigation Test

1. Click **Play** button in toolbar (or Alt+P)
2. Verify the following:
   - Map loads without errors
   - Player spawns correctly
   - No visible artifacts or issues
   - Lighting appears as expected

### Step 2: Sector Bounds Test

1. Create a test Blueprint actor:
```blueprint
Event BeginPlay
    → Get Actor Location (Self)
    → IsPositionInSector (SpaceSectorMap reference)
    → Print String (result)
```

2. Place test actors at various positions:
   - Inside sector bounds
   - Outside sector bounds
   - At sector edges
3. Verify bounds detection works correctly

### Step 3: Random Placement Test

1. In Blueprint or code, test `GetRandomPositionInSector()`:
```cpp
// Example C++ test
FVector RandomPos = SectorMapActor->GetRandomPositionInSector();
UE_LOG(LogAdastrea, Log, TEXT("Random position: %s"), *RandomPos.ToString());
```

2. Spawn multiple test objects at random positions
3. Verify they appear within the 200km bounds
4. Check distribution appears uniform

### Step 4: Performance Check

1. Open **Stats** display: Press tilde (`~`) and type `stat fps`
2. Verify frame rate is stable (60+ FPS on target hardware)
3. Check for any performance warnings in the Output Log
4. Profile if needed: `stat unit`, `stat gpu`

---

## Best Practices

### Organization

- **Naming Convention**: Use consistent naming for sectors
  - Format: `Map_Sector_[Name]`
  - Examples: `Map_Sector_TradeHub`, `Map_Sector_Nebula07`
- **Folder Structure**: Keep related sectors in subfolders
  - `Content/Maps/CoreSectors/`
  - `Content/Maps/OuterRim/`
  - `Content/Maps/TestSectors/`

### Sector Layout

- **Center at Origin**: Place SpaceSectorMap at (0, 0, 0) for easier math
- **Multiple Sectors**: For connected sectors, offset by 200km (20,000,000 units)
  - Sector 1 at (0, 0, 0)
  - Sector 2 at (20000000, 0, 0) - directly east
  - Sector 3 at (0, 20000000, 0) - directly north
- **Coordinate System**: Use consistent orientation across all sectors

### Performance

- **LOD Systems**: Plan for Level of Detail switching at distance
- **Occlusion**: Use occlusion culling for large object clusters
- **Streaming**: For multiple connected sectors, use level streaming
- **Object Pooling**: Plan for reusable object pools for common actors

### Content Planning

- **Document Before Building**: Use sector Description field for planning
- **Density Guidelines**: Aim for 10-50 major objects per sector
  - Stations: 1-5 per sector
  - Large ships: 5-20 per sector
  - Small ships: 20-100 per sector
  - Asteroids/debris: 100-1000+ (use instancing)
- **Visual Variety**: Break up empty space with points of interest every 20-40km

---

## Next Steps

### Adding Content to Your Sector

1. **Stations**: Place SpaceStation actors at strategic locations
   - Use `GetRandomPositionInSector()` for initial placement
   - Manually adjust for better layout
   - See Content/Blueprints/STATION_EDITOR_README.md for station configuration

2. **Ships**: Add spaceship actors or AI-controlled vessels
   - Reference SpaceshipDataAsset system
   - Configure patrol routes within sector bounds
   - See Assets/SpaceshipTemplates/ for ship setup

3. **Celestial Objects**: Add planets, moons, asteroids
   - Use existing Planet actors
   - Create custom asteroid fields
   - Configure orbital mechanics if desired

4. **Trading Routes**: Set up economy and trading
   - Reference TradingSystemGuide.md
   - Create trade routes between stations
   - Configure supply and demand

### Procedural Generation Integration

1. **Blueprint Generation**:
   - Create Blueprint that references SpaceSectorMap
   - Use `GetRandomPositionInSector()` for placement
   - Implement density controls
   - Add variation rules

2. **C++ Generation** (advanced):
   - Create custom generation classes
   - Reference ASpaceSectorMap for bounds
   - Implement PCG (Procedural Content Generation) nodes
   - Use deterministic random seeds for consistency

### Multi-Sector Layouts

1. **Connected Sectors**: Create multiple sectors in a grid
2. **Jump Gates**: Add transition points between sectors
3. **Sector Streaming**: Implement level streaming for seamless travel
4. **Minimap/Galaxy Map**: Create UI showing sector layout

---

## Troubleshooting

### Common Issues

**Issue**: Can't find SpaceSectorMap in Place Actors
- **Solution**: Ensure project is compiled successfully
- **Solution**: Rebuild project from Visual Studio
- **Solution**: Restart Unreal Engine

**Issue**: Objects falling through the map
- **Solution**: Check World Settings → Kill Z is set very low
- **Solution**: Disable gravity if not needed (Default Gravity Z = 0)

**Issue**: Lighting too bright or too dark
- **Solution**: Adjust Directional Light intensity (3.0-5.0 typical)
- **Solution**: Rebuild lighting (Build → Build Lighting Only)
- **Solution**: Check Post Process Volume exposure settings

**Issue**: Poor performance with empty map
- **Solution**: Disable unnecessary lighting features
- **Solution**: Check for infinite loops in Blueprints
- **Solution**: Profile with `stat unit` to identify bottlenecks

**Issue**: Sector bounds not working correctly
- **Solution**: Verify SpaceSectorMap is at correct location
- **Solution**: Check GetSectorBounds() in debugger
- **Solution**: Ensure using centimeters (UE units), not meters

---

## Advanced Topics

### Dynamic Sector Loading
- Use World Composition for large multi-sector universes
- Implement distance-based streaming
- Create persistent data system for sector state

### Sector Transitions
- Create seamless sector-to-sector travel
- Implement loading screens if needed
- Handle player position across sector boundaries

### Networking Considerations
- Plan for multiplayer sector ownership
- Implement sector-based relevancy
- Handle cross-sector visibility

---

## Reference

### Sector Dimensions
- **Size**: 200km x 200km x 200km
- **Units**: 20,000,000 x 20,000,000 x 20,000,000 (Unreal centimeters)
- **Inspiration**: X4: Foundations sector scale
- **Half-size**: 100km (10,000,000 units) from center to edge

### Related Documentation
- `ARCHITECTURE.md` - Overall project architecture
- `Assets/StationSetupGuide.md` - Space station creation
- `Assets/SpaceshipTemplates/` - Ship configuration
- `Assets/TradingSystemGuide.md` - Economy setup
- `BLUEPRINT_CREATION_GUIDE.md` - Blueprint workflows

### API Reference

**ASpaceSectorMap Functions**:
- `GetSectorBounds()` - Returns FBox of sector boundaries
- `GetSectorHalfSize()` - Returns 100km (10,000,000.0f)
- `IsPositionInSector(FVector)` - Check if position is in sector
- `GetRandomPositionInSector()` - Get random position within bounds
- `GetSectorCenter()` - Get center position in world space

---

**Last Updated**: 2025-10-28  
**Version**: 1.0  
**Unreal Engine**: 5.3+  
**Maintained by**: Adastrea Development Team
