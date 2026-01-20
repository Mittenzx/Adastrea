// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

/**
 * Example usage of Adastrea Director C++ API
 * 
 * NOTE: This header contains example code for documentation purposes.
 * It is recommended to copy the relevant examples into your own source files
 * (e.g., your own headers and .cpp files) rather than depending on this file directly.
 * 
 * This file demonstrates how to use the C++ bridge classes
 * to interact with Unreal Engine without Python dependencies.
 * 
 * These examples can be called from:
 * - C++ game code
 * - Editor utility widgets
 * - Blueprint functions
 * - Console commands
 * 
 * To use these examples:
 * 1. Include the necessary headers in your .cpp file:
 *    #include "UEBridge.h"
 *    #include "AssetHelpers.h"
 *    #include "StandardResult.h"
 * 2. Copy the function(s) you need
 * 3. Call them from your code
 */

// Forward declarations - include these headers in your .cpp file
class UUEBridge;
class UAssetHelpers;
struct FAdastreaResult;

// ============================================================================
// Example 1: Console Commands
// ============================================================================

void Example_ConsoleCommands()
{
	// Execute a simple console command
	FAdastreaResult Result = UUEBridge::ExecuteConsoleCommand("stat fps");
	
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Console command executed successfully"));
	}
	
	// Execute multiple commands
	UUEBridge::ExecuteConsoleCommand("r.SetRes 1920x1080w");
	UUEBridge::ExecuteConsoleCommand("stat unit");
	UUEBridge::ExecuteConsoleCommand("t.MaxFPS 60");
}

// ============================================================================
// Example 2: Asset Queries
// ============================================================================

void Example_AssetQueries()
{
	// Get selected assets in Content Browser
	TArray<FUEAssetInfo> SelectedAssets;
	FAdastreaResult Result = UUEBridge::GetSelectedAssets(SelectedAssets);
	
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Found %d selected assets"), SelectedAssets.Num());
		
		for (const FUEAssetInfo& Asset : SelectedAssets)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s (%s)"), *Asset.AssetName, *Asset.AssetClass);
		}
	}
	
	// Find all materials in the project
	TArray<FUEAssetInfo> Materials;
	UUEBridge::FindAssetsByClass("Material", "/Game", Materials);
	
	UE_LOG(LogTemp, Log, TEXT("Found %d materials"), Materials.Num());
	
	// Load a specific asset
	Result = UUEBridge::LoadAsset("/Game/Materials/M_MyMaterial");
	if (Result.IsSuccess())
	{
		FString AssetPath = Result.Details["asset_path"];
		FString AssetClass = Result.Details["asset_class"];
		UE_LOG(LogTemp, Log, TEXT("Loaded asset: %s (class: %s)"), *AssetPath, *AssetClass);
	}
}

// ============================================================================
// Example 3: Actor Operations
// ============================================================================

void Example_ActorOperations()
{
	// Get all static mesh actors in the level
	TArray<FUEActorInfo> Actors;
	FAdastreaResult Result = UUEBridge::GetAllActorsOfClass("StaticMeshActor", Actors);
	
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Found %d static mesh actors"), Actors.Num());
		
		for (const FUEActorInfo& Actor : Actors)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s at location %s"), 
				*Actor.ActorName, 
				*Actor.Location.ToString());
		}
	}
	
	// Get selected actors
	TArray<FUEActorInfo> SelectedActors;
	UUEBridge::GetSelectedActors(SelectedActors);
	
	UE_LOG(LogTemp, Log, TEXT("Selected actors: %d"), SelectedActors.Num());
	
	// Spawn a new actor
	FVector SpawnLocation(100.0f, 200.0f, 50.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	Result = UUEBridge::SpawnActor(
		"StaticMeshActor",
		SpawnLocation,
		SpawnRotation,
		"MySpawnedActor"
	);
	
	if (Result.IsSuccess())
	{
		FString ActorName = Result.Details["actor_name"];
		UE_LOG(LogTemp, Log, TEXT("Spawned actor: %s"), *ActorName);
	}
	
	// Delete an actor by name (be careful!)
	// Result = UUEBridge::DeleteActor("MySpawnedActor");
}

