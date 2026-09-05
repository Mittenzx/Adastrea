// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/BeamWeaponsLab.h"
#include "Stations/WeaponsLabModule.h"

ABeamWeaponsLab::ABeamWeaponsLab()
{
    InitLab(TEXT("Beam Weapons"), { "BeamWeaponResearch" }, AWeaponsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Beam Weapons Research Lab");
}
