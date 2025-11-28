// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationEditorManager.h"
#include "StationBuildPreview.h"
#include "StationGridSystem.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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

bool UStationEditorManager::BeginEditing_Implementation(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::BeginEditing - Invalid station pointer"));
		return false;
	}

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
	
	// Clear undo/redo stacks for new session
	ClearUndoHistory();
	
	// Clear connections and regenerate from existing modules
	Connections.Empty();
	
	// Clear construction queue
	ConstructionQueue.Empty();
	
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

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::Save - Saved changes to station %s (%d modules added)"), 
		*CurrentStation->GetName(), ModulesAddedThisSession.Num());

	// Clear session tracking without reverting
	ModulesAddedThisSession.Empty();
	OriginalModuleTransforms.Empty();

	EndEditing();
	
	return true;
}

void UStationEditorManager::EndEditing_Implementation()
{
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
	// Validate placement
	EModulePlacementResult Result = CanPlaceModule(ModuleClass, Position, Rotation);
	if (Result != EModulePlacementResult::Success)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::PlaceModule - Cannot place module: %d"), static_cast<int32>(Result));
		return nullptr;
	}

	// Snap to grid if enabled
	FVector FinalPosition = Position;
	if (bSnapToGrid && GridSystem)
	{
		FinalPosition = GridSystem->SnapToGrid(Position);
	}

	// Spawn the module
	UWorld* World = CurrentStation->GetWorld();
	if (!World)
	{
		UE_LOG(LogAdastreaStations, Error, TEXT("StationEditorManager::PlaceModule - No world available"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CurrentStation;
	
	ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
		ModuleClass,
		FinalPosition,
		Rotation,
		SpawnParams
	);

	if (!NewModule)
	{
		UE_LOG(LogAdastreaStations, Error, TEXT("StationEditorManager::PlaceModule - Failed to spawn module"));
		return nullptr;
	}

	// Calculate relative location
	FVector RelativeLocation = FinalPosition - CurrentStation->GetActorLocation();

	// Add to station
	CurrentStation->AddModuleAtLocation(NewModule, RelativeLocation);

	// Track for potential undo
	ModulesAddedThisSession.Add(NewModule);

	// Record action for undo/redo
	FEditorAction Action;
	Action.ActionType = EEditorActionType::PlaceModule;
	Action.ModuleClass = ModuleClass;
	Action.Module = NewModule;
	Action.NewPosition = FinalPosition;
	Action.NewRotation = Rotation;
	Action.Timestamp = CurrentTime;
	RecordAction(Action);

	// Auto-generate connections to adjacent modules
	AutoGenerateConnections(NewModule);

	// Mark statistics as dirty
	bStatisticsDirty = true;

	// Update power balance
	NotifyPowerBalanceChanged();

	// Deduct credits if applicable
	FStationBuildCost Cost;
	if (GetModuleBuildCost(ModuleClass, Cost))
	{
		PlayerCredits = FMath::Max(0, PlayerCredits - Cost.Credits);
	}

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::PlaceModule - Placed module %s at (%.2f, %.2f, %.2f)"),
		*NewModule->GetName(), FinalPosition.X, FinalPosition.Y, FinalPosition.Z);

	// Broadcast event
	OnModulePlaced.Broadcast(NewModule);

	return NewModule;
}

bool UStationEditorManager::RemoveModule_Implementation(ASpaceStationModule* Module)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Not in editing mode"));
		return false;
	}

	if (!CurrentStation || !Module)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Invalid station or module"));
		return false;
	}

	// Store module info for undo before removal
	FEditorAction Action;
	Action.ActionType = EEditorActionType::RemoveModule;
	Action.ModuleClass = Module->GetClass();
	Action.Module = nullptr; // Will be invalid after destroy
	Action.PreviousPosition = Module->GetActorLocation();
	Action.PreviousRotation = Module->GetActorRotation();
	Action.Timestamp = CurrentTime;

	// Remove connections involving this module
	for (int32 i = Connections.Num() - 1; i >= 0; --i)
	{
		if (Connections[i].ModuleA == Module || Connections[i].ModuleB == Module)
		{
			Connections.RemoveAt(i);
		}
	}

	// Remove from station
	if (!CurrentStation->RemoveModule(Module))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Failed to remove module from station"));
		return false;
	}

	// Remove from tracking
	ModulesAddedThisSession.Remove(Module);
	OriginalModuleTransforms.Remove(Module);

	// Record action for undo
	RecordAction(Action);

	// Mark statistics as dirty
	bStatisticsDirty = true;

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::RemoveModule - Removed module %s"), *Module->GetName());

	// Update power balance
	NotifyPowerBalanceChanged();

	// Broadcast event
	OnModuleRemoved.Broadcast(Module);

	// Optionally destroy the module
	Module->Destroy();

	return true;
}

