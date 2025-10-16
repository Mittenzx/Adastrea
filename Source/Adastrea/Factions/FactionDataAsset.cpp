#include "Factions/FactionDataAsset.h"

UFactionDataAsset::UFactionDataAsset()
{
    // Set default values
    FactionName = FText::FromString(TEXT("Unknown Faction"));
    Description = FText::FromString(TEXT("A mysterious faction operating in the galaxy."));
    PrimaryColor = FLinearColor::White;
    SecondaryColor = FLinearColor::Gray;
    InitialReputation = 0;
    bHostileByDefault = false;
    TechnologyLevel = 5;
    MilitaryStrength = 5;
    EconomicPower = 5;
    FactionID = FName(TEXT("UnknownFaction"));
}

// ====================
// Trait System Implementation
// ====================

TArray<FFactionTrait> UFactionDataAsset::GetTraits() const
{
    return Traits;
}

bool UFactionDataAsset::HasTrait(FName TraitID) const
{
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            return true;
        }
    }
    return false;
}

bool UFactionDataAsset::GetTraitByID(FName TraitID, FFactionTrait& OutTrait) const
{
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            OutTrait = Trait;
            return true;
        }
    }
    return false;
}

float UFactionDataAsset::GetTraitModifier(FName TraitID) const
{
    float TotalModifier = 0.0f;
    
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            TotalModifier += Trait.ModifierValue;
        }
    }
    
    return TotalModifier;
}

// ====================
// Diplomacy System Implementation
// ====================

bool UFactionDataAsset::GetRelationship(FName OtherFactionID, FFactionRelationship& OutRelationship) const
{
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            OutRelationship = Relationship;
            return true;
        }
    }
    return false;
}

bool UFactionDataAsset::IsAlliedWith(FName OtherFactionID) const
{
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return Relationship.bIsAllied;
        }
    }
    return false;
}

bool UFactionDataAsset::IsAtWarWith(FName OtherFactionID) const
{
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return Relationship.bAtWar;
        }
    }
    return false;
}

int32 UFactionDataAsset::GetRelationshipValue(FName OtherFactionID) const
{
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return Relationship.RelationshipValue;
        }
    }
    return 0; // Neutral if no relationship exists
}

float UFactionDataAsset::GetTradeModifier(FName OtherFactionID) const
{
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (Relationship.TargetFactionID == OtherFactionID)
        {
            return Relationship.TradeModifier;
        }
    }
    return 1.0f; // Normal trade if no relationship exists
}
