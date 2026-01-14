# Adastrea Map System Guide

## Overview

The Adastrea Map System provides a comprehensive framework for creating, managing, and navigating large-scale space sectors in an open-world environment. The system consists of three main components:

1. **SpaceSectorMap** - Core C++ actor representing individual 200km x 200km sectors
2. **UniverseMapWidget** - UI widget for displaying and navigating the universe grid
3. **SectorMapWidget** - UI widget for viewing detailed sector information

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Universe Map Widget                      │
│  - Grid display of all sectors                             │
│  - Pathfinding between sectors                             │
│  - Bookmark/waypoint system                                │
│  - Discovery tracking                                       │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   │ displays multiple
                   ▼
          ┌────────────────────┐
          │  SpaceSectorMap    │◄────────┐
          │  Actor             │         │
          │  - 200km x 200km   │  references
          │  - Grid coords     │         │
          │  - Actor tracking  │         │
          │  - Validation      │         │
          └─────────┬──────────┘         │
                    │                    │
                    │ selected by        │
                    ▼                    │
          ┌─────────────────────┐        │
          │  Sector Map Widget  │────────┘
          │  - Detailed view    │
          │  - Object tracking  │
          │  - Navigation aids  │
          │  - Statistics       │
          └─────────────────────┘
```

## SpaceSectorMap Actor

### Purpose

The `ASpaceSectorMap` actor represents a single sector in the game world. Each sector is a 200km x 200km x 200km cubic volume of space that can contain stations, ships, asteroids, and other objects.

### Key Features

- **Grid-based positioning** - Sectors snap to a universe grid for organized layout
- **Actor tracking** - Query all actors within sector bounds
- **Neighbor detection** - Find adjacent sectors automatically
- **Validation** - Editor-time checks for overlapping sectors
- **Performance optimization** - Cached neighbor lists with dirty flag system

### Usage in Editor

1. **Placing a Sector**
   ```
   - Drag ASpaceSectorMap into your level
   - Position it on the grid (multiples of 20,000,000 units recommended)
   - Set the SectorName and Description properties
   - The sector automatically validates its configuration
   ```

2. **Sector Properties**
   - `SectorName` (FText) - Display name for the sector
   - `Description` (FText) - Optional description of sector contents/theme
   - `SectorMarker` (UBillboardComponent) - Editor visualization

3. **Editor Validation**
   The sector automatically checks for:
   - Overlapping with other sectors
   - Valid sector name
   - Proper grid alignment

### Blueprint Functions

#### Basic Information

```cpp
// Get the sector's center position
FVector Center = SectorActor->GetSectorCenter();

// Get grid coordinates
FIntVector GridCoords = SectorActor->GetGridCoordinates();

// Get sector bounds as a box
FBox Bounds = SectorActor->GetSectorBounds();

// Get half-size (100km from center)
float HalfSize = SectorActor->GetSectorHalfSize();
```

#### Actor Tracking

```cpp
// Get all actors in sector
TArray<AActor*> AllActors = SectorActor->GetActorsInSector();

// Get specific type of actors
TArray<AActor*> Ships = SectorActor->GetActorsInSector(ASpaceship::StaticClass());

// Get actor count (optimized, doesn't build array)
int32 Count = SectorActor->GetActorCountInSector();

// Check if position is in sector
bool bIsInside = SectorActor->IsPositionInSector(WorldPosition);

// Get random position for spawning
FVector SpawnPos = SectorActor->GetRandomPositionInSector();
```

#### Navigation

```cpp
// Find neighboring sectors
TArray<ASpaceSectorMap*> Neighbors = SectorActor->GetNeighboringSectors();

// Calculate distance to another sector
float Distance = SectorActor->GetDistanceToSector(OtherSector);

// Validate sector configuration
bool bIsValid = SectorActor->ValidateSectorConfiguration();

