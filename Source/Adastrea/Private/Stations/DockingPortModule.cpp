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
    // Only provide a docking point if we have capacity and at least one point defined
    if (!HasAvailableDocking() || DockingPoints.Num() <= 0)
    {
        return nullptr;
    }

    // Select the next available docking point based on how many ships are currently docked.
    // This assumes docking points are filled in order and that HasAvailableDocking()
    // already enforces that CurrentDockedShips is within a valid range.
    const int32 NextDockingIndex = FMath::Clamp(CurrentDockedShips, 0, DockingPoints.Num() - 1);
    return DockingPoints[NextDockingIndex];
}

bool ADockingPortModule::DockShip()
{
    if (!HasAvailableDocking())
    {
        return false;
    }

    CurrentDockedShips++;
    return true;
}

bool ADockingPortModule::UndockShip()
{
    if (CurrentDockedShips <= 0)
    {
        return false;
    }

    CurrentDockedShips--;
    return true;
}
