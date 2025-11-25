// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADockingBayModule::ADockingBayModule()
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
	ModuleType = TEXT("Docking Bay");
	ModulePower = 50.0f;
	ModuleGroup = EStationModuleGroup::Docking;
}
