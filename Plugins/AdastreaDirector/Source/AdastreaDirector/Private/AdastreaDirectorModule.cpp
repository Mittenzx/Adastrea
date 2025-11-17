// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaDirectorModule.h"
#include "PythonBridge.h"
#include "Misc/Paths.h"

// Define custom log category for AdastreaDirector
DEFINE_LOG_CATEGORY(LogAdastreaDirector);

#define LOCTEXT_NAMESPACE "FAdastreaDirectorModule"

void FAdastreaDirectorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogAdastreaDirector, Log, TEXT("AdastreaDirector Runtime Module: StartupModule"));

	// Initialize Python bridge
	PythonBridge = MakeUnique<FPythonBridge>();
	
	// Automatically initialize the Python bridge with default settings
	if (InitializePythonBridge())
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("Python Bridge initialized successfully"));
	}
	else
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Python Bridge initialization failed. Python backend may not be available."));
	}
}

void FAdastreaDirectorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogAdastreaDirector, Log, TEXT("AdastreaDirector Runtime Module: ShutdownModule"));

	// Shutdown Python bridge
	if (PythonBridge.IsValid())
	{
		PythonBridge->Shutdown();
		PythonBridge.Reset();
	}
}

bool FAdastreaDirectorModule::InitializePythonBridge()
{
	if (!PythonBridge.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Python Bridge not created"));
		return false;
	}

	// TODO: These paths should come from plugin settings
	// For now, using placeholder paths
	FString PythonExecutable = TEXT("python");  // Or "python3" on some systems
	FString BackendScript = FPaths::Combine(
		FPaths::ProjectPluginsDir(), 
		TEXT("AdastreaDirector/Python/ipc_server.py")
	);
	int32 Port = 5555;

	UE_LOG(LogAdastreaDirector, Log, TEXT("Initializing Python Bridge with:"));
	UE_LOG(LogAdastreaDirector, Log, TEXT("  Python: %s"), *PythonExecutable);
	UE_LOG(LogAdastreaDirector, Log, TEXT("  Script: %s"), *BackendScript);
	UE_LOG(LogAdastreaDirector, Log, TEXT("  Port: %d"), Port);

	return PythonBridge->Initialize(PythonExecutable, BackendScript, Port);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAdastreaDirectorModule, AdastreaDirector)
