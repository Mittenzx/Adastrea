#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/AchievementDataAsset.h"
#include "AchievementManagerSubsystem.generated.h"

/**
 * Achievement tracking entry
 */
USTRUCT(BlueprintType)
struct FAchievementTracker
{
	GENERATED_BODY()

	/** Achievement being tracked */
	UPROPERTY(BlueprintReadOnly, Category="Achievement")
	UAchievementDataAsset* Achievement;

	/** Current progress */
	UPROPERTY(BlueprintReadWrite, Category="Achievement")
	FAchievementProgress Progress;

	FAchievementTracker()
		: Achievement(nullptr)
	{}

	FAchievementTracker(UAchievementDataAsset* InAchievement)
		: Achievement(InAchievement)
	{
		if (InAchievement)
		{
			Progress = InAchievement->ProgressData;
		}
	}
};

/**
 * Achievement Manager Subsystem
 * 
 * Manages achievement tracking, progression, and rewards across the game.
 * Operates as a Game Instance Subsystem to persist across level transitions.
 * 
 * Usage:
 * - Access via UGameInstance::GetSubsystem<UAchievementManagerSubsystem>()
 * - Register achievements on startup via RegisterAchievement()
 * - Track progress via UpdateAchievementProgress()
 * - Award achievements via AwardAchievement()
 * - Query achievement status for UI and gameplay
 * 
 * Example:
 * - Register achievement: RegisterAchievement(AchievementDataAsset)
 * - Update progress: UpdateAchievementProgress("Ships_Destroyed", 1)
 * - Check if completed: IsAchievementCompleted("FirstBlood")
 * - Get all achievements: GetAchievementsByCategory(EAchievementCategory::Combat)
 * 
 * Integration:
 * - Combat system updates kill/damage stats
 * - Trading system updates trade stats
 * - Quest system updates quest stats
 * - Faction system updates reputation stats
 * - Player progression awards XP from achievements
 */
UCLASS()
class ADASTREA_API UAchievementManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAchievementManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// Core Properties
	// ====================

	/** All registered achievements */
	UPROPERTY()
	TArray<FAchievementTracker> RegisteredAchievements;

	/** Completed achievement IDs */
	UPROPERTY()
	TArray<FName> CompletedAchievements;

	/** Achievement statistics (stat name -> value) */
	UPROPERTY()
	TMap<FName, int32> AchievementStats;

	// ====================
	// Achievement Management
	// ====================

	/**
	 * Register an achievement for tracking
	 * @param Achievement Achievement data asset to register
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void RegisterAchievement(UAchievementDataAsset* Achievement);

	/**
	 * Register multiple achievements at once
	 * @param Achievements Array of achievement data assets
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void RegisterAchievements(const TArray<UAchievementDataAsset*>& Achievements);

	/**
	 * Update achievement progress by ID
	 * @param AchievementID Achievement to update
	 * @param ProgressIncrement Amount to add to progress
	 * @param bAutoAward Whether to auto-award if target reached
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void UpdateAchievementProgress(FName AchievementID, int32 ProgressIncrement = 1, bool bAutoAward = true);

	/**
	 * Update achievement progress by stat name
	 * @param StatName Stat name to update
	 * @param StatIncrement Amount to add to stat
	 * @param bAutoAwardAchievements Whether to auto-award achievements tied to this stat
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void UpdateAchievementStat(FName StatName, int32 StatIncrement = 1, bool bAutoAwardAchievements = true);

	/**
	 * Award an achievement (mark as completed)
	 * @param AchievementID Achievement to award
	 * @param bGrantRewards Whether to grant rewards immediately
	 * @return True if successfully awarded
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	bool AwardAchievement(FName AchievementID, bool bGrantRewards = true);

	/**
	 * Reset achievement progress (for testing or prestige systems)
	 * @param AchievementID Achievement to reset
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void ResetAchievement(FName AchievementID);

	/**
	 * Reset all achievements
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement")
	void ResetAllAchievements();

	// ====================
	// Query Functions
	// ====================

	/**
	 * Check if achievement is completed
	 * @param AchievementID Achievement to check
	 * @return True if completed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	bool IsAchievementCompleted(FName AchievementID) const;

	/**
	 * Get achievement progress
	 * @param AchievementID Achievement to check
	 * @param OutProgress Output progress data
	 * @return True if achievement exists
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement|Query")
	bool GetAchievementProgress(FName AchievementID, FAchievementProgress& OutProgress) const;

	/**
	 * Get all achievements by category
	 * @param Category Category to filter
	 * @return Array of achievement trackers
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	TArray<FAchievementTracker> GetAchievementsByCategory(EAchievementCategory Category) const;

	/**
	 * Get all achievements by rarity
	 * @param Rarity Rarity to filter
	 * @return Array of achievement trackers
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	TArray<FAchievementTracker> GetAchievementsByRarity(EAchievementRarity Rarity) const;

	/**
	 * Get all completed achievements
	 * @return Array of completed achievement trackers
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	TArray<FAchievementTracker> GetCompletedAchievements() const;

	/**
	 * Get all in-progress achievements
	 * @return Array of in-progress achievement trackers
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	TArray<FAchievementTracker> GetInProgressAchievements() const;

	/**
	 * Get achievement completion percentage
	 * @return Completion as 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	float GetOverallCompletionPercentage() const;

	/**
	 * Get total achievement points earned
	 * @return Total points based on rarity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	int32 GetTotalAchievementPoints() const;

	/**
	 * Get stat value
	 * @param StatName Stat to query
	 * @return Current stat value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievement|Query")
	int32 GetStatValue(FName StatName) const;

	/**
	 * Get recently completed achievements
	 * @param MaxCount Maximum number to return
	 * @return Array of recently completed achievements
	 */
	UFUNCTION(BlueprintCallable, Category="Achievement|Query")
	TArray<FAchievementTracker> GetRecentlyCompletedAchievements(int32 MaxCount = 5) const;

	// ====================
	// Events
	// ====================

	/** Event fired when achievement is unlocked */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, FName, AchievementID, UAchievementDataAsset*, Achievement);
	UPROPERTY(BlueprintAssignable, Category="Achievement|Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	/** Event fired when achievement progress updates */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAchievementProgressUpdated, FName, AchievementID, int32, CurrentProgress, int32, TargetProgress);
	UPROPERTY(BlueprintAssignable, Category="Achievement|Events")
	FOnAchievementProgressUpdated OnAchievementProgressUpdated;

	/** Event fired when stat is updated */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUpdated, FName, StatName, int32, NewValue);
	UPROPERTY(BlueprintAssignable, Category="Achievement|Events")
	FOnStatUpdated OnStatUpdated;

protected:
	/**
	 * Find achievement tracker by ID
	 * @param AchievementID ID to find
	 * @return Pointer to tracker, or nullptr
	 */
	FAchievementTracker* FindAchievementTracker(FName AchievementID);

	/**
	 * Find achievement tracker (const version)
	 * @param AchievementID ID to find
	 * @return Pointer to tracker, or nullptr
	 */
	const FAchievementTracker* FindAchievementTracker(FName AchievementID) const;

	/**
	 * Grant achievement rewards
	 * @param Achievement Achievement to grant rewards from
	 */
	void GrantAchievementRewards(UAchievementDataAsset* Achievement);

	/**
	 * Check and award stat-based achievements
	 * @param StatName Stat that was updated
	 */
	void CheckStatBasedAchievements(FName StatName);

	/**
	 * Calculate achievement point value based on rarity
	 * @param Rarity Achievement rarity
	 * @return Point value
	 */
	int32 GetPointsForRarity(EAchievementRarity Rarity) const;
};
