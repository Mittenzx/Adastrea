// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaStartupValidator.h"
#include "AdastreaSettings.h"
#include "PythonBridge.h"
#include "AdastreaDirectorModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

FStartupValidationResult FAdastreaStartupValidator::ValidateStartup(FPythonBridge* PythonBridge)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Starting comprehensive startup validation..."));

	TArray<TPair<FString, bool>> Checks;

	// Step 1: Validate Settings
	FStartupValidationResult SettingsResult = ValidateSettings();
	Checks.Add(TPair<FString, bool>(TEXT("Settings Configuration"), SettingsResult.bSuccess));
	
	if (!SettingsResult.bSuccess)
	{
		FStartupValidationResult Result = FStartupValidationResult::Failure(SettingsResult.ErrorMessage);
		Result.DetailedStatus = BuildDetailedStatus(Checks);
		UE_LOG(LogAdastreaDirector, Error, TEXT("Startup validation failed: %s"), *Result.ErrorMessage);
		return Result;
	}

	// Step 2: Validate Backend (if bridge provided)
	if (PythonBridge)
	{
		FStartupValidationResult BackendResult = ValidateBackend(PythonBridge);
		Checks.Add(TPair<FString, bool>(TEXT("Backend Connectivity"), BackendResult.bSuccess));
		
		if (!BackendResult.bSuccess)
		{
			FStartupValidationResult Result = FStartupValidationResult::Failure(BackendResult.ErrorMessage);
			Result.DetailedStatus = BuildDetailedStatus(Checks);
			UE_LOG(LogAdastreaDirector, Error, TEXT("Backend validation failed: %s"), *Result.ErrorMessage);
			return Result;
		}

		// Step 3: Validate API Key (requires backend)
		FStartupValidationResult APIKeyResult = ValidateAPIKey(PythonBridge);
		Checks.Add(TPair<FString, bool>(TEXT("API Key Validation"), APIKeyResult.bSuccess));
		
		if (!APIKeyResult.bSuccess)
		{
			FStartupValidationResult Result = FStartupValidationResult::Failure(APIKeyResult.ErrorMessage);
			Result.Warnings = APIKeyResult.Warnings;
			Result.DetailedStatus = BuildDetailedStatus(Checks);
			UE_LOG(LogAdastreaDirector, Warning, TEXT("API key validation failed: %s"), *Result.ErrorMessage);
			return Result;
		}
	}
	else
	{
		Checks.Add(TPair<FString, bool>(TEXT("Backend Connectivity"), false));
		Checks.Add(TPair<FString, bool>(TEXT("API Key Validation"), false));
	}

	// All checks passed
	FStartupValidationResult Result = FStartupValidationResult::Success(TEXT("All startup checks passed successfully"));
	Result.DetailedStatus = BuildDetailedStatus(Checks);
	UE_LOG(LogAdastreaDirector, Log, TEXT("Startup validation completed successfully"));
	return Result;
}

FStartupValidationResult FAdastreaStartupValidator::ValidateSettings()
{
	FAdastreaSettings& Settings = FAdastreaSettings::Get();
	
	FString ErrorMessage;
	if (!Settings.ValidateSettings(ErrorMessage))
	{
		return FStartupValidationResult::Failure(ErrorMessage);
	}

	return FStartupValidationResult::Success(TEXT("Settings validated successfully"));
}

FStartupValidationResult FAdastreaStartupValidator::ValidateBackend(FPythonBridge* PythonBridge)
{
	if (!PythonBridge)
	{
		return FStartupValidationResult::Failure(TEXT("Python bridge is not initialized"));
	}

	// Check if bridge is ready
	if (!PythonBridge->IsReady())
	{
		FString Status = PythonBridge->GetStatus();
		return FStartupValidationResult::Failure(
			FString::Printf(TEXT("Python backend is not ready. Status: %s\n\nPlease ensure:\n1. Python is installed\n2. Required Python packages are installed\n3. Backend script is accessible"), *Status)
		);
	}

	// Test connectivity with a ping
	FString Response;
	if (!PythonBridge->SendRequest(TEXT("ping"), TEXT(""), Response))
	{
		return FStartupValidationResult::Failure(TEXT("Failed to communicate with Python backend. Please check the backend logs."));
	}

	// Verify ping response
	if (!Response.Contains(TEXT("pong")))
	{
		return FStartupValidationResult::Failure(
			FString::Printf(TEXT("Backend responded but with unexpected data. Expected 'pong', got: %s"), *Response)
		);
	}

	return FStartupValidationResult::Success(TEXT("Backend connectivity verified"));
}

