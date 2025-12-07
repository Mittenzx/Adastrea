// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "SAdastreaDirectorPanel.h"
#include "SSettingsDialog.h"
#include "SStatusIndicator.h"
#include "AdastreaDirectorEditorModule.h"
#include "AdastreaDirectorModule.h"
#include "PythonBridge.h"
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

#define LOCTEXT_NAMESPACE "AdastreaDirectorPanel"

SAdastreaDirectorPanel::~SAdastreaDirectorPanel()
{
	// Clean up progress file if it exists
	if (!ProgressFilePath.IsEmpty() && FPaths::FileExists(ProgressFilePath))
	{
		IFileManager::Get().Delete(*ProgressFilePath);
	}
}

void SAdastreaDirectorPanel::Construct(const FArguments& InArgs)
{
	// Initialize state
	bIsProcessing = false;
	bIsIngesting = false;
	IngestionProgress = 0.0f;
	IngestionStatusMessage = LOCTEXT("IngestionIdle", "Ready to ingest documents");
	IngestionDetailsMessage = FText::GetEmpty();
	CurrentResults = LOCTEXT("WelcomeMessage", "Welcome to Adastrea Director!\n\nEnter a query above and click 'Send Query' or press Enter to get started.\n\nExample: \"What is Unreal Engine?\"");
	LastProgressUpdateTime = 0.0;
	CurrentTabIndex = 0; // Start with Query tab
	LastDashboardRefreshTime = 0.0;
	LastConnectionStatusUpdateTime = 0.0;
	CurrentLogContent = TEXT("Dashboard logs will appear here...");
	CachedLogContentText = FText::FromString(CurrentLogContent);
	CachedConnectionStatus = FText::FromString(TEXT("⚠️ Not connected - Python backend not ready"));
	LastStatusLightsUpdateTime = 0.0;
	
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
				SNew(STextBlock)
				.Text(LOCTEXT("PanelTitle", "Adastrea Director - AI Assistant"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
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
				.HintText(LOCTEXT("DbPathHint", "Path to ChromaDB database..."))
				.Text(FText::FromString(FPaths::ProjectDir() / TEXT("chroma_db")))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseDbButton", "Browse..."))
				.OnClicked(this, &SAdastreaDirectorPanel::OnBrowseDbPathClicked)
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
				
				// Row 0: Python Process & IPC Connection
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
	// Get the Python bridge from the runtime module
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Failed to get AdastreaDirector runtime module"));
		UpdateResults(TEXT("Error: AdastreaDirector runtime module not available."));
		return;
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge)
	{
		UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Python bridge not available"));
		UpdateResults(TEXT("Error: Python backend is not initialized.\n\nPlease ensure the Python backend is running."));
		return;
	}

	if (!PythonBridge->IsReady())
	{
		UE_LOG(LogAdastreaDirectorEditor, Warning, TEXT("Python bridge not ready"));
		UpdateResults(TEXT("Error: Python backend is not ready.\n\nPlease check that the Python backend is running and connected."));
		return;
	}

	// Send query request
	FString Response;
	bool bSuccess = PythonBridge->SendRequest(TEXT("query"), Query, Response);

	if (bSuccess)
	{
		// Parse the JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString Status;
			if (!JsonObject->TryGetStringField(TEXT("status"), Status))
			{
				UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Response missing 'status' field"));
				UpdateResults(TEXT("Error: Invalid response format (missing 'status')."));
				return;
			}
			
			if (Status == TEXT("success"))
			{
				FString Result;
				if (!JsonObject->TryGetStringField(TEXT("result"), Result))
				{
					UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Response missing 'result' field"));
					UpdateResults(TEXT("Error: Invalid response format (missing 'result')."));
					return;
				}
				UpdateResults(FString::Printf(TEXT("Query: %s\n\nResponse:\n%s"), *Query, *Result));
			}
			else
			{
				FString Error;
				if (!JsonObject->TryGetStringField(TEXT("error"), Error))
				{
					UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Response missing 'error' field"));
					UpdateResults(TEXT("Error: Invalid response format (missing 'error')."));
					return;
				}
				UpdateResults(FString::Printf(TEXT("Error: %s"), *Error));
			}
		}
		else
		{
			UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Failed to parse response JSON: %s"), *Response);
			UpdateResults(FString::Printf(TEXT("Error: Failed to parse response.\n\nRaw response: %s"), *Response));
		}
	}
	else
	{
		UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Failed to send query to Python backend"));
		UpdateResults(TEXT("Error: Failed to communicate with Python backend.\n\nPlease check the connection and try again."));
	}
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
	
	EAppReturnType::Type UserResponse = FMessageDialog::Open(EAppMsgType::YesNo, Message, &Title);
	
	if (UserResponse != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Get the Python bridge
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		UpdateResults(TEXT("Error: Cannot clear history - runtime module not available."));
		return FReply::Handled();
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge || !PythonBridge->IsReady())
	{
		UpdateResults(TEXT("Error: Python backend is not ready."));
		return FReply::Handled();
	}

	// Send clear history request
	FString Response;
	bool bSuccess = PythonBridge->SendRequest(TEXT("clear_history"), TEXT(""), Response);

	if (bSuccess)
	{
		UpdateResults(TEXT("✓ Conversation history cleared successfully."));
	}
	else
	{
		UpdateResults(TEXT("Error: Failed to clear history."));
	}

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

	// Validate paths
	if (DocsPath.IsEmpty() || DbPath.IsEmpty())
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorPathsEmpty", "Error: Please specify both paths");
		return FReply::Handled();
	}

	// Validate docs directory exists
	if (!FPaths::DirectoryExists(DocsPath))
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorDocsNotFound", "Error: Documentation folder does not exist");
		return FReply::Handled();
	}

	// Sanitize paths (resolve to absolute paths)
	DocsPath = FPaths::ConvertRelativePathToFull(DocsPath);
	DbPath = FPaths::ConvertRelativePathToFull(DbPath);

	// Create progress file directory if it doesn't exist
	FString ProgressDir = FPaths::GetPath(ProgressFilePath);
	if (!FPaths::DirectoryExists(ProgressDir))
	{
		IFileManager::Get().MakeDirectory(*ProgressDir, true);
	}

	bIsIngesting = true;
	IngestionProgress = 0.0f;
	IngestionStatusMessage = LOCTEXT("IngestionStarting", "Starting ingestion...");
	IngestionDetailsMessage = FText::GetEmpty();

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

