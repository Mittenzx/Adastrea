// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "UEBridge.h"
#include "AdastreaDirectorModule.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "FileHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

UUEBridge::UUEBridge()
{
}

// ============================================================================
// Console and Logging
// ============================================================================

FAdastreaResult UUEBridge::ExecuteConsoleCommand(const FString& Command)
{
	if (Command.IsEmpty())
	{
		return FAdastreaResult::MakeError(TEXT("Command cannot be empty"));
	}

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	GEngine->Exec(World, *Command);
	
	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Executed console command: %s"), *Command));
	Result.AddDetail(TEXT("command"), Command);
	return Result;
}

void UUEBridge::LogMessage(const FString& Message, bool bIsError, bool bIsWarning)
{
	if (bIsError)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("%s"), *Message);
	}
	else if (bIsWarning)
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("%s"), *Message);
	}
}

// ============================================================================
// Asset Operations
// ============================================================================

FAdastreaResult UUEBridge::GetSelectedAssets(TArray<FUEAssetInfo>& OutAssets)
{
	OutAssets.Empty();

#if WITH_EDITOR
	// UE 5.6+: Use ContentBrowserModule to get selected assets
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (AssetData.IsValid())
		{
			FUEAssetInfo Info;
			Info.AssetName = AssetData.AssetName.ToString();
			Info.AssetPath = AssetData.GetObjectPathString();
			Info.AssetClass = AssetData.AssetClassPath.ToString();
			OutAssets.Add(Info);
		}
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Retrieved %d selected assets"), OutAssets.Num()));
	Result.AddDetail(TEXT("count"), FString::FromInt(OutAssets.Num()));
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

FAdastreaResult UUEBridge::FindAssetsByClass(const FString& AssetClass, const FString& Path, TArray<FUEAssetInfo>& OutAssets)
{
	OutAssets.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Create filter
	FARFilter Filter;
	FTopLevelAssetPath ClassPath = AssetClass.StartsWith(TEXT("/")) || AssetClass.Contains(TEXT("."))
		? FTopLevelAssetPath(AssetClass)
		: FTopLevelAssetPath(TEXT("/Script/Engine"), *AssetClass);
	Filter.ClassPaths.Add(ClassPath);
	Filter.PackagePaths.Add(FName(*Path));
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		FUEAssetInfo Info;
		Info.AssetName = AssetData.AssetName.ToString();
		Info.AssetPath = AssetData.GetObjectPathString();
		Info.AssetClass = AssetData.AssetClassPath.ToString();
		OutAssets.Add(Info);
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Found %d assets of class '%s'"), OutAssets.Num(), *AssetClass));
	Result.AddDetail(TEXT("count"), FString::FromInt(OutAssets.Num()));
	Result.AddDetail(TEXT("class"), AssetClass);
	Result.AddDetail(TEXT("path"), Path);
	return Result;
}

FAdastreaResult UUEBridge::LoadAsset(const FString& AssetPath)
{
	if (AssetPath.IsEmpty())
	{
		return FAdastreaResult::MakeError(TEXT("Asset path cannot be empty"));
	}

	UObject* LoadedAsset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!LoadedAsset)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Loaded asset: %s"), *AssetPath));
	Result.AddDetail(TEXT("asset_path"), AssetPath);
	Result.AddDetail(TEXT("asset_name"), LoadedAsset->GetName());
	Result.AddDetail(TEXT("asset_class"), LoadedAsset->GetClass()->GetName());
	return Result;
}

FAdastreaResult UUEBridge::SaveAsset(const FString& AssetPath)
{
#if WITH_EDITOR
	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!EditorAssetSubsystem)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get EditorAssetSubsystem"));
	}

	bool bSaved = EditorAssetSubsystem->SaveAsset(AssetPath, false);
	if (bSaved)
	{
		FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Saved asset: %s"), *AssetPath));
		Result.AddDetail(TEXT("asset_path"), AssetPath);
		return Result;
	}
	
	return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to save asset: %s"), *AssetPath));
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

// ============================================================================
// Actor Operations
// ============================================================================

FAdastreaResult UUEBridge::GetAllActorsOfClass(const FString& ActorClass, TArray<FUEActorInfo>& OutActors)
{
	OutActors.Empty();

	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	// Try to find the class
	UClass* Class = FindObject<UClass>(nullptr, *ActorClass);
	if (!Class)
	{
		// Try with full path
		FString FullPath = FString::Printf(TEXT("/Script/Engine.%s"), *ActorClass);
		Class = FindObject<UClass>(nullptr, *FullPath);
	}

	if (!Class)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Actor class not found: %s"), *ActorClass));
	}

	// Get all actors of the class
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, Class, Actors);

	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			OutActors.Add(ActorToInfo(Actor));
		}
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Found %d actors of class '%s'"), OutActors.Num(), *ActorClass));
	Result.AddDetail(TEXT("count"), FString::FromInt(OutActors.Num()));
	Result.AddDetail(TEXT("class"), ActorClass);
	return Result;
}

