# Sector Generator Guide

Complete guide for using the procedural sector generator to populate space sectors with stations, ships, asteroids, clouds, and other objects.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [System Components](#system-components)
- [Creating a Generator Config](#creating-a-generator-config)
- [Object Definitions](#object-definitions)
- [Distribution Patterns](#distribution-patterns)
- [Name Generation](#name-generation)
- [Faction Integration](#faction-integration)
- [Usage Examples](#usage-examples)
- [Troubleshooting](#troubleshooting)

## Overview

The Sector Generator system provides procedural content generation for Adastrea's space sectors. It can:

- **Generate sector names** using multiple thematic patterns
- **Populate sectors** with diverse space objects
- **Assign faction ownership** to stations and ships
- **Apply distribution patterns** for realistic placement
- **Support reproducible generation** via random seeds
- **Provide full Blueprint control** for designers

### Key Features

| Feature | Description |
|---------|-------------|
| **Data-Driven** | Configure via Data Assets, no C++ required |
| **Multiple Patterns** | Random, clustered, grid, orbital, scattered |
| **Faction-Aware** | Assign ownership and relationships |
| **Preview Mode** | Test generation without committing |
| **Collision Avoidance** | Automatic spacing between objects |
| **Extensible** | BlueprintNativeEvent hooks for customization |

## Quick Start

### 1. Create a Generator Config Data Asset

1. In Content Browser, right-click → **Blueprint Class**
2. Search for `SectorGeneratorConfig`, select it
3. Name it `DA_SectorConfig_YourSectorType` (e.g., `DA_SectorConfig_Mining`)
4. Open the asset and configure:
   - **Config Name**: Display name for this config
   - **Naming Theme**: Choose sector naming style (Military, Mining, Trade, etc.)
   - **Density Multiplier**: Object density (1.0 = default)
   - **Object Definitions**: Add object types to spawn

### 2. Add Object Definitions

Click **+ Add** under **Object Definitions** for each object type you want to spawn:

```yaml
Example Station Definition:
  Object Type: Station
  Actor Class: BP_SpaceStation_Trading
  Min Count: 1
  Max Count: 3
  Distribution Pattern: Random
  Owner Faction: DA_Faction_TradingGuild

Example Asteroid Field Definition:
  Object Type: Asteroid Field
  Actor Class: BP_AsteroidField
  Min Count: 5
  Max Count: 15
  Distribution Pattern: Clustered
  Scale Variation: 2.0
  Min Distance From Center: 500000 (5km)
  Max Distance From Center: 5000000 (50km)

Example Ship Patrol Definition:
  Object Type: Ship
  Actor Class: BP_Spaceship_Fighter
  Ship Data: DA_Ship_Fighter_Light
  Min Count: 3
  Max Count: 8
  Distribution Pattern: Orbital
  Owner Faction: DA_Faction_Military
```

### 3. Place the Generator in Your Level

1. Find your sector's level map (e.g., `L_Sector_Alpha`)
2. Find the `ASpaceSectorMap` actor in the level
3. Place an `ASectorGenerator` actor in the level
4. In the Details panel:
   - **Target Sector**: Select your `ASpaceSectorMap` actor
   - **Generator Config**: Select your config Data Asset
   - **Show Debug Visuals**: Enable for testing (optional)
5. Click **Generate Sector** button in Details panel

### 4. Generate and Test

- Click **Generate Sector** to populate
- Click **Clear Sector** to remove generated content
- Click **Regenerate Sector** to clear and generate fresh
- Adjust config and regenerate until satisfied

## System Components

### ASectorGenerator (Actor)

Main actor that performs generation. Place one per sector.

**Key Properties:**
- `TargetSector`: Which sector to populate
- `GeneratorConfig`: Configuration Data Asset
- `bShowDebugVisuals`: Display debug spheres during generation
- `bPreviewMode`: Test mode (doesn't persist actors)

**Key Functions:**
- `GenerateSector()`: Populate the sector
- `ClearSector()`: Remove generated content
- `RegenerateSector()`: Clear and generate fresh
- `ValidateConfiguration()`: Check for errors

### USectorGeneratorConfig (Data Asset)

Configuration for generation behavior.

**Sections:**
1. **Basic Info**: Name and description
2. **Sector Settings**: Naming, seed, density
3. **Object Definitions**: What to spawn
4. **Generation Rules**: Spacing, hubs, zones

### UNameGenerator (Utility Class)

Static functions for procedural name generation.

**Name Themes:**
- Military: "Tactical Zone 7", "Defense Perimeter 12"
- Mining: "Ore Field B", "Extraction Zone 23"
- Trade: "Commerce Hub 5", "Market Junction 18"
- Scientific: "Research Lab AB-402"
- Frontier: "Pioneer Territory 9"
- Greek: "Alpha Sector 3", "Omega Sector 12"
- Numeric: "Sector-0042"
- Descriptive: "Blue Nebula", "Golden Expanse"

## Creating a Generator Config

### Basic Configuration

```cpp
Config Name: Mining Sector Generator
Description: Generates a typical mining sector with stations, asteroids, and patrol ships

Naming Theme: Mining
Random Seed: 0 (0 = random each time, or set specific value for reproducibility)
Density Multiplier: 1.0 (adjust for more/fewer objects)
```

### Generation Rules

```cpp
Min Object Spacing: 50000.0 (500 meters minimum between objects)
Max Spawn Attempts: 100 (attempts to find valid location per object)

Create Central Hub: true
Central Hub Class: BP_SpaceStation_MainHub

Create Zones: false
Num Zones: 3 (if zones enabled)
```

## Object Definitions

Each object definition specifies one type of content to spawn.

### Required Fields

- **Object Type**: Station, Asteroid, Cloud, Ship, etc.
- **Actor Class**: Blueprint class to spawn
- **Min/Max Count**: Number range to spawn

### Optional Fields

- **Distribution Pattern**: Placement strategy
- **Scale Variation**: Size randomization (1.0 = no variation)
- **Min/Max Distance From Center**: Distance constraints
- **Owner Faction**: Faction ownership (for stations/ships)
- **Ship Data**: SpaceshipDataAsset (required for ships)
- **Random Rotation**: Whether to randomize orientation

### Example Object Definitions

#### Trading Station
```yaml
Object Type: Station
Actor Class: BP_SpaceStation_Trading
Min Count: 1
Max Count: 2
Distribution Pattern: Random
Scale Variation: 1.0
Owner Faction: DA_Faction_TradingGuild
Random Rotation: false
```

#### Asteroid Belt
```yaml
Object Type: Asteroid Field
Actor Class: BP_AsteroidCluster
Min Count: 10
Max Count: 30
Distribution Pattern: Clustered
Scale Variation: 3.0
Min Distance From Center: 1000000
Max Distance From Center: 8000000
Random Rotation: true
```

#### Military Patrol
```yaml
Object Type: Ship
Actor Class: BP_Spaceship_Corvette
Ship Data: DA_Ship_Corvette_Patrol
Min Count: 2
Max Count: 4
Distribution Pattern: Orbital
Owner Faction: DA_Faction_Military
Random Rotation: false
```

#### Space Cloud
```yaml
Object Type: Cloud
Actor Class: BP_SpaceCloud_Nebula
Min Count: 3
Max Count: 8
Distribution Pattern: Scattered
Scale Variation: 5.0
Random Rotation: true
```

## Distribution Patterns

Different patterns produce different spatial layouts:

### Random (Uniform Distribution)
- **Use Case**: General purpose, natural-looking
- **Behavior**: Uniform random distribution in sphere
- **Best For**: Asteroids, debris, general objects

### Clustered (Groups)
- **Use Case**: Asteroid fields, resource deposits
- **Behavior**: Creates clusters/groups of objects
- **Best For**: Mining zones, natural formations

### Grid
- **Use Case**: Organized layouts, artificial structures
- **Behavior**: Regular grid pattern
- **Best For**: Satellite arrays, defensive perimeters

### Orbital (Around Center)
- **Use Case**: Patrol routes, orbital stations
- **Behavior**: Ring/orbit around sector center
- **Best For**: Patrol ships, orbital facilities

### Scattered (Wide Distribution)
- **Use Case**: Maximum spread, sparse sectors
- **Behavior**: Wide, even distribution
- **Best For**: Frontier sectors, wide patrols

## Name Generation

### Using Default Generation

Set `Custom Sector Name` to empty to use procedural generation:

```cpp
Naming Theme: Military
Custom Sector Name: (empty)
Result: "Defense Perimeter 7" (or similar)
```

### Using Custom Names

Set a specific name:

```cpp
Naming Theme: (ignored)
Custom Sector Name: "Epsilon Mining Reach"
Result: "Epsilon Mining Reach"
```

### Ensuring Unique Names

The generator automatically checks for name collisions and ensures uniqueness. If a collision occurs, it appends a number or generates a new name.

### Name Generation from C++/Blueprint

```cpp
// C++ example
FString SectorName = UNameGenerator::GenerateSectorName(
    static_cast<uint8>(ESectorNamingTheme::Military), 
    0  // seed
);

// Blueprint: Call "Generate Sector Name" node
// Theme: Military (0), Mining (1), Trade (2), Scientific (3), etc.
```

## Faction Integration

### Assigning Faction Ownership

Set `Owner Faction` on object definitions to assign ownership:

```cpp
Station Definition:
  Owner Faction: DA_Faction_TradingGuild
  
Ship Definition:
  Owner Faction: DA_Faction_Military
  Ship Data: DA_Ship_Corvette
```

### Faction-Aware Actors

For faction ownership to work, your actor Blueprint must:
1. Implement the `IFactionMember` interface
2. Have a `SetFaction` function
3. Store the faction reference

The generator will automatically call `SetFaction` on spawned actors that implement the interface.

### Multi-Faction Sectors

Create multiple object definitions with different factions:

```yaml
Object Definitions:
  - Station (Faction: Trading Guild)
  - Station (Faction: Mining Cooperative)  
  - Ships (Faction: Trading Guild)
  - Ships (Faction: Pirates)
```

## Usage Examples

### Example 1: Safe Mining Sector

```yaml
Config: DA_SectorConfig_SafeMining
Naming Theme: Mining
Density Multiplier: 1.0

Objects:
  - Trading Station (1-2, TradingGuild)
  - Mining Station (2-3, MiningCooperative)
  - Asteroid Fields (15-30, clustered)
  - Security Patrol Ships (2-4, SectorPolice)
  - Cargo Ships (3-6, various factions)
```

### Example 2: Frontier Outpost

```yaml
Config: DA_SectorConfig_FrontierOutpost
Naming Theme: Frontier
Density Multiplier: 0.5

Objects:
  - Small Outpost (1, Independent)
  - Asteroids (5-10, scattered)
  - Debris (10-20, random)
  - Scout Ships (1-2, Independent)
```

### Example 3: Military Border Zone

```yaml
Config: DA_SectorConfig_MilitaryBorder
Naming Theme: Military
Density Multiplier: 1.5

Objects:
  - Military Stations (2-3, Military)
  - Patrol Ships (8-12, Military)
  - Defensive Platforms (5-8, Military, orbital)
  - Scanner Buoys (10-15, Military, grid)
```

### Example 4: Contested Territory

```yaml
Config: DA_SectorConfig_ContestedZone
Naming Theme: Descriptive
Create Zones: true
Num Zones: 3

Objects:
  - Station (Faction A, zone 1)
  - Station (Faction B, zone 2)
  - Combat Ships (Faction A, 5-8)
  - Combat Ships (Faction B, 5-8)
  - Debris Fields (10-20, from battles)
```

## Blueprint Customization

### Extending Generator Behavior

Override BlueprintNativeEvents in a Blueprint child of `ASectorGenerator`:

#### OnPreGeneration
Called before generation starts. Use for:
- Custom validation
- Preparing the scene
- Setting up prerequisites

#### OnPostGeneration
Called after generation completes. Use for:
- Post-processing objects
- Creating relationships
- Spawning additional content

#### OnObjectSpawned
Called each time an object spawns. Use for:
- Configuring individual objects
- Applying custom properties
- Creating object links

### Example Blueprint Extension

```
Event OnObjectSpawned
  |
  ├─ If Object Type == Station
  │  └─ Set Station Module Count
  │     └─ Add Docking Bays
  |
  ├─ If Object Type == Ship
  │  └─ Set Ship AI Behavior
  │     └─ Add to Faction Fleet
  |
  └─ If Object Type == Asteroid
     └─ Set Resource Type
        └─ Add Scannable Component
```

## Troubleshooting

### Generation Fails

**Problem**: GenerateSector returns false

**Solutions:**
1. Check `ValidateConfiguration()` errors in log
2. Ensure `TargetSector` is set
3. Ensure `GeneratorConfig` is set
4. Verify all Actor Classes in object definitions exist

### Objects Not Spawning

**Problem**: Generation completes but no objects appear

**Solutions:**
1. Check `MaxSpawnAttempts` is high enough
2. Reduce `MinObjectSpacing` if sector is crowded
3. Check object count ranges (Min/Max Count)
4. Enable `bShowDebugVisuals` to see attempted locations
5. Verify Actor Classes are valid Blueprints

### Name Collisions

**Problem**: "Name already exists" warnings

**Solution**: The generator automatically handles this by appending numbers or regenerating. This warning is informational only.

### Faction Not Applied

**Problem**: Spawned objects don't have faction ownership

**Solutions:**
1. Ensure actor implements `IFactionMember` interface
2. Check faction Data Asset is valid
3. Verify `SetFaction` function exists and works
4. Check log for faction assignment messages

### Performance Issues

**Problem**: Generation is slow or causes lag

**Solutions:**
1. Reduce `DensityMultiplier`
2. Reduce `MaxSpawnAttempts`
3. Use simpler distribution patterns (avoid Grid for large counts)
4. Split generation across multiple frames (Blueprint extension)
5. Generate in editor rather than at runtime

### Objects Overlapping

**Problem**: Objects spawn too close together

**Solutions:**
1. Increase `MinObjectSpacing`
2. Reduce object counts
3. Increase `MaxSpawnAttempts`
4. Use different distribution patterns

## Best Practices

1. **Start Small**: Begin with few object definitions, add more gradually
2. **Test Iteratively**: Generate, review, adjust config, regenerate
3. **Use Appropriate Patterns**: Match distribution to object type
4. **Set Reasonable Spacing**: Balance density with collision avoidance
5. **Use Seeds for Reproducibility**: Set specific seed when you find a good generation
6. **Preview Before Commit**: Use preview mode for testing
7. **Document Your Configs**: Add detailed descriptions to configs
8. **Create Config Templates**: Build library of tested configs for reuse

## Related Documentation

- `SectorMapBuildingGuide.md` - Creating sectors
- `FactionSetupGuide.md` - Faction system integration
- `SpaceshipGuide.md` - Ship Data Assets
- `ARCHITECTURE.md` - System architecture overview

---

**Last Updated**: 2025-12-19  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
