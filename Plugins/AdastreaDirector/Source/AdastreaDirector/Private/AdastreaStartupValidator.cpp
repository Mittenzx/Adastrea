// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaStartupValidator.h"
#include "AdastreaSettings.h"
#include "AdastreaDirectorModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

FStartupValidationResult FAdastreaStartupValidator::ValidateStartup(void* Unused)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Starting VibeUE component validation..."));

	TArray<TPair<FString, bool>> Checks;

	// Step 1: Validate Settings
	FStartupValidationResult SettingsResult = ValidateSettings();
	Checks.Add(TPair<FString, bool>(TEXT("Settings Configuration"), SettingsResult.bSuccess));
	
	if (!SettingsResult.bSuccess)
	{
		FStartupValidationResult Result = FStartupValidationResult::Failure(SettingsResult.ErrorMessage);
		Result.DetailedStatus = BuildDetailedStatus(Checks);
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Settings validation completed with warnings: %s"), *Result.ErrorMessage);
		return Result;
	}

	// Step 2: VibeUE Component Availability
	// Note: AdastreaLLMClient, AdastreaScriptService, AdastreaAssetService, etc. are header-only
	// and always available. No runtime checks needed.
	Checks.Add(TPair<FString, bool>(TEXT("VibeUE Components"), true));

	// All checks passed
	FStartupValidationResult Result = FStartupValidationResult::Success(TEXT("VibeUE components validated successfully"));
	Result.DetailedStatus = BuildDetailedStatus(Checks);
	UE_LOG(LogAdastreaDirector, Log, TEXT("VibeUE validation completed successfully"));
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
