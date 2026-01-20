// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "SAdastreaDirectorPanel.h"
#include "SSettingsDialog.h"
#include "SStatusIndicator.h"
#include "AdastreaDirectorEditorModule.h"
#include "AdastreaDirectorModule.h"
#include "AdastreaSettings.h"
#include "AdastreaStartupValidator.h"
// Legacy IPC components removed in Phase 3 migration
// Use VibeUE components: AdastreaLLMClient, AdastreaScriptService, AdastreaAssetService
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SCheckBox.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "AdastreaDirectorPanel"

// Plugin name constant for consistency
static const FString PluginName(TEXT("AdastreaDirector"));

SAdastreaDirectorPanel::~SAdastreaDirectorPanel()
{
	// Clean up progress file if it exists
	if (!ProgressFilePath.IsEmpty() && FPaths::FileExists(ProgressFilePath))
	{
		IFileManager::Get().Delete(*ProgressFilePath);
	}
}

FString SAdastreaDirectorPanel::GetPluginVersion()
{
	// Cache the version string to avoid repeated plugin manager lookups
	static FString CachedVersion;
	static bool bVersionCached = false;
	
	if (!bVersionCached)
	{
		// Get the plugin descriptor to read the version
		TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (Plugin.IsValid())
		{
			const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
			CachedVersion = Descriptor.VersionName;
		}
		else
		{
			CachedVersion = TEXT("Unknown");
		}
		bVersionCached = true;
	}
	
	return CachedVersion;
}

void SAdastreaDirectorPanel::Construct(const FArguments& InArgs)
{
	// Initialize state
	bIsProcessing = false;
	bIsIngesting = false;
	IngestionProgress = 0.0f;
	IngestionStatusMessage = LOCTEXT("IngestionIdle", "Ready to ingest documents");
	IngestionDetailsMessage = FText::GetEmpty();
	DatabaseStatusMessage = LOCTEXT("DbStatusNotLoaded", "Not loaded - Click Refresh to check");
	CurrentResults = LOCTEXT("WelcomeMessage", "Welcome to Adastrea Director!\n\nEnter a query above and click 'Send Query' or press Enter to get started.\n\nExample: \"What is Unreal Engine?\"");
	LastProgressUpdateTime = 0.0;
	CurrentTabIndex = 0; // Start with Query tab
	LastDashboardRefreshTime = 0.0;
	LastConnectionStatusUpdateTime = 0.0;
	CurrentLogContent = TEXT("Dashboard logs will appear here...");
	CachedLogContentText = FText::FromString(CurrentLogContent);
	CachedConnectionStatus = FText::FromString(TEXT("⚠️ Not connected - Python backend not ready"));
	LastStatusLightsUpdateTime = 0.0;
	
	// Initialize ingestion debug log
	CurrentIngestionDebugLog = TEXT("📋 Ingestion Debug Log\n\nDebug messages will appear here when you start ingestion.\nThis shows exactly what's happening during the ingestion process.\n");
	CachedIngestionDebugLogText = FText::FromString(CurrentIngestionDebugLog);
	
	// Initialize Tests tab state
	bIsTestRunning = false;
	TestProgress = 0.0f;
	TestStatusMessage = LOCTEXT("TestsIdle", "Ready to run tests");
	CurrentTestOutput = TEXT("🧪 Plugin Self-Test Suite\n\nClick a test button above to run tests.\nResults will appear here.\n");
	CachedTestOutputText = FText::FromString(CurrentTestOutput);
	LastTestOutputUpdateTime = 0.0;
	
	// Setup progress file path
	ProgressFilePath = FPaths::ProjectIntermediateDir() / TEXT("AdastreaDirector") / TEXT("ingestion_progress.json");

	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelTitle", "Adastrea Director - AI Assistant"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				]
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Version %s"), *GetPluginVersion())))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
				]
			]
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SettingsButton", "Settings"))
				.ToolTipText(LOCTEXT("SettingsTooltip", "Open Settings (Ctrl+, - requires panel focus)"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnSettingsClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Tab buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			
			// Query Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.IsChecked(this, &SAdastreaDirectorPanel::GetTabButtonCheckedState, 0)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					if (NewState == ECheckBoxState::Checked)
					{
						OnTabButtonClicked(0);
					}
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QueryTabButton", "Query"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]

			// Ingestion Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.IsChecked(this, &SAdastreaDirectorPanel::GetTabButtonCheckedState, 1)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					if (NewState == ECheckBoxState::Checked)
					{
						OnTabButtonClicked(1);
					}
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IngestionTabButton", "Ingestion"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]

			// Dashboard Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.IsChecked(this, &SAdastreaDirectorPanel::GetTabButtonCheckedState, 2)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					if (NewState == ECheckBoxState::Checked)
					{
						OnTabButtonClicked(2);
					}
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DashboardTabButton", "Dashboard"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]

			// Tests Tab Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.IsChecked(this, &SAdastreaDirectorPanel::GetTabButtonCheckedState, 3)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					if (NewState == ECheckBoxState::Checked)
					{
						OnTabButtonClicked(3);
					}
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TestsTabButton", "Tests"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 5.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Tab content area with widget switcher
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TabContentSwitcher, SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() { return CurrentTabIndex; })
			
			// Query Tab (index 0)
			+ SWidgetSwitcher::Slot()
			[
				CreateQueryTab()
			]
			
			// Ingestion Tab (index 1)
			+ SWidgetSwitcher::Slot()
			[
				CreateIngestionTab()
			]
			
			// Dashboard Tab (index 2)
			+ SWidgetSwitcher::Slot()
			[
				CreateDashboardTab()
			]
			
			// Tests Tab (index 3)
			+ SWidgetSwitcher::Slot()
			[
				CreateTestsTab()
			]
		]
	];
}

TSharedRef<SWidget> SAdastreaDirectorPanel::CreateQueryTab()
{
	return SNew(SVerticalBox)
		
		// Query Input Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QueryLabel", "Query:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			
			// Query Input Box
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SAssignNew(QueryInputBox, SEditableTextBox)
				.HintText(LOCTEXT("QueryHint", "Enter your query here..."))
				.OnTextChanged(this, &SAdastreaDirectorPanel::OnQueryTextChanged)
				.OnTextCommitted(this, &SAdastreaDirectorPanel::OnQueryTextCommitted)
			]

			// Send Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SendButton", "Send Query"))
				.ToolTipText(LOCTEXT("SendButtonTooltip", "Send query to Python backend"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnSendQueryClicked)
				.IsEnabled(this, &SAdastreaDirectorPanel::IsSendButtonEnabled)
			]

			// Clear History Button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearHistoryButton", "Clear History"))
				.ToolTipText(LOCTEXT("ClearHistoryTooltip", "Clear conversation history"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnClearHistoryClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 10.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Results Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ResultsLabel", "Results:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(200.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				
				+ SScrollBox::Slot()
				[
					SAssignNew(ResultsDisplay, SMultiLineEditableTextBox)
					.Text_Lambda([this]() { return CurrentResults; })
					.IsReadOnly(true)
					.AutoWrapText(true)
				]
			]
		];
}

