#include "Quest/QuestManagerSubsystem.h"
#include "AdastreaLog.h"
#include "TimerManager.h"
#include "Engine/World.h"

UQuestManagerSubsystem::UQuestManagerSubsystem()
{
    // Default quest generation configuration
    CreditsPerDifficulty = 1000;
    ExperiencePerDifficulty = 100;
    ReputationPerDifficulty = 5;
    DefaultLocationThreshold = 500.0f;
}

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Initialized"));
}

void UQuestManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();
    
    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Deinitialized"));
}

// ====================
// QUEST MANAGEMENT
// ====================

bool UQuestManagerSubsystem::AcceptQuest(UQuestDataAsset* Quest)
{
    if (!Quest)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Cannot accept null quest"));
        return false;
    }

    // Check if already active
    if (IsQuestActive(Quest->QuestID))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Quest %s is already active"), *Quest->QuestID.ToString());
        return false;
    }

    // Check prerequisites
    if (!Quest->CheckPrerequisites())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Prerequisites not met for quest %s"), *Quest->QuestName.ToString());
        return false;
    }

    // Create active quest entry
    FActiveQuest ActiveQuest;
    ActiveQuest.Quest = Quest;
    ActiveQuest.Status = EQuestStatus::Active;
    ActiveQuest.StartTime = FDateTime::Now();
    ActiveQuest.TimeRemaining = Quest->TimeLimit;
    ActiveQuest.ObjectiveStates = Quest->Objectives; // Copy objectives for runtime tracking

    ActiveQuests.Add(ActiveQuest);

    OnQuestAccepted.Broadcast(Quest);

    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Accepted quest '%s'"), *Quest->QuestName.ToString());

    return true;
}

bool UQuestManagerSubsystem::AbandonQuest(FName QuestID)
{
    FActiveQuest* Quest = FindActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Cannot abandon quest %s - not active"), *QuestID.ToString());
        return false;
    }

    Quest->Status = EQuestStatus::Abandoned;
    ActiveQuests.RemoveAll([QuestID](const FActiveQuest& Q) { return Q.Quest && Q.Quest->QuestID == QuestID; });

    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Abandoned quest %s"), *QuestID.ToString());

    return true;
}

bool UQuestManagerSubsystem::CompleteQuest(FName QuestID)
{
    FActiveQuest* Quest = FindActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Cannot complete quest %s - not active"), *QuestID.ToString());
        return false;
    }

    if (!Quest->Quest->AreAllRequiredObjectivesComplete())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Cannot complete quest %s - objectives not complete"), *QuestID.ToString());
        return false;
    }

    Quest->Status = EQuestStatus::Completed;
    CompletedQuestIDs.Add(QuestID);

    // Give rewards
    GiveQuestRewards(Quest->Quest->Rewards);

    // Check for bonus objectives completion
    bool AllObjectivesComplete = true;
    for (const FQuestObjective& Obj : Quest->ObjectiveStates)
    {
        if (!Obj.bIsCompleted)
        {
            AllObjectivesComplete = false;
            break;
        }
    }

    if (AllObjectivesComplete)
    {
        GiveQuestRewards(Quest->Quest->BonusRewards);
    }

    OnQuestCompleted.Broadcast(QuestID);

    // Remove from active quests
    ActiveQuests.RemoveAll([QuestID](const FActiveQuest& Q) { return Q.Quest && Q.Quest->QuestID == QuestID; });

    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Completed quest %s"), *QuestID.ToString());

    return true;
}

bool UQuestManagerSubsystem::FailQuest(FName QuestID, const FString& Reason)
{
    FActiveQuest* Quest = FindActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestManagerSubsystem: Cannot fail quest %s - not active"), *QuestID.ToString());
        return false;
    }

    Quest->Status = EQuestStatus::Failed;
    FailedQuestIDs.Add(QuestID);

    OnQuestFailed.Broadcast(QuestID, Reason);

    // Remove from active quests
    ActiveQuests.RemoveAll([QuestID](const FActiveQuest& Q) { return Q.Quest && Q.Quest->QuestID == QuestID; });

    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Failed quest %s - %s"), *QuestID.ToString(), *Reason);

    return true;
}

// ====================
// OBJECTIVE TRACKING
// ====================

