#include "Player/SaveGameSubsystem.h"
#include "Player/PlayerProgressionComponent.h"
// REMOVED: #include "Player/PlayerReputationComponent.h" - faction reputation system removed per Trade Simulator MVP
#include "Player/PlayerUnlockComponent.h"
#include "Player/AchievementManagerSubsystem.h"
#include "Player/AdastreaGameInstance.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Ships/SpaceshipInterior.h"
#include "Stations/SpaceStation.h"
#include "Stations/DockingBayModule.h"
#include "Trading/CargoComponent.h"
#include "Trading/CraftingTreeLoader.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PawnMovementComponent.h"

namespace
{
	USaveGameSubsystem* GetSaveSubsystem(UWorld* World)
	{
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<USaveGameSubsystem>() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GSaveGameCommand(
		TEXT("adastrea.SaveGame"),
		TEXT("Save the game: ship, credits, cargo, docking, player-built stations. Usage: adastrea.SaveGame [Slot=QuickSave]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (USaveGameSubsystem* SaveSystem = GetSaveSubsystem(World))
			{
				SaveSystem->SaveGame(Args.IsEmpty() ? SaveSystem->QuickSaveSlotName : Args[0], true);
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs GLoadGameCommand(
		TEXT("adastrea.LoadGame"),
		TEXT("Load a saved game into the current level. Usage: adastrea.LoadGame [Slot=QuickSave]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (USaveGameSubsystem* SaveSystem = GetSaveSubsystem(World))
			{
				SaveSystem->LoadGame(Args.IsEmpty() ? SaveSystem->QuickSaveSlotName : Args[0]);
			}
		}));
}

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

	const FString Blocker = GetLoadBlocker();
	if (!Blocker.IsEmpty())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Can't load %s now: %s"), *SlotName, *Blocker);
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
		UE_LOG(LogAdastrea, Error, TEXT("SaveGameSubsystem: Unsupported save version (Save: %d, supported: %d-%d)"),
			LoadedSave->SaveVersion, UAdastreaSaveGame::MIN_SUPPORTED_SAVE_VERSION, UAdastreaSaveGame::CURRENT_SAVE_VERSION);
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

void USaveGameSubsystem::AutoSaveTimerCallback()
{
	bool bSuccess = AutoSave();
	if (!bSuccess)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Auto-save failed"));
	}
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
			&USaveGameSubsystem::AutoSaveTimerCallback,
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

		// REMOVED: Save reputation - faction reputation system removed per Trade Simulator MVP

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

	// Save credits (GameInstance wallet: unlocks and ship upgrades)
	UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		SaveGameObject->PlayerCredits = GameInstance->GetPlayerCredits();
	}

	// Stations first: CollectPlayerShip refers to them by index for docking.
	CollectStations(SaveGameObject);
	CollectPlayerShip(GetPlayerShip(), SaveGameObject);

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game state collected (%d station record(s), ship %s, %d cargo stack(s), docked %d)"),
		SaveGameObject->Stations.Num(),
		SaveGameObject->PlayerShip.bValid ? *SaveGameObject->PlayerShip.ShipClass.ToString() : TEXT("<not at the helm>"),
		SaveGameObject->PlayerShip.Cargo.Num(),
		SaveGameObject->PlayerShip.bDocked ? 1 : 0);
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

