// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationEditorManager.h"
#include "StationBuildPreview.h"
#include "StationGridSystem.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Trading/CargoComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Ships/Spaceship.h"
#include "Stations/ReactorModule.h"
#include "Stations/SolarArrayModule.h"

namespace
{
	// Grid-adjacent neighbours sit at exactly the sum of their radii. Collision,
	// adjacency and auto-connection all compare against that same distance, so
	// they share this slack (1 cm) to stop float noise from flipping "touching"
	// into "overlapping" or "not a neighbour" and making the three disagree.
	constexpr float PlacementDistanceTolerance = 1.0f;
}

UStationEditorManager::UStationEditorManager()
{
	ModuleCatalog = nullptr;
	PlayerTechLevel = 1;
	PlayerCredits = 0;
	bSnapToGrid = true;
	bCheckCollisions = true;
	CollisionRadius = DefaultCollisionRadius;
	bIsEditing = false;
	CurrentStation = nullptr;
	PreviewActor = nullptr;
	GridSystem = nullptr;
	LastPowerBalance = 0.0f;
}

// =====================
// Editing Lifecycle
// =====================

void UStationEditorManager::EnsureCatalogLoaded()
{
	// Default the catalog to the project's DA_StationModuleCatalog asset if none
	// was assigned (so the station editor reliably surfaces all 27 modules).
	if (!ModuleCatalog)
	{
		ModuleCatalog = Cast<UStationModuleCatalog>(
			StaticLoadObject(UStationModuleCatalog::StaticClass(), nullptr,
				TEXT("/Game/DataAssets/Stations/DA_StationModuleCatalog.DA_StationModuleCatalog")));
	}
	if (!ModuleCatalog)
	{
		return;
	}
	if (!ModuleCatalog->IsCatalogLoaded())
	{
		ModuleCatalog->LoadCatalogFromJson();
	}
}

UCargoComponent* UStationEditorManager::GetConstructionCargo() const
{
	if (PlayerCargo.IsValid())
	{
		return PlayerCargo.Get();
	}

	if (!bAutoResolvePlayerCargo)
	{
		return nullptr;
	}

	// Auto-resolve from the player pawn's owning ship (if any).
	UWorld* World = GetWorld();
	if (!World && CurrentStation)
	{
		World = CurrentStation->GetWorld();
	}
	if (!World)
	{
		return nullptr;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (ASpaceship* Ship = Cast<ASpaceship>(PlayerPawn))
	{
		return Ship->CargoComponent.Get();
	}
	return nullptr;
}

bool UStationEditorManager::HasMaterialsForModule(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!bRequireConstructionMaterials || !ModuleCatalog || !ModuleClass)
	{
		// Materials not required, or no catalog to check against.
		return true;
	}

	FStationModuleEntry Entry;
	if (!ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
	{
		return true; // Not in catalog, allow it.
	}

	if (Entry.BuildCost.Materials.Num() == 0)
	{
		return true;
	}

	UCargoComponent* Cargo = GetConstructionCargo();
	if (!Cargo)
	{
		// Verbose: this runs every frame while the placement preview is up.
		UE_LOG(LogAdastreaStations, Verbose,
			TEXT("StationEditorManager::HasMaterialsForModule - No player cargo available to check materials"));
		return false;
	}

	for (const TPair<FName, int32>& Mat : Entry.BuildCost.Materials)
	{
		if (!Mat.Key.IsNone() && Mat.Value > 0 && Cargo->GetItemQuantityByID(Mat.Key) < Mat.Value)
		{
			UE_LOG(LogAdastreaStations, Verbose,
				TEXT("StationEditorManager::HasMaterialsForModule - %s requires %d x %s (have %d)"),
				*ModuleClass->GetName(), Mat.Value, *Mat.Key.ToString(), Cargo->GetItemQuantityByID(Mat.Key));
			return false;
		}
	}

	return true;
}

bool UStationEditorManager::ConsumeMaterialsForModule(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!bRequireConstructionMaterials || !ModuleCatalog || !ModuleClass)
	{
		return true;
	}

	FStationModuleEntry Entry;
	if (!ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
	{
		return true;
	}

	UCargoComponent* Cargo = GetConstructionCargo();
	if (!Cargo)
	{
		return false;
	}

	bool bAllRemoved = true;
	for (const TPair<FName, int32>& Mat : Entry.BuildCost.Materials)
	{
		if (Mat.Key.IsNone() || Mat.Value <= 0)
		{
			continue;
		}
		if (!Cargo->RemoveCargoByID(Mat.Key, Mat.Value))
		{
			bAllRemoved = false;
			UE_LOG(LogAdastreaStations, Warning,
				TEXT("StationEditorManager::ConsumeMaterialsForModule - could not remove %d x %s"),
				Mat.Value, *Mat.Key.ToString());
		}
	}

	return bAllRemoved;
}

// =====================
// Spending / Refunds
// =====================

bool UStationEditorManager::ChargeForModule(TSubclassOf<ASpaceStationModule> ModuleClass, FStationModuleSpend& OutSpend)
{
	OutSpend = FStationModuleSpend();

	if (!CanAffordModule(ModuleClass) || !HasMaterialsForModule(ModuleClass))
	{
		return false;
	}

	FStationBuildCost Cost;
	if (GetModuleBuildCost(ModuleClass, Cost))
	{
		OutSpend.Credits = FMath::Min(Cost.Credits, PlayerCredits);
		PlayerCredits -= OutSpend.Credits;
	}

	// Record exactly which cargo items were taken (diff the hold around the
	// removal) so a refund gives back the same items. RemoveCargoByID matches
	// IDs loosely, so the catalog's material IDs can't be used to add them back.
	UCargoComponent* Cargo = bRequireConstructionMaterials ? GetConstructionCargo() : nullptr;
	if (Cargo && Cost.Materials.Num() > 0)
	{
		TMap<UTradeItemDataAsset*, int32> Before;
		for (const FCargoEntry& CargoEntry : Cargo->GetCargoContents())
		{
			Before.FindOrAdd(CargoEntry.Item) += CargoEntry.Quantity;
		}

		if (!ConsumeMaterialsForModule(ModuleClass))
		{
			UE_LOG(LogAdastreaStations, Warning,
				TEXT("StationEditorManager::ChargeForModule - could not consume all construction materials for %s"),
				*ModuleClass->GetName());
		}

		TMap<UTradeItemDataAsset*, int32> After;
		for (const FCargoEntry& CargoEntry : Cargo->GetCargoContents())
		{
			After.FindOrAdd(CargoEntry.Item) += CargoEntry.Quantity;
		}

		for (const TPair<UTradeItemDataAsset*, int32>& Pair : Before)
		{
			const int32 Taken = Pair.Value - After.FindRef(Pair.Key);
			if (Pair.Key && Taken > 0)
			{
				OutSpend.MaterialItems.Add(Pair.Key);
				OutSpend.MaterialQuantities.Add(Taken);
			}
		}
	}

	return true;
}

bool UStationEditorManager::RechargeSpend(const FStationModuleSpend& Spend)
{
	if (PlayerCredits < Spend.Credits)
	{
		return false;
	}

	UCargoComponent* Cargo = Spend.MaterialItems.Num() > 0 ? GetConstructionCargo() : nullptr;
	if (Spend.MaterialItems.Num() > 0)
	{
		if (!Cargo)
		{
			return false;
		}
		for (int32 i = 0; i < Spend.MaterialItems.Num(); ++i)
		{
			if (Cargo->GetItemQuantity(Spend.MaterialItems[i]) < Spend.MaterialQuantities[i])
			{
				return false;
			}
		}
	}

	PlayerCredits -= Spend.Credits;
	for (int32 i = 0; i < Spend.MaterialItems.Num(); ++i)
	{
		Cargo->RemoveCargo(Spend.MaterialItems[i], Spend.MaterialQuantities[i]);
	}
	return true;
}

void UStationEditorManager::RefundSpend(const FStationModuleSpend& Spend)
{
	PlayerCredits += Spend.Credits;

	if (Spend.MaterialItems.Num() == 0)
	{
		return;
	}

	UCargoComponent* Cargo = GetConstructionCargo();
	for (int32 i = 0; i < Spend.MaterialItems.Num(); ++i)
	{
		UTradeItemDataAsset* Item = Spend.MaterialItems[i];
		const int32 Quantity = Spend.MaterialQuantities[i];
		if (!Cargo || !Item || !Cargo->AddCargo(Item, Quantity))
		{
			UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RefundSpend - could not return %d x %s to cargo"),
				Quantity, Item ? *Item->GetName() : TEXT("<null>"));
			AddNotification(FText::FromString(FString::Printf(TEXT("Couldn't return %d x %s - cargo hold full or unavailable"),
				Quantity, Item ? *Item->ItemName.ToString() : TEXT("materials"))),
				ENotificationSeverity::Warning, nullptr);
		}
	}
}

void UStationEditorManager::RefundConstructionQueue()
{
	if (ConstructionQueue.Num() == 0)
	{
		return;
	}

	for (const FConstructionQueueItem& Item : ConstructionQueue)
	{
		RefundSpend(Item.Spend);
	}
	ConstructionQueue.Empty();
	OnConstructionQueueChanged.Broadcast();
}

// =====================
// Module Bookkeeping
// =====================