bool UQuestManagerSubsystem::UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 NewProgress)
{
    FActiveQuest* Quest = FindActiveQuest(QuestID);
    if (!Quest || ObjectiveIndex < 0 || ObjectiveIndex >= Quest->ObjectiveStates.Num())
    {
        return false;
    }

    FQuestObjective& Objective = Quest->ObjectiveStates[ObjectiveIndex];
    Objective.CurrentProgress = FMath::Min(NewProgress, Objective.RequiredQuantity);

    // Check if objective is now complete
    if (Objective.CurrentProgress >= Objective.RequiredQuantity && !Objective.bIsCompleted)
    {
        Objective.bIsCompleted = true;
        OnObjectiveCompleted.Broadcast(QuestID, ObjectiveIndex);
        
        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Completed objective %d for quest %s"), ObjectiveIndex, *QuestID.ToString());

        // Check if quest is now complete
        CheckQuestCompletion(*Quest);
    }
    else
    {
        OnObjectiveUpdated.Broadcast(QuestID, ObjectiveIndex, NewProgress);
    }

    return true;
}

bool UQuestManagerSubsystem::CompleteObjective(FName QuestID, int32 ObjectiveIndex)
{
    FActiveQuest* Quest = FindActiveQuest(QuestID);
    if (!Quest || ObjectiveIndex < 0 || ObjectiveIndex >= Quest->ObjectiveStates.Num())
    {
        return false;
    }

    FQuestObjective& Objective = Quest->ObjectiveStates[ObjectiveIndex];
    Objective.CurrentProgress = Objective.RequiredQuantity;
    Objective.bIsCompleted = true;

    OnObjectiveCompleted.Broadcast(QuestID, ObjectiveIndex);

    UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Completed objective %d for quest %s"), ObjectiveIndex, *QuestID.ToString());

    // Check if quest is now complete
    CheckQuestCompletion(*Quest);

    return true;
}

void UQuestManagerSubsystem::NotifyLocationReached(FVector Location)
{
    for (FActiveQuest& Quest : ActiveQuests)
    {
        for (int32 i = 0; i < Quest.ObjectiveStates.Num(); ++i)
        {
            FQuestObjective& Objective = Quest.ObjectiveStates[i];
            
            if (Objective.Type == EObjectiveType::ReachLocation && !Objective.bIsCompleted)
            {
                float Distance = FVector::Dist(Location, Objective.TargetLocation);
                float Threshold = Objective.LocationThreshold > 0.0f ? Objective.LocationThreshold : DefaultLocationThreshold;
                if (Distance <= Threshold)
                {
                    CompleteObjective(Quest.Quest->QuestID, i);
                }
            }
        }
    }
}

void UQuestManagerSubsystem::NotifyItemCollected(FName ItemID, int32 Quantity)
{
    for (FActiveQuest& Quest : ActiveQuests)
    {
        for (int32 i = 0; i < Quest.ObjectiveStates.Num(); ++i)
        {
            FQuestObjective& Objective = Quest.ObjectiveStates[i];
            
            if ((Objective.Type == EObjectiveType::CollectItems || Objective.Type == EObjectiveType::DeliverItem) 
                && Objective.ItemID == ItemID && !Objective.bIsCompleted)
            {
                UpdateObjectiveProgress(Quest.Quest->QuestID, i, Objective.CurrentProgress + Quantity);
            }
        }
    }
}

void UQuestManagerSubsystem::NotifyTargetDestroyed(TSubclassOf<AActor> TargetClass)
{
    for (FActiveQuest& Quest : ActiveQuests)
    {
        for (int32 i = 0; i < Quest.ObjectiveStates.Num(); ++i)
        {
            FQuestObjective& Objective = Quest.ObjectiveStates[i];
            
            if (Objective.Type == EObjectiveType::DestroyTarget 
                && Objective.TargetActorClass == TargetClass && !Objective.bIsCompleted)
            {
                UpdateObjectiveProgress(Quest.Quest->QuestID, i, Objective.CurrentProgress + 1);
            }
        }
    }
}

// ====================
// QUERIES
// ====================

TArray<FActiveQuest> UQuestManagerSubsystem::GetActiveQuests() const
{
    return ActiveQuests;
}

bool UQuestManagerSubsystem::GetQuestByID(FName QuestID, FActiveQuest& OutQuest) const
{
    for (const FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.Quest && Quest.Quest->QuestID == QuestID)
        {
            OutQuest = Quest;
            return true;
        }
    }
    return false;
}

