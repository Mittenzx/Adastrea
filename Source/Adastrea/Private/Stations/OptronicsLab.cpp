// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/OptronicsLab.h"
#include "Stations/ElectronicsLabModule.h"

AOptronicsLab::AOptronicsLab()
{
    InitLab(TEXT("Optronics"), { "OptronicsResearch" }, AElectronicsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Optronics Research Lab");
}
