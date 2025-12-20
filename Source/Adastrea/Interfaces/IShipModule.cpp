// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Interfaces/IShipModule.h"

// Default implementations for IShipModule interface

EShipModuleCategory IShipModule::GetModuleCategory_Implementation() const
{
	return EShipModuleCategory::Utility;
}

EShipModuleSize IShipModule::GetModuleSize_Implementation() const
{
	return EShipModuleSize::Medium;
}

FText IShipModule::GetModuleName_Implementation() const
{
	return FText::FromString("Unknown Module");
}

FText IShipModule::GetModuleDescription_Implementation() const
{
	return FText::FromString("No description available");
}

float IShipModule::GetPowerRequirement_Implementation() const
{
	return 0.0f;
}

float IShipModule::GetModuleMass_Implementation() const
{
	return 100.0f;
}

bool IShipModule::CanInstallInSlot_Implementation(const FShipModuleSlot& Slot) const
{
	// Check category compatibility
	if (Slot.AcceptedCategory != Execute_GetModuleCategory(Cast<UObject>(this)))
	{
		return false;
	}

	// Check size compatibility (module must be same size or smaller)
	EShipModuleSize ModuleSize = Execute_GetModuleSize(Cast<UObject>(this));
	if (ModuleSize > Slot.MaxSize)
	{
		return false;
	}

	// Check if slot is not occupied
	if (Slot.bIsOccupied)
	{
		return false;
	}

	return true;
}

void IShipModule::OnModuleInstalled_Implementation(const FShipModuleSlot& Slot)
{
	// Default implementation does nothing
	// Override in derived classes for custom installation logic
}

void IShipModule::OnModuleRemoved_Implementation(const FShipModuleSlot& Slot)
{
	// Default implementation does nothing
	// Override in derived classes for custom removal logic
}

UStaticMesh* IShipModule::GetModuleMesh_Implementation() const
{
	return nullptr;
}

bool IShipModule::IsInstalled_Implementation() const
{
	return false;
}

FShipModuleSlot IShipModule::GetInstalledSlot_Implementation() const
{
	return FShipModuleSlot();
}

void IShipModule::SetModuleEnabled_Implementation(bool bEnabled)
{
	// Default implementation does nothing
	// Override in derived classes for enable/disable logic
}

bool IShipModule::IsModuleEnabled_Implementation() const
{
	return true;
}
