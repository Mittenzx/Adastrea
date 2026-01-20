// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StandardResult.h"
#include "UObject/NoExportTypes.h"
#include "UEBridge.generated.h"

/**
 * Information about an Unreal Engine asset
 */
USTRUCT(BlueprintType)
struct FUEAssetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Asset")
	FString AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Asset")
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Asset")
	FString AssetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Asset")
	int64 AssetSize = 0;
};

/**
 * Information about an Unreal Engine actor
 */
USTRUCT(BlueprintType)
struct FUEActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Actor")
	FString ActorName;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Actor")
	FString ActorClass;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Actor")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Actor")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Adastrea|Actor")
	FVector Scale = FVector::OneVector;
};

/**
 * C++ Bridge class for interacting with Unreal Engine.
 * This is the C++ equivalent of the Python UEPythonBridge class.
 * 
 * Provides high-level methods for common UE operations:
 * - Asset management and queries
 * - Actor spawning and manipulation
 * - Console command execution
 * - Editor scripting utilities
 * - Level operations
 */
UCLASS(BlueprintType)
class ADASTREADIRECTOR_API UUEBridge : public UObject
{
	GENERATED_BODY()

public:
	UUEBridge();

	// ============================================================================
	// Console and Logging
	// ============================================================================

	/**
	 * Execute a console command in Unreal Engine.
	 * @param Command Console command to execute (e.g., "stat fps")
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Console")
	static FAdastreaResult ExecuteConsoleCommand(const FString& Command);

	/**
	 * Log a message to Unreal Engine's output log.
	 * @param Message Message to log
	 * @param bIsError Whether this is an error message
	 * @param bIsWarning Whether this is a warning message
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Console")
	static void LogMessage(const FString& Message, bool bIsError = false, bool bIsWarning = false);

	// ============================================================================
	// Asset Operations
	// ============================================================================

	/**
	 * Get information about currently selected assets in Content Browser.
	 * @param OutAssets Array of asset information
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets")
	static FAdastreaResult GetSelectedAssets(TArray<FUEAssetInfo>& OutAssets);

	/**
	 * Find all assets of a specific class in the project.
	 * @param AssetClass Asset class name (e.g., "StaticMesh", "Material")
	 * @param Path Root path to search (default: "/Game")
	 * @param OutAssets Array of found assets
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets")
	static FAdastreaResult FindAssetsByClass(const FString& AssetClass, const FString& Path, TArray<FUEAssetInfo>& OutAssets);

	/**
	 * Load an asset by its path.
	 * @param AssetPath Full asset path (e.g., "/Game/Materials/M_MyMaterial")
	 * @return Result of the operation with loaded asset reference
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets")
	static FAdastreaResult LoadAsset(const FString& AssetPath);

	/**
	 * Save an asset.
	 * @param AssetPath Full asset path to save
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Assets")
	static FAdastreaResult SaveAsset(const FString& AssetPath);

	// ============================================================================
	// Actor Operations
	// ============================================================================

	/**
	 * Get all actors of a specific class in the current level.
	 * @param ActorClass Actor class name (e.g., "StaticMeshActor", "PointLight")
	 * @param OutActors Array of actor information
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors")
	static FAdastreaResult GetAllActorsOfClass(const FString& ActorClass, TArray<FUEActorInfo>& OutActors);

	/**
	 * Get information about currently selected actors in the level.
	 * @param OutActors Array of actor information
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors")
	static FAdastreaResult GetSelectedActors(TArray<FUEActorInfo>& OutActors);

	/**
	 * Spawn a new actor in the current level.
	 * @param ActorClass Class name of actor to spawn
	 * @param Location Spawn location
	 * @param Rotation Spawn rotation
	 * @param ActorName Optional name for the actor (sets the actor label/display name)
	 * @return Result of the operation with actor_name (internal) and actor_label (display) in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors")
	static FAdastreaResult SpawnActor(const FString& ActorClass, FVector Location, FRotator Rotation, const FString& ActorName = TEXT(""));

	/**
	 * Delete an actor from the current level by name.
	 * @param ActorName Name of the actor to delete (accepts either internal name or display label)
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Actors")
	static FAdastreaResult DeleteActor(const FString& ActorName);

	// ============================================================================
	// Level and World Operations
	// ============================================================================

	/**
	 * Get the name of the currently loaded level.
	 * @return Result with level name in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level")
	static FAdastreaResult GetCurrentLevelName();

	/**
	 * Load a level by its path.
	 * @param LevelPath Path to the level (e.g., "/Game/Maps/MyLevel")
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level")
	static FAdastreaResult LoadLevel(const FString& LevelPath);

	/**
	 * Save the currently loaded level.
	 * @return Result of the operation
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Level")
	static FAdastreaResult SaveCurrentLevel();

	// ============================================================================
	// Editor Utilities
	// ============================================================================

	/**
	 * Get the project's root directory path.
	 * @return Result with project directory in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Editor")
	static FAdastreaResult GetProjectDirectory();

	/**
	 * Get the Unreal Engine version.
	 * @return Result with engine version in details
	 */
	UFUNCTION(BlueprintCallable, Category = "Adastrea|Editor")
	static FAdastreaResult GetEngineVersion();

private:
	/** Helper to get editor world */
	static UWorld* GetEditorWorld();

	/** Helper to convert actor to info struct */
	static FUEActorInfo ActorToInfo(AActor* Actor);
};
