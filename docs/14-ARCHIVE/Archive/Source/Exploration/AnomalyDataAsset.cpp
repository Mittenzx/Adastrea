#include "Exploration/AnomalyDataAsset.h"

UAnomalyDataAsset::UAnomalyDataAsset()
{
	// Initialize defaults
	AnomalyName = FText::FromString("Unknown Anomaly");
	Description = FText::FromString("A spatial phenomenon requiring investigation.");
	AnomalyType = EAnomalyType::UnknownPhenomenon;
	AnomalyID = NAME_None;

	// Detection defaults
	SignalStrength = 50;
	RequiredScannerType = EScannerTypeRequirement::Any;
	bRequiresDeepScan = false;
	MinimumScanLevel = EScanDetailLevel::Basic;
	DetectionRangeMultiplier = 1.0f;

	// Rarity defaults
	RarityTier = ERarityTier::Common;
	DiscoveryValue = 100;
	ScientificSignificance = 50;

	// Reward defaults
	CreditsReward = 500;
	ExperienceReward = 100;
	ReputationReward = 10;

	// Presentation defaults
	ParticleEffect = nullptr;
	NiagaraEffect = nullptr;
	SoundEffect = nullptr;
	AmbientSound = nullptr;
	EffectColor = FLinearColor::White;
	EffectScale = 1.0f;

	// Interaction defaults
	bCanBeHarvested = false;
	HarvestTime = 30.0f;
	bIsHostile = false;
	DamagePerSecond = 0.0f;
	InteractionRadius = 10000.0f; // 100m
	InteractionEnergyCost = 50;
	bIsReusable = false;
	InteractionCooldown = 300.0f; // 5 minutes

	// Lore defaults
	AssociatedQuestID = NAME_None;
}

int32 UAnomalyDataAsset::GetTotalRewardValue(bool bIncludeItems) const
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
	}

	return TotalValue;
}

float UAnomalyDataAsset::GetRarityMultiplier() const
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

bool UAnomalyDataAsset::CanInteract(int32 PlayerEnergy, FText& OutReason) const
{
	// Check if hostile
	if (bIsHostile)
	{
		OutReason = FText::FromString("This anomaly is too dangerous to approach!");
		return false;
	}

	// Check if harvestable
	if (!bCanBeHarvested)
	{
		OutReason = FText::FromString("This anomaly cannot be interacted with.");
		return false;
	}

	// Check energy requirements
	if (PlayerEnergy < InteractionEnergyCost)
	{
		OutReason = FText::Format(
			FText::FromString("Insufficient energy. Required: {0}, Available: {1}"),
			FText::AsNumber(InteractionEnergyCost),
			FText::AsNumber(PlayerEnergy)
		);
		return false;
	}

	OutReason = FText::GetEmpty();
	return true;
}

FLinearColor UAnomalyDataAsset::GetRarityColor() const
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

FText UAnomalyDataAsset::GetRarityDisplayName() const
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

int32 UAnomalyDataAsset::RollItemRewards(TArray<FItemReward>& OutItems) const
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

FText UAnomalyDataAsset::GetAnomalyTypeDisplayName(EAnomalyType Type)
{
	switch (Type)
	{
		case EAnomalyType::GravitationalAnomaly:
			return FText::FromString("Gravitational Anomaly");
		case EAnomalyType::TemporalDistortion:
			return FText::FromString("Temporal Distortion");
		case EAnomalyType::EnergyField:
			return FText::FromString("Energy Field");
		case EAnomalyType::WormholeSignature:
			return FText::FromString("Wormhole Signature");
		case EAnomalyType::RadiationZone:
			return FText::FromString("Radiation Zone");
		case EAnomalyType::QuantumFluctuation:
			return FText::FromString("Quantum Fluctuation");
		case EAnomalyType::DarkMatterConcentration:
			return FText::FromString("Dark Matter Concentration");
		case EAnomalyType::PlasmaStorm:
			return FText::FromString("Plasma Storm");
		case EAnomalyType::SubspaceRift:
			return FText::FromString("Subspace Rift");
		case EAnomalyType::UnknownPhenomenon:
		default:
			return FText::FromString("Unknown Phenomenon");
	}
}
