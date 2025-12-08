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
    // Difficulty range constants
    static constexpr int32 EasyMax = 2;
    static constexpr int32 NormalMax = 4;
    static constexpr int32 ModerateMax = 6;
    static constexpr int32 HardMax = 8;
    static constexpr int32 VeryHardMax = 10;

    if (Difficulty <= EasyMax)
        return FText::FromString(TEXT("Easy"));
    else if (Difficulty <= NormalMax)
        return FText::FromString(TEXT("Normal"));
    else if (Difficulty <= ModerateMax)
        return FText::FromString(TEXT("Moderate"));
    else if (Difficulty <= HardMax)
        return FText::FromString(TEXT("Hard"));
    else if (Difficulty <= VeryHardMax)
        return FText::FromString(TEXT("Very Hard"));
    else
        return FText::FromString(TEXT("Unknown"));
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

#if WITH_EDITOR
EDataValidationResult UQuestDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    // Validate basic info
    if (QuestName.IsEmpty())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Quest Name is empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (QuestID.IsNone())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Quest ID is not set")));
        Result = EDataValidationResult::Invalid;
    }

    if (QuestDescription.IsEmpty())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Quest Description is empty")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate objectives
    if (Objectives.Num() == 0)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Quest has no objectives")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate each objective
    for (int32 i = 0; i < Objectives.Num(); i++)
    {
        const FQuestObjective& Objective = Objectives[i];
        
        if (Objective.ObjectiveDescription.IsEmpty())
        {
            ValidationErrors.Add(FText::Format(
                FText::FromString(TEXT("Objective {0} has empty description")),
                FText::AsNumber(i + 1)
            ));
            Result = EDataValidationResult::Invalid;
        }

        if (Objective.RequiredProgress <= 0)
        {
            ValidationErrors.Add(FText::Format(
                FText::FromString(TEXT("Objective {0} has invalid required progress (<= 0)")),
                FText::AsNumber(i + 1)
            ));
            Result = EDataValidationResult::Invalid;
        }
    }

    // Validate rewards
    if (CreditReward < 0)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Credit Reward cannot be negative")));
        Result = EDataValidationResult::Invalid;
    }

    if (ExperienceReward < 0)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Experience Reward cannot be negative")));
        Result = EDataValidationResult::Invalid;
    }

    if (ReputationChange < -100 || ReputationChange > 100)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Reputation Change ({0}) must be between -100 and 100")),
            FText::AsNumber(ReputationChange)
        ));
        Result = EDataValidationResult::Invalid;
    }

    // Warn about potential issues
    if (CreditReward == 0 && ExperienceReward == 0 && ItemRewards.Num() == 0)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Warning: Quest has no rewards")));
        // Just a warning
    }

    if (QuestType == EQuestType::MainStory && !bIsRepeatable)
    {
        // Good - main story quests should not be repeatable
    }
    else if (QuestType == EQuestType::DailyMission && !bIsRepeatable)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Warning: Daily mission should probably be repeatable")));
        // Just a warning
    }

    // Validate quest chain consistency
    if (NextQuestInChain && PreviousQuestInChain == NextQuestInChain)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Next Quest and Previous Quest cannot be the same")));
        Result = EDataValidationResult::Invalid;
    }

    // Log validation result
    if (Result == EDataValidationResult::Valid)
    {
        UE_LOG(LogAdastrea, Log, TEXT("QuestDataAsset %s passed validation"), *QuestName.ToString());
    }
    else
    {
        UE_LOG(LogAdastrea, Warning, TEXT("QuestDataAsset %s failed validation with %d errors"), 
            *QuestName.ToString(), ValidationErrors.Num());
    }

    return Result;
}
#endif
