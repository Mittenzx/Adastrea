// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SMultiLineEditableTextBox;
class SEditableTextBox;
class STextBlock;
class SProgressBar;
class SWidgetSwitcher;
class SStatusIndicator;

/**
 * Main Slate panel widget for Adastrea Director.
 * Provides UI for querying the Python backend, managing documents, and displaying results.
 */
class SAdastreaDirectorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAdastreaDirectorPanel) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Called when the widget is destroyed */
	virtual ~SAdastreaDirectorPanel();

	/** Tick method for updating progress */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Get the plugin version string */
	static FString GetPluginVersion();

	/** Handle keyboard shortcuts */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	// Tab management
	/** Current active tab (0 = Query, 1 = Ingestion, 2 = Dashboard, 3 = Tests) */
	int32 CurrentTabIndex;

	/** Content switcher widget to hold tab contents */
	TSharedPtr<SWidgetSwitcher> TabContentSwitcher;

	// Query tab widgets
	/** Query input text box */
	TSharedPtr<SEditableTextBox> QueryInputBox;

	/** Results display text box */
	TSharedPtr<SMultiLineEditableTextBox> ResultsDisplay;

	/** Current query text */
	FText CurrentQuery;

	/** Current results text */
	FText CurrentResults;

	/** Is a query currently being processed */
	bool bIsProcessing;

	// Ingestion tab widgets
	/** Docs directory path text box */
	TSharedPtr<SEditableTextBox> DocsPathBox;

	/** Database path text box */
	TSharedPtr<SEditableTextBox> DbPathBox;

	/** Ingestion progress bar */
	TSharedPtr<SProgressBar> IngestionProgressBar;

	/** Ingestion status text */
	TSharedPtr<STextBlock> IngestionStatusText;

	/** Ingestion details text */
	TSharedPtr<STextBlock> IngestionDetailsText;

	/** Is ingestion currently running */
	bool bIsIngesting;

	/** Ingestion progress (0-1) */
	float IngestionProgress;

	/** Ingestion status message */
	FText IngestionStatusMessage;

	/** Ingestion details message */
	FText IngestionDetailsMessage;

	/** Path to progress file for ingestion updates */
	FString ProgressFilePath;

	/** Time since last progress update check */
	double LastProgressUpdateTime;

	// Query tab methods
	/** Called when the Send Query button is clicked */
	FReply OnSendQueryClicked();

	/** Called when query text is changed */
	void OnQueryTextChanged(const FText& NewText);

	/** Called when query text is committed (Enter pressed) */
	void OnQueryTextCommitted(const FText& NewText, ETextCommit::Type CommitType);

	/** Helper to send query to Python backend */
	void SendQueryToPython(const FString& Query);

	/** Helper to update results display */
	void UpdateResults(const FString& Results);

	/** Helper to check if query can be sent */
	bool CanSendQuery() const;

	/** Get the enabled state of the send button */
	bool IsSendButtonEnabled() const;

	/** Called when Clear History button is clicked */
	FReply OnClearHistoryClicked();

	/** Called when Settings button is clicked */
	FReply OnSettingsClicked();

	// Tab switching methods
	/** Called when a tab button is clicked */
	FReply OnTabButtonClicked(int32 TabIndex);

	/** Get the checked state for a tab button */
	ECheckBoxState GetTabButtonCheckedState(int32 TabIndex) const;

	// Ingestion tab methods
	/** Called when Browse button is clicked for docs path */
	FReply OnBrowseDocsPathClicked();

	/** Called when Browse button is clicked for database path */
	FReply OnBrowseDbPathClicked();

	/** Called when Start Ingestion button is clicked */
	FReply OnStartIngestionClicked();

	/** Called when Stop Ingestion button is clicked */
	FReply OnStopIngestionClicked();

	/** Helper to check if ingestion can start */
	bool CanStartIngestion() const;

	/** Helper to check if ingestion can be stopped */
	bool CanStopIngestion() const;

	/** Update ingestion progress from progress file */
	void UpdateIngestionProgress();

	/** Helper to send ingestion request to Python backend */
	void StartIngestion(const FString& DocsPath, const FString& DbPath);

	// Dashboard tab widgets
	/** Connection status text */
	TSharedPtr<STextBlock> ConnectionStatusText;

	/** Log display text box */
	TSharedPtr<SMultiLineEditableTextBox> LogDisplay;

	/** Current log content (stored as FString for efficient appending) */
	FString CurrentLogContent;

	/** Cached FText version of log content for display */
	FText CachedLogContentText;

	/** Time since last dashboard refresh */
	double LastDashboardRefreshTime;

	/** Cached connection status text */
	FText CachedConnectionStatus;

	/** Time since last connection status update */
	double LastConnectionStatusUpdateTime;

	/** Maximum log content size in characters */
	static constexpr int32 MaxLogCharacters = 5000;

	/** Dashboard refresh interval in seconds */
	static constexpr double DashboardRefreshInterval = 2.0;

	/** Connection status update interval in seconds */
	static constexpr double ConnectionStatusUpdateInterval = 0.5;

	/** Timer reset value to prevent immediate auto-refresh */
	static constexpr double RefreshTimerReset = -10.0;

	// Status indicator widgets
	/** Python process status indicator */
	TSharedPtr<SStatusIndicator> PythonProcessStatusLight;
	
	/** IPC connection status indicator */
	TSharedPtr<SStatusIndicator> IPCConnectionStatusLight;
	
	/** Python bridge ready status indicator */
	TSharedPtr<SStatusIndicator> BridgeReadyStatusLight;
	
	/** Query processing status indicator */
	TSharedPtr<SStatusIndicator> QueryProcessingStatusLight;
	
	/** Ingestion status indicator */
	TSharedPtr<SStatusIndicator> IngestionStatusLight;
	
	/** Backend health status indicator */
	TSharedPtr<SStatusIndicator> BackendHealthStatusLight;

	/** Status lights update interval in seconds */
	static constexpr double StatusLightsUpdateInterval = 0.5;
	
	/** Time since last status lights update */
	double LastStatusLightsUpdateTime;

	// Dashboard tab methods
	/** Called when Refresh Dashboard button is clicked */
	FReply OnRefreshDashboardClicked();

	/** Called when Reconnect button is clicked */
	FReply OnReconnectClicked();

	/** Called when Clear Logs button is clicked */
	FReply OnClearLogsClicked();

	/** Update dashboard logs */
	void UpdateDashboardLogs();

	/** Helper to append log entry with truncation */
	void AppendLogEntry(const FString& Entry);

	/** Update connection status cache */
	void UpdateConnectionStatus();

	/** Update all status indicator lights */
	void UpdateStatusLights();

	/** Helper to set all status lights to error state */
	void SetAllStatusLightsToError(const FText& Reason);

	// Utility methods
	/** Create the Query tab content */
	TSharedRef<SWidget> CreateQueryTab();

	/** Create the Ingestion tab content */
	TSharedRef<SWidget> CreateIngestionTab();

	/** Create the Dashboard tab content */
	TSharedRef<SWidget> CreateDashboardTab();

	/** Create the Tests tab content */
	TSharedRef<SWidget> CreateTestsTab();

	// Tests tab widgets
	/** Test output display text box */
	TSharedPtr<SMultiLineEditableTextBox> TestOutputDisplay;

	/** Test progress bar */
	TSharedPtr<SProgressBar> TestProgressBar;

	/** Test status text */
	TSharedPtr<STextBlock> TestStatusText;

	/** Current test output content */
	FString CurrentTestOutput;

	/** Cached FText version of test output for display */
	FText CachedTestOutputText;

	/** Is a test currently running */
	bool bIsTestRunning;

	/** Test progress (0-1) */
	float TestProgress;

	/** Test status message */
	FText TestStatusMessage;

	/** Time since last test output update */
	double LastTestOutputUpdateTime;

	/** Test output update interval in seconds */
	static constexpr double TestOutputUpdateInterval = 0.1;

	/** Maximum test output size in characters */
	static constexpr int32 MaxTestOutputCharacters = 10000;

	// Tests tab methods
	/** Called when Run All Tests button is clicked */
	FReply OnRunAllTestsClicked();

	/** Called when Run IPC Tests button is clicked */
	FReply OnRunIPCTestsClicked();

	/** Called when Run Plugin Tests button is clicked */
	FReply OnRunPluginTestsClicked();

	/** Called when Run Self-Check button is clicked */
	FReply OnRunSelfCheckClicked();

	/** Called when Clear Test Output button is clicked */
	FReply OnClearTestOutputClicked();

	/** Called when Save Log button is clicked */
	FReply OnSaveTestLogClicked();

	/** Run a specific test type via Python backend */
	void RunTests(const FString& TestType);

	/** Update test output display */
	void UpdateTestOutput();

	/** Append test output entry */
	void AppendTestOutput(const FString& Entry);

	/** Check if tests can be run */
	bool CanRunTests() const;

	/** Perform self-check of plugin components */
	void PerformSelfCheck();

	/** Save test output to a log file */
	bool SaveTestLogToFile(const FString& FilePath);
};
