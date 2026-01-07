// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"

ADockingBayModule::ADockingBayModule()
{
    ModuleType = TEXT("Docking Bay");
    ModulePower = 50.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}

USceneComponent* ADockingBayModule::GetAvailableDockingPoint() const
{
    // Return the first docking point if any exist and there's capacity
    if (HasAvailableDocking() && DockingPoints.Num() > 0)
    {
        return DockingPoints[0];
    }
    return nullptr;
}