ASpaceStationModule* UStationEditorManager::SpawnModuleInternal(TSubclassOf<ASpaceStationModule> ModuleClass, FVector WorldPosition, FRotator Rotation)
{
	if (!ModuleClass || !CurrentStation)
	{
		return nullptr;
	}

	UWorld* World = CurrentStation->GetWorld();
	if (!World)
	{
		UE_LOG(LogAdastreaStations, Error, TEXT("StationEditorManager::SpawnModuleInternal - No world available"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CurrentStation;
	// Neighbouring modules touch by design; placement validity is our own rule
	// (CanPlaceModule), not the physics overlap test.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(ModuleClass, WorldPosition, Rotation, SpawnParams);
	if (!NewModule)
	{
		UE_LOG(LogAdastreaStations, Error, TEXT("StationEditorManager::SpawnModuleInternal - Failed to spawn %s"), *ModuleClass->GetName());
		return nullptr;
	}

	CurrentStation->AddModuleAtLocation(NewModule, WorldPosition - CurrentStation->GetActorLocation());
	// Marks it for the save system (SaveGameSubsystem stores player-built modules only).
	NewModule->Tags.AddUnique(ASpaceStation::PlayerBuiltTag);
	ModulesAddedThisSession.Add(NewModule);
	AutoGenerateConnections(NewModule);

	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();
	OnModulePlaced.Broadcast(NewModule);

	return NewModule;
}

void UStationEditorManager::SoftDeleteModule(ASpaceStationModule* Module)
{
	if (!Module || !CurrentStation)
	{
		return;
	}

	RemoveConnectionsFor(Module);
	CurrentStation->RemoveModule(Module);

	// Hidden + no collision so it neither renders nor catches the placement trace.
	Module->SetActorHiddenInGame(true);
	Module->SetActorEnableCollision(false);
	Module->SetActorTickEnabled(false);
	SoftDeletedModules.AddUnique(Module);

	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();
	OnModuleRemoved.Broadcast(Module);
}

void UStationEditorManager::RestoreModule(ASpaceStationModule* Module)
{
	if (!Module || !CurrentStation)
	{
		return;
	}

	SoftDeletedModules.Remove(Module);

	Module->SetActorHiddenInGame(false);
	Module->SetActorEnableCollision(true);
	Module->SetActorTickEnabled(true);
	CurrentStation->AddModuleAtLocation(Module, Module->GetActorLocation() - CurrentStation->GetActorLocation());
	AutoGenerateConnections(Module);

	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();
	OnModulePlaced.Broadcast(Module);
}

void UStationEditorManager::RemoveConnectionsFor(const ASpaceStationModule* Module)
{
	const int32 NumRemoved = Connections.RemoveAll([Module](const FModuleConnection& Conn)
	{
		return Conn.ModuleA == Module || Conn.ModuleB == Module;
	});
	if (NumRemoved > 0)
	{
		bStatisticsDirty = true;
	}
}

void UStationEditorManager::RefreshModuleConnections(ASpaceStationModule* Module)
{
	RemoveConnectionsFor(Module);
	AutoGenerateConnections(Module);
	bStatisticsDirty = true;
}

int32 UStationEditorManager::CountConnectedComponents(const ASpaceStationModule* ExcludedModule) const
{
	if (!CurrentStation)
	{
		return 0;
	}

	TSet<const ASpaceStationModule*> Unvisited;
	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module != ExcludedModule)
		{
			Unvisited.Add(Module);
		}
	}

	// Every face-matched neighbour pair gets a Power connection
	// (AutoGenerateConnections), so the Power links are the station's structure.
	TMultiMap<const ASpaceStationModule*, const ASpaceStationModule*> Links;
	for (const FModuleConnection& Conn : Connections)
	{
		if (Conn.ConnectionType == EModuleConnectionType::Power && Conn.ModuleA && Conn.ModuleB)
		{
			Links.Add(Conn.ModuleA, Conn.ModuleB);
			Links.Add(Conn.ModuleB, Conn.ModuleA);
		}
	}

	int32 NumComponents = 0;
	TArray<const ASpaceStationModule*> Stack;
	while (Unvisited.Num() > 0)
	{
		++NumComponents;
		const ASpaceStationModule* Seed = *Unvisited.CreateConstIterator();
		Unvisited.Remove(Seed);
		Stack.Reset();
		Stack.Push(Seed);
		while (Stack.Num() > 0)
		{
			const ASpaceStationModule* Current = Stack.Pop(EAllowShrinking::No);
			TArray<const ASpaceStationModule*> Neighbours;
			Links.MultiFind(Current, Neighbours);
			for (const ASpaceStationModule* Neighbour : Neighbours)
			{
				if (Unvisited.Remove(Neighbour) > 0)
				{
					Stack.Push(Neighbour);
				}
			}
		}
	}

	return NumComponents;
}

bool UStationEditorManager::BeginEditing_Implementation(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::BeginEditing - Invalid station pointer"));
		return false;
	}

	// Make sure the module catalog has entries before the editor surfaces modules.
	EnsureCatalogLoaded();

	if (bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::BeginEditing - Already in editing mode, ending previous session"));
		EndEditing();
	}

	CurrentStation = Station;
	bIsEditing = true;

	// Clear session tracking
	ModulesAddedThisSession.Empty();
	OriginalModuleTransforms.Empty();
	SoftDeletedModules.Empty();
	SessionSpend.Empty();

	// Clear undo/redo stacks for new session
	ClearUndoHistory();

	// Clear connections and regenerate from existing modules
	Connections.Empty();

	// Clear construction queue (refunding anything left over)
	RefundConstructionQueue();

	// Clear notifications
	ClearNotifications();

	// Reset view mode
	CurrentViewMode = EStationEditorViewMode::Edit;

	// Reset time
	CurrentTime = 0.0f;

	// Cache initial power balance
	LastPowerBalance = GetPowerBalance();

	// Mark statistics as dirty
	bStatisticsDirty = true;

	// Create grid system
	if (!GridSystem)
	{
		GridSystem = NewObject<UStationGridSystem>(this);
	}

	// Snap relative to the station, not the world origin. Existing modules sit
	// on the station's own grid (BuildFromLayout / previous sessions), so a
	// world-origin grid put new modules off-grid for any station that wasn't
	// itself at a multiple of GridSize, breaking adjacency and collision spacing.
	GridSystem->SetGridOrigin(Station->GetActorLocation());

	// Create preview actor - clean up existing one first if invalid
	UWorld* World = Station->GetWorld();
	if (World)
	{
		// Clean up any existing preview actor that may have become invalid
		if (PreviewActor && !IsValid(PreviewActor))
		{
			PreviewActor = nullptr;
		}

		// Destroy existing preview actor before creating new one
		if (IsValid(PreviewActor))
		{
			PreviewActor->Destroy();
			PreviewActor = nullptr;
		}

		// Spawn new preview actor
		FActorSpawnParameters SpawnParams;
		PreviewActor = World->SpawnActor<AStationBuildPreview>(AStationBuildPreview::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PreviewActor)
		{
			PreviewActor->Hide();
		}
	}

	// Auto-generate connections for existing modules
	for (ASpaceStationModule* Module : Station->Modules)
	{
		if (Module)
		{
			AutoGenerateConnections(Module);
		}
	}

	// Calculate initial statistics
	RecalculateStatistics();

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::BeginEditing - Started editing station %s"), *Station->GetName());

	// Broadcast state change
	OnEditingStateChanged.Broadcast(true);

	return true;
}

void UStationEditorManager::Cancel_Implementation()
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::Cancel - Not in editing mode"));
		return;
	}

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::Cancel - Canceling editing session, reverting changes"));

	// Revert all changes made during this session
	RevertChanges();

	EndEditing();
}

bool UStationEditorManager::Save_Implementation()
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::Save - Not in editing mode"));
		return false;
	}

	if (!CurrentStation)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::Save - No station to save"));
		return false;
	}

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::Save - Saved changes to station %s (%d modules added, %d removed)"),
		*CurrentStation->GetName(), ModulesAddedThisSession.Num() - SoftDeletedModules.Num(), SoftDeletedModules.Num());

	// EndEditing commits: destroys removed modules and drops session tracking.
	EndEditing();

	return true;
}

void UStationEditorManager::EndEditing_Implementation()
{
	// Queued builds can only complete while a station is being edited, so
	// anything still queued never gets built - give its cost back.
	RefundConstructionQueue();

	// Commit removals: modules only hidden for undo's sake are gone for good now.
	for (ASpaceStationModule* Module : SoftDeletedModules)
	{
		if (IsValid(Module))
		{
			Module->Destroy();
		}
	}
	SoftDeletedModules.Empty();
	ModulesAddedThisSession.Empty();
	OriginalModuleTransforms.Empty();
	SessionSpend.Empty();

	// Undo records point at the modules above; they mean nothing past this session.
	ClearUndoHistory();

	// Clean up preview actor
	if (IsValid(PreviewActor))
	{
		PreviewActor->Hide();
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	// Clear state
	CurrentStation = nullptr;
	bIsEditing = false;

	// Broadcast state change
	OnEditingStateChanged.Broadcast(false);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::EndEditing - Ended editing session"));
}

// =====================
// Module Operations
// =====================

ASpaceStationModule* UStationEditorManager::PlaceModule_Implementation(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation)
{
	// Validate the transform that will actually be used. (Validating the raw
	// position and then spawning at the snapped one let the preview say "valid"
	// for a spot the module never ends up in, and vice versa.)
	FVector FinalPosition;
	FRotator FinalRotation;
	SnapPlacement(Position, Rotation, FinalPosition, FinalRotation);

	const EModulePlacementResult Result = CanPlaceModule(ModuleClass, FinalPosition, FinalRotation);
	if (Result != EModulePlacementResult::Success)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::PlaceModule - Cannot place module: %s"),
			*GetPlacementResultText(Result).ToString());
		return nullptr;
	}

	FStationModuleSpend Spend;
	if (!ChargeForModule(ModuleClass, Spend))
	{
		return nullptr;
	}

	ASpaceStationModule* NewModule = SpawnModuleInternal(ModuleClass, FinalPosition, FinalRotation);
	if (!NewModule)
	{
		RefundSpend(Spend);
		return nullptr;
	}

	SessionSpend.Add(NewModule, Spend);

	// Record action for undo/redo
	FEditorAction Action;
	Action.ActionType = EEditorActionType::PlaceModule;
	Action.ModuleClass = ModuleClass;
	Action.Module = NewModule;
	Action.NewPosition = FinalPosition;
	Action.NewRotation = FinalRotation;
	Action.Spend = Spend;
	Action.Timestamp = CurrentTime;
	RecordAction(Action);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::PlaceModule - Placed module %s at (%.2f, %.2f, %.2f)"),
		*NewModule->GetName(), FinalPosition.X, FinalPosition.Y, FinalPosition.Z);

	// X4-style "smart" feedback: docking-access completeness + production-chain
	// warnings, evaluated once now rather than every stat refresh.
	GenerateStatusNotifications();
	CheckProductionChainWarning(NewModule);

	return NewModule;
}

