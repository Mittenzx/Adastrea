#include "Characters/PersonnelDataAsset.h"
#include "Misc/DataValidation.h"

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

bool UPersonnelDataAsset::GetSkillByName(const FString& SkillName, FPersonnelSkill& OutSkill) const
{
    for (const FPersonnelSkill& Skill : Skills)
    {
        if (Skill.SkillName == SkillName)
        {
            OutSkill = Skill;
            return true;
        }
    }
    return false;
}

int32 UPersonnelDataAsset::GetSkillLevel(const FString& SkillName) const
{
    FPersonnelSkill Skill;
    if (GetSkillByName(SkillName, Skill))
    {
        return Skill.SkillLevel;
    }
    return 0;
}

bool UPersonnelDataAsset::HasSpecialty(const FString& Specialty) const
{
    return Specialties.Contains(Specialty);
}

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
    FPersonnelRelationship Relationship;
    if (GetRelationship(OtherPersonnelID, Relationship))
    {
        return Relationship.RelationshipStrength;
    }
    return 0;
}

bool UPersonnelDataAsset::IsFriendsWith(FName OtherPersonnelID) const
{
    return GetRelationshipStrength(OtherPersonnelID) > 20;
}

bool UPersonnelDataAsset::IsRivalWith(FName OtherPersonnelID) const
{
    return GetRelationshipStrength(OtherPersonnelID) < -20;
}

bool UPersonnelDataAsset::GetPerformanceMetric(const FString& MetricName, FPerformanceMetric& OutMetric) const
{
    for (const FPerformanceMetric& Metric : PerformanceMetrics)
    {
        if (Metric.MetricName == MetricName)
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
        return 0.0f;
    }

    float Total = 0.0f;
    for (const FPerformanceMetric& Metric : PerformanceMetrics)
    {
        Total += Metric.Value;
    }
    return Total / PerformanceMetrics.Num();
}

bool UPersonnelDataAsset::IsPerformingWell() const
{
    return GetAveragePerformance() > 70.0f;
}

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

bool UPersonnelDataAsset::IsInLeadershipRole() const
{
    return PrimaryRole == EPersonnelRole::Captain ||
           PrimaryRole == EPersonnelRole::XO ||
           PrimaryRole == EPersonnelRole::StationManager ||
           PrimaryRole == EPersonnelRole::Ambassador ||
           PrimaryRole == EPersonnelRole::Diplomat;
}

bool UPersonnelDataAsset::IsInCombatRole() const
{
    return PrimaryRole == EPersonnelRole::SecurityOfficer ||
           PrimaryRole == EPersonnelRole::GunneryChief ||
           PrimaryRole == EPersonnelRole::Gunner ||
           PrimaryRole == EPersonnelRole::Pilot;
}

int32 UPersonnelDataAsset::GetDirectReportCount() const
{
    return DirectReports.Num();
}

#if WITH_EDITOR
EDataValidationResult UPersonnelDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    if (PersonnelName.IsEmpty())
    {
        Context.AddError(FText::FromString(TEXT("Personnel name cannot be empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (PersonnelID.IsNone())
    {
        Context.AddError(FText::FromString(TEXT("Personnel ID cannot be empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (Age < 18 || Age > 200)
    {
        Context.AddError(FText::FromString(TEXT("Age must be between 18 and 200")));
        Result = EDataValidationResult::Invalid;
    }

    if (OverallSkillLevel < 1 || OverallSkillLevel > 10)
    {
        Context.AddError(FText::FromString(TEXT("Overall skill level must be between 1 and 10")));
        Result = EDataValidationResult::Invalid;
    }

    if (Morale < 0.0f || Morale > 100.0f)
    {
        Context.AddError(FText::FromString(TEXT("Morale must be between 0 and 100")));
        Result = EDataValidationResult::Invalid;
    }

    if (Health < 0.0f || Health > 100.0f)
    {
        Context.AddError(FText::FromString(TEXT("Health must be between 0 and 100")));
        Result = EDataValidationResult::Invalid;
    }

    if (Fatigue < 0.0f || Fatigue > 100.0f)
    {
        Context.AddError(FText::FromString(TEXT("Fatigue must be between 0 and 100")));
        Result = EDataValidationResult::Invalid;
    }

    if (Loyalty < 0.0f || Loyalty > 100.0f)
    {
        Context.AddError(FText::FromString(TEXT("Loyalty must be between 0 and 100")));
        Result = EDataValidationResult::Invalid;
    }

    if (Reputation < -100 || Reputation > 100)
    {
        Context.AddError(FText::FromString(TEXT("Reputation must be between -100 and 100")));
        Result = EDataValidationResult::Invalid;
    }

    if (Salary < 0)
    {
        Context.AddError(FText::FromString(TEXT("Salary cannot be negative")));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