FAdastreaResult UUEBridge::GetSelectedActors(TArray<FUEActorInfo>& OutActors)
{
	OutActors.Empty();

#if WITH_EDITOR
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	if (!EditorActorSubsystem)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get EditorActorSubsystem"));
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	
	for (AActor* Actor : SelectedActors)
	{
		if (Actor)
		{
			OutActors.Add(ActorToInfo(Actor));
		}
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Retrieved %d selected actors"), OutActors.Num()));
	Result.AddDetail(TEXT("count"), FString::FromInt(OutActors.Num()));
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

FAdastreaResult UUEBridge::SpawnActor(const FString& ActorClass, FVector Location, FRotator Rotation, const FString& ActorName)
{
#if WITH_EDITOR
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	// Try to find the class
	UClass* Class = FindObject<UClass>(nullptr, *ActorClass);
	if (!Class)
	{
		// Try with full path
		FString FullPath = FString::Printf(TEXT("/Script/Engine.%s"), *ActorClass);
		Class = FindObject<UClass>(nullptr, *FullPath);
	}

	if (!Class)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Actor class not found: %s"), *ActorClass));
	}

	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	if (!EditorActorSubsystem)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get EditorActorSubsystem"));
	}

	AActor* SpawnedActor = EditorActorSubsystem->SpawnActorFromClass(Class, Location, Rotation);
	if (!SpawnedActor)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to spawn actor of class '%s'"), *ActorClass));
	}

	if (!ActorName.IsEmpty())
	{
		SpawnedActor->SetActorLabel(ActorName);
	}

	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Spawned actor: %s at location (%s)"), 
		*SpawnedActor->GetActorLabel(), *Location.ToString()));
	Result.AddDetail(TEXT("actor_name"), SpawnedActor->GetName());
	Result.AddDetail(TEXT("actor_label"), SpawnedActor->GetActorLabel());
	Result.AddDetail(TEXT("actor_class"), ActorClass);
	
	// Note: actor_name is the internal name (GetName()), actor_label is the display name (GetActorLabel())
	// DeleteActor() accepts either name for lookup flexibility
	return Result;
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

FAdastreaResult UUEBridge::DeleteActor(const FString& ActorName)
{
#if WITH_EDITOR
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	// Find the actor (check both internal name and label for flexibility)
	AActor* FoundActor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && (Actor->GetName() == ActorName || Actor->GetActorLabel() == ActorName))
		{
			FoundActor = Actor;
			break;
		}
	}

	if (!FoundActor)
	{
		return FAdastreaResult::MakeError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	if (!EditorActorSubsystem)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get EditorActorSubsystem"));
	}

	bool bDestroyed = EditorActorSubsystem->DestroyActor(FoundActor);
	if (bDestroyed)
	{
		FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Deleted actor: %s"), *ActorName));
		Result.AddDetail(TEXT("actor_name"), ActorName);
		return Result;
	}

	return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to delete actor: %s"), *ActorName));
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

// ============================================================================
// Level and World Operations
// ============================================================================

FAdastreaResult UUEBridge::GetCurrentLevelName()
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	FString LevelName = World->GetName();
	
	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Current level: %s"), *LevelName));
	Result.AddDetail(TEXT("level_name"), LevelName);
	return Result;
}

FAdastreaResult UUEBridge::LoadLevel(const FString& LevelPath)
{
#if WITH_EDITOR
	if (LevelPath.IsEmpty())
	{
		return FAdastreaResult::MakeError(TEXT("Level path cannot be empty"));
	}

	FWorldContext& WorldContext = GEditor->GetEditorWorldContext();
	FURL URL(*LevelPath);
	FString Error;
	bool bLoaded = GEditor->LoadMap(WorldContext, URL, nullptr, Error);
	if (bLoaded)
	{
		FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Loaded level: %s"), *LevelPath));
		Result.AddDetail(TEXT("level_path"), LevelPath);
		return Result;
	}

	return FAdastreaResult::MakeError(FString::Printf(TEXT("Failed to load level: %s"), *LevelPath));
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

FAdastreaResult UUEBridge::SaveCurrentLevel()
{
#if WITH_EDITOR
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FAdastreaResult::MakeError(TEXT("Failed to get editor world"));
	}

	bool bSaved = FEditorFileUtils::SaveLevel(World->GetCurrentLevel());
	if (bSaved)
	{
		return FAdastreaResult::MakeSuccess(TEXT("Saved current level"));
	}

	return FAdastreaResult::MakeError(TEXT("Failed to save current level"));
#else
	return FAdastreaResult::MakeError(TEXT("Editor-only functionality"));
#endif
}

// ============================================================================
// Editor Utilities
// ============================================================================

FAdastreaResult UUEBridge::GetProjectDirectory()
{
	FString ProjectDir = FPaths::ProjectDir();
	
	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Project directory: %s"), *ProjectDir));
	Result.AddDetail(TEXT("project_dir"), ProjectDir);
	return Result;
}

FAdastreaResult UUEBridge::GetEngineVersion()
{
	FString EngineVersion = FEngineVersion::Current().ToString();
	
	FAdastreaResult Result = FAdastreaResult::MakeSuccess(FString::Printf(TEXT("Engine version: %s"), *EngineVersion));
	Result.AddDetail(TEXT("engine_version"), EngineVersion);
	return Result;
}

// ============================================================================
// Private Helpers
// ============================================================================

UWorld* UUEBridge::GetEditorWorld()
{
#if WITH_EDITOR
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
#endif
	return nullptr;
}

FUEActorInfo UUEBridge::ActorToInfo(AActor* Actor)
{
	FUEActorInfo Info;
	if (Actor)
	{
		Info.ActorName = Actor->GetName();
		Info.ActorClass = Actor->GetClass()->GetName();
		Info.Location = Actor->GetActorLocation();
		Info.Rotation = Actor->GetActorRotation();
		Info.Scale = Actor->GetActorScale3D();
	}
	return Info;
}
