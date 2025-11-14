#include "Way/VerseSubsystem.h"
#include "Way/Feat.h"
#include "Way/Way.h"
#include "Way/WayNetwork.h"
#include "AdastreaLog.h"

void UVerseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadVerseState();
    UE_LOG(LogAdastreaWay, Log, TEXT("Verse Subsystem Initialized."));
}

void UVerseSubsystem::Deinitialize()
{
    CompletedFeats.Empty();
    Super::Deinitialize();
}

void UVerseSubsystem::RecordFeat(const UFeatDataAsset* FeatToRecord)
{
    if (!FeatToRecord)
    {
        UE_LOG(LogAdastreaWay, Warning, TEXT("RecordFeat called with a null FeatDataAsset."));
        return;
    }

    bool bAlreadyCompleted;
    CompletedFeats.Add(FeatToRecord, &bAlreadyCompleted);

    if (!bAlreadyCompleted)
    {
        UE_LOG(LogAdastreaWay, Log, TEXT("New Feat Recorded: '%s' (Title: %s)"), *FeatToRecord->FeatID.ToString(), *FeatToRecord->TitleName.ToString());
        // In a full game, you would likely want to broadcast a delegate here to notify the UI.
        // OnFeatRecorded.Broadcast(FeatToRecord);
    }
}

float UVerseSubsystem::GetVerseScore(const UWayDataAsset* TargetWay) const
{
    if (!TargetWay)
    {
        UE_LOG(LogAdastreaWay, Warning, TEXT("GetVerseScore called with a null WayDataAsset."));
        return 0.0f;
    }

    float TotalScore = 0.0f;

    // Iterate through every Feat the player has completed.
    for (const UFeatDataAsset* CompletedFeat : CompletedFeats)
    {
        if (!CompletedFeat) continue;

        // Get the Precepts valued by the Way
        const TArray<FPreceptValue>& WayPrecepts = TargetWay->CorePrecepts;
        
        // Get the Precept alignments of the Feat
        const TArray<FFeatPreceptAlignment>& FeatAlignments = CompletedFeat->PreceptAlignments;

        // Check for alignment between Feat Precepts and Way Precepts
        for (const FFeatPreceptAlignment& FeatAlignment : FeatAlignments)
        {
            // Find if the Way values this Precept
            for (const FPreceptValue& WayPrecept : WayPrecepts)
            {
                if (FeatAlignment.Precept == WayPrecept.Precept)
                {
                    // Calculate the contribution to Verse score
                    // Formula: (FeatAlignment.AlignmentStrength * WayPrecept.ImportanceValue) / 100
                    // This normalizes the score based on how strongly the Feat represents the Precept
                    // and how much the Way values that Precept
                    float AlignmentScore = (FeatAlignment.AlignmentStrength * WayPrecept.ImportanceValue) / 100.0f;
                    TotalScore += AlignmentScore;
                    break; // Move to next Feat alignment
                }
            }
        }
    }

    return TotalScore;
}

EReputationLevel UVerseSubsystem::GetReputationLevel(const UWayDataAsset* TargetWay) const
{
    const float Score = GetVerseScore(TargetWay);

    // These thresholds can be tweaked to balance the game's progression.
    if (Score >= 75.0f)
    {
        return EReputationLevel::Trusted;
    }
    if (Score >= 25.0f)
    {
        return EReputationLevel::Respected;
    }
    if (Score > -25.0f) // Note: Greater than, so -25 is Neutral.
    {
        return EReputationLevel::Neutral;
    }

    return EReputationLevel::Distrusted;
}

bool UVerseSubsystem::HasCompletedFeat(const UFeatDataAsset* FeatToCheck) const
{
    if (!FeatToCheck)
    {
        return false;
    }
    return CompletedFeats.Contains(FeatToCheck);
}

void UVerseSubsystem::LoadVerseState()
{
    // --- FAKE IMPLEMENTATION FOR TESTING ---
    // In a real game, this function would load a UAdastreaSaveGame object
    // and populate the CompletedFeats set from its data.
    
    // For now, we will clear any existing feats to ensure a clean state on start.
    CompletedFeats.Empty();
    RegisteredNetworks.Empty();
    UE_LOG(LogAdastreaWay, Log, TEXT("Verse state loaded (cleared for new session). Player has %d completed Feats."), CompletedFeats.Num());
}

// ====================
// Network Functions
// ====================