void SAdastreaDirectorPanel::StartIngestion(const FString& DocsPath, const FString& DbPath)
{
	// Get the Python bridge
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorModuleNotAvailable", "Error: Runtime module not available");
		bIsIngesting = false;
		return;
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge || !PythonBridge->IsReady())
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorBackendNotReady", "Error: Python backend not ready");
		bIsIngesting = false;
		return;
	}

	// Build JSON request
	TSharedPtr<FJsonObject> RequestData = MakeShared<FJsonObject>();
	RequestData->SetStringField(TEXT("docs_dir"), DocsPath);
	RequestData->SetStringField(TEXT("persist_dir"), DbPath);
	RequestData->SetStringField(TEXT("progress_file"), ProgressFilePath);
	RequestData->SetBoolField(TEXT("force_reingest"), false);
	RequestData->SetStringField(TEXT("collection_name"), TEXT("adastrea_docs"));

	FString RequestDataString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestDataString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	// Send ingestion request
	FString Response;
	bool bSuccess = PythonBridge->SendRequest(TEXT("ingest"), RequestDataString, Response);

	if (bSuccess)
	{
		IngestionStatusMessage = LOCTEXT("IngestionInProgress", "Ingestion in progress...");
	}
	else
	{
		IngestionStatusMessage = LOCTEXT("IngestionErrorFailedToStart", "Error: Failed to start ingestion");
		bIsIngesting = false;
	}
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
		return;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ProgressFilePath))
	{
		return;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	// Extract progress data
	double Percent = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("percent"), Percent))
	{
		IngestionProgress = static_cast<float>(Percent / 100.0);
	}

	FString Label;
	if (JsonObject->TryGetStringField(TEXT("label"), Label))
	{
		IngestionStatusMessage = FText::FromString(Label);
	}

	FString Details;
	if (JsonObject->TryGetStringField(TEXT("details"), Details))
	{
		IngestionDetailsMessage = FText::FromString(Details);
	}

	FString Status;
	if (JsonObject->TryGetStringField(TEXT("status"), Status))
	{
		if (Status == TEXT("complete"))
		{
			bIsIngesting = false;
			IngestionProgress = 1.0f;
		}
		else if (Status == TEXT("error"))
		{
			bIsIngesting = false;
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
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		AppendLogEntry(TEXT("Error: Runtime module not available\n"));
		return FReply::Handled();
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge)
	{
		AppendLogEntry(TEXT("Error: Python bridge not initialized\n"));
		return FReply::Handled();
	}

	FString LogEntry = TEXT("Attempting to reconnect to Python backend...\n");
	
	bool bSuccess = PythonBridge->Reconnect();
	
	if (bSuccess)
	{
		LogEntry += TEXT("✅ Reconnection successful!\n");
	}
	else
	{
		LogEntry += TEXT("❌ Reconnection failed. Please check Python backend.\n");
	}
	
	AppendLogEntry(LogEntry);
	UpdateConnectionStatus();
	UpdateStatusLights();
	
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

void SAdastreaDirectorPanel::UpdateConnectionStatus()
{
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	if (!RuntimeModule)
	{
		CachedConnectionStatus = FText::FromString(TEXT("❌ Runtime module not available"));
		return;
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	if (!PythonBridge)
	{
		CachedConnectionStatus = FText::FromString(TEXT("❌ Python bridge not initialized"));
		return;
	}

	if (PythonBridge->IsReady())
	{
		FString Status = PythonBridge->GetStatus();
		CachedConnectionStatus = FText::FromString(FString::Printf(TEXT("✅ Connected - %s"), *Status));
	}
	else
	{
		CachedConnectionStatus = FText::FromString(TEXT("⚠️ Not connected - Python backend not ready"));
	}
}

void SAdastreaDirectorPanel::UpdateDashboardLogs()
{
	// Get the Python bridge
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		AppendLogEntry(TEXT("Error: Runtime module not available - cannot fetch logs\n"));
		return;
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge)
	{
		AppendLogEntry(TEXT("Error: Python bridge not initialized - cannot fetch logs\n"));
		return;
	}

	// Build diagnostic log entry using Printf for efficiency
	FString NewLogEntry = FString::Printf(
		TEXT("=== Dashboard Status Update ===\n")
		TEXT("Timestamp: %s\n")
		TEXT("Python Bridge Ready: %s\n")
		TEXT("Status: %s\n")
		TEXT("===============================\n\n"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")),
		PythonBridge->IsReady() ? TEXT("Yes") : TEXT("No"),
		*PythonBridge->GetStatus()
	);
	
	AppendLogEntry(NewLogEntry);
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
	if (QueryProcessingStatusLight.IsValid())
		QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("QueryProcessingErrorFmt", "Query Processing: {0}"), Reason));
	if (IngestionStatusLight.IsValid())
		IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, FText::Format(LOCTEXT("IngestionErrorFmt", "Document Ingestion: {0}"), Reason));
}

