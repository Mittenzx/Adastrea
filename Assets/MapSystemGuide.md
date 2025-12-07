# Map System Guide

## Overview

The Adastrea map system provides players with comprehensive universe navigation through two complementary widgets:

1. **UniverseMapWidget** - Galaxy-level view showing all sectors
2. **SectorMapWidget** - Detailed view of individual sectors

Both widgets are fully Blueprint-exposed and designer-friendly.

## System Components

### UniverseMapWidget

**Purpose**: Display all sectors in the universe organized in a grid with exploration tracking.

**Key Features**:
- Automatic sector discovery system
- Grid-based universe organization
- Player location tracking
- Exploration progress statistics
- Sector selection and navigation

**Main Functions**:
- `InitializeUniverseMap()` - Scan world and build sector grid
- `RefreshUniverseMap()` - Update sector data
- `SetSelectedSector()` - Select a sector for viewing
- `GetPlayerCurrentSector()` - Find sector containing player
- `MarkSectorDiscovered()` - Track exploration progress
- `ToggleUniverseMapVisibility()` - Show/hide map

### SectorMapWidget

**Purpose**: Display detailed information about a specific sector.

**Key Features**:
- Sector boundaries and size display
- Object listing within sector
- Random spawn position generation
- Sector navigation helpers
- Integration with universe map

**Main Functions**:
- `SetTargetSector()` - Set sector to display
- `UpdateSectorInfo()` - Update sector display
- `GetNeighboringSectors()` - Find adjacent sectors
- `GetRandomSpawnPosition()` - Generate spawn locations
- `IsPositionInCurrentSector()` - Bounds checking

## Creating a Map Blueprint

### Step 1: Create Universe Map Widget Blueprint

1. In Content Browser, navigate to `Content/UI/`
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_UniverseMap`
4. Open the Blueprint
5. In Class Settings, set Parent Class to `UniverseMapWidget`

### Step 2: Design Universe Map Layout

Add these UI elements to your map:

**Required Elements**:
- Canvas Panel (root)
- Sector Grid Display (can be a custom widget or overlay)
- Selected Sector Info Panel
- Exploration Progress Bar
- Close Button

**Optional Elements**:
- Minimap in corner
- Legend/key for sector types
- Player location indicator
- Navigation controls
- Search/filter functionality

### Step 3: Bind Blueprint Events

Override these BlueprintNativeEvent functions:

```blueprint
Event UpdateUniverseGrid
    // Draw sector icons on grid
    // Update sector colors based on discovery
    // Show player position marker

Event OnSectorSelected
    // Update info panel with sector details
    // Highlight selected sector
    // Enable navigation button

Event OnMapOpened
    // Play open animation
    // Pause game (optional)
    // Refresh sector data

Event OnMapClosed
    // Play close animation
    // Resume game (optional)
    // Save map state
```

### Step 4: Create Sector Map Widget Blueprint

1. Create `WBP_SectorMap` similar to above
2. Set Parent Class to `SectorMapWidget`
3. Design detailed sector view layout

**Recommended Elements**:
- Sector name and description
- Object list (stations, ships, asteroids)
- Sector size and boundaries
- Distance to neighboring sectors
- Navigation options

## Integration with HUD

### Adding Map Toggle to Player Controller

1. Open your Player Controller Blueprint
2. Add Input Action for Map (default: M key)
3. Create Map Widget instance on Begin Play
4. Toggle map visibility on input

**Blueprint Example**:
```blueprint
Event BeginPlay
    Create Widget → WBP_UniverseMap
    Add to Viewport (Hidden)
    Save as MapWidget variable

Input Action Map Pressed
    Get MapWidget
    Toggle Universe Map Visibility
```

### Integrating with Enhanced Input System

If using Enhanced Input (recommended):

1. Create Input Action `IA_Map` in `Content/Input/`
2. Add to Input Mapping Context `IMC_Spaceship`
3. Bind M key to `IA_Map`
4. In Player Controller, bind `IA_Map` to map toggle function

## Sector Discovery System

### Automatic Discovery

The map system supports automatic sector discovery:

```cpp
// Configuration in UniverseMapWidget
bAutoDiscoverVisitedSectors = true;  // Discover when player enters
bShowUndiscoveredSectors = true;     // Show unknown sectors as blank
```

### Manual Discovery

To manually mark sectors discovered:

```blueprint
Get Universe Map Widget
Mark Sector Discovered (Sector)
```

### Checking Discovery Status

```blueprint
Get Universe Map Widget
Is Sector Discovered → Boolean
```

## Grid System

### Grid Coordinates

Sectors are organized in a 3D grid based on world position:

- Each grid cell = 200km (one sector size)
- Grid coordinates calculated from world position
- Formula: `GridCoord = Round(WorldPosition / SectorSize)`

### Grid Position Examples

```
Sector at (0, 0, 0)          → Grid (0, 0, 0)
Sector at (20000000, 0, 0)   → Grid (1, 0, 0)
Sector at (-20000000, 0, 0)  → Grid (-1, 0, 0)
```

## Blueprint API Reference

### UniverseMapWidget Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `GetAllSectors()` | Get all sectors in world | Array of ASpaceSectorMap |
| `GetSelectedSector()` | Get currently selected sector | ASpaceSectorMap |
| `GetPlayerCurrentSector()` | Find sector containing player | ASpaceSectorMap |
| `GetTotalSectorCount()` | Total number of sectors | Integer |
| `GetDiscoveredSectorCount()` | Number of discovered sectors | Integer |
| `GetExplorationProgress()` | Exploration percentage | Float (0.0-1.0) |
| `GetSectorAtGridCoordinates()` | Find sector at grid position | ASpaceSectorMap |

### SectorMapWidget Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `SetTargetSector()` | Set sector to display | Void |
| `GetTargetSector()` | Get current sector | ASpaceSectorMap |
| `RefreshSectorData()` | Update sector info | Void |
| `GetAllSectors()` | Get all sectors in world | Array of ASpaceSectorMap |
| `GetNeighboringSectors()` | Get adjacent sectors | Array of ASpaceSectorMap |
| `GetRandomSpawnPosition()` | Get spawn point in sector | Vector |
| `GetDistanceToSector()` | Distance to other sector | Float |

## Common Use Cases

### Use Case 1: Fast Travel

```blueprint
Event OnSectorSelected (Sector)
    If Sector Is Discovered
        Show "Travel Here" Button
        On Button Clicked:
            Teleport Player to Sector
            Update Current Sector Display
