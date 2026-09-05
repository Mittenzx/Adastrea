// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/FabricationModule.h"

AFabricationModule::AFabricationModule()
{
    ModuleType = TEXT("Fabrication");
    ModulePower = 150.0f;
    ModuleGroup = EStationModuleGroup::Processing;

    MaxJobCount = 4;
    BuildRatePerSecond = 50.0f;
}

int32 AFabricationModule::GetQueuedJobCount() const
{
    return JobQueue.Num();
}

bool AFabricationModule::CanAcceptJob() const
{
    return !IsDestroyed_Implementation() && GetQueuedJobCount() < MaxJobCount;
}

bool AFabricationModule::AddJob(float WorkUnits)
{
    if (WorkUnits <= 0.0f || !CanAcceptJob())
    {
        return false;
    }
    JobQueue.Add(WorkUnits);
    return true;
}

int32 AFabricationModule::AdvanceJob(float WorkDone)
{
    if (IsDestroyed_Implementation() || JobQueue.IsEmpty() || WorkDone <= 0.0f)
    {
        return 0;
    }

    int32 CompletedJobs = 0;
    float RemainingWork = WorkDone;

    int32 Index = 0;
    while (Index < JobQueue.Num() && RemainingWork > 0.0f)
    {
        JobQueue[Index] -= RemainingWork;
        if (JobQueue[Index] <= 0.0f)
        {
            // Front job finished; carry leftover work into the next job.
                        RemainingWork = -JobQueue[Index];
                        JobQueue.RemoveAt(Index);
            CompletedJobs++;
        }
        else
        {
            RemainingWork = 0.0f;
        }
    }

    return CompletedJobs;
}

float AFabricationModule::GetFrontJobProgress() const
{
    return JobQueue.IsEmpty() ? 0.0f : 1.0f;
}