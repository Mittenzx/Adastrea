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
#include "Misc/MessageDialog.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"

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
	// Get the .env file path
	FString EnvFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".env"));
	FString EnvExamplePath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".env.example"));
	bool bEnvFileExists = FPaths::FileExists(EnvFilePath);
	
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
				.Text(LOCTEXT("APIKeysSection", "API Configuration (.env)"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]

			// Instructions
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 15.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(10.0f)
				[
					SNew(SVerticalBox)
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 5.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("EnvInstructions", "📝 API keys are configured via .env file"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(
							TEXT("1. Copy .env.example to .env in your project root\n")
							TEXT("2. Edit .env and add your API key:\n")
							TEXT("   GEMINI_KEY=your-api-key-here\n")
							TEXT("   (or GOOGLE_API_KEY for compatibility)\n")
							TEXT("   OPENAI_API_KEY=your-key (if using OpenAI)\n")
							TEXT("3. Restart Unreal Engine\n\n")
							TEXT(".env location: %s\n")
							TEXT("Status: %s"),
							*EnvFilePath,
							bEnvFileExists ? TEXT("✓ File exists") : TEXT("⚠ File not found")
						)))
						.AutoWrapText(true)
					]
				]
			]

			// LLM Provider Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
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

			// Embedding Provider Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 15.0f)
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

			// Helper Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenEnvFileButton", "Open .env File"))
					.ToolTipText(LOCTEXT("OpenEnvFileTooltip", "Open the .env file in your default text editor"))
					.OnClicked_Lambda([EnvFilePath]() -> FReply {
						if (FPaths::FileExists(EnvFilePath))
						{
							FPlatformProcess::LaunchFileInDefaultExternalApplication(*EnvFilePath);
						}
						else
						{
							FMessageDialog::Open(EAppMsgType::Ok, 
								FText::FromString(TEXT(".env file not found. Please copy .env.example to .env first.")));
						}
						return FReply::Handled();
					})
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenProjectFolderButton", "Open Project Folder"))
					.ToolTipText(LOCTEXT("OpenProjectFolderTooltip", "Open the project folder in file explorer"))
					.OnClicked_Lambda([]() -> FReply {
						FPlatformProcess::ExploreFolder(*FPaths::ProjectDir());
						return FReply::Handled();
					})
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CreateEnvButton", "Create .env from Template"))
					.ToolTipText(LOCTEXT("CreateEnvTooltip", "Copy .env.example to .env"))
					.IsEnabled_Lambda([EnvFilePath]() { return !FPaths::FileExists(EnvFilePath); })
					.OnClicked_Lambda([EnvFilePath, EnvExamplePath]() -> FReply {
						if (FPaths::FileExists(EnvExamplePath))
						{
							if (IFileManager::Get().Copy(*EnvFilePath, *EnvExamplePath) == COPY_OK)
							{
								FMessageDialog::Open(EAppMsgType::Ok,
									FText::FromString(TEXT(".env file created successfully! Please edit it to add your API key, then restart Unreal Engine.")));
							}
							else
							{
								FMessageDialog::Open(EAppMsgType::Ok,
									FText::FromString(TEXT("Failed to create .env file. Please create it manually.")));
							}
						}
						else
						{
							FMessageDialog::Open(EAppMsgType::Ok,
								FText::FromString(TEXT(".env.example not found in project root.")));
						}
						return FReply::Handled();
					})
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
	// Load from config file
	// Note: API keys are now configured via .env file, not stored in config.ini
	LLMProvider = LoadConfigValue(TEXT("LLMProvider"), TEXT("gemini"));
	EmbeddingProvider = LoadConfigValue(TEXT("EmbeddingProvider"), TEXT("huggingface"));
	
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
	
	// Initialize API key strings as empty (they're read from .env by Python backend)
	GeminiAPIKey = TEXT("");
	OpenAIAPIKey = TEXT("");
}

void SSettingsDialog::SaveSettings()
{
	// Save provider preferences and display settings
	// Note: API keys are configured via .env file and not saved here
	SaveConfigValue(TEXT("LLMProvider"), LLMProvider);
	SaveConfigValue(TEXT("EmbeddingProvider"), EmbeddingProvider);
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
