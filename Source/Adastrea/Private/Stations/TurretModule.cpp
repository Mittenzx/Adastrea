// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/TurretModule.h"

ATurretModule::ATurretModule()
{
    ModuleType = TEXT("Turret");
    ModulePower = 25.0f;
    ModuleGroup = EStationModuleGroup::Defence;

    DamagePerShot = 10.0f;
    EngagementRange = 12000.0f;
    FireRate = 2.0f;
}

bool ATurretModule::CanFire() const
{
    // A turret is operational while undamaged. Power availability is enforced by
    // the station combat layer (a powerless station can't provide firing energy).
    return !IsDestroyed_Implementation();
}

float ATurretModule::GetDps() const
{
    return DamagePerShot * FireRate;
}