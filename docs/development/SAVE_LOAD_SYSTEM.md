# Save/Load System for Trade Simulator MVP

**Purpose**: Persist player progress between game sessions for the Trade Simulator MVP.

**Design Goal**: Simple, reliable save system that preserves trading progress without over-complication.

---

## Overview

### What to Save

**Critical (Must Save)**:
1. Player Credits (current balance)
2. Total Lifetime Earnings (for ship unlocks)
3. Current Ship ID (which ship owned)
4. Cargo Inventory (items in cargo hold)
5. Current Location (docked station or coordinates)

**Important (Should Save)**:
6. Ship Upgrades Unlocked (progression flags)
7. Visited Stations (discovery tracking)
8. Trading Statistics (trades completed, profit made)

**Nice-to-Have (Optional for MVP)**:
9. Market prices (current state)
10. Active market events
11. UI preferences (tutorial completed, etc.)

**NOT Saved (Session-Only)**:
- AI trader states
- Dynamic world state (regenerate on load)
- Temporary UI state

---

## Save Data Structure

### USaveGameData Class

```cpp
UCLASS()
class UAdastreaTradeSimSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Constructor
    UAdastreaTradeSimSaveGame();
    
    // Save Slot Info
    UPROPERTY(VisibleAnywhere, Category="Save Info")
    FString SaveSlotName;
    
    UPROPERTY(VisibleAnywhere, Category="Save Info")
    int32 SaveVersion; // For backwards compatibility
    
    UPROPERTY(VisibleAnywhere, Category="Save Info")
    FDateTime SaveTimestamp;
    
    // Player Progression
    UPROPERTY(VisibleAnywhere, Category="Player")
    int32 CurrentCredits;
    
    UPROPERTY(VisibleAnywhere, Category="Player")
    int32 TotalLifetimeEarnings;
    
    UPROPERTY(VisibleAnywhere, Category="Player")
    FName CurrentShipID; // "ship_starter_compact", "ship_medium_hauler", etc.
    
    UPROPERTY(VisibleAnywhere, Category="Player")
    TArray<FName> UnlockedShipIDs; // All ships player can switch to
    
    // Cargo State
    UPROPERTY(VisibleAnywhere, Category="Cargo")
    TMap<FName, int32> CargoInventory; // ItemID → Quantity
    
    UPROPERTY(VisibleAnywhere, Category="Cargo")
    int32 UsedCargoSpace;
    
    UPROPERTY(VisibleAnywhere, Category="Cargo")
    int32 MaxCargoSpace;
    
    // Location
    UPROPERTY(VisibleAnywhere, Category="Location")
    FName CurrentStationID; // If docked, which station
    
    UPROPERTY(VisibleAnywhere, Category="Location")
    FVector WorldPosition; // If flying, where
    
    UPROPERTY(VisibleAnywhere, Category="Location")
    FRotator WorldRotation;
    
    UPROPERTY(VisibleAnywhere, Category="Location")
    bool bIsDocked;
    
    // Trading Statistics
    UPROPERTY(VisibleAnywhere, Category="Statistics")
    int32 TotalTradesCompleted;
    
    UPROPERTY(VisibleAnywhere, Category="Statistics")
    int32 TotalProfitMade; // Can go negative
    
    UPROPERTY(VisibleAnywhere, Category="Statistics")
    TArray<FName> VisitedStationIDs;
    
    UPROPERTY(VisibleAnywhere, Category="Statistics")
    float TotalPlayTimeSeconds;
    
    // Optional: Tutorial/Settings
    UPROPERTY(VisibleAnywhere, Category="Settings")
    bool bHasCompletedTutorial;
    
    UPROPERTY(VisibleAnywhere, Category="Settings")
    TMap<FName, bool> TutorialStepsCompleted; // "first_dock", "first_trade", etc.
};
```

### C++ Implementation

```cpp
// In PlayerTraderComponent or GameInstance

bool USaveLoadManager::SaveGame(const FString& SlotName)
{
    UAdastreaTradeSimSaveGame* SaveGameInstance = Cast<UAdastreaTradeSimSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UAdastreaTradeSimSaveGame::StaticClass())
    );
    
    if (!SaveGameInstance)
    {
        UE_LOG(LogTrading, Error, TEXT("Failed to create save game object"));
        return false;
    }
    
    // Populate save data
    SaveGameInstance->SaveSlotName = SlotName;
    SaveGameInstance->SaveVersion = 1; // MVP version
    SaveGameInstance->SaveTimestamp = FDateTime::Now();
    
    // Get player trader component
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UPlayerTraderComponent* TraderComp = PC->FindComponentByClass<UPlayerTraderComponent>();
    
    if (TraderComp)
    {
        SaveGameInstance->CurrentCredits = TraderComp->GetCredits();
        SaveGameInstance->TotalLifetimeEarnings = TraderComp->GetTotalLifetimeEarnings();
        SaveGameInstance->CurrentShipID = TraderComp->GetCurrentShipID();
        SaveGameInstance->UnlockedShipIDs = TraderComp->GetUnlockedShipIDs();
    }
    
    // Get cargo component
    UCargoComponent* CargoComp = PC->GetPawn()->FindComponentByClass<UCargoComponent>();
    if (CargoComp)
    {
        SaveGameInstance->CargoInventory = CargoComp->GetCargoInventory();
        SaveGameInstance->UsedCargoSpace = CargoComp->GetUsedCargoSpace();
        SaveGameInstance->MaxCargoSpace = CargoComp->GetCargoCapacity();
    }
    
    // Get location
    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn)
    {
        SaveGameInstance->WorldPosition = PlayerPawn->GetActorLocation();
        SaveGameInstance->WorldRotation = PlayerPawn->GetActorRotation();
    }
    
    // Save statistics
    SaveGameInstance->TotalTradesCompleted = TraderComp->GetTotalTrades();
    SaveGameInstance->TotalProfitMade = TraderComp->GetTotalProfit();
    SaveGameInstance->VisitedStationIDs = TraderComp->GetVisitedStations();
    
    // Perform save
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
    
    if (bSuccess)
    {
        UE_LOG(LogTrading, Log, TEXT("Game saved successfully to slot: %s"), *SlotName);
    }
    else
    {
        UE_LOG(LogTrading, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
    }
    
    return bSuccess;
}

bool USaveLoadManager::LoadGame(const FString& SlotName)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTrading, Warning, TEXT("Save game does not exist: %s"), *SlotName);
        return false;
    }
    
    UAdastreaTradeSimSaveGame* LoadedGame = Cast<UAdastreaTradeSimSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );
    
    if (!LoadedGame)
    {
        UE_LOG(LogTrading, Error, TEXT("Failed to load save game: %s"), *SlotName);
        return false;
    }
    
    // Restore player state
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UPlayerTraderComponent* TraderComp = PC->FindComponentByClass<UPlayerTraderComponent>();
    
    if (TraderComp)
    {
        TraderComp->SetCredits(LoadedGame->CurrentCredits);
        TraderComp->SetTotalLifetimeEarnings(LoadedGame->TotalLifetimeEarnings);
        TraderComp->SetCurrentShip(LoadedGame->CurrentShipID);
        TraderComp->SetUnlockedShips(LoadedGame->UnlockedShipIDs);
        TraderComp->SetTotalTrades(LoadedGame->TotalTradesCompleted);
        TraderComp->SetTotalProfit(LoadedGame->TotalProfitMade);
        TraderComp->SetVisitedStations(LoadedGame->VisitedStationIDs);
    }
    
    // Restore cargo
    UCargoComponent* CargoComp = PC->GetPawn()->FindComponentByClass<UCargoComponent>();
    if (CargoComp)
    {
        CargoComp->SetCargoInventory(LoadedGame->CargoInventory);
    }
    
    // Restore location
    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn)
    {
        if (LoadedGame->bIsDocked)
        {
            // Teleport to station
            ASpaceStation* Station = FindStationByID(LoadedGame->CurrentStationID);
            if (Station)
            {
                FVector DockingLocation = Station->GetDockingLocation();
                PlayerPawn->SetActorLocation(DockingLocation);
                // Auto-dock
                TraderComp->DockAtStation(Station);
            }
        }
        else
        {
            // Restore world position
            PlayerPawn->SetActorLocationAndRotation(
                LoadedGame->WorldPosition,
                LoadedGame->WorldRotation
            );
        }
    }
    
    UE_LOG(LogTrading, Log, TEXT("Game loaded successfully from slot: %s"), *SlotName);
    return true;
}
```

---

## Save Slot System

### Single Slot (MVP)

**Slot Name**: "TradingSimSlot_01"

**Simple System**:
- Auto-save on significant events
- Manual save button in menu
- Single slot (no multiple saves in MVP)
- Continue = load last save
- New Game = overwrite save

### Auto-Save Triggers

**When to Auto-Save**:
1. After completing a trade
2. After ship upgrade
3. After docking at new station
4. Every 5 minutes (background)
5. On game pause/quit

