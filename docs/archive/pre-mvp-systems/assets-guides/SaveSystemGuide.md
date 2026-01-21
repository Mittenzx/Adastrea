# Save System - Complete Guide

> **Version**: 1.0.0  
> **Last Updated**: November 2025  
> **System**: Persistence & Save/Load

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [AdastreaSaveGame](#adastreasavegame)
- [SaveGameSubsystem](#savegamesubsystem)
- [Usage Examples](#usage-examples)
- [Save Data Structure](#save-data-structure)
- [Auto-Save System](#auto-save-system)
- [Cloud Save Support](#cloud-save-support)
- [Best Practices](#best-practices)

---

## Overview

The Save System provides comprehensive persistence for all player and world state data. Built on Unreal Engine's `USaveGame` class with custom serialization support.

### Features

- **Multiple Save Slots**: Support for unlimited save files
- **Auto-Save**: Configurable automatic saving
- **Quick Save/Load**: Dedicated quick save slot
- **Version Control**: Save compatibility checking
- **Metadata**: Save file information for UI display
- **Comprehensive State**: Player, world, quests, achievements, inventory
- **Validation**: Save file integrity checking
- **Playtime Tracking**: Accurate session time tracking

---

## Architecture

### Core Classes

```
UAdastreaSaveGame (USaveGame)
├─ Save metadata and versioning
├─ Player progression data
├─ Reputation and unlocks
├─ Achievement progress
├─ Quest states
├─ Ship customizations
├─ World state
└─ Game settings

USaveGameSubsystem (UGameInstanceSubsystem)
├─ Save/load operations
├─ Auto-save management
├─ Slot management
├─ Data collection/application
└─ Validation

UAdastreaGameInstance
├─ High-level save/load interface
└─ System coordination
```

### Data Flow

```
Save Operation:
Player Action → SaveGameSubsystem
              → CollectGameState()
              → Serialize components
              → Validate data
              → Write to disk
              → Broadcast OnGameSaved

Load Operation:
User Selection → SaveGameSubsystem
               → Read from disk
               → Validate version
               → ApplyGameState()
               → Restore components
               → Broadcast OnGameLoaded
```

---

## AdastreaSaveGame

### Save Data Categories

#### 1. Metadata
```cpp
FString SaveSlotName;           // "SaveSlot_0", "QuickSave", etc.
int32 SaveVersion;               // Version for compatibility
FDateTime SaveTimestamp;         // When save was created
FString PlayerName;              // Character name
FString CurrentLevelName;        // Current map
float TotalPlaytimeSeconds;      // Total play time
```

#### 2. Player Data
```cpp
FSavedPlayerProgression PlayerProgression;  // Level, XP, skills
int32 PlayerCredits;                        // Current credits
FVector PlayerLocation;                     // World position
FRotator PlayerRotation;                    // Orientation
FName CurrentShipID;                        // Active ship
```

#### 3. Reputation
```cpp
TArray<FPlayerFactionReputation> FactionReputations;  // All faction standings
```

#### 4. Unlocks & Achievements
```cpp
TArray<FName> UnlockedContentIDs;                    // Unlocked ships/modules/etc
TArray<FSavedAchievementProgress> AchievementProgress;  // Achievement tracking
TArray<FName> CompletedAchievements;                 // Completed achievement IDs
TMap<FName, int32> AchievementStats;                 // Gameplay statistics
```

#### 5. Quests
```cpp
TArray<FSavedQuestState> QuestStates;  // All quest progress
```

#### 6. Inventory & Equipment
```cpp
TMap<FName, int32> InventoryItems;            // Item quantities
TArray<FSavedShipCustomization> ShipCustomizations;  // Ship loadouts
TArray<FName> OwnedShips;                     // Player's ships
```

#### 7. World State
```cpp
FSavedWorldState WorldState;
├─ TMap<FString, int32> FactionRelationships  // Inter-faction relations
├─ TMap<FName, int32> MarketPrices            // Current market prices
├─ TArray<FName> DiscoveredLocations          // Explored locations
└─ float GameTimeSeconds                       // In-game time
```

#### 8. Settings
```cpp
int32 DifficultyLevel;          // Game difficulty
bool bAutoSaveEnabled;           // Auto-save on/off
int32 AutoSaveIntervalMinutes;   // Auto-save frequency
```

### Helper Functions

```cpp
// Get formatted display info
FText GetDisplayName() const;     // "Player - Level 25"
FText GetDescription() const;      // "City Map | 10:30:45 | 2025-11-15"
FText GetFormattedPlaytime() const;  // "10:30:45"

// Version checking
bool IsCompatibleVersion() const;  // Check save compatibility
```

---

## SaveGameSubsystem

### Core Functions

#### Save Operations

```cpp
// Save to specific slot
bool SaveGame(const FString& SlotName, bool bUpdatePlaytime = true);

// Quick save to dedicated slot
bool QuickSave();

// Auto-save (called by timer)
bool AutoSave();

// Delete save file
bool DeleteSave(const FString& SlotName);
```

#### Load Operations

```cpp
// Load from specific slot
bool LoadGame(const FString& SlotName);

// Quick load from dedicated slot
bool QuickLoad();
```

#### Query Functions

```cpp
// Check if save exists
bool DoesSaveExist(const FString& SlotName) const;

// Get save slot information
bool GetSaveSlotInfo(const FString& SlotName, FSaveSlotInfo& OutSlotInfo) const;

// Get all save slots
TArray<FSaveSlotInfo> GetAllSaveSlots(int32 MaxSlots = 10) const;

// Get current save
UAdastreaSaveGame* GetCurrentSaveGame() const;

// Check if save is loaded
bool IsSaveLoaded() const;
```

#### Auto-Save Management

```cpp
// Enable auto-save with interval (seconds)
void EnableAutoSave(float IntervalSeconds = 600.0f);

// Disable auto-save
void DisableAutoSave();

// Reset auto-save timer
void ResetAutoSaveTimer();
```

### Events

```cpp
// Called when game is saved
UPROPERTY(BlueprintAssignable)
FOnGameSaved OnGameSaved;  // Params: FString SlotName

// Called when game is loaded
UPROPERTY(BlueprintAssignable)
FOnGameLoaded OnGameLoaded;  // Params: FString SlotName

// Called when save fails
UPROPERTY(BlueprintAssignable)
FOnSaveFailed OnSaveFailed;  // Params: FString SlotName, FText Reason

// Called when auto-save triggers
UPROPERTY(BlueprintAssignable)
FOnAutoSaveTriggered OnAutoSaveTriggered;
```

---

## Usage Examples

### C++ Examples

#### Basic Save

```cpp
// Get save system
USaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

// Save to slot 1
if (SaveSystem->SaveGame("SaveSlot_1"))
{
    UE_LOG(LogGame, Log, TEXT("Game saved successfully"));
}
```

#### Basic Load

```cpp
USaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

// Load from slot 1
if (SaveSystem->LoadGame("SaveSlot_1"))
{
    UE_LOG(LogGame, Log, TEXT("Game loaded successfully"));
}
```

#### Query Save Slots

```cpp
USaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

// Get all saves
TArray<FSaveSlotInfo> SaveSlots = SaveSystem->GetAllSaveSlots();

for (const FSaveSlotInfo& SlotInfo : SaveSlots)
{
    UE_LOG(LogGame, Log, TEXT("Save: %s - Level %d - %s"),
        *SlotInfo.PlayerName,
        SlotInfo.PlayerLevel,
        *SlotInfo.SaveTimestamp.ToString());
}
```

### Blueprint Examples

#### Save Game

```blueprint
// Manual save
Function Save Game To Slot
├─ Parameters: Slot Index (Integer)
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        └─> Save Game
            ├─ Slot Name: Append("SaveSlot_", Slot Index)
            ├─ Update Playtime: True
            └─> Branch: Success?
                ├─ True: Show Message "Game Saved"
                └─ False: Show Message "Save Failed"
```

#### Load Game

```blueprint
// Load from specific slot
Function Load Game From Slot
├─ Parameters: Slot Index (Integer)
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        ├─> Does Save Exist?
        │   ├─ Slot Name: Append("SaveSlot_", Slot Index)
        │   └─> Branch: Exists?
        │       └─ True: Load Game
        │           └─> Open Level (from save data)
        └─ False: Show Message "No Save File"
```

#### Display Save Slots in UI

```blueprint
// Populate save slot list
Function Update Save Slot List
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        └─> Get All Save Slots
            ├─ Max Slots: 10
            └─> For Each Slot Info
                └─> Create Widget: Save Slot Entry
                    ├─ Slot Name: Slot Info.Slot Name
                    ├─ Player Name: Slot Info.Player Name
                    ├─ Level: Slot Info.Player Level
                    ├─ Timestamp: Slot Info.Save Timestamp
                    ├─ Playtime: Format Seconds(Slot Info.Playtime Seconds)
                    └─> Add to List View
```

#### Quick Save/Load

```blueprint
// Quick save button
Event On Quick Save Clicked
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        └─> Quick Save
            └─> Branch: Success?
                ├─ True: 
                │   ├─> Play Sound: Save Sound
                │   └─> Show Widget: "Game Saved"
                └─ False: Show Widget: "Save Failed"

// Quick load button
Event On Quick Load Clicked
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        └─> Does Save Exist?
            ├─ Slot Name: "QuickSave"
            └─> Branch: Exists?
                ├─ True: Quick Load
                │   └─> Show Widget: "Loading..."
                └─ False: Show Widget: "No Quick Save"
```

#### Auto-Save Setup

```blueprint
// Enable auto-save on game start
Event Begin Play (Game Mode)
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        └─> Enable Auto Save
            └─ Interval Seconds: 600  // 10 minutes
```

#### Listen to Save Events

```blueprint
// Bind to save events
Event Begin Play
└─> Get Game Instance
    └─> Get Subsystem (Save Game Subsystem)
        ├─> Bind Event to On Game Saved
        │   └─> Function: Handle Game Saved
        │       └─> Show Notification: "Game Saved"
        ├─> Bind Event to On Auto Save Triggered
        │   └─> Function: Handle Auto Save
        │       └─> Show Icon: Auto Save Icon
        └─> Bind Event to On Save Failed
            └─> Function: Handle Save Failed
                └─> Show Error: Reason
```

---

## Save Data Structure

### Custom Serialization

Add custom data to save system:

#### 1. Create Save Struct

```cpp
USTRUCT(BlueprintType)
struct FCustomGameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 CustomValue;

    UPROPERTY(BlueprintReadWrite)
    TArray<FName> CustomArray;

    FCustomGameData()
        : CustomValue(0)
    {}
};
```

#### 2. Add to SaveGame

```cpp
// In AdastreaSaveGame.h
UPROPERTY(BlueprintReadWrite, Category="Save|Custom")
FCustomGameData CustomData;
```

#### 3. Collect Data

```cpp
// In SaveGameSubsystem::CollectGameState()
SaveGameObject->CustomData.CustomValue = YourSystem->GetValue();
SaveGameObject->CustomData.CustomArray = YourSystem->GetArray();
```

#### 4. Apply Data

```cpp
// In SaveGameSubsystem::ApplyGameState()
YourSystem->SetValue(SaveGameObject->CustomData.CustomValue);
YourSystem->SetArray(SaveGameObject->CustomData.CustomArray);
```

### Component Serialization

Make components saveable:

```cpp
// In your component
UFUNCTION(BlueprintCallable, Category="Save")
void SerializeState(UAdastreaSaveGame* SaveGame)
{
    // Save component state
    SaveGame->CustomData.ComponentValue = MyValue;
}

UFUNCTION(BlueprintCallable, Category="Save")
void DeserializeState(UAdastreaSaveGame* SaveGame)
{
    // Restore component state
    MyValue = SaveGame->CustomData.ComponentValue;
}
```

---

## Auto-Save System

### Configuration

```cpp
// In project settings or game instance
USaveGameSubsystem* SaveSystem = GetSubsystem<USaveGameSubsystem>();

// Enable with custom interval (10 minutes)
SaveSystem->EnableAutoSave(600.0f);

// Or disable
SaveSystem->DisableAutoSave();
```

### Auto-Save Triggers

Auto-save occurs:
- **Timer-based**: Every X seconds (configurable)
- **Event-based**: After significant events (optional)
  - Quest completion
  - Level up
  - Major progress milestones

```cpp
// Trigger auto-save after quest completion
void OnQuestCompleted()
{
    USaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();
    if (SaveSystem)
    {
        SaveSystem->ResetAutoSaveTimer();  // Reset timer
        SaveSystem->AutoSave();             // Save immediately
    }
}
```

### Auto-Save UI

```blueprint
// Show auto-save indicator
Event On Auto Save Triggered
└─> Create Widget: Auto Save Icon
    ├─> Set Position: Bottom Right
    ├─> Play Animation: Fade In
    ├─> Delay: 2 seconds
    └─> Play Animation: Fade Out
```

---

## Cloud Save Support

### Platform Integration

For Steam/Epic/Console cloud saves:

#### 1. Enable Platform Plugin

```cpp
// In .uproject
"Plugins": [
    {
        "Name": "OnlineSubsystemSteam",
        "Enabled": true
    }
]
```

#### 2. Use Platform Save API

```cpp
// Steam Cloud example (future implementation)
#include "OnlineSubsystemSteam.h"

bool USaveGameSubsystem::SaveToCloud(const FString& SlotName)
{
    // Platform-specific cloud save implementation
    // Would use Steam Remote Storage API or equivalent
    return false; // Not yet implemented
}
```

#### 3. Sync on Load

```cpp
// Check cloud for newer save
bool USaveGameSubsystem::SyncWithCloud(const FString& SlotName)
{
    // Compare local vs cloud timestamps
    // Download if cloud is newer
    // Upload if local is newer
    return false; // Not yet implemented
}
```

---

## Best Practices

### When to Save

**Good Times:**
- Player-initiated (manual save)
- After major progress (quest completion, level up)
- Before risky actions (entering combat zone)
- On game exit
- Auto-save intervals

**Bad Times:**
- During combat
- While loading
- During cutscenes
- In middle of transition
- When player is dead

### Save Frequency

```cpp
// Recommended intervals
const float AUTOSAVE_INTERVAL_CASUAL = 300.0f;   // 5 minutes
const float AUTOSAVE_INTERVAL_NORMAL = 600.0f;   // 10 minutes
const float AUTOSAVE_INTERVAL_HARDCORE = 1200.0f; // 20 minutes
```

### Save Slot Management

```cpp
// Default slot naming convention
FString GetSlotName(int32 Index)
{
    return FString::Printf(TEXT("SaveSlot_%d"), Index);
}

// Special slots
const FString QUICKSAVE_SLOT = TEXT("QuickSave");
const FString AUTOSAVE_SLOT = TEXT("AutoSave");
const FString MANUAL_SLOT_PREFIX = TEXT("SaveSlot_");
```

### Error Handling

```cpp
// Robust save with error handling
bool USaveGameSubsystem::SaveGameSafe(const FString& SlotName)
{
    if (SlotName.IsEmpty())
    {
        UE_LOG(LogSave, Error, TEXT("Invalid slot name"));
        return false;
    }

    // Collect game state
    if (!CurrentSaveGame)
    {
        CurrentSaveGame = Cast<UAdastreaSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UAdastreaSaveGame::StaticClass())
        );
    }

    if (!CurrentSaveGame)
    {
        UE_LOG(LogSave, Error, TEXT("Failed to create save object"));
        return false;
    }

    CollectGameState(CurrentSaveGame);

    // Validate before saving
    if (!ValidateSaveGame(CurrentSaveGame))
    {
        UE_LOG(LogSave, Error, TEXT("Save validation failed"));
        return false;
    }

    // Perform save
    if (!UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0))
    {
        UE_LOG(LogSave, Error, TEXT("Failed to write save file"));
        return false;
    }

    UE_LOG(LogSave, Log, TEXT("Game saved: %s"), *SlotName);
    return true;
}
```

### Version Migration

```cpp
// Handle old save versions
bool UAdastreaSaveGame::MigrateFromVersion(int32 OldVersion)
{
    if (OldVersion < 1)
    {
        // Migrate from version 0 to 1
        // Add default values for new fields
        return true;
    }

    return false;
}
```

### Testing Saves

```cpp
// Test save/load cycle
void TestSaveLoad()
{
    USaveGameSubsystem* SaveSystem = GetSubsystem<USaveGameSubsystem>();
    
    // Save current state
    FString TestSlot = TEXT("TestSave");
    if (SaveSystem->SaveGame(TestSlot))
    {
        // Modify state
        ModifyGameState();
        
        // Load previous state
        if (SaveSystem->LoadGame(TestSlot))
        {
            // Verify state restored
            VerifyGameState();
            
            // Cleanup
            SaveSystem->DeleteSave(TestSlot);
        }
    }
}
```

---

## Troubleshooting

### Save Not Working

**Check:**
- Slot name not empty
- Save directory writable
- Sufficient disk space
- No pending operations
- Valid save data

**Debug:**
```cpp
// Enable detailed logging
UE_LOG(LogSave, Log, TEXT("Saving to: %s"), *SlotName);
UE_LOG(LogSave, Log, TEXT("Save data size: %d bytes"), SaveDataSize);
```

### Load Failing

**Check:**
- Save file exists
- Version compatible
- Data not corrupted
- All systems initialized

**Validation:**
```cpp
// Verify save file integrity
bool ValidateSave(const FString& SlotName)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
        return false;

    UAdastreaSaveGame* TestLoad = Cast<UAdastreaSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!TestLoad)
        return false;

    if (!TestLoad->IsCompatibleVersion())
        return false;

    return true;
}
```

### Auto-Save Issues

**Check:**
- Auto-save enabled
- Timer active
- Interval valid (>= 60 seconds)
- No conflicting saves

### Performance Issues

**Optimize:**
- Save on background thread (future)
- Compress large data
- Limit save frequency
- Clean up old saves

```cpp
// Cleanup old auto-saves
void CleanupOldSaves(int32 MaxAutoSaves = 5)
{
    // Keep only last N auto-saves
    // Delete older files
}
```

---

## Next Steps

1. **Test Save/Load**: Verify all systems save correctly
2. **UI Implementation**: Create save/load menus
3. **Cloud Integration**: Add platform cloud save support
4. **Backup System**: Implement save file backup
5. **Analytics**: Track save/load metrics

## Related Documentation

- [PlayerProgressionGuide.md](PlayerProgressionGuide.md) - Progression system integration
- [QuestSystemGuide.md](QuestSystemGuide.md) - Quest state serialization
- [ARCHITECTURE.md](../ARCHITECTURE.md) - System architecture

---

**Last Updated**: November 2025  
**Version**: 1.0.0  
**Maintained by**: Adastrea Development Team
