// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ReactorModule.h"

AReactorModule::AReactorModule()
{
    ModuleType = TEXT("Reactor");
    ModulePower = -500.0f;  // Negative = generates power
    ModuleGroup = EStationModuleGroup::Power;
}
