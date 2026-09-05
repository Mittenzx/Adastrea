// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ElectronicsLabModule.h"

AElectronicsLabModule::AElectronicsLabModule()
{
    InitLab(TEXT("Computing"), { "ComputingResearch", "QuantumComputingResearch" }, nullptr);
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Computing Research Lab");
}
