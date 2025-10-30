#include "Public/AI/PersonnelLogic.h"

UPersonnelLogic::UPersonnelLogic()
{
    // Set personnel-specific defaults
    Disposition = EPersonnelDisposition::Pragmatic;
    CurrentTask = EPersonnelTask::Personal;
    bOnDuty = false;
    bIsResting = false;
    HoursUntilNextShift = 8.0f;
    HoursUntilTaskComplete = 0.0f;

    // Personnel update more frequently than factions
    UpdateInterval = 1.0f;
    
    // Default to peaceful behavior
    CurrentBehaviorMode = EAIBehaviorMode::Peaceful;
}

// ====================
// Personnel-Specific AI Implementation
// ====================

EPersonnelTask UPersonnelLogic::EvaluateTaskPriority_Implementation()
{
    // Check personnel data for status
    if (!PersonnelData)
    {
        return EPersonnelTask::Personal;
    }

    // Emergency situations take priority
    if (PersonnelData->Health < 30.0f)
    {
        return EPersonnelTask::Emergency;
    }

    // Check if needs rest
    if (NeedsRest() && !bOnDuty)
    {
        return EPersonnelTask::Personal;
    }

    // If on duty, prioritize duty station
    if (bOnDuty)
    {
        return EPersonnelTask::DutyStation;
    }

    // Off-duty behavior based on disposition
    switch (Disposition)
    {
        case EPersonnelDisposition::Curious:
            // Curious personnel explore when off duty
            if (PersonnelData->Morale > 60.0f)
            {
                return EPersonnelTask::Exploration;
            }
            break;

        case EPersonnelDisposition::Empathetic:
            // Empathetic personnel socialize
            if (PersonnelData->Morale > 50.0f && FriendsList.Num() > 0)
            {
                return EPersonnelTask::Social;
            }
            break;

        case EPersonnelDisposition::Analytical:
        case EPersonnelDisposition::Creative:
            // Analytical and creative types pursue training
            if (ShouldPursueTraining())
            {
                return EPersonnelTask::Training;
            }
            break;

        case EPersonnelDisposition::Disciplined:
            // Disciplined personnel do maintenance
            return EPersonnelTask::Maintenance;

        default:
            break;
    }

    // Default to personal time
    return EPersonnelTask::Personal;
}

void UPersonnelLogic::HandleSocialInteraction_Implementation(FName OtherPersonnelID, const FString& InteractionType)
{
    if (!PersonnelData)
    {
        return;
    }

    // Check relationship with the other person
    FPersonnelRelationship Relationship;
    bool bHasRelationship = PersonnelData->GetRelationship(OtherPersonnelID, Relationship);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("%s: %s interaction with %s"),
                *PersonnelData->PersonnelName.ToString(),
                *InteractionType,
                *OtherPersonnelID.ToString()));
    }

    // Handle based on relationship and disposition
    if (bHasRelationship)
    {
        if (Relationship.RelationshipStrength > 50)
        {
            // Positive interaction with friend
            // Boost morale slightly (capped at 100)
            if (PersonnelData->Morale < 100.0f)
            {
                // This is a const function, so we can't modify directly
                // In a real implementation, this would trigger an event
                // that the owner can respond to
            }
        }
        else if (Relationship.RelationshipStrength < -50)
        {
            // Negative interaction with rival
            // Reduce morale slightly
            HandleStressChange(5.0f);
        }
    }

    // Empathetic disposition gains more from social interactions
    if (Disposition == EPersonnelDisposition::Empathetic)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("%s: Enjoys the social interaction"),
                    *PersonnelData->PersonnelName.ToString()));
        }
    }
}

