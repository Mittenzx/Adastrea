// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaDirectorBlueprintLibrary.h"

// ============================================================================
// Console and Logging
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::ExecuteConsoleCommand(const FString& Command)
{
	return UUEBridge::ExecuteConsoleCommand(Command);
}

void UAdastreaDirectorBlueprintLibrary::LogMessage(const FString& Message, bool bIsError, bool bIsWarning)
{
	UUEBridge::LogMessage(Message, bIsError, bIsWarning);
}

// ============================================================================
// Asset Operations
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetSelectedAssets(TArray<FUEAssetInfo>& OutAssets)
{
	return UUEBridge::GetSelectedAssets(OutAssets);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::FindAssetsByClass(const FString& AssetClass, const FString& Path, TArray<FUEAssetInfo>& OutAssets)
{
	return UUEBridge::FindAssetsByClass(AssetClass, Path, OutAssets);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::LoadAsset(const FString& AssetPath)
{
	return UUEBridge::LoadAsset(AssetPath);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::SaveAsset(const FString& AssetPath)
{
	return UUEBridge::SaveAsset(AssetPath);
}

// ============================================================================
// Actor Operations
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetAllActorsOfClass(const FString& ActorClass, TArray<FUEActorInfo>& OutActors)
{
	return UUEBridge::GetAllActorsOfClass(ActorClass, OutActors);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetSelectedActors(TArray<FUEActorInfo>& OutActors)
{
	return UUEBridge::GetSelectedActors(OutActors);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::SpawnActor(const FString& ActorClass, FVector Location, FRotator Rotation, const FString& ActorName)
{
	return UUEBridge::SpawnActor(ActorClass, Location, Rotation, ActorName);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::DeleteActor(const FString& ActorName)
{
	return UUEBridge::DeleteActor(ActorName);
}

// ============================================================================
// Level Operations
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetCurrentLevelName()
{
	return UUEBridge::GetCurrentLevelName();
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::LoadLevel(const FString& LevelPath)
{
	return UUEBridge::LoadLevel(LevelPath);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::SaveCurrentLevel()
{
	return UUEBridge::SaveCurrentLevel();
}

// ============================================================================
// Asset Import
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::ImportTexture(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return UAssetHelpers::ImportTexture(FilePath, TargetFolder, AssetName);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::ImportStaticMesh(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return UAssetHelpers::ImportStaticMesh(FilePath, TargetFolder, AssetName);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::ImportAudio(const FString& FilePath, const FString& TargetFolder, const FString& AssetName)
{
	return UAssetHelpers::ImportAudio(FilePath, TargetFolder, AssetName);
}

// ============================================================================
// Asset Creation
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::CreateBlueprint(const FString& BlueprintName, const FString& ParentClass, const FString& PackagePath)
{
	return UAssetHelpers::CreateBlueprint(BlueprintName, ParentClass, PackagePath);
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::CreateMaterial(const FString& MaterialName, const FString& PackagePath)
{
	return UAssetHelpers::CreateMaterial(MaterialName, PackagePath);
}

// ============================================================================
// Editor Utilities
// ============================================================================

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetProjectDirectory()
{
	return UUEBridge::GetProjectDirectory();
}

FAdastreaResult UAdastreaDirectorBlueprintLibrary::GetEngineVersion()
{
	return UUEBridge::GetEngineVersion();
}

// ============================================================================
// Result Helpers
// ============================================================================

bool UAdastreaDirectorBlueprintLibrary::IsResultSuccess(const FAdastreaResult& Result)
{
	return Result.IsSuccess();
}

bool UAdastreaDirectorBlueprintLibrary::IsResultError(const FAdastreaResult& Result)
{
	return Result.IsError();
}

FString UAdastreaDirectorBlueprintLibrary::GetResultDetail(const FAdastreaResult& Result, const FString& Key)
{
	const FString* Value = Result.Details.Find(Key);
	return Value ? *Value : FString();
}

FString UAdastreaDirectorBlueprintLibrary::GetResultMessage(const FAdastreaResult& Result)
{
	return Result.Message;
}
