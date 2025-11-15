#include "Player/SaveGameSubsystem.h"
#include "Player/PlayerProgressionComponent.h"
#include "Player/PlayerReputationComponent.h"
#include "Player/PlayerUnlockComponent.h"
#include "Player/AchievementManagerSubsystem.h"
#include "Player/AdastreaGameInstance.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

USaveGameSubsystem::USaveGameSubsystem()
	: CurrentSaveGame(nullptr)
	, CurrentSaveSlot("")
	, bAutoSaveEnabled(true)
	, AutoSaveIntervalSeconds(600.0f) // 10 minutes default
	, QuickSaveSlotName("QuickSave")
	, AutoSaveSlotName("AutoSave")
	, AccumulatedPlaytime(0.0f)
{
}

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PlaytimeStartTime = FDateTime::Now();
	
	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Initialized"));
}

void USaveGameSubsystem::Deinitialize()
{
	DisableAutoSave();
	
	Super::Deinitialize();
	
	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Deinitialized"));
}

bool USaveGameSubsystem::SaveGame(const FString& SlotName, bool bUpdatePlaytime)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Cannot save - slot name is empty"));
		OnSaveFailed.Broadcast(SlotName, FText::FromString("Invalid slot name"));
		return false;
	}

	// Create or use existing save game object
	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<UAdastreaSaveGame>(UGameplayStatics::CreateSaveGameObject(UAdastreaSaveGame::StaticClass()));
	}

	if (!CurrentSaveGame)
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Failed to create save game object"));
		OnSaveFailed.Broadcast(SlotName, FText::FromString("Failed to create save object"));
		return false;
	}

	// Update metadata
	CurrentSaveGame->SaveSlotName = SlotName;
	CurrentSaveGame->SaveTimestamp = FDateTime::Now();
	CurrentSaveGame->SaveVersion = UAdastreaSaveGame::CURRENT_SAVE_VERSION;

	// Update playtime
	if (bUpdatePlaytime)
	{
		UpdatePlaytime(CurrentSaveGame);
	}

	// Get current level name
	UWorld* World = GetWorld();
	if (World)
	{
		CurrentSaveGame->CurrentLevelName = World->GetMapName();
	}

	// Collect game state
	CollectGameState(CurrentSaveGame);

	// Validate before saving
	if (!ValidateSaveGame(CurrentSaveGame))
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Save validation failed"));
		OnSaveFailed.Broadcast(SlotName, FText::FromString("Save validation failed"));
		return false;
	}

	// Save to slot
	bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
	
	if (bSuccess)
	{
		CurrentSaveSlot = SlotName;
		UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game saved to slot: %s"), *SlotName);
		OnGameSaved.Broadcast(SlotName);
	}
	else
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Failed to save game to slot: %s"), *SlotName);
		OnSaveFailed.Broadcast(SlotName, FText::FromString("Save operation failed"));
	}

	return bSuccess;
}

bool USaveGameSubsystem::LoadGame(const FString& SlotName)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Cannot load - slot name is empty"));
		return false;
	}

	if (!DoesSaveExist(SlotName))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Save does not exist: %s"), *SlotName);
		return false;
	}

	// Load from slot
	UAdastreaSaveGame* LoadedSave = Cast<UAdastreaSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	
	if (!LoadedSave)
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Failed to load game from slot: %s"), *SlotName);
		return false;
	}

	// Check version compatibility
	if (!LoadedSave->IsCompatibleVersion())
	{
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Save version mismatch (Save: %d, Current: %d)"),
			LoadedSave->SaveVersion, UAdastreaSaveGame::CURRENT_SAVE_VERSION);
		return false;
	}

	// Set as current save
	CurrentSaveGame = LoadedSave;
	CurrentSaveSlot = SlotName;

	// Apply game state
	ApplyGameState(CurrentSaveGame);

	// Reset playtime tracking
	PlaytimeStartTime = FDateTime::Now();
	AccumulatedPlaytime = CurrentSaveGame->TotalPlaytimeSeconds;

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game loaded from slot: %s"), *SlotName);
	OnGameLoaded.Broadcast(SlotName);

	return true;
}

bool USaveGameSubsystem::DeleteSave(const FString& SlotName)
{
	if (!DoesSaveExist(SlotName))
	{
		return false;
	}

	bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	
	if (bSuccess)
	{
		UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Deleted save from slot: %s"), *SlotName);
		
		// Clear current save if it was the deleted one
		if (CurrentSaveSlot == SlotName)
		{
			CurrentSaveGame = nullptr;
			CurrentSaveSlot = "";
		}
	}

	return bSuccess;
}

bool USaveGameSubsystem::QuickSave()
{
	return SaveGame(QuickSaveSlotName, true);
}

bool USaveGameSubsystem::QuickLoad()
{
	return LoadGame(QuickSaveSlotName);
}

bool USaveGameSubsystem::AutoSave()
{
	OnAutoSaveTriggered.Broadcast();
	return SaveGame(AutoSaveSlotName, true);
}

