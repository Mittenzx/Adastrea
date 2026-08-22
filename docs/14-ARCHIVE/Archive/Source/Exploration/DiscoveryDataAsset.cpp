#include "Exploration/DiscoveryDataAsset.h"

UDiscoveryDataAsset::UDiscoveryDataAsset()
{
	// Initialize defaults
	DiscoveryName = FText::FromString("Unknown Location");
	Description = FText::FromString("An undiscovered location of interest.");
	DiscoveryType = EDiscoveryType::MysteryLocation;
	DiscoveryID = NAME_None;

	// Detection defaults
	SignalStrength = 50;
	RequiredScannerType = EScannerTypeRequirement::Any;
	bRequiresDeepScan = false;
	MinimumScanLevel = EScanDetailLevel::Basic;
	DetectionRangeMultiplier = 1.0f;
	bIsHidden = false;

	// Rarity defaults
	RarityTier = ERarityTier::Common;
	DiscoveryValue = 100;
	HistoricalSignificance = 50;

	// Reward defaults
	CreditsReward = 1000;
	ExperienceReward = 200;
	ReputationReward = 15;

	// Content defaults
	AssociatedFactionID = NAME_None;
	EstimatedAge = 0;
	Condition = 50;

	// Quest integration defaults
	TriggeredQuestID = NAME_None;
	bUnlocksQuests = false;
	bIsQuestObjective = false;
	QuestObjectiveID = NAME_None;

	// Interaction defaults
	bCanBeExplored = true;
	ExplorationTime = 60.0f;
	bHasHostiles = false;
	HostileDifficulty = 1;
	InteractionRadius = 20000.0f; // 200m
	bCanBeSalvaged = false;
	SalvageValue = 0;
	bCanRevisit = true;

	// Presentation defaults
	DiscoveryActorClass = nullptr;
	ParticleEffect = nullptr;
	NiagaraEffect = nullptr;
	AmbientSound = nullptr;
	DiscoverySound = nullptr;
}

int32 UDiscoveryDataAsset::GetTotalRewardValue(bool bIncludeItems, bool bIncludeSalvage) const
{
	int32 TotalValue = CreditsReward;

	// Add experience value (1 XP = 10 credits equivalent)
	TotalValue += ExperienceReward * 10;

	// Add reputation value (1 rep = 50 credits equivalent)
	TotalValue += ReputationReward * 50;

	// Add discovery value
	TotalValue += DiscoveryValue;

	// Optionally add item values (estimated at 100 credits per item)
	if (bIncludeItems)
	{
		for (const FItemReward& Item : ItemRewards)
		{
			TotalValue += Item.Quantity * 100;
		}

		// Story items are valuable (500 credits each)
		TotalValue += StoryItems.Num() * 500;
	}

	// Optionally add salvage value
	if (bIncludeSalvage && bCanBeSalvaged)
	{
		TotalValue += SalvageValue;
	}

	return TotalValue;
}

float UDiscoveryDataAsset::GetRarityMultiplier() const
{
	switch (RarityTier)
	{
		case ERarityTier::Common:
			return 1.0f;
		case ERarityTier::Uncommon:
			return 1.5f;
		case ERarityTier::Rare:
			return 2.5f;
		case ERarityTier::VeryRare:
			return 4.0f;
		case ERarityTier::Legendary:
			return 5.0f;
		default:
			return 1.0f;
	}
}

bool UDiscoveryDataAsset::CanExplore(int32 PlayerLevel, FText& OutReason) const
{
	// Check if explorable
	if (!bCanBeExplored)
	{
		OutReason = FText::FromString("This location cannot be explored.");
		return false;
	}

	// Check hostiles and player level
	if (bHasHostiles && PlayerLevel < HostileDifficulty)
	{
		OutReason = FText::Format(
			FText::FromString("This location contains hostiles too powerful for your level. Recommended level: {0}"),
			FText::AsNumber(HostileDifficulty)
		);
		return false;
	}

	OutReason = FText::GetEmpty();
	return true;
}

FLinearColor UDiscoveryDataAsset::GetRarityColor() const
{
	switch (RarityTier)
	{
		case ERarityTier::Common:
			return FLinearColor(0.8f, 0.8f, 0.8f); // Light gray
		case ERarityTier::Uncommon:
			return FLinearColor(0.0f, 1.0f, 0.0f); // Green
		case ERarityTier::Rare:
			return FLinearColor(0.0f, 0.5f, 1.0f); // Blue
		case ERarityTier::VeryRare:
			return FLinearColor(0.8f, 0.0f, 1.0f); // Purple
		case ERarityTier::Legendary:
			return FLinearColor(1.0f, 0.6f, 0.0f); // Orange
		default:
			return FLinearColor::White;
	}
}

FText UDiscoveryDataAsset::GetRarityDisplayName() const
{
	switch (RarityTier)
	{
		case ERarityTier::Common:
			return FText::FromString("Common");
		case ERarityTier::Uncommon:
			return FText::FromString("Uncommon");
		case ERarityTier::Rare:
			return FText::FromString("Rare");
		case ERarityTier::VeryRare:
			return FText::FromString("Very Rare");
		case ERarityTier::Legendary:
			return FText::FromString("Legendary");
		default:
			return FText::FromString("Unknown");
	}
}

int32 UDiscoveryDataAsset::RollItemRewards(TArray<FItemReward>& OutItems) const
{
	OutItems.Empty();

	for (const FItemReward& Item : ItemRewards)
	{
		// Roll against drop chance
		int32 Roll = FMath::RandRange(1, 100);
		if (Roll <= Item.DropChance)
		{
			OutItems.Add(Item);
		}
	}

	return OutItems.Num();
}

FText UDiscoveryDataAsset::GetDiscoveryTypeDisplayName(EDiscoveryType Type)
{
	switch (Type)
	{
		case EDiscoveryType::ArchaeologicalSite:
			return FText::FromString("Archaeological Site");
		case EDiscoveryType::DerelictShip:
			return FText::FromString("Derelict Ship");
		case EDiscoveryType::DerelictStation:
			return FText::FromString("Derelict Station");
		case EDiscoveryType::AbandonedOutpost:
			return FText::FromString("Abandoned Outpost");
		case EDiscoveryType::AncientRuins:
			return FText::FromString("Ancient Ruins");
		case EDiscoveryType::CrashedVessel:
			return FText::FromString("Crashed Vessel");
		case EDiscoveryType::HiddenCache:
			return FText::FromString("Hidden Cache");
		case EDiscoveryType::SecretFacility:
			return FText::FromString("Secret Facility");
		case EDiscoveryType::AlienArtifact:
			return FText::FromString("Alien Artifact");
		case EDiscoveryType::MysteryLocation:
		default:
			return FText::FromString("Mystery Location");
	}
}

FText UDiscoveryDataAsset::GetConditionText() const
{
	if (Condition >= 90)
	{
		return FText::FromString("Pristine");
	}
	else if (Condition >= 70)
	{
		return FText::FromString("Excellent");
	}
	else if (Condition >= 50)
	{
		return FText::FromString("Good");
	}
	else if (Condition >= 30)
	{
		return FText::FromString("Fair");
	}
	else if (Condition >= 10)
	{
		return FText::FromString("Poor");
	}
	else
	{
		return FText::FromString("Ruined");
	}
}
