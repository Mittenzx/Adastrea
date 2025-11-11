#include "Quest/QuestDataAsset.h"
#include "AdastreaLog.h"

UQuestDataAsset::UQuestDataAsset()
{
    QuestName = FText::FromString(TEXT("New Quest"));
    Description = FText::FromString(TEXT("Quest description"));
    QuestID = NAME_None;
    QuestType = EQuestType::Delivery;
    QuestGiverWayID = NAME_None;
    Difficulty = 1;
    bIsMainQuest = false;
    bIsRepeatable = false;
    TimeLimit = 0.0f;
    NextQuestInChain = nullptr;
    PreviousQuestInChain = nullptr;
}

TArray<FQuestObjective> UQuestDataAsset::GetObjectives() const
{
    return Objectives;
}

int32 UQuestDataAsset::GetCompletedObjectiveCount() const
{
    int32 Count = 0;
    for (const FQuestObjective& Objective : Objectives)
    {
        if (Objective.bIsCompleted)
        {
            Count++;
        }
    }
    return Count;
}

int32 UQuestDataAsset::GetTotalObjectiveCount() const
{
    return Objectives.Num();
}

float UQuestDataAsset::GetCompletionPercentage() const
{
    int32 Total = GetTotalObjectiveCount();
    if (Total == 0)
    {
        return 0.0f;
    }

    int32 Completed = GetCompletedObjectiveCount();
    return (static_cast<float>(Completed) / Total) * 100.0f;
}

bool UQuestDataAsset::AreAllRequiredObjectivesComplete() const
{
    for (const FQuestObjective& Objective : Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }
    return true;
}

bool UQuestDataAsset::CheckPrerequisites() const
{
    // This would need access to player state to check properly
    // For now, just check if prerequisites exist
    // In actual implementation, this would query the game state
    
    for (const FQuestPrerequisite& Prereq : Prerequisites)
    {
        if (Prereq.RequiredQuest != nullptr)
        {
            // Would check if player has completed this quest
            // return false if not completed
        }
        
        if (Prereq.RequiredWayID != NAME_None)
        {
            // Would check player reputation with Way
            // return false if below threshold
        }
        
        if (Prereq.MinimumPlayerLevel > 1)
        {
            // Would check player level
            // return false if below level
        }
    }
    
    return true;
}

int32 UQuestDataAsset::GetTotalRewardValue() const
{
    int32 TotalValue = Rewards.Credits;
    
    // Could add estimated value of items, reputation, etc.
    TotalValue += Rewards.ReputationGain * 10; // Arbitrary value for reputation
    TotalValue += Rewards.ExperiencePoints; // XP value
    
    return TotalValue;
}

FText UQuestDataAsset::GetDifficultyText() const
{
    switch (Difficulty)
    {
    case 1:
    case 2:
        return FText::FromString(TEXT("Easy"));
    case 3:
    case 4:
        return FText::FromString(TEXT("Normal"));
    case 5:
    case 6:
        return FText::FromString(TEXT("Moderate"));
    case 7:
    case 8:
        return FText::FromString(TEXT("Hard"));
    case 9:
    case 10:
        return FText::FromString(TEXT("Very Hard"));
    default:
        return FText::FromString(TEXT("Unknown"));
    }
}

FText UQuestDataAsset::GetQuestTypeDisplayName(EQuestType Type)
{
    switch (Type)
    {
    case EQuestType::Delivery:
        return FText::FromString(TEXT("Delivery"));
    case EQuestType::Exploration:
        return FText::FromString(TEXT("Exploration"));
    case EQuestType::Combat:
        return FText::FromString(TEXT("Combat"));
    case EQuestType::Escort:
        return FText::FromString(TEXT("Escort"));
    case EQuestType::Mining:
        return FText::FromString(TEXT("Mining"));
    case EQuestType::Research:
        return FText::FromString(TEXT("Research"));
    case EQuestType::Diplomatic:
        return FText::FromString(TEXT("Diplomatic"));
    case EQuestType::Rescue:
        return FText::FromString(TEXT("Rescue"));
    case EQuestType::Sabotage:
        return FText::FromString(TEXT("Sabotage"));
    case EQuestType::Custom:
        return FText::FromString(TEXT("Custom"));
    default:
        return FText::FromString(TEXT("Unknown"));
    }
}

FText UQuestDataAsset::GetObjectiveTypeDisplayName(EObjectiveType Type)
{
    switch (Type)
    {
    case EObjectiveType::ReachLocation:
        return FText::FromString(TEXT("Reach Location"));
    case EObjectiveType::DeliverItem:
        return FText::FromString(TEXT("Deliver Item"));
    case EObjectiveType::DestroyTarget:
        return FText::FromString(TEXT("Destroy Target"));
    case EObjectiveType::CollectItems:
        return FText::FromString(TEXT("Collect Items"));
    case EObjectiveType::ScanObject:
        return FText::FromString(TEXT("Scan Object"));
    case EObjectiveType::TalkToNPC:
        return FText::FromString(TEXT("Talk to NPC"));
    case EObjectiveType::ProtectTarget:
        return FText::FromString(TEXT("Protect Target"));
    case EObjectiveType::SurviveTime:
        return FText::FromString(TEXT("Survive Time"));
    case EObjectiveType::Custom:
        return FText::FromString(TEXT("Custom"));
    default:
        return FText::FromString(TEXT("Unknown"));
    }
}
