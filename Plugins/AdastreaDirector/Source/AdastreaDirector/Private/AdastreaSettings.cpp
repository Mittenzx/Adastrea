// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaSettings.h"
#include "AdastreaDirectorModule.h"

FAdastreaSettings& FAdastreaSettings::Get()
{
	static FAdastreaSettings Instance;
	return Instance;
}

FAdastreaSettings::FAdastreaSettings()
	: DefaultFontSize(10)
	, bAutoSaveSettings(true)
	, bShowTimestamps(true)
{
	LoadSettings();
}

void FAdastreaSettings::LoadSettings()
{
	// Load config map once instead of reading file multiple times
	FString ConfigPath = GetConfigFilePath();
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	// Helper lambda to get value with default
	auto GetValue = [&ConfigMap](const FString& Key, const FString& DefaultValue) -> FString
	{
		const FString* Value = ConfigMap.Find(Key);
		return Value ? *Value : DefaultValue;
	};
	
	LLMProvider = GetValue(TEXT("LLMProvider"), TEXT("gemini"));
	EmbeddingProvider = GetValue(TEXT("EmbeddingProvider"), TEXT("huggingface"));
	
	// API keys are no longer stored in config.ini - they're configured via .env file
	// The Python backend reads them from environment variables
	// For validation purposes, we'll mark them as empty here
	GeminiAPIKey = TEXT("");
	OpenAIAPIKey = TEXT("");
	
	FString FontSizeStr = GetValue(TEXT("DefaultFontSize"), TEXT("10"));
	DefaultFontSize = FCString::Atoi(*FontSizeStr);
	if (DefaultFontSize < 8 || DefaultFontSize > 20)
	{
		DefaultFontSize = 10;
	}
	
	FString AutoSaveStr = GetValue(TEXT("AutoSaveSettings"), TEXT("true"));
	bAutoSaveSettings = AutoSaveStr == TEXT("true");
	
	FString ShowTimestampsStr = GetValue(TEXT("ShowTimestamps"), TEXT("true"));
	bShowTimestamps = ShowTimestampsStr == TEXT("true");
}

void FAdastreaSettings::SaveSettings()
{
	FString ConfigPath = GetConfigFilePath();
	FString ConfigDir = FPaths::GetPath(ConfigPath);
	
	// Create directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ConfigDir))
	{
		PlatformFile.CreateDirectoryTree(*ConfigDir);
	}
	
	// Load existing content
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	// Update all values in the map (write once, not multiple times)
	ConfigMap.FindOrAdd(TEXT("LLMProvider")) = LLMProvider;
	ConfigMap.FindOrAdd(TEXT("EmbeddingProvider")) = EmbeddingProvider;
	// API keys are not saved - they're managed via .env file
	ConfigMap.FindOrAdd(TEXT("DefaultFontSize")) = FString::FromInt(DefaultFontSize);
	ConfigMap.FindOrAdd(TEXT("AutoSaveSettings")) = bAutoSaveSettings ? TEXT("true") : TEXT("false");
	ConfigMap.FindOrAdd(TEXT("ShowTimestamps")) = bShowTimestamps ? TEXT("true") : TEXT("false");
	
	// Write back to file once
	FString NewContent;
	NewContent += TEXT("# Adastrea Director Configuration\n");
	NewContent += TEXT("# Auto-generated file\n\n");
	
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
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to save settings to: %s"), *ConfigPath);
	}
}

bool FAdastreaSettings::ValidateSettings(FString& OutErrorMessage) const
{
	// Note: API keys are now configured via .env file, not in plugin settings
	// We skip local validation and rely on the Python backend validation
	// which will check if the .env file has the required keys
	
	// Just validate that a provider is selected
	if (LLMProvider.IsEmpty())
	{
		OutErrorMessage = TEXT("No LLM provider selected. Please select a provider in Settings.");
		return false;
	}
	
	// Provider must be valid
	if (LLMProvider != TEXT("gemini") && LLMProvider != TEXT("openai"))
	{
		OutErrorMessage = FString::Printf(TEXT("Invalid LLM provider '%s'. Must be 'gemini' or 'openai'."), *LLMProvider);
		return false;
	}

	return true;
}

bool FAdastreaSettings::HasAPIKey() const
{
	// API keys are configured via .env file
	// We can't check them from the plugin side
	// The Python backend will validate them during startup
	// Return true here to allow the validation to proceed to backend checks
	return true;
}

FString FAdastreaSettings::GetConfigFilePath()
{
	return FPaths::ProjectSavedDir() / TEXT("AdastreaDirector") / TEXT("config.ini");
}

TMap<FString, FString> FAdastreaSettings::LoadConfigMap(const FString& ConfigPath)
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
		
		// Find first '=' to handle values that contain '=' characters
		int32 EqualIndex = INDEX_NONE;
		if (TrimmedLine.FindChar(TEXT('='), EqualIndex) && EqualIndex > 0)
		{
			FString LineKey = TrimmedLine.Left(EqualIndex).TrimStartAndEnd();
			FString LineValue = TrimmedLine.Mid(EqualIndex + 1).TrimStartAndEnd();
			ConfigMap.Add(LineKey, LineValue);
		}
	}
	
	return ConfigMap;
}

FString FAdastreaSettings::LoadConfigValue(const FString& Key, const FString& DefaultValue)
{
	FString ConfigPath = GetConfigFilePath();
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	const FString* Value = ConfigMap.Find(Key);
	return Value ? *Value : DefaultValue;
}

void FAdastreaSettings::SaveConfigValue(const FString& Key, const FString& Value)
{
	FString ConfigPath = GetConfigFilePath();
	FString ConfigDir = FPaths::GetPath(ConfigPath);
	
	// Create directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ConfigDir))
	{
		PlatformFile.CreateDirectoryTree(*ConfigDir);
	}
	
	// Load existing content
	TMap<FString, FString> ConfigMap = LoadConfigMap(ConfigPath);
	
	// Update or add the key
	ConfigMap.FindOrAdd(Key) = Value;
	
	// Write back to file
	FString NewContent;
	NewContent += TEXT("# Adastrea Director Configuration\n");
	NewContent += TEXT("# Auto-generated file\n\n");
	
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
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to save settings to: %s"), *ConfigPath);
	}
}