// ============================================================================
// Example 4: Level Operations
// ============================================================================

void Example_LevelOperations()
{
	// Get current level name
	FAdastreaResult Result = UUEBridge::GetCurrentLevelName();
	
	if (Result.IsSuccess())
	{
		FString LevelName = Result.Details["level_name"];
		UE_LOG(LogTemp, Log, TEXT("Current level: %s"), *LevelName);
	}
	
	// Save current level
	Result = UUEBridge::SaveCurrentLevel();
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Level saved successfully"));
	}
	
	// Load a different level (be careful!)
	// Result = UUEBridge::LoadLevel("/Game/Maps/TestLevel");
}

// ============================================================================
// Example 5: Asset Import
// ============================================================================

void Example_AssetImport()
{
	// Import a texture
	FAdastreaResult Result = UAssetHelpers::ImportTexture(
		"C:/Assets/MyTexture.png",
		"/Game/Textures",
		"ImportedTexture"
	);
	
	if (Result.IsSuccess())
	{
		FString AssetPath = Result.Details["asset_path"];
		UE_LOG(LogTemp, Log, TEXT("Imported texture: %s"), *AssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import texture: %s"), *Result.Message);
	}
	
	// Import a static mesh
	Result = UAssetHelpers::ImportStaticMesh(
		"C:/Assets/MyMesh.fbx",
		"/Game/Meshes",
		"ImportedMesh"
	);
	
	// Import audio
	Result = UAssetHelpers::ImportAudio(
		"C:/Assets/MySound.wav",
		"/Game/Audio",
		"ImportedSound"
	);
}

// ============================================================================
// Example 6: Asset Creation
// ============================================================================

void Example_AssetCreation()
{
	// Create a new Blueprint
	FAdastreaResult Result = UAssetHelpers::CreateBlueprint(
		"BP_MyActor",
		"Actor",
		"/Game/Blueprints"
	);
	
	if (Result.IsSuccess())
	{
		FString AssetPath = Result.Details["asset_path"];
		UE_LOG(LogTemp, Log, TEXT("Created blueprint: %s"), *AssetPath);
	}
	
	// Create a Blueprint with different parent class
	Result = UAssetHelpers::CreateBlueprint(
		"BP_MyCharacter",
		"Character",
		"/Game/Blueprints/Characters"
	);
	
	// Create a new Material
	Result = UAssetHelpers::CreateMaterial(
		"M_MyMaterial",
		"/Game/Materials"
	);
	
	if (Result.IsSuccess())
	{
		FString AssetPath = Result.Details["asset_path"];
		UE_LOG(LogTemp, Log, TEXT("Created material: %s"), *AssetPath);
	}
}

// ============================================================================
// Example 7: Editor Utilities
// ============================================================================

void Example_EditorUtilities()
{
	// Get project directory
	FAdastreaResult Result = UUEBridge::GetProjectDirectory();
	
	if (Result.IsSuccess())
	{
		FString ProjectDir = Result.Details["project_dir"];
		UE_LOG(LogTemp, Log, TEXT("Project directory: %s"), *ProjectDir);
	}
	
	// Get engine version
	Result = UUEBridge::GetEngineVersion();
	
	if (Result.IsSuccess())
	{
		FString EngineVersion = Result.Details["engine_version"];
		UE_LOG(LogTemp, Log, TEXT("Engine version: %s"), *EngineVersion);
	}
}

// ============================================================================
// Example 8: Error Handling
// ============================================================================

void Example_ErrorHandling()
{
	// Attempt to load a non-existent asset
	FAdastreaResult Result = UUEBridge::LoadAsset("/Game/NonExistent/Asset");
	
	if (Result.IsError())
	{
		// Log the error
		UE_LOG(LogTemp, Error, TEXT("Operation failed: %s"), *Result.Message);
		
		// Access error details if available
		for (const auto& Detail : Result.Details)
		{
			UE_LOG(LogTemp, Error, TEXT("  %s: %s"), *Detail.Key, *Detail.Value);
		}
	}
	
	// Example of proper error checking pattern
	Result = UUEBridge::SpawnActor("InvalidClass", FVector::ZeroVector, FRotator::ZeroRotator);
	
	if (Result.IsSuccess())
	{
		// Handle success
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Result.Message);
	}
	else
	{
		// Handle error
		UE_LOG(LogTemp, Warning, TEXT("Failed: %s"), *Result.Message);
	}
}

