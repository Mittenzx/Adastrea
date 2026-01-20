// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

/**
 * Settings manager for Adastrea Director plugin.
 * Handles loading/saving configuration including API keys and backend settings.
 */
class ADASTREADIRECTOR_API FAdastreaSettings
{
public:
	/** Get the singleton instance */
	static FAdastreaSettings& Get();

	/** Load settings from config file */
	void LoadSettings();

	/** Save settings to config file */
	void SaveSettings();

	/** Get Gemini API key */
	FString GetGeminiAPIKey() const { return GeminiAPIKey; }

	/** Get OpenAI API key */
	FString GetOpenAIAPIKey() const { return OpenAIAPIKey; }

	/** Get current LLM provider */
	FString GetLLMProvider() const { return LLMProvider; }

	/** Get current embedding provider */
	FString GetEmbeddingProvider() const { return EmbeddingProvider; }

	/** Set Gemini API key */
	void SetGeminiAPIKey(const FString& Key) { GeminiAPIKey = Key; }

	/** Set OpenAI API key */
	void SetOpenAIAPIKey(const FString& Key) { OpenAIAPIKey = Key; }

	/** Set LLM provider */
	void SetLLMProvider(const FString& Provider) { LLMProvider = Provider; }

	/** Set embedding provider */
	void SetEmbeddingProvider(const FString& Provider) { EmbeddingProvider = Provider; }

	/** Validate current settings */
	bool ValidateSettings(FString& OutErrorMessage) const;

	/** Check if API key is configured for current provider */
	bool HasAPIKey() const;

	/** Get config file path */
	static FString GetConfigFilePath();

private:
	FAdastreaSettings();
	~FAdastreaSettings() = default;

	// Prevent copying
	FAdastreaSettings(const FAdastreaSettings&) = delete;
	FAdastreaSettings& operator=(const FAdastreaSettings&) = delete;

	/** Helper to load config map from file */
	TMap<FString, FString> LoadConfigMap(const FString& ConfigPath);

	/** Helper to load a single config value */
	FString LoadConfigValue(const FString& Key, const FString& DefaultValue);

	/** Helper to save a single config value */
	void SaveConfigValue(const FString& Key, const FString& Value);

	// Settings
	FString GeminiAPIKey;
	FString OpenAIAPIKey;
	FString LLMProvider;
	FString EmbeddingProvider;
	int32 DefaultFontSize;
	bool bAutoSaveSettings;
	bool bShowTimestamps;
};
