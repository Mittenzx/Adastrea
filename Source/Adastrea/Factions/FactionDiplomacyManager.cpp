#include "Factions/FactionDiplomacyManager.h"
#include "Factions/FactionRuntimeState.h"
#include "AdastreaLog.h"

UFactionDiplomacyManager::UFactionDiplomacyManager()
{
}

void UFactionDiplomacyManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    RegisteredFactions.Empty();
    
    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager initialized"));
}

void UFactionDiplomacyManager::Deinitialize()
{
    RegisteredFactions.Empty();
    
    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager deinitialized"));
    
    Super::Deinitialize();
}

// ====================
// Faction Registration
// ====================

void UFactionDiplomacyManager::RegisterFaction(FName FactionID, UFactionRuntimeState* RuntimeState)
{
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::RegisterFaction - Invalid faction ID"));
        return;
    }

    if (!RuntimeState)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::RegisterFaction - Invalid runtime state for %s"), 
            *FactionID.ToString());
        return;
    }

    if (RegisteredFactions.Contains(FactionID))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::RegisterFaction - Faction %s already registered"), 
            *FactionID.ToString());
        return;
    }

    RegisteredFactions.Add(FactionID, RuntimeState);
    
    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::RegisterFaction - Registered faction %s"), 
        *FactionID.ToString());
}

void UFactionDiplomacyManager::UnregisterFaction(FName FactionID)
{
    if (FactionID.IsNone())
    {
        return;
    }

    if (RegisteredFactions.Remove(FactionID) > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::UnregisterFaction - Unregistered faction %s"), 
            *FactionID.ToString());
    }
}

UFactionRuntimeState* UFactionDiplomacyManager::GetFactionState(FName FactionID) const
{
    if (FactionID.IsNone())
    {
        return nullptr;
    }

    UFactionRuntimeState* const* FoundState = RegisteredFactions.Find(FactionID);
    return FoundState ? *FoundState : nullptr;
}

bool UFactionDiplomacyManager::IsFactionRegistered(FName FactionID) const
{
    return !FactionID.IsNone() && RegisteredFactions.Contains(FactionID);
}

// ====================
// Diplomatic Actions
// ====================

void UFactionDiplomacyManager::DeclareWar(FName FactionA, FName FactionB, bool bBroadcastEvent)
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::DeclareWar - Invalid faction IDs"));
        return;
    }

    ApplyBidirectionalAction(FactionA, FactionB, [](UFactionRuntimeState* State, FName OtherFaction)
    {
        State->DeclareWar(OtherFaction);
    });

    if (bBroadcastEvent)
    {
        FDiplomaticEventData EventData = CreateEventData(
            EDiplomaticEvent::WarDeclared,
            FactionA,
            FactionB,
            0,
            FText::FromString(FString::Printf(TEXT("%s declared war on %s"), 
                *FactionA.ToString(), *FactionB.ToString()))
        );
        BroadcastDiplomaticEvent(EventData);
    }

    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::DeclareWar - %s and %s are now at war"), 
        *FactionA.ToString(), *FactionB.ToString());
}

void UFactionDiplomacyManager::MakePeace(FName FactionA, FName FactionB, bool bBroadcastEvent)
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::MakePeace - Invalid faction IDs"));
        return;
    }

    ApplyBidirectionalAction(FactionA, FactionB, [](UFactionRuntimeState* State, FName OtherFaction)
    {
        State->MakePeace(OtherFaction);
    });

    if (bBroadcastEvent)
    {
        FDiplomaticEventData EventData = CreateEventData(
            EDiplomaticEvent::PeaceMade,
            FactionA,
            FactionB,
            0,
            FText::FromString(FString::Printf(TEXT("%s and %s have made peace"), 
                *FactionA.ToString(), *FactionB.ToString()))
        );
        BroadcastDiplomaticEvent(EventData);
    }

    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::MakePeace - %s and %s are now at peace"), 
        *FactionA.ToString(), *FactionB.ToString());
}

