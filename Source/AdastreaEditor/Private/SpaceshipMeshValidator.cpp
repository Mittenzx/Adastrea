// Copyright Mittenzx. All Rights Reserved.

#include "SpaceshipMeshValidator.h"
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "MeshTypes.h"
#include "StaticMeshAttributes.h"
#include "AssetRegistry/AssetData.h"

#define LOCTEXT_NAMESPACE "AdastreaMeshValidation"

bool USpaceshipMeshValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	// Only validate UStaticMesh assets that belong to the Adastrea procedural
	// space-asset set (SM_Ship_*, SM_Station_*, SM_Int_*, SM_Obj_*, SM_Prop_*).
	if (!InObject || !InObject->IsA<UStaticMesh>())
	{
		return false;
	}
	const FString Name = InAssetData.AssetName.ToString();
	return Name.StartsWith(TEXT("SM_"))
		&& (Name.Contains(TEXT("Ship")) || Name.Contains(TEXT("Station"))
			|| Name.Contains(TEXT("_Int_")) || Name.StartsWith(TEXT("SM_Int_"))
			|| Name.Contains(TEXT("Obj")) || Name.Contains(TEXT("Prop")));
}

int32 USpaceshipMeshValidator::GetTriBudgetForMesh(const FString& InAssetName) const
{
	// Mirror Tools/qa_assets.py category-aware budgets.
	if (InAssetName.Contains(TEXT("_Assembled")))
	{
		return 50000; // whole-ship composition bundles many components
	}
	if (InAssetName.Contains(TEXT("Ship")) && !InAssetName.Contains(TEXT("_Int_")) && !InAssetName.StartsWith(TEXT("SM_Int_")))
	{
		return 10000;
	}
	if (InAssetName.Contains(TEXT("Station")) || InAssetName.StartsWith(TEXT("SM_Int_")) || InAssetName.Contains(TEXT("_Int_")))
	{
		return 50000;
	}
	if (InAssetName.Contains(TEXT("Obj")) || InAssetName.Contains(TEXT("Prop")))
	{
		return 3000;
	}
	return 1000;
}

EDataValidationResult USpaceshipMeshValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	UStaticMesh* Mesh = Cast<UStaticMesh>(InAsset);
	if (!Mesh)
	{
		return EDataValidationResult::NotValidated;
	}

	int32 TotalTris = 0;
	int32 TotalVerts = 0;
	int32 DegenerateTris = 0;
	bool bHasAnyMesh = false;

	const int32 NumLODs = Mesh->GetNumSourceModels();
	for (int32 LODIndex = 0; LODIndex < NumLODs; ++LODIndex)
	{
		FMeshDescription* MD = Mesh->GetMeshDescription(LODIndex);
		if (!MD)
		{
			continue;
		}
		const FMeshDescription& MeshDesc = *MD;
		bHasAnyMesh = true;
		TotalVerts += MeshDesc.Vertices().Num();
		TotalTris += MeshDesc.Polygons().Num();

		// Degenerate check: any polygon with fewer than 3 vertex instances is a
		// zero-area face. (Polygons in imported game meshes are triangles.)
		for (const FPolygonID PolygonID : MeshDesc.Polygons().GetElementIDs())
		{
			if (MeshDesc.GetPolygonVertexInstances(PolygonID).Num() < 3)
			{
				++DegenerateTris;
			}
		}
	}

	if (!bHasAnyMesh)
	{
		AssetFails(InAsset, LOCTEXT("NoMeshDescription", "Source mesh has no valid MeshDescription (empty geometry)."));
		return EDataValidationResult::Invalid;
	}

	// Non-empty check (mirrors qa_assets non_empty threshold >10 verts/tris).
	if (TotalVerts <= 10 || TotalTris <= 10)
	{
		AssetFails(InAsset, LOCTEXT("EmptyMesh", "Mesh is effectively empty: too few verts/tris. Expected modelled geometry."));
		return EDataValidationResult::Invalid;
	}

	// Triangle budget check (category-aware).
	const int32 Budget = GetTriBudgetForMesh(InAssetData.AssetName.ToString());
	if (TotalTris > Budget)
	{
		AssetFails(InAsset, FText::Format(
			LOCTEXT("TriBudget", "{0} triangles exceeds the {1}-triangle budget for this asset category."),
			FText::AsNumber(TotalTris), FText::AsNumber(Budget)));
		return EDataValidationResult::Invalid;
	}

	// Degenerate faces warn (not hard-fail) in the editor path so a legitimate
	// in-progress iteration isn't blocked on save; qa_assets.py still hard-fails.
	if (DegenerateTris > 0)
	{
		AssetWarning(InAsset, FText::Format(
			LOCTEXT("DegenerateWarn", "{0} degenerate face(s) (polygon < 3 vertex instances) detected. Re-run Tools/qa_assets.py before ship."),
			FText::AsNumber(DegenerateTris)));
	}

	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE