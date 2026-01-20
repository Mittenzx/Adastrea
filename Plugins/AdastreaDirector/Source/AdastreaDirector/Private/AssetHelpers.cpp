// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AssetHelpers.h"
#include "AdastreaDirectorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/TextureFactory.h"
#include "Factories/FbxFactory.h"
#include "Factories/SoundFactory.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Engine/Blueprint.h"
#include "Materials/Material.h"

FAdastreaResult UAssetHelpers::ImportTexture(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return ImportAssetGeneric(FilePath, TargetFolder, AssetName, TEXT("Texture"));
}

FAdastreaResult UAssetHelpers::ImportStaticMesh(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return ImportAssetGeneric(FilePath, TargetFolder, AssetName, TEXT("StaticMesh"));
}

FAdastreaResult UAssetHelpers::ImportAudio(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return ImportAssetGeneric(FilePath, TargetFolder, AssetName, TEXT("Audio"));
}

FAdastreaResult UAssetHelpers::CreateBlueprint(const FString& BlueprintName, const FString& ParentClass, const FString& PackagePath)
{
#if WITH_EDITOR
	if (BlueprintName.IsEmpty())
	{
		return FAdastreaResult::MakeError(TEXT("Blueprint name cannot be empty"));
	}

	// Find parent class
	UClass* ParentClassObj = FindObject<UClass>(nullptr, *ParentClass);
	if (!ParentClassObj)
	{
		FString FullPath = FString::Printf(TEXT("/Script/Engine.%s"), *ParentClass);
		ParentClassObj = FindObject<UClass>(nullptr, *FullPath);
	}

	if (!ParentClassObj)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Parent class not found: %s"), *ParentClass));
	}

	// Get asset tools
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// Create blueprint factory
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClassObj;

	// Create the asset
	FString PackagePathStr = PackagePath;
	if (!PackagePathStr.EndsWith(TEXT("/")))
	{
		PackagePathStr += TEXT("/");
	}

	UObject* NewAsset = AssetTools.CreateAsset(BlueprintName, PackagePath, UBlueprint::StaticClass(), Factory);
	
	if (!NewAsset)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to create blueprint: %s"), *BlueprintName));
	}

	// Get the actual path from the created asset
	FString AssetPath = NewAsset->GetPathName();
	bool bSaved = UEditorAssetLibrary::SaveAsset(AssetPath, false);

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Created blueprint: %s"), *BlueprintName));
	Result.AddDetail(TEXT("asset_path"), AssetPath);
	Result.AddDetail(TEXT("asset_name"), BlueprintName);
	Result.AddDetail(TEXT("parent_class"), ParentClass);
	Result.AddDetail(TEXT("saved"), bSaved ? TEXT("true") : TEXT("false"));
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

FAdastreaResult UAssetHelpers::CreateMaterial(const FString& MaterialName, const FString& PackagePath)
{
#if WITH_EDITOR
	if (MaterialName.IsEmpty())
	{
		return FAdastreaResult::MakeError(TEXT("Material name cannot be empty"));
	}

	// Get asset tools
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// Create material factory
	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();

	// Create the asset
	FString PackagePathStr = PackagePath;
	if (!PackagePathStr.EndsWith(TEXT("/")))
	{
		PackagePathStr += TEXT("/");
	}

	UObject* NewAsset = AssetTools.CreateAsset(MaterialName, PackagePath, UMaterial::StaticClass(), Factory);
	
	if (!NewAsset)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to create material: %s"), *MaterialName));
	}

	// Get the actual path from the created asset
	FString AssetPath = NewAsset->GetPathName();
	bool bSaved = UEditorAssetLibrary::SaveAsset(AssetPath, false);

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Created material: %s"), *MaterialName));
	Result.AddDetail(TEXT("asset_path"), AssetPath);
	Result.AddDetail(TEXT("asset_name"), MaterialName);
	Result.AddDetail(TEXT("saved"), bSaved ? TEXT("true") : TEXT("false"));
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

// ============================================================================
// Private Helpers
// ============================================================================

FString UAssetHelpers::GetAssetNameFromPath(const FString& FilePath, const FString& ProvidedName)
{
	if (!ProvidedName.IsEmpty())
	{
		return ProvidedName;
	}

	return FPaths::GetBaseFilename(FilePath);
}

FAdastreaResult UAssetHelpers::ImportAssetGeneric(const FString& FilePath, const FString& TargetFolder, const FString& AssetName, const FString& AssetType)
{
#if WITH_EDITOR
	// Check if file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*FilePath))
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("File not found: %s"), *FilePath));
	}

	// Get or generate asset name
	FString FinalAssetName = GetAssetNameFromPath(FilePath, AssetName);

	// Use AssetTools for import
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FString> FilesToImport;
	FilesToImport.Add(FilePath);
	
	TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssets(FilesToImport, TargetFolder);
	
	FString ImportedAssetPath;
	if (ImportedObjects.Num() > 0 && ImportedObjects[0])
	{
		ImportedAssetPath = ImportedObjects[0]->GetPathName();
	}
	
	if (ImportedAssetPath.IsEmpty())
	{
		return FAdastreaResult::MakeError(FString::Printf(
			TEXT("Import task completed but no asset was created for %s '%s'. ")
			TEXT("Check the Unreal Editor log for detailed import errors. ")
			TEXT("Possible causes: unsupported file format, invalid %s data, or corrupted file."),
			*AssetType, *FilePath, *AssetType));
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Successfully imported %s: %s"), *AssetType, *ImportedAssetPath));
	Result.AddDetail(TEXT("asset_path"), ImportedAssetPath);
	Result.AddDetail(TEXT("local_path"), FilePath);
	Result.AddDetail(TEXT("asset_type"), AssetType);
	Result.AddDetail(TEXT("destination_folder"), TargetFolder);
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}
