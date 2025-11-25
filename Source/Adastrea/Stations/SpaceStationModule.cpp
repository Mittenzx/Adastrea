#include "Stations/SpaceStationModule.h"
#include "Factions/FactionDataAsset.h"
#include "UObject/ConstructorHelpers.h"

ASpaceStationModule::ASpaceStationModule()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create static mesh component as root
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ModuleMesh"));
    RootComponent = MeshComponent;

    // Load the cube mesh from engine basic shapes
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
    }

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
