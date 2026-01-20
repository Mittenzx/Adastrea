// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Custom log category for Adastrea Director
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaDirector, Log, All);

/**
 * Runtime module for Adastrea Director plugin.
 * Provides core functionality for AI-powered development assistance using VibeUE architecture.
 * 
 * The module uses native C++ components:
 * - AdastreaLLMClient: Direct LLM API calls (Gemini, OpenAI)
 * - AdastreaScriptService: In-process Python execution via IPythonScriptPlugin
 * - AdastreaAssetService: Runtime asset discovery via Asset Registry
 * - AdastreaToolSystem: Extensible tool system for AI capabilities
 * - AdastreaMCPServer: MCP protocol server for external AI clients
 */
class FAdastreaDirectorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Checks if the module is fully initialized and ready to use.
	 * @return true if all startup validation passed
	 */
	bool IsFullyInitialized() const { return bIsFullyInitialized; }

	/**
	 * Gets the initialization error message if initialization failed.
	 * @return Error message, or empty string if no error
	 */
	FString GetInitializationError() const { return InitializationError; }

private:
	/** Whether the module passed all startup validation */
	bool bIsFullyInitialized = false;

	/** Initialization error message if validation failed */
	FString InitializationError;

	/** Register built-in asset tools (VibeUE-style) */
	void RegisterAssetTools();

	/** Register built-in Python execution tools (VibeUE-style, DISABLED by default for security) */
	void RegisterPythonTools();
};