int32 UPersonnelLogic::MakePersonalityDrivenDecision_Implementation(const FString& DecisionContext, const TArray<FString>& Options)
{
    if (Options.Num() == 0)
    {
        return -1;
    }

    // Decision making based on disposition
    switch (Disposition)
    {
        case EPersonnelDisposition::Cautious:
            // Choose the first (safest) option
            return 0;

        case EPersonnelDisposition::Bold:
            // Choose the last (most risky) option
            return Options.Num() - 1;

        case EPersonnelDisposition::Analytical:
            // Analyze and choose middle option (balanced)
            return Options.Num() / 2;

        case EPersonnelDisposition::Empathetic:
            // Consider others - choose option mentioning people/crew
            for (int32 i = 0; i < Options.Num(); i++)
            {
                if (Options[i].Contains(TEXT("crew")) || Options[i].Contains(TEXT("people")))
                {
                    return i;
                }
            }
            return 0;

        case EPersonnelDisposition::Pragmatic:
            // Choose most efficient option - look for keywords
            for (int32 i = 0; i < Options.Num(); i++)
            {
                if (Options[i].Contains(TEXT("efficient")) || Options[i].Contains(TEXT("quick")))
                {
                    return i;
                }
            }
            return 0;

        case EPersonnelDisposition::Curious:
            // Choose most interesting/novel option
            for (int32 i = 0; i < Options.Num(); i++)
            {
                if (Options[i].Contains(TEXT("new")) || Options[i].Contains(TEXT("explore")))
                {
                    return i;
                }
            }
            return Options.Num() - 1; // Default to something different

        case EPersonnelDisposition::Disciplined:
            // Follow rules - choose by-the-book option
            for (int32 i = 0; i < Options.Num(); i++)
            {
                if (Options[i].Contains(TEXT("protocol")) || Options[i].Contains(TEXT("regulation")))
                {
                    return i;
                }
            }
            return 0;

        case EPersonnelDisposition::Creative:
            // Choose unconventional option
            return Options.Num() > 1 ? 1 : 0; // Second option if available

        default:
            return 0;
    }
}

void UPersonnelLogic::OnTaskCompleted_Implementation(EPersonnelTask CompletedTask, bool Success)
{
    if (GEngine)
    {
        FColor MessageColor = Success ? FColor::Green : FColor::Yellow;
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, MessageColor,
            FString::Printf(TEXT("%s: Completed %s - %s"),
                PersonnelData ? *PersonnelData->PersonnelName.ToString() : TEXT("Unknown"),
                *UEnum::GetValueAsString(CompletedTask),
                Success ? TEXT("Success") : TEXT("Partial Success")));
    }

    // Reset task timer
    HoursUntilTaskComplete = 0.0f;

    // In a real implementation, this would update PersonnelData
    // But since data assets are const, we'd need a runtime state manager
    // For now, just log the completion
}

void UPersonnelLogic::UpdateDailyRoutine_Implementation()
{
    // Decrement shift timer
    if (HoursUntilNextShift > 0.0f)
    {
        HoursUntilNextShift -= UpdateInterval / 3600.0f; // Convert seconds to hours
    }

    // Check if it's time for shift change
    if (HoursUntilNextShift <= 0.0f && !bOnDuty)
    {
        bOnDuty = true;
        HoursUntilNextShift = 8.0f; // 8 hour shift
        
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
                FString::Printf(TEXT("%s: Starting duty shift"),
                    *PersonnelData->PersonnelName.ToString()));
        }
    }
    else if (HoursUntilNextShift <= -8.0f && bOnDuty)
    {
        // End of shift
        bOnDuty = false;
        HoursUntilNextShift = 16.0f; // 16 hours until next shift
        
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                FString::Printf(TEXT("%s: Ending duty shift"),
                    *PersonnelData->PersonnelName.ToString()));
        }
    }

    // Check if should be resting
    if (!bOnDuty && NeedsRest())
    {
        bIsResting = true;
    }
    else if (PersonnelData && PersonnelData->Fatigue < 30.0f)
    {
        bIsResting = false;
    }
}

