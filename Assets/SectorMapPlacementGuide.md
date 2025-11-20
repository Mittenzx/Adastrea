# Sector Map Placement Guide

Complete guide for placing and organizing SpaceSectorMap actors in your Unreal Engine levels.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Understanding Sector Boundaries](#understanding-sector-boundaries)
- [Placement Strategies](#placement-strategies)
- [Example Layouts](#example-layouts)
- [Coordinate Planning](#coordinate-planning)
- [Debugging and Visualization](#debugging-and-visualization)
- [Best Practices](#best-practices)

## Overview

`ASpaceSectorMap` actors mark 200km x 200km regions of space in your level. They provide:

- **Physical boundaries** for sector regions
- **Reference points** for content placement
- **Sector identification** for gameplay systems
- **Editor visualization** for level design

## Quick Start

### 1. Place First Sector

1. Open your level in the Unreal Editor
2. In the **Place Actors** panel, search for `SpaceSectorMap`
3. Drag the actor into your level
4. Place it at your desired sector center (e.g., `0, 0, 0`)
5. In **Details** panel, set:
   - **Sector Name:** "Your Sector Name"
   - **Description:** Brief description

### 2. Configure Properties

```
Location: (0, 0, 0)  // Sector center
Rotation: (0, 0, 0)  // Usually not needed
Scale: (1, 1, 1)     // Don't change scale!

Sector Name: "Core Systems"
Description: "Central hub of civilization"
```

### 3. Verify Placement

- Check console log on Play for initialization message
- Use debug visualization to see sector bounds
- Ensure name matches corresponding Sector Council Data Asset

## Understanding Sector Boundaries

### Sector Dimensions

- **Size:** 200km x 200km x 200km
- **Unreal Units:** 20,000,000 x 20,000,000 x 20,000,000 cm
- **From Center:** ±100km (±10,000,000 units) in each direction

### Boundary Math

```cpp
// Sector constants
SectorSize = 20,000,000 units (200km)
SectorHalfSize = 10,000,000 units (100km)

// If sector center is at (X, Y, Z):
MinX = X - 10,000,000
MaxX = X + 10,000,000
MinY = Y - 10,000,000
MaxY = Y + 10,000,000
MinZ = Z - 10,000,000
MaxZ = Z + 10,000,000
```

### Example Sector Bounds

**Sector at Origin (0, 0, 0):**
```
Min: (-10,000,000, -10,000,000, -10,000,000)
Max: ( 10,000,000,  10,000,000,  10,000,000)
```

**Sector at (20,000,000, 0, 0):**
```
Min: ( 10,000,000, -10,000,000, -10,000,000)
Max: ( 30,000,000,  10,000,000,  10,000,000)
```

## Placement Strategies

### Strategy 1: Grid Layout (Recommended)

**Use Case:** Organized, connected sectors

Place sectors in a regular grid pattern with spacing of exactly 20,000,000 units:

```
Sector A: (0, 0, 0)
Sector B: (20000000, 0, 0)        // 200km east
Sector C: (0, 20000000, 0)        // 200km north
Sector D: (20000000, 20000000, 0) // 200km northeast
Sector E: (0, 0, 20000000)        // 200km up
```

**Advantages:**
- Clear organization
- No gaps or overlaps
- Easy to navigate
- Predictable coordinates

**Blueprint Helper:**
```cpp
// Get adjacent sector center
FVector GetAdjacentSectorCenter(FVector CurrentCenter, EDirection Direction)
{
    const float SectorSize = 20000000.0f;
    
    switch(Direction)
    {
        case EDirection::East:  return CurrentCenter + FVector(SectorSize, 0, 0);
        case EDirection::West:  return CurrentCenter + FVector(-SectorSize, 0, 0);
        case EDirection::North: return CurrentCenter + FVector(0, SectorSize, 0);
        case EDirection::South: return CurrentCenter + FVector(0, -SectorSize, 0);
        case EDirection::Up:    return CurrentCenter + FVector(0, 0, SectorSize);
        case EDirection::Down:  return CurrentCenter + FVector(0, 0, -SectorSize);
    }
}
```

### Strategy 2: Clustered Layout

**Use Case:** Groups of related sectors with empty space between

Place sectors in clusters with gaps between groups:

```
// Core Cluster
Core_A: (0, 0, 0)
Core_B: (20000000, 0, 0)
Core_C: (0, 20000000, 0)

// Frontier Cluster (200km gap to the east)
Frontier_A: (60000000, 0, 0)
Frontier_B: (80000000, 0, 0)
Frontier_C: (60000000, 20000000, 0)
```

**Advantages:**
- Logical grouping
- Travel distance meaningful
- Empty space feels vast
- Different regions distinct

### Strategy 3: Sparse Layout

**Use Case:** Isolated sectors across vast distances

Place sectors with significant gaps:

```
CoreSystems: (0, 0, 0)
FrontierPost: (100000000, 0, 0)      // 1,000km away
RemoteColony: (0, 150000000, 0)      // 1,500km away
DistantOutpost: (200000000, 200000000, 0)  // Very far
```

**Advantages:**
- Emphasizes scale
- Jump drives essential
- True frontier feel
- Each sector is special

### Strategy 4: Vertical Stacking

**Use Case:** Multiple sectors at same XY but different Z

Place sectors above/below each other:

```
Lower_Sector: (0, 0, -20000000)    // Below
Mid_Sector: (0, 0, 0)              // Middle
Upper_Sector: (0, 0, 20000000)     // Above
```

**Advantages:**
- 3D space utilization
- Distinct vertical zones
- Interesting navigation
- Unique gameplay per level

## Example Layouts

### Example 1: Tutorial System (3 Sectors)

**Purpose:** Small, safe starting area

```
Coordinates:

Starting_Station: (0, 0, 0)
  - Spawn point
  - Core Systems council
  - Safe, friendly

Training_Ground: (20000000, 0, 0)
  - Tutorial missions
  - Practice combat
  - Teaching sector

Trade_Outpost: (0, 20000000, 0)
  - Learn trading
  - Basic economy
  - Merchant training
```

**Travel Distance:** 200km between adjacent sectors (easy)

### Example 2: Core Region (9 Sectors)

**Purpose:** Central civilization hub

```
3x3 Grid Layout (all at Z=0):

Northwest     North         Northeast
(-20M, 20M)   (0, 20M)     (20M, 20M)

West          Center        East
(-20M, 0)     (0, 0)        (20M, 0)

Southwest     South         Southeast
(-20M, -20M)  (0, -20M)     (20M, -20M)

Center sector: Capital system (Core Systems template)
Adjacent 4: Major hub sectors (Trade, Research, Manufacturing)
Corner 4: Specialized sectors (Mining, Agriculture, Tourism)
```

**Total Area:** 600km x 600km

### Example 3: Frontier Chain (5 Sectors)

**Purpose:** Mining frontier with progressive danger

```
Linear Chain (traveling east):

Safe_Zone: (0, 0, 0)
  - Last outpost of civilization
  - Resupply station
  - Low danger

Mining_Alpha: (20000000, 0, 0)
  - Basic mining operations
  - Moderate danger
  - Some pirate activity

Mining_Beta: (40000000, 0, 0)
  - Rich ore deposits
  - High danger
  - Frequent pirate raids

Mining_Gamma: (60000000, 0, 0)
  - Premium resources
  - Very high danger
  - Organized pirate base nearby

Pirate_Territory: (80000000, 0, 0)
  - Enemy sector
  - Extreme danger
  - High rewards for brave pilots
```

**Progression:** Danger and reward increase with distance

### Example 4: Trade Network (7 Sectors)

**Purpose:** Economic trade route

```
Hub and Spoke Pattern:

Central_Hub: (0, 0, 0)
  - Major trade station
  - Trade Hub council template
  - All routes converge here

Production_Alpha: (-30000000, 0, 0)
  - Manufacturing sector
  - Produces goods
  - 300km west

Production_Beta: (30000000, 0, 0)
  - Mining sector
  - Produces raw materials
  - 300km east

Production_Gamma: (0, -30000000, 0)
  - Agricultural sector
  - Produces food/organics
  - 300km south

Consumer_Alpha: (0, 30000000, 0)
  - Residential sector
  - High demand for goods
  - 300km north

Research_Station: (0, 0, 30000000)
  - Technology sector
  - Special products
  - 300km up

Military_Base: (0, 0, -30000000)
  - Defense sector
  - Military contracts
  - 300km down
```

**Trade Routes:** All routes go through central hub

### Example 5: Border Defense (6 Sectors)

**Purpose:** Military defense line

```
Defense Line (facing east):

Safe_Rear_West: (-40000000, -20000000, 0)
Safe_Rear_Center: (-40000000, 0, 0)
Safe_Rear_East: (-40000000, 20000000, 0)

Border_West: (-20000000, -20000000, 0)
  - Military Border template
  - Heavy defenses

Border_Center: (-20000000, 0, 0)
  - Command post
  - Strongest defenses

Border_East: (-20000000, 20000000, 0)
  - Military Border template
  - Heavy defenses

Enemy_Territory: (0, 0, 0) and beyond
  - Dangerous space
  - Hostile forces
```

**Defense Depth:** 400km buffer zone before enemy space

## Coordinate Planning

### Planning Worksheet

Use this template to plan your sector layout:

```
SECTOR LAYOUT PLAN
==================

Sector Name: _____________________
Location: (________, ________, ________)
Council Template: _____________________
Theme: _____________________

Adjacent Sectors:
  North (+Y): _____________________
  South (-Y): _____________________
  East (+X):  _____________________
  West (-X):  _____________________
  Up (+Z):    _____________________
  Down (-Z):  _____________________

Travel Times:
  To [Sector]: ___ km = ___ minutes at 500m/s
  To [Sector]: ___ km = ___ minutes at 500m/s

Content:
  Major Stations: ___
  Minor Stations: ___
  NPC Ships: ___
  Resource Fields: ___
```

### Coordinate Calculator

```python
# Python helper to calculate sector positions
SECTOR_SIZE = 20_000_000  # 200km in cm

def sector_position(grid_x, grid_y, grid_z, origin=(0,0,0)):
    """
    Calculate world position for a sector in grid coordinates.
    
    Args:
        grid_x, grid_y, grid_z: Grid position (0,0,0 is origin)
        origin: World position of grid origin
    
    Returns:
        (x, y, z) world coordinates
    """
    return (
        origin[0] + (grid_x * SECTOR_SIZE),
        origin[1] + (grid_y * SECTOR_SIZE),
        origin[2] + (grid_z * SECTOR_SIZE)
    )

# Example usage:
core_system = sector_position(0, 0, 0)      # (0, 0, 0)
trade_hub = sector_position(1, 0, 0)        # (20000000, 0, 0)
research = sector_position(0, 1, 0)         # (0, 20000000, 0)
frontier = sector_position(3, -2, 0)        # (60000000, -40000000, 0)
```

### Distance Calculator

```python
import math

def distance_between_sectors(pos1, pos2):
    """Calculate distance between two sector centers."""
    dx = pos2[0] - pos1[0]
    dy = pos2[1] - pos1[1]
    dz = pos2[2] - pos1[2]
    return math.sqrt(dx*dx + dy*dy + dz*dz)

def travel_time(distance_cm, speed_cm_per_sec):
    """Calculate travel time."""
    return distance_cm / speed_cm_per_sec

# Example:
pos_a = (0, 0, 0)
pos_b = (20000000, 0, 0)
dist = distance_between_sectors(pos_a, pos_b)
time = travel_time(dist, 50000)  # 500m/s = 50000cm/s

print(f"Distance: {dist/100000:.1f} km")
print(f"Travel time at 500m/s: {time:.1f} seconds = {time/60:.1f} minutes")
```

## Debugging and Visualization

### Console Commands

```cpp
// Show sector bounds (if implemented)
ShowSectorBounds

// List all sectors in level
ListSectors

// Teleport to sector center
TeleportToSector SectorName

// Show sector info
GetSectorInfo SectorName
```

### Blueprint Visualization

Create a Blueprint to visualize sector boundaries:

```cpp
// On tick or event
DrawDebugBox(
    GetWorld(),
    SectorMapActor->GetActorLocation(),
    FVector(10000000, 10000000, 10000000),  // Half-size
    FColor::Green,
    false,  // Persistent
    -1,     // Lifetime (infinite)
    0,      // Depth priority
    100     // Thickness
);
```

### Editor Gizmos

The `UBillboardComponent` in `ASpaceSectorMap` provides editor visualization:

- Visible in editor viewport
- Selectable for easy finding
- Icon indicates sector center
- Screen-size scaled (always visible)

## Best Practices

### Do:

✓ **Plan layout before placing**
  - Sketch on paper or spreadsheet
  - Consider travel distances
  - Think about sector relationships

✓ **Use consistent spacing**
  - Stick to multiples of 20,000,000
  - Regular grids are easiest
  - Document your spacing strategy

✓ **Name sectors clearly**
  - Descriptive names
  - Consistent naming convention
  - Match Data Asset names exactly

✓ **Consider 3D space**
  - Use vertical dimension
  - Stack sectors when appropriate
  - Think about visual interest

✓ **Test travel times**
  - Fly between sectors
  - Check if distances feel right
  - Adjust if too far or too close

✓ **Document your layout**
  - Keep a sector map document
  - Note coordinates
  - Explain organization

### Don't:

✗ **Don't overlap sectors**
  - Causes boundary confusion
  - Unclear which sector you're in
  - System errors possible

✗ **Don't use random positions**
  - Hard to maintain
  - Confusing for players
  - Difficult to expand later

✗ **Don't forget Z axis**
  - Space is 3D!
  - Vertical separation useful
  - Adds gameplay variety

✗ **Don't make all sectors adjacent**
  - No sense of scale
  - Empty space is okay
  - Distance matters

✗ **Don't change actor scale**
  - Breaks boundary calculations
  - Keep scale at (1, 1, 1)
  - Use position only

✗ **Don't place without testing**
  - Always verify in Play mode
  - Check console for errors
  - Fly through to test

## Quick Reference

### Standard Coordinates (Origin at 0,0,0)

```
Grid Position | World Coordinates      | Distance from Origin
--------------|------------------------|---------------------
(0, 0, 0)     | (0, 0, 0)             | 0 km
(1, 0, 0)     | (20000000, 0, 0)      | 200 km
(0, 1, 0)     | (0, 20000000, 0)      | 200 km
(1, 1, 0)     | (20000000, 20000000, 0)| 283 km (diagonal)
(2, 0, 0)     | (40000000, 0, 0)      | 400 km
(0, 0, 1)     | (0, 0, 20000000)      | 200 km
```

### Travel Time Reference (at 500 m/s)

| Distance | Travel Time |
|----------|-------------|
| 200 km (1 sector) | 6.7 minutes |
| 283 km (diagonal) | 9.4 minutes |
| 400 km (2 sectors) | 13.3 minutes |
| 600 km (3 sectors) | 20 minutes |
| 1000 km (5 sectors) | 33.3 minutes |

### Sector Count by Layout

| Layout | Sectors | Total Area | Organization |
|--------|---------|------------|--------------|
| 3x3 grid | 9 | 600x600 km | Core region |
| 5x5 grid | 25 | 1000x1000 km | Major system |
| Linear chain (10) | 10 | 2000 km long | Route |
| Hub + 6 spokes | 7 | Varies | Trade network |
| Clustered (3x5) | 15 | Custom | Multiple regions |

## Next Steps

1. **Plan your layout** using the patterns above
2. **Calculate coordinates** for each sector
3. **Place SpaceSectorMap actors** in your level
4. **Configure properties** to match Sector Council Data Assets
5. **Test in Play mode** to verify placement
6. **Populate with content** using sector bounds as reference

## Additional Resources

- **Sector Councils:** `Assets/SectorCreationGuide.md`
- **Templates:** `Assets/SectorTemplates/`
- **Way System:** `Assets/WaySystemGuide.md`
- **Empty Map Setup:** `Docs/EmptyMapSetup.md`

---

**Last Updated:** November 2024  
**Version:** 1.0  
**Maintained by:** Adastrea Development Team
