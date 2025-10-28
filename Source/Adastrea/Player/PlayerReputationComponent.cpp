#include "Player/PlayerReputationComponent.h"
#include "AdastreaLog.h"

UPlayerReputationComponent::UPlayerReputationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    DefaultReputation = 0;
    ReputationGainMultiplier = 1.0f;
}

// ====================
// Reputation Management
// ====================

void UPlayerReputationComponent::ModifyReputation(FName FactionID, int32 Delta, bool bClamp)
{
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("PlayerReputationComponent::ModifyReputation - Invalid faction ID"));
        return;
    }

    // Apply multiplier to delta
    float AdjustedDelta = Delta * ReputationGainMultiplier;
    int32 FinalDelta = FMath::RoundToInt(AdjustedDelta);

    FPlayerFactionReputation* Rep = FindOrCreateReputation(FactionID);
    if (Rep)
    {
        int32 OldValue = Rep->ReputationValue;
        Rep->ReputationValue += FinalDelta;

        if (bClamp)
        {
            Rep->ReputationValue = FMath::Clamp(Rep->ReputationValue, -100, 100);
        }

        UE_LOG(LogAdastrea, Log, TEXT("PlayerReputationComponent::ModifyReputation - %s: %d -> %d (Delta: %d)"), 
            *FactionID.ToString(), OldValue, Rep->ReputationValue, FinalDelta);
    }
}

void UPlayerReputationComponent::SetReputation(FName FactionID, int32 NewValue)
{
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("PlayerReputationComponent::SetReputation - Invalid faction ID"));
        return;
    }

    FPlayerFactionReputation* Rep = FindOrCreateReputation(FactionID);
    if (Rep)
    {
        Rep->ReputationValue = FMath::Clamp(NewValue, -100, 100);
        
        UE_LOG(LogAdastrea, Log, TEXT("PlayerReputationComponent::SetReputation - %s set to %d"), 
            *FactionID.ToString(), Rep->ReputationValue);
    }
}

int32 UPlayerReputationComponent::GetReputation(FName FactionID) const
{
    if (FactionID.IsNone())
    {
        return DefaultReputation;
    }

    const FPlayerFactionReputation* Rep = FindReputation(FactionID);
    return Rep ? Rep->ReputationValue : DefaultReputation;
}

EReputationTier UPlayerReputationComponent::GetReputationTier(FName FactionID) const
{
    int32 RepValue = GetReputation(FactionID);
    return ValueToTier(RepValue);
}

bool UPlayerReputationComponent::IsReputationAtLeast(FName FactionID, int32 Threshold) const
{
    return GetReputation(FactionID) >= Threshold;
}

TArray<FPlayerFactionReputation> UPlayerReputationComponent::GetAllReputations() const
{
    return FactionReputations;
}

// ====================
// Gameplay Effects
// ====================

float UPlayerReputationComponent::GetTradePriceModifier(FName FactionID) const
{
    int32 Rep = GetReputation(FactionID);
    
    // Linear interpolation from reputation to price modifier
    // -100 rep = 1.5x price (50% markup)
    // 0 rep = 1.0x price (normal)
    // 100 rep = 0.7x price (30% discount)
    
    if (Rep >= 0)
    {
        // Positive reputation gives discounts
        float Discount = (Rep / 100.0f) * 0.3f; // Up to 30% off at max rep
        return 1.0f - Discount;
    }
    else
    {
        // Negative reputation adds markup
        float Markup = (FMath::Abs(Rep) / 100.0f) * 0.5f; // Up to 50% markup at min rep
        return 1.0f + Markup;
    }
}

bool UPlayerReputationComponent::ShouldBeHostile(FName FactionID) const
{
    // NPCs are hostile if reputation is below -25 (Unfriendly or worse)
    return GetReputation(FactionID) < -25;
}

bool UPlayerReputationComponent::CanAccessRestrictedAreas(FName FactionID) const
{
    // Require at least Friendly reputation (26+) for restricted access
    return GetReputation(FactionID) >= 26;
}

int32 UPlayerReputationComponent::GetMissionQualityLevel(FName FactionID) const
{
    int32 Rep = GetReputation(FactionID);
    
    // Map reputation to mission quality level (0-5)
    // -100 to -51: Level 0 (worst missions)
    // -50 to -1:   Level 1
    // 0 to 25:     Level 2 (neutral, average missions)
    // 26 to 50:    Level 3
    // 51 to 75:    Level 4
    // 76 to 100:   Level 5 (best missions, highest rewards)
    
    if (Rep < -50)
    {
        return 0;
    }
    else if (Rep < 0)
    {
        return 1;
    }
    else if (Rep < 26)
    {
        return 2;
    }
    else if (Rep < 51)
    {
        return 3;
    }
    else if (Rep < 76)
    {
        return 4;
    }
    else
    {
        return 5;
    }
}

