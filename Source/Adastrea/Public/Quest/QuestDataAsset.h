#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Way/Way.h"
#include "QuestDataAsset.generated.h"

/**
 * Enum for quest types
 */
UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Delivery        UMETA(DisplayName = "Delivery"),        // Transport cargo/passengers
    Exploration     UMETA(DisplayName = "Exploration"),     // Discover locations
    Combat          UMETA(DisplayName = "Combat"),          // Destroy targets
    Escort          UMETA(DisplayName = "Escort"),          // Protect ship/convoy
    Mining          UMETA(DisplayName = "Mining"),          // Collect resources
    Research        UMETA(DisplayName = "Research"),        // Scan/analyze objects
    Diplomatic      UMETA(DisplayName = "Diplomatic"),      // Negotiate/meet NPCs
    Rescue          UMETA(DisplayName = "Rescue"),          // Save ships/personnel
    Sabotage        UMETA(DisplayName = "Sabotage"),        // Infiltrate/disable
    Custom          UMETA(DisplayName = "Custom")           // Custom objectives
};

/**
 * Enum for quest status
 */
UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),     // Quest available but not accepted
    Active          UMETA(DisplayName = "Active"),          // Quest in progress
    Completed       UMETA(DisplayName = "Completed"),       // Quest finished successfully
    Failed          UMETA(DisplayName = "Failed"),          // Quest failed
    Abandoned       UMETA(DisplayName = "Abandoned")        // Player abandoned quest
};

/**
 * Enum for objective types
 */
UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),  // Go to specific location
    DeliverItem     UMETA(DisplayName = "Deliver Item"),    // Deliver cargo/item
    DestroyTarget   UMETA(DisplayName = "Destroy Target"),  // Destroy enemies
    CollectItems    UMETA(DisplayName = "Collect Items"),   // Gather resources
    ScanObject      UMETA(DisplayName = "Scan Object"),     // Scan target
    TalkToNPC       UMETA(DisplayName = "Talk to NPC"),     // Interact with NPC
    ProtectTarget   UMETA(DisplayName = "Protect Target"),  // Keep target alive
    SurviveTime     UMETA(DisplayName = "Survive Time"),    // Survive for duration
    Custom          UMETA(DisplayName = "Custom")           // Custom logic
};

/**
 * Structure defining a quest objective
 */
USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    /** Objective type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
    EObjectiveType Type;

    /** Description of objective */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta=(MultiLine=true))
    FText Description;

    /** Target location (for location-based objectives) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
    FVector TargetLocation;

    /** Target actor class (for interaction objectives) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
    TSubclassOf<AActor> TargetActorClass;

    /** Item/resource ID (for delivery/collection objectives) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
    FName ItemID;

    /** Required quantity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta=(ClampMin="1"))
    int32 RequiredQuantity;

    /** Current progress */
    UPROPERTY(BlueprintReadOnly, Category="Objective")
    int32 CurrentProgress;

    /** Whether objective is optional */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective")
    bool bIsOptional;

    /** Whether objective is completed */
    UPROPERTY(BlueprintReadOnly, Category="Objective")
    bool bIsCompleted;

    /** Distance threshold for location objectives (0 = use default 500m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective", meta=(ClampMin="0", ClampMax="10000"))
    float LocationThreshold;

    FQuestObjective()
        : Type(EObjectiveType::ReachLocation)
        , Description(FText::GetEmpty())
        , TargetLocation(FVector::ZeroVector)
        , TargetActorClass(nullptr)
        , ItemID(NAME_None)
        , RequiredQuantity(1)
        , CurrentProgress(0)
        , bIsOptional(false)
        , bIsCompleted(false)
        , LocationThreshold(0.0f)
    {}
};

/**
 * Structure defining quest rewards
 */
USTRUCT(BlueprintType)
struct FQuestReward
{
    GENERATED_BODY()

    /** Credits reward */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward", meta=(ClampMin="0"))
    int32 Credits;

    /** Reputation gain with Way */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward")
    FName WayID;

    /** Reputation amount to gain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward", meta=(ClampMin="-100", ClampMax="100"))
    int32 ReputationGain;

    /** Item rewards (item ID and quantity) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward")
    TMap<FName, int32> ItemRewards;

    /** Experience points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward", meta=(ClampMin="0"))
    int32 ExperiencePoints;

    FQuestReward()
        : Credits(0)
        , WayID(NAME_None)
        , ReputationGain(0)
        , ExperiencePoints(0)
    {}
};

/**
 * Structure for quest prerequisites
 */
USTRUCT(BlueprintType)
struct FQuestPrerequisite
{
    GENERATED_BODY()

