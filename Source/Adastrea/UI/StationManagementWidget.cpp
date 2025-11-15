// Copyright Mittenzx. All Rights Reserved.

#include "UI/StationManagementWidget.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "TimerManager.h"

UStationManagementWidget::UStationManagementWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ManagedStation(nullptr)
	, SelectedModuleGroup(EStationModuleGroup::None)
	, bBuildModeActive(false)
{
}

void UStationManagementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start periodic update timer (every 2 seconds)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this,
			&UStationManagementWidget::RefreshStationData, 2.0f, true);
	}
}

void UStationManagementWidget::NativeDestruct()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
	}

	Super::NativeDestruct();
}

void UStationManagementWidget::SetManagedStation(ASpaceStation* Station)
{
	ManagedStation = Station;
	
	if (ManagedStation)
	{
		RefreshStationData();
	}
}

void UStationManagementWidget::RefreshStationData()
{
	if (!ManagedStation)
	{
		return;
	}

	// Fire blueprint event to update UI
	OnStationStatusUpdated();
}

TArray<ASpaceStationModule*> UStationManagementWidget::GetModulesByGroup(EStationModuleGroup Group) const
{
	TArray<ASpaceStationModule*> FilteredModules;

	if (!ManagedStation)
	{
		return FilteredModules;
	}

	// Get all attached modules
	TArray<AActor*> AttachedActors;
	ManagedStation->GetAttachedActors(AttachedActors);

	for (AActor* Actor : AttachedActors)
	{
		ASpaceStationModule* Module = Cast<ASpaceStationModule>(Actor);
		if (Module)
		{
			// Filter by group if specified
			if (Group == EStationModuleGroup::None || Module->GetModuleGroup() == Group)
			{
				FilteredModules.Add(Module);
			}
		}
	}

	return FilteredModules;
}

int32 UStationManagementWidget::GetTotalModuleCount() const
{
	return GetModulesByGroup(EStationModuleGroup::None).Num();
}

float UStationManagementWidget::GetOperationalStatusPercent() const
{
	if (!ManagedStation)
	{
		return 0.0f;
	}

	TArray<ASpaceStationModule*> AllModules = GetModulesByGroup(EStationModuleGroup::None);
	if (AllModules.Num() == 0)
	{
		return 100.0f; // No modules = fully operational (empty station)
	}

	int32 OperationalCount = 0;
	for (ASpaceStationModule* Module : AllModules)
	{
		if (Module && Module->IsOperational())
		{
			OperationalCount++;
		}
	}

	return (static_cast<float>(OperationalCount) / static_cast<float>(AllModules.Num())) * 100.0f;
}

bool UStationManagementWidget::CanBuildModule(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!ManagedStation || !ModuleClass)
	{
		return false;
	}

	// Basic checks - in a full implementation, this would check:
	// - Available resources
	// - Available attachment points
	// - Module prerequisites
	// - Player permissions
	
	return true; // Placeholder - implement full logic in Blueprint or here
}

bool UStationManagementWidget::RequestBuildModule(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	if (!CanBuildModule(ModuleClass))
	{
		return false;
	}

	// Placeholder for module construction logic
	// In a full implementation:
	// 1. Check resources
	// 2. Find attachment point
	// 3. Spawn module
	// 4. Deduct resources
	// 5. Fire OnModuleAdded event

	return false; // Implement in derived Blueprint class or extend here
}

bool UStationManagementWidget::RequestRemoveModule(ASpaceStationModule* Module)
{
	if (!Module || !ManagedStation)
	{
		return false;
	}

	// Placeholder for module removal logic
	// In a full implementation:
	// 1. Check if module can be removed (no dependencies)
	// 2. Return resources
	// 3. Destroy module
	// 4. Fire OnModuleRemoved event

	return false; // Implement in derived Blueprint class or extend here
}

void UStationManagementWidget::SetSelectedModuleGroup(EStationModuleGroup Group)
{
	SelectedModuleGroup = Group;
	RefreshStationData();
}

void UStationManagementWidget::ToggleBuildMode()
{
	bBuildModeActive = !bBuildModeActive;
	RefreshStationData();
}

void UStationManagementWidget::CloseInterface()
{
	RemoveFromParent();
}

void UStationManagementWidget::HandleStationChanged()
{
	RefreshStationData();
}
