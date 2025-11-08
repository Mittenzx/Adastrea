#include "Way/Way.h"
#include "AdastreaLog.h"

UWayDataAsset::UWayDataAsset()
{
    // Set default values for core identity
    WayName = FText::FromString(TEXT("Unnamed Way"));
    Description = FText::FromString(TEXT("A group with shared values and philosophy."));
    WayID = FName(TEXT("UnknownWay"));
    bIsSchool = true;
    
    // Default UI colors
    PrimaryColor = FLinearColor(0.2f, 0.5f, 0.8f, 1.0f);
    SecondaryColor = FLinearColor(0.1f, 0.3f, 0.5f, 1.0f);
}

// ====================
// Precept Query Functions
// ====================

TArray<FPreceptValue> UWayDataAsset::GetPrecepts() const
{
    return CorePrecepts;
}

bool UWayDataAsset::HasPrecept(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return true;
        }
    }
    return false;
}

int32 UWayDataAsset::GetPreceptValue(EPrecept Precept) const
{
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.Precept == Precept)
        {
            return PreceptValue.ImportanceValue;
        }
    }
    return 0;
}

bool UWayDataAsset::GetPrimaryPrecept(EPrecept& OutPrecept) const
{
    if (CorePrecepts.Num() == 0)
    {
        return false;
    }
    
    int32 HighestValue = 0;
    EPrecept PrimaryPrecept = EPrecept::Honor;
    
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.ImportanceValue > HighestValue)
        {
            HighestValue = PreceptValue.ImportanceValue;
            PrimaryPrecept = PreceptValue.Precept;
        }
    }
    
    OutPrecept = PrimaryPrecept;
    return true;
}

TArray<FPreceptValue> UWayDataAsset::GetPreceptsAboveThreshold(int32 Threshold) const
{
    TArray<FPreceptValue> Result;
    
    for (const FPreceptValue& PreceptValue : CorePrecepts)
    {
        if (PreceptValue.ImportanceValue >= Threshold)
        {
            Result.Add(PreceptValue);
        }
    }
    
    return Result;
}

// ====================
// Helper Functions
// ====================

FText UWayDataAsset::GetPreceptDisplayName(EPrecept Precept)
{
    // Return the enum display name defined in the UMETA tag
    const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/Adastrea.EPrecept"), true);
    if (EnumPtr)
    {
        return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Precept));
    }
    return FText::FromString(TEXT("Unknown"));
}

FText UWayDataAsset::GetPreceptDescription(EPrecept Precept)
{
    // Provide descriptions for each Precept
    switch (Precept)
    {
        case EPrecept::Honor:
            return FText::FromString(TEXT("Upholding principles and keeping one's word"));
        case EPrecept::Justice:
            return FText::FromString(TEXT("Fairness and righteousness in actions"));
        case EPrecept::Compassion:
            return FText::FromString(TEXT("Mercy and kindness toward others"));
        case EPrecept::Loyalty:
            return FText::FromString(TEXT("Dedication and steadfastness to causes"));
        case EPrecept::Mastery:
            return FText::FromString(TEXT("Pursuit of skill perfection"));
        case EPrecept::Innovation:
            return FText::FromString(TEXT("Creating new solutions and ideas"));
        case EPrecept::Discovery:
            return FText::FromString(TEXT("Uncovering hidden knowledge and truths"));
        case EPrecept::Craftsmanship:
            return FText::FromString(TEXT("Excellence in creation and building"));
        case EPrecept::Strength:
            return FText::FromString(TEXT("Physical and military might"));
        case EPrecept::Dominance:
            return FText::FromString(TEXT("Control and supremacy over others"));
        case EPrecept::Cunning:
            return FText::FromString(TEXT("Strategic thinking and cleverness"));
        case EPrecept::Ambition:
            return FText::FromString(TEXT("Drive to rise and succeed"));
        case EPrecept::Unity:
            return FText::FromString(TEXT("Togetherness and cooperation"));
        case EPrecept::Freedom:
            return FText::FromString(TEXT("Liberty and independence"));
        case EPrecept::Tradition:
            return FText::FromString(TEXT("Preserving heritage and customs"));
        case EPrecept::Progress:
            return FText::FromString(TEXT("Advancement and positive change"));
        case EPrecept::Prosperity:
            return FText::FromString(TEXT("Wealth and material abundance"));
        case EPrecept::Survival:
            return FText::FromString(TEXT("Endurance and resilience"));
        case EPrecept::Efficiency:
            return FText::FromString(TEXT("Optimal use of resources"));
        case EPrecept::Harmony:
            return FText::FromString(TEXT("Balance and peaceful coexistence"));
        default:
            return FText::FromString(TEXT("A core value of this group"));
    }
}