    /** Required quest to complete first */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Prerequisite")
    class UQuestDataAsset* RequiredQuest;

    /** Minimum reputation with Way */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Prerequisite")
    FName RequiredWayID;

    /** Minimum reputation level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Prerequisite", meta=(ClampMin="-100", ClampMax="100"))
    int32 MinimumReputation;

    /** Minimum player level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Prerequisite", meta=(ClampMin="1"))
    int32 MinimumPlayerLevel;

    FQuestPrerequisite()
        : RequiredQuest(nullptr)
        , RequiredWayID(NAME_None)
        , MinimumReputation(0)
        , MinimumPlayerLevel(1)
    {}
};

/**
 * Data Asset representing a quest/mission
 * 
 * Defines objectives, rewards, prerequisites, and quest flow.
 * Used for both hand-crafted quests and procedurally generated missions.
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on UQuestDataAsset
 * - Configure quest details, objectives, and rewards
 * - Set prerequisites if needed
 * - Reference in QuestManagerSubsystem for quest tracking
 */
UCLASS(BlueprintType)
class ADASTREA_API UQuestDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UQuestDataAsset();

    // ====================
    // Core Identity
    // ====================

    /** Quest name/title */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    FText QuestName;

    /** Quest description */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info", meta=(MultiLine=true))
    FText Description;

    /** Unique identifier for this quest */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    FName QuestID;

    /** Quest type classification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    EQuestType QuestType;

    /** Quest giver Way (who offers this quest) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    FName QuestGiverWayID;

    /** Quest difficulty (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info", meta=(ClampMin="1", ClampMax="10"))
    int32 Difficulty;

    /** Is this a main story quest */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    bool bIsMainQuest;

    /** Is this a repeatable quest */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Info")
    bool bIsRepeatable;

    /** Time limit in seconds (0 = no limit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest Info", meta=(ClampMin="0"))
    float TimeLimit;

    // ====================
    // Prerequisites
    // ====================

    /** Prerequisites that must be met to accept this quest */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prerequisites")
    TArray<FQuestPrerequisite> Prerequisites;

    // ====================
    // Objectives
    // ====================

    /** Quest objectives that must be completed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objectives")
    TArray<FQuestObjective> Objectives;

    // ====================
    // Rewards
    // ====================

    /** Rewards given upon quest completion */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
    FQuestReward Rewards;

    /** Optional rewards for bonus objectives */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
    FQuestReward BonusRewards;

    // ====================
    // Narrative
    // ====================

    /** Text shown when quest is offered */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative", meta=(MultiLine=true))
    FText AcceptText;

    /** Text shown when objectives update */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative", meta=(MultiLine=true))
    FText UpdateText;

    /** Text shown upon quest completion */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative", meta=(MultiLine=true))
    FText CompletionText;

    /** Text shown upon quest failure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative", meta=(MultiLine=true))
    FText FailureText;

    // ====================
    // Quest Chain
    // ====================

    /** Next quest in chain (unlocked upon completion) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Chain")
    UQuestDataAsset* NextQuestInChain;

    /** Previous quest in chain */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Chain")
    UQuestDataAsset* PreviousQuestInChain;

    // ====================
    // Query Functions
    // ====================

    /**
     * Get all objectives for this quest
     * @return Array of objectives
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    TArray<FQuestObjective> GetObjectives() const;

    /**
     * Get number of completed objectives
     * @return Count of completed objectives
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    int32 GetCompletedObjectiveCount() const;

    /**
     * Get total number of objectives
     * @return Total objective count
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    int32 GetTotalObjectiveCount() const;

    /**
     * Get quest completion percentage (0-100)
     * @return Completion percentage
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    float GetCompletionPercentage() const;

    /**
     * Check if all required objectives are complete
     * @return True if quest can be turned in
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    bool AreAllRequiredObjectivesComplete() const;

    /**
     * Check if prerequisites are met for a player
     * @return True if player meets prerequisites
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    bool CheckPrerequisites() const;

    /**
     * Get total reward value (for display purposes)
     * @return Estimated credit value of all rewards
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    int32 GetTotalRewardValue() const;

    /**
     * Get quest difficulty as text
     * @return Localized difficulty name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest")
    FText GetDifficultyText() const;

    /**
     * Get display name for quest type
     * @param Type Quest type to get name for
     * @return Localized type name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest|Utility")
    static FText GetQuestTypeDisplayName(EQuestType Type);

    /**
     * Get display name for objective type
     * @param Type Objective type to get name for
     * @return Localized type name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest|Utility")
    static FText GetObjectiveTypeDisplayName(EObjectiveType Type);

#if WITH_EDITOR
    /**
     * Validate quest data asset properties
     * Checks for required fields and logical constraints
     */
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
