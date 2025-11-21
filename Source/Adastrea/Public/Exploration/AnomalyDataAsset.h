#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnomalyDataAsset.generated.h"

/**
 * Anomaly Type
 * Defines the type of spatial anomaly
 */
UENUM(BlueprintType)
enum class EAnomalyType : uint8
{
	GravitationalAnomaly	UMETA(DisplayName = "Gravitational Anomaly"),
	TemporalDistortion		UMETA(DisplayName = "Temporal Distortion"),
	EnergyField				UMETA(DisplayName = "Energy Field"),
	WormholeSignature		UMETA(DisplayName = "Wormhole Signature"),
	RadiationZone			UMETA(DisplayName = "Radiation Zone"),
	QuantumFluctuation		UMETA(DisplayName = "Quantum Fluctuation"),
	DarkMatterConcentration	UMETA(DisplayName = "Dark Matter Concentration"),
	PlasmaStorm				UMETA(DisplayName = "Plasma Storm"),
	SubspaceRift			UMETA(DisplayName = "Subspace Rift"),
	UnknownPhenomenon		UMETA(DisplayName = "Unknown Phenomenon")
};

/**
 * Rarity Tier
 * How rare and valuable a discovery is
 */
UENUM(BlueprintType)
enum class ERarityTier : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	VeryRare	UMETA(DisplayName = "Very Rare"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * Scanner Type Requirement
 * Type of scanner required to detect
 */
UENUM(BlueprintType)
enum class EScannerTypeRequirement : uint8
{
	Any			UMETA(DisplayName = "Any Scanner"),
	Basic		UMETA(DisplayName = "Basic Scanner"),
	Advanced	UMETA(DisplayName = "Advanced Scanner"),
	Military	UMETA(DisplayName = "Military Scanner"),
	Scientific	UMETA(DisplayName = "Scientific Scanner"),
	Exploration	UMETA(DisplayName = "Exploration Scanner")
};

/**
 * Item Reward
 * Structure for item rewards
 */
USTRUCT(BlueprintType)
struct FItemReward
{
	GENERATED_BODY()

	/** Item identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Reward")
	FName ItemID;

	/** Quantity of items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Reward", meta=(ClampMin=1))
	int32 Quantity;

	/** Probability of receiving this item (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Reward", meta=(ClampMin=0, ClampMax=100))
	int32 DropChance;

	FItemReward()
		: ItemID(NAME_None)
		, Quantity(1)
		, DropChance(100)
	{
	}
};

/**
 * Anomaly Data Asset
 * 
 * Defines configuration for spatial anomalies and phenomena including:
 * - Detection requirements and signal characteristics
 * - Visual and audio presentation
 * - Interaction mechanics (harvest, avoid, etc.)
 * - Reward structures (credits, experience, items)
 * - Rarity and discovery value
 * 
 * **Usage:**
 * 1. Create a Blueprint Data Asset based on this class
 * 2. Configure anomaly properties and behavior
 * 3. Set detection requirements and rarity
 * 4. Define rewards for discovery/interaction
 * 5. Place in world or spawn procedurally
 * 
 * **Anomaly Types:**
 * - Gravitational: Mass concentrations, singularities
 * - Temporal: Time dilation zones, temporal echoes
 * - Energy: High-energy plasma, exotic radiation
 * - Wormhole: Stable/unstable wormhole signatures
 * - Quantum: Quantum foam, uncertainty fields
 * 
 * **Integration:**
 * - Works with ScannerComponent for detection
 * - DiscoveryComponent handles discovery tracking
 * - Can trigger quests or story events
 */
UCLASS(BlueprintType)
class ADASTREA_API UAnomalyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAnomalyDataAsset();

	// ====================
	// BASIC INFO
	// ====================

	/** Display name of this anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText AnomalyName;

	/** Description of the anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Type of anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EAnomalyType AnomalyType;

	/** Unique identifier for this anomaly type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FName AnomalyID;

	// ====================
	// DETECTION
	// ====================

	/** Signature strength (0-100, higher = easier to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection", meta=(ClampMin=0, ClampMax=100))
	int32 SignalStrength;

	/** Required scanner type to detect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	EScannerTypeRequirement RequiredScannerType;

	/** Does this anomaly require a deep scan to fully analyze? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	bool bRequiresDeepScan;

	/** Minimum scan detail level required for discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	EScanDetailLevel MinimumScanLevel;

	/** Detection range multiplier (1.0 = normal, >1.0 = easier to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection", meta=(ClampMin=0.1, ClampMax=5.0))
	float DetectionRangeMultiplier;

	// ====================
	// RARITY & VALUE
	// ====================

	/** How rare this anomaly is */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity")
	ERarityTier RarityTier;

	/** Discovery value (used for quest rewards, bounties, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity", meta=(ClampMin=0))
	int32 DiscoveryValue;

	/** Scientific significance (affects research rewards) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity", meta=(ClampMin=0, ClampMax=100))
	int32 ScientificSignificance;

	// ====================
	// REWARDS
	// ====================

	/** Credit reward for discovering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 CreditsReward;

	/** Experience points for discovering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 ExperienceReward;

	/** Item rewards for interacting/harvesting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	TArray<FItemReward> ItemRewards;

	/** Reputation gain with scientific organizations/factions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 ReputationReward;

	// ====================
	// VISUAL & AUDIO
	// ====================

	/** Particle effect for the anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class UParticleSystem* ParticleEffect;

	/** Niagara effect for the anomaly (preferred) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class UNiagaraSystem* NiagaraEffect;

	/** Sound effect when detecting/approaching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class USoundBase* SoundEffect;

	/** Ambient sound when near anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class USoundBase* AmbientSound;

	/** Color tint for the anomaly effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	FLinearColor EffectColor;

	/** Scale for visual effects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation", meta=(ClampMin=0.1, ClampMax=10.0))
	float EffectScale;

	// ====================
	// INTERACTION
	// ====================

	/** Can this anomaly be harvested for resources? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bCanBeHarvested;

	/** Time required to harvest (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=1, ClampMax=300, EditCondition="bCanBeHarvested"))
	float HarvestTime;

	/** Is this anomaly hostile/dangerous? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bIsHostile;

	/** Damage per second when inside anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=0, ClampMax=1000, EditCondition="bIsHostile"))
	float DamagePerSecond;

	/** Interaction radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=100, ClampMax=1000000))
	float InteractionRadius;

	/** Energy cost to harvest/interact */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=0))
	int32 InteractionEnergyCost;

	/** Can this anomaly be interacted with multiple times? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bIsReusable;

	/** Cooldown between interactions (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=0, EditCondition="bIsReusable"))
	float InteractionCooldown;

	// ====================
	// LORE & NARRATIVE
	// ====================

	/** Scientific explanation of the anomaly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore", meta=(MultiLine=true))
	FText ScientificData;

	/** Lore/story information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore", meta=(MultiLine=true))
	FText LoreText;

	/** Associated quest (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
	FName AssociatedQuestID;

	// ====================
	// BLUEPRINT FUNCTIONS
	// ====================

	/**
	 * Calculate total reward value for this anomaly
	 * @param bIncludeItems Include estimated item value
	 * @return Total reward value in credits
	 */
	UFUNCTION(BlueprintPure, Category="Anomaly")
	int32 GetTotalRewardValue(bool bIncludeItems = true) const;

	/**
	 * Get rarity multiplier for rewards
	 * @return Multiplier based on rarity (1.0 = common, 5.0 = legendary)
	 */
	UFUNCTION(BlueprintPure, Category="Anomaly")
	float GetRarityMultiplier() const;

	/**
	 * Check if player can interact with this anomaly
	 * @param PlayerEnergy Player's available energy
	 * @param OutReason Reason why interaction is not possible
	 * @return True if can interact
	 */
	UFUNCTION(BlueprintCallable, Category="Anomaly")
	bool CanInteract(int32 PlayerEnergy, FText& OutReason) const;

	/**
	 * Get display color for rarity tier
	 * @return Color representing rarity
	 */
	UFUNCTION(BlueprintPure, Category="Anomaly")
	FLinearColor GetRarityColor() const;

	/**
	 * Get localized rarity name
	 * @return Display name for rarity tier
	 */
	UFUNCTION(BlueprintPure, Category="Anomaly")
	FText GetRarityDisplayName() const;

	/**
	 * Roll item rewards based on drop chances
	 * @param OutItems Array to fill with rewarded items
	 * @return Number of items awarded
	 */
	UFUNCTION(BlueprintCallable, Category="Anomaly|Rewards")
	int32 RollItemRewards(TArray<FItemReward>& OutItems) const;

	/**
	 * Get anomaly type display name
	 * @param Type Anomaly type to get name for
	 * @return Localized type name
	 */
	UFUNCTION(BlueprintPure, Category="Anomaly|Utility")
	static FText GetAnomalyTypeDisplayName(EAnomalyType Type);
};