// Get debug information
FString DebugInfo = SectorActor->GetDebugInfo();
```

### C++ Integration

```cpp
// Example: Spawn AI ships in a sector
void AGameMode::PopulateSectorWithAI(ASpaceSectorMap* Sector, int32 ShipCount)
{
    if (!Sector)
    {
        return;
    }
    
    for (int32 i = 0; i < ShipCount; ++i)
    {
        // Get random position in sector
        FVector SpawnPosition = Sector->GetRandomPositionInSector();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        // Spawn AI ship
        AActor* AIShip = GetWorld()->SpawnActor<ASpaceship>(
            AIShipClass,
            SpawnPosition,
            SpawnRotation
        );
    }
    
    UE_LOG(LogGame, Log, TEXT("Spawned %d ships in sector '%s'"), 
        ShipCount, *Sector->SectorName.ToString());
}
```

## UniverseMapWidget

### Purpose

The `UUniverseMapWidget` provides a comprehensive universe map interface showing all sectors in a grid layout with discovery tracking, pathfinding, and bookmarks.

### Key Features

- **Sector Grid Display** - Visual representation of all sectors in universe
- **Discovery System** - Track which sectors the player has visited
- **Pathfinding** - A* algorithm to find routes between sectors
- **Bookmarks** - Mark important sectors with custom names
- **Search & Filter** - Find sectors by name
- **Exploration Progress** - Track percentage of universe discovered

### Blueprint Setup

1. **Create Widget Blueprint**
   ```
   - Create new Widget Blueprint (WBP_UniverseMap)
   - Parent Class: UniverseMapWidget
   - Design your UI layout
   ```

2. **Optional Widget Bindings**
   The widget can auto-create UI elements, but you can bind these for custom styling:
   - `SectorGridPanel` (UCanvasPanel) - Container for sector icons
   - `Text_SectorName` (UTextBlock) - Selected sector name
   - `Text_SectorDescription` (UTextBlock) - Selected sector description
   - `ProgressBar_Exploration` (UProgressBar) - Exploration progress
   - `Text_ExplorationPercent` (UTextBlock) - Exploration percentage
   - `Button_Close` (UButton) - Close map button

3. **Configuration Properties**
   - `bAutoDiscoverVisitedSectors` (bool) - Automatically discover sectors player enters
   - `bShowUndiscoveredSectors` (bool) - Show undiscovered sectors as blank/unknown
   - `bAutoCreateMissingWidgets` (bool) - Create UI programmatically if not bound

### Using the Widget

#### Opening the Map

```cpp
// In your HUD or player controller
void AMyPlayerController::OpenUniverseMap()
{
    if (!UniverseMapWidget)
    {
        // Create widget
        UniverseMapWidget = CreateWidget<UUniverseMapWidget>(
            this, 
            UniverseMapWidgetClass
        );
    }
    
    if (UniverseMapWidget)
    {
        UniverseMapWidget->AddToViewport();
        UniverseMapWidget->ToggleUniverseMapVisibility(true);
        
        // Optionally pause game
        SetPause(true);
        
        // Show mouse cursor
        bShowMouseCursor = true;
        SetInputMode(FInputModeGameAndUI());
    }
}
```

#### Blueprint Functions

```cpp
// Initialize the map
UniverseMapWidget->InitializeUniverseMap();

// Refresh data (rescan for sectors)
UniverseMapWidget->RefreshUniverseMap();

// Select a sector
UniverseMapWidget->SetSelectedSector(SectorActor);

// Get selected sector
ASpaceSectorMap* Selected = UniverseMapWidget->GetSelectedSector();

// Get all sectors
TArray<ASpaceSectorMap*> AllSectors = UniverseMapWidget->GetAllSectors();

// Get sector at grid position
ASpaceSectorMap* Sector = UniverseMapWidget->GetSectorAtGridCoordinates(FIntVector(1, 2, 0));

// Get player's current sector
ASpaceSectorMap* PlayerSector = UniverseMapWidget->GetPlayerCurrentSector();
```

#### Discovery System

```cpp
// Check if sector is discovered
bool bDiscovered = UniverseMapWidget->IsSectorDiscovered(SectorActor);

// Mark sector as discovered
UniverseMapWidget->MarkSectorDiscovered(SectorActor);

// Get exploration statistics
int32 TotalSectors = UniverseMapWidget->GetTotalSectorCount();
int32 DiscoveredSectors = UniverseMapWidget->GetDiscoveredSectorCount();
float Progress = UniverseMapWidget->GetExplorationProgress(); // 0.0 to 1.0
```

#### Pathfinding

```cpp
// Find path between two sectors
TArray<ASpaceSectorMap*> Path = UniverseMapWidget->FindPathBetweenSectors(
    StartSector,
    EndSector
);