	const bool bHasShipAndStations = SaveGameObject->SaveVersion >= UAdastreaSaveGame::SHIP_AND_STATIONS_SAVE_VERSION;
	if (!bHasShipAndStations)
	{
		UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Version %d save has no ship/cargo/station data; keeping the current ones"),
			SaveGameObject->SaveVersion);
	}

	// Leave any dock before stations are rebuilt or the ship is moved: the bay may be
	// about to be destroyed, and a docked ship is hidden with its input disabled.
	ASpaceship* Ship = GetPlayerShip();
	if (Ship && Ship->IsDocked())
	{
		Ship->Undock();
	}

	TArray<ASpaceStation*> SavedStations;
	if (bHasShipAndStations)
	{
		ApplyStations(SaveGameObject, SavedStations);

		if (Ship && SaveGameObject->PlayerShip.bValid)
		{
			Ship = ApplyPlayerShipClass(PC, Ship, SaveGameObject->PlayerShip);
		}
	}

	APawn* PlayerPawn = PC->GetPawn();

	// Restore player location and rotation
	if (PlayerPawn)
	{
		PlayerPawn->SetActorLocation(SaveGameObject->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
		PlayerPawn->SetActorRotation(SaveGameObject->PlayerRotation, ETeleportType::TeleportPhysics);
		if (UPawnMovementComponent* Movement = PlayerPawn->GetMovementComponent())
		{
			Movement->StopMovementImmediately();
		}
	}
	if (Ship)
	{
		Ship->SetThrottle(0.0f);
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

		// REMOVED: Restore reputation - faction reputation system removed per Trade Simulator MVP

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

	if (bHasShipAndStations && Ship && SaveGameObject->PlayerShip.bValid)
	{
		ApplyShipCreditsAndCargo(Ship, SaveGameObject->PlayerShip);
		ApplyDocking(Ship, SaveGameObject->PlayerShip, SavedStations);
	}

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Game state applied"));
}

// ====================
// Ship, cargo, stations (save version 2+)
// ====================

namespace
{
	// Quantisation step for saved station layouts. Finer than the editor's 400 cm
	// build grid (which it divides evenly), so snapped placements round-trip exactly
	// and unsnapped ones land within 5 cm.
	constexpr float SavedStationGridSpacing = 10.0f;

	bool HasPlayerBuiltModule(const ASpaceStation* Station)
	{
		for (const ASpaceStationModule* Module : Station->Modules)
		{
			if (IsValid(Module) && Module->ActorHasTag(ASpaceStation::PlayerBuiltTag))
			{
				return true;
			}
		}
		return false;
	}

	ASpaceStation* FindStationOwningModule(UWorld* World, const ASpaceStationModule* Module)
	{
		for (TActorIterator<ASpaceStation> It(World); It; ++It)
		{
			if (It->Modules.Contains(Module))
			{
				return *It;
			}
		}
		return nullptr;
	}
}

FString USaveGameSubsystem::GetLoadBlocker() const
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	if (!PC)
	{
		return TEXT("no player controller");
	}

	// The editor session holds pointers to the station and modules a load rebuilds.
	const AAdastreaPlayerController* AdastreaPC = Cast<AAdastreaPlayerController>(PC);
	if (AdastreaPC && AdastreaPC->IsStationEditorOpen())
	{
		return TEXT("close the Station Editor first");
	}

	// Ship swaps and docking need the ship to be the possessed pawn.
	if (!Cast<ASpaceship>(PC->GetPawn()))
	{
		return TEXT("return to the helm first (not flying a ship)");
	}

	return FString();
}

ASpaceship* USaveGameSubsystem::GetPlayerShip() const
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	return PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
}

void USaveGameSubsystem::CollectStations(UAdastreaSaveGame* SaveGameObject) const
{
	SaveGameObject->Stations.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASpaceStation> It(World); It; ++It)
	{
		ASpaceStation* Station = *It;
		if (!IsValid(Station))
		{
			continue;
		}

		if (Station->ActorHasTag(ASpaceStation::PlayerBuiltTag))
		{
			// A builder station the player opened and then left empty isn't worth keeping.
			if (Station->GetModuleCount() == 0)
			{
				continue;
			}

			FSavedStation& Saved = SaveGameObject->Stations.AddDefaulted_GetRef();
			Saved.bPlayerBuilt = true;
			Saved.StationActorName = Station->GetFName();
			Saved.StationClass = FSoftClassPath(Station->GetClass());
			Saved.StationTransform = Station->GetActorTransform();
			Saved.Blueprint = Station->ExportBlueprintString(SavedStationGridSpacing);
		}
		else if (HasPlayerBuiltModule(Station))
		{
			FSavedStation& Saved = SaveGameObject->Stations.AddDefaulted_GetRef();
			Saved.bPlayerBuilt = false;
			Saved.StationActorName = Station->GetFName();
			Saved.Blueprint = Station->ExportBlueprintString(SavedStationGridSpacing, true);
		}
	}
}