bool USaveGameSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool USaveGameSubsystem::GetSaveSlotInfo(const FString& SlotName, FSaveSlotInfo& OutSlotInfo) const
{
	if (!DoesSaveExist(SlotName))
	{
		OutSlotInfo.SlotName = SlotName;
		OutSlotInfo.bExists = false;
		return false;
	}

	UAdastreaSaveGame* SaveGame = Cast<UAdastreaSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGame)
	{
		return false;
	}

	OutSlotInfo.SlotName = SlotName;
	OutSlotInfo.PlayerName = SaveGame->PlayerName;
	OutSlotInfo.PlayerLevel = SaveGame->PlayerProgression.PlayerLevel;
	OutSlotInfo.SaveTimestamp = SaveGame->SaveTimestamp;
	OutSlotInfo.PlaytimeSeconds = SaveGame->TotalPlaytimeSeconds;
	OutSlotInfo.bExists = true;
	OutSlotInfo.bIsCompatible = SaveGame->IsCompatibleVersion();

	return true;
}

TArray<FSaveSlotInfo> USaveGameSubsystem::GetAllSaveSlots(int32 MaxSlots) const
{
	TArray<FSaveSlotInfo> SlotInfos;

	// Check numbered slots
	int32 SlotsToCheck = (MaxSlots > 0) ? MaxSlots : 100; // Check up to 100 slots if no limit
	
	for (int32 i = 0; i < SlotsToCheck; ++i)
	{
		FString SlotName = GetDefaultSlotName(i);
		FSaveSlotInfo SlotInfo;
		
		if (GetSaveSlotInfo(SlotName, SlotInfo))
		{
			SlotInfos.Add(SlotInfo);
		}
	}

	// Check special slots
	FSaveSlotInfo QuickSaveInfo;
	if (GetSaveSlotInfo(QuickSaveSlotName, QuickSaveInfo))
	{
		SlotInfos.Add(QuickSaveInfo);
	}

	FSaveSlotInfo AutoSaveInfo;
	if (GetSaveSlotInfo(AutoSaveSlotName, AutoSaveInfo))
	{
		SlotInfos.Add(AutoSaveInfo);
	}

	// Sort by timestamp (most recent first)
	SlotInfos.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B)
	{
		return A.SaveTimestamp > B.SaveTimestamp;
	});

	return SlotInfos;
}

void USaveGameSubsystem::EnableAutoSave(float IntervalSeconds)
{
	DisableAutoSave();
	
	bAutoSaveEnabled = true;
	AutoSaveIntervalSeconds = FMath::Max(60.0f, IntervalSeconds); // Minimum 1 minute

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			this,
			&USaveGameSubsystem::AutoSave,
			AutoSaveIntervalSeconds,
			true
		);
		
		UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Auto-save enabled (interval: %.0f seconds)"), AutoSaveIntervalSeconds);
	}
}

void USaveGameSubsystem::DisableAutoSave()
{
	bAutoSaveEnabled = false;

	UWorld* World = GetWorld();
	if (World && AutoSaveTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		AutoSaveTimerHandle.Invalidate();
		
		UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Auto-save disabled"));
	}
}

void USaveGameSubsystem::ResetAutoSaveTimer()
{
	if (bAutoSaveEnabled)
	{
		EnableAutoSave(AutoSaveIntervalSeconds);
	}
}

void USaveGameSubsystem::CollectGameState(UAdastreaSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get player controller and pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();

	// Save player location and rotation
	if (PlayerPawn)
	{
		SaveGameObject->PlayerLocation = PlayerPawn->GetActorLocation();
		SaveGameObject->PlayerRotation = PlayerPawn->GetActorRotation();
	}

	// Save player progression
	if (PlayerPawn)
	{
		UPlayerProgressionComponent* ProgressionComp = PlayerPawn->FindComponentByClass<UPlayerProgressionComponent>();
		if (ProgressionComp)
		{
			SaveGameObject->PlayerProgression.PlayerLevel = ProgressionComp->PlayerLevel;
			SaveGameObject->PlayerProgression.CurrentXP = ProgressionComp->CurrentXP;
			SaveGameObject->PlayerProgression.TotalXPEarned = ProgressionComp->TotalXPEarned;
			SaveGameObject->PlayerProgression.AvailableSkillPoints = ProgressionComp->AvailableSkillPoints;
			SaveGameObject->PlayerProgression.Skills = ProgressionComp->Skills;
		}

		// Save reputation
		UPlayerReputationComponent* ReputationComp = PlayerPawn->FindComponentByClass<UPlayerReputationComponent>();
		if (ReputationComp)
		{
			SaveGameObject->FactionReputations = ReputationComp->GetAllReputations();
		}

		// Save unlocks
		UPlayerUnlockComponent* UnlockComp = PlayerPawn->FindComponentByClass<UPlayerUnlockComponent>();
		if (UnlockComp)
		{
			SaveGameObject->UnlockedContentIDs = UnlockComp->UnlockedIDs;
		}
	}

	// Save achievements
	UAchievementManagerSubsystem* AchievementMgr = GetGameInstance()->GetSubsystem<UAchievementManagerSubsystem>();
	if (AchievementMgr)
	{
		SaveGameObject->CompletedAchievements = AchievementMgr->CompletedAchievements;
		SaveGameObject->AchievementStats = AchievementMgr->AchievementStats;

		// Save achievement progress
		SaveGameObject->AchievementProgress.Empty();
		for (const FAchievementTracker& Tracker : AchievementMgr->RegisteredAchievements)
		{
			if (Tracker.Achievement)
			{
				FSavedAchievementProgress SavedProgress;
				SavedProgress.AchievementID = Tracker.Achievement->AchievementID;
				SavedProgress.Progress = Tracker.Progress;
				SaveGameObject->AchievementProgress.Add(SavedProgress);
			}
		}
	}

	// Save credits
	UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		SaveGameObject->PlayerCredits = GameInstance->GetPlayerCredits();
	}

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game state collected"));
}

