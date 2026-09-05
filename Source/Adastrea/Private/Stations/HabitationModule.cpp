// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/HabitationModule.h"

AHabitationModule::AHabitationModule()
{
    ModuleType = TEXT("Habitation");
    ModulePower = 30.0f;
    ModuleGroup = EStationModuleGroup::Habitation;

    Capacity = 250;
    Residents = 0;
}

int32 AHabitationModule::GetFreeCapacity() const
{
    return FMath::Max(0, Capacity - Residents);
}

float AHabitationModule::GetOccupancyRatio() const
{
    if (Capacity <= 0)
    {
        return 0.0f;
    }
    return FMath::Clamp((float)Residents / (float)Capacity, 0.0f, 1.0f);
}

int32 AHabitationModule::AddResidents(int32 Count)
{
    if (Count <= 0 || IsDestroyed_Implementation())
    {
        return 0;
    }
    const int32 Added = FMath::Min(Count, GetFreeCapacity());
    Residents += Added;
    return Added;
}

int32 AHabitationModule::RemoveResidents(int32 Count)
{
    if (Count <= 0)
    {
        return 0;
    }
    const int32 Removed = FMath::Min(Count, Residents);
    Residents -= Removed;
    return Removed;
}