void USaveGameSubsystem::CollectPlayerShip(ASpaceship* Ship, UAdastreaSaveGame* SaveGameObject) const
{
	FSavedPlayerShip& Saved = SaveGameObject->PlayerShip;
	Saved = FSavedPlayerShip();

	if (!Ship)
	{
		// Walking the interior or in a menu pawn: leave the ship record empty so a
		// load keeps whatever ship state is current instead of wiping it.
		return;
	}

	Saved.bValid = true;
	Saved.ShipClass = FSoftClassPath(Ship->GetClass());
	Saved.ShipDataAsset = FSoftObjectPath(Ship->ShipDataAsset.Get());
	SaveGameObject->CurrentShipID = Ship->ShipDataAsset ? Ship->ShipDataAsset->GetFName() : Ship->GetClass()->GetFName();

	if (const UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent)
	{
		Saved.Credits = Trader->Credits;
		Saved.StartingCredits = Trader->StartingCredits;
	}

	if (const UCargoComponent* Cargo = Ship->CargoComponent)
	{
		for (const FCargoEntry& Entry : Cargo->CargoInventory)
		{
			if (!Entry.Item || Entry.Quantity <= 0)
			{
				continue;
			}

			FSavedCargoEntry& SavedEntry = Saved.Cargo.AddDefaulted_GetRef();
			SavedEntry.ItemID = Entry.Item->ItemID;
			// Crafting-tree items are transient objects built at runtime (no package
			// path); they're found again by ID. Authored assets keep their path.
			if (Entry.Item->IsAsset())
			{
				SavedEntry.ItemAsset = FSoftObjectPath(Entry.Item.Get());
			}
			SavedEntry.Quantity = Entry.Quantity;
		}
	}

	ASpaceStationModule* DockModule = Ship->GetNearbyStation();
	if (Ship->IsDocked() && DockModule)
	{
		ASpaceStation* Station = FindStationOwningModule(Ship->GetWorld(), DockModule);
		if (Station)
		{
			Saved.bDocked = true;
			Saved.DockedBayIndex = FMath::Max(0, Station->GetDockingBayModules().IndexOfByKey(Cast<ADockingBayModule>(DockModule)));
			if (Station->ActorHasTag(ASpaceStation::PlayerBuiltTag))
			{
				Saved.DockedSavedStationIndex = SaveGameObject->Stations.IndexOfByPredicate([Station](const FSavedStation& S)
				{
					return S.bPlayerBuilt && S.StationActorName == Station->GetFName();
				});
			}
			else
			{
				Saved.DockedStationName = Station->GetFName();
			}
		}
	}
}

