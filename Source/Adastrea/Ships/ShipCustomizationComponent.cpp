// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/ShipCustomizationComponent.h"
#include "Ships/ShipModuleComponent.h"
#include "Ships/ShipModuleDataAsset.h"

UShipCustomizationComponent::UShipCustomizationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxPowerCapacity = 1000.0f;
	MaxMassCapacity = 10000.0f;
}

void UShipCustomizationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShipCustomizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ====================
// MODULE MANAGEMENT
// ====================

bool UShipCustomizationComponent::InstallModule(UShipModuleComponent* Module, FName SlotID)
{
	if (!Module)
	{
		return false;
	}

	// Find the target slot
	FShipModuleSlot* Slot = FindSlotByID(SlotID);
	if (!Slot)
	{
		return false;
	}

	// Check if slot is already occupied
	if (Slot->bIsOccupied)
	{
		return false;
	}

	// Check if module can be installed in this slot
	FText FailureReason;
	if (!CanInstallModule(Module, SlotID, FailureReason))
	{
		return false;
	}

	// Install the module
	IShipModule::Execute_OnModuleInstalled(Module, *Slot);

	// Update slot state
	UpdateSlotOccupancy(SlotID, true, Module);

	// Track installed module
	InstalledModules.Add(Module);

	// Notify
	OnModuleInstalled(Module, *Slot);

	return true;
}

UShipModuleComponent* UShipCustomizationComponent::RemoveModule(FName SlotID)
{
	// Find the slot
	FShipModuleSlot* Slot = FindSlotByID(SlotID);
	if (!Slot || !Slot->bIsOccupied)
	{
		return nullptr;
	}

	// Get the module
	UShipModuleComponent* Module = Cast<UShipModuleComponent>(Slot->InstalledModule.Get());
	if (!Module)
	{
		return nullptr;
	}

	// Remove the module
	IShipModule::Execute_OnModuleRemoved(Module, *Slot);

	// Update slot state
	UpdateSlotOccupancy(SlotID, false, nullptr);

	// Remove from tracking
	InstalledModules.Remove(Module);

	// Notify
	OnModuleRemoved(Module, *Slot);

	return Module;
}

UShipModuleComponent* UShipCustomizationComponent::ReplaceModule(UShipModuleComponent* NewModule, FName SlotID)
{
	if (!NewModule)
	{
		return nullptr;
	}

	// Remove old module first
	UShipModuleComponent* OldModule = RemoveModule(SlotID);

	// Install new module
	if (InstallModule(NewModule, SlotID))
	{
		return OldModule;
	}

	// If installation failed, try to restore old module
	if (OldModule)
	{
		InstallModule(OldModule, SlotID);
	}

	return nullptr;
}

UShipModuleComponent* UShipCustomizationComponent::GetModuleInSlot(FName SlotID) const
{
	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (Slot.SlotID == SlotID && Slot.bIsOccupied)
		{
			return Cast<UShipModuleComponent>(Slot.InstalledModule.Get());
		}
	}
	return nullptr;
}

TArray<UShipModuleComponent*> UShipCustomizationComponent::GetModulesByCategory(EShipModuleCategory Category) const
{
	TArray<UShipModuleComponent*> Result;
	
	for (UShipModuleComponent* Module : InstalledModules)
	{
		if (Module && IShipModule::Execute_GetModuleCategory(Module) == Category)
		{
			Result.Add(Module);
		}
	}

	return Result;
}

bool UShipCustomizationComponent::IsSlotOccupied(FName SlotID) const
{
	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (Slot.SlotID == SlotID)
		{
			return Slot.bIsOccupied;
		}
	}
	return false;
}

// ====================
// SLOT QUERIES
// ====================

FShipModuleSlot* UShipCustomizationComponent::FindSlotByID(FName SlotID)
{
	for (FShipModuleSlot& Slot : ModuleSlots)
	{
		if (Slot.SlotID == SlotID)
		{
			return &Slot;
		}
	}
	return nullptr;
}

TArray<FShipModuleSlot> UShipCustomizationComponent::GetSlotsByCategory(EShipModuleCategory Category) const
{
	TArray<FShipModuleSlot> Result;
	
	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (Slot.AcceptedCategory == Category)
		{
			Result.Add(Slot);
		}
	}

	return Result;
}

TArray<FShipModuleSlot> UShipCustomizationComponent::GetEmptySlots() const
{
	TArray<FShipModuleSlot> Result;
	
	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (!Slot.bIsOccupied)
		{
			Result.Add(Slot);
		}
	}

	return Result;
}

TArray<FShipModuleSlot> UShipCustomizationComponent::GetCompatibleEmptySlots(UShipModuleComponent* Module) const
{
	TArray<FShipModuleSlot> Result;
	
	if (!Module)
	{
		return Result;
	}

	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (!Slot.bIsOccupied && IShipModule::Execute_CanInstallInSlot(Module, Slot))
		{
			Result.Add(Slot);
		}
	}

	return Result;
}

// ====================
// RESOURCE CALCULATIONS
// ====================

float UShipCustomizationComponent::GetTotalPowerConsumption() const
{
	float TotalPower = 0.0f;

	for (UShipModuleComponent* Module : InstalledModules)
	{
		if (Module)
		{
			TotalPower += IShipModule::Execute_GetPowerRequirement(Module);
		}
	}

	return TotalPower;
}

