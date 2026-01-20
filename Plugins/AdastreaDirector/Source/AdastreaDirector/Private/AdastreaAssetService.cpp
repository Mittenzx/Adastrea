// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaAssetService.h"
#include "AdastreaDirectorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"

IAssetRegistry& FAdastreaAssetService::GetAssetRegistry()
{
	FAssetRegistryModule& AssetRegistryModule = 
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	return AssetRegistryModule.Get();
}

bool FAdastreaAssetService::IsAssetRegistryReady()
{
	return !GetAssetRegistry().IsLoadingAssets();
}

TArray<FAssetInfo> FAdastreaAssetService::SearchAssets(
	const FString& SearchPattern,
	const FString& ClassName,
	int32 MaxResults)
{
	TArray<FAssetInfo> Results;
	IAssetRegistry& AssetRegistry = GetAssetRegistry();

	// Build filter
	FARFilter Filter;
	
	if (!ClassName.IsEmpty())
	{
		// Map common class names to proper FTopLevelAssetPath format
		// FTopLevelAssetPath expects format: /Script/ModuleName.ClassName
		FString ClassPath;
		if (ClassName == TEXT("Blueprint"))
		{
			ClassPath = TEXT("/Script/Engine.Blueprint");
		}
		else if (ClassName == TEXT("Material"))
		{
			ClassPath = TEXT("/Script/Engine.Material");
		}
		else if (ClassName == TEXT("WidgetBlueprint"))
		{
			ClassPath = TEXT("/Script/UMGEditor.WidgetBlueprint");
		}
		else if (ClassName.Contains(TEXT("/")))
		{
			// Already in proper format
			ClassPath = ClassName;
		}
		else
		{
			// Assume Engine module for unknown classes
			ClassPath = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
		}
		
		Filter.ClassPaths.Add(FTopLevelAssetPath(ClassPath));
	}

	// Search all game content
	Filter.PackagePaths.Add(FName("/Game"));
	Filter.bRecursivePaths = true;

	// Get assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	// Filter by name pattern
	for (const FAssetData& AssetData : AssetDataList)
	{
		FString AssetName = AssetData.AssetName.ToString();
		
		if (SearchPattern == TEXT("*") || AssetName.Contains(SearchPattern))
		{
			Results.Add(ConvertAssetData(AssetData));
			
			if (Results.Num() >= MaxResults)
			{
				break;
			}
		}
	}

	UE_LOG(LogAdastreaDirector, Log, TEXT("Asset search: '%s' class '%s' -> %d results"),
		*SearchPattern, *ClassName, Results.Num());

	return Results;
}

TArray<FAssetInfo> FAdastreaAssetService::GetBlueprints(const FString& PathPrefix)
{
	IAssetRegistry& AssetRegistry = GetAssetRegistry();
	
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(PathPrefix.IsEmpty() ? FName("/Game") : FName(*PathPrefix));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	TArray<FAssetInfo> Results;
	for (const FAssetData& AssetData : AssetDataList)
	{
		Results.Add(ConvertAssetData(AssetData));
	}

	return Results;
}

TArray<FAssetInfo> FAdastreaAssetService::GetMaterials(const FString& PathPrefix)
{
	IAssetRegistry& AssetRegistry = GetAssetRegistry();
	
	FARFilter Filter;
	Filter.ClassPaths.Add(UMaterial::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(PathPrefix.IsEmpty() ? FName("/Game") : FName(*PathPrefix));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	TArray<FAssetInfo> Results;
	for (const FAssetData& AssetData : AssetDataList)
	{
		Results.Add(ConvertAssetData(AssetData));
	}

	return Results;
}

TArray<FAssetInfo> FAdastreaAssetService::GetWidgets(const FString& PathPrefix)
{
	IAssetRegistry& AssetRegistry = GetAssetRegistry();
	
	FARFilter Filter;
	// Use FTopLevelAssetPath directly instead of UWidgetBlueprint::StaticClass() to avoid UMGEditor dependency
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/UMGEditor.WidgetBlueprint")));
	Filter.PackagePaths.Add(PathPrefix.IsEmpty() ? FName("/Game") : FName(*PathPrefix));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	TArray<FAssetInfo> Results;
	for (const FAssetData& AssetData : AssetDataList)
	{
		Results.Add(ConvertAssetData(AssetData));
	}

	return Results;
}

TOptional<FAssetInfo> FAdastreaAssetService::GetAssetByPath(const FString& AssetPath)
{
	IAssetRegistry& AssetRegistry = GetAssetRegistry();
	
	// Use newer UE5 API with FTopLevelAssetPath for compatibility
	FSoftObjectPath ObjectPath(AssetPath);
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(ObjectPath);
	
	if (AssetData.IsValid())
	{
		return ConvertAssetData(AssetData);
	}
	
	return TOptional<FAssetInfo>();
}

FAssetInfo FAdastreaAssetService::ConvertAssetData(const FAssetData& AssetData)
{
	FAssetInfo Info;
	Info.Name = AssetData.AssetName.ToString();
	Info.Path = AssetData.GetObjectPathString();
	Info.Class = AssetData.AssetClassPath.GetAssetName().ToString();
	Info.DiskSize = 0; // Initialize to 0 in case package data is not available
	
	// Get disk size - UE 5.6+ uses GetAssetPackageDataCopy returning TOptional
	IAssetRegistry& AssetRegistry = GetAssetRegistry();
	TOptional<FAssetPackageData> PackageData = AssetRegistry.GetAssetPackageDataCopy(AssetData.PackageName);
	if (PackageData.IsSet())
	{
		Info.DiskSize = PackageData.GetValue().DiskSize;
	}
	
	return Info;
}

TSharedPtr<FJsonObject> FAssetInfo::ToJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("name"), Name);
	Json->SetStringField(TEXT("path"), Path);
	Json->SetStringField(TEXT("class"), Class);
	Json->SetNumberField(TEXT("diskSize"), DiskSize);
	return Json;
}
