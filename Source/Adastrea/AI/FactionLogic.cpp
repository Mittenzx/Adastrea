#include "AI/FactionLogic.h"

UFactionLogic::UFactionLogic()
{
    // Set faction-specific defaults
    CurrentStrategicPriority = EFactionPriority::Exploration;
    bIsEarlyGame = true;
    TerritoryCount = 0;
    EconomicStrength = 50.0f;
    MilitaryPower = 30.0f;  // Lower for peaceful early game
    DiplomaticInfluence = 50.0f;

    // Default to peaceful exploration mode
    CurrentBehaviorMode = EAIBehaviorMode::Peaceful;
    UpdateInterval = 5.0f;  // Factions update less frequently than individual NPCs
}

// ====================
// Faction-Specific AI Implementation
// ====================

EFactionPriority UFactionLogic::EvaluateStrategicPriority_Implementation()
{
    // If in early game, use early game priorities
    if (bIsEarlyGame)
    {
        return GetTopEarlyGamePriority();
    }

    // Later game - evaluate based on current state
    // Check for survival threats
    if (MilitaryPower < 20.0f && !IsPeaceful())
    {
        return EFactionPriority::Survival;
    }

    // Check for defensive needs
    if (TerritoryCount > 0 && MilitaryPower < 40.0f)
    {
        return EFactionPriority::Defense;
    }

    // Check economic needs
    if (EconomicStrength < 50.0f)
    {
        return EFactionPriority::Trade;
    }

    // Default to expansion or exploration
    if (TerritoryCount < 3)
    {
        return EFactionPriority::Exploration;
    }

    return EFactionPriority::Expansion;
}

void UFactionLogic::HandleDiplomaticInteraction_Implementation(FName OtherFactionID, bool bInitiatedByUs)
{
    // Verify we have faction data
    if (!FactionData)
    {
        return;
    }

    // Check if we have a relationship with this faction
    FFactionRelationship Relationship;
    if (!FactionData->GetRelationship(OtherFactionID, Relationship))
    {
        // No existing relationship - create neutral contact
        if (GEngine && bInitiatedByUs)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                FString::Printf(TEXT("Faction %s: Making first contact with %s"),
                    *FactionData->FactionName.ToString(),
                    *OtherFactionID.ToString()));
        }
        return;
    }

    // Handle based on relationship status
    if (Relationship.bIsAllied)
    {
        // Allied factions - cooperative behavior
        if (GEngine && bInitiatedByUs)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                FString::Printf(TEXT("Faction %s: Coordinating with ally %s"),
                    *FactionData->FactionName.ToString(),
                    *OtherFactionID.ToString()));
        }
    }
    else if (Relationship.bAtWar)
    {
        // At war - hostile behavior (if not in peaceful mode)
        if (!IsPeaceful())
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                    FString::Printf(TEXT("Faction %s: Hostile encounter with enemy %s"),
                        *FactionData->FactionName.ToString(),
                        *OtherFactionID.ToString()));
            }
        }
    }
    else if (HasTruceWith(OtherFactionID))
    {
        // Truce - maintain peace
        if (GEngine && bInitiatedByUs)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                FString::Printf(TEXT("Faction %s: Respecting truce with %s"),
                    *FactionData->FactionName.ToString(),
                    *OtherFactionID.ToString()));
        }
    }
    else
    {
        // Neutral or friendly - opportunity for trade/diplomacy
        if (IsPeaceful() && bInitiatedByUs)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                    FString::Printf(TEXT("Faction %s: Peaceful interaction with %s"),
                        *FactionData->FactionName.ToString(),
                        *OtherFactionID.ToString()));
            }
        }
    }
}

void UFactionLogic::OnTerritoryDiscovered_Implementation(const FString& TerritoryName, float TerritoryValue)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("Faction %s: Discovered territory '%s' (Value: %.1f)"),
                FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown"),
                *TerritoryName,
                TerritoryValue));
    }

    // Evaluate if we should claim this territory
    if (bIsEarlyGame && IsExplorationFocused())
    {
        // Early game exploration - likely to claim
        if (TerritoryValue > 50.0f)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("Faction %s: Claiming territory '%s'"),
                        FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown"),
                        *TerritoryName));
            }
            TerritoryCount++;
        }
    }
}

