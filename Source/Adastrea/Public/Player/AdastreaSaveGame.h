#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/PlayerProgressionComponent.h"
#include "Player/PlayerReputationComponent.h"
#include "Player/PlayerUnlockComponent.h"
#include "Player/AchievementDataAsset.h"
#include "AdastreaSaveGame.generated.h"

/**
 * Saved player progression data
 */
USTRUCT(BlueprintType)
struct FSavedPlayerProgression
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 PlayerLevel;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 CurrentXP;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 TotalXPEarned;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 AvailableSkillPoints;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FPlayerSkill> Skills;

	FSavedPlayerProgression()
		: PlayerLevel(1)
		, CurrentXP(0)
		, TotalXPEarned(0)
		, AvailableSkillPoints(0)
	{}
};

/**
 * Saved achievement progress
 */
USTRUCT(BlueprintType)
struct FSavedAchievementProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName AchievementID;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	FAchievementProgress Progress;

	FSavedAchievementProgress()
		: AchievementID(NAME_None)
	{}
};

/**
 * Saved quest state
 */
USTRUCT(BlueprintType)
struct FSavedQuestState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName QuestID;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 QuestStatus; // Using int32 instead of enum for serialization safety

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<int32> ObjectiveProgress;

	FSavedQuestState()
		: QuestID(NAME_None)
		, QuestStatus(0)
	{}
};

/**
 * Saved ship customization data
 */
USTRUCT(BlueprintType)
struct FSavedShipCustomization
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName ShipID;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FName> InstalledUpgrades;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FName> EquippedWeapons;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FName, FString> CustomizationOptions;

	FSavedShipCustomization()
		: ShipID(NAME_None)
	{}
};

/**
 * Saved world state data
 */
USTRUCT(BlueprintType)
struct FSavedWorldState
{
	GENERATED_BODY()

	/** Faction relationship values (FactionA_FactionB -> relationship value) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FString, int32> FactionRelationships;

	/** Market prices (ItemID -> price) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	TMap<FName, int32> MarketPrices;

	/** Discovered locations */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FName> DiscoveredLocations;

	/** In-game time (seconds since game start) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	float GameTimeSeconds;

	FSavedWorldState()
		: GameTimeSeconds(0.0f)
	{}
};

/**
 * Main save game class for Adastrea.
 * Stores all persistent player and world state data.
 * 
 * Usage:
 * - Create via UGameplayStatics::CreateSaveGameObject()
 * - Populate with current game state
 * - Save via UGameplayStatics::SaveGameToSlot()
 * - Load via UGameplayStatics::LoadGameFromSlot()
 * 
 * Stored Data:
 * - Player progression (level, XP, skills)
 * - Reputation with all factions
 * - Unlocked content (ships, modules, equipment)
 * - Achievement progress
 * - Quest states
 * - Ship customizations
 * - World state (faction relations, markets, discoveries)
 * - Game settings and options
 * 
 * Integration:
 * - SaveGameSubsystem handles save/load operations
 * - AdastreaGameInstance coordinates system restoration
 * - All progression components serialize their state
 */
UCLASS()
class ADASTREA_API UAdastreaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAdastreaSaveGame();

	// ====================
	// Save Metadata
	// ====================

	/** Save slot name */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	FString SaveSlotName;

	/** Save game version (for migration handling) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	int32 SaveVersion;

	/** Timestamp when save was created */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	FDateTime SaveTimestamp;

	/** Player name/character name */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	FString PlayerName;

	/** Current level/map name */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	FString CurrentLevelName;

	/** Playtime in seconds */
	UPROPERTY(BlueprintReadWrite, Category="Save|Metadata")
	float TotalPlaytimeSeconds;

	// ====================
	// Player Data
	// ====================

	/** Player progression (level, XP, skills) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	FSavedPlayerProgression PlayerProgression;

	/** Player credits */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	int32 PlayerCredits;

	/** Player location */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	FVector PlayerLocation;

	/** Player rotation */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	FRotator PlayerRotation;

	/** Current ship ID */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	FName CurrentShipID;

	// ====================
	// Reputation Data
	// ====================

	/** All faction reputations */
	UPROPERTY(BlueprintReadWrite, Category="Save|Reputation")
	TArray<FPlayerFactionReputation> FactionReputations;

	// ====================
	// Unlocks & Achievements
	// ====================

	/** Unlocked content IDs */
	UPROPERTY(BlueprintReadWrite, Category="Save|Unlocks")
	TArray<FName> UnlockedContentIDs;

	/** Achievement progress */
	UPROPERTY(BlueprintReadWrite, Category="Save|Achievements")
	TArray<FSavedAchievementProgress> AchievementProgress;

	/** Completed achievement IDs */
	UPROPERTY(BlueprintReadWrite, Category="Save|Achievements")
	TArray<FName> CompletedAchievements;

	/** Achievement statistics */
	UPROPERTY(BlueprintReadWrite, Category="Save|Achievements")
	TMap<FName, int32> AchievementStats;

	// ====================
	// Quest Data
	// ====================

	/** Quest states (active, completed, failed) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Quests")
	TArray<FSavedQuestState> QuestStates;

	// ====================
	// Inventory & Equipment
	// ====================

	/** Inventory items (ItemID -> quantity) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Inventory")
	TMap<FName, int32> InventoryItems;

	/** Ship customizations */
	UPROPERTY(BlueprintReadWrite, Category="Save|Ships")
	TArray<FSavedShipCustomization> ShipCustomizations;

	/** Owned ship IDs */
	UPROPERTY(BlueprintReadWrite, Category="Save|Ships")
	TArray<FName> OwnedShips;

	// ====================
	// World State
	// ====================

	/** World state data (faction relations, markets, etc.) */
	UPROPERTY(BlueprintReadWrite, Category="Save|World")
	FSavedWorldState WorldState;

	// ====================
	// Game Settings
	// ====================

	/** Game difficulty setting */
	UPROPERTY(BlueprintReadWrite, Category="Save|Settings")
	int32 DifficultyLevel;

	/** Auto-save enabled */
	UPROPERTY(BlueprintReadWrite, Category="Save|Settings")
	bool bAutoSaveEnabled;

	/** Auto-save interval (minutes) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Settings")
	int32 AutoSaveIntervalMinutes;

	// ====================
	// Helper Functions
	// ====================

	/**
	 * Get save game display name for UI
	 * @return Formatted display name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
	FText GetDisplayName() const;

	/**
	 * Get save game description for UI
	 * @return Formatted description
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
	FText GetDescription() const;

	/**
	 * Get playtime as formatted string
	 * @return Formatted playtime (HH:MM:SS)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
	FText GetFormattedPlaytime() const;

	/**
	 * Check if this save is compatible with current game version
	 * @return True if compatible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
	bool IsCompatibleVersion() const;

	// ====================
	// Constants
	// ====================

	/** Current save game version */
	static constexpr int32 CURRENT_SAVE_VERSION = 1;
};
