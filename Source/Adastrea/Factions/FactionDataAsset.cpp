#include "Factions/FactionDataAsset.h"
#include "Way/Way.h"
#include "Way/Feat.h"
#include "AdastreaLog.h"

UFactionDataAsset::UFactionDataAsset()
    : bRelationshipCacheValid(false)
    , WayReputationSpillover(25) // Default 25% spillover from Ways to faction
    , FactionToWaySpillover(15) // Default 15% spillover from faction to Ways
    , bNetworkInfluencesDiplomacy(true) // Networks influence diplomacy by default
    , WayReputationWeight(0.3f) // Ways contribute 30% to faction standing by default
    , bDeriveTraitsFromWays(false) // Manual trait definition by default
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

void UFactionDataAsset::RebuildRelationshipCache() const
{
    RelationshipCache.Empty(FactionRelationships.Num());
    
    for (const FFactionRelationship& Relationship : FactionRelationships)
    {
        if (!Relationship.TargetFactionID.IsNone())
        {
            RelationshipCache.Add(Relationship.TargetFactionID, &Relationship);
        }
    }
    
    bRelationshipCacheValid = true;
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
    // Early exit for invalid input
    if (TraitID.IsNone())
    {
        return false;
    }

    // Use range-based for loop with const reference for better performance
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
    // Early exit for invalid input
    if (TraitID.IsNone())
    {
        return false;
    }

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
    // Early exit for invalid input
    if (TraitID.IsNone())
    {
        return 0.0f;
    }

    float TotalModifier = 0.0f;
    
    // Sum all modifiers for traits matching the given ID
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            TotalModifier += Trait.ModifierValue;
        }
    }
    
    return TotalModifier;
}

TArray<FFactionTrait> UFactionDataAsset::GetTraitsByCategory(EFactionTraitCategory Category) const
{
    TArray<FFactionTrait> CategoryTraits;
    
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.Category == Category)
        {
            CategoryTraits.Add(Trait);
        }
    }
    
    return CategoryTraits;
}

bool UFactionDataAsset::HasTraitInCategory(EFactionTraitCategory Category) const
{
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.Category == Category)
        {
            return true;
        }
    }
    return false;
}