void USaveGameSubsystem::ApplyStations(const UAdastreaSaveGame* SaveGameObject, TArray<ASpaceStation*>& OutSavedStations)
{
	OutSavedStations.Init(nullptr, SaveGameObject->Stations.Num());

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Clear what the player has built this session, so the world ends up exactly as saved.
	TArray<ASpaceStation*> PlayerStationsToDestroy;
	TMap<FName, ASpaceStation*> LevelStations;
	for (TActorIterator<ASpaceStation> It(World); It; ++It)
	{
		ASpaceStation* Station = *It;
		if (!IsValid(Station))
		{
			continue;
		}

		if (Station->ActorHasTag(ASpaceStation::PlayerBuiltTag))
		{
			PlayerStationsToDestroy.Add(Station);
		}
		else
		{
			Station->RemovePlayerBuiltModules();
			LevelStations.Add(Station->GetFName(), Station);
		}
	}

	for (ASpaceStation* Station : PlayerStationsToDestroy)
	{
		for (ASpaceStationModule* Module : Station->GetModules())
		{
			if (IsValid(Module))
			{
				Module->Destroy();
			}
		}
		Station->Destroy();
	}

	int32 NumModules = 0;
	for (int32 Index = 0; Index < SaveGameObject->Stations.Num(); ++Index)
	{
		const FSavedStation& Saved = SaveGameObject->Stations[Index];
		ASpaceStation* Station = nullptr;

		if (Saved.bPlayerBuilt)
		{
			UClass* StationClass = Saved.StationClass.TryLoadClass<ASpaceStation>();
			if (!StationClass)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Can't load station class %s; skipping a saved station"),
					*Saved.StationClass.ToString());
				continue;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Station = World->SpawnActor<ASpaceStation>(StationClass, Saved.StationTransform, SpawnParams);
			if (Station)
			{
				Station->Tags.AddUnique(ASpaceStation::PlayerBuiltTag);
			}
		}
		else
		{
			ASpaceStation** Found = LevelStations.Find(Saved.StationActorName);
			Station = Found ? *Found : nullptr;
			if (!Station)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Level station %s not found; its player-built modules are lost"),
					*Saved.StationActorName.ToString());
			}
		}

		if (Station)
		{
			NumModules += Station->BuildFromBlueprintString(Saved.Blueprint);
			OutSavedStations[Index] = Station;
		}
	}

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Restored %d station record(s), %d module(s); cleared %d unsaved player station(s)"),
		SaveGameObject->Stations.Num(), NumModules, PlayerStationsToDestroy.Num());
}

ASpaceship* USaveGameSubsystem::ApplyPlayerShipClass(APlayerController* PC, ASpaceship* CurrentShip, const FSavedPlayerShip& Saved)
{
	UClass* ShipClass = Saved.ShipClass.TryLoadClass<ASpaceship>();
	USpaceshipDataAsset* DataAsset = Cast<USpaceshipDataAsset>(Saved.ShipDataAsset.TryLoad());
	if (!ShipClass)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Can't load ship class %s; keeping %s"),
			*Saved.ShipClass.ToString(), *CurrentShip->GetClass()->GetName());
		return CurrentShip;
	}

	if (CurrentShip->GetClass() == ShipClass && CurrentShip->ShipDataAsset == DataAsset)
	{
		return CurrentShip;
	}

	// Deferred so the data asset is in place before BeginPlay reads it (hull, mining laser).
	const FTransform SpawnTransform = CurrentShip->GetActorTransform();
	ASpaceship* NewShip = GetWorld()->SpawnActorDeferred<ASpaceship>(ShipClass, SpawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!NewShip)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Failed to spawn saved ship %s"), *ShipClass->GetName());
		return CurrentShip;
	}
	if (DataAsset)
	{
		NewShip->ShipDataAsset = DataAsset;
	}
	NewShip->FinishSpawning(SpawnTransform);

	// Same swap as the ship-select screen (AAdastreaHUD::SpawnSelectedShip).
	PC->UnPossess();
	PC->Possess(NewShip);
	PC->SetViewTarget(NewShip);
	if (ASpaceshipInterior* OldInterior = CurrentShip->GetInteriorInstance())
	{
		OldInterior->Destroy();
	}
	CurrentShip->Destroy();

	UE_LOG(LogAdastrea, Log, TEXT("SaveGameSubsystem: Swapped player ship to %s (%s)"),
		*ShipClass->GetName(), DataAsset ? *DataAsset->GetName() : TEXT("class default data asset"));
	return NewShip;
}