void UVerseSubsystem::RegisterNetwork(UWayNetworkDataAsset* Network)
{
    if (!Network)
    {
        UE_LOG(LogAdastreaWay, Warning, TEXT("RegisterNetwork called with null Network"));
        return;
    }

    if (!RegisteredNetworks.Contains(Network))
    {
        RegisteredNetworks.Add(Network);
        UE_LOG(LogAdastreaWay, Log, TEXT("Registered Way Network: %s with %d members"), 
            *Network->NetworkName.ToString(), Network->GetMemberCount());
    }
}

void UVerseSubsystem::UnregisterNetwork(UWayNetworkDataAsset* Network)
{
    if (!Network)
    {
        UE_LOG(LogAdastreaWay, Warning, TEXT("UnregisterNetwork called with null Network"));
        return;
    }

    if (RegisteredNetworks.Remove(Network) > 0)
    {
        UE_LOG(LogAdastreaWay, Log, TEXT("Unregistered Way Network: %s"), *Network->NetworkName.ToString());
    }
}

TArray<UWayNetworkDataAsset*> UVerseSubsystem::GetNetworksForWay(const UWayDataAsset* Way) const
{
    TArray<UWayNetworkDataAsset*> Networks;
    
    if (!Way)
    {
        return Networks;
    }

    for (UWayNetworkDataAsset* Network : RegisteredNetworks)
    {
        if (Network && Network->bIsActive && Network->IsMember(Way))
        {
            Networks.Add(Network);
        }
    }

    return Networks;
}

float UVerseSubsystem::GetNetworkVerseScore(const UWayNetworkDataAsset* Network) const
{
    if (!Network || !Network->bIsActive)
    {
        return 0.0f;
    }

    float TotalScore = 0.0f;
    int32 MemberCount = 0;

    // Calculate average Verse score across all network members
    for (const FNetworkMember& Member : Network->GetMembers())
    {
        if (Member.MemberWay)
        {
            float MemberScore = GetVerseScore(Member.MemberWay);
            
            // Weight by member's influence in the network
            float WeightedScore = MemberScore * (Member.InfluenceLevel / 100.0f);
            TotalScore += WeightedScore;
            MemberCount++;
        }
    }

    // Return weighted average
    if (MemberCount > 0)
    {
        return TotalScore / MemberCount;
    }

    return 0.0f;
}

bool UVerseSubsystem::QualifiesForNetworkBonuses(const UWayNetworkDataAsset* Network) const
{
    if (!Network || !Network->bIsActive)
    {
        return false;
    }

    // Check if player has minimum reputation with any network member
    for (const FNetworkMember& Member : Network->GetMembers())
    {
        if (Member.MemberWay)
        {
            float MemberScore = GetVerseScore(Member.MemberWay);
            if (Network->QualifiesForNetworkBonuses(static_cast<int32>(MemberScore)))
            {
                return true;
            }
        }
    }

    return false;
}

void UVerseSubsystem::RecordFeatWithNetworkEffects(const UFeatDataAsset* FeatToRecord, bool bApplyNetworkEffects)
{
    if (!FeatToRecord)
    {
        UE_LOG(LogAdastreaWay, Warning, TEXT("RecordFeatWithNetworkEffects called with null Feat"));
        return;
    }

    // First, record the feat normally
    RecordFeat(FeatToRecord);

    if (!bApplyNetworkEffects)
    {
        return;
    }

    // Process network spillover effects
    // Track which networks have been processed to avoid double-counting
    TSet<UWayNetworkDataAsset*> ProcessedNetworks;

    // For each registered network, check if the Feat aligns with network's shared Precepts
    for (UWayNetworkDataAsset* Network : RegisteredNetworks)
    {
        if (!Network || !Network->bIsActive || ProcessedNetworks.Contains(Network))
        {
            continue;
        }

        // Calculate alignment between Feat and Network
        float NetworkAlignment = Network->CalculateNetworkAlignment(FeatToRecord->PreceptAlignments);
        
        if (NetworkAlignment > 0.0f)
        {
            UE_LOG(LogAdastreaWay, Log, TEXT("Feat '%s' aligns with network '%s' (Alignment: %.2f)"), 
                *FeatToRecord->TitleName.ToString(), 
                *Network->NetworkName.ToString(), 
                NetworkAlignment);

            // Apply network alignment bonus
            float BonusAlignment = NetworkAlignment * Network->NetworkAlignmentBonus;
            
            UE_LOG(LogAdastreaWay, Log, TEXT("  Network bonus applied: %.2f (%.2fx multiplier)"), 
                BonusAlignment, 
                Network->NetworkAlignmentBonus);
        }

        ProcessedNetworks.Add(Network);
    }
}
