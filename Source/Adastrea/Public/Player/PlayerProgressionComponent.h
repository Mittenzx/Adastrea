#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerProgressionComponent.generated.h"

/**
 * Skill category types for player progression
 */
UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
	Combat      UMETA(DisplayName = "Combat"),       // Weapons, targeting, damage
	Piloting    UMETA(DisplayName = "Piloting"),     // Ship handling, speed
	Engineering UMETA(DisplayName = "Engineering"),  // Repairs, efficiency
	Trading     UMETA(DisplayName = "Trading"),      // Better prices, cargo
	Exploration UMETA(DisplayName = "Exploration"),  // Scanning, discovery
	Diplomacy   UMETA(DisplayName = "Diplomacy"),    // Faction relations
	Leadership  UMETA(DisplayName = "Leadership")    // Crew management
};

/**
 * Individual skill data
 */
USTRUCT(BlueprintType)
struct FPlayerSkill
{
	GENERATED_BODY()

	/** Skill category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	ESkillCategory Category;

	/** Current rank (0-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill", meta=(ClampMin="0", ClampMax="10"))
	int32 Rank;

	/** XP progress to next rank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill", meta=(ClampMin="0"))
	int32 CurrentXP;

	/** XP required for next rank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill", meta=(ClampMin="0"))
	int32 RequiredXP;

	FPlayerSkill()
		: Category(ESkillCategory::Combat)
		, Rank(0)
		, CurrentXP(0)
		, RequiredXP(1000)
	{}

	FPlayerSkill(ESkillCategory InCategory)
		: Category(InCategory)
		, Rank(0)
		, CurrentXP(0)
		, RequiredXP(1000)
	{}
};

/**
 * Player progression component managing experience, levels, and skills.
 * Tracks player advancement through gameplay and provides stat bonuses.
 * 
 * Usage:
 * - Attach to player pawn or controller
 * - Award XP through quest completion, combat, trading, exploration
 * - Level up provides skill points to invest in skill trees
 * - Skills provide gameplay bonuses
 * 
 * Example:
 * - Complete quest: AwardExperience(500)
 * - Player levels up: OnLevelUp event fires
 * - Spend skill points: InvestSkillPoints(ESkillCategory::Combat, 1)
 * - Get bonus: GetSkillBonus(ESkillCategory::Combat) returns 1.15 (15% bonus)
 * 
 * Integration:
 * - Quest system awards XP via quest rewards
 * - Combat system awards XP for kills
 * - Trading system awards XP for profitable trades
 * - Achievement system can award bonus XP
 */
UCLASS(BlueprintType, ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerProgressionComponent();

	// ====================
	// Core Properties
	// ====================

	/** Current player level (1-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progression", meta=(ClampMin="1", ClampMax="100"))
	int32 PlayerLevel;

	/** Current experience points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progression", meta=(ClampMin="0"))
	int32 CurrentXP;

	/** XP required for next level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progression", meta=(ClampMin="0"))
	int32 XPToNextLevel;

	/** Total XP earned lifetime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
	int32 TotalXPEarned;

	/** Unspent skill points available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progression", meta=(ClampMin="0"))
	int32 AvailableSkillPoints;

	/** Player skills by category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progression")
	TArray<FPlayerSkill> Skills;

	// ====================
	// Configuration
	// ====================

	/** Base XP required for level 2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Configuration", meta=(ClampMin="100"))
	int32 BaseXPRequirement;

	/** XP scaling factor per level (e.g., 1.15 = 15% more XP per level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Configuration", meta=(ClampMin="1.0", ClampMax="2.0"))
	float XPScalingFactor;

	/** Skill points awarded per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Configuration", meta=(ClampMin="1"))
	int32 SkillPointsPerLevel;

	/** Maximum player level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Configuration", meta=(ClampMin="1", ClampMax="200"))
	int32 MaxLevel;

	// ====================
	// Experience Management
	// ====================

	/**
	 * Award experience points to the player
	 * @param XPAmount Amount of XP to award
	 * @param bShowNotification Whether to show UI notification
	 */
	UFUNCTION(BlueprintCallable, Category="Progression|Experience")
	void AwardExperience(int32 XPAmount, bool bShowNotification = true);

	/**
	 * Check if player has enough XP to level up
	 * @return True if player can level up
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Experience")
	bool CanLevelUp() const;

	/**
	 * Level up the player (if possible)
	 * @return True if level up was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Progression|Experience")
	bool LevelUp();

	/**
	 * Calculate XP required for a specific level
	 * @param Level The target level
	 * @return XP required to reach that level
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Experience")
	int32 CalculateXPForLevel(int32 Level) const;

	/**
	 * Get percentage progress to next level
	 * @return Progress as 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Experience")
	float GetLevelProgress() const;

	// ====================
	// Skill Management
	// ====================

	/**
	 * Invest skill points in a category
	 * @param Category Skill category to invest in
	 * @param Points Number of points to invest
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category="Progression|Skills")
	bool InvestSkillPoints(ESkillCategory Category, int32 Points = 1);

	/**
	 * Get current rank in a skill category
	 * @param Category Skill category to check
	 * @return Current rank (0-10)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Skills")
	int32 GetSkillRank(ESkillCategory Category) const;

	/**
	 * Get skill bonus multiplier for a category
	 * @param Category Skill category to check
	 * @return Bonus multiplier (1.0 = no bonus, 1.5 = 50% bonus)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Skills")
	float GetSkillBonus(ESkillCategory Category) const;

	/**
	 * Check if can invest in skill (has points and not maxed)
	 * @param Category Skill category to check
	 * @return True if can invest
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Skills")
	bool CanInvestInSkill(ESkillCategory Category) const;

	/**
	 * Reset all skills and refund points
	 * @param bRefundPoints Whether to refund skill points
	 */
	UFUNCTION(BlueprintCallable, Category="Progression|Skills")
	void ResetSkills(bool bRefundPoints = true);

	// ====================
	// Query Functions
	// ====================

	/**
	 * Get all skills data
	 * @return Array of skill data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Query")
	TArray<FPlayerSkill> GetAllSkills() const { return Skills; }

	/**
	 * Check if player meets level requirement
	 * @param RequiredLevel Minimum level needed
	 * @return True if player level >= required level
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Query")
	bool MeetsLevelRequirement(int32 RequiredLevel) const;

	/**
	 * Get total skill points invested
	 * @return Total points spent
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Progression|Query")
	int32 GetTotalSkillPoints() const;

	// ====================
	// Events
	// ====================

	/** Event fired when player levels up */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, SkillPointsGained);
	UPROPERTY(BlueprintAssignable, Category="Progression|Events")
	FOnLevelUp OnLevelUp;

	/** Event fired when XP is awarded */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceAwarded, int32, XPAmount, int32, TotalXP);
	UPROPERTY(BlueprintAssignable, Category="Progression|Events")
	FOnExperienceAwarded OnExperienceAwarded;

	/** Event fired when skill is invested */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillInvested, ESkillCategory, Category, int32, NewRank, int32, PointsRemaining);
	UPROPERTY(BlueprintAssignable, Category="Progression|Events")
	FOnSkillInvested OnSkillInvested;

protected:
	virtual void BeginPlay() override;

	/**
	 * Initialize skills array with all categories
	 */
	void InitializeSkills();

	/**
	 * Find or create skill entry for category
	 * @param Category Skill category to find
	 * @return Pointer to skill data
	 */
	FPlayerSkill* FindOrCreateSkill(ESkillCategory Category);

	/**
	 * Find skill entry (const version)
	 * @param Category Skill category to find
	 * @return Pointer to skill data, or nullptr
	 */
	const FPlayerSkill* FindSkill(ESkillCategory Category) const;

	/**
	 * Calculate skill bonus from rank
	 * @param Rank Current skill rank
	 * @return Bonus multiplier
	 */
	float CalculateSkillBonus(int32 Rank) const;
};
