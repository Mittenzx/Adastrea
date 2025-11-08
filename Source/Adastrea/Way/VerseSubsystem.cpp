#include "Way/VerseSubsystem.h"
#include "Way/Feat.h"
#include "Way/Way.h"
#include "AdastreaLog.h"

void UVerseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadVerseState();
    UE_LOG(LogAdastrea, Log, TEXT("Verse Subsystem Initialized."));
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
        UE_LOG(LogAdastrea, Warning, TEXT("RecordFeat called with a null FeatDataAsset."));
        return;
    }

    bool bAlreadyCompleted;
    CompletedFeats.Add(FeatToRecord, &bAlreadyCompleted);

    if (!bAlreadyCompleted)
    {
        UE_LOG(LogAdastrea, Log, TEXT("New Feat Recorded: '%s' (Title: %s)"), *FeatToRecord->FeatID.ToString(), *FeatToRecord->TitleName.ToString());
        // In a full game, you would likely want to broadcast a delegate here to notify the UI.
        // OnFeatRecorded.Broadcast(FeatToRecord);
    }
}

float UVerseSubsystem::GetVerseScore(const UWayDataAsset* TargetWay) const
{
    if (!TargetWay)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("GetVerseScore called with a null WayDataAsset."));
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
    UE_LOG(LogAdastrea, Log, TEXT("Verse state loaded (cleared for new session). Player has %d completed Feats."), CompletedFeats.Num());
}
