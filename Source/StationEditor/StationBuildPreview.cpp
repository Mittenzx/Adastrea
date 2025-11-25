// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationBuildPreview.h"
#include "AdastreaLog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

// Static member definitions
const FName AStationBuildPreview::ColorParameterName = TEXT("BaseColor");
const FName AStationBuildPreview::OpacityParameterName = TEXT("Opacity");

AStationBuildPreview::AStationBuildPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create preview mesh component
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = PreviewMesh;

	// Configure mesh for preview (no collision, transparent)
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetCastShadow(false);
	PreviewMesh->SetVisibility(false);

	// Default colors
	ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
	InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
	PreviewOpacity = 0.5f;

	// State initialization
	bIsVisible = false;
	bIsValidPlacement = true;
	CurrentModuleClass = nullptr;
	DynamicMaterial = nullptr;
	ValidPlacementMaterial = nullptr;
	InvalidPlacementMaterial = nullptr;
}

void AStationBuildPreview::BeginPlay()
{
	Super::BeginPlay();

	// Load default mesh if specified, otherwise load cube mesh
	if (DefaultPreviewMesh.IsNull())
	{
		// Load default cube mesh at runtime using LoadObject (safer than StaticLoadObject)
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeMesh)
		{
			PreviewMesh->SetStaticMesh(CubeMesh);
		}
	}
	else
	{
		// Load the configured default mesh
		UStaticMesh* Mesh = DefaultPreviewMesh.LoadSynchronous();
		if (Mesh)
		{
			PreviewMesh->SetStaticMesh(Mesh);
		}
	}

	// Create dynamic material
	CreateDynamicMaterial();

	// Start hidden
	Hide();
}

// =====================
// Preview Control
// =====================

void AStationBuildPreview::SetPreviewModule(TSubclassOf<ASpaceStationModule> ModuleClass)
{
	CurrentModuleClass = ModuleClass;

	if (!ModuleClass)
	{
		Hide();
		return;
	}

	// Get default object to access mesh
	ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
	if (!DefaultModule)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationBuildPreview::SetPreviewModule - Failed to get default object for module class %s"), *ModuleClass->GetName());
		return;
	}

	// Try to get mesh from the module's mesh component
	UStaticMeshComponent* ModuleMeshComp = DefaultModule->FindComponentByClass<UStaticMeshComponent>();
	if (ModuleMeshComp && ModuleMeshComp->GetStaticMesh())
	{
		PreviewMesh->SetStaticMesh(ModuleMeshComp->GetStaticMesh());
	}

	// Show preview
	Show();

	// Update material
	UpdateMaterial();

	UE_LOG(LogAdastreaStations, Log, TEXT("StationBuildPreview::SetPreviewModule - Set preview for module class %s"), *ModuleClass->GetName());
}

void AStationBuildPreview::UpdatePosition(FVector Position, FRotator Rotation)
{
	SetActorLocation(Position);
	SetActorRotation(Rotation);
}

void AStationBuildPreview::SetPosition(FVector Position)
{
	SetActorLocation(Position);
}

void AStationBuildPreview::SetRotation(FRotator Rotation)
{
	SetActorRotation(Rotation);
}

void AStationBuildPreview::SetValid(bool bValid)
{
	if (bIsValidPlacement != bValid)
	{
		bIsValidPlacement = bValid;
		UpdateMaterial();
	}
}

void AStationBuildPreview::Show()
{
	bIsVisible = true;
	PreviewMesh->SetVisibility(true);
	SetActorHiddenInGame(false);
}

void AStationBuildPreview::Hide()
{
	bIsVisible = false;
	PreviewMesh->SetVisibility(false);
	SetActorHiddenInGame(true);
}

bool AStationBuildPreview::ToggleVisibility()
{
	if (bIsVisible)
	{
		Hide();
	}
	else
	{
		Show();
	}
	return bIsVisible;
}

bool AStationBuildPreview::IsVisible() const
{
	return bIsVisible;
}

bool AStationBuildPreview::IsPlacementValid() const
{
	return bIsValidPlacement;
}

// =====================
// Material Control
// =====================

void AStationBuildPreview::SetValidMaterial(UMaterialInterface* Material)
{
	ValidPlacementMaterial = Material;
	UpdateMaterial();
}

void AStationBuildPreview::SetInvalidMaterial(UMaterialInterface* Material)
{
	InvalidPlacementMaterial = Material;
	UpdateMaterial();
}

void AStationBuildPreview::SetPreviewMesh(UStaticMesh* Mesh)
{
	if (Mesh)
	{
		PreviewMesh->SetStaticMesh(Mesh);
	}
}

// =====================
// Protected Functions
// =====================

void AStationBuildPreview::UpdateMaterial()
{
	if (!PreviewMesh)
	{
		return;
	}

	// Use specific materials if provided
	if (bIsValidPlacement && ValidPlacementMaterial)
	{
		PreviewMesh->SetMaterial(0, ValidPlacementMaterial);
		return;
	}
	else if (!bIsValidPlacement && InvalidPlacementMaterial)
	{
		PreviewMesh->SetMaterial(0, InvalidPlacementMaterial);
		return;
	}

	// Use dynamic material with color parameter
	if (DynamicMaterial)
	{
		FLinearColor Color = bIsValidPlacement ? ValidColor : InvalidColor;
		DynamicMaterial->SetVectorParameterValue(ColorParameterName, Color);
		DynamicMaterial->SetScalarParameterValue(OpacityParameterName, PreviewOpacity);
		PreviewMesh->SetMaterial(0, DynamicMaterial);
	}
}

void AStationBuildPreview::CreateDynamicMaterial()
{
	// Try to create dynamic material from existing material
	UMaterialInterface* BaseMaterial = PreviewMesh->GetMaterial(0);
	
	if (BaseMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (DynamicMaterial)
		{
			PreviewMesh->SetMaterial(0, DynamicMaterial);
			
			// Set initial color
			FLinearColor Color = bIsValidPlacement ? ValidColor : InvalidColor;
			DynamicMaterial->SetVectorParameterValue(ColorParameterName, Color);
			DynamicMaterial->SetScalarParameterValue(OpacityParameterName, PreviewOpacity);
		}
	}
	else
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationBuildPreview::CreateDynamicMaterial - No base material available"));
	}
}
