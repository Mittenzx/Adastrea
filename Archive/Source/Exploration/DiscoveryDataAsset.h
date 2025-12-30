#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Exploration/AnomalyDataAsset.h"
#include "Exploration/ScannerDataAsset.h"
#include "DiscoveryDataAsset.generated.h"

/**
 * Discovery Type
 * Categories of discoverable locations and objects
 */
UENUM(BlueprintType)
enum class EDiscoveryType : uint8
{
	ArchaeologicalSite	UMETA(DisplayName = "Archaeological Site"),
	DerelictShip		UMETA(DisplayName = "Derelict Ship"),
	DerelictStation		UMETA(DisplayName = "Derelict Station"),
	AbandonedOutpost	UMETA(DisplayName = "Abandoned Outpost"),
	AncientRuins		UMETA(DisplayName = "Ancient Ruins"),
	CrashedVessel		UMETA(DisplayName = "Crashed Vessel"),
	HiddenCache			UMETA(DisplayName = "Hidden Cache"),
	SecretFacility		UMETA(DisplayName = "Secret Facility"),
	AlienArtifact		UMETA(DisplayName = "Alien Artifact"),
	MysteryLocation		UMETA(DisplayName = "Mystery Location")
};

/**
 * Story Item
 * Special items with narrative significance
 */
USTRUCT(BlueprintType)
struct FStoryItem
{
	GENERATED_BODY()

	/** Item identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story Item")
	FName ItemID;

	/** Item name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story Item")
	FText ItemName;

	/** Item description/lore */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story Item", meta=(MultiLine=true))
	FText ItemDescription;

	/** Is this a key item? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story Item")
	bool bIsKeyItem;

	/** Associated quest (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story Item")
	FName AssociatedQuestID;

	FStoryItem()
		: ItemID(NAME_None)
		, bIsKeyItem(false)
		, AssociatedQuestID(NAME_None)
	{
	}
};

/**
 * Discovery Data Asset
 * 
 * Defines configuration for discoverable locations, derelicts, and archaeological sites including:
 * - Location type and characteristics
 * - Detection requirements
 * - Exploration and interaction mechanics
 * - Rewards and lore content
 * - Quest integration
 * 
 * **Usage:**
 * 1. Create a Blueprint Data Asset based on this class
 * 2. Configure discovery properties and content
 * 3. Set detection requirements and rarity
 * 4. Define lore text and story items
 * 5. Place in world or reference in procedural generation
 * 
 * **Discovery Types:**
 * - Archaeological Sites: Ancient civilizations, historical ruins
 * - Derelict Ships: Abandoned vessels, ghost ships
 * - Derelict Stations: Abandoned orbital facilities
 * - Abandoned Outposts: Remote settlements, mining camps
 * - Hidden Caches: Secret stashes, smuggler hideouts
 * 
 * **Integration:**
 * - Works with ScannerComponent for initial detection
 * - DiscoveryComponent handles discovery state
 * - Can spawn NPCs, enemies, or quest triggers
 * - Integrates with quest system
 */
UCLASS(BlueprintType)
class ADASTREA_API UDiscoveryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UDiscoveryDataAsset();

	// ====================
	// BASIC INFO
	// ====================

	/** Display name of this discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText DiscoveryName;

	/** Description of the discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Type of discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EDiscoveryType DiscoveryType;

	/** Unique identifier for this discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FName DiscoveryID;

	// ====================
	// DETECTION
	// ====================

	/** Signature strength (0-100, higher = easier to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection", meta=(ClampMin=0, ClampMax=100))
	int32 SignalStrength;

	/** Required scanner type to detect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	EScannerTypeRequirement RequiredScannerType;

	/** Does this discovery require a deep scan to fully reveal? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	bool bRequiresDeepScan;

	/** Minimum scan detail level required for discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	EScanDetailLevel MinimumScanLevel;

	/** Detection range multiplier (1.0 = normal, >1.0 = easier to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection", meta=(ClampMin=0.1, ClampMax=5.0))
	float DetectionRangeMultiplier;

	/** Is this discovery hidden/camouflaged? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detection")
	bool bIsHidden;

	// ====================
	// RARITY & VALUE
	// ====================

	/** How rare this discovery is */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity")
	ERarityTier RarityTier;

