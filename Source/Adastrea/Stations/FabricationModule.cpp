// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/FabricationModule.h"

AFabricationModule::AFabricationModule()
{
    ModuleType = TEXT("Fabrication");
    ModulePower = 150.0f;
    ModuleGroup = EStationModuleGroup::Processing;
}
