// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/IonPropulsionLab.h"
#include "Stations/PhysicsLabModule.h"

AIonPropulsionLab::AIonPropulsionLab()
{
    InitLab(TEXT("Ion Propulsion"), { "IonPropulsionResearch" }, APhysicsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Ion Propulsion Research Lab");
}