float UFactionDataAsset::GetCategoryModifierTotal(EFactionTraitCategory Category) const
{
    float TotalModifier = 0.0f;
    
    for (const FFactionTrait& Trait : Traits)
    {
        if (Trait.Category == Category)
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
    // Early exit for invalid input
    if (OtherFactionID.IsNone())
    {
        return false;
    }

    // Build cache if needed
    if (!bRelationshipCacheValid)
    {
        RebuildRelationshipCache();
    }

    // O(1) lookup using cache
    if (const FFactionRelationship* const* FoundPtr = RelationshipCache.Find(OtherFactionID))
    {
        OutRelationship = **FoundPtr;
        return true;
    }
    
    return false;
}

bool UFactionDataAsset::IsAlliedWith(FName OtherFactionID) const
{
    // Early exit for invalid input
    if (OtherFactionID.IsNone())
    {
        return false;
    }

    // Build cache if needed
    if (!bRelationshipCacheValid)
    {
        RebuildRelationshipCache();
    }

    // O(1) lookup using cache
    if (const FFactionRelationship* const* FoundPtr = RelationshipCache.Find(OtherFactionID))
    {
        return (*FoundPtr)->bIsAllied;
    }
    
    return false;
}

bool UFactionDataAsset::IsAtWarWith(FName OtherFactionID) const
{
    // Early exit for invalid input
    if (OtherFactionID.IsNone())
    {
        return false;
    }

    // Build cache if needed
    if (!bRelationshipCacheValid)
    {
        RebuildRelationshipCache();
    }

    // O(1) lookup using cache
    if (const FFactionRelationship* const* FoundPtr = RelationshipCache.Find(OtherFactionID))
    {
        return (*FoundPtr)->bAtWar;
    }
    
    return false;
}

int32 UFactionDataAsset::GetRelationshipValue(FName OtherFactionID) const
{
    // Early exit for invalid input
    if (OtherFactionID.IsNone())
    {
        return 0;
    }

    // Build cache if needed
    if (!bRelationshipCacheValid)
    {
        RebuildRelationshipCache();
    }

    // O(1) lookup using cache
    if (const FFactionRelationship* const* FoundPtr = RelationshipCache.Find(OtherFactionID))
    {
        return (*FoundPtr)->RelationshipValue;
    }
    
    return 0; // Neutral if no relationship exists
}

float UFactionDataAsset::GetTradeModifier(FName OtherFactionID) const
{
    // Early exit for invalid input
    if (OtherFactionID.IsNone())
    {
        return 1.0f;
    }

    // Build cache if needed
    if (!bRelationshipCacheValid)
    {
        RebuildRelationshipCache();
    }

    // O(1) lookup using cache
    if (const FFactionRelationship* const* FoundPtr = RelationshipCache.Find(OtherFactionID))
    {
        return (*FoundPtr)->TradeModifier;
    }
    
    return 1.0f; // Normal trade if no relationship exists
}

// ====================
// Way System Implementation
// ====================

TArray<UWayDataAsset*> UFactionDataAsset::GetAssociatedWays() const
{
    return AssociatedWays;
}

bool UFactionDataAsset::HasAssociatedWay(const UWayDataAsset* Way) const
{
    if (!Way)
    {
        return false;
    }

    return AssociatedWays.Contains(Way);
}

int32 UFactionDataAsset::CalculateWayReputationSpillover(int32 WayReputationGain) const
{
    if (WayReputationSpillover <= 0 || WayReputationGain == 0)
    {
        return 0;
    }

    // Calculate spillover as percentage of Way reputation gain
    float Spillover = static_cast<float>(WayReputationGain) * (static_cast<float>(WayReputationSpillover) / 100.0f);
    
    return FMath::RoundToInt(Spillover);
}

int32 UFactionDataAsset::CalculateFactionToWaySpillover(int32 FactionReputationGain) const
{
    if (FactionToWaySpillover <= 0 || FactionReputationGain == 0)
    {
        return 0;
    }

    // Calculate reverse spillover from faction to Ways
    float Spillover = static_cast<float>(FactionReputationGain) * (static_cast<float>(FactionToWaySpillover) / 100.0f);
    
    return FMath::RoundToInt(Spillover);
}

float UFactionDataAsset::CalculateAggregateWayReputation(const TMap<FName, float>& WayReputationScores) const
{
    if (AssociatedWays.Num() == 0)
    {
        return 0.0f;
    }

    float TotalReputation = 0.0f;
    int32 CountedWays = 0;

    // Sum reputation from all associated Ways
    for (const UWayDataAsset* Way : AssociatedWays)
    {
        if (Way && Way->WayID != NAME_None)
        {
            if (const float* ReputationPtr = WayReputationScores.Find(Way->WayID))
            {
                TotalReputation += *ReputationPtr;
                CountedWays++;
            }
        }
    }

    // Return average reputation across all counted Ways
    return (CountedWays > 0) ? (TotalReputation / static_cast<float>(CountedWays)) : 0.0f;
}

float UFactionDataAsset::CalculateCombinedStanding(int32 BaseReputation, float WayAggregate) const
{
    // Weighted combination of base faction reputation and Way aggregate
    float BasePart = static_cast<float>(BaseReputation) * (1.0f - WayReputationWeight);
    float WayPart = WayAggregate * WayReputationWeight;
    
    return BasePart + WayPart;
}

// ====================
// Precept System Implementation
// ====================

TArray<FPreceptValue> UFactionDataAsset::GetFactionPrecepts() const
{
    return FactionPrecepts;
}

bool UFactionDataAsset::HasFactionPrecept(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : FactionPrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return true;
        }
    }
    return false;
}

int32 UFactionDataAsset::GetPreceptImportance(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : FactionPrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return PreceptValue.ImportanceValue;
        }
    }
    return 0;
}

float UFactionDataAsset::CalculatePhilosophicalAlignment(const UFactionDataAsset* OtherFaction) const
{
    if (!OtherFaction || FactionPrecepts.Num() == 0 || OtherFaction->FactionPrecepts.Num() == 0)
    {
        return 0.0f;
    }

    float AlignmentScore = 0.0f;
    int32 SharedPrecepts = 0;

    // Calculate alignment based on shared Precepts
    for (const FPreceptValue& MyPrecept : FactionPrecepts)
    {
        for (const FPreceptValue& TheirPrecept : OtherFaction->FactionPrecepts)
        {
            if (MyPrecept.Precept == TheirPrecept.Precept)
            {
                // Alignment strength based on how much both factions value the Precept
                float MyWeight = static_cast<float>(MyPrecept.ImportanceValue) / 100.0f;
                float TheirWeight = static_cast<float>(TheirPrecept.ImportanceValue) / 100.0f;
                AlignmentScore += (MyWeight + TheirWeight) / 2.0f;
                SharedPrecepts++;
                break;
            }
        }
    }

    // Normalize by number of possible Precepts considered
    int32 TotalPrecepts = FMath::Max(FactionPrecepts.Num(), OtherFaction->FactionPrecepts.Num());
    return (TotalPrecepts > 0) ? (AlignmentScore / static_cast<float>(TotalPrecepts)) : 0.0f;
}

