#include "Stations/SpaceStation.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();
}

bool ASpaceStation::AddModule(ASpaceStationModule* Module, FVector RelativeLocation)
{
    if (!Module)
    {
        return false;
    }

    // Attach the module to this station
    Module->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    Module->SetActorRelativeLocation(RelativeLocation);

    // Add to modules array
    Modules.Add(Module);

    return true;
}

bool ASpaceStation::RemoveModule(ASpaceStationModule* Module)
{
    if (!Module || !Modules.Contains(Module))
    {
        return false;
    }

    // Detach the module from this station
    Module->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Remove from modules array
    Modules.Remove(Module);

    return true;
}

bool ASpaceStation::MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation)
{
    if (!Module || !Modules.Contains(Module))
    {
        return false;
    }

    // Update the module's relative location
    Module->SetActorRelativeLocation(NewRelativeLocation);

    return true;
}

TArray<ASpaceStationModule*> ASpaceStation::GetModulesByType(const FString& ModuleType)
{
    TArray<ASpaceStationModule*> MatchingModules;

    for (ASpaceStationModule* Module : Modules)
    {
        if (Module && Module->ModuleType == ModuleType)
        {
            MatchingModules.Add(Module);
        }
    }

    return MatchingModules;
}
