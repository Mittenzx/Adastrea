// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/ShipModuleComponent.h"
#include "Ships/ShipModuleDataAsset.h"
#include "Components/StaticMeshComponent.h"

UShipModuleComponent::UShipModuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Enable only when needed

	ModuleData = nullptr;
	ModuleMeshComponent = nullptr;
	bIsEnabled = false;
	bIsCurrentlyInstalled = false;
}

void UShipModuleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize mesh component if needed
	if (ModuleData && !ModuleMeshComponent)
	{
		UpdateModuleMesh();
	}
}

void UShipModuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup mesh component
	if (ModuleMeshComponent)
	{
		ModuleMeshComponent->DestroyComponent();
		ModuleMeshComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UShipModuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsEnabled && bIsCurrentlyInstalled)
	{
		UpdateModule(DeltaTime);
	}
}

// ====================
// IShipModule Interface Implementation
// ====================

EShipModuleCategory UShipModuleComponent::GetModuleCategory_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->Category;
	}
	return IShipModule::GetModuleCategory_Implementation();
}

EShipModuleSize UShipModuleComponent::GetModuleSize_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->Size;
	}
	return IShipModule::GetModuleSize_Implementation();
}

FText UShipModuleComponent::GetModuleName_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->ModuleName;
	}
	return IShipModule::GetModuleName_Implementation();
}

FText UShipModuleComponent::GetModuleDescription_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->Description;
	}
	return IShipModule::GetModuleDescription_Implementation();
}

float UShipModuleComponent::GetPowerRequirement_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->PowerRequirement;
	}
	return IShipModule::GetPowerRequirement_Implementation();
}

float UShipModuleComponent::GetModuleMass_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->Mass;
	}
	return IShipModule::GetModuleMass_Implementation();
}

bool UShipModuleComponent::CanInstallInSlot_Implementation(const FShipModuleSlot& Slot) const
{
	// Use base interface implementation for basic checks
	if (!IShipModule::CanInstallInSlot_Implementation(Slot))
	{
		return false;
	}

	// Additional checks can be added by derived classes
	return true;
}

void UShipModuleComponent::OnModuleInstalled_Implementation(const FShipModuleSlot& Slot)
{
	CurrentSlot = Slot;
	bIsCurrentlyInstalled = true;

	// Update mesh position and rotation
	if (ModuleMeshComponent)
	{
		ModuleMeshComponent->SetRelativeLocation(Slot.SlotLocation);
		ModuleMeshComponent->SetRelativeRotation(Slot.SlotRotation);
		ModuleMeshComponent->SetVisibility(true);
	}

	// Enable the module
	SetModuleEnabled(true);

	IShipModule::OnModuleInstalled_Implementation(Slot);
}

void UShipModuleComponent::OnModuleRemoved_Implementation(const FShipModuleSlot& Slot)
{
	// Disable the module
	SetModuleEnabled(false);

	// Hide mesh
	if (ModuleMeshComponent)
	{
		ModuleMeshComponent->SetVisibility(false);
	}

	bIsCurrentlyInstalled = false;
	CurrentSlot = FShipModuleSlot();

	IShipModule::OnModuleRemoved_Implementation(Slot);
}

UStaticMesh* UShipModuleComponent::GetModuleMesh_Implementation() const
{
	if (ModuleData)
	{
		return ModuleData->ModuleMesh;
	}
	return IShipModule::GetModuleMesh_Implementation();
}

bool UShipModuleComponent::IsInstalled_Implementation() const
{
	return bIsCurrentlyInstalled;
}

FShipModuleSlot UShipModuleComponent::GetInstalledSlot_Implementation() const
{
	return CurrentSlot;
}

void UShipModuleComponent::SetModuleEnabled_Implementation(bool bEnabled)
{
	if (bIsEnabled == bEnabled)
	{
		return; // No change
	}

	bIsEnabled = bEnabled;

	if (bEnabled)
	{
		SetComponentTickEnabled(true);
		OnModuleEnabled();
	}
	else
	{
		SetComponentTickEnabled(false);
		OnModuleDisabled();
	}
}

bool UShipModuleComponent::IsModuleEnabled_Implementation() const
{
	return bIsEnabled;
}

// ====================
// MODULE OPERATIONS
// ====================

void UShipModuleComponent::InitializeWithData(UShipModuleDataAsset* InModuleData)
{
	if (!InModuleData)
	{
		return;
	}

	ModuleData = InModuleData;
	UpdateModuleMesh();
}

void UShipModuleComponent::UpdateModuleMesh()
{
	if (!ModuleData)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create mesh component if it doesn't exist
	if (!ModuleMeshComponent && ModuleData->ModuleMesh)
	{
		ModuleMeshComponent = NewObject<UStaticMeshComponent>(Owner, UStaticMeshComponent::StaticClass());
		if (ModuleMeshComponent)
		{
			ModuleMeshComponent->RegisterComponent();
			ModuleMeshComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			Owner->AddInstanceComponent(ModuleMeshComponent);
			ModuleMeshComponent->SetStaticMesh(ModuleData->ModuleMesh);
			
			// Apply material if specified
			if (ModuleData->ModuleMaterial)
			{
				ModuleMeshComponent->SetMaterial(0, ModuleData->ModuleMaterial);
			}

			// Initially hidden until installed
			ModuleMeshComponent->SetVisibility(false);
		}
	}
	else if (ModuleMeshComponent && ModuleData->ModuleMesh)
	{
		// Update existing mesh
		ModuleMeshComponent->SetStaticMesh(ModuleData->ModuleMesh);
		
		if (ModuleData->ModuleMaterial)
		{
			ModuleMeshComponent->SetMaterial(0, ModuleData->ModuleMaterial);
		}
	}
}

FText UShipModuleComponent::GetModuleStatus() const
{
	if (!bIsCurrentlyInstalled)
	{
		return FText::FromString("Not Installed");
	}
	
	if (!bIsEnabled)
	{
		return FText::FromString("Disabled");
	}

	return FText::FromString("Operational");
}

// ====================
// Protected Blueprint Native Events
// ====================

void UShipModuleComponent::OnModuleEnabled_Implementation()
{
	// Override in derived classes
}

void UShipModuleComponent::OnModuleDisabled_Implementation()
{
	// Override in derived classes
}

void UShipModuleComponent::UpdateModule_Implementation(float DeltaTime)
{
	// Override in derived classes for per-frame updates
}
