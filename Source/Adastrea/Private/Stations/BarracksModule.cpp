// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/BarracksModule.h"

ABarracksModule::ABarracksModule()
{
    ModuleType = TEXT("Barracks");
    ModulePower = 20.0f;
    ModuleGroup = EStationModuleGroup::Habitation;

    Capacity = 120;
    Occupancy = 0;
}

int32 ABarracksModule::GetFreeBerths() const
{
    return FMath::Max(0, Capacity - Occupancy);
}

float ABarracksModule::GetOccupancyRatio() const
{
    if (Capacity <= 0)
    {
        return 0.0f;
    }
    return FMath::Clamp((float)Occupancy / (float)Capacity, 0.0f, 1.0f);
}

int32 ABarracksModule::HouseCrew(int32 Count)
{
    if (Count <= 0 || IsDestroyed_Implementation())
    {
        return 0;
    }
    const int32 Housed = FMath::Min(Count, GetFreeBerths());
    Occupancy += Housed;
    return Housed;
}

int32 ABarracksModule::ReleaseCrew(int32 Count)
{
    if (Count <= 0)
    {
        return 0;
    }
    const int32 Released = FMath::Min(Count, Occupancy);
    Occupancy -= Released;
    return Released;
}