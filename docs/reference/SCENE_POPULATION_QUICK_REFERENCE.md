# Scene Population Quick Reference

Quick reference for the Adastrea scene and asset population tools.

## Import Statements

```python
import ScenePopulator
import InteriorLayoutPopulator
import EditorUtilities
```

## Scene Population - Quick Commands

### Spawn Patterns

```python
# Grid
ScenePopulator.spawn_actors_grid("/Game/Ships/BP_Fighter", count=10, spacing=5000)

# Orbit
ScenePopulator.spawn_actors_orbit("/Game/Stations/BP_Station", count=8, radius=15000)

# Random
ScenePopulator.spawn_actors_random("/Game/Planets/BP_Asteroid", count=50, radius=100000)

# From Config
ScenePopulator.populate_from_config("config.yaml")
```

### Advanced Patterns

```python
from ScenePopulator import ScenePopulator
populator = ScenePopulator()

# Spiral
populator.spawn_actors_pattern("/Game/Ships/BP_Scout", 'spiral', 12,
    {'radius': 25000, 'height': 5000, 'turns': 2})

# Cluster
populator.spawn_actors_pattern("/Game/Ships/BP_Cargo", 'cluster', 20,
    {'cluster_count': 4, 'cluster_radius': 2000, 'spread': 30000})

# Line
populator.spawn_actors_pattern("/Game/Ships/BP_Fighter", 'line', 10,
    {'spacing': 5000, 'start': (0, 0, 0), 'direction': (1, 0, 0)})
```

## Interior Population - Quick Commands

### Create Rooms

```python
# Single room
InteriorLayoutPopulator.populate_room('bridge', center=(0, 0, 100), 
    size=(3000, 3000, 400), theme='military')

# Corridor
InteriorLayoutPopulator.create_corridor(start=(0, 0, 100), end=(5000, 0, 100), 
    width=400, theme='military')

# Complete station
InteriorLayoutPopulator.populate_station_interior(station_type="hub", 
    theme="military")
```

### Room Types

- `bridge` - Command center
- `cargo_bay` - Storage area  
- `living_quarters` - Crew quarters
- `mess_hall` - Dining area
- `engineering` - Engineering bay
- `medbay` - Medical facility
- `hangar` - Vehicle bay
- `armory` - Weapons storage
- `laboratory` - Research lab
- `corridor` - Hallway

### Themes

- `military` - Gray, utilitarian, harsh lighting
- `luxury` - Gold/white, elegant, warm lighting
- `industrial` - Rust/steel, functional, bright lighting
- `civilian` - Blue/white, comfortable, soft lighting
- `alien` - Purple/green, organic, bioluminescent

## Editor Utilities Menu

```python
import EditorUtilities
EditorUtilities.show_menu()
```

Menu options:
- **15**: Populate Level from Config
- **16**: Spawn Actors (Grid)
- **17**: Spawn Actors (Orbit)
- **18**: Populate Station Interior
- **19**: Clear Spawned Actors

## Configuration File Templates

### YAML Scene Config

```yaml
spawn_groups:
  - asset_path: "/Game/Ships/BP_Fighter"
    pattern: orbit
    count: 8
    pattern_params:
      radius: 15000
      center: [0, 0, 0]
    rotation_randomness: 15
    scale_randomness: 0.1
```

### CSV Scene Config

```csv
asset_path,x,y,z,pitch,yaw,roll,scale_x,scale_y,scale_z
/Game/Ships/BP_Fighter,0,0,0,0,0,0,1,1,1
```

### YAML Interior Config

```yaml
rooms:
  - type: bridge
    center: [0, 0, 200]
    size: [3000, 3000, 400]
    theme: military
    prop_density: 1.5

corridors:
  - start: [1500, 0, 200]
    end: [6000, 0, 100]
    width: 400
    height: 350
    theme: military
```

## Pattern Parameters Reference

| Pattern | Required Params | Optional Params |
|---------|----------------|-----------------|
| grid | spacing | center |
| random | radius | center |
| orbit | radius | center, z_variation |
| spiral | radius, height | center, turns |
| cluster | cluster_count, cluster_radius, spread | center |
| line | spacing, start, direction | - |

## Common Workflows

### 1. Populate Open Space Level

```python
# Create YAML config or use CSV
# Then:
import ScenePopulator
ScenePopulator.populate_from_config("my_scene.yaml")
```

### 2. Create Station Interior

```python
from InteriorLayoutPopulator import InteriorLayoutPopulator
pop = InteriorLayoutPopulator()

pop.create_room('bridge', (0, 0, 200), (3000, 3000, 400), 'military')
pop.create_corridor((1500, 0, 200), (5000, 0, 100), theme='military')
pop.create_room('cargo_bay', (5000, 0, 100), (4000, 4000, 600), 'industrial')
```

### 3. Quick Battle Scene

```python
from ScenePopulator import ScenePopulator
pop = ScenePopulator()

# Player fleet
pop.spawn_actors_pattern("/Game/Ships/BP_Fighter", 'line', 10,
    {'spacing': 3000, 'start': (-15000, 0, 0), 'direction': (0, 1, 0)})

# Enemy fleet  
pop.spawn_actors_pattern("/Game/Ships/BP_Enemy", 'line', 10,
    {'spacing': 3000, 'start': (15000, 0, 0), 'direction': (0, 1, 0)})
```

## Command-Line Usage

```bash
# Scene population
python ScenePopulator.py --config scene.yaml
python ScenePopulator.py --pattern grid --asset /Game/Ships/BP_Fighter --count 10

# Interior population
python InteriorLayoutPopulator.py --template interior.yaml
python InteriorLayoutPopulator.py --room bridge --size 3000,3000,400 --theme military
```

## Troubleshooting Quick Fixes

| Problem | Solution |
|---------|----------|
| Actors not spawning | Check asset paths, ensure level is open |
| PyYAML error | `import subprocess, sys; subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyyaml'])` |
| Slow performance | Reduce count, clear actors regularly |
| Overlapping rooms | Check center positions and sizes |
| Wrong assets | Verify asset path with Copy Reference in Content Browser |

## File Locations

- Scripts: Project root (`ScenePopulator.py`, `InteriorLayoutPopulator.py`)
- Templates: `PopulationTemplates/`
- Documentation: `SCENE_POPULATION_GUIDE.md`

---

**For detailed documentation, see**: [SCENE_POPULATION_GUIDE.md](SCENE_POPULATION_GUIDE.md)
