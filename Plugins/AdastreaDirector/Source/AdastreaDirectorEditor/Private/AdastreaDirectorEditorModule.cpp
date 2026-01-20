// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaDirectorEditorModule.h"
#include "SAdastreaDirectorPanel.h"
#include "SSettingsDialog.h"
#include "AdastreaDirectorModule.h"
#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
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

	// Check if runtime module is fully initialized
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule || !RuntimeModule->IsFullyInitialized())
	{
		FString ErrorMessage = RuntimeModule ? RuntimeModule->GetInitializationError() : TEXT("Runtime module not loaded");
		
		UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Cannot spawn Adastrea Director tab - initialization failed: %s"), *ErrorMessage);
		
		// Create error display tab instead of main panel
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(20.0f)
				[
					SNew(SVerticalBox)
					
					// Error Icon and Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("⚠️ Adastrea Director - Initialization Failed")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor(1.0f, 0.4f, 0.0f))
					]
					
					// Error Message
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ErrorMessage))
						.AutoWrapText(true)
					]
					
					// Help Text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 20.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(
							TEXT("To resolve this issue:\n\n")
							TEXT("1. Create/edit .env file in your project root (copy from .env.example)\n")
							TEXT("2. Add your API key: GEMINI_KEY=your-api-key-here\n")
							TEXT("3. Ensure Python backend is properly installed\n")
							TEXT("4. Check the Output Log for detailed error information\n")
							TEXT("5. Restart Unreal Engine after fixing the issue")
						))
						.AutoWrapText(true)
					]
					
					// Settings Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Open Settings")))
						.OnClicked_Lambda([]() -> FReply {
							SSettingsDialog::OpenDialog();
							return FReply::Handled();
						})
					]
				]
			];
	}

	// Normal initialization - create main panel
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
