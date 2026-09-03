// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "SpaceshipMeshValidator.generated.h"

/**
 * Editor Data Validation for the procedurally-generated Adastrea space-asset
 * meshes (SM_Ship_*, SM_Station_*, SM_Obj_*, SM_Prop_*). Mirrors the numeric
 * QA checks enforced by Tools/qa_assets.py (tri budgets by category, non-
 * degenerate faces, non-empty geometry) so the same conventions hold inside
 * the UE editor, at asset-save, and via Asset Audit / Data Validation runs.
 *
 * Subclasses of UEditorValidatorBase are auto-discovered at editor start and
 * run against assets on save / via "Validate" or the `DataValidation` command.
 */
UCLASS()
class ADASTREAEDITOR_API USpaceshipMeshValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	/** Only validate static meshes that belong to the Adastrea space-asset set. */
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;

	/** Run the mesh QA checks. */
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;

private:
	/** Map an FMeshDescription's tri count against the category-aware budget. Returns 0 if within budget, else the overage. */
	int32 GetTriBudgetForMesh(const FString& InAssetName) const;
};