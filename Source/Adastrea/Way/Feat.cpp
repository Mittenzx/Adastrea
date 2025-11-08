#include "Way/Feat.h"
#include "AdastreaLog.h"

UFeatDataAsset::UFeatDataAsset()
{
    // Set default values for core identity
    TitleName = FText::FromString(TEXT("Unnamed Title"));
    Description = FText::FromString(TEXT("An accomplishment of note."));
    FeatID = FName(TEXT("UnknownFeat"));
    Rarity = EFeatRarity::Common;
    LegendaryDescription = FText::GetEmpty();
    
    // Requirements defaults
    bUniquePerPlaythrough = true;
    bHiddenUntilEarned = false;
    
    // Reputation defaults
    BaseReputationGain = 10;
    ReputationMultiplier = 1.0f;
}

// ====================
// Precept Query Functions
// ====================

TArray<FFeatPreceptAlignment> UFeatDataAsset::GetPreceptAlignments() const
{
    return PreceptAlignments;
}

bool UFeatDataAsset::AlignsWith(EPrecept Precept) const
{
    for (const FFeatPreceptAlignment& Alignment : PreceptAlignments)
    {
        if (Alignment.Precept == Precept)
        {
            return true;
        }
    }
    return false;
}

int32 UFeatDataAsset::GetAlignmentStrength(EPrecept Precept) const
{
    for (const FFeatPreceptAlignment& Alignment : PreceptAlignments)
    {
        if (Alignment.Precept == Precept)
        {
            return Alignment.AlignmentStrength;
        }
    }
    return 0;
}

bool UFeatDataAsset::GetPrimaryAlignment(EPrecept& OutPrecept) const
{
    if (PreceptAlignments.Num() == 0)
    {
        return false;
    }
    
    int32 HighestStrength = 0;
    EPrecept PrimaryPrecept = EPrecept::Honor;
    
    for (const FFeatPreceptAlignment& Alignment : PreceptAlignments)
    {
        if (Alignment.AlignmentStrength > HighestStrength)
        {
            HighestStrength = Alignment.AlignmentStrength;
            PrimaryPrecept = Alignment.Precept;
        }
    }
    
    OutPrecept = PrimaryPrecept;
    return true;
}

int32 UFeatDataAsset::CalculateReputationGain(const TArray<FPreceptValue>& WayPrecepts) const
{
    if (WayPrecepts.Num() == 0 || PreceptAlignments.Num() == 0)
    {
        return 0;
    }
    
    float TotalReputation = 0.0f;
    
    // For each Precept alignment in this Feat
    for (const FFeatPreceptAlignment& FeatAlignment : PreceptAlignments)
    {
        // Check if the Way values this Precept
        for (const FPreceptValue& WayPrecept : WayPrecepts)
        {
            if (WayPrecept.Precept == FeatAlignment.Precept)
            {
                // Calculate reputation based on alignment match
                // Formula: BaseGain * Multiplier * (WayValue * FeatStrength / 10000)
                float AlignmentScore = (WayPrecept.ImportanceValue * FeatAlignment.AlignmentStrength) / 10000.0f;
                TotalReputation += BaseReputationGain * ReputationMultiplier * AlignmentScore;
                break;
            }
        }
    }
    
    return FMath::RoundToInt(TotalReputation);
}

// ====================
// Requirements Functions
// ====================

bool UFeatDataAsset::CanBeEarned(const TArray<UFeatDataAsset*>& EarnedFeats) const
{
    // Check if all prerequisites are met
    for (UFeatDataAsset* PrereqFeat : PrerequisiteFeats)
    {
        if (PrereqFeat == nullptr)
        {
            continue;
        }
        
        bool bHasPrereq = false;
        for (UFeatDataAsset* EarnedFeat : EarnedFeats)
        {
            if (EarnedFeat == PrereqFeat)
            {
                bHasPrereq = true;
                break;
            }
        }
        
        if (!bHasPrereq)
        {
            return false;
        }
    }
    
    return true;
}

TArray<UFeatDataAsset*> UFeatDataAsset::GetPrerequisites() const
{
    return PrerequisiteFeats;
}

// ====================
// Helper Functions
// ====================

FText UFeatDataAsset::GetRarityDisplayName(EFeatRarity Rarity)
{
    // Return the enum display name defined in the UMETA tag
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EFeatRarity"), true);
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Rarity));
    }
    return FText::FromString(TEXT("Unknown"));
}

FLinearColor UFeatDataAsset::GetRarityColor(EFeatRarity Rarity)
{
    // Return appropriate colors for each rarity tier
    switch (Rarity)
    {
        case EFeatRarity::Common:
            return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
        case EFeatRarity::Uncommon:
            return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // Green
        case EFeatRarity::Rare:
            return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f); // Blue
        case EFeatRarity::Epic:
            return FLinearColor(0.7f, 0.3f, 0.9f, 1.0f); // Purple
        case EFeatRarity::Legendary:
            return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange/Gold
        case EFeatRarity::Mythic:
            return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red
        default:
            return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
    }
}