if (Path.Num() > 0)
{
    UE_LOG(LogTemp, Log, TEXT("Path found with %d sectors"), Path.Num());
    
    // Display path on map or guide player
    for (ASpaceSectorMap* SectorInPath : Path)
    {
        // Highlight sector, create waypoints, etc.
    }
}

// Get grid distance between sectors
int32 GridDistance = UniverseMapWidget->GetGridDistanceBetweenSectors(
    SectorA,
    SectorB
);
```

#### Bookmarks

```cpp
// Add bookmark to a sector
UniverseMapWidget->AddSectorBookmark(
    SectorActor,
    FText::FromString("Trade Hub")
);

// Remove bookmark
UniverseMapWidget->RemoveSectorBookmark(SectorActor);

// Check if bookmarked
bool bIsBookmarked = UniverseMapWidget->IsSectorBookmarked(SectorActor);

// Get all bookmarked sectors
TArray<ASpaceSectorMap*> Bookmarks = UniverseMapWidget->GetBookmarkedSectors();
```

#### Search

```cpp
// Filter sectors by name
TArray<ASpaceSectorMap*> Results = UniverseMapWidget->FilterSectorsByName("Trading");

// Display search results
for (ASpaceSectorMap* Result : Results)
{
    UE_LOG(LogTemp, Log, TEXT("Found sector: %s"), *Result->SectorName.ToString());
}
```

### Events (BlueprintNativeEvent)

Override these in Blueprint for custom behavior:

```cpp
// Called when map is initialized
void OnMapOpened_Implementation();

// Called when map is closed
void OnMapClosed_Implementation();

// Called when a sector is selected
void OnSectorSelected_Implementation(ASpaceSectorMap* Sector);

// Update grid display (called automatically)
void UpdateUniverseGrid_Implementation();
```

## SectorMapWidget

### Purpose

The `USectorMapWidget` provides a detailed view of a single sector, showing objects within it, navigation information, and statistics.

### Key Features

- **Object Tracking** - List all actors in the sector
- **Player Integration** - Show player position relative to sector
- **Navigation Aids** - Direction and distance to sector center
- **Statistics Display** - Detailed sector information
- **Travel Time Estimation** - Calculate journey time to sector

### Blueprint Setup

1. **Create Widget Blueprint**
   ```
   - Create new Widget Blueprint (WBP_SectorMap)
   - Parent Class: SectorMapWidget
   - Design your UI layout
   ```

2. **Optional Widget Bindings**
   - `Text_SectorName` (UTextBlock) - Sector name
   - `Text_SectorDescription` (UTextBlock) - Sector description
   - `Text_ObjectCount` (UTextBlock) - Number of objects in sector
   - `ObjectListScrollBox` (UScrollBox) - List of sector objects

3. **Configuration Properties**
   - `bAutoCreateMissingWidgets` (bool) - Auto-create UI if not bound

### Using the Widget

#### Basic Usage

```cpp
// Set the sector to display
SectorMapWidget->SetTargetSector(SectorActor);

// Refresh sector data
SectorMapWidget->RefreshSectorData();

// Clear the display
SectorMapWidget->ClearSectorInfo();

// Toggle visibility
SectorMapWidget->ToggleSectorMapVisibility(true);
```

#### Navigation Functions

```cpp
// Get player's distance to sector center
float Distance = SectorMapWidget->GetPlayerDistanceToSectorCenter();

// Check if player is in this sector
bool bPlayerInside = SectorMapWidget->IsPlayerInSector();

// Get direction to sector center
FVector Direction = SectorMapWidget->GetNavigationDirectionToCenter();

// Calculate travel time at a given speed
float TravelSpeed = 50000.0f; // 500 m/s
float TravelTime = SectorMapWidget->CalculateTravelTimeToSector(TravelSpeed);

// Display estimated time
int32 Minutes = FMath::FloorToInt(TravelTime / 60.0f);
int32 Seconds = FMath::FloorToInt(TravelTime) % 60;
```

#### Object Tracking

```cpp
// Update object tracking
SectorMapWidget->UpdateObjectTracking();

