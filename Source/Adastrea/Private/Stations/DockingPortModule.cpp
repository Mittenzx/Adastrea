// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingPortModule.h"

ADockingPortModule::ADockingPortModule()
{
    ModuleType = TEXT("Docking Port");
    ModulePower = 10.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}

USceneComponent* ADockingPortModule::GetAvailableDockingPoint() const
{
    // Return the first docking point if any exist and there's capacity
    if (HasAvailableDocking() && DockingPoints.Num() > 0)
    {
        return DockingPoints[0];
    }
    return nullptr;
}
