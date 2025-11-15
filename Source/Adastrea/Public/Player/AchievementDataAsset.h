#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AchievementDataAsset.generated.h"

/**
 * Achievement category types
 */
UENUM(BlueprintType)
enum class EAchievementCategory : uint8
{
	Combat          UMETA(DisplayName = "Combat"),         // Combat-related achievements
	Trading         UMETA(DisplayName = "Trading"),        // Trading and economy
	Exploration     UMETA(DisplayName = "Exploration"),    // Discovery and scanning
	Diplomacy       UMETA(DisplayName = "Diplomacy"),      // Faction relations
	Construction    UMETA(DisplayName = "Construction"),   // Station building
	Personnel       UMETA(DisplayName = "Personnel"),      // Crew management
	Progression     UMETA(DisplayName = "Progression"),    // Leveling and skills
	Story           UMETA(DisplayName = "Story"),          // Main storyline
	Hidden          UMETA(DisplayName = "Hidden")          // Secret achievements
};

/**
 * Achievement rarity/difficulty
 */
UENUM(BlueprintType)
enum class EAchievementRarity : uint8
{
	Common      UMETA(DisplayName = "Common"),        // Easy to obtain
	Uncommon    UMETA(DisplayName = "Uncommon"),      // Moderate difficulty
	Rare        UMETA(DisplayName = "Rare"),          // Challenging
	Epic        UMETA(DisplayName = "Epic"),          // Very difficult
	Legendary   UMETA(DisplayName = "Legendary")      // Extremely rare
};

/**
 * Achievement reward data
 */
USTRUCT(BlueprintType)
struct FAchievementReward
{
	GENERATED_BODY()

	/** Credits reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward", meta=(ClampMin="0"))
	int32 Credits;

	/** Experience points reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward", meta=(ClampMin="0"))
	int32 ExperiencePoints;

	/** Unlocked content ID (ship, module, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward")
	FName UnlockID;

	/** Item rewards (item ID and quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward")
	TMap<FName, int32> ItemRewards;

	/** Title/badge earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward")
	FText EarnedTitle;

	FAchievementReward()
		: Credits(0)
		, ExperiencePoints(0)
		, UnlockID(NAME_None)
		, EarnedTitle(FText::GetEmpty())
	{}
};

/**
 * Achievement progress tracking data
 */
USTRUCT(BlueprintType)
struct FAchievementProgress
{
	GENERATED_BODY()

	/** Current progress value */
	UPROPERTY(BlueprintReadWrite, Category="Progress")
	int32 CurrentProgress;

	/** Target progress value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progress", meta=(ClampMin="1"))
	int32 TargetProgress;

	/** Whether achievement is completed */
	UPROPERTY(BlueprintReadOnly, Category="Progress")
	bool bIsCompleted;

	/** Timestamp when completed */
	UPROPERTY(BlueprintReadOnly, Category="Progress")
	FDateTime CompletedTimestamp;

	FAchievementProgress()
		: CurrentProgress(0)
		, TargetProgress(1)
		, bIsCompleted(false)
		, CompletedTimestamp(FDateTime::MinValue())
	{}
};

/**
 * Data Asset representing an achievement.
 * Defines achievement criteria, rewards, and progression tracking.
 * 
 * Usage:
 * - Create Blueprint Data Asset based on UAchievementDataAsset
 * - Configure achievement details, category, rarity
 * - Set target progress and rewards
 * - Track via AchievementManagerSubsystem
 * 
 * Example Achievements:
 * - "First Blood": Destroy your first enemy ship (Combat, Common)
 * - "Trade Baron": Complete 100 trade contracts (Trading, Rare)
 * - "Explorer": Discover 50 star systems (Exploration, Uncommon)
 * - "Diplomatic Genius": Reach Revered with all factions (Diplomacy, Legendary)
 * 
 * Integration:
 * - AchievementManagerSubsystem tracks progress
 * - Quest system can track achievement-related objectives
 * - Combat system increments kill counters
 * - Trading system tracks trade metrics
 */
UCLASS(BlueprintType)
class ADASTREA_API UAchievementDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UAchievementDataAsset();

	// ====================
	// Basic Info
	// ====================

	/** Unique achievement ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FName AchievementID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EAchievementCategory Category;

	/** Rarity/Difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EAchievementRarity Rarity;

	/** Whether this is a hidden achievement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	bool bIsHidden;

	/** Icon for achievement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	class UTexture2D* Icon;

	// ====================
	// Progress Tracking
	// ====================

	/** Default progress data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	FAchievementProgress ProgressData;

	/** Tracking stat name (e.g., "Ships_Destroyed", "Credits_Earned") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	FName TrackingStatName;

	/** Whether progress is tracked automatically by stat system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	bool bAutoTrackedByStat;

	// ====================
	// Rewards
	// ====================

	/** Rewards for completing this achievement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	FAchievementReward Rewards;

	// ====================
	// Requirements
	// ====================

	/** Required achievements (must complete these first) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
	TArray<UAchievementDataAsset*> PrerequisiteAchievements;

	/** Minimum player level required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="1"))
	int32 MinimumPlayerLevel;

	// ====================
	// Blueprint Functions
	// ====================

	/**
	 * Get progress percentage (0.0 to 1.0)
	 * @return Progress as percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement")
	float GetProgressPercentage() const;

	/**
	 * Get progress display text
	 * @return Text showing "CurrentProgress / TargetProgress"
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement")
	FText GetProgressText() const;

	/**
	 * Get rarity display name
	 * @return Localized rarity name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement")
	FText GetRarityDisplayName() const;

	/**
	 * Get category display name
	 * @return Localized category name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement")
	FText GetCategoryDisplayName() const;
};
