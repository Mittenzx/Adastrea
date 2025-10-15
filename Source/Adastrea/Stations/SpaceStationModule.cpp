#include "Stations/SpaceStationModule.h"

ASpaceStationModule::ASpaceStationModule()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Default values
    ModuleType = TEXT("Generic");
    ModulePower = 0.0f;
}
