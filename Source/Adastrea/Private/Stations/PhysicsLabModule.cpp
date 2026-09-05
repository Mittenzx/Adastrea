// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/PhysicsLabModule.h"

APhysicsLabModule::APhysicsLabModule()
{
    InitLab(TEXT("Propulsion"), { "PropulsionResearch", "AdvancedPropulsionResearch" }, nullptr);
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Propulsion Research Lab");
}
