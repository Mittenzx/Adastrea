# Scene and Asset Population Guide

This guide explains how to use the automated scene and asset population scripts to quickly populate Unreal Engine levels with actors based on data assets and configuration files.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Scene Populator](#scene-populator)
- [Interior Layout Populator](#interior-layout-populator)
- [Configuration Files](#configuration-files)
- [Placement Patterns](#placement-patterns)
- [Integration with Editor Utilities](#integration-with-editor-utilities)
- [Command-Line Usage](#command-line-usage)
- [Examples](#examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

## Overview

The scene population system provides two main scripts:

1. **ScenePopulator.py** - Populates levels with actors (ships, stations, NPCs, planets)
2. **InteriorLayoutPopulator.py** - Layouts interior spaces with rooms and props

Both scripts support:
- ✓ Multiple placement patterns (grid, random, orbit, spiral, cluster, line)
- ✓ YAML and CSV configuration files
- ✓ Batch placement with customizable parameters
- ✓ Command-line and Editor Utility Widget support
- ✓ Randomization and variation

## Quick Start

### In Unreal Editor

1. **Enable Python Plugin**:
   - Edit → Plugins
   - Search for "Python Editor Script Plugin"
   - Enable and restart

2. **Open Python Console**:
   - Tools → Python → Open Python Console

3. **Populate from Config**:
   ```python
   import ScenePopulator
   ScenePopulator.populate_from_config("PopulationTemplates/example_scene_config.yaml")
   ```

### Using Editor Utilities Menu

```python
import EditorUtilities
EditorUtilities.show_menu()
# Select option 15-19 for scene population
```

## Scene Populator

### Features

- Spawn actors at precise locations or using patterns
- Support for data asset-based actors
- Multiple placement patterns
- Randomization options for rotation and scale
- Export/import actor placements via CSV

### Basic Usage

**Spawn actors in a grid pattern:**
```python
import ScenePopulator

ScenePopulator.spawn_actors_grid(
    asset_path="/Game/Ships/Blueprints/BP_Ship_Fighter",
    count=10,
    spacing=5000,
    center=(0, 0, 0)
)
```

**Spawn actors in orbit:**
```python
ScenePopulator.spawn_actors_orbit(
    asset_path="/Game/Stations/Blueprints/BP_Station_Small",
    count=8,
    radius=15000,
    center=(0, 0, 0)
)
```

**Spawn actors randomly:**
```python
ScenePopulator.spawn_actors_random(
    asset_path="/Game/Planets/Blueprints/BP_Asteroid",
    count=50,
    radius=100000,
    center=(0, 0, 0)
)
```

### Advanced Usage with Custom Patterns

```python
from ScenePopulator import ScenePopulator

populator = ScenePopulator()

# Spawn in spiral pattern
populator.spawn_actors_pattern(
    asset_path="/Game/Ships/BP_Scout",
    pattern='spiral',
    count=12,
    pattern_params={
        'radius': 25000,
        'height': 5000,
        'center': (0, 0, 0),
        'turns': 2
    },
    rotation_randomness=30,
    scale_randomness=0.1
)

# Spawn in cluster pattern
populator.spawn_actors_pattern(
    asset_path="/Game/Ships/BP_Cargo",
    pattern='cluster',
    count=20,
    pattern_params={
        'cluster_count': 4,
        'cluster_radius': 2000,
        'spread': 30000,
        'center': (0, 0, 0)
    }
)
```

## Interior Layout Populator

### Features

- Generate complete room layouts
- Support for 10 room types (bridge, cargo bay, living quarters, etc.)
- 5 decoration themes (military, luxury, industrial, civilian, alien)
- Automatic prop placement
- Corridor generation
- Template-based population

### Room Types

| Room Type | Description | Default Props |
|-----------|-------------|---------------|
| bridge | Command center | Command chairs, consoles, holotable, screens |
| cargo_bay | Storage area | Cargo containers, cranes, crates |
| living_quarters | Crew quarters | Beds, desks, chairs, lockers |
| mess_hall | Dining area | Tables, chairs, benches, food dispensers |
| engineering | Engineering bay | Reactor core, control panels, generators |
| medbay | Medical facility | Medical beds, scanners, cabinets |
| hangar | Vehicle bay | Landing pads, fuel pumps, repair stations |
| armory | Weapons storage | Weapon racks, ammo crates, armor stands |
| laboratory | Research lab | Lab tables, microscopes, computers |
| corridor | Connecting hallway | Wall panels, lights, doors, vents |

### Basic Usage

**Create a single room:**
```python
import InteriorLayoutPopulator

InteriorLayoutPopulator.populate_room(
    room_type='bridge',
    center=(0, 0, 100),
    size=(3000, 3000, 400),
    theme='military'
)
```

**Create a corridor:**
```python
InteriorLayoutPopulator.create_corridor(
    start=(0, 0, 100),
    end=(5000, 0, 100),
    width=400,
    theme='military'
)
```

**Populate complete station interior:**
```python
InteriorLayoutPopulator.populate_station_interior(
    station_type="hub",
    theme="military"
)
```

### Advanced Usage

```python
from InteriorLayoutPopulator import InteriorLayoutPopulator

populator = InteriorLayoutPopulator()

# Create custom interior layout
populator.create_room('bridge', (0, 0, 200), (3000, 3000, 400), 'military', prop_density=1.5)
populator.create_corridor((1500, 0, 200), (5000, 0, 100), 400, 350, 'military')
populator.create_room('cargo_bay', (5000, 0, 100), (5000, 5000, 600), 'industrial', prop_density=1.2)
populator.create_room('living_quarters', (0, 6000, 200), (2000, 2000, 300), 'civilian')
populator.create_room('medbay', (0, -6000, 200), (2500, 2500, 350), 'civilian')
```

## Configuration Files

### YAML Configuration Format

Scene population config example (`example_scene_config.yaml`):

```yaml
spawn_groups:
  - asset_path: "/Game/Ships/Blueprints/BP_Ship_Fighter"
    pattern: orbit
    count: 8
    pattern_params:
      radius: 15000
      center: [0, 0, 0]
      z_variation: 500
    rotation_randomness: 15
    scale_randomness: 0.1
  
  - asset_path: "/Game/Stations/Blueprints/BP_Station_Small"
    pattern: grid
    count: 4
    pattern_params:
      spacing: 20000
      center: [50000, 0, 0]

metadata:
  name: "Basic Space Sector"
  description: "A basic populated space sector"
  author: "Your Name"
  version: "1.0"
```

Interior layout template example (`station_interior_template.yaml`):

```yaml
rooms:
  - type: bridge
    center: [0, 0, 200]
    size: [3000, 3000, 400]
    theme: military
    prop_density: 1.5
  
  - type: cargo_bay
    center: [8000, 0, 100]
    size: [5000, 5000, 600]
    theme: industrial
    prop_density: 1.2

corridors:
  - start: [1500, 0, 200]
    end: [6000, 0, 100]
    width: 400
    height: 350
    theme: military

metadata:
  name: "Standard Space Station Interior"
  station_type: "Military Hub"
```

### CSV Configuration Format

Simple CSV format for precise actor placement (`example_scene_config.csv`):

```csv
asset_path,x,y,z,pitch,yaw,roll,scale_x,scale_y,scale_z
/Game/Ships/Blueprints/BP_Ship_Fighter,0,0,0,0,0,0,1,1,1
/Game/Ships/Blueprints/BP_Ship_Fighter,5000,0,0,0,45,0,1,1,1
/Game/Stations/Blueprints/BP_Station_Small,0,5000,0,0,0,0,1.5,1.5,1.5
```

### Loading Configurations

**From YAML:**
```python
populator = ScenePopulator()
populator.populate_from_config("PopulationTemplates/example_scene_config.yaml")
```

**From CSV:**
```python
populator = ScenePopulator()
populator.populate_from_config("PopulationTemplates/example_scene_config.csv")
```

**Interior from Template:**
```python
populator = InteriorLayoutPopulator()
populator.populate_from_template("PopulationTemplates/station_interior_template.yaml")
```

## Placement Patterns

### Grid Pattern

Places actors in a regular grid:

```python
pattern_params = {
    'spacing': 5000,      # Distance between actors
    'center': (0, 0, 0)   # Grid center point
}
```

### Random (Sphere) Pattern

Randomly distributes actors within a sphere:

```python
pattern_params = {
    'radius': 10000,      # Sphere radius
    'center': (0, 0, 0)   # Sphere center
}
```

### Orbit Pattern

Places actors in circular orbit:

```python
pattern_params = {
    'radius': 10000,      # Orbit radius
    'center': (0, 0, 0),  # Orbit center
    'z_variation': 500    # Random Z variation
}
```

### Spiral Pattern

Places actors in a spiral:

```python
pattern_params = {
    'radius': 25000,      # Maximum radius
    'height': 5000,       # Total height
    'center': (0, 0, 0),  # Spiral center
    'turns': 3            # Number of complete turns
}
```

### Cluster Pattern

Creates multiple clusters of actors:

```python
pattern_params = {
    'cluster_count': 3,      # Number of clusters
    'cluster_radius': 2000,  # Radius of each cluster
    'spread': 10000,         # Distance between clusters
    'center': (0, 0, 0)      # Overall center
}
```

### Line Pattern

Places actors in a straight line:

```python
pattern_params = {
    'spacing': 5000,           # Distance between actors
    'start': (0, 0, 0),        # Starting point
    'direction': (1, 0, 0)     # Direction vector
}
```

## Integration with Editor Utilities

The scene population features are integrated into the unified Editor Utilities menu:

```python
import EditorUtilities
EditorUtilities.show_menu()
```

Menu options:
- **Option 15**: Populate Level from Config
- **Option 16**: Spawn Actors (Grid Pattern)
- **Option 17**: Spawn Actors (Orbit Pattern)
- **Option 18**: Populate Station Interior
- **Option 19**: Clear Spawned Actors

## Command-Line Usage

### ScenePopulator

```bash
# Populate from config
python ScenePopulator.py --config PopulationTemplates/example_scene_config.yaml

# Quick spawn with grid pattern
python ScenePopulator.py --pattern grid --asset /Game/Ships/BP_Fighter --count 10 --spacing 5000

# Spawn in orbit
python ScenePopulator.py --pattern orbit --asset /Game/Stations/BP_Station --count 8 --radius 15000
```

### InteriorLayoutPopulator

```bash
# Populate from template
python InteriorLayoutPopulator.py --template PopulationTemplates/station_interior_template.yaml

# Create specific room
python InteriorLayoutPopulator.py --room bridge --size 3000,3000,400 --theme military
```

## Examples

### Example 1: Populate a Space Battle Scene

```python
from ScenePopulator import ScenePopulator

populator = ScenePopulator()

# Spawn player fleet in formation
populator.spawn_actors_pattern(
    "/Game/Ships/BP_Fighter",
    'line',
    10,
    {'spacing': 3000, 'start': (-15000, 0, 0), 'direction': (0, 1, 0)}
)

# Spawn enemy fleet
populator.spawn_actors_pattern(
    "/Game/Ships/BP_Enemy_Fighter",
    'line',
    10,
    {'spacing': 3000, 'start': (15000, 0, 0), 'direction': (0, 1, 0)}
)

# Add asteroids for cover
populator.spawn_actors_pattern(
    "/Game/Planets/BP_Asteroid",
    'random',
    30,
    {'radius': 20000, 'center': (0, 0, 0)},
    rotation_randomness=180,
    scale_randomness=0.4
)
```

### Example 2: Create a Trading Hub Station

```python
from InteriorLayoutPopulator import InteriorLayoutPopulator

populator = InteriorLayoutPopulator()

# Create main trading floor
populator.create_room('bridge', (0, 0, 200), (5000, 5000, 500), 'civilian', prop_density=2.0)

# Add cargo storage
populator.create_room('cargo_bay', (10000, 0, 100), (6000, 6000, 600), 'industrial')

# Create living quarters for traders
for i in range(4):
    x = i * 3000
    populator.create_room('living_quarters', (0, x + 8000, 200), (2000, 2000, 300), 'civilian')

# Connect with corridors
populator.create_corridor((2500, 0, 200), (8000, 0, 100), 500, 400, 'civilian')
```

### Example 3: Populate from Configuration File

**Create config file** (`my_scene.yaml`):
```yaml
spawn_groups:
  - asset_path: "/Game/Ships/BP_Trader"
    pattern: orbit
    count: 12
    pattern_params:
      radius: 20000
      center: [0, 0, 0]
  
  - asset_path: "/Game/Stations/BP_TradeStation"
    pattern: grid
    count: 3
    pattern_params:
      spacing: 40000
      center: [0, 0, 0]
```

**Populate:**
```python
import ScenePopulator
ScenePopulator.populate_from_config("my_scene.yaml")
```

## Best Practices

### Scene Population

✓ **Use Appropriate Patterns**: Choose patterns that match your scene needs
✓ **Start Small**: Test with small counts before populating entire levels
✓ **Use Configuration Files**: Store complex setups in YAML for reusability
✓ **Consider Performance**: Large numbers of actors may impact editor performance
✓ **Export Configurations**: Use `export_spawned_to_csv()` to save successful layouts
✓ **Clear Regularly**: Use `clear_spawned_actors()` when iterating

### Interior Layout

✓ **Plan Layout First**: Sketch room positions before populating
✓ **Match Themes**: Use consistent themes for coherent aesthetics
✓ **Adjust Prop Density**: Start with default (1.0), adjust as needed
✓ **Test Navigation**: Ensure corridors connect properly
✓ **Room Sizes**: Use realistic sizes based on function
✓ **Height Matters**: Different rooms need different ceiling heights

### Configuration Files

✓ **Comment Liberally**: Add comments to explain complex setups
✓ **Version Control**: Commit configs to track changes
✓ **Reuse Templates**: Create library of common patterns
✓ **Validate Coordinates**: Double-check positions don't overlap
✓ **Metadata**: Include author, date, description for reference

## Troubleshooting

### Actors Not Spawning

**Problem**: `populate_from_config()` completes but no actors appear

**Solutions**:
- Verify asset paths are correct (use `/Game/...` format)
- Check assets exist in Content Browser
- Ensure level is open in editor
- Check console for error messages
- Verify Python plugin is enabled

### Assets Loading Incorrectly

**Problem**: Wrong assets spawn or assets missing

**Solutions**:
- Use full asset paths including blueprint name
- Check asset paths in Content Browser (right-click → Copy Reference)
- Ensure assets are not in protected folders
- Verify asset types are compatible (Blueprints, not Data Assets)

### Interior Rooms Overlap

**Problem**: Rooms overlap or corridors don't connect

**Solutions**:
- Review room center positions and sizes
- Calculate corridor endpoints carefully
- Use paper/digital sketch to plan layout first
- Ensure coordinates account for room sizes (half-width from center)
- Check Z coordinates match between connected rooms

### Performance Issues

**Problem**: Editor becomes slow when spawning many actors

**Solutions**:
- Reduce actor counts during iteration
- Use simpler assets for testing
- Clear spawned actors regularly
- Spawn in batches
- Consider Level of Detail (LOD) for distant actors
- Disable real-time rendering during population

### PyYAML Not Available

**Problem**: "PyYAML not available" error

**Solution**:
```python
# In Unreal Editor Python Console:
import subprocess, sys
subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyyaml'])
```

### CSV Format Errors

**Problem**: CSV import fails or spawns incorrectly

**Solutions**:
- Ensure first row is header with exact column names
- Use commas as delimiters (not semicolons)
- Don't use quotes unless necessary
- Verify numeric values don't have units
- Check for extra spaces in asset paths

## Additional Resources

- [ScenePopulator.py](../ScenePopulator.py) - Full source code
- [InteriorLayoutPopulator.py](../InteriorLayoutPopulator.py) - Full source code
- [EditorUtilities.py](../EditorUtilities.py) - Unified interface
- [PopulationTemplates/](../PopulationTemplates/) - Example configurations
- [Unreal Python API Documentation](https://docs.unrealengine.com/en-US/ProductionPipelines/ScriptingAndAutomation/Python/)

## Contributing

When creating new patterns or room types:

1. Add pattern/room definition to appropriate class
2. Update documentation with examples
3. Create example configuration files
4. Test thoroughly in editor
5. Submit pull request with examples

---

**Last Updated**: 2025-11-07  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
