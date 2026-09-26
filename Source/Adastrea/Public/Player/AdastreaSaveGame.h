// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/PlayerProgressionComponent.h"
// REMOVED: #include "Player/PlayerReputationComponent.h" - faction reputation system removed per Trade Simulator MVP
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
 * One stack of cargo in the player's hold (save version 2+)
 */
USTRUCT(BlueprintType)
struct FSavedCargoEntry
{
	GENERATED_BODY()

	/** Item ID (UTradeItemDataAsset::ItemID) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName ItemID;

	/** Asset path for hand-authored items (DA_/TradeItem_ assets). Empty for the
	 *  crafting-tree items built at runtime, which are resolved by ItemID instead. */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FSoftObjectPath ItemAsset;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 Quantity;

	FSavedCargoEntry()
		: ItemID(NAME_None)
		, Quantity(0)
	{}
};

/**
 * The ship the player flies: class, credits, hold and docking (save version 2+)
 */
USTRUCT(BlueprintType)
struct FSavedPlayerShip
{
	GENERATED_BODY()

	/** False when the save was made away from the helm (e.g. walking the interior) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	bool bValid;

	/** Ship pawn class (e.g. BP_Fighter) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FSoftClassPath ShipClass;

	/** USpaceshipDataAsset the ship was flying with */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FSoftObjectPath ShipDataAsset;

	/** UPlayerTraderComponent credits (the wallet the HUD and markets use) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 Credits;

	/** UPlayerTraderComponent starting credits (profit baseline) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 StartingCredits;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	TArray<FSavedCargoEntry> Cargo;

	UPROPERTY(BlueprintReadWrite, Category="Save")
	bool bDocked;

	/** Docked at a level station: its actor name */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName DockedStationName;

	/** Docked at a player-built station: index into UAdastreaSaveGame::Stations (else INDEX_NONE) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 DockedSavedStationIndex;

	/** Which of the station's docking bays (ASpaceStation::GetDockingBayModules order) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	int32 DockedBayIndex;

	FSavedPlayerShip()
		: bValid(false)
		, Credits(0)
		, StartingCredits(0)
		, bDocked(false)
		, DockedStationName(NAME_None)
		, DockedSavedStationIndex(INDEX_NONE)
		, DockedBayIndex(0)
	{}
};

/**
 * A station the player built or extended in the Station Editor (save version 2+).
 * Modules are stored as a Station Editor blueprint string (ASpaceStation::ExportBlueprintString).
 */
USTRUCT(BlueprintType)
struct FSavedStation
{
	GENERATED_BODY()

	/** True: spawned by the player's builder, respawned on load. False: a level station the player added modules to. */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	bool bPlayerBuilt;

	/** Level station actor name (bPlayerBuilt == false) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FName StationActorName;

	/** Station class to respawn (bPlayerBuilt == true) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FSoftClassPath StationClass;

	/** Station world transform (bPlayerBuilt == true) */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FTransform StationTransform;

	/** Player-built modules, as a blueprint string */
	UPROPERTY(BlueprintReadWrite, Category="Save")
	FString Blueprint;

	FSavedStation()
		: bPlayerBuilt(false)
		, StationActorName(NAME_None)
	{}
};

/**
 * Saved world state data
 */
USTRUCT(BlueprintType)
struct FSavedWorldState
{
	GENERATED_BODY()

	// REMOVED: FactionRelationships - faction system removed per Trade Simulator MVP

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
 * - Unlocked content (ships, modules, equipment)
 * - Achievement progress
 * - Quest states
 * - Ship customizations
 * - World state (markets, discoveries)
 * - Game settings and options
 * - Flown ship: class, data asset, trader credits, cargo (incl. mined ore), docking (v2)
 * - Player-built stations / modules as Station Editor blueprint strings (v2)
 *
 * Versioning: see CURRENT_SAVE_VERSION. Older saves load; fields they predate are skipped.
 *
 * REMOVED (Trade Simulator MVP):
 * - Faction reputation (removed per MVP scope)
 * - Faction relations (removed per MVP scope)
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

	/** The flown ship: class, data asset, trader credits, cargo, docking (version 2+) */
	UPROPERTY(BlueprintReadWrite, Category="Save|Player")
	FSavedPlayerShip PlayerShip;

	/** Player-built stations and player-added modules on level stations (version 2+) */
	UPROPERTY(BlueprintReadWrite, Category="Save|World")
	TArray<FSavedStation> Stations;

	// REMOVED: Reputation Data - faction reputation system removed per Trade Simulator MVP

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

	/**
	 * Current save game version.
	 * 1: progression, GameInstance credits, pawn transform, unlocks, achievements.
	 * 2: + PlayerShip (class, data asset, trader credits, cargo, docking) and Stations.
	 * Fields added after a save's version keep their defaults and are not applied on load.
	 */
	static constexpr int32 CURRENT_SAVE_VERSION = 2;

	/** Oldest version LoadGame still accepts */
	static constexpr int32 MIN_SUPPORTED_SAVE_VERSION = 1;

	/** Constructor default for SaveVersion, so files that never wrote it (all of version 1) read back as 1 */
	static constexpr int32 LEGACY_SAVE_VERSION = 1;

	/** First version that records PlayerShip and Stations */
	static constexpr int32 SHIP_AND_STATIONS_SAVE_VERSION = 2;
};
