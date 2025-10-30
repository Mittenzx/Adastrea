#include "Factions/FactionRuntimeState.h"
#include "AdastreaLog.h"

UFactionRuntimeState::UFactionRuntimeState()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    FactionTemplate = nullptr;
    PlayerReputation = 0;
    bAtWarWithPlayer = false;
    bAlliedWithPlayer = false;
}

// ====================
// Initialization
// ====================

void UFactionRuntimeState::InitializeFromTemplate(UFactionDataAsset* InFactionTemplate)
{
    if (!InFactionTemplate)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::InitializeFromTemplate - Invalid template provided"));
        return;
    }

    FactionTemplate = InFactionTemplate;
    
    // Copy initial reputation from template
    PlayerReputation = FactionTemplate->InitialReputation;
    
    // Copy relationships from template as starting point
    DynamicRelationships = FactionTemplate->FactionRelationships;
    
    // Clear runtime data
    RuntimeTraits.Empty();
    ControlledSectors.Empty();
    
    // Set initial diplomatic state with player
    bAtWarWithPlayer = FactionTemplate->bHostileByDefault;
    bAlliedWithPlayer = false;

    UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::InitializeFromTemplate - Initialized from %s"), 
        *FactionTemplate->FactionName.ToString());
}

// ====================
// Reputation System
// ====================

void UFactionRuntimeState::ModifyPlayerReputation(int32 Delta, bool bClamp)
{
    PlayerReputation += Delta;
    
    if (bClamp)
    {
        PlayerReputation = FMath::Clamp(PlayerReputation, -100, 100);
    }

    UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::ModifyPlayerReputation - Changed by %d, now %d"), 
        Delta, PlayerReputation);
}

void UFactionRuntimeState::SetPlayerReputation(int32 NewReputation)
{
    PlayerReputation = FMath::Clamp(NewReputation, -100, 100);
    
    UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::SetPlayerReputation - Set to %d"), PlayerReputation);
}

int32 UFactionRuntimeState::GetPlayerReputation() const
{
    return PlayerReputation;
}

bool UFactionRuntimeState::IsReputationAtLeast(int32 Threshold) const
{
    return PlayerReputation >= Threshold;
}

// ====================
// Dynamic Relationships
// ====================

void UFactionRuntimeState::UpdateFactionRelationship(FName OtherFactionID, int32 NewValue)
{
    if (OtherFactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::UpdateFactionRelationship - Invalid faction ID"));
        return;
    }

    FFactionRelationship* Relationship = FindOrCreateRelationship(OtherFactionID);
    if (Relationship)
    {
        Relationship->RelationshipValue = FMath::Clamp(NewValue, -100, 100);
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::UpdateFactionRelationship - Updated relationship with %s to %d"), 
            *OtherFactionID.ToString(), NewValue);
    }
}

int32 UFactionRuntimeState::GetFactionRelationshipValue(FName OtherFactionID) const
{
    if (OtherFactionID.IsNone())
    {
        return 0;
    }

    // Check dynamic relationships first
    for (const FFactionRelationship& Relationship : DynamicRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return Relationship.RelationshipValue;
        }
    }

    // Fall back to template if not found in runtime
    if (FactionTemplate)
    {
        return FactionTemplate->GetRelationshipValue(OtherFactionID);
    }

    return 0;
}

void UFactionRuntimeState::DeclareWar(FName OtherFactionID)
{
    if (OtherFactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::DeclareWar - Invalid faction ID"));
        return;
    }

    FFactionRelationship* Relationship = FindOrCreateRelationship(OtherFactionID);
    if (Relationship)
    {
        Relationship->bAtWar = true;
        Relationship->bIsAllied = false; // Can't be at war and allied
        Relationship->RelationshipValue = FMath::Min(Relationship->RelationshipValue, -50); // War implies hostile
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::DeclareWar - Declared war on %s"), 
            *OtherFactionID.ToString());
    }
}

void UFactionRuntimeState::MakePeace(FName OtherFactionID)
{
    if (OtherFactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::MakePeace - Invalid faction ID"));
        return;
    }

    FFactionRelationship* Relationship = FindOrCreateRelationship(OtherFactionID);
    if (Relationship)
    {
        Relationship->bAtWar = false;
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::MakePeace - Made peace with %s"), 
            *OtherFactionID.ToString());
    }
}