FStartupValidationResult FAdastreaStartupValidator::ValidateAPIKey(FPythonBridge* PythonBridge)
{
	if (!PythonBridge || !PythonBridge->IsReady())
	{
		return FStartupValidationResult::Failure(TEXT("Cannot validate API key - backend not ready"));
	}

	FAdastreaSettings& Settings = FAdastreaSettings::Get();
	FString Provider = Settings.GetLLMProvider();
	
	if (Provider != TEXT("gemini") && Provider != TEXT("openai"))
	{
		return FStartupValidationResult::Failure(
			FString::Printf(TEXT("Unknown LLM provider: %s"), *Provider)
		);
	}

	// Build JSON request with provider only (API key is read from .env by Python backend)
	TSharedPtr<FJsonObject> RequestData = MakeShared<FJsonObject>();
	RequestData->SetStringField(TEXT("provider"), Provider);
	// Don't send api_key - the Python backend will read it from .env file

	FString RequestDataString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestDataString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	// Send validation request to backend
	FString Response;
	if (!PythonBridge->SendRequest(TEXT("validate_api_key"), RequestDataString, Response))
	{
		FStartupValidationResult Result = FStartupValidationResult::Failure(
			TEXT("Failed to communicate with backend for API key validation")
		);
		Result.Warnings.Add(TEXT("API key validation could not be performed. Plugin will start but functionality may be limited."));
		return Result;
	}

	// Parse response
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		FStartupValidationResult Result = FStartupValidationResult::Failure(
			TEXT("Failed to parse API key validation response")
		);
		Result.Warnings.Add(TEXT("API key validation returned invalid data. Plugin will start but functionality may be limited."));
		return Result;
	}

	FString Status;
	if (!JsonObject->TryGetStringField(TEXT("status"), Status))
	{
		FStartupValidationResult Result = FStartupValidationResult::Failure(
			TEXT("Invalid validation response format")
		);
		Result.Warnings.Add(TEXT("API key validation response missing status field."));
		return Result;
	}

	if (Status == TEXT("success"))
	{
		bool bValid = false;
		JsonObject->TryGetBoolField(TEXT("valid"), bValid);
		
		if (bValid)
		{
			return FStartupValidationResult::Success(
				FString::Printf(TEXT("%s API key validated successfully (from .env)"), *Provider)
			);
		}
		else
		{
			FString ErrorDetail;
			JsonObject->TryGetStringField(TEXT("error"), ErrorDetail);
			
			return FStartupValidationResult::Failure(
				FString::Printf(
					TEXT("%s API key validation failed.\n\n%s\n\nPlease check your .env file in the project root directory."),
					*Provider,
					*ErrorDetail
				)
			);
		}
	}
	else
	{
		FString Error;
		JsonObject->TryGetStringField(TEXT("error"), Error);
		
		FStartupValidationResult Result = FStartupValidationResult::Failure(
			FString::Printf(TEXT("API key validation failed: %s"), *Error)
		);
		Result.Warnings.Add(TEXT("Plugin will start but AI features may not work correctly."));
		return Result;
	}
}

FString FAdastreaStartupValidator::BuildDetailedStatus(const TArray<TPair<FString, bool>>& Checks)
{
	FString Status = TEXT("Startup Validation Results:\n");
	Status += TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
	
	for (const TPair<FString, bool>& Check : Checks)
	{
		Status += FString::Printf(TEXT("%s %s\n"), 
			Check.Value ? TEXT("✓") : TEXT("✗"),
			*Check.Key
		);
	}
	
	Status += TEXT("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
	return Status;
}
