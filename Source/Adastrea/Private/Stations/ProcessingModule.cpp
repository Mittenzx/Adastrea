// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ProcessingModule.h"

AProcessingModule::AProcessingModule()
{
    ModuleType = TEXT("Processing");
    ModulePower = 100.0f;
    ModuleGroup = EStationModuleGroup::Processing;
}
