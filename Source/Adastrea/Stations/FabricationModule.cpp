// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/FabricationModule.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AFabricationModule::AFabricationModule()
{
	// Create static mesh component as root
	UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ModuleMesh"));
	RootComponent = MeshComponent;

	// Load the cube mesh from engine basic shapes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
	}

	// Set default module properties
	ModuleType = TEXT("Fabrication");
	ModulePower = 150.0f;
	ModuleGroup = EStationModuleGroup::Processing;
}
