#include "Stations/SpaceStationModule.h"
#include "Public/Factions/FactionDataAsset.h"

ASpaceStationModule::ASpaceStationModule()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Default values
    ModuleType = TEXT("Generic");
    ModulePower = 0.0f;
    ModuleGroup = EStationModuleGroup::Other;
    ModuleFaction = nullptr;
}

UFactionDataAsset* ASpaceStationModule::GetModuleFaction() const
{
    return ModuleFaction;
}

void ASpaceStationModule::SetModuleFaction(UFactionDataAsset* NewFaction)
{
    ModuleFaction = NewFaction;
}
