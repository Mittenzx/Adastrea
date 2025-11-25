// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/SolarArrayModule.h"

ASolarArrayModule::ASolarArrayModule()
{
    ModuleType = TEXT("Solar Array");
    ModulePower = -100.0f;  // Negative = generates power
    ModuleGroup = EStationModuleGroup::Power;
}