bool UFactionLogic::ShouldInitiateTrade_Implementation(FName OtherFactionID)
{
    // Don't trade if we don't have faction data
    if (!FactionData)
    {
        return false;
    }

    // Check relationship
    FFactionRelationship Relationship;
    if (FactionData->GetRelationship(OtherFactionID, Relationship))
    {
        // Don't trade with enemies
        if (Relationship.bAtWar)
        {
            return false;
        }

        // Always trade with allies
        if (Relationship.bIsAllied)
        {
            return true;
        }

        // Trade with friendly factions if trade-focused
        if (IsTradeFocused() && Relationship.RelationshipValue > 0)
        {
            return true;
        }
    }

    // In early game, be more willing to trade (peaceful expansion)
    if (bIsEarlyGame && IsPeaceful())
    {
        return true;
    }

    return false;
}

bool UFactionLogic::ShouldPursuepeacefulExpansion_Implementation()
{
    // Must be in peaceful mode
    if (!IsPeaceful())
    {
        return false;
    }

    // Check if expansion is a priority
    if (bIsEarlyGame)
    {
        return EarlyGamePriorities.ExpansionPriority >= 5 ||
               EarlyGamePriorities.ExplorationPriority >= 6;
    }

    // Later game - check strategic priority
    return CurrentStrategicPriority == EFactionPriority::Expansion ||
           CurrentStrategicPriority == EFactionPriority::Exploration;
}

void UFactionLogic::UpdateStrategicGoals_Implementation()
{
    // Re-evaluate strategic priority
    EFactionPriority NewPriority = EvaluateStrategicPriority();
    if (NewPriority != CurrentStrategicPriority)
    {
        CurrentStrategicPriority = NewPriority;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                FString::Printf(TEXT("Faction %s: Strategic priority changed to %s"),
                    FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown"),
                    *UEnum::GetValueAsString(NewPriority)));
        }
    }

    // Check if we should transition out of early game
    if (bIsEarlyGame)
    {
        // Transition criteria: multiple territories or strong economic/military power
        if (TerritoryCount >= 5 || EconomicStrength >= 70.0f || MilitaryPower >= 60.0f)
        {
            bIsEarlyGame = false;
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                    FString::Printf(TEXT("Faction %s: Transitioned out of early game phase"),
                        FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown")));
            }
        }
    }
}

// ====================
// Utility Functions
// ====================

bool UFactionLogic::HasTruceWith(FName OtherFactionID) const
{
    return TruceList.Contains(OtherFactionID);
}

void UFactionLogic::AddTruce(FName OtherFactionID)
{
    if (!TruceList.Contains(OtherFactionID))
    {
        TruceList.Add(OtherFactionID);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                FString::Printf(TEXT("Faction %s: Truce established with %s"),
                    FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown"),
                    *OtherFactionID.ToString()));
        }
    }
}

void UFactionLogic::RemoveTruce(FName OtherFactionID)
{
    if (TruceList.Remove(OtherFactionID) > 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                FString::Printf(TEXT("Faction %s: Truce ended with %s"),
                    FactionData ? *FactionData->FactionName.ToString() : TEXT("Unknown"),
                    *OtherFactionID.ToString()));
        }
    }
}

bool UFactionLogic::CanPeacefullyInteract(FName OtherFactionID) const
{
    if (!FactionData)
    {
        return false;
    }

    // Can't peacefully interact if not in peaceful mode
    if (!IsPeaceful())
    {
        return false;
    }

    // Check for truce
    if (HasTruceWith(OtherFactionID))
    {
        return true;
    }

    // Check relationship
    FFactionRelationship Relationship;
    if (FactionData->GetRelationship(OtherFactionID, Relationship))
    {
        // Can't peacefully interact if at war
        if (Relationship.bAtWar)
        {
            return false;
        }

        // Can peacefully interact if allied or friendly
        if (Relationship.bIsAllied || Relationship.RelationshipValue >= 0)
        {
            return true;
        }
    }

    // Default to true for peaceful factions in early game
    return bIsEarlyGame;
}

