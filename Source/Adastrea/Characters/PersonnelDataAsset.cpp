#include "Characters/PersonnelDataAsset.h"

UPersonnelDataAsset::UPersonnelDataAsset()
{
    // Set default values
    PersonnelName = FText::FromString(TEXT("Unknown Personnel"));
    PersonnelID = FName(TEXT("UnknownPersonnel"));
    Biography = FText::FromString(TEXT("No biography available."));
    Age = 30;
    Gender = TEXT("Unknown");
    Species = TEXT("Human");
    Nationality = TEXT("Unknown");
    
    PrimaryRole = EPersonnelRole::CrewMember;
    CurrentAssignment = TEXT("Unassigned");
    Department = TEXT("General");
    ManagerID = NAME_None;
    
    OverallSkillLevel = 5;
    TotalExperience = 0;
    
    Morale = 50.0f;
    Health = 100.0f;
    Fatigue = 0.0f;
    Loyalty = 50.0f;
    Reputation = 0;
    
    Salary = 1000;
    ContractDuration = 12;
    ContractMonthsRemaining = 12;
    
    PersonalityType = TEXT("Balanced");
    PersonalityDescription = FText::FromString(TEXT("A balanced individual with standard temperament."));
}

// ====================
// Trait System Implementation
// ====================

TArray<FPersonnelTrait> UPersonnelDataAsset::GetTraits() const
{
    return Traits;
}

bool UPersonnelDataAsset::HasTrait(FName TraitID) const
{
    for (const FPersonnelTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            return true;
        }
    }
    return false;
}

bool UPersonnelDataAsset::GetTraitByID(FName TraitID, FPersonnelTrait& OutTrait) const
{
    for (const FPersonnelTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            OutTrait = Trait;
            return true;
        }
    }
    return false;
}

float UPersonnelDataAsset::GetTraitModifier(FName TraitID) const
{
    float TotalModifier = 0.0f;
    
    for (const FPersonnelTrait& Trait : Traits)
    {
        if (Trait.TraitID == TraitID)
        {
            TotalModifier += Trait.ModifierValue;
        }
    }
    
    return TotalModifier;
}

// ====================
// Skill System Implementation
// ====================

bool UPersonnelDataAsset::GetSkillByName(const FString& SkillName, FPersonnelSkill& OutSkill) const
{
    for (const FPersonnelSkill& Skill : Skills)
    {
        if (Skill.SkillName.Equals(SkillName, ESearchCase::IgnoreCase))
        {
            OutSkill = Skill;
            return true;
        }
    }
    return false;
}

int32 UPersonnelDataAsset::GetSkillLevel(const FString& SkillName) const
{
    for (const FPersonnelSkill& Skill : Skills)
    {
        if (Skill.SkillName.Equals(SkillName, ESearchCase::IgnoreCase))
        {
            return Skill.SkillLevel;
        }
    }
    return 0;
}

