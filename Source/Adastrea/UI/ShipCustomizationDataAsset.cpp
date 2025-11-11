#include "UI/ShipCustomizationDataAsset.h"

float UShipCustomizationDataAsset::GetStatModifier(FName StatName, float DefaultValue) const
{
	const float* FoundValue = StatModifiers.Find(StatName);
	return FoundValue ? *FoundValue : DefaultValue;
}

FString UShipCustomizationDataAsset::GetCustomProperty(FName PropertyName, const FString& DefaultValue) const
{
	const FString* FoundValue = CustomProperties.Find(PropertyName);
	return FoundValue ? *FoundValue : DefaultValue;
}

bool UShipCustomizationDataAsset::IsCompatibleWithShip(const FString& ShipClassName) const
{
	// If no ship class restrictions, compatible with all ships
	if (RestrictedToShipClasses.Num() == 0)
	{
		return true;
	}

	// Check if ship class is in the allowed list
	return RestrictedToShipClasses.Contains(ShipClassName);
}