// ============================================================================
// Example 9: Batch Operations
// ============================================================================

void Example_BatchOperations()
{
	// Batch spawn multiple actors
	TArray<FVector> SpawnLocations = {
		FVector(0, 0, 0),
		FVector(100, 0, 0),
		FVector(200, 0, 0),
		FVector(300, 0, 0)
	};
	
	int32 SuccessCount = 0;
	int32 FailureCount = 0;
	
	for (int32 i = 0; i < SpawnLocations.Num(); i++)
	{
		FString ActorName = FString::Printf(TEXT("SpawnedActor_%d"), i);
		FAdastreaResult Result = UUEBridge::SpawnActor(
			"StaticMeshActor",
			SpawnLocations[i],
			FRotator::ZeroRotator,
			ActorName
		);
		
		if (Result.IsSuccess())
		{
			SuccessCount++;
		}
		else
		{
			FailureCount++;
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn actor %d: %s"), i, *Result.Message);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Batch spawn complete: %d succeeded, %d failed"), 
		SuccessCount, FailureCount);
}

// ============================================================================
// Example 10: Logging Helper
// ============================================================================

void Example_Logging()
{
	// Log various message types
	UUEBridge::LogMessage("This is a normal log message");
	UUEBridge::LogMessage("This is a warning message", false, true);
	UUEBridge::LogMessage("This is an error message", true, false);
	
	// These will appear in the Output Log with appropriate colors
}

// ============================================================================
// Integration Example: Complete Workflow
// ============================================================================

void Example_CompleteWorkflow()
{
	UE_LOG(LogTemp, Log, TEXT("=== Starting Complete Workflow Example ==="));
	
	// 1. Get project info
	FAdastreaResult Result = UUEBridge::GetProjectDirectory();
	UE_LOG(LogTemp, Log, TEXT("Project: %s"), *Result.Details["project_dir"]);
	
	// 2. Get current level
	Result = UUEBridge::GetCurrentLevelName();
	UE_LOG(LogTemp, Log, TEXT("Level: %s"), *Result.Details["level_name"]);
	
	// 3. Find all materials
	TArray<FUEAssetInfo> Materials;
	UUEBridge::FindAssetsByClass("Material", "/Game", Materials);
	UE_LOG(LogTemp, Log, TEXT("Found %d materials"), Materials.Num());
	
	// 4. Get selected actors
	TArray<FUEActorInfo> SelectedActors;
	UUEBridge::GetSelectedActors(SelectedActors);
	UE_LOG(LogTemp, Log, TEXT("Selected actors: %d"), SelectedActors.Num());
	
	// 5. Create a new material
	Result = UAssetHelpers::CreateMaterial("M_ExampleMaterial", "/Game/Materials");
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Created material: %s"), *Result.Details["asset_path"]);
	}
	
	// 6. Create a new blueprint
	Result = UAssetHelpers::CreateBlueprint("BP_Example", "Actor", "/Game/Blueprints");
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Created blueprint: %s"), *Result.Details["asset_path"]);
	}
	
	// 7. Spawn an actor
	Result = UUEBridge::SpawnActor(
		"StaticMeshActor",
		FVector(0, 0, 100),
		FRotator::ZeroRotator,
		"ExampleActor"
	);
	
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned actor: %s"), *Result.Details["actor_name"]);
	}
	
	UE_LOG(LogTemp, Log, TEXT("=== Workflow Complete ==="));
}
