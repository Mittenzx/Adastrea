// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/CorridorModule.h"

ACorridorModule::ACorridorModule()
{
    ModuleType = TEXT("Corridor");
    ModulePower = 2.0f;
    ModuleGroup = EStationModuleGroup::Connection;
}
