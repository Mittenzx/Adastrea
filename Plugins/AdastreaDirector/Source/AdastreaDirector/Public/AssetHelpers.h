// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StandardResult.h"
#include "UObject/NoExportTypes.h"
#include "AssetHelpers.generated.h"

/**
 * Helper utilities for asset operations.
 * C++ equivalent of the Python adastrea_helpers.py functionality.
 * 
 * Provides standardized asset import and manipulation functions
 * with consistent error handling and result reporting.
 */
UCLASS(BlueprintType)
class ADASTREADIRECTOR_API UAssetHelpers : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Import a texture file as a Texture2D asset.
	 * @param FilePath Full path to image file
	 * @param TargetFolder Target asset folder (e.g., "/Game/Textures")
	 * @param AssetName Optional name for the asset (uses filename if not provided)
	 * @return Result of the operation with asset_path in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Asset Import")
	static FAdastreaResult ImportTexture(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Textures"), const FString& AssetName = TEXT(""));

	/**
	 * Import a 3D model file as a StaticMesh asset.
	 * @param FilePath Full path to mesh file (FBX, OBJ, etc.)
	 * @param TargetFolder Target asset folder (e.g., "/Game/Meshes")
	 * @param AssetName Optional name for the asset (uses filename if not provided)
	 * @return Result of the operation with asset_path in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Asset Import")
	static FAdastreaResult ImportStaticMesh(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Meshes"), const FString& AssetName = TEXT(""));

	/**
	 * Import an audio file as a SoundWave asset.
	 * @param FilePath Full path to audio file (WAV, MP3, etc.)
	 * @param TargetFolder Target asset folder (e.g., "/Game/Audio")
	 * @param AssetName Optional name for the asset (uses filename if not provided)
	 * @return Result of the operation with asset_path in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Asset Import")
	static FAdastreaResult ImportAudio(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Audio"), const FString& AssetName = TEXT(""));

	/**
	 * Create a new Blueprint asset.
	 * @param BlueprintName Name for the new blueprint (e.g., "BP_MyActor")
	 * @param ParentClass Parent class for the blueprint (e.g., "Actor", "Pawn")
	 * @param PackagePath Directory path where to save the blueprint
	 * @return Result of the operation with asset_path in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Asset Creation")
	static FAdastreaResult CreateBlueprint(const FString& BlueprintName, const FString& ParentClass = TEXT("Actor"), const FString& PackagePath = TEXT("/Game/Blueprints"));

	/**
	 * Create a new Material asset.
	 * @param MaterialName Name for the new material
	 * @param PackagePath Directory path where to save the material
	 * @return Result of the operation with asset_path in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Asset Creation")
	static FAdastreaResult CreateMaterial(const FString& MaterialName, const FString& PackagePath = TEXT("/Game/Materials"));

private:
	/** Helper to generate asset name from file path if not provided */
	static FString GetAssetNameFromPath(const FString& FilePath, const FString& ProvidedName);

	/** Helper to perform generic asset import */
	static FAdastreaResult ImportAssetGeneric(const FString& FilePath, const FString& TargetFolder, const FString& AssetName, const FString& AssetType);
};
