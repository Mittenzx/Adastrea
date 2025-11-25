// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/TurretModule.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ATurretModule::ATurretModule()
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
	ModuleType = TEXT("Turret");
	ModulePower = 25.0f;
	ModuleGroup = EStationModuleGroup::Defence;
}