bool UStationEditorManager::MoveModule_Implementation(ASpaceStationModule* Module, FVector NewPosition)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::MoveModule - Not in editing mode"));
		return false;
	}

	if (!CurrentStation || !Module)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::MoveModule - Invalid station or module"));
		return false;
	}

	// Store previous position for undo
	FVector PreviousPosition = Module->GetActorLocation();

	// Store original transform if not already stored
	if (!OriginalModuleTransforms.Contains(Module))
	{
		OriginalModuleTransforms.Add(Module, Module->GetActorTransform());
	}

	// Snap to grid if enabled
	FVector FinalPosition = NewPosition;
	if (bSnapToGrid && GridSystem)
	{
		FinalPosition = GridSystem->SnapToGrid(NewPosition);
	}

	// Calculate relative location
	FVector RelativeLocation = FinalPosition - CurrentStation->GetActorLocation();

	// Move the module
	bool bSuccess = CurrentStation->MoveModule(Module, RelativeLocation);

	if (bSuccess)
	{
		// Record action for undo
		FEditorAction Action;
		Action.ActionType = EEditorActionType::MoveModule;
		Action.Module = Module;
		Action.PreviousPosition = PreviousPosition;
		Action.NewPosition = FinalPosition;
		Action.Timestamp = CurrentTime;
		RecordAction(Action);

		UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::MoveModule - Moved module to (%.2f, %.2f, %.2f)"),
			FinalPosition.X, FinalPosition.Y, FinalPosition.Z);
	}

	return bSuccess;
}

bool UStationEditorManager::RotateModule_Implementation(ASpaceStationModule* Module, FRotator NewRotation)
{
	if (!bIsEditing)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RotateModule - Not in editing mode"));
		return false;
	}

	if (!Module)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RotateModule - Invalid module"));
		return false;
	}

	// Store previous rotation for undo
	FRotator PreviousRotation = Module->GetActorRotation();

	// Store original transform if not already stored
	if (!OriginalModuleTransforms.Contains(Module))
	{
		OriginalModuleTransforms.Add(Module, Module->GetActorTransform());
	}

	Module->SetActorRotation(NewRotation);

	// Record action for undo
	FEditorAction Action;
	Action.ActionType = EEditorActionType::RotateModule;
	Action.Module = Module;
	Action.PreviousRotation = PreviousRotation;
	Action.NewRotation = NewRotation;
	Action.Timestamp = CurrentTime;
	RecordAction(Action);

	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::RotateModule - Rotated module to (%.2f, %.2f, %.2f)"),
		NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll);

	return true;
}

// =====================
// Validation
// =====================

EModulePlacementResult UStationEditorManager::CanPlaceModule_Implementation(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation)
{
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

	// Check collisions
	if (bCheckCollisions && CheckCollision(ModuleClass, Position, Rotation))
	{
		return EModulePlacementResult::CollisionDetected;
	}

	// Check power (warn but don't block - stations can run at deficit)
	// This is a soft check; actual placement allows it
	if (WouldCausePowerDeficit(ModuleClass))
	{
		// Log warning but still allow placement
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::CanPlaceModule - Warning: This placement would cause power deficit"));
	}

	return EModulePlacementResult::Success;
}

