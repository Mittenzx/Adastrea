// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Custom log category for Adastrea Director
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaDirector, Log, All);

// Forward declarations
class FPythonBridge;

/**
 * Runtime module for Adastrea Director plugin.
 * Provides core functionality for AI-powered development assistance.
 */
class FAdastreaDirectorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Gets the Python bridge instance.
	 * @return Pointer to the bridge, or nullptr if not initialized
	 */
	FPythonBridge* GetPythonBridge() const { return PythonBridge.Get(); }

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
	/** Python bridge for backend communication */
	TUniquePtr<FPythonBridge> PythonBridge;

	/** Whether the module passed all startup validation */
	bool bIsFullyInitialized = false;

	/** Initialization error message if validation failed */
	FString InitializationError;

	/** Helper to initialize the Python bridge */
	bool InitializePythonBridge();
};