bool UStationEditorManager::RemoveModule_Implementation(ASpaceStationModule* Module)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Not in editing mode"));
		return false;
	}

	if (!CurrentStation || !Module || !CurrentStation->Modules.Contains(Module))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Invalid station or module"));
		return false;
	}

	if (!CanRemoveModule(Module))
	{
		AddNotificationOnce(FString::Printf(TEXT("Can't remove %s - it would split the station in two"), *Module->ModuleType),
			ENotificationSeverity::Warning, Module);
		return false;
	}

	FEditorAction Action;
	Action.ActionType = EEditorActionType::RemoveModule;
	Action.ModuleClass = Module->GetClass();
	Action.Module = Module;
	Action.PreviousPosition = Module->GetActorLocation();
	Action.PreviousRotation = Module->GetActorRotation();
	Action.Timestamp = CurrentTime;

	// A module placed this session was never committed: removing it gives back
	// what it cost. Removing a module the station already had refunds nothing.
	if (const FStationModuleSpend* Spend = SessionSpend.Find(Module))
	{
		Action.Spend = *Spend;
		RefundSpend(*Spend);
		SessionSpend.Remove(Module);
	}

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::RemoveModule - Removed module %s"), *Module->GetName());

	// Hidden, not destroyed, until Save() - so undo and Cancel() can restore it.
	SoftDeleteModule(Module);
	RecordAction(Action);

	GenerateStatusNotifications();

	return true;
}

bool UStationEditorManager::MoveModule_Implementation(ASpaceStationModule* Module, FVector NewPosition)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::MoveModule - Not in editing mode"));
		return false;
	}

	if (!CurrentStation || !Module || !CurrentStation->Modules.Contains(Module))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::MoveModule - Invalid station or module"));
		return false;
	}

	const FVector PreviousPosition = Module->GetActorLocation();

	FVector FinalPosition;
	FRotator UnusedRotation;
	SnapPlacement(NewPosition, Module->GetActorRotation(), FinalPosition, UnusedRotation);

	if (bCheckCollisions && CheckCollisionIgnoring(Module->GetClass(), FinalPosition, Module))
	{
		AddNotificationOnce(TEXT("Can't move there - it overlaps another module"), ENotificationSeverity::Warning, Module);
		return false;
	}

	const int32 PiecesBefore = CountConnectedComponents(nullptr);
	if (!CurrentStation->MoveModule(Module, FinalPosition - CurrentStation->GetActorLocation()))
	{
		return false;
	}
	RefreshModuleConnections(Module);

	// Same connectivity rule as placement/removal: a move can't strand modules.
	if (CountConnectedComponents(nullptr) > PiecesBefore)
	{
		CurrentStation->MoveModule(Module, PreviousPosition - CurrentStation->GetActorLocation());
		RefreshModuleConnections(Module);
		AddNotificationOnce(TEXT("Can't move there - the module would no longer connect to the station"), ENotificationSeverity::Warning, Module);
		return false;
	}

	// Store original transform if not already stored
	if (!OriginalModuleTransforms.Contains(Module))
	{
		FTransform Original = Module->GetActorTransform();
		Original.SetLocation(PreviousPosition);
		OriginalModuleTransforms.Add(Module, Original);
	}

	FEditorAction Action;
	Action.ActionType = EEditorActionType::MoveModule;
	Action.ModuleClass = Module->GetClass();
	Action.Module = Module;
	Action.PreviousPosition = PreviousPosition;
	Action.NewPosition = FinalPosition;
	Action.Timestamp = CurrentTime;
	RecordAction(Action);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::MoveModule - Moved module to (%.2f, %.2f, %.2f)"),
		FinalPosition.X, FinalPosition.Y, FinalPosition.Z);

	return true;
}

bool UStationEditorManager::RotateModule_Implementation(ASpaceStationModule* Module, FRotator NewRotation)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RotateModule - Not in editing mode"));
		return false;
	}

	if (!CurrentStation || !Module || !CurrentStation->Modules.Contains(Module))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RotateModule - Invalid module"));
		return false;
	}

	const FRotator PreviousRotation = Module->GetActorRotation();

	FVector UnusedPosition;
	FRotator FinalRotation;
	SnapPlacement(Module->GetActorLocation(), NewRotation, UnusedPosition, FinalRotation);

	// Faces rotate with the module, so its connections can change.
	const int32 PiecesBefore = CountConnectedComponents(nullptr);
	Module->SetActorRotation(FinalRotation);
	RefreshModuleConnections(Module);

	if (CountConnectedComponents(nullptr) > PiecesBefore)
	{
		Module->SetActorRotation(PreviousRotation);
		RefreshModuleConnections(Module);
		AddNotificationOnce(TEXT("Can't rotate - no connecting face would point at the station"), ENotificationSeverity::Warning, Module);
		return false;
	}

	// Store original transform if not already stored
	if (!OriginalModuleTransforms.Contains(Module))
	{
		FTransform Original = Module->GetActorTransform();
		Original.SetRotation(PreviousRotation.Quaternion());
		OriginalModuleTransforms.Add(Module, Original);
	}

	FEditorAction Action;
	Action.ActionType = EEditorActionType::RotateModule;
	Action.ModuleClass = Module->GetClass();
	Action.Module = Module;
	Action.PreviousRotation = PreviousRotation;
	Action.NewRotation = FinalRotation;
	Action.Timestamp = CurrentTime;
	RecordAction(Action);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::RotateModule - Rotated module to (%.2f, %.2f, %.2f)"),
		FinalRotation.Pitch, FinalRotation.Yaw, FinalRotation.Roll);

	return true;
}

// =====================
// Validation
// =====================

EModulePlacementResult UStationEditorManager::CanPlaceModule_Implementation(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation)
{
	// Called every frame by the placement preview - keep logging here at Verbose.
	if (!bIsEditing)
	{
		return EModulePlacementResult::NotEditing;
	}

	if (!CurrentStation)
	{
		return EModulePlacementResult::NoStation;
	}

	if (!ModuleClass)
	{
		return EModulePlacementResult::InvalidModule;
	}

	// Judge the snapped transform PlaceModule will use (snapping is idempotent,
	// so already-snapped input is unaffected).
	FVector FinalPosition;
	FRotator FinalRotation;
	SnapPlacement(Position, Rotation, FinalPosition, FinalRotation);

	// Check tech level
	if (!HasSufficientTechLevel(ModuleClass))
	{
		return EModulePlacementResult::InsufficientTech;
	}

	// Check funds
	if (!CanAffordModule(ModuleClass))
	{
		return EModulePlacementResult::InsufficientFunds;
	}

	// Check construction materials are held in the player's cargo
	if (!HasMaterialsForModule(ModuleClass))
	{
		UE_LOG(LogAdastreaStations, Verbose, TEXT("StationEditorManager::CanPlaceModule - Missing construction materials for %s"), *ModuleClass->GetName());
		return EModulePlacementResult::InsufficientMaterials;
	}

	// Check collisions
	if (bCheckCollisions && CheckCollision(ModuleClass, FinalPosition, FinalRotation))
	{
		return EModulePlacementResult::CollisionDetected;
	}

	// X4-style connectivity: every module past the first must attach to the
	// growing structure. Prevents disconnected/floating modules.
	if (!IsAdjacentToExistingModule(ModuleClass, FinalPosition, FinalRotation))
	{
		UE_LOG(LogAdastreaStations, Verbose, TEXT("StationEditorManager::CanPlaceModule - %s is not adjacent to any existing module"), *ModuleClass->GetName());
		return EModulePlacementResult::Disconnected;
	}

	// Check power - this is a hard gate: placing a module that would push the
	// station into a power deficit is disallowed (stations must stay powered).
	if (WouldCausePowerDeficit(ModuleClass))
	{
		UE_LOG(LogAdastreaStations, Verbose, TEXT("StationEditorManager::CanPlaceModule - %s would cause a power deficit"), *ModuleClass->GetName());
		return EModulePlacementResult::InsufficientPower;
	}

	return EModulePlacementResult::Success;
}

bool UStationEditorManager::CheckCollision(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation) const
{
	return CheckCollisionIgnoring(ModuleClass, Position, nullptr);
}

bool UStationEditorManager::CheckCollisionIgnoring(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, const ASpaceStationModule* IgnoredModule) const
{
	if (!ModuleClass || !CurrentStation)
	{
		return false;
	}

	// Sphere-based collision check against each existing module's own footprint-
	// derived radius (GetModuleEffectiveRadius), summed with this module's own -
	// so a large module (a 3x2 DockingBay) needs more clearance than a small one
	// (a 1x1 Corridor) instead of every module using the same fixed CollisionRadius.
	const float ThisRadius = GetModuleEffectiveRadius(ModuleClass);

	for (ASpaceStationModule* ExistingModule : CurrentStation->Modules)
	{
		if (!ExistingModule || ExistingModule == IgnoredModule)
		{
			continue;
		}

		const float OtherRadius = GetModuleEffectiveRadius(ExistingModule->GetClass());
		const float Distance = FVector::Dist(Position, ExistingModule->GetActorLocation());
		if (Distance < ThisRadius + OtherRadius - PlacementDistanceTolerance)
		{
			return true; // Collision detected
		}
	}

	return false;
}

