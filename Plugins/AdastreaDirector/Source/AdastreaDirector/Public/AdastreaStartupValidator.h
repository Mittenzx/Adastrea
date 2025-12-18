// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FPythonBridge;

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
 * Validator for plugin startup checks.
 * Performs comprehensive validation of settings, backend connectivity, and API keys.
 */
class ADASTREADIRECTOR_API FAdastreaStartupValidator
{
public:
	/**
	 * Perform complete startup validation.
	 * Checks settings, backend connectivity, and API key validity.
	 * @param PythonBridge The Python bridge to validate (optional, can be nullptr)
	 * @return Validation result with status and any error messages
	 */
	static FStartupValidationResult ValidateStartup(FPythonBridge* PythonBridge = nullptr);

	/**
	 * Validate settings configuration only (no backend check).
	 * @return Validation result
	 */
	static FStartupValidationResult ValidateSettings();

	/**
	 * Validate backend connectivity and health.
	 * @param PythonBridge The Python bridge to test
	 * @return Validation result
	 */
	static FStartupValidationResult ValidateBackend(FPythonBridge* PythonBridge);

	/**
	 * Validate API key by attempting to use it (requires backend).
	 * @param PythonBridge The Python bridge to use for validation
	 * @return Validation result
	 */
	static FStartupValidationResult ValidateAPIKey(FPythonBridge* PythonBridge);

private:
	/** Helper to create detailed status message */
	static FString BuildDetailedStatus(const TArray<TPair<FString, bool>>& Checks);
};
