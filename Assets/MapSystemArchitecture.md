# Map System Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Player Controller                         │
│  • Input Handling (M key)                                   │
│  • Map Toggle Control                                        │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   │ Creates & Manages
                   ▼
┌─────────────────────────────────────────────────────────────┐
│              UniverseMapWidget (WBP_UniverseMap)            │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Universe View                                       │    │
│  │  • Grid-based sector display                       │    │
│  │  • Player location indicator                       │    │
│  │  • Exploration progress (X% discovered)            │    │
│  │  • Sector selection                                │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  State:                                                      │
│  • AllSectors: Array<ASpaceSectorMap>                      │
│  • SelectedSector: ASpaceSectorMap*                         │
│  • DiscoveredSectors: Set<ASpaceSectorMap>                 │
│  • UniverseSectorInfo: Array<FUniverseSectorInfo>          │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   │ Can integrate with
                   ▼
┌─────────────────────────────────────────────────────────────┐
│              SectorMapWidget (WBP_SectorMap)                │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Detailed Sector View                               │    │
│  │  • Sector name & description                       │    │
│  │  • Sector boundaries (200km x 200km)               │    │
│  │  • Object list (stations, ships, asteroids)        │    │
│  │  • Neighboring sectors                             │    │
│  │  • Navigation options                              │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  State:                                                      │
│  • CurrentSector: ASpaceSectorMap*                          │
│  • CurrentSectorInfo: FSectorDisplayInfo                    │
│  • SectorObjectNames: Array<FText>                          │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   │ References
                   ▼
┌─────────────────────────────────────────────────────────────┐
│                 ASpaceSectorMap Actors                       │
│  • Placed in world at grid-aligned positions                │
│  • Each represents 200km x 200km x 200km space             │
│  • Contains SectorName, Description, Position               │
│  • Provides bounds checking and random position generation  │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow

### Map Opening Sequence

```
1. Player presses M key
   ↓
2. Player Controller receives input
   ↓
3. Toggle UniverseMapWidget visibility
   ↓
4. UniverseMapWidget::OnMapOpened()
   ↓
5. RefreshUniverseMap()
   ├─ FindAllSectorsInWorld()
   ├─ BuildUniverseSectorGrid()
   └─ UpdateUniverseGrid()
   ↓
6. Get player's current sector
   ↓
7. Auto-select and discover player sector
   ↓
8. Display universe grid with sectors
```

### Sector Selection Flow

```
1. Player clicks sector icon in universe map
   ↓
2. OnSectorSelected(ASpaceSectorMap* Sector)
   ↓
3. SetSelectedSector(Sector)
   ├─ Update selection state in UniverseSectorInfo
   └─ Highlight selected sector in UI
   ↓
4. Optional: Show detailed SectorMapWidget
   ↓
5. SectorMapWidget::SetTargetSector(Sector)
   ├─ BuildSectorDisplayInfo()
   └─ UpdateSectorInfo()
   ↓
6. Display sector details panel
```

### Discovery Flow

```
1. Player moves spaceship
   ↓
2. Check if in new sector (per frame or periodic)
   ↓
3. GetPlayerCurrentSector()
   ↓
4. If bAutoDiscoverVisitedSectors == true
   ↓
5. MarkSectorDiscovered(CurrentSector)
   ├─ Add to DiscoveredSectors set
   ├─ Update UniverseSectorInfo[i].bIsDiscovered
   └─ UpdateUniverseGrid()
   ↓
6. Show discovery notification (optional)
```

## Class Hierarchy

```
UUserWidget (Unreal Base Class)
├─ UUniverseMapWidget
│  └─ WBP_UniverseMap (Blueprint)
│     └─ Custom UI Design
└─ USectorMapWidget
   └─ WBP_SectorMap (Blueprint)
      └─ Custom UI Design

AActor (Unreal Base Class)
└─ ASpaceSectorMap
   └─ BP_SectorMarker (Optional Blueprint variant)
```

## Key Data Structures

### FUniverseSectorInfo

