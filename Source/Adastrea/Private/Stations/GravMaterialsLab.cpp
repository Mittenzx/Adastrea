// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/GravMaterialsLab.h"
#include "Stations/MaterialsLabModule.h"

AGravMaterialsLab::AGravMaterialsLab()
{
    InitLab(TEXT("Grav Materials"), { "GravMaterialsResearch" }, AMaterialsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Grav Materials Research Lab");
}
