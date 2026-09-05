// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ShieldGeneratorModule.h"

AShieldGeneratorModule::AShieldGeneratorModule()
{
    ModuleType = TEXT("Shield Generator");
    ModulePower = 200.0f;
    ModuleGroup = EStationModuleGroup::Defence;

    MaxShieldStrength = 4000.0f;
    CurrentShieldStrength = 4000.0f;
    RechargePerSecond = 25.0f;
    bShieldsActive = true;
}

float AShieldGeneratorModule::GetShieldRatio() const
{
    if (MaxShieldStrength <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp(CurrentShieldStrength / MaxShieldStrength, 0.0f, 1.0f);
}

float AShieldGeneratorModule::AbsorbDamage(float IncomingDamage)
{
    if (IncomingDamage <= 0.0f || !bShieldsActive || IsDestroyed_Implementation())
    {
        return IncomingDamage;
    }
    const float Absorbed = FMath::Min(CurrentShieldStrength, IncomingDamage);
    CurrentShieldStrength -= Absorbed;
    return IncomingDamage - Absorbed;
}

void AShieldGeneratorModule::RechargeShield(float PowerAllocated)
{
    if (!bShieldsActive || IsDestroyed_Implementation() || PowerAllocated <= 0.0f)
    {
        return;
    }
    CurrentShieldStrength = FMath::Min(MaxShieldStrength, CurrentShieldStrength + PowerAllocated);
}