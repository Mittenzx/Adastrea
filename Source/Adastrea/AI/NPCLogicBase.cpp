#include "AI/NPCLogicBase.h"

UNPCLogicBase::UNPCLogicBase()
{
    // Set default values
    CurrentBehaviorMode = EAIBehaviorMode::Peaceful;
    bIsActive = true;
    UpdateInterval = 1.0f; // Update once per second by default
    TimeSinceLastUpdate = 0.0f;
}

// ====================
// Core AI Loop Implementation
// ====================

void UNPCLogicBase::InitializeAI_Implementation()
{
    // Default implementation - can be overridden in Blueprint
    TimeSinceLastUpdate = 0.0f;
    
    // Log initialization for debugging
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("AI Initialized: %s"), *GetName()));
    }
}

void UNPCLogicBase::OnTickAI_Implementation(float DeltaTime)
{
    // Default implementation - evaluate priorities and decide actions
    // This is meant to be overridden in Blueprint or derived classes
    
    // Step 1: Evaluate current priority
    EAIPriority CurrentPriority = EvaluateCurrentPriority();
    
    // Step 2: Decide on action based on priority
    DecideNextAction(CurrentPriority);
    
    // Step 3: Check if we should change behavior mode
    EAIBehaviorMode NewMode = ShouldChangeBehaviorMode();
    if (NewMode != CurrentBehaviorMode)
    {
        SetBehaviorMode(NewMode);
    }
}

void UNPCLogicBase::OnActivateAI_Implementation()
{
    // Default implementation
    bIsActive = true;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("AI Activated: %s"), *GetName()));
    }
}

void UNPCLogicBase::OnDeactivateAI_Implementation()
{
    // Default implementation
    bIsActive = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("AI Deactivated: %s"), *GetName()));
    }
}

void UNPCLogicBase::OnBehaviorModeChanged_Implementation(EAIBehaviorMode OldMode, EAIBehaviorMode NewMode)
{
    // Default implementation - log the change
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("AI Mode Changed: %s -> %s"), 
                *UEnum::GetValueAsString(OldMode),
                *UEnum::GetValueAsString(NewMode)));
    }
}

// ====================
// Decision Making Implementation
// ====================

EAIPriority UNPCLogicBase::EvaluateCurrentPriority_Implementation()
{
    // Default implementation - returns Idle
    // Override in Blueprint or derived class to implement actual priority logic
    return EAIPriority::Idle;
}

bool UNPCLogicBase::DecideNextAction_Implementation(EAIPriority CurrentPriority)
{
    // Default implementation - does nothing
    // Override in Blueprint or derived class to implement actual decision logic
    return false;
}

EAIBehaviorMode UNPCLogicBase::ShouldChangeBehaviorMode_Implementation()
{
    // Default implementation - keep current mode
    // Override in Blueprint or derived class to implement mode switching logic
    return CurrentBehaviorMode;
}

// ====================
// Public Control Functions
// ====================

void UNPCLogicBase::ActivateAI()
{
    if (!bIsActive)
    {
        OnActivateAI();
    }
}

void UNPCLogicBase::DeactivateAI()
{
    if (bIsActive)
    {
        OnDeactivateAI();
    }
}

void UNPCLogicBase::SetBehaviorMode(EAIBehaviorMode NewMode)
{
    if (NewMode != CurrentBehaviorMode)
    {
        EAIBehaviorMode OldMode = CurrentBehaviorMode;
        CurrentBehaviorMode = NewMode;
        OnBehaviorModeChanged(OldMode, NewMode);
    }
}

// ====================
// Query Functions
// ====================

bool UNPCLogicBase::IsInBehaviorMode(EAIBehaviorMode Mode) const
{
    return CurrentBehaviorMode == Mode;
}

bool UNPCLogicBase::IsPeaceful() const
{
    return CurrentBehaviorMode == EAIBehaviorMode::Peaceful ||
           CurrentBehaviorMode == EAIBehaviorMode::Trading ||
           CurrentBehaviorMode == EAIBehaviorMode::Exploration ||
           CurrentBehaviorMode == EAIBehaviorMode::Diplomatic;
}

bool UNPCLogicBase::IsAggressive() const
{
    return CurrentBehaviorMode == EAIBehaviorMode::Aggressive ||
           CurrentBehaviorMode == EAIBehaviorMode::Defensive;
}

FString UNPCLogicBase::GetAIStateDescription() const
{
    FString ModeString = UEnum::GetValueAsString(CurrentBehaviorMode);
    FString ActiveString = bIsActive ? TEXT("Active") : TEXT("Inactive");
    
    return FString::Printf(TEXT("%s - %s"), *ModeString, *ActiveString);
}

void UNPCLogicBase::UpdateAI(float DeltaTime)
{
    if (!bIsActive)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    // Only tick AI at the specified interval
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        OnTickAI(TimeSinceLastUpdate);
        TimeSinceLastUpdate = 0.0f;
    }
}
