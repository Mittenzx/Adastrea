// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"

ADockingBayModule::ADockingBayModule()
{
    ModuleType = TEXT("Docking Bay");
    ModulePower = 50.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}