**Auto-Save Implementation**:
```cpp
void UPlayerTraderComponent::OnTradeCompleted()
{
    // Trade logic...
    
    // Auto-save
    USaveLoadManager::Get()->SaveGame("TradingSimSlot_01");
    
    // Show small UI notification
    ShowNotification(TEXT("Progress Saved"), 2.0f);
}
```

---

## Main Menu Integration

### WBP_MainMenu Widget

**Buttons**:
```
┌─────────────────────────────┐
│     TRADE SIMULATOR MVP      │
│─────────────────────────────│
│                             │
│  [Continue]       ← Load    │
│  [New Game]       ← Reset   │
│  [Settings]                 │
│  [Quit]                     │
│                             │
└─────────────────────────────┘
```

**Logic**:
```
OnContinueClicked:
  ↓
Check: DoesSaveGameExist("TradingSimSlot_01")
  ├─ True: LoadGame(), OpenLevel("L_TradingPrototype")
  └─ False: Show "No Save Found", Redirect to New Game

OnNewGameClicked:
  ↓
Check: DoesSaveGameExist("TradingSimSlot_01")
  ├─ True: Show Confirm Dialog ("Overwrite save?")
  │   ├─ Yes: DeleteSave(), StartNewGame()
  │   └─ No: Return to menu
  └─ False: StartNewGame()
```

---

## In-Game Save/Load UI

### WBP_PauseMenu Widget

**Layout**:
```
┌─────────────────────────────┐
│         PAUSED              │
│─────────────────────────────│
│                             │
│  [Resume]                   │
│  [Save Game]    ← Manual    │
│  [Settings]                 │
│  [Main Menu]                │
│  [Quit to Desktop]          │
│                             │
│  Last Saved: 2 minutes ago  │
│  Autosave: ON               │
└─────────────────────────────┘
```

**Save Button Logic**:
```
OnSaveButtonClicked:
  ↓
SaveGame("TradingSimSlot_01")
  ↓
Branch: Success?
  ├─ True: Show "Game Saved Successfully!" (3 sec)
  └─ False: Show "Save Failed!" (5 sec), Log error
```

---

## Save File Location

**Platform-Specific Paths**:

**Windows**:
```
%LOCALAPPDATA%/Adastrea/Saved/SaveGames/TradingSimSlot_01.sav
```

**Mac**:
```
~/Library/Application Support/Adastrea/Saved/SaveGames/TradingSimSlot_01.sav
```

**Linux**:
```
~/.config/Epic/Adastrea/Saved/SaveGames/TradingSimSlot_01.sav
```

**Unreal Default**:
Unreal Engine handles platform-specific paths automatically via `UGameplayStatics::SaveGameToSlot()`.

---

## Backwards Compatibility

### Save Version System

**Purpose**: Allow loading old saves after game updates

**Implementation**:
```cpp
bool USaveLoadManager::LoadGame(const FString& SlotName)
{
    UAdastreaTradeSimSaveGame* LoadedGame = Cast<UAdastreaTradeSimSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );
    
    if (!LoadedGame)
        return false;
    
    // Check version
    switch (LoadedGame->SaveVersion)
    {
        case 1: // MVP version
            LoadGameVersion1(LoadedGame);
            break;
        
        case 2: // Future update
            LoadGameVersion2(LoadedGame);
            break;
        
        default:
            UE_LOG(LogTrading, Error, TEXT("Unknown save version: %d"), LoadedGame->SaveVersion);
            return false;
    }
    
    return true;
}

void USaveLoadManager::LoadGameVersion1(UAdastreaTradeSimSaveGame* SaveData)
{
    // Load MVP-version save data
    // All properties from version 1
}

void USaveLoadManager::LoadGameVersion2(UAdastreaTradeSimSaveGame* SaveData)
{
    // Load version 2 data
    // Handle new properties added in update
    // Migrate old data if needed
}
```

---

## Error Handling

### Common Issues & Solutions

#### Issue: Save Corrupted

**Detection**:
```cpp
bool USaveLoadManager::ValidateSaveData(UAdastreaTradeSimSaveGame* SaveData)
{
    if (!SaveData)
        return false;
    
    // Check critical fields
    if (SaveData->SaveVersion <= 0)
        return false;
    
    if (SaveData->CurrentCredits < 0) // Negative credits = corrupted
        return false;
    
    if (SaveData->MaxCargoSpace <= 0)
        return false;
    
    // All checks passed
    return true;
}
```

**Recovery**:
1. Attempt to load backup (auto-save before last manual save)
2. If backup fails, start new game
3. Show error message to player
4. Log detailed error for debugging

#### Issue: Save File Missing

