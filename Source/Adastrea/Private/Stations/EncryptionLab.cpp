// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/EncryptionLab.h"
#include "Stations/ElectronicsLabModule.h"

AEncryptionLab::AEncryptionLab()
{
    InitLab(TEXT("Encryption"), { "EncryptionResearch" }, AElectronicsLabModule::StaticClass());
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("Encryption Research Lab");
}