TArray<FFactionTrait> UFactionDataAsset::GetDerivedTraitsFromWays() const
{
    TArray<FFactionTrait> DerivedTraits;

    if (!bDeriveTraitsFromWays || AssociatedWays.Num() == 0)
    {
        return DerivedTraits;
    }

    // Count Precept occurrences across all associated Ways
    TMap<EPrecept, int32> PreceptCounts;
    TMap<EPrecept, int32> PreceptTotalImportance;

    for (const UWayDataAsset* Way : AssociatedWays)
    {
        if (!Way)
        {
            continue;
        }

        for (const FPreceptValue& PreceptValue : Way->CorePrecepts)
        {
            PreceptCounts.FindOrAdd(PreceptValue.Precept)++;
            PreceptTotalImportance.FindOrAdd(PreceptValue.Precept) += PreceptValue.ImportanceValue;
        }
    }

    // Generate traits from dominant Precepts
    for (const TPair<EPrecept, int32>& PreceptPair : PreceptCounts)
    {
        // Only create trait if multiple Ways share this Precept
        if (PreceptPair.Value >= 2)
        {
            FFactionTrait DerivedTrait;
            DerivedTrait.TraitID = FName(*FString::Printf(TEXT("DerivedTrait_%s"), 
                *UEnum::GetValueAsString(PreceptPair.Key)));
            DerivedTrait.TraitName = FText::Format(
                FText::FromString(TEXT("Values {0}")),
                UWayDataAsset::GetPreceptDisplayName(PreceptPair.Key)
            );
            DerivedTrait.TraitDescription = FText::Format(
                FText::FromString(TEXT("Derived from {0} associated Ways that value this Precept")),
                FText::AsNumber(PreceptPair.Value)
            );
            DerivedTrait.Category = EFactionTraitCategory::Diplomatic;
            
            // Modifier based on average importance across Ways
            int32 AverageImportance = PreceptTotalImportance[PreceptPair.Key] / PreceptPair.Value;
            DerivedTrait.ModifierValue = static_cast<float>(AverageImportance) / 10.0f; // Scale to reasonable range
            
            DerivedTraits.Add(DerivedTrait);
        }
    }

    return DerivedTraits;
}

TArray<UWayDataAsset*> UFactionDataAsset::GetWaysByPrecept(EPrecept Precept, int32 MinimumImportance) const
{
    TArray<UWayDataAsset*> MatchingWays;

    for (UWayDataAsset* Way : AssociatedWays)
    {
        if (!Way)
        {
            continue;
        }

        // Check if Way values this Precept above threshold
        if (Way->HasPrecept(Precept))
        {
            int32 Importance = Way->GetPreceptValue(Precept);
            if (Importance >= MinimumImportance)
            {
                MatchingWays.Add(Way);
            }
        }
    }

    return MatchingWays;
}

float UFactionDataAsset::CalculateFeatReputationModifier(const TArray<FFeatPreceptAlignment>& FeatPrecepts) const
{
    if (FactionPrecepts.Num() == 0 || FeatPrecepts.Num() == 0)
    {
        return 1.0f; // Neutral modifier
    }

    float TotalModifier = 0.0f;
    int32 MatchingPrecepts = 0;

    // Calculate modifier based on Precept alignment
    for (const FFeatPreceptAlignment& FeatAlignment : FeatPrecepts)
    {
        for (const FPreceptValue& FactionPreceptValue : FactionPrecepts)
        {
            if (FeatAlignment.Precept == FactionPreceptValue.Precept)
            {
                // Multiply faction's importance by feat's alignment strength
                float AlignmentScore = (static_cast<float>(FactionPreceptValue.ImportanceValue) / 100.0f) * 
                                      (static_cast<float>(FeatAlignment.AlignmentStrength) / 100.0f);
                TotalModifier += AlignmentScore;
                MatchingPrecepts++;
                break;
            }
        }
    }

    // Convert to multiplier: 0 alignment = 0.5x, perfect alignment = 3.0x
    if (MatchingPrecepts > 0)
    {
        float NormalizedScore = TotalModifier / static_cast<float>(MatchingPrecepts);
        return 0.5f + (NormalizedScore * 2.5f);
    }

    return 1.0f; // No matching Precepts = neutral
}
