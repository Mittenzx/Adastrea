#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/AdastreaSaveGame.h"
#include "SaveGameSubsystem.generated.h"

/**
 * Save slot info for UI display
 */
USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Save")
	FString SlotName;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	int32 PlayerLevel;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	FDateTime SaveTimestamp;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	float PlaytimeSeconds;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	bool bExists;

	UPROPERTY(BlueprintReadOnly, Category="Save")
	bool bIsCompatible;

	FSaveSlotInfo()
		: SlotName("Empty")
		, PlayerName("Empty")
		, PlayerLevel(1)
		, SaveTimestamp(FDateTime::MinValue())
		, PlaytimeSeconds(0.0f)
		, bExists(false)
		, bIsCompatible(true)
	{}
};

/**
 * Save Game Subsystem
 * 
 * Manages all save/load operations for the game.
 * Operates as a Game Instance Subsystem to persist across level transitions.
 * 
 * Features:
 * - Multiple save slots
 * - Auto-save functionality
 * - Quick save/load
 * - Save file validation
 * - Async save/load operations
 * - Cloud save support (optional)
 * 
 * Usage:
 * - Access via UGameInstance::GetSubsystem<USaveGameSubsystem>()
 * - Call SaveGame() to save current state
 * - Call LoadGame() to restore from save
 * - Enable auto-save via EnableAutoSave()
 * - Query save slots via GetSaveSlotInfo()
 * 
 * Example:
 * - Quick save: QuickSave()
 * - Quick load: QuickLoad()
 * - Save to slot: SaveGame("Slot1")
 * - Load from slot: LoadGame("Slot1")
 * - List saves: GetAllSaveSlots()
 * 
 * Integration:
 * - AdastreaGameInstance coordinates save/load
 * - PlayerProgressionComponent serializes progression
 * - PlayerReputationComponent serializes reputation
 * - AchievementManagerSubsystem serializes achievements
 * - QuestManagerSubsystem serializes quest states
 */
UCLASS()
class ADASTREA_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USaveGameSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// Core Properties
	// ====================

	/** Current save game object */
	UPROPERTY()
	UAdastreaSaveGame* CurrentSaveGame;

	/** Active save slot name */
	UPROPERTY(BlueprintReadOnly, Category="Save")
	FString CurrentSaveSlot;

	/** Auto-save enabled */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	bool bAutoSaveEnabled;

	/** Auto-save interval (seconds) */
	UPROPERTY(BlueprintReadWrite, Category="Save", meta=(ClampMin="60", ClampMax="3600"))
	float AutoSaveIntervalSeconds;

	/** Quick save slot name */
	UPROPERTY(BlueprintReadOnly, Category="Save")
	FString QuickSaveSlotName;

	/** Auto-save slot name */
	UPROPERTY(BlueprintReadOnly, Category="Save")
	FString AutoSaveSlotName;

	// ====================
	// Save Operations
	// ====================

	/**
	 * Save game to specified slot
	 * @param SlotName Save slot name
	 * @param bUpdatePlaytime Whether to update total playtime
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveGame(const FString& SlotName, bool bUpdatePlaytime = true);

	/**
	 * Load game from specified slot
	 * @param SlotName Save slot name
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadGame(const FString& SlotName);

	/**
	 * Delete save from specified slot
	 * @param SlotName Save slot name
	 * @return True if delete was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool DeleteSave(const FString& SlotName);

	/**
	 * Quick save to dedicated quick save slot
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool QuickSave();

	/**
	 * Quick load from dedicated quick save slot
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool QuickLoad();

	/**
	 * Perform auto-save
	 * Called automatically by timer
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool AutoSave();

	// ====================
	// Query Functions
	// ====================

	/**
	 * Check if save exists in slot
	 * @param SlotName Save slot name
	 * @return True if save exists
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|Query")
	bool DoesSaveExist(const FString& SlotName) const;

	/**
	 * Get save slot information
	 * @param SlotName Save slot name
	 * @param OutSlotInfo Output slot information
	 * @return True if slot exists
	 */
	UFUNCTION(BlueprintCallable, Category="Save|Query")
	bool GetSaveSlotInfo(const FString& SlotName, FSaveSlotInfo& OutSlotInfo) const;

	/**
	 * Get all available save slots
	 * @param MaxSlots Maximum number of slots to return (0 = all)
	 * @return Array of save slot information
	 */
	UFUNCTION(BlueprintCallable, Category="Save|Query")
	TArray<FSaveSlotInfo> GetAllSaveSlots(int32 MaxSlots = 10) const;

	/**
	 * Get current active save game
	 * @return Current save game object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|Query")
	UAdastreaSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	/**
	 * Check if a save is currently loaded
	 * @return True if save is loaded
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|Query")
	bool IsSaveLoaded() const { return CurrentSaveGame != nullptr; }

	// ====================
	// Auto-Save Management
	// ====================

	/**
	 * Enable auto-save with specified interval
	 * @param IntervalSeconds Time between auto-saves (in seconds)
	 */
	UFUNCTION(BlueprintCallable, Category="Save|AutoSave")
	void EnableAutoSave(float IntervalSeconds = 600.0f);

	/**
	 * Disable auto-save
	 */
	UFUNCTION(BlueprintCallable, Category="Save|AutoSave")
	void DisableAutoSave();

	/**
	 * Reset auto-save timer
	 */
	UFUNCTION(BlueprintCallable, Category="Save|AutoSave")
	void ResetAutoSaveTimer();

	// ====================
	// Data Collection
	// ====================

	/**
	 * Collect all game state data into save game object
	 * @param SaveGameObject Save game to populate
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	void CollectGameState(UAdastreaSaveGame* SaveGameObject);

	/**
	 * Apply save game data to game state
	 * @param SaveGameObject Save game to restore from
	 */
	UFUNCTION(BlueprintCallable, Category="Save")
	void ApplyGameState(UAdastreaSaveGame* SaveGameObject);

	// ====================
	// Events
	// ====================

	/** Event fired when game is saved */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, FString, SlotName);
	UPROPERTY(BlueprintAssignable, Category="Save|Events")
	FOnGameSaved OnGameSaved;

	/** Event fired when game is loaded */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, FString, SlotName);
	UPROPERTY(BlueprintAssignable, Category="Save|Events")
	FOnGameLoaded OnGameLoaded;

	/** Event fired when save fails */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveFailed, FString, SlotName, FText, Reason);
	UPROPERTY(BlueprintAssignable, Category="Save|Events")
	FOnSaveFailed OnSaveFailed;

	/** Event fired when auto-save occurs */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAutoSaveTriggered);
	UPROPERTY(BlueprintAssignable, Category="Save|Events")
	FOnAutoSaveTriggered OnAutoSaveTriggered;

protected:
	/** Auto-save timer handle */
	FTimerHandle AutoSaveTimerHandle;

	/** Playtime tracking start time */
	FDateTime PlaytimeStartTime;

	/** Accumulated playtime (seconds) */
	float AccumulatedPlaytime;

	/**
	 * Validate save game object
	 * @param SaveGameObject Save to validate
	 * @return True if valid
	 */
	bool ValidateSaveGame(UAdastreaSaveGame* SaveGameObject) const;

	/**
	 * Get default slot name for numbered slots
	 * @param SlotIndex Slot index
	 * @return Formatted slot name
	 */
	FString GetDefaultSlotName(int32 SlotIndex) const;

	/**
	 * Update playtime in save game
	 * @param SaveGameObject Save to update
	 */
	void UpdatePlaytime(UAdastreaSaveGame* SaveGameObject);
};