	/** Discovery value (used for quest rewards, exploration score, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity", meta=(ClampMin=0))
	int32 DiscoveryValue;

	/** Historical/scientific significance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rarity", meta=(ClampMin=0, ClampMax=100))
	int32 HistoricalSignificance;

	// ====================
	// REWARDS
	// ====================

	/** Credit reward for discovering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 CreditsReward;

	/** Experience points for discovering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 ExperienceReward;

	/** Item rewards found at this location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	TArray<FItemReward> ItemRewards;

	/** Story items found at this location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	TArray<FStoryItem> StoryItems;

	/** Reputation gain with explorer/archaeologist factions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards", meta=(ClampMin=0))
	int32 ReputationReward;

	// ====================
	// CONTENT
	// ====================

	/** Detailed lore text revealed upon exploration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Content", meta=(MultiLine=true))
	FText LoreText;

	/** Historical records or logs found here */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Content")
	TArray<FText> HistoricalRecords;

	/** Civilization or faction this discovery is associated with */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Content")
	FName AssociatedFactionID;

	/** Age of the discovery (years) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Content", meta=(ClampMin=0))
	int32 EstimatedAge;

	/** Condition of the discovery (0-100, 0=completely destroyed, 100=pristine) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Content", meta=(ClampMin=0, ClampMax=100))
	int32 Condition;

	// ====================
	// QUEST INTEGRATION
	// ====================

	/** Quest triggered when discovered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Integration")
	FName TriggeredQuestID;

	/** Does discovering this unlock new quests? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Integration")
	bool bUnlocksQuests;

	/** Quest IDs unlocked by this discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Integration", meta=(EditCondition="bUnlocksQuests"))
	TArray<FName> UnlockedQuestIDs;

	/** Is this discovery required for a quest? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Integration")
	bool bIsQuestObjective;

	/** Quest ID this is an objective for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Integration", meta=(EditCondition="bIsQuestObjective"))
	FName QuestObjectiveID;

	// ====================
	// INTERACTION
	// ====================

	/** Can this location be explored (entered)? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bCanBeExplored;

	/** Time required to fully explore (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=1, ClampMax=600, EditCondition="bCanBeExplored"))
	float ExplorationTime;

	/** Are there hostiles at this location? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bHasHostiles;

	/** Difficulty of hostiles (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=1, ClampMax=10, EditCondition="bHasHostiles"))
	int32 HostileDifficulty;

	/** Interaction radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=100, ClampMax=1000000))
	float InteractionRadius;

	/** Can this location be salvaged? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bCanBeSalvaged;

	/** Salvage value if location can be salvaged */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin=0, EditCondition="bCanBeSalvaged"))
	int32 SalvageValue;

	/** Can this location be revisited? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bCanRevisit;

	// ====================
	// VISUAL & AUDIO
	// ====================

	/** Mesh or actor class for this discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	TSubclassOf<AActor> DiscoveryActorClass;

	/** Particle effect for the discovery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class UParticleSystem* ParticleEffect;

	/** Niagara effect for the discovery (preferred) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class UNiagaraSystem* NiagaraEffect;

	/** Ambient sound at the location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class USoundBase* AmbientSound;

	/** Discovery notification sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presentation")
	class USoundBase* DiscoverySound;

	// ====================
	// BLUEPRINT FUNCTIONS
	// ====================

	/**
	 * Calculate total reward value for this discovery
	 * @param bIncludeItems Include estimated item value
	 * @param bIncludeSalvage Include salvage value
	 * @return Total reward value in credits
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	int32 GetTotalRewardValue(bool bIncludeItems = true, bool bIncludeSalvage = true) const;

	/**
	 * Get rarity multiplier for rewards
	 * @return Multiplier based on rarity (1.0 = common, 5.0 = legendary)
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	float GetRarityMultiplier() const;

	/**
	 * Check if player can explore this discovery
	 * @param PlayerLevel Player's level
	 * @param OutReason Reason why exploration is not possible
	 * @return True if can explore
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery")
	bool CanExplore(int32 PlayerLevel, FText& OutReason) const;

	/**
	 * Get display color for rarity tier
	 * @return Color representing rarity
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FLinearColor GetRarityColor() const;

	/**
	 * Get localized rarity name
	 * @return Display name for rarity tier
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FText GetRarityDisplayName() const;

	/**
	 * Roll item rewards based on drop chances
	 * @param OutItems Array to fill with rewarded items
	 * @return Number of items awarded
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery|Rewards")
	int32 RollItemRewards(TArray<FItemReward>& OutItems) const;

	/**
	 * Get discovery type display name
	 * @param Type Discovery type to get name for
	 * @return Localized type name
	 */
	UFUNCTION(BlueprintPure, Category="Discovery|Utility")
	static FText GetDiscoveryTypeDisplayName(EDiscoveryType Type);

	/**
	 * Get condition as text
	 * @return Description of condition state
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FText GetConditionText() const;
};