float UShipCustomizationComponent::GetTotalModuleMass() const
{
	float TotalMass = 0.0f;

	for (UShipModuleComponent* Module : InstalledModules)
	{
		if (Module)
		{
			TotalMass += IShipModule::Execute_GetModuleMass(Module);
		}
	}

	return TotalMass;
}

float UShipCustomizationComponent::GetRemainingPowerCapacity() const
{
	return MaxPowerCapacity - GetTotalPowerConsumption();
}

float UShipCustomizationComponent::GetRemainingMassCapacity() const
{
	return MaxMassCapacity - GetTotalModuleMass();
}

bool UShipCustomizationComponent::HasSufficientPower(UShipModuleComponent* Module) const
{
	if (!Module)
	{
		return false;
	}

	float RequiredPower = IShipModule::Execute_GetPowerRequirement(Module);
	return GetRemainingPowerCapacity() >= RequiredPower;
}

bool UShipCustomizationComponent::HasSufficientMassCapacity(UShipModuleComponent* Module) const
{
	if (!Module)
	{
		return false;
	}

	float RequiredMass = IShipModule::Execute_GetModuleMass(Module);
	return GetRemainingMassCapacity() >= RequiredMass;
}

bool UShipCustomizationComponent::CanInstallModule(UShipModuleComponent* Module, FName SlotID, FText& OutReason) const
{
	if (!Module)
	{
		OutReason = FText::FromString("Invalid module");
		return false;
	}

	// Find slot
	const FShipModuleSlot* Slot = nullptr;
	for (const FShipModuleSlot& S : ModuleSlots)
	{
		if (S.SlotID == SlotID)
		{
			Slot = &S;
			break;
		}
	}

	if (!Slot)
	{
		OutReason = FText::FromString("Slot not found");
		return false;
	}

	if (Slot->bIsOccupied)
	{
		OutReason = FText::FromString("Slot is occupied");
		return false;
	}

	// Check compatibility
	if (!IShipModule::Execute_CanInstallInSlot(Module, *Slot))
	{
		OutReason = FText::FromString("Module not compatible with slot");
		return false;
	}

	// Check power
	if (!HasSufficientPower(Module))
	{
		OutReason = FText::FromString("Insufficient power capacity");
		return false;
	}

	// Check mass
	if (!HasSufficientMassCapacity(Module))
	{
		OutReason = FText::FromString("Insufficient mass capacity");
		return false;
	}

	OutReason = FText::FromString("Can install");
	return true;
}

// ====================
// LOADOUT MANAGEMENT
// ====================

FString UShipCustomizationComponent::GetLoadoutString() const
{
	FString Loadout = "Ship Loadout:\n";

	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		Loadout += FString::Printf(TEXT("  %s: "), *Slot.SlotName.ToString());
		
		if (Slot.bIsOccupied)
		{
			UShipModuleComponent* Module = Cast<UShipModuleComponent>(Slot.InstalledModule.Get());
			if (Module)
			{
				Loadout += IShipModule::Execute_GetModuleName(Module).ToString();
			}
			else
			{
				Loadout += TEXT("Unknown");
			}
		}
		else
		{
			Loadout += TEXT("Empty");
		}
		
		Loadout += TEXT("\n");
	}

	Loadout += FString::Printf(TEXT("\nPower: %.1f / %.1f\n"), GetTotalPowerConsumption(), MaxPowerCapacity);
	Loadout += FString::Printf(TEXT("Mass: %.1f / %.1f kg\n"), GetTotalModuleMass(), MaxMassCapacity);

	return Loadout;
}

void UShipCustomizationComponent::ClearAllModules()
{
	// Remove all modules
	TArray<FName> SlotIDs;
	for (const FShipModuleSlot& Slot : ModuleSlots)
	{
		if (Slot.bIsOccupied)
		{
			SlotIDs.Add(Slot.SlotID);
		}
	}

	for (FName SlotID : SlotIDs)
	{
		RemoveModule(SlotID);
	}

	InstalledModules.Empty();
}

// ====================
// BLUEPRINT NATIVE EVENTS
// ====================

void UShipCustomizationComponent::OnModuleInstalled_Implementation(UShipModuleComponent* Module, const FShipModuleSlot& Slot)
{
	// Override in Blueprint for custom logic
}

void UShipCustomizationComponent::OnModuleRemoved_Implementation(UShipModuleComponent* Module, const FShipModuleSlot& Slot)
{
	// Override in Blueprint for custom logic
}

void UShipCustomizationComponent::OnPowerCapacityExceeded_Implementation()
{
	// Override in Blueprint for warning/error handling
}

void UShipCustomizationComponent::OnMassCapacityExceeded_Implementation()
{
	// Override in Blueprint for warning/error handling
}

// ====================
// PRIVATE HELPERS
// ====================

void UShipCustomizationComponent::UpdateSlotOccupancy(FName SlotID, bool bOccupied, UShipModuleComponent* Module)
{
	FShipModuleSlot* Slot = FindSlotByID(SlotID);
	if (Slot)
	{
		Slot->bIsOccupied = bOccupied;
		Slot->InstalledModule = Module;
	}
}
