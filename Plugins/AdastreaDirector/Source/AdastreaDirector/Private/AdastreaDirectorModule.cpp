// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaDirectorModule.h"
#include "AdastreaSettings.h"
#include "AdastreaStartupValidator.h"
#include "AdastreaToolSystem.h"
#include "AdastreaAssetService.h"
#include "AdastreaScriptService.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// Define custom log category for AdastreaDirector
DEFINE_LOG_CATEGORY(LogAdastreaDirector);

#define LOCTEXT_NAMESPACE "FAdastreaDirectorModule"

void FAdastreaDirectorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogAdastreaDirector, Log, TEXT("AdastreaDirector Runtime Module: StartupModule - VibeUE Architecture"));

	// Register built-in tools (VibeUE-style architecture)
	RegisterAssetTools();
	RegisterPythonTools();
	UE_LOG(LogAdastreaDirector, Log, TEXT("Registered built-in tools"));

	// Perform startup validation (VibeUE components)
	FStartupValidationResult ValidationResult = FAdastreaStartupValidator::ValidateStartup(nullptr);
	
	if (ValidationResult.bSuccess)
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("Startup validation passed - VibeUE components ready"));
		bIsFullyInitialized = true;
	}
	else
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Startup validation completed with warnings: %s"), *ValidationResult.ErrorMessage);
		InitializationError = ValidationResult.ErrorMessage;
		bIsFullyInitialized = true; // Still mark as initialized since VibeUE components work without backend
		
		// Log warnings if any
		for (const FString& Warning : ValidationResult.Warnings)
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("  Warning: %s"), *Warning);
		}
	}
	
	UE_LOG(LogAdastreaDirector, Log, TEXT("AdastreaDirector module startup complete. VibeUE architecture active."));
}

void FAdastreaDirectorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogAdastreaDirector, Log, TEXT("AdastreaDirector Runtime Module: ShutdownModule"));
}

void FAdastreaDirectorModule::RegisterAssetTools()
{
	// search_assets tool
	FAdastreaToolInfo SearchAssetsTool;
	SearchAssetsTool.Name = TEXT("search_assets");
	SearchAssetsTool.Description = TEXT("Search for assets in the project by name pattern and/or class type");
	SearchAssetsTool.Category = TEXT("Asset");
	
	// Parameter schema
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShared<FJsonObject>();
	
	TSharedPtr<FJsonObject> PatternProp = MakeShared<FJsonObject>();
	PatternProp->SetStringField(TEXT("type"), TEXT("string"));
	PatternProp->SetStringField(TEXT("description"), TEXT("Name pattern to search for (supports wildcards)"));
	Properties->SetObjectField(TEXT("pattern"), PatternProp);
	
	TSharedPtr<FJsonObject> ClassProp = MakeShared<FJsonObject>();
	ClassProp->SetStringField(TEXT("type"), TEXT("string"));
	ClassProp->SetStringField(TEXT("description"), TEXT("Asset class filter (e.g., Blueprint, Material)"));
	Properties->SetObjectField(TEXT("class"), ClassProp);
	
	Schema->SetObjectField(TEXT("properties"), Properties);
	SearchAssetsTool.ParameterSchema = Schema;
	
	// Executor
	SearchAssetsTool.Executor.BindLambda([](const TSharedPtr<FJsonObject>& Args) -> FToolExecutionResult
	{
		FToolExecutionResult Result;
		
		FString Pattern = TEXT("*");
		Args->TryGetStringField(TEXT("pattern"), Pattern);
		
		FString ClassName;
		Args->TryGetStringField(TEXT("class"), ClassName);
		
		TArray<FAssetInfo> Assets = FAdastreaAssetService::SearchAssets(Pattern, ClassName, 100);
		
		// Build JSON response
		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetNumberField(TEXT("count"), Assets.Num());
		
		TArray<TSharedPtr<FJsonValue>> AssetsArray;
		for (const FAssetInfo& Asset : Assets)
		{
			AssetsArray.Add(MakeShared<FJsonValueObject>(Asset.ToJson()));
		}
		Data->SetArrayField(TEXT("assets"), AssetsArray);
		
		Result.bSuccess = true;
		Result.Output = FString::Printf(TEXT("Found %d assets"), Assets.Num());
		Result.Data = Data;
		
		return Result;
	});
	
	FAdastreaToolSystem::Get().RegisterTool(SearchAssetsTool);
}

void FAdastreaDirectorModule::RegisterPythonTools()
{
	// execute_python tool
	FAdastreaToolInfo ExecutePythonTool;
	ExecutePythonTool.Name = TEXT("execute_python");
	ExecutePythonTool.Description = TEXT("Execute Python code in the Unreal Editor. SECURITY: Only execute trusted, reviewed code.");
	ExecutePythonTool.Category = TEXT("Python");
	
	TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	
	TSharedPtr<FJsonObject> Properties = MakeShared<FJsonObject>();
	
	TSharedPtr<FJsonObject> CodeProp = MakeShared<FJsonObject>();
	CodeProp->SetStringField(TEXT("type"), TEXT("string"));
	CodeProp->SetStringField(TEXT("description"), TEXT("Python code to execute"));
	Properties->SetObjectField(TEXT("code"), CodeProp);
	
	Schema->SetObjectField(TEXT("properties"), Properties);
	
	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShared<FJsonValueString>(TEXT("code")));
	Schema->SetArrayField(TEXT("required"), Required);
	
	ExecutePythonTool.ParameterSchema = Schema;
	
	// ⚠️ CRITICAL SECURITY WARNING:
	// This tool is DISABLED by default because it executes arbitrary Python code.
	// An attacker controlling tool inputs (e.g., via MCP or compromised client) can run
	// arbitrary Python in the Unreal Editor process, leading to full project compromise.
	// 
	// DO NOT ENABLE unless you implement:
	// 1. Strict allowlist of permitted operations/modules
	// 2. Interactive user confirmation in the editor
	// 3. Code review and approval workflow
	// 4. Audit logging of all executed code
	// 5. Sandboxing or restricted execution environment
	//
	// See Section 1, Step 5 "Security Considerations" for detailed mitigation strategies.
	
	// Executor is DISABLED for security - do not execute arbitrary Python from untrusted inputs
	ExecutePythonTool.Executor.BindLambda([](const TSharedPtr<FJsonObject>& Args) -> FToolExecutionResult
	{
		FToolExecutionResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT(
			"SECURITY: The 'execute_python' tool is DISABLED by default. "
			"This tool executes arbitrary Python code which poses severe security risks. "
			"Do NOT enable without implementing proper security controls:\n"
			"1. Allowlist permitted operations/modules\n"
			"2. Require explicit user approval in editor UI\n"
			"3. Implement code review workflow\n"
			"4. Add comprehensive audit logging\n"
			"5. Use sandboxed execution environment\n\n"
			"See VIBEUE_IMPLEMENTATION_GUIDE.md Section 1, Step 5 for security guidance.\n\n"
			"If you understand the risks and have implemented proper controls, "
			"replace this lambda with a hardened execution wrapper."
		);
		
		return Result;
	});
	
	FAdastreaToolSystem::Get().RegisterTool(ExecutePythonTool);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAdastreaDirectorModule, AdastreaDirector)
