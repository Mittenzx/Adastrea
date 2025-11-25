// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/CargoBayModule.h"

ACargoBayModule::ACargoBayModule()
{
    ModuleType = TEXT("Cargo Bay");
    ModulePower = 5.0f;
    ModuleGroup = EStationModuleGroup::Storage;
}