void UPersonnelLogic::HandleStressChange_Implementation(float StressAmount)
{
    if (!PersonnelData)
    {
        return;
    }

    // Apply disposition modifiers
    float ModifiedStress = StressAmount;

    switch (Disposition)
    {
        case EPersonnelDisposition::Cautious:
            // Cautious people stress more easily
            ModifiedStress *= 1.2f;
            break;

        case EPersonnelDisposition::Bold:
            // Bold people handle stress better
            ModifiedStress *= 0.8f;
            break;

        case EPersonnelDisposition::Disciplined:
            // Disciplined people are resilient
            ModifiedStress *= 0.9f;
            break;

        case EPersonnelDisposition::Empathetic:
            // Empathetic people absorb others' stress
            ModifiedStress *= 1.1f;
            break;

        default:
            break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, 
            ModifiedStress > 0 ? FColor::Orange : FColor::Green,
            FString::Printf(TEXT("%s: Stress %s by %.1f"),
                *PersonnelData->PersonnelName.ToString(),
                ModifiedStress > 0 ? TEXT("increased") : TEXT("decreased"),
                FMath::Abs(ModifiedStress)));
    }

    // In real implementation, this would update morale/fatigue
    // through a runtime state manager
}

bool UPersonnelLogic::ShouldPursueTraining_Implementation()
{
    if (!PersonnelData)
    {
        return false;
    }

    // Don't train if tired or low morale
    if (NeedsRest() || PersonnelData->Morale < 50.0f)
    {
        return false;
    }

    // Certain dispositions are more inclined to train
    if (Disposition == EPersonnelDisposition::Curious ||
        Disposition == EPersonnelDisposition::Analytical ||
        Disposition == EPersonnelDisposition::Creative)
    {
        return true;
    }

    // Check if skills are below overall level (need improvement)
    if (PersonnelData->OverallSkillLevel < 7)
    {
        return true;
    }

    return false;
}

// ====================
// Utility Functions
// ====================

bool UPersonnelLogic::IsFriendsWith(FName OtherPersonnelID) const
{
    return FriendsList.Contains(OtherPersonnelID);
}

bool UPersonnelLogic::HasConflictWith(FName OtherPersonnelID) const
{
    return ConflictList.Contains(OtherPersonnelID);
}

void UPersonnelLogic::MakeFriend(FName OtherPersonnelID)
{
    if (!FriendsList.Contains(OtherPersonnelID))
    {
        FriendsList.Add(OtherPersonnelID);
        
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("%s: Made friends with %s"),
                    *PersonnelData->PersonnelName.ToString(),
                    *OtherPersonnelID.ToString()));
        }
    }
}

void UPersonnelLogic::RemoveFriend(FName OtherPersonnelID)
{
    FriendsList.Remove(OtherPersonnelID);
}

void UPersonnelLogic::AddConflict(FName OtherPersonnelID)
{
    if (!ConflictList.Contains(OtherPersonnelID))
    {
        ConflictList.Add(OtherPersonnelID);
        
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                FString::Printf(TEXT("%s: Conflict with %s"),
                    *PersonnelData->PersonnelName.ToString(),
                    *OtherPersonnelID.ToString()));
        }
    }
}

void UPersonnelLogic::ResolveConflict(FName OtherPersonnelID)
{
    if (ConflictList.Remove(OtherPersonnelID) > 0)
    {
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                FString::Printf(TEXT("%s: Resolved conflict with %s"),
                    *PersonnelData->PersonnelName.ToString(),
                    *OtherPersonnelID.ToString()));
        }
    }
}

bool UPersonnelLogic::NeedsRest() const
{
    if (!PersonnelData)
    {
        return false;
    }

    return PersonnelData->Fatigue > 70.0f || PersonnelData->Morale < 30.0f;
}

bool UPersonnelLogic::IsFitForDuty() const
{
    if (!PersonnelData)
    {
        return false;
    }

    return PersonnelData->Health > 50.0f && 
           PersonnelData->Morale > 40.0f &&
           PersonnelData->Fatigue < 80.0f;
}

bool UPersonnelLogic::IsSuitedForTask(EPersonnelTask Task) const
{
    // Match disposition to task compatibility
    switch (Task)
    {
        case EPersonnelTask::Exploration:
            return Disposition == EPersonnelDisposition::Curious ||
                   Disposition == EPersonnelDisposition::Bold;

        case EPersonnelTask::Social:
            return Disposition == EPersonnelDisposition::Empathetic ||
                   Disposition == EPersonnelDisposition::Creative;

        case EPersonnelTask::Maintenance:
            return Disposition == EPersonnelDisposition::Disciplined ||
                   Disposition == EPersonnelDisposition::Pragmatic;

        case EPersonnelTask::Training:
            return Disposition == EPersonnelDisposition::Analytical ||
                   Disposition == EPersonnelDisposition::Curious;

        case EPersonnelTask::DutyStation:
            return Disposition == EPersonnelDisposition::Disciplined ||
                   Disposition == EPersonnelDisposition::Pragmatic;

        default:
            return true; // Everyone can do Personal and Emergency tasks
    }
}

