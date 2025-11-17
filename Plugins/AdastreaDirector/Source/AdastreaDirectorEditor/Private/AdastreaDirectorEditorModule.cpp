// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaDirectorEditorModule.h"
#include "SAdastreaDirectorPanel.h"
#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"

#if WITH_EDITOR
	#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
	#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#endif

// Define custom log category for AdastreaDirectorEditor
DEFINE_LOG_CATEGORY(LogAdastreaDirectorEditor);

#define LOCTEXT_NAMESPACE "FAdastreaDirectorEditorModule"

static const FName AdastreaDirectorTabName("AdastreaDirector");

void FAdastreaDirectorEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("AdastreaDirector Editor Module: StartupModule"));
	
	RegisterTabSpawner();
	RegisterMenuExtensions();
}

void FAdastreaDirectorEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("AdastreaDirector Editor Module: ShutdownModule"));
	
	UnregisterMenuExtensions();
	UnregisterTabSpawner();
}

void FAdastreaDirectorEditorModule::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		AdastreaDirectorTabName,
		FOnSpawnTab::CreateRaw(this, &FAdastreaDirectorEditorModule::SpawnAdastreaDirectorTab))
		.SetDisplayName(LOCTEXT("AdastreaDirectorTabTitle", "Adastrea Director"))
		.SetTooltipText(LOCTEXT("AdastreaDirectorTabTooltip", "Opens the Adastrea Director AI assistant panel"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics"));

	UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("Registered Adastrea Director tab spawner"));
}

void FAdastreaDirectorEditorModule::UnregisterTabSpawner()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AdastreaDirectorTabName);
		UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("Unregistered Adastrea Director tab spawner"));
	}
}

TSharedRef<SDockTab> FAdastreaDirectorEditorModule::SpawnAdastreaDirectorTab(const FSpawnTabArgs& Args)
{
	UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("Spawning Adastrea Director tab"));

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAdastreaDirectorPanel)
		];
}

void FAdastreaDirectorEditorModule::RegisterMenuExtensions()
{
	// Menu extensions are handled by the tab spawner registration
	// The tab will appear in Window > Developer Tools menu
	UE_LOG(LogAdastreaDirectorEditor, Log, TEXT("Menu extensions registered"));
}

void FAdastreaDirectorEditorModule::UnregisterMenuExtensions()
{
	// Cleanup is handled by UnregisterTabSpawner
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAdastreaDirectorEditorModule, AdastreaDirectorEditor)