void USaveGameSubsystem::ApplyGameState(UAdastreaSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get player controller and pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();

	// Restore player location and rotation
	if (PlayerPawn)
	{
		PlayerPawn->SetActorLocation(SaveGameObject->PlayerLocation);
		PlayerPawn->SetActorRotation(SaveGameObject->PlayerRotation);
	}

	// Restore player progression
	if (PlayerPawn)
	{
		UPlayerProgressionComponent* ProgressionComp = PlayerPawn->FindComponentByClass<UPlayerProgressionComponent>();
		if (ProgressionComp)
		{
			ProgressionComp->PlayerLevel = SaveGameObject->PlayerProgression.PlayerLevel;
			ProgressionComp->CurrentXP = SaveGameObject->PlayerProgression.CurrentXP;
			ProgressionComp->TotalXPEarned = SaveGameObject->PlayerProgression.TotalXPEarned;
			ProgressionComp->AvailableSkillPoints = SaveGameObject->PlayerProgression.AvailableSkillPoints;
			ProgressionComp->Skills = SaveGameObject->PlayerProgression.Skills;
		}

		// Restore reputation
		UPlayerReputationComponent* ReputationComp = PlayerPawn->FindComponentByClass<UPlayerReputationComponent>();
		if (ReputationComp)
		{
			ReputationComp->FactionReputations = SaveGameObject->FactionReputations;
		}

		// Restore unlocks
		UPlayerUnlockComponent* UnlockComp = PlayerPawn->FindComponentByClass<UPlayerUnlockComponent>();
		if (UnlockComp)
		{
			UnlockComp->UnlockedIDs = SaveGameObject->UnlockedContentIDs;
			
			// Update unlock entries
			for (FUnlockEntry& Entry : UnlockComp->Unlocks)
			{
				Entry.bIsUnlocked = SaveGameObject->UnlockedContentIDs.Contains(Entry.UnlockID);
			}
		}
	}

	// Restore achievements
	UAchievementManagerSubsystem* AchievementMgr = GetGameInstance()->GetSubsystem<UAchievementManagerSubsystem>();
	if (AchievementMgr)
	{
		AchievementMgr->CompletedAchievements = SaveGameObject->CompletedAchievements;
		AchievementMgr->AchievementStats = SaveGameObject->AchievementStats;

		// Restore achievement progress
		for (const FSavedAchievementProgress& SavedProgress : SaveGameObject->AchievementProgress)
		{
			FAchievementTracker* Tracker = AchievementMgr->FindAchievementTracker(SavedProgress.AchievementID);
			if (Tracker)
			{
				Tracker->Progress = SavedProgress.Progress;
			}
		}
	}

	// Restore credits
	UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		int32 CreditDelta = SaveGameObject->PlayerCredits - GameInstance->GetPlayerCredits();
		GameInstance->ModifyPlayerCredits(CreditDelta);
	}

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game state applied"));
}

bool USaveGameSubsystem::ValidateSaveGame(UAdastreaSaveGame* SaveGameObject) const
{
	if (!SaveGameObject)
	{
		return false;
	}

	// Basic validation
	if (SaveGameObject->SaveSlotName.IsEmpty())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Validation failed - empty slot name"));
		return false;
	}

	if (SaveGameObject->PlayerProgression.PlayerLevel < 1)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Validation failed - invalid player level"));
		return false;
	}

	return true;
}

FString USaveGameSubsystem::GetDefaultSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
}

void USaveGameSubsystem::UpdatePlaytime(UAdastreaSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	FTimespan ElapsedTime = FDateTime::Now() - PlaytimeStartTime;
	float SessionPlaytime = static_cast<float>(ElapsedTime.GetTotalSeconds());
	
	SaveGameObject->TotalPlaytimeSeconds = AccumulatedPlaytime + SessionPlaytime;
}