FText UPersonnelLogic::GetPersonnelName() const
{
    if (PersonnelData)
    {
        return PersonnelData->PersonnelName;
    }
    return FText::FromString(TEXT("Unknown Personnel"));
}

FString UPersonnelLogic::GetPersonnelStateDescription() const
{
    FString BaseState = GetAIStateDescription();
    FString TaskStr = UEnum::GetValueAsString(CurrentTask);
    FString DutyStatus = bOnDuty ? TEXT("On Duty") : TEXT("Off Duty");
    
    return FString::Printf(TEXT("%s | %s | %s | %s"),
        *GetPersonnelName().ToString(),
        *BaseState,
        *TaskStr,
        *DutyStatus);
}

float UPersonnelLogic::ApplyPersonalityModifier(float BaseModifier) const
{
    // Apply disposition-based modifications
    switch (Disposition)
    {
        case EPersonnelDisposition::Cautious:
            return BaseModifier * 0.9f; // More conservative
        
        case EPersonnelDisposition::Bold:
            return BaseModifier * 1.2f; // More aggressive
        
        case EPersonnelDisposition::Analytical:
            return BaseModifier * 1.0f; // Balanced
        
        case EPersonnelDisposition::Empathetic:
            return BaseModifier * 0.95f; // Slightly cautious with others
        
        case EPersonnelDisposition::Pragmatic:
            return BaseModifier * 1.1f; // Efficient boost
        
        case EPersonnelDisposition::Curious:
            return BaseModifier * 1.15f; // Exploration bonus
        
        case EPersonnelDisposition::Disciplined:
            return BaseModifier * 1.05f; // Consistent performance
        
        case EPersonnelDisposition::Creative:
            return BaseModifier * 1.1f; // Innovation bonus
        
        default:
            return BaseModifier;
    }
}

// ====================
// Overridden Functions
// ====================

void UPersonnelLogic::InitializeAI_Implementation()
{
    Super::InitializeAI_Implementation();

    // Set initial task
    CurrentTask = EvaluateTaskPriority();

    if (GEngine && PersonnelData)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("Personnel AI Initialized: %s - Task: %s"),
                *PersonnelData->PersonnelName.ToString(),
                *UEnum::GetValueAsString(CurrentTask)));
    }
}

void UPersonnelLogic::OnTickAI_Implementation(float DeltaTime)
{
    // Update daily routine
    UpdateDailyRoutine();

    // Update current task
    EPersonnelTask NewTask = EvaluateTaskPriority();
    if (NewTask != CurrentTask)
    {
        CurrentTask = NewTask;
        
        if (GEngine && PersonnelData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
                FString::Printf(TEXT("%s: Switching to %s"),
                    *PersonnelData->PersonnelName.ToString(),
                    *UEnum::GetValueAsString(CurrentTask)));
        }
    }

    // Call parent implementation
    Super::OnTickAI_Implementation(DeltaTime);
}

EAIPriority UPersonnelLogic::EvaluateCurrentPriority_Implementation()
{
    // Map personnel task to AI priority
    switch (CurrentTask)
    {
        case EPersonnelTask::Emergency:
            return EAIPriority::Critical;
        
        case EPersonnelTask::DutyStation:
            return EAIPriority::High;
        
        case EPersonnelTask::Maintenance:
        case EPersonnelTask::Training:
            return EAIPriority::Medium;
        
        case EPersonnelTask::Social:
        case EPersonnelTask::Exploration:
            return EAIPriority::Low;
        
        case EPersonnelTask::Personal:
        default:
            return EAIPriority::Idle;
    }
}
