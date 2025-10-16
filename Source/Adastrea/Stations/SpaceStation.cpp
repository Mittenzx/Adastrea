#include "Stations/SpaceStation.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();
}

void ASpaceStation::AddModule(ASpaceStationModule* Module)
{
    if (Module && !Modules.Contains(Module))
    {
        Modules.Add(Module);
        
        // Attach the module to this station
        Module->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    }
}

bool ASpaceStation::AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation)
{
    if (!Module)
    {
        return false;
    }

    // Attach the module to this station
    Module->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    Module->SetActorRelativeLocation(RelativeLocation);

    // Add to modules array if not already present
    if (!Modules.Contains(Module))
    {
        Modules.Add(Module);
    }

    return true;
}

bool ASpaceStation::RemoveModule(ASpaceStationModule* Module)
{
    if (!Module || !Modules.Contains(Module))
    {
        return false;
    }

    // Remove from modules array
    Modules.Remove(Module);
    
    // Detach the module from this station
    Module->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

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

TArray<ASpaceStationModule*> ASpaceStation::GetModules() const
{
    return Modules;
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

int32 ASpaceStation::GetModuleCount() const
{
    return Modules.Num();
}
