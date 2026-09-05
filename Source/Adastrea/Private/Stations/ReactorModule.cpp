// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ReactorModule.h"
#include "AdastreaLog.h"

AReactorModule::AReactorModule()
{
    ModuleType = TEXT("Reactor");
    ModulePower = -500.0f;  // Negative = generates power
    ModuleGroup = EStationModuleGroup::Power;

    MaxPowerOutput = 500.0f;
    bReactorOnline = true;
    bCanShutdown = true;
}

void AReactorModule::SetReactorOnline(bool bOnline)
{
    // If the reactor is destroyed, it cannot be brought back online.
    if (IsDestroyed_Implementation())
    {
        bReactorOnline = false;
        UE_LOG(LogAdastreaStations, Warning, TEXT("ReactorModule %s: cannot go online - destroyed"), *GetName());
        return;
    }
    if (!bCanShutdown)
    {
        UE_LOG(LogAdastreaStations, Log, TEXT("ReactorModule %s: shutdown not permitted, staying online"), *GetName());
        return;
    }
    bReactorOnline = bOnline;
    UE_LOG(LogAdastreaStations, Log, TEXT("ReactorModule %s: set online=%d (output %.0f)"),
        *GetName(), bReactorOnline, GetCurrentPowerOutput());
}

float AReactorModule::GetCurrentPowerOutput() const
{
    if (!bReactorOnline || IsDestroyed_Implementation())
    {
        return 0.0f;
    }
    // Scale output by remaining integrity so damaged reactors produce less.
    const float HealthRatio = GetHealthPercentage_Implementation();
    return MaxPowerOutput * FMath::Clamp(HealthRatio, 0.0f, 1.0f);
}

bool AReactorModule::IsProducingFullOutput() const
{
    return bReactorOnline && !IsDestroyed_Implementation() && GetHealthPercentage_Implementation() >= 0.99f;
}