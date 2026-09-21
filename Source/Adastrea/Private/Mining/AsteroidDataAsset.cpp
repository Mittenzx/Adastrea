#include "Mining/AsteroidDataAsset.h"

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