void UFactionRuntimeState::FormAlliance(FName OtherFactionID)
{
    if (OtherFactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::FormAlliance - Invalid faction ID"));
        return;
    }

    FFactionRelationship* Relationship = FindOrCreateRelationship(OtherFactionID);
    if (Relationship)
    {
        Relationship->bIsAllied = true;
        Relationship->bAtWar = false; // Can't be allied and at war
        Relationship->RelationshipValue = FMath::Max(Relationship->RelationshipValue, 50); // Alliance implies friendly
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::FormAlliance - Formed alliance with %s"), 
            *OtherFactionID.ToString());
    }
}

void UFactionRuntimeState::BreakAlliance(FName OtherFactionID)
{
    if (OtherFactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::BreakAlliance - Invalid faction ID"));
        return;
    }

    FFactionRelationship* Relationship = FindOrCreateRelationship(OtherFactionID);
    if (Relationship)
    {
        Relationship->bIsAllied = false;
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::BreakAlliance - Broke alliance with %s"), 
            *OtherFactionID.ToString());
    }
}

// ====================
// Runtime Traits
// ====================

void UFactionRuntimeState::AddRuntimeTrait(const FFactionTrait& NewTrait)
{
    if (NewTrait.TraitID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::AddRuntimeTrait - Invalid trait ID"));
        return;
    }

    // Check if trait already exists
    if (HasActiveTrait(NewTrait.TraitID))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::AddRuntimeTrait - Trait %s already exists"), 
            *NewTrait.TraitID.ToString());
        return;
    }

    RuntimeTraits.Add(NewTrait);
    
    UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::AddRuntimeTrait - Added trait %s"), 
        *NewTrait.TraitID.ToString());
}

bool UFactionRuntimeState::RemoveRuntimeTrait(FName TraitID)
{
    if (TraitID.IsNone())
    {
        return false;
    }

    int32 RemovedCount = RuntimeTraits.RemoveAll([TraitID](const FFactionTrait& Trait)
    {
        return Trait.TraitID == TraitID;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::RemoveRuntimeTrait - Removed trait %s"), 
            *TraitID.ToString());
        return true;
    }

    return false;
}

TArray<FFactionTrait> UFactionRuntimeState::GetAllActiveTraits() const
{
    TArray<FFactionTrait> AllTraits;

    // Add template traits
    if (FactionTemplate)
    {
        AllTraits.Append(FactionTemplate->GetTraits());
    }

    // Add runtime traits
    AllTraits.Append(RuntimeTraits);

    return AllTraits;
}

bool UFactionRuntimeState::HasActiveTrait(FName TraitID) const
{
    if (TraitID.IsNone())
    {
        return false;
    }

    // Check runtime traits
    for (const FFactionTrait& Trait : RuntimeTraits)
    {
        if (Trait.TraitID == TraitID)
        {
            return true;
        }
    }

    // Check template traits
    if (FactionTemplate)
    {
        return FactionTemplate->HasTrait(TraitID);
    }

    return false;
}

// ====================
// Territory Management
// ====================

void UFactionRuntimeState::AddControlledSector(FName SectorID)
{
    if (SectorID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("FactionRuntimeState::AddControlledSector - Invalid sector ID"));
        return;
    }

    if (!ControlledSectors.Contains(SectorID))
    {
        ControlledSectors.Add(SectorID);
        
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::AddControlledSector - Added sector %s"), 
            *SectorID.ToString());
    }
}

bool UFactionRuntimeState::RemoveControlledSector(FName SectorID)
{
    if (SectorID.IsNone())
    {
        return false;
    }

    int32 RemovedCount = ControlledSectors.Remove(SectorID);
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("FactionRuntimeState::RemoveControlledSector - Removed sector %s"), 
            *SectorID.ToString());
        return true;
    }

    return false;
}

bool UFactionRuntimeState::ControlsSector(FName SectorID) const
{
    if (SectorID.IsNone())
    {
        return false;
    }

    return ControlledSectors.Contains(SectorID);
}

int32 UFactionRuntimeState::GetControlledSectorCount() const
{
    return ControlledSectors.Num();
}

// ====================
// Helper Functions
// ====================

FFactionRelationship* UFactionRuntimeState::FindOrCreateRelationship(FName OtherFactionID)
{
    if (OtherFactionID.IsNone())
    {
        return nullptr;
    }

    // Try to find existing relationship
    for (FFactionRelationship& Relationship : DynamicRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return &Relationship;
        }
    }

    // Create new relationship if not found
    FFactionRelationship NewRelationship;
    NewRelationship.TargetFactionID = OtherFactionID;
    NewRelationship.RelationshipValue = 0;
    NewRelationship.bIsAllied = false;
    NewRelationship.bAtWar = false;
    NewRelationship.TradeModifier = 1.0f;

    DynamicRelationships.Add(NewRelationship);
    return &DynamicRelationships.Last();
}