TSharedRef<SWidget> SAdastreaDirectorPanel::CreateIngestionTab()
{
	return SNew(SVerticalBox)
		
		// Docs Path Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DocsPathLabel", "Documentation Folder:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SAssignNew(DocsPathBox, SEditableTextBox)
				.HintText(LOCTEXT("DocsPathHint", "Path to documentation folder..."))
				.Text(FText::FromString(FPaths::ProjectDir() / TEXT("Docs")))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseDocsButton", "Browse..."))
				.OnClicked(this, &SAdastreaDirectorPanel::OnBrowseDocsPathClicked)
			]
		]

		// Database Path Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DbPathLabel", "Database Path:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SAssignNew(DbPathBox, SEditableTextBox)
				.HintText(LOCTEXT("DbPathHint", "Path to ChromaDB database (can select existing database)..."))
				.Text(FText::FromString(FPaths::ProjectDir() / TEXT("chroma_db_adastrea")))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseDbButton", "Browse..."))
				.OnClicked(this, &SAdastreaDirectorPanel::OnBrowseDbPathClicked)
			]
		]

		// Database Status Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DbStatusLabel", "Database Status:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(DatabaseStatusText, STextBlock)
				.Text_Lambda([this]() { return DatabaseStatusMessage; })
				.AutoWrapText(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshDbStatusButton", "Refresh"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnRefreshDbStatusClicked)
				.IsEnabled_Lambda([this]() { return CanRefreshDbStatus(); })
			]
		]

		// Control Buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("StartIngestionButton", "Start Ingestion"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnStartIngestionClicked)
				.IsEnabled_Lambda([this]() { return CanStartIngestion(); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("StopIngestionButton", "Stop"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnStopIngestionClicked)
				.IsEnabled_Lambda([this]() { return CanStopIngestion(); })
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Progress Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SAssignNew(IngestionStatusText, STextBlock)
			.Text_Lambda([this]() { return IngestionStatusMessage; })
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 5.0f)
		[
			SAssignNew(IngestionProgressBar, SProgressBar)
			.Percent_Lambda([this]() { return IngestionProgress; })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SAssignNew(IngestionDetailsText, STextBlock)
			.Text_Lambda([this]() { return IngestionDetailsMessage; })
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Debug Log Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IngestionDebugLogLabel", "Debug Log:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(150.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				
				+ SScrollBox::Slot()
				[
					SAssignNew(IngestionDebugLogDisplay, SMultiLineEditableTextBox)
					.Text_Lambda([this]() { return CachedIngestionDebugLogText; })
					.IsReadOnly(true)
					.AutoWrapText(true)
				]
			]
		];
}