```cpp
struct FUniverseSectorInfo
{
    ASpaceSectorMap* Sector;       // Reference to sector actor
    FIntVector GridCoordinates;    // Grid position (X, Y, Z)
    bool bIsDiscovered;            // Exploration state
    bool bIsSelected;              // UI selection state
};
```

### FSectorDisplayInfo

```cpp
struct FSectorDisplayInfo
{
    FText SectorName;              // Display name
    FText Description;             // Sector description
    FVector SectorCenter;          // World position
    float SectorSize;              // Boundary size (200km)
    int32 ObjectCount;             // Number of objects
};
```

## Grid Coordinate System

```
Y-Axis (North)
     ▲
     │  (0, 1, 0)
     │     │
(-1,0,0)───┼───(1, 0, 0) ──► X-Axis (East)
     │     │
     │  (0,-1, 0)
     │
     ▼
  Z-Axis (Down)

Each grid cell = 200km (20,000,000 Unreal Units)
Grid coordinates calculated from world position:
  GridX = Round(WorldX / 20000000)
  GridY = Round(WorldY / 20000000)
  GridZ = Round(WorldZ / 20000000)
```

## Component Interaction

```
┌──────────────┐     Creates      ┌──────────────┐
│   Player     │ ──────────────►  │ Universe Map │
│  Controller  │                   │    Widget    │
└──────┬───────┘                   └──────┬───────┘
       │                                  │
       │ Possesses                        │ Queries
       │                                  │
       ▼                                  ▼
┌──────────────┐                   ┌──────────────┐
│  Spaceship   │◄──────────────────│    World     │
│    Pawn      │   Gets position   │              │
└──────────────┘                   └──────┬───────┘
                                          │
                                          │ Contains
                                          ▼
                                   ┌──────────────┐
                                   │ SpaceSector  │
                                   │  Map Actors  │
                                   └──────────────┘
```

## Integration Points

### With Quest System

```
Quest Created
    ↓
Get Objective Sector
    ↓
UniverseMapWidget::SetSelectedSector()
    ↓
Show quest marker on map
```

### With Trading System

```
Open Universe Map
    ↓
For each discovered sector:
    Get trading stations
    Calculate trade opportunities
    Display trade route indicators
```

### With Faction System

```
UpdateUniverseGrid()
    ↓
For each sector:
    Get controlling faction
    Apply faction color to sector icon
    Show faction territory boundaries
```

## Performance Considerations

### Optimization Strategies

1. **Lazy Loading**: Only load sector details when selected
2. **Update Throttling**: Limit grid updates to 0.5-1.0 seconds
3. **Culling**: Only render visible sectors in viewport
4. **Caching**: Cache grid calculations until sectors move
5. **Object Pooling**: Reuse UI elements for sector icons

### Memory Management

```
Event OnMapOpened:
    • Find all sectors (lightweight)
    • Build grid (O(n) where n = sector count)
    • Create UI elements as needed

Event OnMapClosed:
    • Keep sector references (small memory)
    • Optional: Clear UI element pool
    • Maintain discovery state
```

## Extension Points

### Custom Behaviors

All events are BlueprintNativeEvent, allowing override:

```
OnSectorSelected(Sector)
    → Add fast travel button
    → Show sector population stats
    → Display faction reputation

OnMapOpened()
    → Pause game time
    → Play sound effect
    → Show tutorial hint

UpdateUniverseGrid()
    → Custom sector icon rendering
    → Draw connections between sectors
    → Add visual effects
```

## Testing Strategy

### Unit Tests

1. Grid coordinate calculation
2. Sector discovery tracking
3. Neighboring sector finding
4. Distance calculations

### Integration Tests

1. Map opens correctly
2. Sectors display in grid
3. Player sector auto-selected
4. Discovery persists

### Performance Tests

1. 100+ sectors display smoothly
2. Grid updates within frame budget
3. Memory usage stays reasonable

---

**Last Updated**: 2025-12-07  
**Version**: 1.0  
**See Also**: MapSystemGuide.md, MapSystemQuickStart.md