bool UQuestManagerSubsystem::IsQuestActive(FName QuestID) const
{
    for (const FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.Quest && Quest.Quest->QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

bool UQuestManagerSubsystem::HasCompletedQuest(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

int32 UQuestManagerSubsystem::GetActiveQuestCount() const
{
    return ActiveQuests.Num();
}

int32 UQuestManagerSubsystem::GetCompletedQuestCount() const
{
    return CompletedQuestIDs.Num();
}

TArray<FActiveQuest> UQuestManagerSubsystem::GetQuestsByType(EQuestType Type) const
{
    TArray<FActiveQuest> FilteredQuests;
    
    for (const FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.Quest && Quest.Quest->QuestType == Type)
        {
            FilteredQuests.Add(Quest);
        }
    }
    
    return FilteredQuests;
}

TArray<FActiveQuest> UQuestManagerSubsystem::GetQuestsFromGiver(FName WayID) const
{
    TArray<FActiveQuest> FilteredQuests;
    
    for (const FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.Quest && Quest.Quest->QuestGiverWayID == WayID)
        {
            FilteredQuests.Add(Quest);
        }
    }
    
    return FilteredQuests;
}

// ====================
// QUEST GENERATION
// ====================

UQuestDataAsset* UQuestManagerSubsystem::GenerateRandomQuest(EQuestType Type, int32 Difficulty, FName QuestGiver)
{
    // This is a basic implementation - would be expanded for proper procedural generation
    UQuestDataAsset* GeneratedQuest = NewObject<UQuestDataAsset>(this);
    
    if (GeneratedQuest)
    {
        GeneratedQuest->QuestID = FName(*FString::Printf(TEXT("Quest_%d"), FMath::Rand()));
        GeneratedQuest->QuestType = Type;
        GeneratedQuest->Difficulty = Difficulty;
        GeneratedQuest->QuestGiverWayID = QuestGiver;
        GeneratedQuest->bIsRepeatable = true;

        // Generate quest name based on type
        switch (Type)
        {
        case EQuestType::Delivery:
            GeneratedQuest->QuestName = FText::FromString(TEXT("Delivery Mission"));
            GeneratedQuest->Description = FText::FromString(TEXT("Deliver cargo to destination"));
            break;
        case EQuestType::Combat:
            GeneratedQuest->QuestName = FText::FromString(TEXT("Combat Mission"));
            GeneratedQuest->Description = FText::FromString(TEXT("Eliminate hostile targets"));
            break;
        case EQuestType::Exploration:
            GeneratedQuest->QuestName = FText::FromString(TEXT("Exploration Mission"));
            GeneratedQuest->Description = FText::FromString(TEXT("Scout unknown sector"));
            break;
        default:
            GeneratedQuest->QuestName = FText::FromString(TEXT("Mission"));
            GeneratedQuest->Description = FText::FromString(TEXT("Complete objectives"));
            break;
        }

        // Set rewards based on difficulty and configured multipliers
        GeneratedQuest->Rewards.Credits = Difficulty * CreditsPerDifficulty;
        GeneratedQuest->Rewards.ExperiencePoints = Difficulty * ExperiencePerDifficulty;
        GeneratedQuest->Rewards.ReputationGain = Difficulty * ReputationPerDifficulty;
        GeneratedQuest->Rewards.WayID = QuestGiver;

        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Generated quest '%s'"), *GeneratedQuest->QuestName.ToString());
    }

    return GeneratedQuest;
}

// ====================
// PRIVATE HELPERS
// ====================

void UQuestManagerSubsystem::UpdateQuestTimers()
{
    // Update time-limited quests
    for (FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.TimeRemaining > 0.0f)
        {
            Quest.TimeRemaining -= 1.0f; // Decrement by 1 second
            
            if (Quest.TimeRemaining <= 0.0f)
            {
                FailQuest(Quest.Quest->QuestID, TEXT("Time limit expired"));
            }
        }
    }
}

void UQuestManagerSubsystem::GiveQuestRewards(const FQuestReward& Rewards)
{
    // This would integrate with player inventory/reputation systems
    // For now, just log the rewards
    
    if (Rewards.Credits > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Rewarding %d credits"), Rewards.Credits);
    }
    
    if (Rewards.ReputationGain != 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Rewarding %d reputation with %s"), 
               Rewards.ReputationGain, *Rewards.WayID.ToString());
    }
    
    if (Rewards.ExperiencePoints > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Rewarding %d XP"), Rewards.ExperiencePoints);
    }
}

bool UQuestManagerSubsystem::CheckQuestCompletion(FActiveQuest& Quest)
{
    if (Quest.Quest && Quest.Quest->AreAllRequiredObjectivesComplete())
    {
        UE_LOG(LogAdastrea, Log, TEXT("QuestManagerSubsystem: Quest %s ready for completion"), 
               *Quest.Quest->QuestName.ToString());
        return true;
    }
    return false;
}

FActiveQuest* UQuestManagerSubsystem::FindActiveQuest(FName QuestID)
{
    for (FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.Quest && Quest.Quest->QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}
