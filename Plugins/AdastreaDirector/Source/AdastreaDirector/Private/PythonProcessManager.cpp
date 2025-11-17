// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "PythonProcessManager.h"
#include "AdastreaDirectorModule.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

FPythonProcessManager::FPythonProcessManager()
	: ProcessId(0)
	, IPCPort(0)
{
}

FPythonProcessManager::~FPythonProcessManager()
{
	StopPythonProcess();
}

bool FPythonProcessManager::StartPythonProcess(const FString& PythonExecutablePath, const FString& BackendScriptPath, int32 Port)
{
	// Stop any existing process
	if (IsProcessRunning())
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Python process already running. Stopping existing process."));
		StopPythonProcess();
	}

	// Validate inputs
	if (PythonExecutablePath.IsEmpty() || BackendScriptPath.IsEmpty())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Invalid Python executable or script path."));
		return false;
	}

	if (Port <= 0 || Port > 65535)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Invalid port number: %d"), Port);
		return false;
	}

	// Store parameters for potential restart
	PythonPath = PythonExecutablePath;
	ScriptPath = BackendScriptPath;
	IPCPort = Port;

	// Build command line arguments
	FString Args = FString::Printf(TEXT("\"%s\" --port %d"), *ScriptPath, Port);

	// Create the process
	UE_LOG(LogAdastreaDirector, Log, TEXT("Starting Python process: %s %s"), *PythonPath, *Args);
	
	// Launch the process
	// bLaunchDetached = false: We want to manage the process lifecycle
	// bLaunchHidden = true: Don't show console window
	// bLaunchReallyHidden = true: Completely hide the process window
	uint32 OutProcessId = 0;
	ProcessHandle = FPlatformProcess::CreateProc(
		*PythonPath,
		*Args,
		false,  // bLaunchDetached
		true,   // bLaunchHidden
		true,   // bLaunchReallyHidden
		&OutProcessId,
		0,      // PriorityModifier
		nullptr, // OptionalWorkingDirectory
		nullptr  // PipeWriteChild
	);

	if (ProcessHandle.IsValid())
	{
		ProcessId = OutProcessId;
		UE_LOG(LogAdastreaDirector, Log, TEXT("Python process started successfully. PID: %d"), ProcessId);
		
		// Give the process a moment to initialize
		FPlatformProcess::Sleep(0.5f);
		
		// Verify it's still running
		if (VerifyProcessAlive())
		{
			return true;
		}
		else
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Python process terminated immediately after starting."));
			ProcessHandle = FProcHandle();
			ProcessId = 0;
			return false;
		}
	}
	else
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to start Python process."));
		ProcessId = 0;
		return false;
	}
}

void FPythonProcessManager::StopPythonProcess()
{
	if (ProcessHandle.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("Stopping Python process (PID: %d)"), ProcessId);

		// Try graceful termination first
		FPlatformProcess::TerminateProc(ProcessHandle, true);
		
		// Wait a moment for graceful shutdown
		FPlatformProcess::Sleep(0.2f);
		
		// Force kill if still running
		FProcHandle HandleCopy = ProcessHandle;
		if (FPlatformProcess::IsProcRunning(HandleCopy))
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("Forcing Python process termination."));
			FPlatformProcess::TerminateProc(ProcessHandle, false);
		}

		// Close the process handle
		FPlatformProcess::CloseProc(ProcessHandle);
		ProcessHandle = FProcHandle();
		ProcessId = 0;

		UE_LOG(LogAdastreaDirector, Log, TEXT("Python process stopped."));
	}
}

bool FPythonProcessManager::IsProcessRunning() const
{
	return ProcessHandle.IsValid() && VerifyProcessAlive();
}

uint32 FPythonProcessManager::GetProcessId() const
{
	return ProcessId;
}

bool FPythonProcessManager::RestartProcess()
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Restarting Python process..."));
	
	StopPythonProcess();
	
	// Wait a moment before restarting
	FPlatformProcess::Sleep(0.5f);
	
	return StartPythonProcess(PythonPath, ScriptPath, IPCPort);
}

bool FPythonProcessManager::VerifyProcessAlive() const
{
	if (!ProcessHandle.IsValid())
	{
		return false;
	}

	FProcHandle HandleCopy = ProcessHandle;
	return FPlatformProcess::IsProcRunning(HandleCopy);
}
