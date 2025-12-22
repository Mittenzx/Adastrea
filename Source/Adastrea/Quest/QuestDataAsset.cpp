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
