// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/MaterialsLabModule.h"

AMaterialsLabModule::AMaterialsLabModule()
{
    InitLab(TEXT("Materials"), { "MaterialsResearch", "NanoMaterialsResearch" }, nullptr);
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Materials Research Lab");
}