void UFactionDiplomacyManager::FormAlliance(FName FactionA, FName FactionB, bool bBroadcastEvent)
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::FormAlliance - Invalid faction IDs"));
        return;
    }

    ApplyBidirectionalAction(FactionA, FactionB, [](UFactionRuntimeState* State, FName OtherFaction)
    {
        State->FormAlliance(OtherFaction);
    });

    if (bBroadcastEvent)
    {
        FDiplomaticEventData EventData = CreateEventData(
            EDiplomaticEvent::AllianceFormed,
            FactionA,
            FactionB,
            0,
            FText::FromString(FString::Printf(TEXT("%s and %s have formed an alliance"), 
                *FactionA.ToString(), *FactionB.ToString()))
        );
        BroadcastDiplomaticEvent(EventData);
    }

    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::FormAlliance - %s and %s are now allied"), 
        *FactionA.ToString(), *FactionB.ToString());
}

void UFactionDiplomacyManager::BreakAlliance(FName FactionA, FName FactionB, bool bBroadcastEvent)
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::BreakAlliance - Invalid faction IDs"));
        return;
    }

    ApplyBidirectionalAction(FactionA, FactionB, [](UFactionRuntimeState* State, FName OtherFaction)
    {
        State->BreakAlliance(OtherFaction);
    });

    if (bBroadcastEvent)
    {
        FDiplomaticEventData EventData = CreateEventData(
            EDiplomaticEvent::AllianceBroken,
            FactionA,
            FactionB,
            0,
            FText::FromString(FString::Printf(TEXT("%s and %s have broken their alliance"), 
                *FactionA.ToString(), *FactionB.ToString()))
        );
        BroadcastDiplomaticEvent(EventData);
    }

    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::BreakAlliance - %s and %s alliance broken"), 
        *FactionA.ToString(), *FactionB.ToString());
}

void UFactionDiplomacyManager::ModifyRelationship(FName FactionA, FName FactionB, int32 Delta, bool bBroadcastEvent)
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionDiplomacyManager::ModifyRelationship - Invalid faction IDs"));
        return;
    }

    UFactionRuntimeState* StateA = GetFactionState(FactionA);
    UFactionRuntimeState* StateB = GetFactionState(FactionB);

    if (StateA && StateB)
    {
        int32 OldValue = StateA->GetFactionRelationshipValue(FactionB);
        int32 NewValue = FMath::Clamp(OldValue + Delta, -100, 100);

        StateA->UpdateFactionRelationship(FactionB, NewValue);
        StateB->UpdateFactionRelationship(FactionA, NewValue);

        if (bBroadcastEvent)
        {
            EDiplomaticEvent EventType = Delta > 0 ? 
                EDiplomaticEvent::ReputationIncreased : 
                EDiplomaticEvent::ReputationDecreased;

            FDiplomaticEventData EventData = CreateEventData(
                EventType,
                FactionA,
                FactionB,
                Delta,
                FText::FromString(FString::Printf(TEXT("%s relationship with %s changed by %d"), 
                    *FactionA.ToString(), *FactionB.ToString(), Delta))
            );
            BroadcastDiplomaticEvent(EventData);
        }

        UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::ModifyRelationship - %s <-> %s: %d -> %d (Delta: %d)"), 
            *FactionA.ToString(), *FactionB.ToString(), OldValue, NewValue, Delta);
    }
}

// ====================
// Diplomatic Queries
// ====================

bool UFactionDiplomacyManager::AreFactionsAtWar(FName FactionA, FName FactionB) const
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        return false;
    }

    UFactionRuntimeState* StateA = GetFactionState(FactionA);
    if (StateA)
    {
        FFactionRelationship Relationship;
        if (StateA->GetFactionState()->GetRelationship(FactionB, Relationship))
        {
            return Relationship.bAtWar;
        }

        // Check dynamic relationships
        for (const FFactionRelationship& DynRel : StateA->DynamicRelationships)
        {
            if (DynRel.TargetFactionID == FactionB)
            {
                return DynRel.bAtWar;
            }
        }
    }

    return false;
}

