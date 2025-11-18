// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "SSettingsDialog.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "HAL/PlatformFileManager.h"

#define LOCTEXT_NAMESPACE "SettingsDialog"

void SSettingsDialog::Construct(const FArguments& InArgs, TSharedPtr<SWindow> InParentWindow)
{
	ParentWindow = InParentWindow;
	
	// Load existing settings
	LoadSettings();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(20.0f)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SettingsTitle", "Settings"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			// Content (scrollable)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)

				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// API Keys Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 15.0f)
					[
						CreateAPIKeysSection()
					]

					// Display Settings Section
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateDisplaySettingsSection()
					]
				]
			]

			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 20.0f, 0.0f, 0.0f)
			[
				CreateButtonSection()
			]
		]
	];
}

void SSettingsDialog::OpenDialog()
{
	TSharedRef<SWindow> SettingsWindow = SNew(SWindow)
		.Title(LOCTEXT("SettingsWindowTitle", "Settings"))
		.ClientSize(FVector2D(550.0f, 600.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::FixedSize);

	TSharedRef<SSettingsDialog> SettingsDialog = SNew(SSettingsDialog, SettingsWindow);
	SettingsWindow->SetContent(SettingsDialog);

	FSlateApplication::Get().AddModalWindow(SettingsWindow, FSlateApplication::Get().GetActiveTopLevelWindow());
}

TSharedRef<SWidget> SSettingsDialog::CreateAPIKeysSection()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(15.0f)
		[
			SNew(SVerticalBox)

			// Section Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("APIKeysSection", "API Keys"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

			// LLM Provider Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 10.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LLMProvider", "LLM Provider:"))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(LLMProvider == TEXT("gemini") ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
						if (NewState == ECheckBoxState::Checked && LLMProvider != TEXT("gemini"))
						{
							OnLLMProviderChanged(TEXT("gemini"));
						}
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GeminiRecommended", "Gemini (Recommended)"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(LLMProvider == TEXT("openai") ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
						if (NewState == ECheckBoxState::Checked && LLMProvider != TEXT("openai"))
						{
							OnLLMProviderChanged(TEXT("openai"));
						}
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OpenAI", "OpenAI"))
					]
				]
			]

			// Gemini API Key
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GeminiAPIKey", "Gemini API Key:"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(GeminiKeyBox, SEditableTextBox)
					.IsPassword(true)
					.Text(FText::FromString(GeminiAPIKey))
					.OnTextChanged_Lambda([this](const FText& NewText) {
						GeminiAPIKey = NewText.ToString().TrimStartAndEnd();
					})
				]
			]

			// OpenAI API Key
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OpenAIAPIKey", "OpenAI API Key:"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(OpenAIKeyBox, SEditableTextBox)
					.IsPassword(true)
					.Text(FText::FromString(OpenAIAPIKey))
					.OnTextChanged_Lambda([this](const FText& NewText) {
						OpenAIAPIKey = NewText.ToString().TrimStartAndEnd();
					})
				]
			]

			// Embedding Provider Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 0.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EmbeddingProvider", "Embedding Provider:"))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(EmbeddingProvider == TEXT("huggingface") ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
						if (NewState == ECheckBoxState::Checked && EmbeddingProvider != TEXT("huggingface"))
						{
							OnEmbeddingProviderChanged(TEXT("huggingface"));
						}
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HuggingFaceFree", "HuggingFace (Free)"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.Style(FAppStyle::Get(), "RadioButton")
					.IsChecked(EmbeddingProvider == TEXT("openai") ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
						if (NewState == ECheckBoxState::Checked && EmbeddingProvider != TEXT("openai"))
						{
							OnEmbeddingProviderChanged(TEXT("openai"));
						}
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OpenAIEmbedding", "OpenAI"))
					]
				]
			]
		];
}

TSharedRef<SWidget> SSettingsDialog::CreateDisplaySettingsSection()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(15.0f)
		[
			SNew(SVerticalBox)

			// Section Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DisplaySection", "Display"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

			// Font Size
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 10.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DefaultFontSize", "Default Font Size:"))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SSpinBox<int32>)
					.MinValue(8)
					.MaxValue(20)
					.Value(DefaultFontSize)
					.OnValueChanged_Lambda([this](int32 NewValue) {
						OnFontSizeChanged(NewValue);
					})
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FontSizeUnit", "pt"))
				]
			]

			// Auto-save Settings
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(SCheckBox)
				.IsChecked(bAutoSaveSettings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					OnAutoSaveChanged(NewState);
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AutoSaveSettings", "Auto-save settings"))
				]
			]

			// Show Timestamps
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SCheckBox)
				.IsChecked(bShowTimestamps ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
					OnShowTimestampsChanged(NewState);
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ShowTimestamps", "Show timestamps in conversation"))
				]
			]
		];
}

TSharedRef<SWidget> SSettingsDialog::CreateButtonSection()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 10.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SaveButton", "Save"))
			.OnClicked(this, &SSettingsDialog::OnSaveClicked)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("CancelButton", "Cancel"))
			.OnClicked(this, &SSettingsDialog::OnCancelClicked)
		];
}

FReply SSettingsDialog::OnSaveClicked()
{
	SaveSettings();
	
	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	
	return FReply::Handled();
}

