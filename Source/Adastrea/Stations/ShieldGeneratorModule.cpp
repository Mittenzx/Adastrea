// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ShieldGeneratorModule.h"

AShieldGeneratorModule::AShieldGeneratorModule()
{
    ModuleType = TEXT("Shield Generator");
    ModulePower = 200.0f;
    ModuleGroup = EStationModuleGroup::Defence;
}