TSharedRef<SWidget> SAdastreaDirectorPanel::CreateDashboardTab()
{
	return SNew(SVerticalBox)
		
		// Status Indicators Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("StatusIndicatorsLabel", "System Status Indicators:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(10.0f)
			[
				SNew(SGridPanel)
				.FillColumn(0, 1.0f)
				.FillColumn(1, 1.0f)
				.FillColumn(2, 1.0f)
				
				// Row 0: Python Process, IPC Connection, & API Key Status
				+ SGridPanel::Slot(0, 0)
				.Padding(5.0f)
				[
					SAssignNew(PythonProcessStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("PythonProcessStatus", "Python Process"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				+ SGridPanel::Slot(1, 0)
				.Padding(5.0f)
				[
					SAssignNew(IPCConnectionStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("IPCConnectionStatus", "IPC Connection"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				+ SGridPanel::Slot(2, 0)
				.Padding(5.0f)
				[
					SAssignNew(APIKeyStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("APIKeyStatus", "API Key Validation"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				// Row 1: Python Bridge & Backend Health
				+ SGridPanel::Slot(0, 1)
				.Padding(5.0f)
				[
					SAssignNew(BridgeReadyStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("BridgeReadyStatus", "Python Bridge Ready"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				+ SGridPanel::Slot(1, 1)
				.Padding(5.0f)
				[
					SAssignNew(BackendHealthStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("BackendHealthStatus", "Backend Health"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				// Row 2: Query Processing & Ingestion
				+ SGridPanel::Slot(0, 2)
				.Padding(5.0f)
				[
					SAssignNew(QueryProcessingStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("QueryProcessingStatus", "Query Processing"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
				
				+ SGridPanel::Slot(1, 2)
				.Padding(5.0f)
				[
					SAssignNew(IngestionStatusLight, SStatusIndicator)
					.StatusText(LOCTEXT("IngestionStatus", "Document Ingestion"))
					.InitialStatus(SStatusIndicator::EStatus::Unknown)
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 10.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Connection Status Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ConnectionStatusLabel", "Detailed Status:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(10.0f)
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					SAssignNew(ConnectionStatusText, STextBlock)
					.Text_Lambda([this]() { return CachedConnectionStatus; })
					.AutoWrapText(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 5.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RefreshStatusButton", "Refresh Status"))
						.ToolTipText(LOCTEXT("RefreshStatusTooltip", "Update connection status and indicators"))
						.OnClicked(this, &SAdastreaDirectorPanel::OnRefreshDashboardClicked)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("ReconnectButton", "Reconnect"))
						.ToolTipText(LOCTEXT("ReconnectTooltip", "Attempt to reconnect to Python backend"))
						.OnClicked(this, &SAdastreaDirectorPanel::OnReconnectClicked)
					]
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 10.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Logs Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LogsLabel", "System Logs:"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearLogsButton", "Clear Logs"))
				.ToolTipText(LOCTEXT("ClearLogsTooltip", "Clear the log display"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnClearLogsClicked)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(300.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				
				+ SScrollBox::Slot()
				[
					SAssignNew(LogDisplay, SMultiLineEditableTextBox)
					.Text_Lambda([this]() { return CachedLogContentText; })
					.IsReadOnly(true)
					.AutoWrapText(true)
				]
			]
		];
}

void SAdastreaDirectorPanel::OnQueryTextChanged(const FText& NewText)
{
	CurrentQuery = NewText;
}

void SAdastreaDirectorPanel::OnQueryTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	// If user pressed Enter, send the query
	if (CommitType == ETextCommit::OnEnter)
	{
		CurrentQuery = NewText;
		OnSendQueryClicked();
	}
}

FReply SAdastreaDirectorPanel::OnSendQueryClicked()
{
	if (!CanSendQuery())
	{
		return FReply::Handled();
	}

	FString QueryString = CurrentQuery.ToString().TrimStartAndEnd();
	
	if (QueryString.IsEmpty())
	{
		UpdateResults(TEXT("Error: Query cannot be empty."));
		return FReply::Handled();
	}

	// Set processing state with RAII guard to ensure it's reset
	struct FProcessingGuard
	{
		bool& Flag;
		FProcessingGuard(bool& InFlag) : Flag(InFlag) { Flag = true; }
		~FProcessingGuard() { Flag = false; }
	} ProcessingGuard(bIsProcessing);
	
	UpdateResults(TEXT("Processing query..."));

	// Send query to Python backend
	SendQueryToPython(QueryString);

	return FReply::Handled();
}

void SAdastreaDirectorPanel::SendQueryToPython(const FString& Query)
{
	// Legacy IPC system has been removed in Phase 3 migration to VibeUE architecture
	// This feature is no longer available - use VibeUE components instead
	UE_LOG(LogAdastreaDirectorEditor, Warning, TEXT("Legacy IPC query feature is no longer available - migrated to VibeUE architecture"));
	UpdateResults(TEXT("Notice: Legacy IPC query system has been removed.\n\n")
		TEXT("The Adastrea Director plugin has migrated to the VibeUE architecture which provides:\n")
		TEXT("• Direct LLM integration via AdastreaLLMClient\n")
		TEXT("• In-process Python execution via AdastreaScriptService\n")
		TEXT("• Runtime asset discovery via AdastreaAssetService\n\n")
		TEXT("See MIGRATION_GUIDE.md for updated usage examples."));
	return;
}


void SAdastreaDirectorPanel::UpdateResults(const FString& Results)
{
	CurrentResults = FText::FromString(Results);
}

bool SAdastreaDirectorPanel::CanSendQuery() const
{
	return !bIsProcessing && !CurrentQuery.IsEmpty();
}

bool SAdastreaDirectorPanel::IsSendButtonEnabled() const
{
	return CanSendQuery();
}

FReply SAdastreaDirectorPanel::OnClearHistoryClicked()
{
	// Show confirmation dialog
	const FText Title = LOCTEXT("ClearHistoryTitle", "Clear Conversation History");
	const FText Message = LOCTEXT("ClearHistoryMessage", "Are you sure you want to clear the conversation history?\n\nThis action cannot be undone.");
	
	EAppReturnType::Type UserResponse = FMessageDialog::Open(EAppMsgType::YesNo, Message, Title);
	
	if (UserResponse != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Legacy IPC system has been removed in Phase 3 migration
	UpdateResults(TEXT("Notice: Legacy conversation history feature is no longer available.\n\n")
		TEXT("The IPC-based query system has been replaced with VibeUE architecture.\n")
		TEXT("See MIGRATION_GUIDE.md for updated approaches."));
	// Removed legacy code - previously cleared conversation history via FPythonBridge IPC
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnSettingsClicked()
{
	SSettingsDialog::OpenDialog();
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnBrowseDocsPathClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		FString FolderPath;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		
		if (DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Select Documentation Folder"),
			FPaths::ProjectDir(),
			FolderPath))
		{
			DocsPathBox->SetText(FText::FromString(FolderPath));
		}
	}

	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnBrowseDbPathClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		FString FolderPath;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		
		if (DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Select Database Path"),
			FPaths::ProjectDir(),
			FolderPath))
		{
			DbPathBox->SetText(FText::FromString(FolderPath));
		}
	}

	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnStartIngestionClicked()
{
	if (!CanStartIngestion())
	{
		return FReply::Handled();
	}

	FString DocsPath = DocsPathBox->GetText().ToString().TrimStartAndEnd();
	FString DbPath = DbPathBox->GetText().ToString().TrimStartAndEnd();

	// Validate paths before clearing the log to preserve context on error
	if (DocsPath.IsEmpty() || DbPath.IsEmpty())
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorPathsEmpty", "Error: Please specify both paths");
		AppendIngestionDebugLog(TEXT("❌ Error: Both documentation path and database path must be specified\n"));
		return FReply::Handled();
	}

	// Validate docs directory exists
	if (!FPaths::DirectoryExists(DocsPath))
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorDocsNotFound", "Error: Documentation folder does not exist");
		AppendIngestionDebugLog(TEXT("❌ Error: Documentation folder does not exist\n"));
		return FReply::Handled();
	}

	// Clear debug log and add initial message after validation passes
	CurrentIngestionDebugLog = TEXT("");
	CachedIngestionDebugLogText = FText::FromString(CurrentIngestionDebugLog);
	AppendIngestionDebugLog(TEXT("🚀 Ingestion started\n"));

	AppendIngestionDebugLog(FString::Printf(TEXT("📁 Documentation path: %s\n"), *DocsPath));
	AppendIngestionDebugLog(FString::Printf(TEXT("💾 Database path: %s\n"), *DbPath));

	AppendIngestionDebugLog(TEXT("✅ Documentation folder exists\n"));

	// Sanitize paths (resolve to absolute paths)
	DocsPath = FPaths::ConvertRelativePathToFull(DocsPath);
	DbPath = FPaths::ConvertRelativePathToFull(DbPath);

	AppendIngestionDebugLog(TEXT("🔄 Converting paths to absolute format\n"));
	AppendIngestionDebugLog(FString::Printf(TEXT("  → Docs: %s\n"), *DocsPath));
	AppendIngestionDebugLog(FString::Printf(TEXT("  → DB: %s\n"), *DbPath));

	// Create progress file directory if it doesn't exist
	FString ProgressDir = FPaths::GetPath(ProgressFilePath);
	if (!FPaths::DirectoryExists(ProgressDir))
	{
		IFileManager::Get().MakeDirectory(*ProgressDir, true);
		AppendIngestionDebugLog(FString::Printf(TEXT("📂 Created progress directory: %s\n"), *ProgressDir));
	}

	AppendIngestionDebugLog(FString::Printf(TEXT("📝 Progress file: %s\n"), *ProgressFilePath));

	bIsIngesting = true;
	IngestionProgress = 0.0f;
	IngestionStatusMessage = LOCTEXT("IngestionStarting", "Starting ingestion...");
	IngestionDetailsMessage = FText::GetEmpty();

	AppendIngestionDebugLog(TEXT("🔌 Connecting to Python backend...\n"));

	// Start ingestion
	StartIngestion(DocsPath, DbPath);

	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnStopIngestionClicked()
{
	// For now, just mark as not ingesting
	// TODO: Send stop signal to Python backend (e.g., via IPC stop_ingest request or process interruption).
	//       This will require adding a cancellation mechanism to the Python ingestion loop.
	//       The ingestion will continue in Python but UI will stop monitoring progress.
	bIsIngesting = false;
	IngestionStatusMessage = LOCTEXT("IngestionStopped", "Ingestion stopped by user");
	
	return FReply::Handled();
}

bool SAdastreaDirectorPanel::CanStartIngestion() const
{
	return !bIsIngesting;
}

bool SAdastreaDirectorPanel::CanStopIngestion() const
{
	return bIsIngesting;
}

FReply SAdastreaDirectorPanel::OnRefreshDbStatusClicked()
{
	// Legacy IPC system has been removed in Phase 3 migration
	DatabaseStatusMessage = FText::FromString(TEXT("Legacy database ingestion feature is no longer available.\n\n")
		TEXT("The VibeUE architecture uses runtime asset discovery instead:\n")
		TEXT("• AdastreaAssetService provides instant asset queries\n")
		TEXT("• No document ingestion needed\n\n")
		TEXT("See MIGRATION_GUIDE.md for details."));
	// Removed legacy code - previously queried ChromaDB status via FPythonBridge IPC
	return FReply::Handled();
}

bool SAdastreaDirectorPanel::CanRefreshDbStatus() const
{
	// Can always refresh status
	return true;
}

void SAdastreaDirectorPanel::StartIngestion(const FString& DocsPath, const FString& DbPath)
{
	// Legacy IPC ingestion system has been removed in Phase 3 migration
	IngestionStatusMessage = LOCTEXT("IngestionNotAvailable", "Legacy ingestion feature is no longer available");
	AppendIngestionDebugLog(TEXT("❌ Legacy document ingestion feature has been removed\n\n"));
	AppendIngestionDebugLog(TEXT("The VibeUE architecture uses runtime asset discovery instead of document ingestion:\n"));
	AppendIngestionDebugLog(TEXT("• AdastreaAssetService provides instant asset queries via Unreal's Asset Registry\n"));
	AppendIngestionDebugLog(TEXT("• No ChromaDB or vector database ingestion needed\n"));
	AppendIngestionDebugLog(TEXT("• See MIGRATION_GUIDE.md for updated asset query examples\n\n"));
	bIsIngesting = false;
	return;
}


void SAdastreaDirectorPanel::UpdateIngestionProgress()
{
	if (!bIsIngesting)
	{
		return;
	}

	// Read progress file
	if (!FPaths::FileExists(ProgressFilePath))
	{
		// Log once per ingestion session to avoid spam
		if (IngestionProgress == 0.0f)
		{
			AppendIngestionDebugLog(TEXT("⏳ Waiting for progress file to be created...\n"));
		}
		return;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ProgressFilePath))
	{
		AppendIngestionDebugLog(TEXT("⚠️ Warning: Could not read progress file\n"));
		return;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		AppendIngestionDebugLog(TEXT("⚠️ Warning: Could not parse progress JSON\n"));
		return;
	}

	// Extract progress data
	double Percent = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("percent"), Percent))
	{
		float OldProgress = IngestionProgress;
		IngestionProgress = static_cast<float>(Percent / 100.0);
		
		// Only log significant progress changes (every 5% - calculated as 100/20 = 5)
		// This prevents flooding the debug log with excessive updates
		if (FMath::FloorToInt(OldProgress * 20.0f) != FMath::FloorToInt(IngestionProgress * 20.0f))
		{
			AppendIngestionDebugLog(FString::Printf(TEXT("📊 Progress: %.0f%%\n"), Percent));
		}
	}

	FString Label;
	if (JsonObject->TryGetStringField(TEXT("label"), Label))
	{
		// Only update if changed to avoid spam
		if (IngestionStatusMessage.ToString() != Label)
		{
			IngestionStatusMessage = FText::FromString(Label);
			AppendIngestionDebugLog(FString::Printf(TEXT("📝 Status: %s\n"), *Label));
		}
	}

	FString Details;
	if (JsonObject->TryGetStringField(TEXT("details"), Details))
	{
		// Only update if changed to avoid spam
		if (IngestionDetailsMessage.ToString() != Details && !Details.IsEmpty())
		{
			IngestionDetailsMessage = FText::FromString(Details);
			AppendIngestionDebugLog(FString::Printf(TEXT("  → %s\n"), *Details));
		}
	}

	FString Status;
	if (JsonObject->TryGetStringField(TEXT("status"), Status))
	{
		if (Status == TEXT("complete"))
		{
			bIsIngesting = false;
			IngestionProgress = 1.0f;
			AppendIngestionDebugLog(TEXT("✅ Ingestion completed successfully!\n"));
			
			// Log final stats if available
			FString FinalDetails = IngestionDetailsMessage.ToString();
			if (!FinalDetails.IsEmpty())
			{
				AppendIngestionDebugLog(FString::Printf(TEXT("  → Final stats: %s\n"), *FinalDetails));
			}
		}
		else if (Status == TEXT("error"))
		{
			bIsIngesting = false;
			AppendIngestionDebugLog(TEXT("❌ Ingestion failed with error\n"));
			if (!Details.IsEmpty())
			{
				AppendIngestionDebugLog(FString::Printf(TEXT("  → Error details: %s\n"), *Details));
			}
		}
	}
}

FReply SAdastreaDirectorPanel::OnRefreshDashboardClicked()
{
	UpdateDashboardLogs();
	UpdateConnectionStatus();
	UpdateStatusLights();
	LastDashboardRefreshTime = RefreshTimerReset; // Reset timer to prevent immediate auto-refresh
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnReconnectClicked()
{
	// Legacy IPC reconnection feature has been removed in Phase 3 migration
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		AppendLogEntry(TEXT("Error: Runtime module not available\n"));
		return FReply::Handled();
	}

	FString LogEntry = TEXT("Legacy IPC reconnection feature is no longer available.\n");
	LogEntry += TEXT("The VibeUE architecture does not use IPC connections.\n");
	LogEntry += TEXT("See MIGRATION_GUIDE.md for updated architecture.\n");
	
	AppendLogEntry(LogEntry);
	UpdateConnectionStatus();
	UpdateStatusLights();
	
	// Removed legacy code - previously reconnected to Python IPC server via FPythonBridge
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnClearLogsClicked()
{
	CurrentLogContent = TEXT("Logs cleared.\n");
	CachedLogContentText = FText::FromString(CurrentLogContent);
	return FReply::Handled();
}

void SAdastreaDirectorPanel::AppendLogEntry(const FString& Entry)
{
	// Prepend new entry to existing logs (newest first)
	CurrentLogContent = Entry + CurrentLogContent;
	
	// Keep only last MaxLogCharacters characters to prevent unbounded growth
	if (CurrentLogContent.Len() > MaxLogCharacters)
	{
		CurrentLogContent = CurrentLogContent.Left(MaxLogCharacters);
	}
	
	// Update cached FText version
	CachedLogContentText = FText::FromString(CurrentLogContent);
}

void SAdastreaDirectorPanel::AppendIngestionDebugLog(const FString& Entry)
{
	// Prepend new entry with timestamp
	FString Timestamp = FDateTime::Now().ToString(TEXT("[%H:%M:%S] "));
	FString NewEntry = Timestamp + Entry;
	
	// Keep only last MaxIngestionDebugLogCharacters characters to prevent unbounded growth
	if (CurrentIngestionDebugLog.Len() + NewEntry.Len() > MaxIngestionDebugLogCharacters)
	{
		// Truncate at a newline boundary to preserve message integrity.
		// The log buffer is stored in reverse chronological order: newest entries are at the beginning,
		// and the oldest entries are at the end of CurrentIngestionDebugLog. To preserve the newest
		// messages, we discard characters from the end (removing the oldest messages).
		int32 TargetLength = MaxIngestionDebugLogCharacters - NewEntry.Len();
		if (TargetLength > 0)
		{
			// Within the portion we keep (up to TargetLength), find the last newline so we keep
			// only complete messages while preserving the newest entries at the beginning.
			int32 LastNewlinePos = CurrentIngestionDebugLog.Left(TargetLength).Find(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
			if (LastNewlinePos != INDEX_NONE && LastNewlinePos > 0)
			{
				CurrentIngestionDebugLog = CurrentIngestionDebugLog.Left(LastNewlinePos + 1);
			}
			else
			{
				// If no newline found, just truncate to target length
				CurrentIngestionDebugLog = CurrentIngestionDebugLog.Left(TargetLength);
			}
		}
		else
		{
			// New entry alone exceeds max, clear the log
			CurrentIngestionDebugLog = TEXT("");
		}
	}
	
	// Prepend new entry to existing logs (newest first, like a console log viewer)
	// This allows users to see the most recent updates without scrolling down
	CurrentIngestionDebugLog = NewEntry + CurrentIngestionDebugLog;
	
	// Update cached FText version
	CachedIngestionDebugLogText = FText::FromString(CurrentIngestionDebugLog);
}

void SAdastreaDirectorPanel::UpdateConnectionStatus()
{
	// Legacy IPC connection status has been removed in Phase 3 migration
	CachedConnectionStatus = FText::FromString(TEXT("ℹ️ Legacy IPC system removed - migrated to VibeUE architecture"));
	// Removed legacy code - previously checked FPythonBridge IPC connection status
	return;
}

void SAdastreaDirectorPanel::UpdateDashboardLogs()
{
	// Legacy IPC dashboard logs have been removed in Phase 3 migration
	FString NewLogEntry = FString::Printf(
		TEXT("=== Dashboard Status Update ===\n")
		TEXT("Timestamp: %s\n")
		TEXT("Architecture: VibeUE (native C++)\n")
		TEXT("Legacy IPC: Removed (Phase 3)\n")
		TEXT("===============================\n\n"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"))
	);
	
	AppendLogEntry(NewLogEntry);
	// Removed legacy code - previously fetched Python process logs via FPythonBridge IPC
}

void SAdastreaDirectorPanel::SetAllStatusLightsToError(const FText& Reason)
{
	// Helper method to set all status lights to error state with the same reason
	if (PythonProcessStatusLight.IsValid())
		PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("PythonProcessErrorFmt", "Python Process: {0}"), Reason));
	if (IPCConnectionStatusLight.IsValid())
		IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("IPCConnectionErrorFmt", "IPC Connection: {0}"), Reason));
	if (BridgeReadyStatusLight.IsValid())
		BridgeReadyStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("BridgeReadyErrorFmt", "Python Bridge: {0}"), Reason));
	if (BackendHealthStatusLight.IsValid())
		BackendHealthStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("BackendHealthErrorFmt", "Backend Health: {0}"), Reason));
	if (APIKeyStatusLight.IsValid())
		APIKeyStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("APIKeyErrorFmt", "API Key: {0}"), Reason));
	if (QueryProcessingStatusLight.IsValid())
		QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("QueryProcessingErrorFmt", "Query Processing: {0}"), Reason));
	if (IngestionStatusLight.IsValid())
		IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("IngestionErrorFmt", "Document Ingestion: {0}"), Reason));
}

