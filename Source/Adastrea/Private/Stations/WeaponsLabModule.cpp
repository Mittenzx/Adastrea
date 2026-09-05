// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/WeaponsLabModule.h"

AWeaponsLabModule::AWeaponsLabModule()
{
    InitLab(TEXT("Weapons"), { "DefenceResearch", "AdvancedDefenceResearch" }, nullptr);
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Weapons Research Lab");
}
