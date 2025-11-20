#include "Player/Verse.h"
#include "Way/Feat.h"
#include "Way/Way.h"
#include "AdastreaLog.h"

UVerseComponent::UVerseComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize component defaults
    DisplayedTitle = nullptr;
}

// ====================
// Feat Management
// ====================

bool UVerseComponent::AwardFeat(UFeatDataAsset* Feat, FText Location, FText Context)
{
    if (Feat == nullptr)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("VerseComponent::AwardFeat - Attempted to award null Feat"));
        return false;
    }
    
    // Check if Feat is unique and already earned
    if (Feat->bUniquePerPlaythrough && HasFeat(Feat))
    {
        UE_LOG(LogAdastrea, Log, TEXT("VerseComponent::AwardFeat - Feat '%s' already earned and is unique"), 
            *Feat->TitleName.ToString());
        return false;
    }
    
    // Create new earned Feat entry
    FEarnedFeat NewEarnedFeat;
    NewEarnedFeat.Feat = Feat;
    NewEarnedFeat.EarnedTimestamp = FDateTime::Now();
    NewEarnedFeat.LocationEarned = Location;
    NewEarnedFeat.EarnedContext = Context;
    
    // Add to player's Verse
    EarnedFeats.Add(NewEarnedFeat);
    
    UE_LOG(LogAdastrea, Log, TEXT("VerseComponent::AwardFeat - Awarded Feat '%s' to player"), 
        *Feat->TitleName.ToString());
    
    return true;
}

bool UVerseComponent::HasFeat(UFeatDataAsset* Feat) const
{
    if (Feat == nullptr)
    {
        return false;
    }
    
    return FindEarnedFeat(Feat) != nullptr;
}

bool UVerseComponent::HasFeatByID(FName FeatID) const
{
    if (FeatID.IsNone())
    {
        return false;
    }
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr && EarnedFeat.Feat->FeatID == FeatID)
        {
            return true;
        }
    }
    
    return false;
}

TArray<FEarnedFeat> UVerseComponent::GetAllEarnedFeats() const
{
    return EarnedFeats;
}

bool UVerseComponent::GetEarnedFeatByIndex(int32 Index, FEarnedFeat& OutFeat) const
{
    if (Index >= 0 && Index < EarnedFeats.Num())
    {
        OutFeat = EarnedFeats[Index];
        return true;
    }
    
    return false;
}

int32 UVerseComponent::GetEarnedFeatCount() const
{
    return EarnedFeats.Num();
}

// ====================
// Display Title Management
// ====================

bool UVerseComponent::SetDisplayedTitle(UFeatDataAsset* Feat)
{
    if (Feat == nullptr)
    {
        return false;
    }
    
    // Check if player has earned this Feat
    if (!HasFeat(Feat))
    {
        UE_LOG(LogAdastrea, Warning, TEXT("VerseComponent::SetDisplayedTitle - Cannot display unearned Feat '%s'"), 
            *Feat->TitleName.ToString());
        return false;
    }
    
    DisplayedTitle = Feat;
    UE_LOG(LogAdastrea, Log, TEXT("VerseComponent::SetDisplayedTitle - Now displaying '%s'"), 
        *Feat->TitleName.ToString());
    
    return true;
}

UFeatDataAsset* UVerseComponent::GetDisplayedTitle() const
{
    return DisplayedTitle;
}

bool UVerseComponent::ClearDisplayedTitle()
{
    DisplayedTitle = nullptr;
    return true;
}

// ====================
// Precept Analysis
// ====================

int32 UVerseComponent::GetTotalPreceptAlignment(EPrecept Precept) const
{
    int32 TotalAlignment = 0;
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr)
        {
            TotalAlignment += EarnedFeat.Feat->GetAlignmentStrength(Precept);
        }
    }
    
    return TotalAlignment;
}

void UVerseComponent::GetTopAlignedPrecepts(TArray<EPrecept>& OutTopPrecepts) const
{
    OutTopPrecepts.Empty();
    
    // Calculate alignment for all Precepts
    TMap<EPrecept, int32> PreceptScores;
    
    // Iterate through all possible Precepts
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EPrecept"), true);
    if (EnumPtr)
    {
        for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i) // -1 to skip MAX
        {
            EPrecept CurrentPrecept = static_cast<EPrecept>(EnumPtr->GetValueByIndex(i));
            int32 Score = GetTotalPreceptAlignment(CurrentPrecept);
            if (Score > 0)
            {
                PreceptScores.Add(CurrentPrecept, Score);
            }
        }
    }
    
    // Sort by score and get top 3
    PreceptScores.ValueSort([](int32 A, int32 B) { return A > B; });
    
    int32 Count = 0;
    for (const auto& Pair : PreceptScores)
    {
        if (Count >= 3)
        {
            break;
        }
        OutTopPrecepts.Add(Pair.Key);
        ++Count;
    }
}