// Get sector statistics
FString Stats = SectorMapWidget->GetSectorStatistics();
UE_LOG(LogTemp, Log, TEXT("Sector Stats:\n%s"), *Stats);

// Get random spawn position
FVector SpawnPos = SectorMapWidget->GetRandomSpawnPosition();

// Check if position is in sector
bool bInBounds = SectorMapWidget->IsPositionInCurrentSector(WorldPosition);
```

#### Querying Neighboring Sectors

```cpp
// Get neighboring sectors
TArray<ASpaceSectorMap*> Neighbors = SectorMapWidget->GetNeighboringSectors();

// Calculate distance to another sector
float DistanceToOther = SectorMapWidget->GetDistanceToSector(OtherSector);
```

## Best Practices

### Sector Layout

1. **Grid Alignment**
   - Place sectors on multiples of 20,000,000 units (sector size)
   - Example positions: (0,0,0), (20000000,0,0), (0,20000000,0)
   - This ensures clean grid coordinates and no overlap

2. **Sector Spacing**
   - Sectors can be adjacent (touching) or separated
   - Leave gaps for jump points or warp gates
   - Consider gameplay flow when placing sectors

3. **Naming Convention**
   ```
   - "Alpha Sector" - Simple names for early game
   - "Trading Hub Prime" - Functional descriptions
   - "Contested Zone 7" - Context-based names
   - "Nebula Field X-47" - Coordinate-based names
   ```

### Performance Optimization

1. **Actor Tracking**
   ```cpp
   // Prefer actor count when you don't need the array
   int32 Count = Sector->GetActorCountInSector(); // Fast
   
   // Only get array when needed
   TArray<AActor*> Actors = Sector->GetActorsInSector(); // Slower
   ```

2. **Neighbor Caching**
   - Neighbor lists are cached automatically
   - Cache invalidates on sector move or property change
   - Reuse neighbor queries within the same frame

3. **Widget Updates**
   - Don't call UpdateObjectTracking every frame
   - Use timers for periodic updates (e.g., every 2 seconds)
   - Only update visible widgets

### Save/Load Integration

```cpp
// Example: Save discovered sectors
void AMyGameInstance::SaveUniverseMapData()
{
    TArray<FString> DiscoveredSectorNames;
    TArray<ASpaceSectorMap*> DiscoveredSectors = UniverseMapWidget->GetDiscoveredSectors();
    
    for (ASpaceSectorMap* Sector : DiscoveredSectors)
    {
        if (Sector)
        {
            DiscoveredSectorNames.Add(Sector->SectorName.ToString());
        }
    }
    
    // Save to save game object
    MySaveGame->DiscoveredSectors = DiscoveredSectorNames;
}

// Example: Load discovered sectors
void AMyGameInstance::LoadUniverseMapData()
{
    TArray<ASpaceSectorMap*> AllSectors = UniverseMapWidget->GetAllSectors();
    
    for (const FString& SectorName : MySaveGame->DiscoveredSectors)
    {
        for (ASpaceSectorMap* Sector : AllSectors)
        {
            if (Sector && Sector->SectorName.ToString() == SectorName)
            {
                UniverseMapWidget->MarkSectorDiscovered(Sector);
                break;
            }
        }
    }
}
```

## Example: Complete Map System Setup

### 1. Create Sectors in Level

```cpp
// Place 3x3 grid of sectors in your level
for (int32 X = -1; X <= 1; ++X)
{
    for (int32 Y = -1; Y <= 1; ++Y)
    {
        FVector Position(
            X * 20000000.0f,  // 200km spacing
            Y * 20000000.0f,
            0.0f
        );
        
        ASpaceSectorMap* Sector = World->SpawnActor<ASpaceSectorMap>(
            ASpaceSectorMap::StaticClass(),
            Position,
            FRotator::ZeroRotator
        );
        
        if (Sector)
        {
            Sector->SectorName = FText::FromString(
                FString::Printf(TEXT("Sector (%d, %d)"), X, Y)
            );
        }
    }
}
```

### 2. Setup Universe Map Widget

```cpp
// In your HUD or main menu
void AMyHUD::CreateUniverseMap()
{
    // Create widget
    UniverseMapWidget = CreateWidget<UUniverseMapWidget>(
        GetOwningPlayerController(),
        UniverseMapWidgetClass
    );
    
    if (UniverseMapWidget)
    {
        // Configure
        UniverseMapWidget->bAutoDiscoverVisitedSectors = true;
        UniverseMapWidget->bShowUndiscoveredSectors = false;
        
        // Add to viewport but hidden
        UniverseMapWidget->AddToViewport();
        UniverseMapWidget->ToggleUniverseMapVisibility(false);
    }
}
```

### 3. Setup Input Bindings

```cpp
// In your player controller
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Bind 'M' key to toggle universe map
    InputComponent->BindAction(
        "ToggleUniverseMap",
        IE_Pressed,
        this,
        &AMyPlayerController::ToggleUniverseMap
    );
}

