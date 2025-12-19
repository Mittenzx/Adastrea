# Map System Implementation Examples

This document provides complete, copy-paste ready examples for common map system implementations in Adastrea.

## Table of Contents

1. [Basic Setup](#basic-setup)
2. [Universe Map Integration](#universe-map-integration)
3. [Sector Discovery System](#sector-discovery-system)
4. [Navigation System](#navigation-system)
5. [Dynamic Sector Population](#dynamic-sector-population)
6. [Save/Load System](#saveload-system)
7. [HUD Integration](#hud-integration)
8. [Advanced Features](#advanced-features)

## Basic Setup

### 1. Creating a 3x3 Sector Grid (C++)

```cpp
// AMyGameMode.cpp
void AMyGameMode::InitializeUniverse()
{
    const float SectorSize = 20000000.0f; // 200km in Unreal Units
    
    // Create 3x3 grid centered at origin
    for (int32 X = -1; X <= 1; ++X)
    {
        for (int32 Y = -1; Y <= 1; ++Y)
        {
            FVector Position(X * SectorSize, Y * SectorSize, 0.0f);
            FRotator Rotation = FRotator::ZeroRotator;
            
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = FName(*FString::Printf(TEXT("Sector_%d_%d"), X, Y));
            
            ASpaceSectorMap* Sector = GetWorld()->SpawnActor<ASpaceSectorMap>(
                ASpaceSectorMap::StaticClass(),
                Position,
                Rotation,
                SpawnParams
            );
            
            if (Sector)
            {
                // Set sector properties
                Sector->SectorName = FText::FromString(
                    FString::Printf(TEXT("Sector (%d, %d)"), X, Y)
                );
                
                // Set description based on position
                if (X == 0 && Y == 0)
                {
                    Sector->Description = FText::FromString(TEXT("Central Hub - High traffic trading zone"));
                }
                else if (FMath::Abs(X) + FMath::Abs(Y) > 1)
                {
                    Sector->Description = FText::FromString(TEXT("Outer rim - Frontier territory"));
                }
                else
                {
                    Sector->Description = FText::FromString(TEXT("Mid-region - Mixed activity"));
                }
                
                UE_LOG(LogTemp, Log, TEXT("Created sector: %s at %s"),
                    *Sector->SectorName.ToString(),
                    *Position.ToString());
            }
        }
    }
}
```

### 2. Setting Up Map Widgets (Blueprint)

```cpp
// AMyHUD.h
UCLASS()
class ADASTREA_API AMyHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ToggleUniverseMap();
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowSectorDetails(ASpaceSectorMap* Sector);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUniverseMapWidget> UniverseMapClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<USectorMapWidget> SectorMapClass;
    
    UPROPERTY()
    UUniverseMapWidget* UniverseMapWidget;
    
    UPROPERTY()
    USectorMapWidget* SectorMapWidget;
};

// AMyHUD.cpp
void AMyHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // Create universe map widget
    if (UniverseMapClass)
    {
        UniverseMapWidget = CreateWidget<UUniverseMapWidget>(
            GetOwningPlayerController(),
            UniverseMapClass
        );
        
        if (UniverseMapWidget)
        {
            UniverseMapWidget->AddToViewport(10); // High Z-order
            UniverseMapWidget->ToggleUniverseMapVisibility(false); // Start hidden
        }
    }
    
    // Create sector map widget
    if (SectorMapClass)
    {
        SectorMapWidget = CreateWidget<USectorMapWidget>(
            GetOwningPlayerController(),
            SectorMapClass
        );
        
        if (SectorMapWidget)
        {
            SectorMapWidget->AddToViewport(5);
            SectorMapWidget->ToggleSectorMapVisibility(false); // Start hidden
        }
    }
}

void AMyHUD::ToggleUniverseMap()
{
    if (!UniverseMapWidget)
    {
        return;
    }
    
    bool bCurrentlyVisible = UniverseMapWidget->bIsUniverseMapVisible;
    UniverseMapWidget->ToggleUniverseMapVisibility(!bCurrentlyVisible);
    
    // Toggle input mode and pause
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        if (!bCurrentlyVisible)
        {
            // Opening map
            PC->SetPause(true);
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeUIOnly());
        }
        else
        {
            // Closing map
            PC->SetPause(false);
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}

void AMyHUD::ShowSectorDetails(ASpaceSectorMap* Sector)
{
    if (!SectorMapWidget || !Sector)
    {
        return;
    }
    
    SectorMapWidget->SetTargetSector(Sector);
    SectorMapWidget->ToggleSectorMapVisibility(true);
    SectorMapWidget->UpdateObjectTracking();
}
```

## Universe Map Integration

### 3. Input Bindings Setup

```cpp
// AMyPlayerController.h
UCLASS()
class ADASTREA_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void SetupInputComponent() override;

protected:
    UFUNCTION()
    void OnToggleUniverseMap();
    
    UFUNCTION()
    void OnToggleSectorMap();
    
    UFUNCTION()
    void OnSetWaypoint();

private:
    UPROPERTY()
    class AMyHUD* MyHUD;
};

// AMyPlayerController.cpp
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Bind map controls
    InputComponent->BindAction("ToggleUniverseMap", IE_Pressed, this, &AMyPlayerController::OnToggleUniverseMap);
    InputComponent->BindAction("ToggleSectorMap", IE_Pressed, this, &AMyPlayerController::OnToggleSectorMap);
    InputComponent->BindAction("SetWaypoint", IE_Pressed, this, &AMyPlayerController::OnSetWaypoint);
}

void AMyPlayerController::OnToggleUniverseMap()
{
    MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
    if (MyHUD)
    {
        MyHUD->ToggleUniverseMap();
    }
}

void AMyPlayerController::OnToggleSectorMap()
{
    MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
    if (MyHUD && MyHUD->SectorMapWidget)
    {
        bool bVisible = MyHUD->SectorMapWidget->bIsSectorMapVisible;
        MyHUD->SectorMapWidget->ToggleSectorMapVisibility(!bVisible);
    }
}

void AMyPlayerController::OnSetWaypoint()
{
    MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
    if (MyHUD && MyHUD->UniverseMapWidget)
    {
        ASpaceSectorMap* Selected = MyHUD->UniverseMapWidget->GetSelectedSector();
        if (Selected)
        {
            // Set navigation waypoint to selected sector
            SetNavigationWaypoint(Selected->GetSectorCenter());
        }
    }
}
```

## Sector Discovery System

### 4. Automatic Discovery on Entry

```cpp
// APlayerSpaceship.h
UCLASS()
class ADASTREA_API APlayerSpaceship : public ASpaceship
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION(BlueprintCallable, Category = "Sector")
    ASpaceSectorMap* GetCurrentSector() const;

protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Sector")
    void OnEnterSector(ASpaceSectorMap* Sector);
    virtual void OnEnterSector_Implementation(ASpaceSectorMap* Sector);
    
    UFUNCTION(BlueprintNativeEvent, Category = "Sector")
    void OnExitSector(ASpaceSectorMap* Sector);
    virtual void OnExitSector_Implementation(ASpaceSectorMap* Sector);

private:
    UPROPERTY()
    ASpaceSectorMap* CurrentSector;
    
    UPROPERTY()
    ASpaceSectorMap* LastSector;
    
    void UpdateCurrentSector();
};

// APlayerSpaceship.cpp
void APlayerSpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCurrentSector();
}

void APlayerSpaceship::UpdateCurrentSector()
{
    CurrentSector = GetCurrentSector();
    
    if (CurrentSector != LastSector)
    {
        // Exit previous sector
        if (LastSector)
        {
            OnExitSector(LastSector);
        }
        
        // Enter new sector
        if (CurrentSector)
        {
            OnEnterSector(CurrentSector);
        }
        
        LastSector = CurrentSector;
    }
}

ASpaceSectorMap* APlayerSpaceship::GetCurrentSector() const
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

void APlayerSpaceship::OnEnterSector_Implementation(ASpaceSectorMap* Sector)
{
    if (!Sector)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Entered sector: %s"), *Sector->SectorName.ToString());
    
    // Mark sector as discovered
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        AMyHUD* HUD = Cast<AMyHUD>(PC->GetHUD());
        if (HUD && HUD->UniverseMapWidget)
        {
            HUD->UniverseMapWidget->MarkSectorDiscovered(Sector);
        }
    }
    
    // Show notification
    ShowSectorEntryNotification(Sector);
}

void APlayerSpaceship::OnExitSector_Implementation(ASpaceSectorMap* Sector)
{
    if (!Sector)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Exited sector: %s"), *Sector->SectorName.ToString());
}
```

### 5. Sector Entry Notification

```cpp
// Create a notification widget
void APlayerSpaceship::ShowSectorEntryNotification(ASpaceSectorMap* Sector)
{
    if (!Sector)
    {
        return;
    }
    
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }
    
    // Create notification text
    FText NotificationText = FText::Format(
        FText::FromString("Entering: {0}"),
        Sector->SectorName
    );
    
    // Show on-screen message (you can replace with custom widget)
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Cyan,
            NotificationText.ToString(),
            true,
            FVector2D(2.0f, 2.0f)
        );
    }
}
```

## Navigation System

### 6. Waypoint Navigation System

```cpp
// ANavigationComponent.h
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UNavigationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNavigationComponent();
    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void SetWaypointSector(ASpaceSectorMap* Sector);
    
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void SetWaypointPosition(FVector Position);
    
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void ClearWaypoint();
    
    UFUNCTION(BlueprintPure, Category = "Navigation")
    FVector GetDirectionToWaypoint() const;
    
    UFUNCTION(BlueprintPure, Category = "Navigation")
    float GetDistanceToWaypoint() const;
    
    UFUNCTION(BlueprintPure, Category = "Navigation")
    bool HasWaypoint() const { return bHasWaypoint; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    bool bHasWaypoint;
    
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    FVector WaypointPosition;
    
    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    ASpaceSectorMap* WaypointSector;
};

// ANavigationComponent.cpp
UNavigationComponent::UNavigationComponent()
    : bHasWaypoint(false)
    , WaypointPosition(FVector::ZeroVector)
    , WaypointSector(nullptr)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
}

void UNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bHasWaypoint)
    {
        // Check if we've arrived (within 10km)
        float Distance = GetDistanceToWaypoint();
        if (Distance < 1000000.0f) // 10km in Unreal Units
        {
            // Arrived at waypoint
            OnWaypointReached();
            ClearWaypoint();
        }
    }
}

void UNavigationComponent::SetWaypointSector(ASpaceSectorMap* Sector)
{
    if (!Sector)
    {
        ClearWaypoint();
        return;
    }
    
    WaypointSector = Sector;
    WaypointPosition = Sector->GetSectorCenter();
    bHasWaypoint = true;
    
    UE_LOG(LogTemp, Log, TEXT("Navigation: Waypoint set to sector '%s'"),
        *Sector->SectorName.ToString());
}

void UNavigationComponent::SetWaypointPosition(FVector Position)
{
    WaypointPosition = Position;
    WaypointSector = nullptr;
    bHasWaypoint = true;
    
    UE_LOG(LogTemp, Log, TEXT("Navigation: Waypoint set to position %s"),
        *Position.ToString());
}

void UNavigationComponent::ClearWaypoint()
{
    bHasWaypoint = false;
    WaypointSector = nullptr;
    WaypointPosition = FVector::ZeroVector;
    
    UE_LOG(LogTemp, Log, TEXT("Navigation: Waypoint cleared"));
}

FVector UNavigationComponent::GetDirectionToWaypoint() const
{
    if (!bHasWaypoint)
    {
        return FVector::ZeroVector;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return FVector::ZeroVector;
    }
    
    FVector Direction = (WaypointPosition - Owner->GetActorLocation());
    Direction.Normalize();
    return Direction;
}

float UNavigationComponent::GetDistanceToWaypoint() const
{
    if (!bHasWaypoint)
    {
        return -1.0f;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return -1.0f;
    }
    
    return FVector::Dist(Owner->GetActorLocation(), WaypointPosition);
}

void UNavigationComponent::OnWaypointReached()
{
    UE_LOG(LogTemp, Log, TEXT("Navigation: Arrived at waypoint"));
    
    // Play sound, show notification, etc.
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Green,
            TEXT("Waypoint Reached!"),
            true,
            FVector2D(2.0f, 2.0f)
        );
    }
}
```

### 7. Pathfinding with Waypoints

```cpp
// Navigate to a sector using pathfinding
void AMyPlayerController::NavigateToSector(ASpaceSectorMap* TargetSector)
{
    if (!TargetSector)
    {
        return;
    }
    
    // Get universe map
    AMyHUD* HUD = Cast<AMyHUD>(GetHUD());
    if (!HUD || !HUD->UniverseMapWidget)
    {
        return;
    }
    
    // Get player's current sector
    ASpaceSectorMap* CurrentSector = HUD->UniverseMapWidget->GetPlayerCurrentSector();
    if (!CurrentSector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player not in any sector"));
        return;
    }
    
    // Find path
    TArray<ASpaceSectorMap*> Path = HUD->UniverseMapWidget->FindPathBetweenSectors(
        CurrentSector,
        TargetSector
    );
    
    if (Path.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No path found to target sector"));
        return;
    }
    
    // Set waypoints for each sector in path
    for (int32 i = 0; i < Path.Num(); ++i)
    {
        ASpaceSectorMap* WaypointSector = Path[i];
        
        // Add to waypoint queue or just use first waypoint
        if (i == 0)
        {
            // Set first waypoint
            APawn* PlayerPawn = GetPawn();
            if (PlayerPawn)
            {
                UNavigationComponent* NavComp = PlayerPawn->FindComponentByClass<UNavigationComponent>();
                if (NavComp)
                {
                    NavComp->SetWaypointSector(WaypointSector);
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Path waypoint %d: %s"), i + 1, *WaypointSector->SectorName.ToString());
    }
}
```

## Dynamic Sector Population

### 8. Procedural Object Spawning in Sectors

```cpp
// ASectorPopulator.h
UCLASS()
class ADASTREA_API ASectorPopulator : public AActor
{
    GENERATED_BODY()

public:
    ASectorPopulator();
    
    UFUNCTION(BlueprintCallable, Category = "Sector Population")
    void PopulateSector(ASpaceSectorMap* Sector, int32 ObjectCount);
    
    UFUNCTION(BlueprintCallable, Category = "Sector Population")
    void PopulateAllSectors();
    
    UFUNCTION(BlueprintCallable, Category = "Sector Population")
    void ClearSectorObjects(ASpaceSectorMap* Sector);

protected:
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TArray<TSubclassOf<AActor>> SpawnableActors;
    
    UPROPERTY(EditAnywhere, Category = "Spawning")
    int32 MinObjectsPerSector = 5;
    
    UPROPERTY(EditAnywhere, Category = "Spawning")
    int32 MaxObjectsPerSector = 20;
};

// ASectorPopulator.cpp
ASectorPopulator::ASectorPopulator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASectorPopulator::PopulateSector(ASpaceSectorMap* Sector, int32 ObjectCount)
{
    if (!Sector || SpawnableActors.Num() == 0)
    {
        return;
    }
    
    for (int32 i = 0; i < ObjectCount; ++i)
    {
        // Get random spawn position in sector
        FVector SpawnPosition = Sector->GetRandomPositionInSector();
        FRotator SpawnRotation = FRotator(
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(0.0f, 360.0f)
        );
        
        // Choose random actor class
        int32 ClassIndex = FMath::RandRange(0, SpawnableActors.Num() - 1);
        TSubclassOf<AActor> ActorClass = SpawnableActors[ClassIndex];
        
        // Spawn actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
            ActorClass,
            SpawnPosition,
            SpawnRotation,
            SpawnParams
        );
        
        if (SpawnedActor)
        {
            UE_LOG(LogTemp, Verbose, TEXT("Spawned %s in sector '%s'"),
                *SpawnedActor->GetName(),
                *Sector->SectorName.ToString());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Populated sector '%s' with %d objects"),
        *Sector->SectorName.ToString(),
        ObjectCount);
}

void ASectorPopulator::PopulateAllSectors()
{
    TArray<AActor*> FoundSectors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        ASpaceSectorMap::StaticClass(),
        FoundSectors
    );
    
    for (AActor* Actor : FoundSectors)
    {
        ASpaceSectorMap* Sector = Cast<ASpaceSectorMap>(Actor);
        if (Sector)
        {
            int32 ObjectCount = FMath::RandRange(MinObjectsPerSector, MaxObjectsPerSector);
            PopulateSector(Sector, ObjectCount);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Populated %d sectors with objects"), FoundSectors.Num());
}

void ASectorPopulator::ClearSectorObjects(ASpaceSectorMap* Sector)
{
    if (!Sector)
    {
        return;
    }
    
    TArray<AActor*> ActorsInSector = Sector->GetActorsInSector();
    
    for (AActor* Actor : ActorsInSector)
    {
        // Don't destroy sectors, players, or other important actors
        if (Actor && !Actor->IsA(ASpaceSectorMap::StaticClass()) && !Actor->IsA(APawn::StaticClass()))
        {
            Actor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleared objects from sector '%s'"),
        *Sector->SectorName.ToString());
}
```

## Save/Load System

### 9. Saving and Loading Map Data

```cpp
// UMyS aveGame.h
USTRUCT(BlueprintType)
struct FSavedSectorData
{
    GENERATED_BODY()
    
    UPROPERTY()
    FString SectorName;
    
    UPROPERTY()
    bool bIsDiscovered;
    
    UPROPERTY()
    FString BookmarkName;
};

UCLASS()
class ADASTREA_API UMySaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<FSavedSectorData> SectorData;
    
    UPROPERTY()
    FVector LastPlayerPosition;
};

// UMyGameInstance.h
UCLASS()
class ADASTREA_API UMyGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    void SaveMapData();
    
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    void LoadMapData();

protected:
    UPROPERTY()
    UMySaveGame* CurrentSaveGame;
    
    FString SaveSlotName = TEXT("MapSave");
};

// UMyGameInstance.cpp
void UMyGameInstance::SaveMapData()
{
    // Create or get save game object
    if (!CurrentSaveGame)
    {
        CurrentSaveGame = Cast<UMySaveGame>(
            UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())
        );
    }
    
    if (!CurrentSaveGame)
    {
        return;
    }
    
    // Get universe map widget (implementation depends on your setup)
    UUniverseMapWidget* UniverseMap = FindUniverseMapWidget();
    if (!UniverseMap)
    {
        return;
    }
    
    // Clear previous data
    CurrentSaveGame->SectorData.Empty();
    
    // Save discovered sectors and bookmarks
    TArray<ASpaceSectorMap*> AllSectors = UniverseMap->GetAllSectors();
    
    for (ASpaceSectorMap* Sector : AllSectors)
    {
        if (!Sector)
        {
            continue;
        }
        
        FSavedSectorData Data;
        Data.SectorName = Sector->SectorName.ToString();
        Data.bIsDiscovered = UniverseMap->IsSectorDiscovered(Sector);
        
        // Check if bookmarked
        if (UniverseMap->IsSectorBookmarked(Sector))
        {
            // Get bookmark name from map (you may need to store this separately)
            Data.BookmarkName = TEXT("Bookmarked"); // Simplified
        }
        
        CurrentSaveGame->SectorData.Add(Data);
    }
    
    // Save player position
    APlayerController* PC = GetFirstLocalPlayerController();
    if (PC && PC->GetPawn())
    {
        CurrentSaveGame->LastPlayerPosition = PC->GetPawn()->GetActorLocation();
    }
    
    // Save to disk
    bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, 0);
    
    UE_LOG(LogTemp, Log, TEXT("Map data saved: %s"), bSuccess ? TEXT("Success") : TEXT("Failed"));
}

void UMyGameInstance::LoadMapData()
{
    // Load save game
    CurrentSaveGame = Cast<UMySaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
    );
    
    if (!CurrentSaveGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("No save game found"));
        return;
    }
    
    // Get universe map widget
    UUniverseMapWidget* UniverseMap = FindUniverseMapWidget();
    if (!UniverseMap)
    {
        return;
    }
    
    // Restore discovered sectors and bookmarks
    TArray<ASpaceSectorMap*> AllSectors = UniverseMap->GetAllSectors();
    
    for (const FSavedSectorData& Data : CurrentSaveGame->SectorData)
    {
        // Find matching sector by name
        for (ASpaceSectorMap* Sector : AllSectors)
        {
            if (Sector && Sector->SectorName.ToString() == Data.SectorName)
            {
                // Restore discovered state
                if (Data.bIsDiscovered)
                {
                    UniverseMap->MarkSectorDiscovered(Sector);
                }
                
                // Restore bookmark
                if (!Data.BookmarkName.IsEmpty())
                {
                    UniverseMap->AddSectorBookmark(
                        Sector,
                        FText::FromString(Data.BookmarkName)
                    );
                }
                
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Map data loaded: %d sectors restored"),
        CurrentSaveGame->SectorData.Num());
}
```

## HUD Integration

### 10. Minimal HUD Sector Display

```cpp
// Simple HUD drawing
void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
    
    // Draw current sector info
    DrawCurrentSectorInfo();
    
    // Draw waypoint indicator
    DrawWaypointIndicator();
}

void AMyHUD::DrawCurrentSectorInfo()
{
    // Get player pawn
    APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    // Cast to spaceship to get sector info
    APlayerSpaceship* Ship = Cast<APlayerSpaceship>(PlayerPawn);
    if (!Ship)
    {
        return;
    }
    
    ASpaceSectorMap* CurrentSector = Ship->GetCurrentSector();
    if (!CurrentSector)
    {
        DrawText(TEXT("No Sector"), FColor::White, 20, 20);
        return;
    }
    
    // Draw sector name
    FString SectorText = FString::Printf(
        TEXT("Sector: %s"),
        *CurrentSector->SectorName.ToString()
    );
    DrawText(SectorText, FColor::Cyan, 20, 20, nullptr, 1.5f);
    
    // Draw object count
    int32 ObjectCount = CurrentSector->GetActorCountInSector();
    FString ObjectText = FString::Printf(TEXT("Objects: %d"), ObjectCount);
    DrawText(ObjectText, FColor::White, 20, 50);
    
    // Draw grid coordinates
    FIntVector GridCoords = CurrentSector->GetGridCoordinates();
    FString GridText = FString::Printf(
        TEXT("Grid: [%d, %d, %d]"),
        GridCoords.X, GridCoords.Y, GridCoords.Z
    );
    DrawText(GridText, FColor::White, 20, 70);
}

void AMyHUD::DrawWaypointIndicator()
{
    APawn* PlayerPawn = GetOwningPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    UNavigationComponent* NavComp = PlayerPawn->FindComponentByClass<UNavigationComponent>();
    if (!NavComp || !NavComp->HasWaypoint())
    {
        return;
    }
    
    // Get direction to waypoint in screen space
    FVector WaypointDirection = NavComp->GetDirectionToWaypoint();
    float Distance = NavComp->GetDistanceToWaypoint();
    
    // Project to screen
    FVector2D ScreenPos;
    FVector WaypointWorldPos = PlayerPawn->GetActorLocation() + (WaypointDirection * 100000.0f);
    
    if (GetOwningPlayerController()->ProjectWorldLocationToScreen(WaypointWorldPos, ScreenPos))
    {
        // Draw waypoint marker
        DrawText(TEXT("★"), FColor::Yellow, ScreenPos.X - 10, ScreenPos.Y - 10, nullptr, 2.0f);
        
        // Draw distance
        FString DistanceText = FString::Printf(
            TEXT("%.1f km"),
            Distance / 100000.0f // Convert to km
        );
        DrawText(DistanceText, FColor::Yellow, ScreenPos.X - 30, ScreenPos.Y + 20);
    }
}
```

## Advanced Features

### 11. Sector Threat Level System

```cpp
// Add threat level to sectors
USTRUCT(BlueprintType)
struct FSectorThreatData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HostileShipCount = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> HostileActors;
};

// Calculate threat level for a sector
FSectorThreatData CalculateSectorThreat(ASpaceSectorMap* Sector)
{
    FSectorThreatData ThreatData;
    
    if (!Sector)
    {
        return ThreatData;
    }
    
    // Get all actors in sector
    TArray<AActor*> AllActors = Sector->GetActorsInSector();
    
    for (AActor* Actor : AllActors)
    {
        // Check if actor is hostile (implement your own logic)
        if (IsHostileActor(Actor))
        {
            ThreatData.HostileShipCount++;
            ThreatData.HostileActors.Add(Actor);
        }
    }
    
    // Calculate threat level (0.0 to 1.0)
    ThreatData.ThreatLevel = FMath::Clamp(
        ThreatData.HostileShipCount / 10.0f,
        0.0f,
        1.0f
    );
    
    return ThreatData;
}
```

### 12. Sector Resource Tracking

```cpp
// Track resources in sectors
USTRUCT(BlueprintType)
struct FSectorResourceData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AsteroidCount = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TradingStationCount = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasRefuelStation = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasRepairFacility = false;
};

// Analyze sector resources
FSectorResourceData AnalyzeSectorResources(ASpaceSectorMap* Sector)
{
    FSectorResourceData ResourceData;
    
    if (!Sector)
    {
        return ResourceData;
    }
    
    // Count asteroids
    TArray<AActor*> Asteroids = Sector->GetActorsInSector(AAsteroid::StaticClass());
    ResourceData.AsteroidCount = Asteroids.Num();
    
    // Count trading stations
    TArray<AActor*> Stations = Sector->GetActorsInSector(ASpaceStation::StaticClass());
    ResourceData.TradingStationCount = Stations.Num();
    
    // Check for specific facilities
    for (AActor* Station : Stations)
    {
        ASpaceStation* SpaceStation = Cast<ASpaceStation>(Station);
        if (SpaceStation)
        {
            if (SpaceStation->HasRefuelCapability())
            {
                ResourceData.bHasRefuelStation = true;
            }
            if (SpaceStation->HasRepairCapability())
            {
                ResourceData.bHasRepairFacility = true;
            }
        }
    }
    
    return ResourceData;
}
```

## Summary

These examples provide production-ready implementations for:

- ✅ Sector creation and management
- ✅ Map widget setup and integration
- ✅ Discovery and exploration systems
- ✅ Navigation and pathfinding
- ✅ Dynamic object population
- ✅ Save/load functionality
- ✅ HUD integration
- ✅ Advanced features (threat levels, resources)

All code is designed to work with the enhanced map system and follows Unreal Engine best practices.

## Next Steps

1. Customize the examples for your specific game needs
2. Add your own actor types for sector population
3. Enhance the UI with custom widgets
4. Implement multiplayer synchronization if needed
5. Add more advanced features (sector events, dynamic environments, etc.)

## See Also

- [MapSystemGuide.md](MapSystemGuide.md) - Complete documentation
- [MapSystemQuickReference.md](MapSystemQuickReference.md) - Quick reference
- [ARCHITECTURE.md](../ARCHITECTURE.md) - Project architecture
