// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/CargoBayModule.h"

ACargoBayModule::ACargoBayModule()
{
    ModuleType = TEXT("Cargo Bay");
    ModulePower = 5.0f;
    ModuleGroup = EStationModuleGroup::Storage;

    StorageCapacity = 800;
    StoredAmount = 0;
}

int32 ACargoBayModule::GetFreeStorage() const
{
    return FMath::Max(0, StorageCapacity - StoredAmount);
}

float ACargoBayModule::GetFillRatio() const
{
    if (StorageCapacity <= 0)
    {
        return 0.0f;
    }
    return FMath::Clamp((float)StoredAmount / (float)StorageCapacity, 0.0f, 1.0f);
}

bool ACargoBayModule::HasSpaceFor(int32 Amount) const
{
    return Amount >= 0 && (StoredAmount + Amount) <= StorageCapacity;
}

bool ACargoBayModule::AddStorage(int32 Amount)
{
    if (Amount < 0 || !HasSpaceFor(Amount) || IsDestroyed_Implementation())
    {
        return false;
    }
    StoredAmount += Amount;
    return true;
}

bool ACargoBayModule::RemoveStorage(int32 Amount)
{
    if (Amount < 0 || Amount > StoredAmount)
    {
        return false;
    }
    StoredAmount -= Amount;
    return true;
}