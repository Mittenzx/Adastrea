// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "UI/ModularShipCustomizationWidget.h"
#include "Ships/ShipCustomizationComponent.h"
#include "Ships/ShipModuleComponent.h"

void UModularShipCustomizationWidget::Initialize(UShipCustomizationComponent* InCustomizationComponent)
{
	CustomizationComponent = InCustomizationComponent;
	bHasSlotSelected = false;
	bHasModuleSelected = false;
	SelectedModule = nullptr;
	
	RefreshUI();
}

void UModularShipCustomizationWidget::RefreshUI()
{
	OnUIRefresh();
}

// ====================
// SLOT OPERATIONS
// ====================

void UModularShipCustomizationWidget::SelectSlot(FName SlotID)
{
	if (!CustomizationComponent)
	{
		return;
	}

	FShipModuleSlot Slot;
	if (CustomizationComponent->FindSlotByID(SlotID, Slot))
	{
		SelectedSlot = Slot;
		bHasSlotSelected = true;
		OnSlotSelected(SelectedSlot);
	}
}

void UModularShipCustomizationWidget::ClearSlotSelection()
{
	bHasSlotSelected = false;
	SelectedSlot = FShipModuleSlot();
}

TArray<FShipModuleSlot> UModularShipCustomizationWidget::GetAllSlots() const
{
	if (!CustomizationComponent)
	{
		return TArray<FShipModuleSlot>();
	}

	return CustomizationComponent->ModuleSlots;
}

TArray<FShipModuleSlot> UModularShipCustomizationWidget::GetSlotsByCategory(EShipModuleCategory Category) const
{
	if (!CustomizationComponent)
	{
		return TArray<FShipModuleSlot>();
	}

	return CustomizationComponent->GetSlotsByCategory(Category);
}

// ====================
// MODULE OPERATIONS
// ====================

void UModularShipCustomizationWidget::SelectModule(UShipModuleComponent* Module)
{
	if (!Module)
	{
		return;
	}

	SelectedModule = Module;
	bHasModuleSelected = true;
	OnModuleSelected(Module);
}

void UModularShipCustomizationWidget::ClearModuleSelection()
{
	bHasModuleSelected = false;
	SelectedModule = nullptr;
}

bool UModularShipCustomizationWidget::InstallSelectedModule()
{
	if (!CustomizationComponent || !bHasSlotSelected || !bHasModuleSelected || !SelectedModule)
	{
		OnInstallationFailed(FText::FromString("Invalid selection"));
		return false;
	}

	FText FailureReason;
	if (!CanInstallSelectedModule(FailureReason))
	{
		OnInstallationFailed(FailureReason);
		return false;
	}

	if (CustomizationComponent->InstallModule(SelectedModule, SelectedSlot.SlotID))
	{
		OnModuleInstalled(SelectedModule, SelectedSlot);
		RefreshUI();
		return true;
	}

	OnInstallationFailed(FText::FromString("Installation failed"));
	return false;
}

UShipModuleComponent* UModularShipCustomizationWidget::RemoveSelectedSlotModule()
{
	if (!CustomizationComponent || !bHasSlotSelected)
	{
		return nullptr;
	}

	UShipModuleComponent* RemovedModule = CustomizationComponent->RemoveModule(SelectedSlot.SlotID);
	
	if (RemovedModule)
	{
		OnModuleRemoved(RemovedModule, SelectedSlot);
		RefreshUI();
	}

	return RemovedModule;
}

bool UModularShipCustomizationWidget::CanInstallSelectedModule(FText& OutReason) const
{
	if (!CustomizationComponent || !SelectedModule || !bHasSlotSelected)
	{
		OutReason = FText::FromString("Invalid selection");
		return false;
	}

	return CustomizationComponent->CanInstallModule(SelectedModule, SelectedSlot.SlotID, OutReason);
}

// ====================
// INFORMATION QUERIES
// ====================

void UModularShipCustomizationWidget::GetPowerStats(float& OutCurrent, float& OutMax, float& OutRemaining) const
{
	if (!CustomizationComponent)
	{
		OutCurrent = 0.0f;
		OutMax = 0.0f;
		OutRemaining = 0.0f;
		return;
	}

	OutCurrent = CustomizationComponent->GetTotalPowerConsumption();
	OutMax = CustomizationComponent->MaxPowerCapacity;
	OutRemaining = CustomizationComponent->GetRemainingPowerCapacity();
}

void UModularShipCustomizationWidget::GetMassStats(float& OutCurrent, float& OutMax, float& OutRemaining) const
{
	if (!CustomizationComponent)
	{
		OutCurrent = 0.0f;
		OutMax = 0.0f;
		OutRemaining = 0.0f;
		return;
	}

	OutCurrent = CustomizationComponent->GetTotalModuleMass();
	OutMax = CustomizationComponent->MaxMassCapacity;
	OutRemaining = CustomizationComponent->GetRemainingMassCapacity();
}

FText UModularShipCustomizationWidget::GetPowerUsageText() const
{
	float Current, Max, Remaining;
	GetPowerStats(Current, Max, Remaining);

	float Percentage = (Max > 0.0f) ? (Current / Max) * 100.0f : 0.0f;

	return FText::FromString(FString::Printf(TEXT("%.0f / %.0f (%.1f%%)"), Current, Max, Percentage));
}

FText UModularShipCustomizationWidget::GetMassUsageText() const
{
	float Current, Max, Remaining;
	GetMassStats(Current, Max, Remaining);

	float Percentage = (Max > 0.0f) ? (Current / Max) * 100.0f : 0.0f;

	return FText::FromString(FString::Printf(TEXT("%.0f / %.0f kg (%.1f%%)"), Current, Max, Percentage));
}

// ====================
// BLUEPRINT NATIVE EVENTS
// ====================

void UModularShipCustomizationWidget::OnUIRefresh_Implementation()
{
	// Override in Blueprint to refresh visual elements
}

void UModularShipCustomizationWidget::OnSlotSelected_Implementation(const FShipModuleSlot& Slot)
{
	// Override in Blueprint to highlight slot
}

void UModularShipCustomizationWidget::OnModuleSelected_Implementation(UShipModuleComponent* Module)
{
	// Override in Blueprint to show module details
}

void UModularShipCustomizationWidget::OnModuleInstalled_Implementation(UShipModuleComponent* Module, const FShipModuleSlot& Slot)
{
	// Override in Blueprint for success feedback
}

void UModularShipCustomizationWidget::OnModuleRemoved_Implementation(UShipModuleComponent* Module, const FShipModuleSlot& Slot)
{
	// Override in Blueprint for removal feedback
}

void UModularShipCustomizationWidget::OnInstallationFailed_Implementation(const FText& Reason)
{
	// Override in Blueprint to show error message
}