float UStationEditorManager::GetModuleEffectiveRadius(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (ModuleClass && ModuleCatalog)
	{
		FStationModuleEntry Entry;
		if (ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
		{
			const float CellSize = GridSystem ? GridSystem->GridSize : CollisionRadius * 2.0f;
			// Horizontal footprint only (X/Y) - the dimension that matters for
			// station-plan layout; use the larger side as the bounding radius.
			const int32 LargerSideCells = FMath::Max(Entry.GridFootprint.X, Entry.GridFootprint.Y);
			return (LargerSideCells * CellSize) * 0.5f;
		}
	}

	// No catalog entry (no catalog assigned, or module not in it) - fall back to
	// the configurable instance radius so behavior degrades gracefully rather than
	// breaking, and still respects a designer's CollisionRadius override.
	return CollisionRadius;
}

bool UStationEditorManager::IsAdjacentToExistingModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation) const
{
	if (!CurrentStation)
	{
		return false;
	}

	// Empty station: this module becomes the core/anchor, always allowed.
	if (CurrentStation->Modules.Num() == 0)
	{
		return true;
	}

	if (!GridSystem)
	{
		// No grid system to judge adjacency against - don't block placement,
		// just skip the connectivity rule (matches CheckCollision's fail-open).
		return true;
	}

	// "Neighbour" = just clear of collision (ThisRadius + OtherRadius) out to one
	// more grid cell beyond that - i.e. touching or one cell of gap, not floating
	// off in open space. Footprint-aware, so a big module's neighbour band starts
	// further out than a small module's does.
	const float ThisRadius = GetModuleEffectiveRadius(ModuleClass);
	const float CellSize = GridSystem->GridSize;

	for (const ASpaceStationModule* ExistingModule : CurrentStation->Modules)
	{
		if (!ExistingModule)
		{
			continue;
		}

		const float OtherRadius = GetModuleEffectiveRadius(ExistingModule->GetClass());
		const float ClearDistance = ThisRadius + OtherRadius;
		const FVector ToOther = ExistingModule->GetActorLocation() - Position;
		const float Distance = ToOther.Size();
		if (Distance < ClearDistance - PlacementDistanceTolerance || Distance > ClearDistance + CellSize + PlacementDistanceTolerance)
		{
			continue;
		}

		// X4-style face matching: both modules need a face pointed at each other,
		// not just be close enough (a SolarArrayModule facing the wrong way is
		// still a neighbour by distance, but not a valid connection).
		const FVector Direction = ToOther.GetSafeNormal();
		const bool bThisFaces = DoesModuleFaceDirection(ModuleClass, Rotation, Direction);
		const bool bOtherFaces = DoesModuleFaceDirection(ExistingModule->GetClass(), ExistingModule->GetActorRotation(), -Direction);
		if (bThisFaces && bOtherFaces)
		{
			return true;
		}
	}

	return false;
}

bool UStationEditorManager::DoesModuleFaceDirection(TSubclassOf<ASpaceStationModule> ModuleClass, FRotator ModuleRotation, FVector DirectionToOther) const
{
	if (!ModuleClass || !ModuleCatalog)
	{
		return true; // Fail-open, same convention as the rest of validation.
	}

	FStationModuleEntry Entry;
	if (!ModuleCatalog->FindModuleByClass(ModuleClass, Entry) || Entry.ConnectionFaces.Num() == 0)
	{
		return true; // Not in catalog, or "all" faces (empty = unrestricted).
	}

	// N/S/E/W/Up/Down in the module's own local space, rotated into world space.
	// N = local forward (+X), matching UE's convention and BuildFromLayout's plain
	// yaw rotation - an arbitrary but internally consistent choice, since this data
	// has no prior in-engine convention to match (STATION_BUILDER.md's faces are a
	// pure-Python/plan-mode concept until now).
	static const TMap<FName, FVector> FaceLocalDirections = {
		{ FName("N"),    FVector(1, 0, 0) },
		{ FName("S"),    FVector(-1, 0, 0) },
		{ FName("E"),    FVector(0, 1, 0) },
		{ FName("W"),    FVector(0, -1, 0) },
		{ FName("Up"),   FVector(0, 0, 1) },
		{ FName("Down"), FVector(0, 0, -1) },
	};

	for (const FName& Face : Entry.ConnectionFaces)
	{
		const FVector* LocalDir = FaceLocalDirections.Find(Face);
		if (!LocalDir)
		{
			continue;
		}

		const FVector WorldFaceDir = ModuleRotation.RotateVector(*LocalDir);
		// Generous tolerance (60 degrees either side of the face's exact direction)
		// since faces are 90 degrees apart on a cardinal grid - this only needs to
		// disambiguate between faces, not demand pixel-perfect alignment.
		if (FVector::DotProduct(WorldFaceDir, DirectionToOther) > 0.5f)
		{
			return true;
		}
	}

	return false;
}

bool UStationEditorManager::HasSufficientTechLevel(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!ModuleCatalog || !ModuleClass)
	{
		return true; // No catalog means no restrictions
	}

	FStationModuleEntry Entry;
	if (ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
	{
		return PlayerTechLevel >= Entry.RequiredTechLevel;
	}

	return true; // Module not in catalog, allow it
}

bool UStationEditorManager::CanRemoveModule(ASpaceStationModule* Module) const
{
	if (!CurrentStation || !Module || !CurrentStation->Modules.Contains(Module))
	{
		return false;
	}

	// Compare piece counts rather than demanding exactly one piece, so a station
	// that was already fragmented (e.g. hand-placed in the level) can still have
	// its leaves trimmed - only a removal that makes things worse is refused.
	return CountConnectedComponents(Module) <= CountConnectedComponents(nullptr);
}

bool UStationEditorManager::FindAttachPosition(TSubclassOf<ASpaceStationModule> ModuleClass, ASpaceStationModule* ExistingModule, FVector HitNormal, FVector& OutPosition) const
{
	if (!ModuleClass || !CurrentStation || !ExistingModule || !CurrentStation->Modules.Contains(ExistingModule))
	{
		return false;
	}

	// Collapse the normal to its dominant world axis - modules sit on a cardinal grid.
	const FVector Abs = HitNormal.GetAbs();
	FVector Direction;
	if (Abs.X >= Abs.Y && Abs.X >= Abs.Z)
	{
		Direction = FVector(FMath::Sign(HitNormal.X), 0.0f, 0.0f);
	}
	else if (Abs.Y >= Abs.Z)
	{
		Direction = FVector(0.0f, FMath::Sign(HitNormal.Y), 0.0f);
	}
	else
	{
		Direction = FVector(0.0f, 0.0f, FMath::Sign(HitNormal.Z));
	}
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}

	// Exactly touching: the same clearance CheckCollision/IsAdjacentToExistingModule use.
	float Distance = GetModuleEffectiveRadius(ModuleClass) + GetModuleEffectiveRadius(ExistingModule->GetClass());

	// Round the offset up to whole cells before snapping. Snapping the raw point
	// rounds half-cells upward in world terms (+1.5 -> 2 but -1.5 -> -1), which on
	// the -X/-Y/-Z side pulled odd-footprint pairs into each other.
	if (bSnapToGrid && GridSystem && GridSystem->GridSize > 0.0f)
	{
		Distance = FMath::CeilToFloat(Distance / GridSystem->GridSize - KINDA_SMALL_NUMBER) * GridSystem->GridSize;
	}
	const FVector Raw = ExistingModule->GetActorLocation() + Direction * Distance;

	FRotator UnusedRotation;
	SnapPlacement(Raw, FRotator::ZeroRotator, OutPosition, UnusedRotation);
	return true;
}

FText UStationEditorManager::GetPlacementResultText(EModulePlacementResult Result)
{
	switch (Result)
	{
		case EModulePlacementResult::Success:				return NSLOCTEXT("StationEditor", "PlaceOk", "Click to build");
		case EModulePlacementResult::InvalidModule:			return NSLOCTEXT("StationEditor", "PlaceInvalidModule", "No module selected");
		case EModulePlacementResult::InvalidPosition:		return NSLOCTEXT("StationEditor", "PlaceInvalidPos", "Can't build here");
		case EModulePlacementResult::CollisionDetected:		return NSLOCTEXT("StationEditor", "PlaceCollision", "Overlaps an existing module");
		case EModulePlacementResult::InsufficientPower:		return NSLOCTEXT("StationEditor", "PlacePower", "Not enough power - build a Reactor or Solar Array first");
		case EModulePlacementResult::InsufficientTech:		return NSLOCTEXT("StationEditor", "PlaceTech", "Tech level too low");
		case EModulePlacementResult::InsufficientFunds:		return NSLOCTEXT("StationEditor", "PlaceFunds", "Not enough credits");
		case EModulePlacementResult::InsufficientMaterials:	return NSLOCTEXT("StationEditor", "PlaceMaterials", "Missing construction materials in cargo");
		case EModulePlacementResult::NoStation:				return NSLOCTEXT("StationEditor", "PlaceNoStation", "No station selected");
		case EModulePlacementResult::Disconnected:			return NSLOCTEXT("StationEditor", "PlaceDisconnected", "Must attach to the station (R rotates connection faces)");
		case EModulePlacementResult::NotEditing:			return NSLOCTEXT("StationEditor", "PlaceNotEditing", "Not editing a station");
		default:											return FText::GetEmpty();
	}
}

void UStationEditorManager::SnapPlacement(FVector Position, FRotator Rotation, FVector& OutPosition, FRotator& OutRotation) const
{
	OutPosition = (bSnapToGrid && GridSystem) ? GridSystem->SnapToGrid(Position) : Position;
	OutRotation = GridSystem ? GridSystem->SnapRotation(Rotation) : Rotation;
}

bool UStationEditorManager::CanAffordModule(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!ModuleCatalog || !ModuleClass)
	{
		return true; // No catalog means no cost
	}

	FStationModuleEntry Entry;
	if (ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
	{
		return PlayerCredits >= Entry.BuildCost.Credits;
	}

	return true; // Module not in catalog, assume free
}

// =====================
// Resource Tracking
// Note: ModulePower convention:
// - Positive values = power consumption (e.g., +50 means consuming 50 units)
// - Negative values = power generation (e.g., -500 means generating 500 units)
// =====================

float UStationEditorManager::GetTotalPowerConsumption() const
{
	float TotalConsumption = 0.0f;

	if (!CurrentStation)
	{
		return TotalConsumption;
	}

	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (!Module)
		{
			continue;
		}

		// Reactors and solar arrays generate (handled in generation); every other
		// module with positive ModulePower is a consumer.
		if (Cast<const AReactorModule>(Module) || Cast<const ASolarArrayModule>(Module))
		{
			continue;
		}
		if (Module->ModulePower > 0.0f)
		{
			// Upgrades make a consumer more efficient (needs less power for the
			// same job), not hungrier - the bonus divides consumption down.
			TotalConsumption += Module->ModulePower / (1.0f + UpgradeBonusPerLevel * Module->UpgradeLevel);
		}
	}

	return TotalConsumption;
}

float UStationEditorManager::GetTotalPowerGeneration() const
{
	float TotalGeneration = 0.0f;

	if (!CurrentStation)
	{
		return TotalGeneration;
	}

	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (!Module)
		{
			continue;
		}

		// Use the enhanced per-module output for generators (degrades with damage
		// / illumination); fall back to static ModulePower for other sources.
		// Upgrades multiply a generator's output up directly.
		const float UpgradeMultiplier = 1.0f + UpgradeBonusPerLevel * Module->UpgradeLevel;
		if (const AReactorModule* Reactor = Cast<const AReactorModule>(Module))
		{
			TotalGeneration += Reactor->GetCurrentPowerOutput() * UpgradeMultiplier;
		}
		else if (const ASolarArrayModule* Solar = Cast<const ASolarArrayModule>(Module))
		{
			TotalGeneration += Solar->GetEffectiveOutput() * UpgradeMultiplier;
		}
		else if (Module->ModulePower < 0.0f)
		{
			TotalGeneration += FMath::Abs(Module->ModulePower) * UpgradeMultiplier;
		}
	}

	return TotalGeneration;
}

