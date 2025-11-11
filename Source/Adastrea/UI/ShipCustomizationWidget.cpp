#include "UI/ShipCustomizationWidget.h"
#include "Ships/Spaceship.h"

UShipCustomizationWidget::UShipCustomizationWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Spaceship(nullptr)
	, MaxPowerCapacity(1000.0f)
	, bIsPreviewActive(false)
{
}

void UShipCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShipCustomizationWidget::InitializeCustomization_Implementation(ASpaceship* InSpaceship)
{
	Spaceship = InSpaceship;
	bIsPreviewActive = false;

	if (Spaceship)
	{
		InitializeHardpoints_Implementation();
		RefreshCustomization_Implementation();
	}
}

void UShipCustomizationWidget::RefreshCustomization_Implementation()
{
	// Blueprint implementation handles the actual refresh
}

bool UShipCustomizationWidget::InstallModule_Implementation(UShipCustomizationDataAsset* Module, int32 HardpointIndex)
{
	if (!Module || !Hardpoints.IsValidIndex(HardpointIndex))
	{
		return false;
	}

	// Check if module is compatible with hardpoint
	if (!CanInstallModule(Module, HardpointIndex))
	{
		return false;
	}

	// Check power capacity
	float CurrentPower = GetTotalPowerConsumption();
	if (Hardpoints[HardpointIndex].InstalledModule)
	{
		CurrentPower -= Hardpoints[HardpointIndex].InstalledModule->PowerConsumption;
	}
	if (CurrentPower + Module->PowerConsumption > MaxPowerCapacity)
	{
		return false; // Not enough power
	}

	// Install the module
	Hardpoints[HardpointIndex].InstalledModule = Module;
	RefreshCustomization_Implementation();
	return true;
}

bool UShipCustomizationWidget::RemoveModule_Implementation(int32 HardpointIndex)
{
	if (!Hardpoints.IsValidIndex(HardpointIndex))
	{
		return false;
	}

	if (!Hardpoints[HardpointIndex].InstalledModule)
	{
		return false; // Nothing to remove
	}

	Hardpoints[HardpointIndex].InstalledModule = nullptr;
	RefreshCustomization_Implementation();
	return true;
}

void UShipCustomizationWidget::PreviewModuleStats_Implementation(UShipCustomizationDataAsset* Module, int32 HardpointIndex)
{
	if (!Module)
	{
		return;
	}

	bIsPreviewActive = true;
	// Blueprint implementation handles stat preview display
}

void UShipCustomizationWidget::ClearStatsPreview_Implementation()
{
	bIsPreviewActive = false;
	// Blueprint implementation handles clearing preview
}

TArray<FShipHardpoint> UShipCustomizationWidget::GetHardpoints() const
{
	return Hardpoints;
}

TArray<FInstalledModule> UShipCustomizationWidget::GetInstalledModules() const
{
	TArray<FInstalledModule> InstalledModules;
	
	for (int32 i = 0; i < Hardpoints.Num(); i++)
	{
		if (Hardpoints[i].InstalledModule)
		{
			FInstalledModule Installed;
			Installed.Module = Hardpoints[i].InstalledModule;
			Installed.HardpointSlotIndex = i;
			Installed.bIsActive = true;
			InstalledModules.Add(Installed);
		}
	}
	
	return InstalledModules;
}

float UShipCustomizationWidget::GetTotalPowerConsumption() const
{
	float TotalPower = 0.0f;
	
	for (const FShipHardpoint& Hardpoint : Hardpoints)
	{
		if (Hardpoint.InstalledModule)
		{
			TotalPower += Hardpoint.InstalledModule->PowerConsumption;
		}
	}
	
	return TotalPower;
}

float UShipCustomizationWidget::GetTotalModuleMass() const
{
	float TotalMass = 0.0f;
	
	for (const FShipHardpoint& Hardpoint : Hardpoints)
	{
		if (Hardpoint.InstalledModule)
		{
			TotalMass += Hardpoint.InstalledModule->Mass;
		}
	}
	
	return TotalMass;
}

bool UShipCustomizationWidget::CanInstallModule(UShipCustomizationDataAsset* Module, int32 HardpointIndex) const
{
	if (!Module || !Hardpoints.IsValidIndex(HardpointIndex))
	{
		return false;
	}

	// Check if hardpoint type matches
	if (Hardpoints[HardpointIndex].HardpointType != Module->RequiredHardpoint)
	{
		return false;
	}

	// Check if compatible with ship (if spaceship reference exists)
	if (Spaceship)
	{
		FString ShipClassName = Spaceship->GetClass()->GetName();
		if (!Module->IsCompatibleWithShip(ShipClassName))
		{
			return false;
		}
	}

	return true;
}

void UShipCustomizationWidget::ApplyCustomizations()
{
	if (!Spaceship)
	{
		return;
	}

	// Blueprint can override to apply customizations to the ship
	// This could involve updating ship stats, visuals, etc.
}

void UShipCustomizationWidget::ResetCustomizations()
{
	// Remove all installed modules
	for (FShipHardpoint& Hardpoint : Hardpoints)
	{
		Hardpoint.InstalledModule = nullptr;
	}
	
	RefreshCustomization_Implementation();
}

void UShipCustomizationWidget::InitializeHardpoints_Implementation()
{
	Hardpoints.Empty();

	// Create default hardpoints
	// In a real implementation, this would query the spaceship for its hardpoint configuration
	
	// Example: Add some default hardpoints
	FShipHardpoint SmallWeapon1;
	SmallWeapon1.HardpointType = EShipHardpointType::Small;
	SmallWeapon1.HardpointName = FText::FromString(TEXT("Small Weapon 1"));
	Hardpoints.Add(SmallWeapon1);

	FShipHardpoint SmallWeapon2;
	SmallWeapon2.HardpointType = EShipHardpointType::Small;
	SmallWeapon2.HardpointName = FText::FromString(TEXT("Small Weapon 2"));
	Hardpoints.Add(SmallWeapon2);

	FShipHardpoint MediumWeapon;
	MediumWeapon.HardpointType = EShipHardpointType::Medium;
	MediumWeapon.HardpointName = FText::FromString(TEXT("Medium Weapon"));
	Hardpoints.Add(MediumWeapon);

	FShipHardpoint UtilitySlot1;
	UtilitySlot1.HardpointType = EShipHardpointType::Utility;
	UtilitySlot1.HardpointName = FText::FromString(TEXT("Utility 1"));
	Hardpoints.Add(UtilitySlot1);

	FShipHardpoint InternalSlot1;
	InternalSlot1.HardpointType = EShipHardpointType::Internal;
	InternalSlot1.HardpointName = FText::FromString(TEXT("Internal Module 1"));
	Hardpoints.Add(InternalSlot1);

	// Blueprint can override this to customize hardpoint configuration
}
