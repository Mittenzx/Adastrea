#include "Player/AchievementDataAsset.h"

UAchievementDataAsset::UAchievementDataAsset()
	: AchievementID(NAME_None)
	, DisplayName(FText::FromString("New Achievement"))
	, Description(FText::GetEmpty())
	, Category(EAchievementCategory::Progression)
	, Rarity(EAchievementRarity::Common)
	, bIsHidden(false)
	, Icon(nullptr)
	, TrackingStatName(NAME_None)
	, bAutoTrackedByStat(false)
	, MinimumPlayerLevel(1)
{
	ProgressData.TargetProgress = 1;
}

float UAchievementDataAsset::GetProgressPercentage() const
{
	if (ProgressData.TargetProgress <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		static_cast<float>(ProgressData.CurrentProgress) / static_cast<float>(ProgressData.TargetProgress),
		0.0f,
		1.0f
	);
}

FText UAchievementDataAsset::GetProgressText() const
{
	return FText::Format(
		FText::FromString("{0} / {1}"),
		FText::AsNumber(ProgressData.CurrentProgress),
		FText::AsNumber(ProgressData.TargetProgress)
	);
}

FText UAchievementDataAsset::GetRarityDisplayName() const
{
	switch (Rarity)
	{
		case EAchievementRarity::Common:
			return FText::FromString("Common");
		case EAchievementRarity::Uncommon:
			return FText::FromString("Uncommon");
		case EAchievementRarity::Rare:
			return FText::FromString("Rare");
		case EAchievementRarity::Epic:
			return FText::FromString("Epic");
		case EAchievementRarity::Legendary:
			return FText::FromString("Legendary");
		default:
			return FText::FromString("Unknown");
	}
}

FText UAchievementDataAsset::GetCategoryDisplayName() const
{
	switch (Category)
	{
		case EAchievementCategory::Combat:
			return FText::FromString("Combat");
		case EAchievementCategory::Trading:
			return FText::FromString("Trading");
		case EAchievementCategory::Exploration:
			return FText::FromString("Exploration");
		case EAchievementCategory::Diplomacy:
			return FText::FromString("Diplomacy");
		case EAchievementCategory::Construction:
			return FText::FromString("Construction");
		case EAchievementCategory::Personnel:
			return FText::FromString("Personnel");
		case EAchievementCategory::Progression:
			return FText::FromString("Progression");
		case EAchievementCategory::Story:
			return FText::FromString("Story");
		case EAchievementCategory::Hidden:
			return FText::FromString("Hidden");
		default:
			return FText::FromString("Unknown");
	}
}