float UStationEditorManager::GetPowerBalance() const
{
	return GetTotalPowerGeneration() - GetTotalPowerConsumption();
}

bool UStationEditorManager::HasSufficientPower() const
{
	return GetPowerBalance() >= 0.0f;
}

bool UStationEditorManager::WouldCausePowerDeficit(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!ModuleClass)
	{
		return false;
	}

	const ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	if (!DefaultModule)
	{
		return false;
	}

	// Positive ModulePower = consumption, negative = generation. Only a consumer
	// can make things worse - a generator or zero-draw module must stay
	// buildable even while the station is already short (otherwise a station
	// in deficit could never build its way out of it).
	const float ModulePower = DefaultModule->ModulePower;
	if (ModulePower <= 0.0f)
	{
		return false;
	}

	// Builds already queued will draw (or add) power too once they finish.
	float QueuedPower = 0.0f;
	for (const FConstructionQueueItem& Item : ConstructionQueue)
	{
		if (Item.ModuleClass)
		{
			QueuedPower += Item.ModuleClass->GetDefaultObject<ASpaceStationModule>()->ModulePower;
		}
	}

	const float NewBalance = GetPowerBalance() - QueuedPower - ModulePower;
	return NewBalance < 0.0f;
}

// =====================
// Preview System
// =====================

void UStationEditorManager::ShowPreview(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!PreviewActor)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::ShowPreview - No preview actor available"));
		return;
	}

	PreviewActor->SetPreviewModule(ModuleClass);
}

void UStationEditorManager::UpdatePreview(FVector Position, FRotator Rotation)
{
	if (!PreviewActor)
	{
		return;
	}

	FVector FinalPosition;
	FRotator FinalRotation;
	SnapPlacement(Position, Rotation, FinalPosition, FinalRotation);

	PreviewActor->UpdatePosition(FinalPosition, FinalRotation);

	// Full validation (tech, funds, materials, collision, connectivity, power),
	// so green always means "clicking will build it".
	if (PreviewActor->CurrentModuleClass)
	{
		PreviewActor->SetValid(CanPlaceModule(PreviewActor->CurrentModuleClass, FinalPosition, FinalRotation) == EModulePlacementResult::Success);
	}
}

void UStationEditorManager::HidePreview()
{
	if (PreviewActor)
	{
		PreviewActor->Hide();
	}
}

// =====================
// Utility
// =====================

TArray<FStationModuleEntry> UStationEditorManager::GetAvailableModules() const
{
	if (!ModuleCatalog)
	{
		return TArray<FStationModuleEntry>();
	}

	return ModuleCatalog->GetModulesByTechLevel(PlayerTechLevel);
}

bool UStationEditorManager::GetModuleBuildCost(TSubclassOf<ASpaceStationModule> ModuleClass, FStationBuildCost& OutCost) const
{
	if (!ModuleCatalog || !ModuleClass)
	{
		return false;
	}

	FStationModuleEntry Entry;
	if (ModuleCatalog->FindModuleByClass(ModuleClass, Entry))
	{
		OutCost = Entry.BuildCost;
		return true;
	}

	return false;
}

// =====================
// Blueprint Save/Load
// =====================

FString UStationEditorManager::ExportStationBlueprint() const
{
	if (!CurrentStation)
	{
		return FString();
	}

	// The format lives on ASpaceStation so the save system (Adastrea module,
	// which can't depend on StationEditor) writes the same strings.
	const float Spacing = GridSystem ? GridSystem->GridSize : CollisionRadius * 2.0f;
	return CurrentStation->ExportBlueprintString(Spacing);
}

int32 UStationEditorManager::ImportStationBlueprint(const FString& BlueprintString)
{
	if (!bIsEditing || !CurrentStation)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::ImportStationBlueprint - Not in editing mode"));
		return 0;
	}

	float Spacing = 1.0f;
	TArray<FStationBlueprintEntry> Entries;
	if (!ASpaceStation::ParseBlueprintString(BlueprintString, Spacing, Entries))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::ImportStationBlueprint - Malformed blueprint string (expected at least SchemaVersion;PlotSize;GridSpacing)"));
		return 0;
	}

	const FVector StationOrigin = CurrentStation->GetActorLocation();

	int32 NumSpawned = 0;
	int32 NumSkipped = 0;
	for (const FStationBlueprintEntry& Entry : Entries)
	{
		UClass* ModuleClass = ASpaceStation::ResolveBlueprintModuleClass(Entry.ItemID);
		if (!ModuleClass)
		{
			UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::ImportStationBlueprint - No module class for ItemID %s; skipping."), *Entry.ItemID);
			continue;
		}

		const FVector WorldPos = StationOrigin + FVector(Entry.GridPos) * Spacing;
		const float RotationDegrees = static_cast<float>(Entry.YawDegrees);

		// Importing onto a station that already has modules must not stack
		// copies inside them.
		if (bCheckCollisions && CheckCollision(ModuleClass, WorldPos, FRotator(0.0f, RotationDegrees, 0.0f)))
		{
			++NumSkipped;
			continue;
		}

		// Tracked as a session module, so Cancel() removes an import too.
		if (SpawnModuleInternal(ModuleClass, WorldPos, FRotator(0.0f, RotationDegrees, 0.0f)))
		{
			++NumSpawned;
		}
	}

	if (NumSkipped > 0)
	{
		AddNotification(FText::FromString(FString::Printf(TEXT("Skipped %d blueprint module(s) that overlap existing modules"), NumSkipped)),
			ENotificationSeverity::Warning, nullptr);
	}

	if (NumSpawned > 0)
	{
		AddNotification(FText::FromString(FString::Printf(TEXT("Imported %d module(s) from blueprint"), NumSpawned)),
			ENotificationSeverity::Success, nullptr);
	}

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::ImportStationBlueprint - Spawned %d module(s)"), NumSpawned);
	return NumSpawned;
}

// =====================
// Internal Functions
// =====================

void UStationEditorManager::RevertChanges()
{
	if (!CurrentStation)
	{
		return;
	}

	// Give back what the session's still-built modules cost. (Modules removed
	// or undone this session were refunded when that happened.)
	for (const TPair<ASpaceStationModule*, FStationModuleSpend>& Pair : SessionSpend)
	{
		RefundSpend(Pair.Value);
	}
	SessionSpend.Empty();

	// Remove all modules added this session (visible or soft-deleted)
	for (ASpaceStationModule* Module : ModulesAddedThisSession)
	{
		if (!IsValid(Module))
		{
			continue;
		}
		RemoveConnectionsFor(Module);
		if (CurrentStation->Modules.Contains(Module))
		{
			CurrentStation->RemoveModule(Module);
		}
		SoftDeletedModules.Remove(Module);
		OriginalModuleTransforms.Remove(Module);
		OnModuleRemoved.Broadcast(Module);
		Module->Destroy();
	}
	ModulesAddedThisSession.Empty();

	// Bring back original modules that were removed this session
	for (ASpaceStationModule* Module : TArray<ASpaceStationModule*>(SoftDeletedModules))
	{
		if (IsValid(Module))
		{
			RestoreModule(Module);
		}
	}
	SoftDeletedModules.Empty();

	// Restore original transforms of moved/rotated modules
	for (const auto& Pair : OriginalModuleTransforms)
	{
		if (IsValid(Pair.Key))
		{
			Pair.Key->SetActorTransform(Pair.Value);
		}
	}
	OriginalModuleTransforms.Empty();

	// Positions and faces changed back - rebuild connectivity from scratch.
	Connections.Empty();
	for (ASpaceStationModule* Module : CurrentStation->Modules)
	{
		AutoGenerateConnections(Module);
	}

	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::RevertChanges - Reverted all changes"));
}

void UStationEditorManager::NotifyPowerBalanceChanged()
{
	float CurrentBalance = GetPowerBalance();

	if (!FMath::IsNearlyEqual(CurrentBalance, LastPowerBalance, 0.01f))
	{
		LastPowerBalance = CurrentBalance;
		OnPowerBalanceChanged.Broadcast(CurrentBalance);
	}
}

// =====================
// Undo/Redo System
// =====================

bool UStationEditorManager::Undo()
{
	if (!CanUndo())
	{
		return false;
	}

	FEditorAction Action = UndoStack.Pop();

	if (ReverseAction(Action))
	{
		RedoStack.Push(Action);
		NotifyUndoRedoStateChanged();
		RecalculateStatistics();
		UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::Undo - Undid action type %d"), static_cast<int32>(Action.ActionType));
		return true;
	}

	// Keep a still-meaningful action (e.g. one that failed only because it
	// couldn't be paid for right now) instead of silently dropping it.
	if (IsValid(Action.Module))
	{
		UndoStack.Push(Action);
	}
	NotifyUndoRedoStateChanged();
	return false;
}

bool UStationEditorManager::Redo()
{
	if (!CanRedo())
	{
		return false;
	}

	FEditorAction Action = RedoStack.Pop();

	if (ExecuteAction(Action))
	{
		UndoStack.Push(Action);
		NotifyUndoRedoStateChanged();
		RecalculateStatistics();
		UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::Redo - Redid action type %d"), static_cast<int32>(Action.ActionType));
		return true;
	}

	if (IsValid(Action.Module))
	{
		RedoStack.Push(Action);
	}
	NotifyUndoRedoStateChanged();
	return false;
}

bool UStationEditorManager::CanUndo() const
{
	return UndoStack.Num() > 0;
}

bool UStationEditorManager::CanRedo() const
{
	return RedoStack.Num() > 0;
}

void UStationEditorManager::ClearUndoHistory()
{
	UndoStack.Empty();
	RedoStack.Empty();
	NotifyUndoRedoStateChanged();
}

int32 UStationEditorManager::GetUndoCount() const
{
	return UndoStack.Num();
}

int32 UStationEditorManager::GetRedoCount() const
{
	return RedoStack.Num();
}

