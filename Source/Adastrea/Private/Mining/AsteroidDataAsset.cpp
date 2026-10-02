#include "Mining/AsteroidDataAsset.h"
#include "Trading/TradeItemDataAsset.h"

UAsteroidDataAsset::UAsteroidDataAsset()
	: OreItem(nullptr)
	, OreYieldPerSecond(2.0f)
	, OreUnitsAtUnitScale(120.0f)
	, Hardness(1.0f)
	, Rarity(1.0f)
	, MinScale(0.6f)
	, MaxScale(1.6f)
	, OreTint(FLinearColor(0.8f, 0.5f, 0.3f, 1.0f))
{
	DisplayName = FText::FromString(TEXT("Asteroid"));
}

float UAsteroidDataAsset::GetTotalOreForScale(float Scale) const
{
	return OreUnitsAtUnitScale * FMath::Pow(FMath::Max(Scale, 0.01f), 3.0f);
}

EResourceKind UAsteroidDataAsset::GetResourceKind() const
{
	if (ResourceKind != EResourceKind::Auto || !OreItem)
	{
		return ResourceKind == EResourceKind::Auto ? EResourceKind::Rock : ResourceKind;
	}
	// The raw resources from Tools/ue_make_resource_types.py (GAS, salvage and ice there).
	static const TCHAR* Gases[] = { TEXT("Hydrogen"), TEXT("MethaneGas"), TEXT("NitrogenGas"), TEXT("NobleGas"), TEXT("Helium3") };
	static const TCHAR* Salvage[] = { TEXT("ScrapMetal"), TEXT("SalvagedComponents"), TEXT("DerelictHullPlate") };
	for (const TCHAR* Id : Gases)
	{
		if (UTradeItemDataAsset::ItemIdsMatch(OreItem->ItemID, Id))
		{
			return EResourceKind::Gas;
		}
	}
	for (const TCHAR* Id : Salvage)
	{
		if (UTradeItemDataAsset::ItemIdsMatch(OreItem->ItemID, Id))
		{
			return EResourceKind::Salvage;
		}
	}
	return UTradeItemDataAsset::ItemIdsMatch(OreItem->ItemID, TEXT("WaterIce")) ? EResourceKind::Ice : EResourceKind::Rock;
}
