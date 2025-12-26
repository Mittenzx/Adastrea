#include "Ships/ShipUpgradeDataAsset.h"

UShipUpgradeDataAsset::UShipUpgradeDataAsset()
	: UpgradeID(NAME_None)
	, DisplayName(FText::FromString("New Upgrade"))
	, Description(FText::GetEmpty())
	, Category(EShipUpgradeCategory::Systems)
	, Tier(EUpgradeTier::Tier1)
	, Icon(nullptr)
	, bIsUnique(false)
	, MaxStackCount(1)
{
}

float UShipUpgradeDataAsset::CalculateStatBonus(FName StatName, float BaseValue) const
{
	float FinalValue = BaseValue;

	// Apply all matching modifiers
	for (const FShipUpgradeStatModifier& Modifier : StatModifiers)
	{
		if (Modifier.StatName == StatName)
		{
			// Apply additive bonus first
			FinalValue += Modifier.AdditiveBonus;
			
			// Then apply multiplicative bonus
			FinalValue *= Modifier.MultiplicativeBonus;
		}
	}

	return FinalValue;
}

bool UShipUpgradeDataAsset::IsCompatibleWithShipType(FName ShipTypeID) const
{
	// If no restrictions, compatible with all ships
	if (CompatibleShipTypes.Num() == 0)
	{
		return true;
	}

	// Check if ship type is in compatibility list
	return CompatibleShipTypes.Contains(ShipTypeID);
}

FText UShipUpgradeDataAsset::GetTierDisplayName() const
{
	switch (Tier)
	{
		case EUpgradeTier::Tier1:
			return FText::FromString("Tier 1");
		case EUpgradeTier::Tier2:
			return FText::FromString("Tier 2");
		case EUpgradeTier::Tier3:
			return FText::FromString("Tier 3");
		case EUpgradeTier::Tier4:
			return FText::FromString("Tier 4");
		case EUpgradeTier::Tier5:
			return FText::FromString("Tier 5");
		default:
			return FText::FromString("Unknown");
	}
}

FText UShipUpgradeDataAsset::GetCategoryDisplayName() const
{
	switch (Category)
	{
		case EShipUpgradeCategory::Weapons:
			return FText::FromString("Weapons");
		case EShipUpgradeCategory::Engines:
			return FText::FromString("Engines");
		case EShipUpgradeCategory::Shields:
			return FText::FromString("Shields");
		case EShipUpgradeCategory::Hull:
			return FText::FromString("Hull");
		case EShipUpgradeCategory::Systems:
			return FText::FromString("Systems");
		case EShipUpgradeCategory::Cargo:
			return FText::FromString("Cargo");
		case EShipUpgradeCategory::Power:
			return FText::FromString("Power");
		case EShipUpgradeCategory::Sensors:
			return FText::FromString("Sensors");
		default:
			return FText::FromString("Unknown");
	}
}

FText UShipUpgradeDataAsset::GetRequirementsDescription() const
{
	TArray<FString> RequirementsList;

	if (this->Requirements.CreditCost > 0)
	{
		RequirementsList.Add(FString::Printf(TEXT("Credits: %d"), this->Requirements.CreditCost));
	}

	if (this->Requirements.RequiredPlayerLevel > 1)
	{
		RequirementsList.Add(FString::Printf(TEXT("Level: %d"), this->Requirements.RequiredPlayerLevel));
	}

	if (this->Requirements.RequiredFactionID != NAME_None)
	{
		RequirementsList.Add(FString::Printf(TEXT("Reputation: %s (%d)"),
			*this->Requirements.RequiredFactionID.ToString(),
			this->Requirements.MinimumReputation));
	}

	if (this->Requirements.RequiredMaterials.Num() > 0)
	{
		RequirementsList.Add(FString::Printf(TEXT("Materials: %d types"), this->Requirements.RequiredMaterials.Num()));
	}

	if (this->Requirements.PrerequisiteUpgrades.Num() > 0)
	{
		RequirementsList.Add(FString::Printf(TEXT("Prerequisites: %d upgrades"), this->Requirements.PrerequisiteUpgrades.Num()));
	}

	if (RequirementsList.Num() == 0)
	{
		return FText::FromString("No requirements");
	}

	return FText::FromString(FString::Join(RequirementsList, TEXT(", ")));
}