```

### Use Case 2: Quest Markers

```blueprint
Event Update Universe Grid
    For Each Quest Objective
        If Objective Has Sector Location
            Draw Quest Marker at Sector Grid Position
```

### Use Case 3: Trade Route Planning

```blueprint
Get Universe Map Widget
Get All Sectors
For Each Sector
    If Sector Has Trading Station
        Mark as Trade Hub
        Calculate Distance to Player Sector
        Display Trade Route Line
```

### Use Case 4: Exploration Tracking

```blueprint
Event Tick
    Get Player Current Sector
    If Sector Changed
        Mark Sector Discovered
        Update Exploration Progress
        Show Discovery Notification
```

## Customization Options

### Visual Themes

You can customize map appearance:
- Sector icon styles (discovered, undiscovered, selected)
- Grid line colors and thickness
- Background starfield or nebula
- Font sizes and colors
- Animation effects

### Gameplay Features

Add custom functionality:
- Sector difficulty ratings
- Resource richness indicators
- Faction territory colors
- Danger zones (hostile sectors)
- Hidden/secret sectors
- Fog of war effects

## Performance Considerations

### Optimization Tips

1. **Update Frequency**: Only refresh map when opened or sector changes
2. **Visible Sectors**: Only draw sectors within view range
3. **LOD System**: Use different detail levels based on zoom
4. **Caching**: Cache sector grid calculations

### Recommended Limits

- Maximum sectors: 100-200 for good performance
- Update interval: 0.5-1.0 seconds when map is open
- Grid cell culling: Only render visible 50x50 cell area

## Debugging

### Common Issues

**Map shows no sectors**:
- Verify ASpaceSectorMap actors exist in world
- Check `FindAllSectorsInWorld()` returns data
- Ensure InitializeUniverseMap is called

**Sectors not discovered**:
- Check `bAutoDiscoverVisitedSectors` is true
- Verify player is actually inside sector bounds
- Call `MarkSectorDiscovered()` manually if needed

**Grid coordinates wrong**:
- Verify sector actors are positioned on sector-size boundaries
- Check SectorSize constant (should be 20,000,000)
- Use Debug Draw to visualize sector bounds

### Debug Commands

Add these for testing:

```blueprint
// Discover all sectors
Console Command: DiscoverAllSectors

// Show sector bounds
Console Command: ShowSectorBounds

// Toggle sector grid overlay
Console Command: ShowSectorGrid
```

## Integration with Other Systems

### With Trading System

```blueprint
On Universe Map Sector Selected
    Get Sector Trading Stations
    Display Trade Prices
    Show Supply/Demand Info
```

### With Quest System

```blueprint
On Quest Accepted
    Get Quest Objective Sector
    Mark on Universe Map
    Calculate Distance from Player
```

### With Faction System

```blueprint
On Update Universe Grid
    For Each Sector
        Get Controlling Faction
        Color Sector by Faction
        Show Faction Territory
```

## Future Enhancements

Potential additions to the map system:

1. **Warp Gate Network**: Show connections between sectors
2. **Sector Filters**: Filter by resources, danger, factions
3. **Route Planning**: Calculate optimal travel paths
4. **Sector Notes**: Let players add custom notes to sectors
5. **Shared Maps**: Multiplayer map data sharing
6. **Procedural Sectors**: Dynamic sector generation
7. **3D Galaxy View**: Rotate and zoom 3D universe
8. **Time-Based Events**: Show temporary events on map

## Additional Resources

- **SpaceSectorMap.h** - Sector actor class documentation
- **SectorMapWidget.h** - Sector map widget API
- **UniverseMapWidget.h** - Universe map widget API
- **EnhancedInputImplementation.md** - Input binding guide

## Examples

See `Content/UI/Examples/` for sample map implementations:
- `WBP_SimpleMap` - Basic 2D grid map
- `WBP_DetailedMap` - Advanced map with filters
- `WBP_MinimapHUD` - Minimap overlay for HUD

---

**Last Updated**: 2025-12-07  
**Version**: 1.0  
**Author**: Adastrea Development Team