// ====================
// Mission Tracking
// ====================

void UPlayerReputationComponent::RecordMissionCompleted(FName FactionID)
{
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("PlayerReputationComponent::RecordMissionCompleted - Invalid faction ID"));
        return;
    }

    FPlayerFactionReputation* Rep = FindOrCreateReputation(FactionID);
    if (Rep)
    {
        Rep->MissionsCompleted++;
        
        UE_LOG(LogAdastrea, Log, TEXT("PlayerReputationComponent::RecordMissionCompleted - %s: %d missions completed"), 
            *FactionID.ToString(), Rep->MissionsCompleted);
    }
}

int32 UPlayerReputationComponent::GetMissionsCompleted(FName FactionID) const
{
    const FPlayerFactionReputation* Rep = FindReputation(FactionID);
    return Rep ? Rep->MissionsCompleted : 0;
}

void UPlayerReputationComponent::MarkStorylineCompleted(FName FactionID)
{
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("PlayerReputationComponent::MarkStorylineCompleted - Invalid faction ID"));
        return;
    }

    FPlayerFactionReputation* Rep = FindOrCreateReputation(FactionID);
    if (Rep)
    {
        Rep->bStorylineCompleted = true;
        
        UE_LOG(LogAdastrea, Log, TEXT("PlayerReputationComponent::MarkStorylineCompleted - %s storyline marked complete"), 
            *FactionID.ToString());
    }
}

bool UPlayerReputationComponent::IsStorylineCompleted(FName FactionID) const
{
    const FPlayerFactionReputation* Rep = FindReputation(FactionID);
    return Rep ? Rep->bStorylineCompleted : false;
}

// ====================
// Helper Functions
// ====================

EReputationTier UPlayerReputationComponent::ValueToTier(int32 ReputationValue)
{
    if (ReputationValue >= 76)
    {
        return EReputationTier::Revered;
    }
    else if (ReputationValue >= 51)
    {
        return EReputationTier::Honored;
    }
    else if (ReputationValue >= 26)
    {
        return EReputationTier::Friendly;
    }
    else if (ReputationValue >= -25)
    {
        return EReputationTier::Neutral;
    }
    else if (ReputationValue >= -50)
    {
        return EReputationTier::Unfriendly;
    }
    else if (ReputationValue >= -75)
    {
        return EReputationTier::Hostile;
    }
    else
    {
        return EReputationTier::Hated;
    }
}

FText UPlayerReputationComponent::GetTierDisplayName(EReputationTier Tier)
{
    switch (Tier)
    {
        case EReputationTier::Hated:
            return FText::FromString(TEXT("Hated"));
        case EReputationTier::Hostile:
            return FText::FromString(TEXT("Hostile"));
        case EReputationTier::Unfriendly:
            return FText::FromString(TEXT("Unfriendly"));
        case EReputationTier::Neutral:
            return FText::FromString(TEXT("Neutral"));
        case EReputationTier::Friendly:
            return FText::FromString(TEXT("Friendly"));
        case EReputationTier::Honored:
            return FText::FromString(TEXT("Honored"));
        case EReputationTier::Revered:
            return FText::FromString(TEXT("Revered"));
        default:
            return FText::FromString(TEXT("Unknown"));
    }
}

FPlayerFactionReputation* UPlayerReputationComponent::FindOrCreateReputation(FName FactionID)
{
    if (FactionID.IsNone())
    {
        return nullptr;
    }

    // Try to find existing reputation
    for (FPlayerFactionReputation& Rep : FactionReputations)
    {
        if (Rep.FactionID == FactionID)
        {
            return &Rep;
        }
    }

    // Create new reputation if not found
    FPlayerFactionReputation NewRep;
    NewRep.FactionID = FactionID;
    NewRep.ReputationValue = DefaultReputation;
    NewRep.bStorylineCompleted = false;
    NewRep.MissionsCompleted = 0;

    FactionReputations.Add(NewRep);
    return &FactionReputations.Last();
}

const FPlayerFactionReputation* UPlayerReputationComponent::FindReputation(FName FactionID) const
{
    if (FactionID.IsNone())
    {
        return nullptr;
    }

    for (const FPlayerFactionReputation& Rep : FactionReputations)
    {
        if (Rep.FactionID == FactionID)
        {
            return &Rep;
        }
    }

    return nullptr;
}