void UStationEditorManager::RecordAction(const FEditorAction& Action)
{
	// Add to undo stack
	UndoStack.Push(Action);

	// Limit stack size efficiently - remove multiple old entries at once
	if (UndoStack.Num() > MaxUndoStackSize)
	{
		const int32 NumToRemove = UndoStack.Num() - MaxUndoStackSize;
		UndoStack.RemoveAt(0, NumToRemove);
	}

	// Clear redo stack when new action is recorded
	RedoStack.Empty();

	NotifyUndoRedoStateChanged();
}

bool UStationEditorManager::ExecuteAction(FEditorAction& Action)
{
	if (!CurrentStation || !IsValid(Action.Module) || Action.Module->IsActorBeingDestroyed())
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::Redo - module for action type %d is gone"), static_cast<int32>(Action.ActionType));
		return false;
	}

	ASpaceStationModule* Module = Action.Module;

	switch (Action.ActionType)
	{
		case EEditorActionType::PlaceModule:
			// The undone module was only hidden - bring the same actor back.
			if (SoftDeletedModules.Contains(Module))
			{
				if (!RechargeSpend(Action.Spend))
				{
					AddNotificationOnce(TEXT("Can't redo - not enough credits or materials"), ENotificationSeverity::Warning, nullptr);
					return false;
				}
				SessionSpend.Add(Module, Action.Spend);
				RestoreModule(Module);
				return true;
			}
			break;

		case EEditorActionType::RemoveModule:
			if (CurrentStation->Modules.Contains(Module))
			{
				if (const FStationModuleSpend* Spend = SessionSpend.Find(Module))
				{
					RefundSpend(*Spend);
					SessionSpend.Remove(Module);
				}
				SoftDeleteModule(Module);
				return true;
			}
			break;

		case EEditorActionType::MoveModule:
			CurrentStation->MoveModule(Module, Action.NewPosition - CurrentStation->GetActorLocation());
			RefreshModuleConnections(Module);
			return true;

		case EEditorActionType::RotateModule:
			Module->SetActorRotation(Action.NewRotation);
			RefreshModuleConnections(Module);
			return true;

		case EEditorActionType::UpgradeModule:
			// Upgrade actions are not undoable/redoable - just log and return false
			UE_LOG(LogAdastreaStations, Log, TEXT("UpgradeModule cannot be redone."));
			break;

		default:
			break;
	}

	return false;
}

bool UStationEditorManager::ReverseAction(FEditorAction& Action)
{
	if (!CurrentStation || !IsValid(Action.Module) || Action.Module->IsActorBeingDestroyed())
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::Undo - module for action type %d is gone"), static_cast<int32>(Action.ActionType));
		return false;
	}

	ASpaceStationModule* Module = Action.Module;

	switch (Action.ActionType)
	{
		case EEditorActionType::PlaceModule:
			// Reverse of place = remove, with a refund of what it cost.
			if (CurrentStation->Modules.Contains(Module))
			{
				if (const FStationModuleSpend* Spend = SessionSpend.Find(Module))
				{
					RefundSpend(*Spend);
					SessionSpend.Remove(Module);
				}
				SoftDeleteModule(Module);
				return true;
			}
			break;

		case EEditorActionType::RemoveModule:
			// Reverse of remove = the same actor back (keeps its upgrades, health, ...).
			if (SoftDeletedModules.Contains(Module))
			{
				if (!Action.Spend.IsEmpty())
				{
					// It was a session module whose cost was refunded on removal.
					if (!RechargeSpend(Action.Spend))
					{
						AddNotificationOnce(TEXT("Can't undo - not enough credits or materials to rebuild it"), ENotificationSeverity::Warning, nullptr);
						return false;
					}
					SessionSpend.Add(Module, Action.Spend);
				}
				RestoreModule(Module);
				return true;
			}
			break;

		case EEditorActionType::MoveModule:
			CurrentStation->MoveModule(Module, Action.PreviousPosition - CurrentStation->GetActorLocation());
			RefreshModuleConnections(Module);
			return true;

		case EEditorActionType::RotateModule:
			Module->SetActorRotation(Action.PreviousRotation);
			RefreshModuleConnections(Module);
			return true;

		case EEditorActionType::UpgradeModule:
			// Upgrade actions are not undoable/redoable - just log and return false
			UE_LOG(LogAdastreaStations, Log, TEXT("UpgradeModule cannot be undone."));
			break;

		default:
			break;
	}

	return false;
}

void UStationEditorManager::NotifyUndoRedoStateChanged()
{
	OnUndoRedoStateChanged.Broadcast(CanUndo(), CanRedo());
}

// =====================
// Module Connections
// =====================

bool UStationEditorManager::AddConnection(ASpaceStationModule* ModuleA, ASpaceStationModule* ModuleB, EModuleConnectionType ConnectionType)
{
	if (!ModuleA || !ModuleB || ModuleA == ModuleB)
	{
		return false;
	}

	// Check if connection already exists
	for (const FModuleConnection& Existing : Connections)
	{
		if (Existing.ConnectionType == ConnectionType &&
			((Existing.ModuleA == ModuleA && Existing.ModuleB == ModuleB) ||
			 (Existing.ModuleA == ModuleB && Existing.ModuleB == ModuleA)))
		{
			return false; // Already exists
		}
	}

	FModuleConnection NewConnection;
	NewConnection.ModuleA = ModuleA;
	NewConnection.ModuleB = ModuleB;
	NewConnection.ConnectionType = ConnectionType;
	NewConnection.bIsActive = true;

	// Set capacity based on connection type using configurable defaults
	switch (ConnectionType)
	{
		case EModuleConnectionType::Power:
			NewConnection.Capacity = DefaultPowerConnectionCapacity; // MW
			break;
		case EModuleConnectionType::Data:
			NewConnection.Capacity = DefaultDataConnectionCapacity; // Mbps
			break;
		case EModuleConnectionType::LifeSupport:
			NewConnection.Capacity = DefaultLifeSupportConnectionCapacity; // Crew capacity
			break;
	}

	Connections.Add(NewConnection);
	OnConnectionChanged.Broadcast(NewConnection);
	bStatisticsDirty = true;

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::AddConnection - Added %d connection between %s and %s"),
		static_cast<int32>(ConnectionType), *ModuleA->GetName(), *ModuleB->GetName());

	return true;
}

bool UStationEditorManager::RemoveConnection(ASpaceStationModule* ModuleA, ASpaceStationModule* ModuleB, EModuleConnectionType ConnectionType)
{
	for (int32 i = Connections.Num() - 1; i >= 0; --i)
	{
		const FModuleConnection& Conn = Connections[i];
		if (Conn.ConnectionType == ConnectionType &&
			((Conn.ModuleA == ModuleA && Conn.ModuleB == ModuleB) ||
			 (Conn.ModuleA == ModuleB && Conn.ModuleB == ModuleA)))
		{
			FModuleConnection RemovedConnection = Connections[i];
			Connections.RemoveAt(i);
			OnConnectionChanged.Broadcast(RemovedConnection);
			bStatisticsDirty = true;
			return true;
		}
	}
	return false;
}

TArray<FModuleConnection> UStationEditorManager::GetModuleConnections(ASpaceStationModule* Module) const
{
	TArray<FModuleConnection> Result;

	for (const FModuleConnection& Conn : Connections)
	{
		if (Conn.ModuleA == Module || Conn.ModuleB == Module)
		{
			Result.Add(Conn);
		}
	}

	return Result;
}

TArray<FModuleConnection> UStationEditorManager::GetConnectionsByType(EModuleConnectionType ConnectionType) const
{
	TArray<FModuleConnection> Result;

	for (const FModuleConnection& Conn : Connections)
	{
		if (Conn.ConnectionType == ConnectionType)
		{
			Result.Add(Conn);
		}
	}

	return Result;
}

TArray<FModuleConnection> UStationEditorManager::GetAllConnections() const
{
	return Connections;
}

void UStationEditorManager::AutoGenerateConnections(ASpaceStationModule* Module)
{
	if (!Module || !CurrentStation || !GridSystem)
	{
		return;
	}

	FVector ModulePosition = Module->GetActorLocation();
	const float ModuleRadius = GetModuleEffectiveRadius(Module->GetClass());
	const float CellSize = GridSystem->GridSize;

	// Check all existing modules for adjacency
	for (ASpaceStationModule* OtherModule : CurrentStation->Modules)
	{
		if (!OtherModule || OtherModule == Module)
		{
			continue;
		}

		// Same footprint- and face-aware "neighbour" test as IsAdjacentToExistingModule,
		// so a module that was allowed to place next to another always gets a
		// connection to it (and vice versa - they can't disagree on who's a neighbour).
		const float OtherRadius = GetModuleEffectiveRadius(OtherModule->GetClass());
		const float ClearDistance = ModuleRadius + OtherRadius;
		const FVector ToOther = OtherModule->GetActorLocation() - ModulePosition;
		const float Distance = ToOther.Size();
		const bool bInRange = Distance >= ClearDistance - PlacementDistanceTolerance && Distance <= ClearDistance + CellSize + PlacementDistanceTolerance;
		const FVector Direction = ToOther.GetSafeNormal();
		const bool bFacesMatch = bInRange
			&& DoesModuleFaceDirection(Module->GetClass(), Module->GetActorRotation(), Direction)
			&& DoesModuleFaceDirection(OtherModule->GetClass(), OtherModule->GetActorRotation(), -Direction);
		if (bFacesMatch)
		{
			// Auto-add power connection
			AddConnection(Module, OtherModule, EModuleConnectionType::Power);

			// Auto-add data connection
			AddConnection(Module, OtherModule, EModuleConnectionType::Data);

			// Add life support if either module is habitation-related
			if (Module->ModuleGroup == EStationModuleGroup::Habitation ||
				OtherModule->ModuleGroup == EStationModuleGroup::Habitation)
			{
				AddConnection(Module, OtherModule, EModuleConnectionType::LifeSupport);
			}
		}
	}
}

bool UStationEditorManager::IsConnectedToPower(const ASpaceStationModule* Module) const
{
	if (!Module || !CurrentStation)
	{
		return false;
	}

	// Simple check: see if module has any power connections
	// A full implementation would do graph traversal to find path to reactor
	for (const FModuleConnection& Conn : Connections)
	{
		if (Conn.ConnectionType == EModuleConnectionType::Power && Conn.bIsActive)
		{
			if (Conn.ModuleA == Module || Conn.ModuleB == Module)
			{
				return true;
			}
		}
	}

	// Power generators are always "connected"
	if (Module->ModuleGroup == EStationModuleGroup::Power)
	{
		return true;
	}

	return false;
}

