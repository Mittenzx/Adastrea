// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/BiologyLabModule.h"

ABiologyLabModule::ABiologyLabModule()
{
    InitLab(TEXT("Biotech"), { "BioResearch", "AdvancedBioResearch" }, nullptr);
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Biotech Research Lab");
}
