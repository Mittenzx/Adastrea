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