bool UStationEditorManager::HasLifeSupport(const ASpaceStationModule* Module) const
{
	if (!Module)
	{
		return false;
	}

	for (const FModuleConnection& Conn : Connections)
	{
		if (Conn.ConnectionType == EModuleConnectionType::LifeSupport && Conn.bIsActive)
		{
			if (Conn.ModuleA == Module || Conn.ModuleB == Module)
			{
				return true;
			}
		}
	}

	return false;
}

// =====================
// Construction Queue
// =====================

int32 UStationEditorManager::QueueConstruction(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation)
{
	if (!ModuleClass)
	{
		return -1;
	}

	FVector FinalPosition;
	FRotator FinalRotation;
	SnapPlacement(Position, Rotation, FinalPosition, FinalRotation);

	// Same gates as a direct placement (tech, funds, materials, collision,
	// connectivity, power - the power check counts earlier queued builds too).
	const EModulePlacementResult Result = CanPlaceModule(ModuleClass, FinalPosition, FinalRotation);
	if (Result != EModulePlacementResult::Success)
	{
		AddNotification(GetPlacementResultText(Result), ENotificationSeverity::Warning, nullptr);
		return -1;
	}

	// Queued builds don't exist yet, so CheckCollision can't see them.
	const float ThisRadius = GetModuleEffectiveRadius(ModuleClass);
	for (const FConstructionQueueItem& Queued : ConstructionQueue)
	{
		if (FVector::Dist(FinalPosition, Queued.TargetPosition) < ThisRadius + GetModuleEffectiveRadius(Queued.ModuleClass))
		{
			AddNotification(FText::FromString(TEXT("Overlaps a module that is already queued")), ENotificationSeverity::Warning, nullptr);
			return -1;
		}
	}

	// Pay up front (X4-style), refunded if the build is cancelled.
	FStationModuleSpend Spend;
	if (!ChargeForModule(ModuleClass, Spend))
	{
		return -1;
	}

	FConstructionQueueItem Item;
	Item.QueueId = NextQueueId++;
	Item.ModuleClass = ModuleClass;
	Item.TargetPosition = FinalPosition;
	Item.TargetRotation = FinalRotation;
	Item.Spend = Spend;

	// Get build time from catalog
	FStationBuildCost Cost;
	if (GetModuleBuildCost(ModuleClass, Cost))
	{
		Item.TotalBuildTime = Cost.BuildTime;
		Item.TimeRemaining = Cost.BuildTime;
	}
	else
	{
		Item.TotalBuildTime = DefaultBuildTime;
		Item.TimeRemaining = DefaultBuildTime;
	}

	// Start building if this is the first item
	if (ConstructionQueue.Num() == 0)
	{
		Item.bIsBuilding = true;
	}

	ConstructionQueue.Add(Item);
	OnConstructionQueueChanged.Broadcast();

	// Add notification
	ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	FString ModuleName = DefaultModule ? DefaultModule->ModuleType : TEXT("Module");
	AddNotification(FText::FromString(FString::Printf(TEXT("Construction of %s queued"), *ModuleName)),
		ENotificationSeverity::Info, nullptr);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::QueueConstruction - Queued %s (ID: %d)"),
		*ModuleClass->GetName(), Item.QueueId);

	return Item.QueueId;
}

bool UStationEditorManager::CancelConstruction(int32 QueueId)
{
	for (int32 i = 0; i < ConstructionQueue.Num(); ++i)
	{
		if (ConstructionQueue[i].QueueId == QueueId)
		{
			RefundSpend(ConstructionQueue[i].Spend);
			ConstructionQueue.RemoveAt(i);

			// Start next item if we removed the current one
			if (i == 0 && ConstructionQueue.Num() > 0)
			{
				ConstructionQueue[0].bIsBuilding = true;
			}

			OnConstructionQueueChanged.Broadcast();
			return true;
		}
	}
	return false;
}

bool UStationEditorManager::SetConstructionPaused(int32 QueueId, bool bPause)
{
	for (FConstructionQueueItem& Item : ConstructionQueue)
	{
		if (Item.QueueId == QueueId)
		{
			Item.bIsPaused = bPause;
			OnConstructionQueueChanged.Broadcast();
			return true;
		}
	}
	return false;
}

bool UStationEditorManager::ReorderConstruction(int32 QueueId, bool MoveUp)
{
	for (int32 i = 0; i < ConstructionQueue.Num(); ++i)
	{
		if (ConstructionQueue[i].QueueId == QueueId)
		{
			int32 NewIndex = MoveUp ? i - 1 : i + 1;

			// Can't move first item up or last item down
			if (NewIndex < 0 || NewIndex >= ConstructionQueue.Num())
			{
				return false;
			}

			// Can't move if we're currently building (first item)
			if (i == 0 && ConstructionQueue[i].bIsBuilding)
			{
				return false;
			}

			// Swap items
			FConstructionQueueItem Temp = ConstructionQueue[i];
			ConstructionQueue[i] = ConstructionQueue[NewIndex];
			ConstructionQueue[NewIndex] = Temp;

			OnConstructionQueueChanged.Broadcast();
			return true;
		}
	}
	return false;
}

TArray<FConstructionQueueItem> UStationEditorManager::GetConstructionQueue() const
{
	return ConstructionQueue;
}

bool UStationEditorManager::GetCurrentConstruction(FConstructionQueueItem& OutItem) const
{
	if (ConstructionQueue.Num() > 0 && ConstructionQueue[0].bIsBuilding)
	{
		OutItem = ConstructionQueue[0];
		return true;
	}
	return false;
}

void UStationEditorManager::UpdateConstruction(float DeltaTime)
{
	CurrentTime += DeltaTime;

	if (ConstructionQueue.Num() == 0)
	{
		return;
	}

	FConstructionQueueItem& CurrentItem = ConstructionQueue[0];

	if (!CurrentItem.bIsBuilding)
	{
		CurrentItem.bIsBuilding = true;
	}

	if (CurrentItem.bIsPaused)
	{
		return;
	}

	CurrentItem.TimeRemaining -= DeltaTime;

	if (CurrentItem.TimeRemaining <= 0.0f)
	{
		CompleteConstruction(CurrentItem);
		ConstructionQueue.RemoveAt(0);

		// Start next item
		if (ConstructionQueue.Num() > 0)
		{
			ConstructionQueue[0].bIsBuilding = true;
		}

		OnConstructionQueueChanged.Broadcast();
	}
}

void UStationEditorManager::CompleteConstruction(FConstructionQueueItem& Item)
{
	if (!Item.ModuleClass || !CurrentStation)
	{
		RefundSpend(Item.Spend);
		return;
	}

	// Something may have been placed on the spot while this was building.
	if (bCheckCollisions && CheckCollision(Item.ModuleClass, Item.TargetPosition, Item.TargetRotation))
	{
		RefundSpend(Item.Spend);
		AddNotification(FText::FromString(TEXT("Construction cancelled - the build site is now occupied (refunded)")),
			ENotificationSeverity::Warning, nullptr);
		return;
	}

	// Spawn the module directly without recording an undo action: construction
	// from the queue is a committed action, not an editor operation. It is still
	// a session module, so Cancel() removes it and refunds it.
	ASpaceStationModule* NewModule = SpawnModuleInternal(Item.ModuleClass, Item.TargetPosition, Item.TargetRotation);
	if (!NewModule)
	{
		RefundSpend(Item.Spend);
		return;
	}

	SessionSpend.Add(NewModule, Item.Spend);

	// Add completion notification
	AddNotification(FText::FromString(FString::Printf(TEXT("%s construction complete"), *NewModule->ModuleType)),
		ENotificationSeverity::Success, NewModule);

	// Same X4-style completeness/production-chain feedback as a direct placement.
	GenerateStatusNotifications();
	CheckProductionChainWarning(NewModule);
}

// =====================
// Station Statistics
// =====================

FStationStatistics UStationEditorManager::GetStationStatistics() const
{
	if (bStatisticsDirty)
	{
		RecalculateStatisticsInternal();
	}
	return CachedStatistics;
}

void UStationEditorManager::RecalculateStatistics()
{
	RecalculateStatisticsInternal();
	OnStatisticsUpdated.Broadcast(CachedStatistics);
}

void UStationEditorManager::RecalculateStatisticsInternal() const
{
	CachedStatistics = FStationStatistics();

	if (!CurrentStation)
	{
		bStatisticsDirty = false;
		return;
	}

	CachedStatistics.TotalModules = CurrentStation->Modules.Num();
	CachedStatistics.PowerGenerated = GetTotalPowerGeneration();
	CachedStatistics.PowerConsumed = GetTotalPowerConsumption();
	CachedStatistics.MaxPopulation = GetPopulationCapacity();
	CachedStatistics.DefenseRating = GetDefenseRating();
	CachedStatistics.EfficiencyRating = GetEfficiencyRating();
	CachedStatistics.bHasDockingAccess = CurrentStation->HasDockingCapability();

	// Calculate cargo capacity using configurable default
	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module->ModuleGroup == EStationModuleGroup::Storage)
		{
			CachedStatistics.CargoCapacity += DefaultCargoCapacityPerModule * (1.0f + UpgradeBonusPerLevel * Module->UpgradeLevel);
		}
	}

	// Calculate data network usage based on connections
	int32 DataConnections = GetConnectionsByType(EModuleConnectionType::Data).Num();
	int32 NumModules = CurrentStation->Modules.Num();
	if (NumModules < 2)
	{
		// No possible connections if fewer than 2 modules
		CachedStatistics.DataNetworkUsage = 0.0f;
	}
	else
	{
		int32 TotalPossibleConnections = NumModules * (NumModules - 1) / 2;
		// TotalPossibleConnections is guaranteed > 0 when NumModules >= 2
		CachedStatistics.DataNetworkUsage = static_cast<float>(DataConnections) / static_cast<float>(TotalPossibleConnections);
	}

	// Calculate life support coverage
	int32 LifeSupportConnections = GetConnectionsByType(EModuleConnectionType::LifeSupport).Num();
	int32 HabitationModules = 0;
	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module->ModuleGroup == EStationModuleGroup::Habitation)
		{
			HabitationModules++;
		}
	}
	CachedStatistics.LifeSupportCoverage = HabitationModules > 0 ?
		FMath::Clamp(static_cast<float>(LifeSupportConnections) / static_cast<float>(HabitationModules), 0.0f, 1.0f) : 1.0f;

	bStatisticsDirty = false;
}

