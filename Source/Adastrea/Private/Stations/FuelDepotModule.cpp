// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/FuelDepotModule.h"

AFuelDepotModule::AFuelDepotModule()
{
    ModuleType = TEXT("Fuel Depot");
    ModulePower = 15.0f;
    ModuleGroup = EStationModuleGroup::Storage;

    FuelCapacity = 5000.0f;
    FuelLevel = 5000.0f;
}

float AFuelDepotModule::GetFuelLevel() const
{
    return FMath::Max(0.0f, FuelLevel);
}

float AFuelDepotModule::GetFuelRatio() const
{
    if (FuelCapacity <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp(GetFuelLevel() / FuelCapacity, 0.0f, 1.0f);
}

bool AFuelDepotModule::HasFuelAvailable(float Amount) const
{
    return Amount >= 0.0f && FuelLevel >= Amount;
}

bool AFuelDepotModule::AddFuel(float Amount)
{
    if (Amount < 0.0f || IsDestroyed_Implementation())
    {
        return false;
    }
    if (FuelLevel + Amount > FuelCapacity)
    {
        return false;
    }
    FuelLevel += Amount;
    return true;
}

float AFuelDepotModule::DrawFuel(float Amount)
{
    if (Amount <= 0.0f || IsDestroyed_Implementation())
    {
        return 0.0f;
    }
    const float Drawn = FMath::Min(GetFuelLevel(), Amount);
    FuelLevel -= Drawn;
    return Drawn;
}