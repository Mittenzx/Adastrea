// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingPortModule.h"

ADockingPortModule::ADockingPortModule()
{
    ModuleType = TEXT("Docking Port");
    ModulePower = 10.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}
