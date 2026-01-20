// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Result of startup validation
 */
struct FStartupValidationResult
{
	/** Whether validation passed */
	bool bSuccess = false;

	/** Error message if validation failed */
	FString ErrorMessage;

	/** Warnings (non-fatal issues) */
	TArray<FString> Warnings;

	/** Detailed status for logging */
	FString DetailedStatus;

	/** Name of the API key that was successfully used (e.g., "GEMINI_API_KEY") */
	FString UsedKeyName;

	FStartupValidationResult() = default;

	static FStartupValidationResult Success(const FString& Status = TEXT("All checks passed"))
	{
		FStartupValidationResult Result;
		Result.bSuccess = true;
		Result.DetailedStatus = Status;
		return Result;
	}

	static FStartupValidationResult Failure(const FString& Error)
	{
		FStartupValidationResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = Error;
		return Result;
	}
};

/**
 * Validator for plugin startup checks (VibeUE architecture).
 * Performs validation of settings and VibeUE component availability.
 * 
 * Note: Legacy backend connectivity checks have been removed as part of Phase 3 migration.
 * The plugin now uses native C++ VibeUE components (AdastreaLLMClient, AdastreaScriptService, etc.)
 */
class ADASTREADIRECTOR_API FAdastreaStartupValidator
{
public:
	/**
	 * Perform complete startup validation for VibeUE components.
	 * Checks settings and component availability.
	 * @param Unused Legacy parameter kept for API compatibility, will be removed
	 * @return Validation result with status and any error messages
	 */
	static FStartupValidationResult ValidateStartup(void* Unused = nullptr);

	/**
	 * Validate settings configuration only.
	 * @return Validation result
	 */
	static FStartupValidationResult ValidateSettings();

private:
	/** Helper to create detailed status message */
	static FString BuildDetailedStatus(const TArray<TPair<FString, bool>>& Checks);
};