FReply SSettingsDialog::OnCancelClicked()
{
	if (TSharedPtr<SWindow> Window = ParentWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	
	return FReply::Handled();
}

void SSettingsDialog::OnLLMProviderChanged(FString NewProvider)
{
	LLMProvider = NewProvider;
}

void SSettingsDialog::OnEmbeddingProviderChanged(FString NewProvider)
{
	EmbeddingProvider = NewProvider;
}

void SSettingsDialog::OnFontSizeChanged(int32 NewSize)
{
	DefaultFontSize = NewSize;
}

void SSettingsDialog::OnAutoSaveChanged(ECheckBoxState NewState)
{
	bAutoSaveSettings = (NewState == ECheckBoxState::Checked);
}

void SSettingsDialog::OnShowTimestampsChanged(ECheckBoxState NewState)
{
	bShowTimestamps = (NewState == ECheckBoxState::Checked);
}

void SSettingsDialog::LoadSettings()
{
	// Load from config file (simplified version, no encryption for now)
	LLMProvider = LoadConfigValue(TEXT("LLMProvider"), TEXT("gemini"));
	EmbeddingProvider = LoadConfigValue(TEXT("EmbeddingProvider"), TEXT("huggingface"));
	GeminiAPIKey = LoadConfigValue(TEXT("GeminiAPIKey"), TEXT(""));
	OpenAIAPIKey = LoadConfigValue(TEXT("OpenAIAPIKey"), TEXT(""));
	
	FString FontSizeStr = LoadConfigValue(TEXT("DefaultFontSize"), TEXT("10"));
	DefaultFontSize = FCString::Atoi(*FontSizeStr);
	// Validate font size is within allowed range
	if (DefaultFontSize < 8 || DefaultFontSize > 20)
	{
		DefaultFontSize = 10; // Reset to default if out of bounds or invalid
	}
	
	FString AutoSaveStr = LoadConfigValue(TEXT("AutoSaveSettings"), TEXT("true"));
	bAutoSaveSettings = AutoSaveStr == TEXT("true");
	
	FString ShowTimestampsStr = LoadConfigValue(TEXT("ShowTimestamps"), TEXT("true"));
	bShowTimestamps = ShowTimestampsStr == TEXT("true");
}

void SSettingsDialog::SaveSettings()
{
	SaveConfigValue(TEXT("LLMProvider"), LLMProvider);
	SaveConfigValue(TEXT("EmbeddingProvider"), EmbeddingProvider);
	SaveConfigValue(TEXT("GeminiAPIKey"), GeminiAPIKey);
	SaveConfigValue(TEXT("OpenAIAPIKey"), OpenAIAPIKey);
	SaveConfigValue(TEXT("DefaultFontSize"), FString::FromInt(DefaultFontSize));
	SaveConfigValue(TEXT("AutoSaveSettings"), bAutoSaveSettings ? TEXT("true") : TEXT("false"));
	SaveConfigValue(TEXT("ShowTimestamps"), bShowTimestamps ? TEXT("true") : TEXT("false"));
}

TMap<FString, FString> SSettingsDialog::LoadConfigMap(const FString& ConfigPath)
{
	TMap<FString, FString> ConfigMap;
	
	if (!FPaths::FileExists(ConfigPath))
	{
		return ConfigMap;
	}
	
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return ConfigMap;
	}
	
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);
	
	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT("#")))
		{
			continue;
		}
		
		FString LineKey, LineValue;
		if (TrimmedLine.Split(TEXT("="), &LineKey, &LineValue))
		{
			ConfigMap.Add(LineKey.TrimStartAndEnd(), LineValue.TrimStartAndEnd());
		}
	}
	
	return ConfigMap;
}

FString SSettingsDialog::LoadConfigValue(const FString& Key, const FString& DefaultValue)
{
	FString ConfigPath = FPaths::ProjectSavedDir() / TEXT("AdastreaDirector") / TEXT("config.ini");
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	const FString* Value = ConfigMap.Find(Key);
	return Value ? *Value : DefaultValue;
}

void SSettingsDialog::SaveConfigValue(const FString& Key, const FString& Value)
{
	// Get config file path
	FString ConfigPath = FPaths::ProjectSavedDir() / TEXT("AdastreaDirector") / TEXT("config.ini");
	FString ConfigDir = FPaths::GetPath(ConfigPath);
	
	// Create directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ConfigDir))
	{
		PlatformFile.CreateDirectoryTree(*ConfigDir);
	}
	
	// Load existing content using helper
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	// Update or add the key
	ConfigMap.FindOrAdd(Key) = Value;
	
	// Write back to file
	FString NewContent;
	NewContent += TEXT("# Adastrea Director Configuration\n");
	NewContent += TEXT("# Auto-generated file\n");
	NewContent += TEXT("# Note: Manual edits to this file may be overwritten when saving from the UI\n\n");
	
	// Sort keys for deterministic output
	TArray<FString> SortedKeys;
	ConfigMap.GetKeys(SortedKeys);
	SortedKeys.Sort();
	for (const FString& SortedKey : SortedKeys)
	{
		NewContent += FString::Printf(TEXT("%s=%s\n"), *SortedKey, *ConfigMap[SortedKey]);
	}
	
	if (!FFileHelper::SaveStringToFile(NewContent, *ConfigPath))
	{
		UE_LOG(LogAdastreaDirectorEditor, Error, TEXT("Failed to save settings to: %s"), *ConfigPath);
	}
}

#undef LOCTEXT_NAMESPACE