float UVerseComponent::CalculateWayCompatibility(const TArray<FPreceptValue>& WayPrecepts) const
{
    if (WayPrecepts.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalCompatibility = 0.0f;
    
    for (const FPreceptValue& WayPrecept : WayPrecepts)
    {
        int32 PlayerAlignment = GetTotalPreceptAlignment(WayPrecept.Precept);
        // Compatibility increases based on player alignment and Way's importance of the Precept
        TotalCompatibility += (PlayerAlignment * WayPrecept.ImportanceValue) / 100.0f;
    }
    
    return TotalCompatibility;
}

TArray<FEarnedFeat> UVerseComponent::GetFeatsAlignedWith(EPrecept Precept) const
{
    TArray<FEarnedFeat> AlignedFeats;
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr && EarnedFeat.Feat->AlignsWith(Precept))
        {
            AlignedFeats.Add(EarnedFeat);
        }
    }
    
    return AlignedFeats;
}

// ====================
// Reputation Integration
// ====================

int32 UVerseComponent::CalculateVerseReputation(UWayDataAsset* GroupWay) const
{
    if (GroupWay == nullptr)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("VerseComponent::CalculateVerseReputation - Null GroupWay provided"));
        return 0;
    }
    
    int32 TotalReputation = 0;
    const TArray<FPreceptValue>& WayPrecepts = GroupWay->GetPrecepts();
    
    // Calculate reputation from all earned Feats
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr)
        {
            TotalReputation += EarnedFeat.Feat->CalculateReputationGain(WayPrecepts);
        }
    }
    
    return TotalReputation;
}

// ====================
// Statistics and Queries
// ====================

int32 UVerseComponent::GetFeatCountByRarity(EFeatRarity Rarity) const
{
    int32 Count = 0;
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr && EarnedFeat.Feat->Rarity == Rarity)
        {
            ++Count;
        }
    }
    
    return Count;
}

bool UVerseComponent::GetMostRecentFeat(FEarnedFeat& OutFeat) const
{
    if (EarnedFeats.Num() == 0)
    {
        return false;
    }
    
    // Most recent is the last one in chronological order
    OutFeat = EarnedFeats.Last();
    return true;
}

TArray<FEarnedFeat> UVerseComponent::GetFeatsInTimeRange(FDateTime StartTime, FDateTime EndTime) const
{
    TArray<FEarnedFeat> FeatsInRange;
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.EarnedTimestamp >= StartTime && EarnedFeat.EarnedTimestamp <= EndTime)
        {
            FeatsInRange.Add(EarnedFeat);
        }
    }
    
    return FeatsInRange;
}

// ====================
// Serialization Support
// ====================

TArray<FName> UVerseComponent::ExportFeatIDs() const
{
    TArray<FName> FeatIDs;
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat != nullptr)
        {
            FeatIDs.Add(EarnedFeat.Feat->FeatID);
        }
    }
    
    return FeatIDs;
}

bool UVerseComponent::ImportFeatIDs(const TArray<FName>& FeatIDs, const TArray<UFeatDataAsset*>& AllFeats)
{
    if (AllFeats.Num() == 0)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("VerseComponent::ImportFeatIDs - No Feats provided for import"));
        return false;
    }
    
    // Clear existing Feats
    EarnedFeats.Empty();
    
    // Resolve each FeatID to its DataAsset and add to Verse
    for (const FName& FeatID : FeatIDs)
    {
        for (UFeatDataAsset* FeatAsset : AllFeats)
        {
            if (FeatAsset != nullptr && FeatAsset->FeatID == FeatID)
            {
                FEarnedFeat NewEarnedFeat;
                NewEarnedFeat.Feat = FeatAsset;
                NewEarnedFeat.EarnedTimestamp = FDateTime::Now(); // Lost original timestamp
                EarnedFeats.Add(NewEarnedFeat);
                break;
            }
        }
    }
    
    UE_LOG(LogAdastrea, Log, TEXT("VerseComponent::ImportFeatIDs - Imported %d Feats"), EarnedFeats.Num());
    return true;
}

// ====================
// Protected Helper Functions
// ====================

const FEarnedFeat* UVerseComponent::FindEarnedFeat(UFeatDataAsset* Feat) const
{
    if (Feat == nullptr)
    {
        return nullptr;
    }
    
    for (const FEarnedFeat& EarnedFeat : EarnedFeats)
    {
        if (EarnedFeat.Feat == Feat)
        {
            return &EarnedFeat;
        }
    }
    
    return nullptr;
}
