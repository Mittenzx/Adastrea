// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/TurretModule.h"

ATurretModule::ATurretModule()
{
    ModuleType = TEXT("Turret");
    ModulePower = 25.0f;
    ModuleGroup = EStationModuleGroup::Defence;
}
