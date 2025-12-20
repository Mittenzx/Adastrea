// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/ShipModuleDataAsset.h"

UShipModuleDataAsset::UShipModuleDataAsset()
{
	ModuleName = FText::FromString("New Module");
	Description = FText::FromString("Module description");
	ModuleID = NAME_None;
	Category = EShipModuleCategory::Utility;
	Size = EShipModuleSize::Medium;
	Mass = 100.0f;
	PowerRequirement = 10.0f;
	ModuleMesh = nullptr;
	ModuleMaterial = nullptr;
	RequiredTechLevel = 1;
	PurchaseCost = 1000;
	bIsAvailable = true;
	Manufacturer = FText::FromString("Unknown");
	RarityTier = FText::FromString("Common");
	ModuleIcon = nullptr;
}

float UShipModuleDataAsset::GetModuleRating() const
{
	// Base implementation - override in derived classes
	// Simple rating based on tech level
	return RequiredTechLevel * 10.0f;
}

bool UShipModuleDataAsset::IsSuitableForShipClass(const FString& ShipClass) const
{
	// Base implementation - most modules work on any ship
	// Override in derived classes for more specific logic
	return true;
}

FText UShipModuleDataAsset::GetModuleSummary() const
{
	FString Summary = FString::Printf(
		TEXT("%s\nSize: %s | Power: %.1f | Mass: %.1f kg\nCost: %d credits"),
		*ModuleName.ToString(),
		Size == EShipModuleSize::Small ? TEXT("Small") :
		Size == EShipModuleSize::Medium ? TEXT("Medium") :
		Size == EShipModuleSize::Large ? TEXT("Large") : TEXT("Capital"),
		PowerRequirement,
		Mass,
		PurchaseCost
	);

	return FText::FromString(Summary);
}

#if WITH_EDITOR
EDataValidationResult UShipModuleDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	// Check for required fields
	if (ModuleName.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString("Module Name is required"));
		Result = EDataValidationResult::Invalid;
	}

	if (ModuleID == NAME_None)
	{
		ValidationErrors.Add(FText::FromString("Module ID is required"));
		Result = EDataValidationResult::Invalid;
	}

	if (Description.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString("Description should be provided"));
		if (Result == EDataValidationResult::Valid)
		{
			Result = EDataValidationResult::NotValidated;
		}
	}

	// Validate ranges
	if (Mass < 0.0f)
	{
		ValidationErrors.Add(FText::FromString("Mass cannot be negative"));
		Result = EDataValidationResult::Invalid;
	}

	if (RequiredTechLevel < 1 || RequiredTechLevel > 10)
	{
		ValidationErrors.Add(FText::FromString("Tech Level must be between 1 and 10"));
		Result = EDataValidationResult::Invalid;
	}

	if (PurchaseCost < 0)
	{
		ValidationErrors.Add(FText::FromString("Purchase Cost cannot be negative"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
