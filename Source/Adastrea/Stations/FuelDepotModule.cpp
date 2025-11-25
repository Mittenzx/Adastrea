// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/FuelDepotModule.h"

AFuelDepotModule::AFuelDepotModule()
{
    ModuleType = TEXT("Fuel Depot");
    ModulePower = 15.0f;
    ModuleGroup = EStationModuleGroup::Storage;
}
