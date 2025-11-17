// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SMultiLineEditableTextBox;
class SEditableTextBox;
class STextBlock;
class SProgressBar;

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

private:
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

	// Utility methods
	/** Create the Query tab content */
	TSharedRef<SWidget> CreateQueryTab();

	/** Create the Ingestion tab content */
	TSharedRef<SWidget> CreateIngestionTab();
};