void SAdastreaDirectorPanel::UpdateStatusLights()
{
	// Legacy IPC status lights have been removed in Phase 3 migration
	// Set all lights to indicate the migration to VibeUE architecture
	
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		// Runtime module not available - all systems down
		SetAllStatusLightsToError(LOCTEXT("RuntimeModuleNotAvailable", "Runtime module not available"));
		return;
	}

	// Legacy Python Process and IPC are no longer used
	if (PythonProcessStatusLight.IsValid())
		PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("PythonProcessRemoved", "Python Process: N/A (VibeUE)"));
	if (IPCConnectionStatusLight.IsValid())
		IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("IPCRemoved", "IPC Connection: N/A (VibeUE)"));
	if (BridgeReadyStatusLight.IsValid())
		BridgeReadyStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("BridgeRemoved", "Python Bridge: Removed (Phase 3)"));
	if (BackendHealthStatusLight.IsValid())
		BackendHealthStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("BackendVibeUE", "Backend: VibeUE (Native C++)"));
	
	// Check API key configuration (VibeUE Phase 3 - settings only, no backend validation)
	if (APIKeyStatusLight.IsValid())
	{
		FStartupValidationResult SettingsResult = FAdastreaStartupValidator::ValidateSettings();
		if (SettingsResult.bSuccess)
		{
			FAdastreaSettings& Settings = FAdastreaSettings::Get();
			FString Provider = Settings.GetLLMProvider();
			
			APIKeyStatusLight->SetStatus(
				SStatusIndicator::EStatus::Good,
				FText::Format(LOCTEXT("APIKeyConfigured", "API Key: {0} configured"), FText::FromString(Provider))
			);
		}
		else
		{
			// Show validation error
			FString ErrorMsg = SettingsResult.ErrorMessage;
			if (ErrorMsg.Len() > 50)
			{
				// Truncate long error messages
				ErrorMsg = ErrorMsg.Left(47) + TEXT("...");
			}
			APIKeyStatusLight->SetStatus(
				SStatusIndicator::EStatus::Error,
				FText::Format(LOCTEXT("APIKeyInvalid", "API Key: {0}"), FText::FromString(ErrorMsg))
			);
		}
	}

	// Legacy query processing is no longer available
	if (QueryProcessingStatusLight.IsValid())
		QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("QueryProcessingRemoved", "Query Processing: N/A (legacy)"));

	// Legacy ingestion is no longer available
	if (IngestionStatusLight.IsValid())
		IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("IngestionRemoved", "Document Ingestion: N/A (use Asset Registry)"));
		
	// Removed legacy code - previously checked FPythonBridge and IPC connection status
}


void SAdastreaDirectorPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Update ingestion progress if ingesting (throttled to every 100ms)
	if (bIsIngesting)
	{
		const double TimeSinceLastUpdate = InCurrentTime - LastProgressUpdateTime;
		if (TimeSinceLastUpdate >= 0.1) // 100ms throttle
		{
			UpdateIngestionProgress();
			LastProgressUpdateTime = InCurrentTime;
		}
	}

	// Update dashboard if on dashboard tab (throttled intervals)
	if (CurrentTabIndex == 2)
	{
		const double TimeSinceLastRefresh = InCurrentTime - LastDashboardRefreshTime;
		if (TimeSinceLastRefresh >= DashboardRefreshInterval)
		{
			UpdateDashboardLogs();
			UpdateConnectionStatus();
			LastDashboardRefreshTime = InCurrentTime;
		}
		
		// Update connection status more frequently
		const double TimeSinceLastStatusUpdate = InCurrentTime - LastConnectionStatusUpdateTime;
		if (TimeSinceLastStatusUpdate >= ConnectionStatusUpdateInterval)
		{
			UpdateConnectionStatus();
			LastConnectionStatusUpdateTime = InCurrentTime;
		}

		// Update status lights
		const double TimeSinceLastLightsUpdate = InCurrentTime - LastStatusLightsUpdateTime;
		if (TimeSinceLastLightsUpdate >= StatusLightsUpdateInterval)
		{
			UpdateStatusLights();
			LastStatusLightsUpdateTime = InCurrentTime;
		}
	}
}

