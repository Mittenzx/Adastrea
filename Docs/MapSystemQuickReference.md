# Map System Quick Reference

## At a Glance

| Component | Purpose | Key Functions |
|-----------|---------|---------------|
| **SpaceSectorMap** | 200km sector actor | Grid coords, actor tracking, neighbors |
| **UniverseMapWidget** | Universe grid UI | Pathfinding, bookmarks, discovery |
| **SectorMapWidget** | Sector detail UI | Object list, navigation, statistics |

## Common Tasks

### Place a Sector in Editor

```
1. Drag SpaceSectorMap into level
2. Position at (0,0,0) or multiples of 20,000,000
3. Set SectorName and Description
4. Sector auto-validates
```

### Open Universe Map

```cpp
// Blueprint
UniverseMapWidget->ToggleUniverseMapVisibility(true);

// C++
UniverseMapWidget->ToggleUniverseMapVisibility(true);
SetInputMode(FInputModeUIOnly());
```

### Find Path Between Sectors

```cpp
TArray<ASpaceSectorMap*> Path = UniverseMapWidget->FindPathBetweenSectors(
    StartSector,
    EndSector
);
```

### Track Objects in Sector

```cpp
// Get all actors
TArray<AActor*> Actors = Sector->GetActorsInSector();

// Get specific type
TArray<AActor*> Ships = Sector->GetActorsInSector(ASpaceship::StaticClass());

// Just count (faster)
int32 Count = Sector->GetActorCountInSector();
```

### Bookmark a Sector

```cpp
UniverseMapWidget->AddSectorBookmark(
    Sector,
    FText::FromString("My Favorite Place")
);
```

### Check Player Location

```cpp
// Is player in this sector?
bool bInSector = SectorMapWidget->IsPlayerInSector();

// Distance to sector center
float Distance = SectorMapWidget->GetPlayerDistanceToSectorCenter();

// Direction to sector
FVector Direction = SectorMapWidget->GetNavigationDirectionToCenter();
```

## SpaceSectorMap Functions

### Position & Bounds
- `GetSectorCenter()` - Center position (FVector)
- `GetSectorBounds()` - Bounds as box (FBox)
- `GetSectorHalfSize()` - 100km from center (float)
- `GetGridCoordinates()` - Grid position (FIntVector)

### Actor Queries
- `GetActorsInSector(Class)` - All actors in bounds (TArray)
- `GetActorCountInSector(Class)` - Count only (int32)
- `IsPositionInSector(Pos)` - Is position inside? (bool)
- `GetRandomPositionInSector()` - Random spawn point (FVector)

### Navigation
- `GetNeighboringSectors()` - Adjacent sectors (TArray)
- `GetDistanceToSector(Other)` - Distance to sector (float)

### Debug
- `ValidateSectorConfiguration()` - Check for issues (bool)
- `GetDebugInfo()` - Full sector info (FString)

## UniverseMapWidget Functions

### Initialization
- `InitializeUniverseMap()` - Setup map, find sectors
- `RefreshUniverseMap()` - Rescan for new sectors
- `UpdateUniverseGrid()` - Refresh display

### Selection
- `SetSelectedSector(Sector)` - Select a sector
- `GetSelectedSector()` - Get current selection
- `GetAllSectors()` - All sectors in world

### Discovery
- `IsSectorDiscovered(Sector)` - Is discovered? (bool)
- `MarkSectorDiscovered(Sector)` - Mark as discovered
- `GetDiscoveredSectorCount()` - Number discovered (int32)
- `GetExplorationProgress()` - Progress 0.0-1.0 (float)

### Navigation
- `FindPathBetweenSectors(Start, End)` - A* pathfinding (TArray)
- `GetGridDistanceBetweenSectors(A, B)` - Manhattan distance (int32)
- `GetPlayerCurrentSector()` - Player's sector (ASpaceSectorMap*)

### Bookmarks
- `AddSectorBookmark(Sector, Name)` - Add bookmark
- `RemoveSectorBookmark(Sector)` - Remove bookmark
- `IsSectorBookmarked(Sector)` - Is bookmarked? (bool)
- `GetBookmarkedSectors()` - All bookmarks (TArray)

### Search
- `FilterSectorsByName(Text)` - Search by name (TArray)

### Display
- `ToggleUniverseMapVisibility(Visible)` - Show/hide map

## SectorMapWidget Functions

### Setup
- `SetTargetSector(Sector)` - Set sector to display
- `GetTargetSector()` - Get current sector
- `RefreshSectorData()` - Update from sector
- `ClearSectorInfo()` - Clear display

### Info Display
- `UpdateSectorInfo(Info)` - Update UI with sector info
- `UpdateObjectList(Names, Count)` - Update object list
- `GetSectorStatistics()` - Get debug info (FString)

### Object Tracking
- `UpdateObjectTracking()` - Refresh actor list
- `GetRandomSpawnPosition()` - Random point (FVector)
- `IsPositionInCurrentSector(Pos)` - In bounds? (bool)

### Player Navigation
- `GetPlayerDistanceToSectorCenter()` - Distance (float)
- `IsPlayerInSector()` - Player inside? (bool)
- `GetNavigationDirectionToCenter()` - Direction (FVector)
- `CalculateTravelTimeToSector(Speed)` - ETA (float seconds)

### Neighbors
- `GetAllSectors()` - All sectors (TArray)
- `GetNeighboringSectors()` - Adjacent sectors (TArray)
- `GetDistanceToSector(Other)` - Distance (float)

### Display
- `ToggleSectorMapVisibility(Visible)` - Show/hide

## Configuration Properties

