#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest/QuestDataAsset.h"
#include "QuestManagerSubsystem.generated.h"

/**
 * Structure for tracking active quest state
 */
USTRUCT(BlueprintType)
struct FActiveQuest
{
    GENERATED_BODY()

    /** Quest data asset */
    UPROPERTY(BlueprintReadOnly)
    UQuestDataAsset* Quest;

    /** Current status */
    UPROPERTY(BlueprintReadOnly)
    EQuestStatus Status;

    /** When quest was started */
    UPROPERTY(BlueprintReadOnly)
    FDateTime StartTime;

    /** Time remaining if there's a time limit (seconds) */
    UPROPERTY(BlueprintReadOnly)
    float TimeRemaining;

    /** Runtime objective state (mirrors Quest->Objectives) */
    UPROPERTY(BlueprintReadOnly)
    TArray<FQuestObjective> ObjectiveStates;

    FActiveQuest()
        : Quest(nullptr)
        , Status(EQuestStatus::NotStarted)
        , StartTime(FDateTime::Now())
        , TimeRemaining(0.0f)
    {}
};

/**
 * Subsystem for managing quest state, progression, and tracking
 * 
 * Handles quest acceptance, objective tracking, completion, and rewards.
 * Persists across level loads and integrates with save system.
 * 
 * Usage:
 * - Access via UGameInstance::GetSubsystem<UQuestManagerSubsystem>()
 * - Use Blueprint functions to accept/complete quests
 * - Query for active quests and their status
 * - Subscribe to events for UI updates
 */
UCLASS()
class ADASTREA_API UQuestManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManagerSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ====================
    // QUEST MANAGEMENT
    // ====================

    /**
     * Accept a new quest
     * @param Quest Quest to accept
     * @return True if quest was accepted
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool AcceptQuest(UQuestDataAsset* Quest);

    /**
     * Abandon an active quest
     * @param QuestID ID of quest to abandon
     * @return True if quest was abandoned
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool AbandonQuest(FName QuestID);

    /**
     * Complete a quest and give rewards
     * @param QuestID ID of quest to complete
     * @return True if quest was completed
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool CompleteQuest(FName QuestID);

    /**
     * Fail a quest
     * @param QuestID ID of quest to fail
     * @param Reason Reason for failure
     * @return True if quest was failed
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool FailQuest(FName QuestID, const FString& Reason);

    // ====================
    // OBJECTIVE TRACKING
    // ====================

    /**
     * Update objective progress
     * @param QuestID Quest ID
     * @param ObjectiveIndex Index of objective to update
     * @param NewProgress New progress value
     * @return True if updated successfully
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 NewProgress);

    /**
     * Complete a specific objective
     * @param QuestID Quest ID
     * @param ObjectiveIndex Index of objective to complete
     * @return True if completed successfully
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool CompleteObjective(FName QuestID, int32 ObjectiveIndex);

    /**
     * Notify when player reaches a location (updates relevant objectives)
     * @param Location Location that was reached
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    void NotifyLocationReached(FVector Location);

    /**
     * Notify when player collects an item (updates relevant objectives)
     * @param ItemID Item that was collected
     * @param Quantity Amount collected
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    void NotifyItemCollected(FName ItemID, int32 Quantity = 1);

    /**
     * Notify when player destroys a target (updates relevant objectives)
     * @param TargetClass Class of actor destroyed
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    void NotifyTargetDestroyed(TSubclassOf<AActor> TargetClass);

    // ====================
    // QUERIES
    // ====================

    /**
     * Get all active quests
     * @return Array of active quest states
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest Manager")
    TArray<FActiveQuest> GetActiveQuests() const;

    /**
     * Get quest by ID
     * @param QuestID Quest to find
     * @param OutQuest Quest state if found
     * @return True if quest is active
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    bool GetQuestByID(FName QuestID, FActiveQuest& OutQuest) const;

    /**
     * Check if quest is active
     * @param QuestID Quest to check
     * @return True if quest is in active list
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest Manager")
    bool IsQuestActive(FName QuestID) const;

    /**
     * Check if quest has been completed
     * @param QuestID Quest to check
     * @return True if quest was completed before
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest Manager")
    bool HasCompletedQuest(FName QuestID) const;

    /**
     * Get number of active quests
     * @return Count of active quests
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest Manager")
    int32 GetActiveQuestCount() const;

    /**
     * Get completed quest count
     * @return Total quests completed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Quest Manager")
    int32 GetCompletedQuestCount() const;

    /**
     * Get quests by type
     * @param Type Quest type to filter by
     * @return Array of quests of that type
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    TArray<FActiveQuest> GetQuestsByType(EQuestType Type) const;

    /**
     * Get quests from specific giver
     * @param WayID Way that gives quests
     * @return Array of quests from that Way
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager")
    TArray<FActiveQuest> GetQuestsFromGiver(FName WayID) const;

    // ====================
    // QUEST GENERATION
    // ====================

    /**
     * Generate a random quest based on parameters
     * @param Type Preferred quest type
     * @param Difficulty Difficulty level (1-10)
     * @param QuestGiver Way offering the quest
     * @return Generated quest data asset
     */
    UFUNCTION(BlueprintCallable, Category="Quest Manager|Generation")
    UQuestDataAsset* GenerateRandomQuest(EQuestType Type, int32 Difficulty, FName QuestGiver);

    // ====================
    // EVENTS
    // ====================

    /** Called when quest is accepted */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, UQuestDataAsset*, Quest);
    UPROPERTY(BlueprintAssignable, Category="Quest Manager|Events")
    FOnQuestAccepted OnQuestAccepted;

    /** Called when objective is updated */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, FName, QuestID, int32, ObjectiveIndex, int32, NewProgress);
    UPROPERTY(BlueprintAssignable, Category="Quest Manager|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    /** Called when objective is completed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, QuestID, int32, ObjectiveIndex);
    UPROPERTY(BlueprintAssignable, Category="Quest Manager|Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    /** Called when quest is completed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FName, QuestID);
    UPROPERTY(BlueprintAssignable, Category="Quest Manager|Events")
    FOnQuestCompleted OnQuestCompleted;

    /** Called when quest fails */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestFailed, FName, QuestID, FString, Reason);
    UPROPERTY(BlueprintAssignable, Category="Quest Manager|Events")
    FOnQuestFailed OnQuestFailed;

protected:
    /** Active quest states */
    UPROPERTY()
    TArray<FActiveQuest> ActiveQuests;

    /** Completed quest IDs */
    UPROPERTY()
    TSet<FName> CompletedQuestIDs;

    /** Failed quest IDs */
    UPROPERTY()
    TSet<FName> FailedQuestIDs;

    /** Timer handle for time-limited quests */
    FTimerHandle QuestTimerHandle;

private:
    /** Update time-limited quests */
    void UpdateQuestTimers();

    /** Give rewards to player */
    void GiveQuestRewards(const FQuestReward& Rewards);

    /** Check if all objectives are complete */
    bool CheckQuestCompletion(FActiveQuest& Quest);

    /** Find active quest by ID */
    FActiveQuest* FindActiveQuest(FName QuestID);
};
