// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/ProcessingModule.h"

AProcessingModule::AProcessingModule()
{
    ModuleType = TEXT("Processing");
    ModulePower = 100.0f;
    ModuleGroup = EStationModuleGroup::Processing;

    BufferCapacity = 400;
    BufferedAmount = 0;
    ThroughputPerSecond = 20.0f;
    bIsRunning = false;
}

float AProcessingModule::GetBufferRatio() const
{
    if (BufferCapacity <= 0)
    {
        return 0.0f;
    }
    return FMath::Clamp((float)BufferedAmount / (float)BufferCapacity, 0.0f, 1.0f);
}

bool AProcessingModule::FeedRawMaterial(int32 Amount)
{
    if (Amount < 0 || IsDestroyed_Implementation() || (BufferedAmount + Amount) > BufferCapacity)
    {
        return false;
    }
    BufferedAmount += Amount;
    return true;
}

int32 AProcessingModule::Process(int32 Amount)
{
    if (Amount <= 0 || IsDestroyed_Implementation() || BufferedAmount == 0)
    {
        return 0;
    }
    const int32 Processed = FMath::Min(Amount, BufferedAmount);
    BufferedAmount -= Processed;
    bIsRunning = true;
    return Processed;
}