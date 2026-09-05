// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/CyberneticsLab.h"
#include "Stations/BiologyLabModule.h"

ACyberneticsLab::ACyberneticsLab()
{
    InitLab(TEXT("Cybernetics"), { "CyberneticsResearch" }, ABiologyLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Cybernetics Research Lab");
}