void SAdastreaDirectorPanel::UpdateStatusLights()
{
	// NOTE: This implementation uses string parsing of GetStatus() output.
	// While not ideal, it works with the current PythonBridge API without requiring
	// changes to the bridge interface. Future enhancement: add structured status
	// methods (e.g., IsProcessRunning(), IsIPCConnected()) to PythonBridge.
	
	// Get the Python bridge
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	
	if (!RuntimeModule)
	{
		// Runtime module not available - all systems down
		SetAllStatusLightsToError(LOCTEXT("RuntimeModuleNotAvailable", "Runtime module not available"));
		return;
	}

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge)
	{
		// Python bridge not initialized - set most to error, query/ingestion to unknown
		if (PythonProcessStatusLight.IsValid())
			PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("PythonProcessNotInit", "Python Process: Bridge not initialized"));
		if (IPCConnectionStatusLight.IsValid())
			IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("IPCConnectionNotInit", "IPC Connection: Bridge not initialized"));
		if (BridgeReadyStatusLight.IsValid())
			BridgeReadyStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("BridgeReadyNotInit", "Python Bridge: Not initialized"));
		if (BackendHealthStatusLight.IsValid())
			BackendHealthStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("BackendHealthNotInit", "Backend Health: Bridge not initialized"));
		if (QueryProcessingStatusLight.IsValid())
			QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("QueryProcessingIdle", "Query Processing: Idle"));
		if (IngestionStatusLight.IsValid())
			IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Unknown, LOCTEXT("IngestionIdle", "Document Ingestion: Not running"));
		return;
	}

	// Check Python process status (we need to access internal state through GetStatus)
	FString StatusString = PythonBridge->GetStatus();
	
	// Use precise, mutually exclusive checks to avoid ambiguity
	// Check for "not running" first as it's the most specific error state
	bool bProcessNotRunning = StatusString.Contains(TEXT("not running"));
	bool bIPCNotConnected = StatusString.Contains(TEXT("IPC not connected"));
	bool bIsReady = StatusString.Contains(TEXT("Ready"));
	
	// Python Process status
	if (bProcessNotRunning)
	{
		if (PythonProcessStatusLight.IsValid())
			PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("PythonProcessStopped", "Python Process: Not running"));
	}
	else if (bIsReady)
	{
		// Extract just the relevant part - "Running" instead of full status string
		if (PythonProcessStatusLight.IsValid())
			PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("PythonProcessRunning", "Python Process: Running"));
	}
	else
	{
		if (PythonProcessStatusLight.IsValid())
			PythonProcessStatusLight->SetStatus(SStatusIndicator::EStatus::Warning, LOCTEXT("PythonProcessUnknown", "Python Process: Unknown state"));
	}

	// Check IPC connection status
	if (bIPCNotConnected)
	{
		if (IPCConnectionStatusLight.IsValid())
			IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("IPCDisconnected", "IPC Connection: Disconnected"));
	}
	else if (bIsReady)
	{
		if (IPCConnectionStatusLight.IsValid())
			IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("IPCConnected", "IPC Connection: Connected"));
	}
	else
	{
		if (IPCConnectionStatusLight.IsValid())
			IPCConnectionStatusLight->SetStatus(SStatusIndicator::EStatus::Warning, LOCTEXT("IPCUnknown", "IPC Connection: Unknown state"));
	}

	// Check Python bridge ready state
	// Note: bIsReady is derived from StatusString.Contains("Ready"), which is equivalent to
	// PythonBridge->IsReady() since GetStatus() returns "Ready..." only when both
	// ProcessManager->IsProcessRunning() and IPCClient->IsConnected() are true.
	if (bIsReady)
	{
		if (BridgeReadyStatusLight.IsValid())
			BridgeReadyStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("BridgeReady", "Python Bridge: Ready"));
		if (BackendHealthStatusLight.IsValid())
			BackendHealthStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("BackendHealthGood", "Backend Health: Operational"));
	}
	else
	{
		if (BridgeReadyStatusLight.IsValid())
			BridgeReadyStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("BridgeNotReady", "Python Bridge: Not ready"));
		if (BackendHealthStatusLight.IsValid())
			BackendHealthStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("BackendHealthBad", "Backend Health: Not operational"));
	}

	// Check query processing state
	if (bIsProcessing)
	{
		if (QueryProcessingStatusLight.IsValid())
			QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("QueryProcessingActive", "Query Processing: Active"));
	}
	else if (bIsReady)
	{
		if (QueryProcessingStatusLight.IsValid())
			QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("QueryProcessingReady", "Query Processing: Ready"));
	}
	else
	{
		if (QueryProcessingStatusLight.IsValid())
			QueryProcessingStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("QueryProcessingUnavailable", "Query Processing: Unavailable"));
	}

	// Check ingestion state
	if (bIsIngesting)
	{
		if (IngestionStatusLight.IsValid())
		{
			float ProgressPercent = IngestionProgress * 100.0f;
			IngestionStatusLight->SetStatus(
				SStatusIndicator::EStatus::Warning, 
				FText::Format(LOCTEXT("IngestionActive", "Document Ingestion: Active ({0}%)"), FText::AsNumber(static_cast<int32>(ProgressPercent)))
			);
		}
	}
	else if (bIsReady)
	{
		if (IngestionStatusLight.IsValid())
			IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Good, LOCTEXT("IngestionReady", "Document Ingestion: Ready"));
	}
	else
	{
		if (IngestionStatusLight.IsValid())
			IngestionStatusLight->SetStatus(SStatusIndicator::EStatus::Error, LOCTEXT("IngestionUnavailable", "Document Ingestion: Unavailable"));
	}
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

	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	
	if (!PythonBridge || !PythonBridge->IsReady())
	{
		AppendTestOutput(TEXT("❌ Error: Python backend not ready\n"));
		AppendTestOutput(TEXT("Please ensure the Python backend is running.\n"));
		bIsTestRunning = false;
		TestStatusMessage = LOCTEXT("TestsFailed", "Tests failed - backend not ready");
		return;
	}

	// Update status
	TestStatusMessage = FText::Format(LOCTEXT("TestsRunning", "Running {0} tests..."), FText::FromString(TestType));
	AppendTestOutput(FString::Printf(TEXT("🧪 Running %s tests...\n"), *TestType));
	AppendTestOutput(FString::Printf(TEXT("Timestamp: %s\n\n"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"))));

	// Send test request to Python backend
	FString Response;
	bool bSuccess = PythonBridge->SendRequest(TEXT("run_tests"), TestType, Response);

	if (bSuccess)
	{
		// Parse the JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString Status;
			if (JsonObject->TryGetStringField(TEXT("status"), Status) && Status == TEXT("success"))
			{
				FString Result;
				if (JsonObject->TryGetStringField(TEXT("result"), Result))
				{
					AppendTestOutput(Result);
					AppendTestOutput(TEXT("\n"));
				}
				
				int32 Passed = 0;
				int32 Failed = 0;
				JsonObject->TryGetNumberField(TEXT("passed"), Passed);
				JsonObject->TryGetNumberField(TEXT("failed"), Failed);
				
				if (Passed == 0 && Failed == 0)
				{
					TestStatusMessage = LOCTEXT("TestsNoResults", "⚠️ No tests found or failed to parse results");
					TestProgress = 1.0f;
				}
				else if (Failed == 0)
				{
					TestStatusMessage = FText::Format(LOCTEXT("TestsPassedStatus", "✅ All tests passed ({0} tests)"), FText::AsNumber(Passed));
					TestProgress = 1.0f;
				}
				else
				{
					TestStatusMessage = FText::Format(LOCTEXT("TestsFailedStatus", "❌ {0} passed, {1} failed"), FText::AsNumber(Passed), FText::AsNumber(Failed));
					TestProgress = 1.0f;
				}
			}
			else
			{
				FString Error;
				if (!JsonObject->TryGetStringField(TEXT("error"), Error) || Error.IsEmpty())
				{
					Error = TEXT("Unknown error occurred");
				}
				AppendTestOutput(FString::Printf(TEXT("❌ Error: %s\n"), *Error));
				TestStatusMessage = LOCTEXT("TestsError", "Tests encountered an error");
			}
		}
		else
		{
			AppendTestOutput(FString::Printf(TEXT("Raw response: %s\n"), *Response));
		}
	}
	else
	{
		AppendTestOutput(TEXT("❌ Failed to communicate with Python backend\n"));
		TestStatusMessage = LOCTEXT("TestsCommError", "Communication error");
	}

	bIsTestRunning = false;
}

void SAdastreaDirectorPanel::PerformSelfCheck()
{
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
	AppendTestOutput(FString::Printf(TEXT("═══════════════════════════════════════════════════════════════\n")));
	AppendTestOutput(FString::Printf(TEXT("🔍 ADASTREA DIRECTOR SELF-CHECK\n")));
	AppendTestOutput(FString::Printf(TEXT("Timestamp: %s\n"), *Timestamp));
	AppendTestOutput(FString::Printf(TEXT("═══════════════════════════════════════════════════════════════\n\n")));

	int32 PassCount = 0;
	int32 FailCount = 0;
	int32 SkippedCount = 0;
	int32 TotalChecks = 6;
	int32 CurrentCheck = 0;

	// Check 1: Runtime Module
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	FAdastreaDirectorModule* RuntimeModule = FModuleManager::GetModulePtr<FAdastreaDirectorModule>("AdastreaDirector");
	if (RuntimeModule)
	{
		AppendTestOutput(TEXT("✅ [1/6] Runtime Module: Loaded successfully\n"));
		PassCount++;
	}
	else
	{
		AppendTestOutput(TEXT("❌ [1/6] Runtime Module: NOT LOADED\n"));
		FailCount++;
		// Cannot continue without runtime module
		TestStatusMessage = LOCTEXT("SelfCheckFailed", "Self-check failed - runtime module not loaded");
		bIsTestRunning = false;
		return;
	}

	// Check 2: Python Bridge
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	FPythonBridge* PythonBridge = RuntimeModule->GetPythonBridge();
	if (PythonBridge)
	{
		AppendTestOutput(TEXT("✅ [2/6] Python Bridge: Initialized\n"));
		PassCount++;
	}
	else
	{
		AppendTestOutput(TEXT("❌ [2/6] Python Bridge: NOT INITIALIZED\n"));
		FailCount++;
	}

	// Check 3: Python Process
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	if (PythonBridge)
	{
		FString Status = PythonBridge->GetStatus();
		if (!Status.Contains(TEXT("not running")))
		{
			AppendTestOutput(TEXT("✅ [3/6] Python Process: Running\n"));
			PassCount++;
		}
		else
		{
			AppendTestOutput(TEXT("❌ [3/6] Python Process: NOT RUNNING\n"));
			FailCount++;
		}
	}
	else
	{
		AppendTestOutput(TEXT("⚠️ [3/6] Python Process: Cannot check (bridge not initialized)\n"));
		SkippedCount++;
	}

	// Check 4: IPC Connection
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	if (PythonBridge && PythonBridge->IsReady())
	{
		AppendTestOutput(TEXT("✅ [4/6] IPC Connection: Connected\n"));
		PassCount++;
	}
	else
	{
		AppendTestOutput(TEXT("❌ [4/6] IPC Connection: NOT CONNECTED\n"));
		FailCount++;
	}

	// Check 5: Backend Health (Ping test)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	if (PythonBridge && PythonBridge->IsReady())
	{
		FString Response;
		bool bPingSuccess = PythonBridge->SendRequest(TEXT("ping"), TEXT(""), Response);
		if (bPingSuccess && Response.Contains(TEXT("pong")))
		{
			AppendTestOutput(TEXT("✅ [5/6] Backend Health: Responding (ping → pong)\n"));
			PassCount++;
		}
		else
		{
			AppendTestOutput(TEXT("❌ [5/6] Backend Health: NOT RESPONDING\n"));
			FailCount++;
		}
	}
	else
	{
		AppendTestOutput(TEXT("⚠️ [5/6] Backend Health: Cannot check (not connected)\n"));
		SkippedCount++;
	}

	// Check 6: Query Processing (verify query handler responds correctly)
	CurrentCheck++;
	TestProgress = static_cast<float>(CurrentCheck) / TotalChecks;
	if (PythonBridge && PythonBridge->IsReady())
	{
		// Test actual query processing by sending a query request
		FString Response;
		bool bQuerySuccess = PythonBridge->SendRequest(TEXT("query"), TEXT("test"), Response);
		if (bQuerySuccess && (Response.Contains(TEXT("success")) || Response.Contains(TEXT("result"))))
		{
			AppendTestOutput(TEXT("✅ [6/6] Query Processing: Working\n"));
			PassCount++;
		}
		else
		{
			// Truncate response for display if too long
			FString DisplayResponse = Response.Len() > 100 ? Response.Left(100) + TEXT("...") : Response;
			AppendTestOutput(FString::Printf(TEXT("❌ [6/6] Query Processing: FAILED - %s\n"), *DisplayResponse));
			FailCount++;
		}
	}
	else
	{
		AppendTestOutput(TEXT("⚠️ [6/6] Query Processing: Cannot check (not connected)\n"));
		SkippedCount++;
	}

	// Summary
	AppendTestOutput(TEXT("\n═══════════════════════════════════════════════════════════════\n"));
	AppendTestOutput(TEXT("SELF-CHECK SUMMARY\n"));
	AppendTestOutput(FString::Printf(TEXT("Passed: %d/%d\n"), PassCount, TotalChecks));
	AppendTestOutput(FString::Printf(TEXT("Failed: %d/%d\n"), FailCount, TotalChecks));
	if (SkippedCount > 0)
	{
		AppendTestOutput(FString::Printf(TEXT("Skipped: %d/%d\n"), SkippedCount, TotalChecks));
	}
	
	if (FailCount == 0 && SkippedCount == 0)
	{
		AppendTestOutput(TEXT("\n✅ All self-checks passed! Plugin is functioning correctly.\n"));
		TestStatusMessage = LOCTEXT("SelfCheckPassed", "✅ All self-checks passed!");
	}
	else if (FailCount == 0 && SkippedCount > 0)
	{
		AppendTestOutput(TEXT("\n⚠️ Some checks were skipped due to dependencies.\n"));
		TestStatusMessage = FText::Format(LOCTEXT("SelfCheckSkipped", "⚠️ {0} passed, {1} skipped"), FText::AsNumber(PassCount), FText::AsNumber(SkippedCount));
	}
	else
	{
		AppendTestOutput(TEXT("\n❌ Some self-checks failed. Please check the issues above.\n"));
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
