// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ScienceLabModule.h"

AScienceLabModule::AScienceLabModule()
{
    ModuleType = TEXT("Science Lab");
    ModulePower = 75.0f;
    ModuleGroup = EStationModuleGroup::Processing;

    MaxResearchProjects = 3;
    ActiveProjects = 0;
    ResearchRatePerSecond = 10.0f;
}

bool AScienceLabModule::CanStartResearch() const
{
    return !IsDestroyed_Implementation() && ActiveProjects < MaxResearchProjects;
}

bool AScienceLabModule::StartResearch()
{
    if (!CanStartResearch())
    {
        return false;
    }
    ActiveProjects++;
    return true;
}

bool AScienceLabModule::CompleteResearchProject()
{
    if (IsDestroyed_Implementation() || ActiveProjects == 0)
    {
        return false;
    }
    ActiveProjects--;
    return true;
}