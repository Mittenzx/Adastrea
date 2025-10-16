#include "Stations/SpaceStation.h"
#include "Factions/FactionDataAsset.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = false;
    Faction = nullptr;
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

bool ASpaceStation::RemoveModule(ASpaceStationModule* Module)
{
    if (Module && Modules.Contains(Module))
    {
        Modules.Remove(Module);
        
        // Detach the module from this station
        Module->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        
        return true;
    }
    
    return false;
}

TArray<ASpaceStationModule*> ASpaceStation::GetModules() const
{
    return Modules;
}

int32 ASpaceStation::GetModuleCount() const
{
    return Modules.Num();
}

UFactionDataAsset* ASpaceStation::GetFaction() const
{
    return Faction;
}

void ASpaceStation::SetFaction(UFactionDataAsset* NewFaction)
{
    Faction = NewFaction;
}