### SpaceSectorMap
- `SectorName` (FText) - Display name
- `Description` (FText) - Description text
- `SectorMarker` (UBillboardComponent) - Editor icon

### UniverseMapWidget
- `bAutoDiscoverVisitedSectors` (bool) - Auto-discover on entry
- `bShowUndiscoveredSectors` (bool) - Show unknown sectors
- `bAutoCreateMissingWidgets` (bool) - Create UI if not bound

### SectorMapWidget
- `bAutoCreateMissingWidgets` (bool) - Create UI if not bound

## Events (Override in Blueprint)

### UniverseMapWidget
- `OnMapOpened` - Map opened
- `OnMapClosed` - Map closed
- `OnSectorSelected(Sector)` - Sector clicked
- `UpdateUniverseGrid` - Grid needs update

### SectorMapWidget
- `OnSectorSelected(Sector)` - Sector selected
- `UpdateSectorInfo(Info)` - Info changed
- `UpdateObjectList(Names, Count)` - Objects changed
- `ClearSectorInfo` - Cleared

## Size Reference

```
Sector Size:     200 km = 200,000 m = 20,000,000 cm (Unreal Units)
Half Size:       100 km = 100,000 m = 10,000,000 cm
Neighbor Range:  300 km = 300,000 m = 30,000,000 cm (1.5x sector size)
```

## Grid Coordinates

```
World Position          Grid Coordinates
(0, 0, 0)          →    (0, 0, 0)
(20000000, 0, 0)   →    (1, 0, 0)
(0, 20000000, 0)   →    (0, 1, 0)
(-20000000, 0, 0)  →    (-1, 0, 0)
(40000000, 20000000, 0) → (2, 1, 0)
```

## Common Patterns

### Spawn Object in Random Sector

```cpp
// Get random sector
TArray<ASpaceSectorMap*> AllSectors = UniverseMapWidget->GetAllSectors();
ASpaceSectorMap* RandomSector = AllSectors[FMath::RandRange(0, AllSectors.Num()-1)];

// Get random position in sector
FVector SpawnPos = RandomSector->GetRandomPositionInSector();

// Spawn actor
GetWorld()->SpawnActor<AActor>(ActorClass, SpawnPos, FRotator::ZeroRotator);
```

### Navigate Player to Sector

```cpp
// Find path
TArray<ASpaceSectorMap*> Path = UniverseMapWidget->FindPathBetweenSectors(
    CurrentSector,
    TargetSector
);

if (Path.Num() > 0)
{
    // Get first waypoint (next sector in path)
    ASpaceSectorMap* NextSector = Path[0];
    FVector WaypointPos = NextSector->GetSectorCenter();
    
    // Set navigation target
    SetNavigationTarget(WaypointPos);
}
```

### Track Sector Entry/Exit

```cpp
// In player ship Tick
void APlayerShip::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Find current sector
    TArray<ASpaceSectorMap*> AllSectors = GetAllSectors();
    ASpaceSectorMap* CurrentSector = nullptr;
    
    for (ASpaceSectorMap* Sector : AllSectors)
    {
        if (Sector->IsPositionInSector(GetActorLocation()))
        {
            CurrentSector = Sector;
            break;
        }
    }
    
    // Check for sector change
    if (CurrentSector != LastSector)
    {
        if (LastSector)
        {
            OnExitSector(LastSector);
        }
        if (CurrentSector)
        {
            OnEnterSector(CurrentSector);
        }
        LastSector = CurrentSector;
    }
}
```

### Display Sector Info HUD

```cpp
// In HUD
void AMyHUD::DrawSectorInfo()
{
    // Get player's current sector
    ASpaceSectorMap* Sector = FindPlayerSector();
    
    if (Sector)
    {
        // Display sector name
        FString SectorText = FString::Printf(
            TEXT("Sector: %s\nObjects: %d"),
            *Sector->SectorName.ToString(),
            Sector->GetActorCountInSector()
        );
        
        DrawText(SectorText, FColor::White, 50, 50);
    }
}
```

## Performance Tips

1. **Cache sector references** - Don't call `GetAllSectors()` every frame
2. **Use actor count** - Faster than getting full actor array
3. **Update periodically** - Don't refresh maps every frame (use timers)
4. **Neighbor caching** - Neighbors are cached automatically
5. **Filter by class** - More efficient than filtering after query

## Debugging

### Enable Logging

```cpp
// In DefaultEngine.ini
[Core.Log]
LogAdastrea=Verbose

// Or at runtime
UE_LOG(LogAdastrea, Log, TEXT("Your message"));
```

### Console Commands

```
// Show sector bounds in editor
ShowDebug COLLISION

// Display actor locations
stat LEVELS
```

### Visual Debugging

```cpp
// Draw sector bounds
FBox Bounds = Sector->GetSectorBounds();
DrawDebugBox(
    GetWorld(),
    Bounds.GetCenter(),
    Bounds.GetExtent(),
    FColor::Green,
    false,
    5.0f
);

// Draw path between sectors
for (int32 i = 0; i < Path.Num() - 1; ++i)
{
    DrawDebugLine(
        GetWorld(),
        Path[i]->GetSectorCenter(),
        Path[i+1]->GetSectorCenter(),
        FColor::Yellow,
        false,
        5.0f,
        0,
        100.0f
    );
}
```

## See Also

- [MapSystemGuide.md](MapSystemGuide.md) - Complete documentation
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Project architecture
- [WIDGET_IMPLEMENTATION_SUMMARY.md](../WIDGET_IMPLEMENTATION_SUMMARY.md) - Widget patterns
