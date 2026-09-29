// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/OutfittingModule.h"
#include "Ships/ShipUpgradeCatalogSubsystem.h"

AOutfittingModule::AOutfittingModule()
{
	ModuleType = TEXT("Outfitting Bay");
	ModulePower = 60.0f;
	ModuleGroup = EStationModuleGroup::Docking;

	ServicedCategories = {
		EShipUpgradeCategory::Engines,
		EShipUpgradeCategory::Weapons,
		EShipUpgradeCategory::Shields,
		EShipUpgradeCategory::Hull,
		EShipUpgradeCategory::Cargo,
	};
	BaseMaxTier = EUpgradeTier::Tier2;
}

bool AOutfittingModule::ServicesCategory(EShipUpgradeCategory Category) const
{
	return !IsDestroyed_Implementation() && ServicedCategories.Contains(Category);
}

EUpgradeTier AOutfittingModule::GetMaxTier() const
{
	const int32 Tier = FMath::Clamp(static_cast<int32>(BaseMaxTier) + UpgradeLevel, 0, static_cast<int32>(EUpgradeTier::Tier5));
	return static_cast<EUpgradeTier>(Tier);
}

bool AOutfittingModule::CanFit(const UShipUpgradeDataAsset* Upgrade) const
{
	return Upgrade && ServicesCategory(Upgrade->Category) && Upgrade->Tier <= GetMaxTier();
}

TArray<UShipUpgradeDataAsset*> AOutfittingModule::GetStock(EShipUpgradeCategory Category) const
{
	TArray<UShipUpgradeDataAsset*> Stock;
	if (!ServicesCategory(Category))
	{
		return Stock;
	}

	if (const UShipUpgradeCatalogSubsystem* Catalog = UShipUpgradeCatalogSubsystem::Get(this))
	{
		Stock = Catalog->GetUpgrades(Category, GetMaxTier());
	}
	for (UShipUpgradeDataAsset* Extra : AdditionalStock)
	{
		if (CanFit(Extra) && Extra->Category == Category)
		{
			Stock.AddUnique(Extra);
		}
	}
	return Stock;
}

int32 AOutfittingModule::GetPrice(const UShipUpgradeDataAsset* Upgrade) const
{
	return Upgrade ? FMath::RoundToInt(Upgrade->Requirements.CreditCost * PriceMultiplier) : 0;
}

// ---- Specialist workshops: one category, one tier higher than the general bay ----

AEngineWorkshopModule::AEngineWorkshopModule()
{
	ModuleType = TEXT("Engine Workshop");
	ModulePower = 80.0f;
	ServicedCategories = { EShipUpgradeCategory::Engines };
	BaseMaxTier = EUpgradeTier::Tier3;
}

AArmouryModule::AArmouryModule()
{
	ModuleType = TEXT("Armoury");
	ModulePower = 70.0f;
	ModuleGroup = EStationModuleGroup::Defence;
	ServicedCategories = { EShipUpgradeCategory::Weapons };
	BaseMaxTier = EUpgradeTier::Tier3;
}

AShieldWorkshopModule::AShieldWorkshopModule()
{
	ModuleType = TEXT("Shield Workshop");
	ModulePower = 90.0f;
	ModuleGroup = EStationModuleGroup::Defence;
	ServicedCategories = { EShipUpgradeCategory::Shields };
	BaseMaxTier = EUpgradeTier::Tier3;
}

AHullWorksModule::AHullWorksModule()
{
	ModuleType = TEXT("Hull Works");
	ModulePower = 60.0f;
	ServicedCategories = { EShipUpgradeCategory::Hull };
	BaseMaxTier = EUpgradeTier::Tier3;
}

ACargoRefitModule::ACargoRefitModule()
{
	ModuleType = TEXT("Cargo Refit Yard");
	ModulePower = 40.0f;
	ModuleGroup = EStationModuleGroup::Storage;
	ServicedCategories = { EShipUpgradeCategory::Cargo };
	BaseMaxTier = EUpgradeTier::Tier3;
}