void USaveGameSubsystem::ApplyShipCreditsAndCargo(ASpaceship* Ship, const FSavedPlayerShip& Saved)
{
	if (UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent)
	{
		Trader->StartingCredits = Saved.StartingCredits;
		// Through AddCredits/RemoveCredits so OnCreditsChanged fires for the HUD.
		const int32 Delta = Saved.Credits - Trader->GetCredits();
		if (Delta > 0)
		{
			Trader->AddCredits(Delta);
		}
		else if (Delta < 0)
		{
			Trader->RemoveCredits(-Delta);
		}
	}

	UCargoComponent* Cargo = Ship->CargoComponent;
	if (!Cargo)
	{
		return;
	}

	Cargo->ClearCargo();
	for (const FSavedCargoEntry& Entry : Saved.Cargo)
	{
		UTradeItemDataAsset* Item = ResolveCargoItem(Entry);
		if (!Item)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Unknown cargo item %s (x%d) dropped"),
				*Entry.ItemID.ToString(), Entry.Quantity);
			continue;
		}
		if (!Cargo->AddCargo(Item, Entry.Quantity))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: No hold space for %d x %s; dropped"),
				Entry.Quantity, *Entry.ItemID.ToString());
		}
	}
}

void USaveGameSubsystem::ApplyDocking(ASpaceship* Ship, const FSavedPlayerShip& Saved, const TArray<ASpaceStation*>& SavedStations)
{
	if (!Saved.bDocked)
	{
		return;
	}

	ASpaceStation* Station = nullptr;
	if (SavedStations.IsValidIndex(Saved.DockedSavedStationIndex))
	{
		Station = SavedStations[Saved.DockedSavedStationIndex];
	}
	else if (!Saved.DockedStationName.IsNone())
	{
		for (TActorIterator<ASpaceStation> It(Ship->GetWorld()); It; ++It)
		{
			if (It->GetFName() == Saved.DockedStationName)
			{
				Station = *It;
				break;
			}
		}
	}

	const TArray<ADockingBayModule*> Bays = Station ? Station->GetDockingBayModules() : TArray<ADockingBayModule*>();
	ADockingBayModule* Bay = Bays.IsValidIndex(Saved.DockedBayIndex) ? Bays[Saved.DockedBayIndex] : (Bays.Num() > 0 ? Bays[0] : nullptr);
	if (!Bay)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Saved docking bay not found; loading undocked"));
		return;
	}

	// The ship is already at the saved (docking point) location, so the normal
	// docking request passes its range check and does the rest (bay slot, UI, input).
	Ship->SetNearbyStation(Bay);
	Ship->RequestDocking();
	if (!Ship->IsDocked())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("SaveGameSubsystem: Re-docking at %s failed; loading undocked"), *Station->GetName());
	}
}

UTradeItemDataAsset* USaveGameSubsystem::ResolveCargoItem(const FSavedCargoEntry& Entry)
{
	if (Entry.ItemAsset.IsValid())
	{
		if (UTradeItemDataAsset* Item = Cast<UTradeItemDataAsset>(Entry.ItemAsset.TryLoad()))
		{
			return Item;
		}
	}

	if (Entry.ItemID.IsNone())
	{
		return nullptr;
	}

	if (!CraftingLoader)
	{
		CraftingLoader = NewObject<UCraftingTreeLoader>(this);
		CraftingLoader->LoadCraftingTree();
	}
	if (UTradeItemDataAsset* Item = CraftingLoader->GetTradeItem(Entry.ItemID.ToString()))
	{
		return Item;
	}

	// Crafted outputs missing from the item pool are built the same way on the fly
	// (UCraftingTreeLoader::CraftRecipe); cargo matches them by ID.
	UTradeItemDataAsset* Item = NewObject<UTradeItemDataAsset>(CraftingLoader);
	Item->ItemID = Entry.ItemID;
	Item->ItemName = FText::FromName(Entry.ItemID);
	return Item;
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