**Handling**:
```cpp
OnContinueClicked:
  ↓
If !DoesSaveGameExist():
  ↓
  Show Dialog:
    "No save game found. Start a new game?"
    [Yes] → StartNewGame()
    [No] → Return to menu
```

#### Issue: Disk Full / Permission Error

**Handling**:
```cpp
bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SlotName, 0);

if (!bSuccess)
{
    UE_LOG(LogTrading, Error, TEXT("Save failed - check disk space and permissions"));
    
    ShowErrorDialog(
        TEXT("Save Failed"),
        TEXT("Unable to save game. Check disk space and folder permissions.")
    );
    
    return false;
}
```

---

## Testing

### Save/Load Test Checklist

**Test 1: Basic Save/Load**:
1. Start new game
2. Earn 5,000 credits
3. Make 5 trades
4. Save game
5. Quit to main menu
6. Continue
7. Verify: Credits = 5,000, Trades = 5

**Test 2: Mid-Trade Save**:
1. Dock at station
2. Buy 5 water units
3. Save game
4. Quit and reload
5. Verify: Cargo has 5 water, credits deducted

**Test 3: Ship Upgrade Save**:
1. Unlock Merchant Hauler
2. Switch to Merchant Hauler
3. Save game
4. Quit and reload
5. Verify: Playing as Merchant Hauler, 30 cargo capacity

**Test 4: Location Save**:
1. Fly halfway between stations
2. Save game
3. Quit and reload
4. Verify: Ship at correct location in space

**Test 5: Corruption Recovery**:
1. Manually corrupt save file
2. Attempt to load
3. Verify: Error shown, fallback to new game

**Test 6: Auto-Save**:
1. Play for 10 minutes
2. Make 3 trades
3. Crash game (force-quit)
4. Restart
5. Verify: Progress saved (within last 5 minutes)

---

## Implementation Checklist

**Phase 1: Save System** (2 hours):
- [ ] Create UAdastreaTradeSimSaveGame class
- [ ] Implement SaveGame() function
- [ ] Test saving all properties
- [ ] Verify save file created

**Phase 2: Load System** (2 hours):
- [ ] Implement LoadGame() function
- [ ] Restore all player state
- [ ] Handle location restoration
- [ ] Test loading saved game

**Phase 3: Main Menu** (1 hour):
- [ ] Add Continue button
- [ ] Implement New Game with overwrite check
- [ ] Show "Last Played" timestamp
- [ ] Test menu flow

**Phase 4: In-Game UI** (1 hour):
- [ ] Add Save button to pause menu
- [ ] Show last save time
- [ ] Implement manual save
- [ ] Test in-game saving

**Phase 5: Auto-Save** (1 hour):
- [ ] Trigger on trade completion
- [ ] Trigger on ship upgrade
- [ ] Trigger on dock
- [ ] Background timer (every 5 min)
- [ ] Test auto-save reliability

**Phase 6: Error Handling** (1 hour):
- [ ] Validate save data
- [ ] Handle corrupted saves
- [ ] Handle missing saves
- [ ] Handle disk errors
- [ ] Test error scenarios

---

## Performance Considerations

### Save Performance

**Target**: <100ms save time

**Optimization**:
- Save on separate thread (async)
- Compress save data (UE built-in)
- Only save changed data (delta saves - post-MVP)

**Monitoring**:
```cpp
double StartTime = FPlatformTime::Seconds();
bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SlotName, 0);
double EndTime = FPlatformTime::Seconds();

float SaveTimeMs = (EndTime - StartTime) * 1000.0f;
UE_LOG(LogTrading, Log, TEXT("Save completed in %.2f ms"), SaveTimeMs);

if (SaveTimeMs > 100.0f)
{
    UE_LOG(LogTrading, Warning, TEXT("Save took longer than 100ms!"));
}
```

### Load Performance

**Target**: <500ms load time

**Optimization**:
- Load on separate thread
- Stream world load (async level streaming)
- Cache frequently accessed data

---

## Summary

**Save Data**:
- Credits, earnings, ship, cargo, location
- Statistics and progression flags
- Tutorial completion state

**Save Slots**:
- Single slot for MVP ("TradingSimSlot_01")
- Auto-save on trades, upgrades, docking
- Manual save in pause menu

**Main Menu**:
- Continue (load save)
- New Game (overwrite save)
- Settings, Quit

**Error Handling**:
- Validate save data
- Handle corruption gracefully
- Show clear error messages

**Performance**:
- <100ms save time
- <500ms load time
- Async operations

---

**Design Goal**: "Set it and forget it" - automatic, reliable saves with manual option

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 9-12 Polish & Demo)
