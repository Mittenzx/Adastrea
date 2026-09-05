// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/CorridorModule.h"

ACorridorModule::ACorridorModule()
{
    ModuleType = TEXT("Corridor");
    ModulePower = 2.0f;
    ModuleGroup = EStationModuleGroup::Connection;

    ConnectedModuleCount = 0;
    MaxTraffic = 80;
    CurrentTraffic = 0;
}

bool ACorridorModule::IsIsolated() const
{
    return ConnectedModuleCount <= 1;
}

float ACorridorModule::GetTrafficRatio() const
{
    if (MaxTraffic <= 0)
    {
        return 0.0f;
    }
    return FMath::Clamp((float)CurrentTraffic / (float)MaxTraffic, 0.0f, 1.0f);
}