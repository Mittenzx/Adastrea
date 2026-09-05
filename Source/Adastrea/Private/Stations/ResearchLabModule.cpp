// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ResearchLabModule.h"

AResearchLabModule::AResearchLabModule()
{
    ModuleType = TEXT("Research Lab");
    ModulePower = 75.0f;
    ModuleGroup = EStationModuleGroup::Processing;

    ResearchDomain = TEXT("Generic");
    MaxResearchProjects = 3;
    ActiveProjects = 0;
    ResearchRatePerSecond = 12.0f;
}

void AResearchLabModule::InitLab(const FString& InDomain, TArray<FString> InBreakthroughs,
    TSubclassOf<AResearchLabModule> InPrerequisite)
{
    ResearchDomain = InDomain;
    BreakthroughItemIDs = MoveTemp(InBreakthroughs);
    PrerequisiteLab = InPrerequisite;
    ModuleType = FString::Printf(TEXT("%s Lab"), *InDomain);
}

bool AResearchLabModule::CanStartResearch() const
{
    return !IsDestroyed_Implementation() && ActiveProjects < MaxResearchProjects;
}

bool AResearchLabModule::StartResearch()
{
    if (!CanStartResearch())
    {
        return false;
    }
    ActiveProjects++;
    return true;
}

bool AResearchLabModule::CompleteResearchProject()
{
    if (IsDestroyed_Implementation() || ActiveProjects == 0)
    {
        return false;
    }
    ActiveProjects--;
    return true;
}

bool AResearchLabModule::IsNicheLab() const
{
    return PrerequisiteLab != nullptr;
}

FString AResearchLabModule::GetPrimaryBreakthrough() const
{
    return BreakthroughItemIDs.Num() > 0 ? BreakthroughItemIDs[0] : FString();
}