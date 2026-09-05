// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ProjectileWeaponsLab.h"
#include "Stations/WeaponsLabModule.h"

AProjectileWeaponsLab::AProjectileWeaponsLab()
{
    InitLab(TEXT("Kinetic Weapons"), { "KineticWeaponResearch" }, AWeaponsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Kinetic Weapons Research Lab");
}
