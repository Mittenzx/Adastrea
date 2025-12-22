// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StandardResult.h"
#include "UEBridge.h"
#include "AssetHelpers.h"
#include "AdastreaDirectorBlueprintLibrary.generated.h"

/**
 * Blueprint Function Library for Adastrea Director C++ API
 * 
 * This library provides easy access to all Adastrea Director functionality
 * from Blueprints. It wraps the UEBridge and AssetHelpers classes with
 * more Blueprint-friendly interfaces.
 */
UCLASS()
class ADASTREADIRECTOR_API UAdastreaDirectorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Console and Logging
	// ============================================================================

	/**
	 * Execute a console command in Unreal Engine.
	 * Example: "stat fps", "r.SetRes 1920x1080w"
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Console", meta = (Keywords = "console command execute"))
	static FAdastreaResult ExecuteConsoleCommand(const FString& Command);

	/**
	 * Log a message to Unreal Engine's output log.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Console", meta = (Keywords = "log message print"))
	static void LogMessage(const FString& Message, bool bIsError = false, bool bIsWarning = false);

	// ============================================================================
	// Asset Operations
	// ============================================================================

	/**
	 * Get information about currently selected assets in Content Browser.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets", meta = (Keywords = "asset selected get"))
	static FAdastreaResult GetSelectedAssets(TArray<FUEAssetInfo>& OutAssets);

	/**
	 * Find all assets of a specific class in the project.
	 * Example: FindAssetsByClass("Material", "/Game/Materials")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets", meta = (Keywords = "asset find search class"))
	static FAdastreaResult FindAssetsByClass(const FString& AssetClass, const FString& Path, TArray<FUEAssetInfo>& OutAssets);

	/**
	 * Load an asset by its path.
	 * Example: "/Game/Materials/M_MyMaterial"
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets", meta = (Keywords = "asset load"))
	static FAdastreaResult LoadAsset(const FString& AssetPath);

	/**
	 * Save an asset by its path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets", meta = (Keywords = "asset save"))
	static FAdastreaResult SaveAsset(const FString& AssetPath);

	// ============================================================================
	// Actor Operations
	// ============================================================================

	/**
	 * Get all actors of a specific class in the current level.
	 * Example: "StaticMeshActor", "PointLight"
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors", meta = (Keywords = "actor get all class"))
	static FAdastreaResult GetAllActorsOfClass(const FString& ActorClass, TArray<FUEActorInfo>& OutActors);

	/**
	 * Get information about currently selected actors in the level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors", meta = (Keywords = "actor selected get"))
	static FAdastreaResult GetSelectedActors(TArray<FUEActorInfo>& OutActors);

	/**
	 * Spawn a new actor in the current level.
	 * Example: SpawnActor("StaticMeshActor", Location, Rotation, "MyActor")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors", meta = (Keywords = "actor spawn create"))
	static FAdastreaResult SpawnActor(const FString& ActorClass, FVector Location, FRotator Rotation, const FString& ActorName = TEXT(""));

	/**
	 * Delete an actor from the current level by name.
	 * WARNING: This cannot be undone!
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors", meta = (Keywords = "actor delete remove destroy"))
	static FAdastreaResult DeleteActor(const FString& ActorName);

	// ============================================================================
	// Level Operations
	// ============================================================================

	/**
	 * Get the name of the currently loaded level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level", meta = (Keywords = "level current get name"))
	static FAdastreaResult GetCurrentLevelName();

	/**
	 * Load a level by its path.
	 * Example: "/Game/Maps/MyLevel"
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level", meta = (Keywords = "level load open"))
	static FAdastreaResult LoadLevel(const FString& LevelPath);

	/**
	 * Save the currently loaded level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level", meta = (Keywords = "level save"))
	static FAdastreaResult SaveCurrentLevel();

	// ============================================================================
	// Asset Import
	// ============================================================================

	/**
	 * Import a texture file as a Texture2D asset.
	 * Example: ImportTexture("C:/Assets/MyTexture.png", "/Game/Textures", "MyTexture")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Import", meta = (Keywords = "texture import image"))
	static FAdastreaResult ImportTexture(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Textures"), const FString& AssetName = TEXT(""));

	/**
	 * Import a 3D model file as a StaticMesh asset.
	 * Example: ImportStaticMesh("C:/Assets/MyMesh.fbx", "/Game/Meshes", "MyMesh")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Import", meta = (Keywords = "mesh import model fbx"))
	static FAdastreaResult ImportStaticMesh(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Meshes"), const FString& AssetName = TEXT(""));

	/**
	 * Import an audio file as a SoundWave asset.
	 * Example: ImportAudio("C:/Assets/MySound.wav", "/Game/Audio", "MySound")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Import", meta = (Keywords = "audio import sound wav"))
	static FAdastreaResult ImportAudio(const FString& FilePath, const FString& TargetFolder = TEXT("/Game/Audio"), const FString& AssetName = TEXT(""));

	// ============================================================================
	// Asset Creation
	// ============================================================================

	/**
	 * Create a new Blueprint asset.
	 * Example: CreateBlueprint("BP_MyActor", "Actor", "/Game/Blueprints")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Create", meta = (Keywords = "blueprint create new"))
	static FAdastreaResult CreateBlueprint(const FString& BlueprintName, const FString& ParentClass = TEXT("Actor"), const FString& PackagePath = TEXT("/Game/Blueprints"));

	/**
	 * Create a new Material asset.
	 * Example: CreateMaterial("M_MyMaterial", "/Game/Materials")
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Create", meta = (Keywords = "material create new"))
	static FAdastreaResult CreateMaterial(const FString& MaterialName, const FString& PackagePath = TEXT("/Game/Materials"));

	// ============================================================================
	// Editor Utilities
	// ============================================================================

	/**
	 * Get the project's root directory path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Editor", meta = (Keywords = "project directory path"))
	static FAdastreaResult GetProjectDirectory();

	/**
	 * Get the Unreal Engine version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Editor", meta = (Keywords = "engine version"))
	static FAdastreaResult GetEngineVersion();

	// ============================================================================
	// Result Helpers
	// ============================================================================

	/**
	 * Check if a result indicates success.
	 */
	UFUNCTION(BlueprintPure, Category = "Adastrea|Result", meta = (Keywords = "result success check"))
	static bool IsResultSuccess(const FAdastreaResult& Result);

	/**
	 * Check if a result indicates an error.
	 */
	UFUNCTION(BlueprintPure, Category = "Adastrea|Result", meta = (Keywords = "result error check"))
	static bool IsResultError(const FAdastreaResult& Result);

	/**
	 * Get a detail value from a result by key.
	 */
	UFUNCTION(BlueprintPure, Category = "Adastrea|Result", meta = (Keywords = "result detail get"))
	static FString GetResultDetail(const FAdastreaResult& Result, const FString& Key);

	/**
	 * Get the status message from a result.
	 */
	UFUNCTION(BlueprintPure, Category = "Adastrea|Result", meta = (Keywords = "result message get"))
	static FString GetResultMessage(const FAdastreaResult& Result);
};