FReply SAdastreaDirectorPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle Ctrl+, (Ctrl+Comma) for Settings
	if (InKeyEvent.GetKey() == EKeys::Comma && InKeyEvent.IsControlDown())
	{
		SSettingsDialog::OpenDialog();
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SAdastreaDirectorPanel::OnTabButtonClicked(int32 TabIndex)
{
	if (TabIndex >= 0 && TabIndex <= 3)
	{
		CurrentTabIndex = TabIndex;
		
		// If switching to dashboard, refresh it immediately
		if (TabIndex == 2)
		{
			UpdateDashboardLogs();
			UpdateConnectionStatus();
			UpdateStatusLights();
			LastDashboardRefreshTime = RefreshTimerReset; // Reset timer to prevent immediate auto-refresh
		}
		// If switching to tests tab, update test output
		else if (TabIndex == 3)
		{
			UpdateTestOutput();
		}
	}
	return FReply::Handled();
}

ECheckBoxState SAdastreaDirectorPanel::GetTabButtonCheckedState(int32 TabIndex) const
{
	return (CurrentTabIndex == TabIndex) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

TSharedRef<SWidget> SAdastreaDirectorPanel::CreateTestsTab()
{
	return SNew(SVerticalBox)
		
		// Tests Section Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TestsLabel", "🧪 Plugin Self-Test Suite:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Test Buttons Row 1
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SelfCheckButton", "🔍 Self-Check"))
				.ToolTipText(LOCTEXT("SelfCheckTooltip", "Run quick self-check of all plugin components"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnRunSelfCheckClicked)
				.IsEnabled_Lambda([this]() { return CanRunTests(); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("IPCTestsButton", "📡 IPC Tests"))
				.ToolTipText(LOCTEXT("IPCTestsTooltip", "Test IPC connection and communication"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnRunIPCTestsClicked)
				.IsEnabled_Lambda([this]() { return CanRunTests(); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("PluginTestsButton", "🔌 Plugin Tests"))
				.ToolTipText(LOCTEXT("PluginTestsTooltip", "Run plugin-specific unit tests"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnRunPluginTestsClicked)
				.IsEnabled_Lambda([this]() { return CanRunTests(); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("AllTestsButton", "🚀 All Tests"))
				.ToolTipText(LOCTEXT("AllTestsTooltip", "Run all available tests via Python backend"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnRunAllTestsClicked)
				.IsEnabled_Lambda([this]() { return CanRunTests(); })
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearTestOutputButton", "🗑️ Clear"))
				.ToolTipText(LOCTEXT("ClearTestOutputTooltip", "Clear test output display"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnClearTestOutputClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveLogButton", "💾 Save Log"))
				.ToolTipText(LOCTEXT("SaveLogTooltip", "Save test output to a log file"))
				.OnClicked(this, &SAdastreaDirectorPanel::OnSaveTestLogClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		]

		// Test Status
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SAssignNew(TestStatusText, STextBlock)
			.Text_Lambda([this]() { return TestStatusMessage; })
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		// Test Progress Bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f, 10.0f, 5.0f)
		[
			SAssignNew(TestProgressBar, SProgressBar)
			.Percent_Lambda([this]() { return TestProgress; })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TestOutputLabel", "Test Output:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		]

		// Test Output Display
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f, 0.0f, 10.0f, 10.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(300.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				
				+ SScrollBox::Slot()
				[
					SAssignNew(TestOutputDisplay, SMultiLineEditableTextBox)
					.Text_Lambda([this]() { return CachedTestOutputText; })
					.IsReadOnly(true)
					.AutoWrapText(true)
				]
			]
		];
}

FReply SAdastreaDirectorPanel::OnRunSelfCheckClicked()
{
	if (!CanRunTests())
	{
		return FReply::Handled();
	}

	bIsTestRunning = true;
	TestProgress = 0.0f;
	TestStatusMessage = LOCTEXT("SelfCheckRunning", "Running self-check...");
	CurrentTestOutput = TEXT("");
	CachedTestOutputText = FText::FromString(CurrentTestOutput);

	// Perform self-check
	PerformSelfCheck();

	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnRunIPCTestsClicked()
{
	RunTests(TEXT("ipc"));
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnRunPluginTestsClicked()
{
	RunTests(TEXT("plugin"));
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnRunAllTestsClicked()
{
	RunTests(TEXT("all"));
	return FReply::Handled();
}

FReply SAdastreaDirectorPanel::OnClearTestOutputClicked()
{
	CurrentTestOutput = TEXT("🧪 Test output cleared.\n\nClick a test button to run tests.\n");
	CachedTestOutputText = FText::FromString(CurrentTestOutput);
	TestProgress = 0.0f;
	TestStatusMessage = LOCTEXT("TestsIdle", "Ready to run tests");
	return FReply::Handled();
}

void SAdastreaDirectorPanel::RunTests(const FString& TestType)
{
	if (!CanRunTests())
	{
		return;
	}

	bIsTestRunning = true;
	TestProgress = 0.0f;
	CurrentTestOutput = TEXT("");
	CachedTestOutputText = FText::FromString(CurrentTestOutput);

	// Get the Python bridge
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		AppendTestOutput(TEXT("❌ Error: Runtime module not available\n"));
		bIsTestRunning = false;
		TestStatusMessage = LOCTEXT("TestsFailed", "Tests failed - module not available");
		return;
	}

	// Legacy IPC tests are no longer available
	AppendTestOutput(TEXT("❌ Error: Legacy IPC test system has been removed\n"));
	AppendTestOutput(TEXT("The plugin has migrated to VibeUE architecture which does not use IPC.\n"));
	AppendTestOutput(TEXT("See MIGRATION_GUIDE.md for information about the new architecture.\n"));
	bIsTestRunning = false;
	TestStatusMessage = LOCTEXT("TestsNotAvailable", "Legacy tests not available");
	return;
}


void SAdastreaDirectorPanel::PerformSelfCheck()
{
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
	FString PluginVersion = GetPluginVersion();
	AppendTestOutput(FString::Printf(TEXT("═══════════════════════════════════════════════════════════════\n")));
	AppendTestOutput(FString::Printf(TEXT("🔍 ADASTREA DIRECTOR SELF-CHECK\n")));
	AppendTestOutput(FString::Printf(TEXT("Timestamp: %s\n"), *Timestamp));
	AppendTestOutput(FString::Printf(TEXT("Plugin Version: %s (UE5.6+)\n"), *PluginVersion));
	AppendTestOutput(FString::Printf(TEXT("═══════════════════════════════════════════════════════════════\n\n")));

	int32 PassCount = 0;
	int32 FailCount = 0;
	int32 SkippedCount = 0;
	int32 WarningCount = 0;
	int32 TotalChecks = 8; // Increased to include new checks
	int32 CurrentCheck = 0;

	// Check 1: Runtime Module
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	if (RuntimeModule)
	{
		AppendTestOutput(TEXT("✅ [1/8] Runtime Module: Loaded successfully\n"));
		PassCount++;
		
		// Check if fully initialized
		if (RuntimeModule->IsFullyInitialized())
		{
			AppendTestOutput(TEXT("    → Startup validation passed\n"));
		}
		else
		{
			FString InitError = RuntimeModule->GetInitializationError();
			AppendTestOutput(FString::Printf(TEXT("    ⚠️ Initialization incomplete: %s\n"), *InitError));
			WarningCount++;
		}
	}
	else
	{
		AppendTestOutput(TEXT("❌ [1/8] Runtime Module: NOT LOADED\n"));
		FailCount++;
		// Cannot continue without runtime module
		TestStatusMessage = LOCTEXT("SelfCheckFailed", "Self-check failed - runtime module not loaded");
		bIsTestRunning = false;
		return;
	}

	// Check 2: Settings Configuration
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	{
		FAdastreaSettings& Settings = FAdastreaSettings::Get();
		FString ErrorMessage;
		if (Settings.ValidateSettings(ErrorMessage))
		{
			AppendTestOutput(TEXT("✅ [2/8] Settings Configuration: Valid\n"));
			AppendTestOutput(FString::Printf(TEXT("    → LLM Provider: %s\n"), *Settings.GetLLMProvider()));
			AppendTestOutput(FString::Printf(TEXT("    → Embedding Provider: %s\n"), *Settings.GetEmbeddingProvider()));
			AppendTestOutput(FString::Printf(TEXT("    → API Key: %s\n"), Settings.HasAPIKey() ? TEXT("Configured") : TEXT("Not configured")));
			PassCount++;
		}
		else
		{
			AppendTestOutput(TEXT("❌ [2/8] Settings Configuration: INVALID\n"));
			AppendTestOutput(FString::Printf(TEXT("    → Error: %s\n"), *ErrorMessage));
			FailCount++;
		}
	}

	// Check 3: Python Bridge (Legacy - No longer used)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	AppendTestOutput(TEXT("ℹ️  [3/8] Python Bridge: N/A (Removed in Phase 3)\n"));
	SkippedCount++;

	// Check 4: Python Process (Legacy - No longer used)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	AppendTestOutput(TEXT("ℹ️  [4/8] Python Process: N/A (VibeUE uses native C++)\n"));
	SkippedCount++;

	// Check 5: IPC Connection (Legacy - No longer used)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	AppendTestOutput(TEXT("ℹ️  [5/8] IPC Connection: N/A (VibeUE architecture)\n"));
	SkippedCount++;
	// Removed legacy code - previously checked FPythonBridge initialization and status

	// Check 6: Backend Health (Legacy - No longer used)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	AppendTestOutput(TEXT("ℹ️  [6/8] Backend Health: N/A (VibeUE native)\n"));
	SkippedCount++;

	// Check 7: API Key Configuration (VibeUE Phase 3 - settings only, no backend validation)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	{
		FStartupValidationResult SettingsResult = FAdastreaStartupValidator::ValidateSettings();
		if (SettingsResult.bSuccess)
		{
			FAdastreaSettings& Settings = FAdastreaSettings::Get();
			FString Provider = Settings.GetLLMProvider();
			AppendTestOutput(TEXT("✅ [7/8] API Key Configuration: CONFIGURED\n"));
			AppendTestOutput(FString::Printf(TEXT("    → LLM Provider: %s\n"), *Provider));
			PassCount++;
		}
		else
		{
			AppendTestOutput(TEXT("❌ [7/8] API Key Configuration: NOT CONFIGURED\n"));
			AppendTestOutput(FString::Printf(TEXT("    → %s\n"), *SettingsResult.ErrorMessage));
			FailCount++;
		}
	}

	// Check 8: Query Processing (Legacy - No longer used)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	AppendTestOutput(TEXT("ℹ️  [8/8] Query Processing: N/A (use VibeUE components)\n"));
	SkippedCount++;

	// Summary
	AppendTestOutput(TEXT("\n═══════════════════════════════════════════════════════════════\n"));
	AppendTestOutput(TEXT("SELF-CHECK SUMMARY\n"));
	AppendTestOutput(TEXT("───────────────────────────────────────────────────────────────\n"));
	AppendTestOutput(FString::Printf(TEXT("✅ Passed:  %d/%d\n"), PassCount, TotalChecks));
	AppendTestOutput(FString::Printf(TEXT("❌ Failed:  %d/%d\n"), FailCount, TotalChecks));
	if (SkippedCount > 0)
	{
		AppendTestOutput(FString::Printf(TEXT("⚠️  Skipped: %d/%d\n"), SkippedCount, TotalChecks));
	}
	if (WarningCount > 0)
	{
		AppendTestOutput(FString::Printf(TEXT("⚠️  Warnings: %d\n"), WarningCount));
	}
	AppendTestOutput(TEXT("───────────────────────────────────────────────────────────────\n"));
	
	// Determine overall status
	if (FailCount == 0 && SkippedCount == 0 && WarningCount == 0)
	{
		AppendTestOutput(TEXT("\n✅ ALL CHECKS PASSED\n"));
		AppendTestOutput(TEXT("Plugin is fully functional and ready for production use.\n"));
		TestStatusMessage = LOCTEXT("SelfCheckPassed", "✅ All self-checks passed!");
	}
	else if (FailCount == 0 && WarningCount > 0 && SkippedCount == 0)
	{
		AppendTestOutput(TEXT("\n⚠️  CHECKS PASSED WITH WARNINGS\n"));
		AppendTestOutput(TEXT("All checks passed but some warnings were raised.\n"));
		AppendTestOutput(TEXT("Plugin is functional but review warnings above.\n"));
		TestStatusMessage = FText::Format(LOCTEXT("SelfCheckWarnings", "⚠️ {0} passed, {1} warnings"), FText::AsNumber(PassCount), FText::AsNumber(WarningCount));
	}
	else if (FailCount == 0 && SkippedCount > 0 && WarningCount == 0)
	{
		AppendTestOutput(TEXT("\n⚠️  CHECKS INCOMPLETE\n"));
		AppendTestOutput(TEXT("Some checks were skipped due to missing dependencies.\n"));
		AppendTestOutput(TEXT("Plugin may have limited functionality.\n"));
		TestStatusMessage = FText::Format(LOCTEXT("SelfCheckSkipped", "⚠️ {0} passed, {1} skipped"), FText::AsNumber(PassCount), FText::AsNumber(SkippedCount));
	}
	else if (FailCount == 0 && SkippedCount > 0 && WarningCount > 0)
	{
		AppendTestOutput(TEXT("\n⚠️  CHECKS INCOMPLETE WITH WARNINGS\n"));
		AppendTestOutput(TEXT("Some checks were skipped and warnings were raised.\n"));
		AppendTestOutput(TEXT("Plugin may have limited functionality.\n"));
		TestStatusMessage = FText::Format(LOCTEXT("SelfCheckSkippedWarnings", "⚠️ {0} passed, {1} skipped, {2} warnings"), 
			FText::AsNumber(PassCount), FText::AsNumber(SkippedCount), FText::AsNumber(WarningCount));
	}
	else if (FailCount > 0)
	{
		AppendTestOutput(TEXT("\n❌ CHECKS FAILED\n"));
		AppendTestOutput(TEXT("Critical issues detected. Please review failures above.\n"));
		if (FailCount > 3)
		{
			AppendTestOutput(TEXT("\nRecommended Actions:\n"));
			AppendTestOutput(TEXT("1. Check Python installation and dependencies\n"));
			AppendTestOutput(TEXT("2. Verify API key configuration in .env file\n"));
			AppendTestOutput(TEXT("3. Review Output Log for detailed error messages\n"));
			AppendTestOutput(TEXT("4. Restart Unreal Engine if issues persist\n"));
		}
		TestStatusMessage = FText::Format(LOCTEXT("SelfCheckPartialFail", "❌ {0}/{1} checks failed"), FText::AsNumber(FailCount), FText::AsNumber(TotalChecks));
	}
	AppendTestOutput(TEXT("═══════════════════════════════════════════════════════════════\n"));

	TestProgress = 1.0f;
	bIsTestRunning = false;
}

void SAdastreaDirectorPanel::UpdateTestOutput()
{
	// Update cached test output text
	CachedTestOutputText = FText::FromString(CurrentTestOutput);
}

void SAdastreaDirectorPanel::AppendTestOutput(const FString& Entry)
{
	CurrentTestOutput += Entry;
	
	// Keep only last MaxTestOutputCharacters characters, preserving line boundaries
	if (CurrentTestOutput.Len() > MaxTestOutputCharacters)
	{
		// Find a newline near the truncation point to avoid cutting mid-line
		int32 TruncateIndex = CurrentTestOutput.Len() - MaxTestOutputCharacters;
		
		// Search for a newline within the next 100 characters after TruncateIndex
		int32 WindowLength = FMath::Min(100, CurrentTestOutput.Len() - TruncateIndex);
		int32 RelativeNewlineIndex = CurrentTestOutput.Mid(TruncateIndex, WindowLength).Find(TEXT("\n"));
		
		if (RelativeNewlineIndex != INDEX_NONE)
		{
			// Found a newline close to truncation point
			CurrentTestOutput = TEXT("[...truncated...]\n") + CurrentTestOutput.Mid(TruncateIndex + RelativeNewlineIndex + 1);
		}
		else
		{
			// No suitable newline found, just truncate with indicator
			CurrentTestOutput = TEXT("[...truncated...]\n") + CurrentTestOutput.Right(MaxTestOutputCharacters);
		}
	}
	
	// Update cached FText version
	CachedTestOutputText = FText::FromString(CurrentTestOutput);
}

bool SAdastreaDirectorPanel::CanRunTests() const
{
	return !bIsTestRunning;
}

FReply SAdastreaDirectorPanel::OnSaveTestLogClicked()
{
	// Create timestamp for filename
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString DefaultFilename = FString::Printf(TEXT("adastrea_test_log_%s.txt"), *Timestamp);
	
	// Open save file dialog
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		AppendTestOutput(TEXT("\n❌ Failed to open save dialog - desktop platform not available.\n"));
		return FReply::Handled();
	}
	
	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	
	bool bOpened = DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		TEXT("Save Test Log"),
		FPaths::ProjectLogDir(),
		DefaultFilename,
		TEXT("Text Files (*.txt)|*.txt|Log Files (*.log)|*.log|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFiles
	);
	
	if (bOpened && OutFiles.Num() > 0)
	{
		if (SaveTestLogToFile(OutFiles[0]))
		{
			AppendTestOutput(FString::Printf(TEXT("\n✅ Log saved to: %s\n"), *OutFiles[0]));
		}
		else
		{
			AppendTestOutput(TEXT("\n❌ Failed to save log file.\n"));
		}
	}
	
	return FReply::Handled();
}

bool SAdastreaDirectorPanel::SaveTestLogToFile(const FString& FilePath)
{
	// Add header with metadata
	FString LogContent;
	LogContent += TEXT("═══════════════════════════════════════════════════════════════\n");
	LogContent += TEXT("ADASTREA DIRECTOR TEST LOG\n");
	LogContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));
	LogContent += FString::Printf(TEXT("Project: %s\n"), *FPaths::GetProjectFilePath());
	LogContent += TEXT("═══════════════════════════════════════════════════════════════\n\n");
	
	// Add the test output content
	LogContent += CurrentTestOutput;
	
	// Write to file
	return FFileHelper::SaveStringToFile(LogContent, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

#undef LOCTEXT_NAMESPACE