void AMyPlayerController::ToggleUniverseMap()
{
    if (!UniverseMapWidget)
    {
        return;
    }
    
    bool bCurrentlyVisible = UniverseMapWidget->bIsUniverseMapVisible;
    UniverseMapWidget->ToggleUniverseMapVisibility(!bCurrentlyVisible);
    
    // Toggle pause and cursor
    SetPause(!bCurrentlyVisible);
    bShowMouseCursor = !bCurrentlyVisible;
    
    if (!bCurrentlyVisible)
    {
        SetInputMode(FInputModeUIOnly());
    }
    else
    {
        SetInputMode(FInputModeGameOnly());
    }
}
```

### 4. Track Player Discovery

```cpp
// In your player pawn or game mode
void AMySpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check if we've entered a new sector
    ASpaceSectorMap* CurrentSector = FindCurrentSector();
    
    if (CurrentSector && CurrentSector != LastSector)
    {
        // Mark sector as discovered
        if (UniverseMapWidget)
        {
            UniverseMapWidget->MarkSectorDiscovered(CurrentSector);
        }
        
        // Show notification
        ShowSectorEntryNotification(CurrentSector);
        
        LastSector = CurrentSector;
    }
}

ASpaceSectorMap* AMySpaceship::FindCurrentSector()
{
    TArray<AActor*> FoundSectors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        ASpaceSectorMap::StaticClass(),
        FoundSectors
    );
    
    FVector MyPosition = GetActorLocation();
    
    for (AActor* Actor : FoundSectors)
    {
        ASpaceSectorMap* Sector = Cast<ASpaceSectorMap>(Actor);
        if (Sector && Sector->IsPositionInSector(MyPosition))
        {
            return Sector;
        }
    }
    
    return nullptr;
}
```

## Troubleshooting

### Sectors Overlapping

**Problem**: Warning about overlapping sectors in log
**Solution**: 
- Sectors should be spaced at least SectorSize/2 apart (10,000,000 units)
- Use grid alignment: multiples of 20,000,000 units
- Run ValidateSectorConfiguration() to check

### Pathfinding Returns Empty Array

**Problem**: FindPathBetweenSectors returns no results
**Solution**:
- Ensure sectors are connected (neighbors exist)
- Check GetNeighboringSectors() returns adjacent sectors
- Verify sectors aren't too far apart (max 1.5 * SectorSize for neighbors)

### Actor Tracking Missing Objects

**Problem**: GetActorsInSector returns fewer actors than expected
**Solution**:
- Verify actor positions are within sector bounds
- Use GetDebugInfo() to check sector bounds
- Ensure actors aren't on sector edges (floating point precision)

### Widget Not Showing Up

**Problem**: Map widget is created but not visible
**Solution**:
- Check widget is added to viewport: `AddToViewport()`
- Verify visibility state: `ToggleUniverseMapVisibility(true)`
- Check Z-order if other widgets are covering it
- Ensure bAutoCreateMissingWidgets is true if no Blueprint setup

## API Reference

See header files for complete API documentation:
- `Source/Adastrea/SpaceSectorMap.h`
- `Source/Adastrea/UI/UniverseMapWidget.h`
- `Source/Adastrea/UI/SectorMapWidget.h`

## Additional Resources

- [Adastrea Architecture](../ARCHITECTURE.md)
- [Blueprint Creation Guide](../BLUEPRINT_CREATION_GUIDE.md)
- [Widget Implementation Summary](../WIDGET_IMPLEMENTATION_SUMMARY.md)
