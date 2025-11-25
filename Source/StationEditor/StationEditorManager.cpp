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
	
	// Cache initial power balance
	LastPowerBalance = GetPowerBalance();

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
		if (PreviewActor && !PreviewActor->IsValidLowLevel())
		{
			PreviewActor = nullptr;
		}
		
		// Destroy existing preview actor before creating new one
		if (PreviewActor)
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
	if (PreviewActor && PreviewActor->IsValidLowLevel())
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

	// Remove from station
	if (!CurrentStation->RemoveModule(Module))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationEditorManager::RemoveModule - Failed to remove module from station"));
		return false;
	}

	// Remove from tracking
	ModulesAddedThisSession.Remove(Module);
	OriginalModuleTransforms.Remove(Module);

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

	// Store original transform if not already stored
	if (!OriginalModuleTransforms.Contains(Module))
	{
		OriginalModuleTransforms.Add(Module, Module->GetActorTransform());
	}

	Module->SetActorRotation(NewRotation);

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