EFactionPriority UFactionLogic::GetTopEarlyGamePriority() const
{
    // Find the highest priority from early game priorities
    int32 MaxPriority = 0;
    EFactionPriority TopPriority = EFactionPriority::Exploration;

    if (EarlyGamePriorities.ExplorationPriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.ExplorationPriority;
        TopPriority = EFactionPriority::Exploration;
    }

    if (EarlyGamePriorities.TradePriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.TradePriority;
        TopPriority = EFactionPriority::Trade;
    }

    if (EarlyGamePriorities.DiplomacyPriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.DiplomacyPriority;
        TopPriority = EFactionPriority::Diplomacy;
    }

    if (EarlyGamePriorities.ResearchPriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.ResearchPriority;
        TopPriority = EFactionPriority::Research;
    }

    if (EarlyGamePriorities.ExpansionPriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.ExpansionPriority;
        TopPriority = EFactionPriority::Expansion;
    }

    if (EarlyGamePriorities.DefensePriority > MaxPriority)
    {
        MaxPriority = EarlyGamePriorities.DefensePriority;
        TopPriority = EFactionPriority::Defense;
    }

    return TopPriority;
}

bool UFactionLogic::IsExplorationFocused() const
{
    return CurrentStrategicPriority == EFactionPriority::Exploration ||
           (bIsEarlyGame && EarlyGamePriorities.ExplorationPriority >= 6);
}

bool UFactionLogic::IsTradeFocused() const
{
    return CurrentStrategicPriority == EFactionPriority::Trade ||
           (bIsEarlyGame && EarlyGamePriorities.TradePriority >= 6);
}

bool UFactionLogic::IsDiplomacyFocused() const
{
    return CurrentStrategicPriority == EFactionPriority::Diplomacy ||
           (bIsEarlyGame && EarlyGamePriorities.DiplomacyPriority >= 6);
}

FText UFactionLogic::GetFactionName() const
{
    if (FactionData)
    {
        return FactionData->FactionName;
    }
    return FText::FromString(TEXT("Unknown Faction"));
}

FString UFactionLogic::GetFactionStateDescription() const
{
    FString BaseState = GetAIStateDescription();
    FString Priority = UEnum::GetValueAsString(CurrentStrategicPriority);
    FString Phase = bIsEarlyGame ? TEXT("Early Game") : TEXT("Mid/Late Game");
    
    return FString::Printf(TEXT("%s | %s | %s | Territories: %d"),
        *GetFactionName().ToString(),
        *BaseState,
        *Priority,
        TerritoryCount);
}

// ====================
// Overridden Functions
// ====================

void UFactionLogic::InitializeAI_Implementation()
{
    Super::InitializeAI_Implementation();

    // Set initial strategic priority
    CurrentStrategicPriority = EvaluateStrategicPriority();

    if (GEngine && FactionData)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("Faction AI Initialized: %s - Priority: %s"),
                *FactionData->FactionName.ToString(),
                *UEnum::GetValueAsString(CurrentStrategicPriority)));
    }
}

void UFactionLogic::OnTickAI_Implementation(float DeltaTime)
{
    // Update strategic goals
    UpdateStrategicGoals();

    // Call parent implementation
    Super::OnTickAI_Implementation(DeltaTime);
}

EAIPriority UFactionLogic::EvaluateCurrentPriority_Implementation()
{
    // Map strategic priority to AI priority
    switch (CurrentStrategicPriority)
    {
        case EFactionPriority::Survival:
            return EAIPriority::Critical;
        
        case EFactionPriority::Defense:
        case EFactionPriority::Conquest:
            return EAIPriority::High;
        
        case EFactionPriority::Expansion:
        case EFactionPriority::Trade:
        case EFactionPriority::Research:
            return EAIPriority::Medium;
        
        case EFactionPriority::Exploration:
        case EFactionPriority::Diplomacy:
            return EAIPriority::Low;
        
        default:
            return EAIPriority::Idle;
    }
}
