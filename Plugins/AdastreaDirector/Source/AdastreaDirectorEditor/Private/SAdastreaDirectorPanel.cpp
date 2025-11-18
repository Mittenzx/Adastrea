// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "SAdastreaDirectorPanel.h"
#include "SSettingsDialog.h"
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

		// Main content - simple vertical layout for query (ingestion to be added)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			CreateQueryTab()
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

#undef LOCTEXT_NAMESPACE
