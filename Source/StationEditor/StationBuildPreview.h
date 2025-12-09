// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "StationBuildPreview.generated.h"

// Forward declarations
class ASpaceStationModule;

/**
 * Station Build Preview - Actor for showing module placement preview
 * 
 * This actor displays a preview of where a module will be placed in the
 * station editor. It shows the module's mesh with material overrides to
 * indicate valid (green) or invalid (red) placement positions.
 * 
 * Features:
 * - Dynamic mesh display based on selected module
 * - Material color coding for placement validity
 * - Position and rotation preview
 * - Show/hide functionality
 * 
 * Usage:
 * 1. Spawn this actor when entering station edit mode
 * 2. Call SetPreviewModule() to set the module being previewed
 * 3. Call UpdatePosition() as the player moves the cursor
 * 4. Call SetValid() to update the validity visual
 * 5. Call Hide() when not placing a module
 * 
 * @see UStationEditorManager
 */
UCLASS(BlueprintType)
class STATIONEDITOR_API AStationBuildPreview : public AActor
{
	GENERATED_BODY()

public:
	AStationBuildPreview();

	// =====================
	// Components
	// =====================

	/** Static mesh component for preview visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* PreviewMesh;

	// =====================
	// Configuration
	// =====================

	/** Material to use for valid placement (green tint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Materials")
	UMaterialInterface* ValidPlacementMaterial;

	/** Material to use for invalid placement (red tint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Materials")
	UMaterialInterface* InvalidPlacementMaterial;

	/** Color to apply for valid placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Materials")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

	/** Color to apply for invalid placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Materials")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	/** Opacity of the preview mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Materials", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float PreviewOpacity = 0.5f;

	/** Default mesh to use for preview (can be set in Blueprint, loads cube if not set) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview|Mesh")
	TSoftObjectPtr<UStaticMesh> DefaultPreviewMesh;

	// =====================
	// State
	// =====================

	/** Whether the preview is currently visible */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview|State")
	bool bIsVisible = false;

	/** Whether current placement is valid */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview|State")
	bool bIsValidPlacement = true;

	/** The module class currently being previewed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Preview|State")
	TSubclassOf<ASpaceStationModule> CurrentModuleClass;

	// =====================
	// Preview Control
	// =====================

	/**
	 * Set the module class to preview
	 * @param ModuleClass The class of module to preview
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void SetPreviewModule(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Update the preview position and rotation
	 * @param Position New world position
	 * @param Rotation New world rotation
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void UpdatePosition(FVector Position, FRotator Rotation);

	/**
	 * Update just the preview position
	 * @param Position New world position
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void SetPosition(FVector Position);

	/**
	 * Update just the preview rotation
	 * @param Rotation New world rotation
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void SetRotation(FRotator Rotation);

	/**
	 * Set whether the current placement is valid
	 * This updates the material color (green = valid, red = invalid)
	 * @param bValid True for valid placement, false for invalid
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void SetValid(bool bValid);

	/**
	 * Show the preview
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void Show();

	/**
	 * Hide the preview
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	void Hide();

	/**
	 * Toggle preview visibility
	 * @return New visibility state
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Control")
	bool ToggleVisibility();

	/**
	 * Check if preview is currently visible
	 * @return True if visible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Preview|Control")
	bool IsVisible() const;

	/**
	 * Check if current placement is valid
	 * @return True if placement is valid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Preview|Control")
	bool IsPlacementValid() const;

	// =====================
	// Material Control
	// =====================

	/**
	 * Set the valid placement material
	 * @param Material The material to use for valid placements
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Materials")
	void SetValidMaterial(UMaterialInterface* Material);

	/**
	 * Set the invalid placement material
	 * @param Material The material to use for invalid placements
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Materials")
	void SetInvalidMaterial(UMaterialInterface* Material);

	/**
	 * Set custom preview mesh
	 * @param Mesh The static mesh to use for preview
	 */
	UFUNCTION(BlueprintCallable, Category="Preview|Mesh")
	void SetPreviewMesh(UStaticMesh* Mesh);

protected:
	virtual void BeginPlay() override;

	/**
	 * Update the material based on current validity state
	 */
	void UpdateMaterial();

	/**
	 * Create a dynamic material instance for the preview
	 */
	void CreateDynamicMaterial();

private:
	/** Dynamic material instance for color changes */
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	/** Parameter name for color in the material */
	static const FName ColorParameterName;

	/** Parameter name for opacity in the material */
	static const FName OpacityParameterName;
};
