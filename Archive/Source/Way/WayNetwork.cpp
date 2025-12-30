#include "Way/WayNetwork.h"
#include "Way/Feat.h"
#include "AdastreaLog.h"

UWayNetworkDataAsset::UWayNetworkDataAsset()
{
    // Set default values for core identity
    NetworkName = FText::FromString(TEXT("Unnamed Network"));
    Description = FText::FromString(TEXT("A network of guilds with shared values and goals."));
    NetworkID = FName(TEXT("UnknownNetwork"));
    
    // Default UI color
    NetworkColor = FLinearColor(0.3f, 0.6f, 0.9f, 1.0f);
    
    // Default mechanics
    ReputationSpilloverPercent = 30;
    NetworkAlignmentBonus = 1.5f;
    MinimumReputationThreshold = 25;
    
    // Network active by default
    bIsActive = true;
    FormationDate = FDateTime::Now();
}

// ====================
// Member Query Functions
// ====================

TArray<FNetworkMember> UWayNetworkDataAsset::GetMembers() const
{
    return MemberWays;
}

bool UWayNetworkDataAsset::IsMember(const UWayDataAsset* Way) const
{
    if (!Way)
    {
        return false;
    }
    
    for (const FNetworkMember& Member : MemberWays)
    {
        if (Member.MemberWay == Way)
        {
            return true;
        }
    }
    return false;
}

int32 UWayNetworkDataAsset::GetMemberCount() const
{
    return MemberWays.Num();
}

bool UWayNetworkDataAsset::GetMostInfluentialMember(FNetworkMember& OutMember) const
{
    if (MemberWays.Num() == 0)
    {
        return false;
    }
    
    int32 HighestInfluence = 0;
    int32 MostInfluentialIndex = 0;
    
    for (int32 i = 0; i < MemberWays.Num(); ++i)
    {
        if (MemberWays[i].InfluenceLevel > HighestInfluence)
        {
            HighestInfluence = MemberWays[i].InfluenceLevel;
            MostInfluentialIndex = i;
        }
    }
    
    OutMember = MemberWays[MostInfluentialIndex];
    return true;
}

int32 UWayNetworkDataAsset::GetMemberInfluence(const UWayDataAsset* Way) const
{
    if (!Way)
    {
        return 0;
    }
    
    for (const FNetworkMember& Member : MemberWays)
    {
        if (Member.MemberWay == Way)
        {
            return Member.InfluenceLevel;
        }
    }
    return 0;
}

// ====================
// Precept Alignment Functions
// ====================

TArray<FPreceptValue> UWayNetworkDataAsset::GetSharedPrecepts() const
{
    return SharedPrecepts;
}

bool UWayNetworkDataAsset::ValuesSharedPrecept(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : SharedPrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return true;
        }
    }
    return false;
}

float UWayNetworkDataAsset::CalculateNetworkAlignment(const TArray<FFeatPreceptAlignment>& FeatAlignments) const
{
    float TotalAlignment = 0.0f;
    
    // Check each Feat alignment against network's shared Precepts
    for (const FFeatPreceptAlignment& FeatAlignment : FeatAlignments)
    {
        for (const FPreceptValue& SharedPrecept : SharedPrecepts)
        {
            if (FeatAlignment.Precept == SharedPrecept.Precept)
            {
                // Calculate alignment score
                // Formula: (FeatAlignment * SharedPrecept) / 100
                float AlignmentScore = (FeatAlignment.AlignmentStrength * SharedPrecept.ImportanceValue) / 100.0f;
                TotalAlignment += AlignmentScore;
                break;
            }
        }
    }
    
    return TotalAlignment;
}

// ====================
// Reputation Functions
// ====================

int32 UWayNetworkDataAsset::CalculateSpilloverReputation(int32 BaseReputationGain) const
{
    if (BaseReputationGain <= 0 || ReputationSpilloverPercent <= 0)
    {
        return 0;
    }
    
    // Calculate spillover amount
    int32 SpilloverAmount = (BaseReputationGain * ReputationSpilloverPercent) / 100;
    
    return SpilloverAmount;
}

bool UWayNetworkDataAsset::QualifiesForNetworkBonuses(int32 PlayerReputation) const
{
    return PlayerReputation >= MinimumReputationThreshold;
}

// ====================
// Helper Functions
// ====================

FText UWayNetworkDataAsset::GetPhilosophySummary() const
{
    if (SharedPrecepts.Num() == 0)
    {
        return FText::FromString(TEXT("No shared philosophy defined"));
    }
    
    FString Summary = TEXT("This network values: ");
    
    for (int32 i = 0; i < SharedPrecepts.Num(); ++i)
    {
        FText PreceptName = UWayDataAsset::GetPreceptDisplayName(SharedPrecepts[i].Precept);
        Summary += PreceptName.ToString();
        
        if (i < SharedPrecepts.Num() - 1)
        {
            Summary += TEXT(", ");
        }
    }
    
    return FText::FromString(Summary);
}