int32 UStationEditorManager::GetPopulationCapacity() const
{
	int32 Capacity = 0;

	if (!CurrentStation)
	{
		return Capacity;
	}

	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module->ModuleGroup == EStationModuleGroup::Habitation)
		{
			Capacity += FMath::RoundToInt(DefaultPopulationCapacityPerModule * (1.0f + UpgradeBonusPerLevel * Module->UpgradeLevel));
		}
	}

	return Capacity;
}

float UStationEditorManager::GetDefenseRating() const
{
	float Rating = 0.0f;

	if (!CurrentStation)
	{
		return Rating;
	}

	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module->ModuleGroup == EStationModuleGroup::Defence)
		{
			Rating += DefaultDefenseRatingPerModule * (1.0f + UpgradeBonusPerLevel * Module->UpgradeLevel);
		}
	}

	return FMath::Clamp(Rating, 0.0f, 100.0f);
}

float UStationEditorManager::GetEfficiencyRating() const
{
	if (!CurrentStation || CurrentStation->Modules.Num() == 0)
	{
		return 1.0f;
	}

	float Efficiency = 1.0f;

	// Power efficiency using configurable penalties
	float PowerBalance = GetPowerBalance();
	if (PowerBalance < 0)
	{
		Efficiency -= PowerDeficitEfficiencyPenalty;
	}
	else if (PowerBalance > GetTotalPowerGeneration() * 0.5f)
	{
		Efficiency -= PowerOverProductionEfficiencyPenalty;
	}

	// Connection efficiency
	int32 ConnectedModules = 0;
	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && IsConnectedToPower(Module))
		{
			ConnectedModules++;
		}
	}
	float ConnectionRatio = static_cast<float>(ConnectedModules) / static_cast<float>(CurrentStation->Modules.Num());
	Efficiency *= ConnectionRatio;

	return FMath::Clamp(Efficiency, 0.0f, 1.0f);
}

// =====================
// Notifications
// =====================

int32 UStationEditorManager::AddNotification(FText Message, ENotificationSeverity Severity, ASpaceStationModule* RelatedModule)
{
	FStationNotification Notification;
	Notification.NotificationId = NextNotificationId++;
	Notification.Message = Message;
	Notification.Severity = Severity;
	Notification.Timestamp = CurrentTime;
	Notification.bIsRead = false;
	Notification.RelatedModule = RelatedModule;

	Notifications.Add(Notification);

	// Keep notifications list manageable using configurable limit
	if (Notifications.Num() > MaxNotifications)
	{
		const int32 NumToRemove = Notifications.Num() - MaxNotifications;
		Notifications.RemoveAt(0, NumToRemove);
	}

	OnNotificationAdded.Broadcast(Notification);

	return Notification.NotificationId;
}

void UStationEditorManager::AddNotificationOnce(const FString& Message, ENotificationSeverity Severity, ASpaceStationModule* RelatedModule)
{
	// Placement feedback runs on every build; don't stack the same warning.
	for (const FStationNotification& Existing : Notifications)
	{
		if (!Existing.bIsRead && Existing.Message.ToString() == Message)
		{
			return;
		}
	}
	AddNotification(FText::FromString(Message), Severity, RelatedModule);
}

void UStationEditorManager::MarkNotificationRead(int32 NotificationId)
{
	for (FStationNotification& Notification : Notifications)
	{
		if (Notification.NotificationId == NotificationId)
		{
			Notification.bIsRead = true;
			return;
		}
	}
}

void UStationEditorManager::ClearNotifications()
{
	Notifications.Empty();
}

TArray<FStationNotification> UStationEditorManager::GetNotifications() const
{
	return Notifications;
}

int32 UStationEditorManager::GetUnreadNotificationCount() const
{
	int32 Count = 0;
	for (const FStationNotification& Notification : Notifications)
	{
		if (!Notification.bIsRead)
		{
			Count++;
		}
	}
	return Count;
}

void UStationEditorManager::GenerateStatusNotifications()
{
	if (!CurrentStation)
	{
		return;
	}

	// Check power status using configurable thresholds
	const float PowerBalance = GetPowerBalance();
	const float PowerGeneration = GetTotalPowerGeneration();
	if (PowerBalance < 0)
	{
		AddNotificationOnce(FString::Printf(TEXT("Power deficit: %.0f MW"), FMath::Abs(PowerBalance)),
			ENotificationSeverity::Warning, nullptr);
	}
	else if (PowerGeneration > 0.0f && GetTotalPowerConsumption() > PowerGeneration * PowerLoadWarningThreshold)
	{
		// Load above the threshold share of generation (was inverted: it fired
		// when there was lots of spare power, and never when close to the limit).
		AddNotificationOnce(TEXT("Power load near maximum capacity"), ENotificationSeverity::Warning, nullptr);
	}

	// Check population using configurable threshold
	FStationStatistics Stats = GetStationStatistics();
	if (Stats.CurrentPopulation > Stats.MaxPopulation * PopulationWarningThreshold)
	{
		AddNotificationOnce(TEXT("Population approaching capacity. Consider adding habitation modules."),
			ENotificationSeverity::Warning, nullptr);
	}

	// X4-style completeness check: a station with no docking module is unreachable
	// by ships (matches ASpaceStation::HasDockingCapability(), the same rule
	// STATION_BUILDER.md's plan-mode validator enforces as a hard block - here it's
	// a warning, since the real-time editor lets you build incrementally).
	if (!Stats.bHasDockingAccess && CurrentStation->Modules.Num() > 0)
	{
		AddNotificationOnce(TEXT("No docking module yet - ships can't reach this station. Add a Docking Bay or Docking Port."),
			ENotificationSeverity::Warning, nullptr);
	}
}

void UStationEditorManager::CheckProductionChainWarning(const ASpaceStationModule* Module)
{
	if (!Module || !CurrentStation || Module->ModuleGroup != EStationModuleGroup::Processing)
	{
		return;
	}

	bool bHasStorage = false;
	for (const ASpaceStationModule* Existing : CurrentStation->Modules)
	{
		if (Existing && Existing->ModuleGroup == EStationModuleGroup::Storage)
		{
			bHasStorage = true;
			break;
		}
	}

	if (!bHasStorage)
	{
		AddNotificationOnce(FString::Printf(
				TEXT("%s has no Cargo Bay on the station to hold its inputs/outputs - it won't have anything to process."),
				*Module->ModuleType),
			ENotificationSeverity::Warning, const_cast<ASpaceStationModule*>(Module));
	}

	if (!HasSufficientPower())
	{
		AddNotificationOnce(FString::Printf(
				TEXT("%s is on a station running a power deficit and won't run at full capacity."), *Module->ModuleType),
			ENotificationSeverity::Warning, const_cast<ASpaceStationModule*>(Module));
	}
}

// =====================
// View Mode
// =====================

void UStationEditorManager::SetViewMode(EStationEditorViewMode NewMode)
{
	if (CurrentViewMode != NewMode)
	{
		CurrentViewMode = NewMode;
		OnViewModeChanged.Broadcast(NewMode);

		UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::SetViewMode - Changed to mode %d"), static_cast<int32>(NewMode));
	}
}

EStationEditorViewMode UStationEditorManager::GetViewMode() const
{
	return CurrentViewMode;
}

// =====================
// Module Upgrade
// =====================

bool UStationEditorManager::CanUpgradeModule(ASpaceStationModule* Module) const
{
	if (!Module || !ModuleCatalog)
	{
		return false;
	}

	// Each tech level the player has above the module's requirement unlocks one
	// upgrade level, up to MaxUpgradeLevel. (Previously any module below the
	// player's tech level could be upgraded without limit.)
	FStationModuleEntry Entry;
	if (ModuleCatalog->FindModuleByClass(Module->GetClass(), Entry))
	{
		const int32 UnlockedLevels = FMath::Min(MaxUpgradeLevel, PlayerTechLevel - Entry.RequiredTechLevel);
		return Module->UpgradeLevel < UnlockedLevels;
	}

	return false;
}

bool UStationEditorManager::GetUpgradeCost(ASpaceStationModule* Module, FStationBuildCost& OutCost) const
{
	if (!CanUpgradeModule(Module))
	{
		return false;
	}

	FStationModuleEntry Entry;
	if (ModuleCatalog && ModuleCatalog->FindModuleByClass(Module->GetClass(), Entry))
	{
		// Upgrade cost uses configurable multiplier
		OutCost = Entry.BuildCost;
		OutCost.Credits = static_cast<int32>(OutCost.Credits * DefaultUpgradeCostMultiplier);
		OutCost.BuildTime *= DefaultUpgradeCostMultiplier;
		return true;
	}

	return false;
}

bool UStationEditorManager::UpgradeModule(ASpaceStationModule* Module)
{
	if (!CanUpgradeModule(Module))
	{
		return false;
	}

	FStationBuildCost Cost;
	if (!GetUpgradeCost(Module, Cost))
	{
		return false;
	}

	// Check if player can afford
	if (PlayerCredits < Cost.Credits)
	{
		AddNotification(FText::FromString(TEXT("Insufficient credits for upgrade")),
			ENotificationSeverity::Warning, Module);
		return false;
	}

	// Deduct credits
	PlayerCredits -= Cost.Credits;

	// The actual upgrade: previously this function charged credits and did
	// nothing else. Incrementing UpgradeLevel is what makes the charge honest -
	// GetTotalPowerGeneration/Consumption, cargo capacity, population capacity,
	// and defense rating all read it (UpgradeBonusPerLevel per level).
	++Module->UpgradeLevel;

	// Note: Upgrade actions are not recorded to undo stack as they cannot be reversed

	// Add notification
	AddNotification(FText::FromString(FString::Printf(TEXT("%s upgraded to level %d"), *Module->ModuleType, Module->UpgradeLevel)),
		ENotificationSeverity::Success, Module);

	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::UpgradeModule - Upgraded %s"), *Module->GetName());

	return true;
}
