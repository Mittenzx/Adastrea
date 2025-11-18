// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SWindow;
class SEditableTextBox;
class SCheckBox;

/**
 * Settings dialog for Adastrea Director plugin.
 * Provides UI for configuring API keys, providers, and display settings.
 */
class SSettingsDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSettingsDialog) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<SWindow> InParentWindow);

	/** Shows the settings dialog as a modal window */
	static void OpenDialog();

private:
	// Widget references
	TWeakPtr<SWindow> ParentWindow;
	TSharedPtr<SEditableTextBox> GeminiKeyBox;
	TSharedPtr<SEditableTextBox> OpenAIKeyBox;
	
	// Settings state
	FString LLMProvider;
	FString EmbeddingProvider;
	FString GeminiAPIKey;
	FString OpenAIAPIKey;
	bool bAutoSaveSettings;
	bool bShowTimestamps;
	int32 DefaultFontSize;

	// UI creation methods
	TSharedRef<SWidget> CreateAPIKeysSection();
	TSharedRef<SWidget> CreateDisplaySettingsSection();
	TSharedRef<SWidget> CreateButtonSection();

	// Event handlers
	FReply OnSaveClicked();
	FReply OnCancelClicked();
	void OnLLMProviderChanged(FString NewProvider);
	void OnEmbeddingProviderChanged(FString NewProvider);
	void OnFontSizeChanged(int32 NewSize);
	void OnAutoSaveChanged(ECheckBoxState NewState);
	void OnShowTimestampsChanged(ECheckBoxState NewState);

	// Helper methods
	void LoadSettings();
	void SaveSettings();
	TMap<FString, FString> LoadConfigMap(const FString& ConfigPath);
	FString LoadConfigValue(const FString& Key, const FString& DefaultValue);
	void SaveConfigValue(const FString& Key, const FString& Value);
};
