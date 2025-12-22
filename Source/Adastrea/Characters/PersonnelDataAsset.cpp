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