bool UStationEditorManager::CheckCollision(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position, FRotator Rotation) const
{
	if (!ModuleClass || !CurrentStation)
	{
		return false;
	}

	// Get default object for collision bounds
	ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	if (!DefaultModule)
	{
		return false;
	}

	// Simple sphere-based collision check with existing modules
	// A more sophisticated system would use actual mesh bounds
	// Uses the configurable CollisionRadius property

	for (ASpaceStationModule* ExistingModule : CurrentStation->Modules)
	{
		if (!ExistingModule)
		{
			continue;
		}

		float Distance = FVector::Dist(Position, ExistingModule->GetActorLocation());
		if (Distance < CollisionRadius * 2.0f)
		{
			return true; // Collision detected
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
		// Positive ModulePower = consumption
		if (Module && Module->ModulePower > 0.0f)
		{
			TotalConsumption += Module->ModulePower;
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
		// Negative ModulePower = generation (use absolute value)
		if (Module && Module->ModulePower < 0.0f)
		{
			TotalGeneration += FMath::Abs(Module->ModulePower);
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

	ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	if (!DefaultModule)
	{
		return false;
	}

	float CurrentBalance = GetPowerBalance();
	float ModulePower = DefaultModule->ModulePower;
	
	// Positive ModulePower = consumption (decreases balance)
	// Negative ModulePower = generation (subtracting negative increases balance)
	float NewBalance = CurrentBalance - ModulePower;

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

	// Snap to grid if enabled
	FVector FinalPosition = Position;
	if (bSnapToGrid && GridSystem)
	{
		FinalPosition = GridSystem->SnapToGrid(Position);
	}

	PreviewActor->UpdatePosition(FinalPosition, Rotation);

	// Update validity visual using current module class from preview actor
	if (CurrentStation && bCheckCollisions && PreviewActor->CurrentModuleClass)
	{
		bool bHasCollision = CheckCollision(PreviewActor->CurrentModuleClass, FinalPosition, Rotation);
		PreviewActor->SetValid(!bHasCollision);
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
// Internal Functions
// =====================

void UStationEditorManager::RevertChanges()
{
	// Remove all modules added this session
	for (ASpaceStationModule* Module : ModulesAddedThisSession)
	{
		if (Module && CurrentStation)
		{
			CurrentStation->RemoveModule(Module);
			Module->Destroy();
		}
	}
	ModulesAddedThisSession.Empty();

	// Restore original transforms of moved modules
	for (const auto& Pair : OriginalModuleTransforms)
	{
		ASpaceStationModule* Module = Pair.Key;
		const FTransform& OriginalTransform = Pair.Value;

		if (Module)
		{
			Module->SetActorTransform(OriginalTransform);
		}
	}
	OriginalModuleTransforms.Empty();

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

bool UStationEditorManager::ExecuteAction(const FEditorAction& Action)
{
	switch (Action.ActionType)
	{
		case EEditorActionType::PlaceModule:
			if (Action.ModuleClass && CurrentStation)
			{
				// Re-place the module without recording action (we're executing from redo stack)
				UWorld* World = CurrentStation->GetWorld();
				if (World)
				{
					FVector FinalPosition = bSnapToGrid && GridSystem ? GridSystem->SnapToGrid(Action.NewPosition) : Action.NewPosition;
					
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = CurrentStation;
					
					ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
						Action.ModuleClass,
						FinalPosition,
						Action.NewRotation,
						SpawnParams
					);
					
					if (NewModule)
					{
						FVector RelativeLocation = FinalPosition - CurrentStation->GetActorLocation();
						CurrentStation->AddModuleAtLocation(NewModule, RelativeLocation);
						AutoGenerateConnections(NewModule);
						bStatisticsDirty = true;
						NotifyPowerBalanceChanged();
						OnModulePlaced.Broadcast(NewModule);
						return true;
					}
				}
			}
			break;
			
		case EEditorActionType::RemoveModule:
			// For redo of remove, we need to find and remove the module we just recreated via undo
			// Since we stored the position, we can find it
			if (CurrentStation)
			{
				for (ASpaceStationModule* Module : CurrentStation->Modules)
				{
					if (Module && 
						FVector::DistSquared(Module->GetActorLocation(), Action.PreviousPosition) < ModuleMatchDistanceSquared &&
						Module->GetClass() == Action.ModuleClass)
					{
						// Remove connections for this module
						for (int32 i = Connections.Num() - 1; i >= 0; --i)
						{
							if (Connections[i].ModuleA == Module || Connections[i].ModuleB == Module)
							{
								Connections.RemoveAt(i);
							}
						}
						CurrentStation->RemoveModule(Module);
						Module->Destroy();
						bStatisticsDirty = true;
						NotifyPowerBalanceChanged();
						return true;
					}
				}
			}
			break;
			
		case EEditorActionType::MoveModule:
			if (Action.Module && IsValid(Action.Module) && !Action.Module->IsActorBeingDestroyed())
			{
				Action.Module->SetActorLocation(Action.NewPosition);
				return true;
			}
			else
			{
				UE_LOG(LogAdastreaStations, Warning, TEXT("Redo MoveModule failed: Module is invalid or being destroyed."));
			}
			break;
			
		case EEditorActionType::RotateModule:
			if (Action.Module && IsValid(Action.Module) && !Action.Module->IsActorBeingDestroyed())
			{
				Action.Module->SetActorRotation(Action.NewRotation);
				return true;
			}
			else
			{
				UE_LOG(LogAdastreaStations, Warning, TEXT("Redo RotateModule failed: Module is invalid or being destroyed."));
			}
			break;
			
		case EEditorActionType::UpgradeModule:
			// Upgrade actions are not undoable/redoable - just log and return false
			UE_LOG(LogAdastreaStations, Log, TEXT("UpgradeModule cannot be redone."));
			break;
			
		default:
			break;
	}
	
	return false;
}

bool UStationEditorManager::ReverseAction(const FEditorAction& Action)
{
	switch (Action.ActionType)
	{
		case EEditorActionType::PlaceModule:
			// Reverse of place = remove
			if (Action.Module && IsValid(Action.Module) && CurrentStation && !Action.Module->IsActorBeingDestroyed())
			{
				// Remove connections for this module
				for (int32 i = Connections.Num() - 1; i >= 0; --i)
				{
					if (Connections[i].ModuleA == Action.Module || Connections[i].ModuleB == Action.Module)
					{
						Connections.RemoveAt(i);
					}
				}
				
				CurrentStation->RemoveModule(Action.Module);
				Action.Module->Destroy();
				bStatisticsDirty = true;
				NotifyPowerBalanceChanged();
				return true;
			}
			break;
			
		case EEditorActionType::RemoveModule:
			// Reverse of remove = place (needs stored data to recreate)
			// Don't call PlaceModule as it records a new action
			if (Action.ModuleClass && CurrentStation)
			{
				UWorld* World = CurrentStation->GetWorld();
				if (World)
				{
					FVector FinalPosition = bSnapToGrid && GridSystem ? GridSystem->SnapToGrid(Action.PreviousPosition) : Action.PreviousPosition;
					
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = CurrentStation;
					
					ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
						Action.ModuleClass,
						FinalPosition,
						Action.PreviousRotation,
						SpawnParams
					);
					
					if (NewModule)
					{
						FVector RelativeLocation = FinalPosition - CurrentStation->GetActorLocation();
						CurrentStation->AddModuleAtLocation(NewModule, RelativeLocation);
						AutoGenerateConnections(NewModule);
						bStatisticsDirty = true;
						NotifyPowerBalanceChanged();
						OnModulePlaced.Broadcast(NewModule);
						return true;
					}
				}
			}
			break;
			
		case EEditorActionType::MoveModule:
			if (Action.Module && IsValid(Action.Module) && !Action.Module->IsActorBeingDestroyed())
			{
				Action.Module->SetActorLocation(Action.PreviousPosition);
				return true;
			}
			else
			{
				UE_LOG(LogAdastreaStations, Warning, TEXT("Undo MoveModule failed: Module is invalid or being destroyed."));
			}
			break;
			
		case EEditorActionType::RotateModule:
			if (Action.Module && IsValid(Action.Module) && !Action.Module->IsActorBeingDestroyed())
			{
				Action.Module->SetActorRotation(Action.PreviousRotation);
				return true;
			}
			else
			{
				UE_LOG(LogAdastreaStations, Warning, TEXT("Undo RotateModule failed: Module is invalid or being destroyed."));
			}
			break;
			
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
	
	// Check all existing modules for adjacency
	for (ASpaceStationModule* OtherModule : CurrentStation->Modules)
	{
		if (!OtherModule || OtherModule == Module)
		{
			continue;
		}
		
		// Check if modules are adjacent
		if (GridSystem->ArePositionsAdjacent(ModulePosition, OtherModule->GetActorLocation()))
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

	FConstructionQueueItem Item;
	Item.QueueId = NextQueueId++;
	Item.ModuleClass = ModuleClass;
	Item.TargetPosition = Position;
	Item.TargetRotation = Rotation;
	
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
		return;
	}
	
	// Spawn the module directly without recording an undo action
	// Construction from queue is a committed action, not an editor operation
	UWorld* World = CurrentStation->GetWorld();
	if (!World)
	{
		return;
	}
	
	FVector FinalPosition = bSnapToGrid && GridSystem ? GridSystem->SnapToGrid(Item.TargetPosition) : Item.TargetPosition;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CurrentStation;
	
	ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
		Item.ModuleClass,
		FinalPosition,
		Item.TargetRotation,
		SpawnParams
	);
	
	if (NewModule)
	{
		FVector RelativeLocation = FinalPosition - CurrentStation->GetActorLocation();
		CurrentStation->AddModuleAtLocation(NewModule, RelativeLocation);
		
		// Auto-generate connections
		AutoGenerateConnections(NewModule);
		
		bStatisticsDirty = true;
		NotifyPowerBalanceChanged();
		OnModulePlaced.Broadcast(NewModule);
		
		// Add completion notification
		AddNotification(FText::FromString(FString::Printf(TEXT("%s construction complete"), *NewModule->ModuleType)),
			ENotificationSeverity::Success, NewModule);
	}
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
	
	// Calculate cargo capacity using configurable default
	for (const ASpaceStationModule* Module : CurrentStation->Modules)
	{
		if (Module && Module->ModuleGroup == EStationModuleGroup::Storage)
		{
			CachedStatistics.CargoCapacity += DefaultCargoCapacityPerModule;
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
		CachedStatistics.DataNetworkUsage = TotalPossibleConnections > 0 ?
			static_cast<float>(DataConnections) / static_cast<float>(TotalPossibleConnections) : 0.0f;
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
			Capacity += DefaultPopulationCapacityPerModule;
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
			Rating += DefaultDefenseRatingPerModule;
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
	float PowerBalance = GetPowerBalance();
	if (PowerBalance < 0)
	{
		AddNotification(FText::FromString(FString::Printf(TEXT("Power deficit: %.0f MW"), FMath::Abs(PowerBalance))),
			ENotificationSeverity::Warning, nullptr);
	}
	else if (PowerBalance > GetTotalPowerGeneration() * PowerLoadWarningThreshold)
	{
		AddNotification(FText::FromString(TEXT("Power load near maximum capacity")),
			ENotificationSeverity::Warning, nullptr);
	}
	
	// Check population using configurable threshold
	FStationStatistics Stats = GetStationStatistics();
	if (Stats.CurrentPopulation > Stats.MaxPopulation * PopulationWarningThreshold)
	{
		AddNotification(FText::FromString(TEXT("Population approaching capacity. Consider adding habitation modules.")),
			ENotificationSeverity::Warning, nullptr);
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
	
	// Check if there's an upgrade entry in the catalog
	// A full implementation would check for specific upgrade paths in the catalog
	FStationModuleEntry Entry;
	if (ModuleCatalog->FindModuleByClass(Module->GetClass(), Entry))
	{
		// Check if player has a higher tech level that could unlock upgrades
		return Entry.RequiredTechLevel < PlayerTechLevel;
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
	
	// Note: Upgrade actions are not recorded to undo stack as they cannot be reversed
	
	// Add notification
	AddNotification(FText::FromString(FString::Printf(TEXT("%s upgraded successfully"), *Module->ModuleType)),
		ENotificationSeverity::Success, Module);
	
	bStatisticsDirty = true;
	NotifyPowerBalanceChanged();
	
	UE_LOG(LogAdastreaStations, Log, TEXT("StationEditorManager::UpgradeModule - Upgraded %s"), *Module->GetName());
	
	return true;
}