bool UFactionDiplomacyManager::AreFactionsAllied(FName FactionA, FName FactionB) const
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        return false;
    }

    UFactionRuntimeState* StateA = GetFactionState(FactionA);
    if (StateA)
    {
        FFactionRelationship Relationship;
        if (StateA->FactionTemplate && StateA->FactionTemplate->GetRelationship(FactionB, Relationship))
        {
            return Relationship.bIsAllied;
        }

        // Check dynamic relationships
        for (const FFactionRelationship& DynRel : StateA->DynamicRelationships)
        {
            if (DynRel.TargetFactionID == FactionB)
            {
                return DynRel.bIsAllied;
            }
        }
    }

    return false;
}

int32 UFactionDiplomacyManager::GetRelationshipValue(FName FactionA, FName FactionB) const
{
    if (FactionA.IsNone() || FactionB.IsNone())
    {
        return 0;
    }

    UFactionRuntimeState* StateA = GetFactionState(FactionA);
    if (StateA)
    {
        return StateA->GetFactionRelationshipValue(FactionB);
    }

    return 0;
}

TArray<FName> UFactionDiplomacyManager::GetWarringFactions(FName FactionID) const
{
    TArray<FName> WarringFactions;

    if (FactionID.IsNone())
    {
        return WarringFactions;
    }

    UFactionRuntimeState* State = GetFactionState(FactionID);
    if (State)
    {
        for (const FFactionRelationship& Relationship : State->DynamicRelationships)
        {
            if (Relationship.bAtWar)
            {
                WarringFactions.Add(Relationship.TargetFactionID);
            }
        }
    }

    return WarringFactions;
}

TArray<FName> UFactionDiplomacyManager::GetAlliedFactions(FName FactionID) const
{
    TArray<FName> AlliedFactions;

    if (FactionID.IsNone())
    {
        return AlliedFactions;
    }

    UFactionRuntimeState* State = GetFactionState(FactionID);
    if (State)
    {
        for (const FFactionRelationship& Relationship : State->DynamicRelationships)
        {
            if (Relationship.bIsAllied)
            {
                AlliedFactions.Add(Relationship.TargetFactionID);
            }
        }
    }

    return AlliedFactions;
}

// ====================
// Event System
// ====================

void UFactionDiplomacyManager::BroadcastDiplomaticEvent(const FDiplomaticEventData& EventData)
{
    // TODO: Implement event listener system when needed
    // For now, just log the event
    UE_LOG(LogAdastrea, Log, TEXT("FactionDiplomacyManager::BroadcastDiplomaticEvent - %s"), 
        *EventData.Description.ToString());
}

// ====================
// Helper Functions
// ====================

void UFactionDiplomacyManager::ApplyBidirectionalAction(FName FactionA, FName FactionB, 
    TFunction<void(UFactionRuntimeState*, FName)> Action)
{
    UFactionRuntimeState* StateA = GetFactionState(FactionA);
    UFactionRuntimeState* StateB = GetFactionState(FactionB);

    if (StateA)
    {
        Action(StateA, FactionB);
    }

    if (StateB)
    {
        Action(StateB, FactionA);
    }
}

FDiplomaticEventData UFactionDiplomacyManager::CreateEventData(EDiplomaticEvent EventType, 
    FName FactionA, FName FactionB, int32 Value, const FText& Description) const
{
    FDiplomaticEventData EventData;
    EventData.EventType = EventType;
    EventData.FactionA = FactionA;
    EventData.FactionB = FactionB;
    EventData.Value = Value;
    EventData.Description = Description;
    return EventData;
}
