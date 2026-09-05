// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/SolarArrayModule.h"

ASolarArrayModule::ASolarArrayModule()
{
    ModuleType = TEXT("Solar Array");
    ModulePower = -100.0f;  // Negative = generates power
    ModuleGroup = EStationModuleGroup::Power;

    PeakOutput = 100.0f;
    Illumination = 1.0f;
}

float ASolarArrayModule::GetEffectiveOutput() const
{
    if (IsDestroyed_Implementation())
    {
        return 0.0f;
    }
    const float HealthRatio = GetHealthPercentage_Implementation();
    return PeakOutput * FMath::Clamp(Illumination, 0.0f, 1.0f) * FMath::Clamp(HealthRatio, 0.0f, 1.0f);
}

bool ASolarArrayModule::IsContributing() const
{
    return GetEffectiveOutput() > 1.0f;
}