bool UPersonnelDataAsset::HasSpecialty(const FString& Specialty) const
{
    for (const FString& SpecialtyItem : Specialties)
    {
        if (SpecialtyItem.Equals(Specialty, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

// ====================
// Relationship System Implementation
// ====================

bool UPersonnelDataAsset::GetRelationship(FName OtherPersonnelID, FPersonnelRelationship& OutRelationship) const
{
    for (const FPersonnelRelationship& Relationship : Relationships)
    {
        if (Relationship.TargetPersonnelID == OtherPersonnelID)
        {
            OutRelationship = Relationship;
            return true;
        }
    }
    return false;
}

int32 UPersonnelDataAsset::GetRelationshipStrength(FName OtherPersonnelID) const
{
    for (const FPersonnelRelationship& Relationship : Relationships)
    {
        if (Relationship.TargetPersonnelID == OtherPersonnelID)
        {
            return Relationship.RelationshipStrength;
        }
    }
    return 0;
}

bool UPersonnelDataAsset::IsFriendsWith(FName OtherPersonnelID) const
{
    for (const FPersonnelRelationship& Relationship : Relationships)
    {
        if (Relationship.TargetPersonnelID == OtherPersonnelID)
        {
            return Relationship.RelationshipStrength > 30;
        }
    }
    return false;
}

bool UPersonnelDataAsset::IsRivalWith(FName OtherPersonnelID) const
{
    for (const FPersonnelRelationship& Relationship : Relationships)
    {
        if (Relationship.TargetPersonnelID == OtherPersonnelID)
        {
            return Relationship.RelationshipStrength < -30;
        }
    }
    return false;
}

// ====================
// Performance System Implementation
// ====================

bool UPersonnelDataAsset::GetPerformanceMetric(const FString& MetricName, FPerformanceMetric& OutMetric) const
{
    for (const FPerformanceMetric& Metric : PerformanceMetrics)
    {
        if (Metric.MetricName.Equals(MetricName, ESearchCase::IgnoreCase))
        {
            OutMetric = Metric;
            return true;
        }
    }
    return false;
}

float UPersonnelDataAsset::GetAveragePerformance() const
{
    if (PerformanceMetrics.Num() == 0)
    {
        return 50.0f; // Default average if no metrics
    }
    
    float TotalPerformance = 0.0f;
    for (const FPerformanceMetric& Metric : PerformanceMetrics)
    {
        TotalPerformance += Metric.Value;
    }
    
    return TotalPerformance / PerformanceMetrics.Num();
}

bool UPersonnelDataAsset::IsPerformingWell() const
{
    return GetAveragePerformance() > 70.0f;
}

// ====================
// Status System Implementation
// ====================

bool UPersonnelDataAsset::IsInGoodCondition() const
{
    return Health > 70.0f && Fatigue < 30.0f;
}

bool UPersonnelDataAsset::NeedsRest() const
{
    return Fatigue > 70.0f;
}

bool UPersonnelDataAsset::IsLoyal() const
{
    return Loyalty > 70.0f;
}

bool UPersonnelDataAsset::HasLowMorale() const
{
    return Morale < 30.0f;
}

// ====================
// Role System Implementation
// ====================

bool UPersonnelDataAsset::IsInLeadershipRole() const
{
    return PrimaryRole == EPersonnelRole::Captain ||
           PrimaryRole == EPersonnelRole::XO ||
           PrimaryRole == EPersonnelRole::StationManager ||
           PrimaryRole == EPersonnelRole::GunneryChief;
}

bool UPersonnelDataAsset::IsInCombatRole() const
{
    return PrimaryRole == EPersonnelRole::SecurityOfficer ||
           PrimaryRole == EPersonnelRole::GunneryChief ||
           PrimaryRole == EPersonnelRole::Gunner;
}

int32 UPersonnelDataAsset::GetDirectReportCount() const
{
    return DirectReports.Num();
}

#if WITH_EDITOR
EDataValidationResult UPersonnelDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    // Validate basic identity
    if (PersonnelName.IsEmpty())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Personnel Name is empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (PersonnelID.IsNone())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Personnel ID is not set")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate age range
    if (Age < 18 || Age > 200)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Age ({0}) must be between 18 and 200")),
            FText::AsNumber(Age)
        ));
        Result = EDataValidationResult::Invalid;
    }

    // Validate skill level
    if (OverallSkillLevel < 1 || OverallSkillLevel > 10)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Overall Skill Level ({0}) must be between 1 and 10")),
            FText::AsNumber(OverallSkillLevel)
        ));
        Result = EDataValidationResult::Invalid;
    }

    // Validate status values are in range
    if (Morale < 0.0f || Morale > 100.0f)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Morale ({0}) must be between 0 and 100")),
            FText::AsNumber(Morale)
        ));
        Result = EDataValidationResult::Invalid;
    }

    if (Health < 0.0f || Health > 100.0f)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Health ({0}) must be between 0 and 100")),
            FText::AsNumber(Health)
        ));
        Result = EDataValidationResult::Invalid;
    }

    if (Fatigue < 0.0f || Fatigue > 100.0f)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Fatigue ({0}) must be between 0 and 100")),
            FText::AsNumber(Fatigue)
        ));
        Result = EDataValidationResult::Invalid;
    }

    if (Loyalty < 0.0f || Loyalty > 100.0f)
    {
        ValidationErrors.Add(FText::Format(
            FText::FromString(TEXT("Loyalty ({0}) must be between 0 and 100")),
            FText::AsNumber(Loyalty)
        ));
        Result = EDataValidationResult::Invalid;
    }

    // Validate skills array
    for (const FPersonnelSkill& Skill : Skills)
    {
        if (Skill.SkillLevel < 1 || Skill.SkillLevel > 10)
        {
            ValidationErrors.Add(FText::Format(
                FText::FromString(TEXT("Skill '{0}' has invalid level ({1}), must be 1-10")),
                FText::FromString(Skill.SkillName),
                FText::AsNumber(Skill.SkillLevel)
            ));
            Result = EDataValidationResult::Invalid;
        }
    }

    // Validate relationships
    for (const FPersonnelRelationship& Relationship : Relationships)
    {
        if (Relationship.RelationshipStrength < -100 || Relationship.RelationshipStrength > 100)
        {
            ValidationErrors.Add(FText::Format(
                FText::FromString(TEXT("Relationship strength with '{0}' ({1}) must be between -100 and 100")),
                FText::FromName(Relationship.TargetPersonnelID),
                FText::AsNumber(Relationship.RelationshipStrength)
            ));
            Result = EDataValidationResult::Invalid;
        }
    }

    // Validate performance metrics
    for (const FPerformanceMetric& Metric : PerformanceMetrics)
    {
        if (Metric.Value < 0.0f || Metric.Value > 100.0f)
        {
            ValidationErrors.Add(FText::Format(
                FText::FromString(TEXT("Performance metric '{0}' value ({1}) must be between 0 and 100")),
                FText::FromString(Metric.MetricName),
                FText::AsNumber(Metric.Value)
            ));
            Result = EDataValidationResult::Invalid;
        }
    }

    // Warn about potential issues
    if (IsInLeadershipRole() && OverallSkillLevel < 5)
    {
        ValidationErrors.Add(FText::FromString(TEXT("Warning: Leadership role with skill level below 5 may affect crew performance")));
        // Just a warning
    }

    if (HasLowMorale() && IsInLeadershipRole())
    {
        ValidationErrors.Add(FText::FromString(TEXT("Warning: Leadership personnel with low morale may negatively impact team")));
        // Just a warning
    }

    // Log validation result
    if (Result == EDataValidationResult::Valid)
    {
        UE_LOG(LogAdastrea, Log, TEXT("PersonnelDataAsset %s passed validation"), *PersonnelName.ToString());
    }
    else
    {
        UE_LOG(LogAdastrea, Warning, TEXT("PersonnelDataAsset %s failed validation with %d errors"), 
            *PersonnelName.ToString(), ValidationErrors.Num());
    }

    return Result;
}
#endif
