// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "PythonBridge.h"
#include "AdastreaDirectorModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformProcess.h"

FPythonBridge::FPythonBridge()
	: IPCPort(0)
{
	ProcessManager = MakeUnique<FPythonProcessManager>();
	IPCClient = MakeUnique<FIPCClient>();
}

FPythonBridge::~FPythonBridge()
{
	Shutdown();
}

bool FPythonBridge::Initialize(const FString& PythonExecutable, const FString& BackendScript, int32 Port)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Initializing Python Bridge..."));

	// Store configuration
	PythonPath = PythonExecutable;
	ScriptPath = BackendScript;
	IPCPort = Port;

	// Start Python process
	if (!ProcessManager->StartPythonProcess(PythonPath, ScriptPath, IPCPort))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to start Python process"));
		return false;
	}

	// Wait a moment for Python server to initialize
	FPlatformProcess::Sleep(1.0f);

	// Connect to IPC server with retries
	if (!ConnectWithRetries(MaxConnectionRetries))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to connect to Python IPC server"));
		ProcessManager->StopPythonProcess();
		return false;
	}

	UE_LOG(LogAdastreaDirector, Log, TEXT("Python Bridge initialized successfully"));
	return true;
}

void FPythonBridge::Shutdown()
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Shutting down Python Bridge..."));

	// Disconnect IPC first
	if (IPCClient->IsConnected())
	{
		IPCClient->Disconnect();
	}

	// Stop Python process
	ProcessManager->StopPythonProcess();

	UE_LOG(LogAdastreaDirector, Log, TEXT("Python Bridge shut down"));
}

bool FPythonBridge::IsReady() const
{
	return ProcessManager->IsProcessRunning() && IPCClient->IsConnected();
}

bool FPythonBridge::SendRequest(const FString& RequestType, const FString& RequestData, FString& OutResponse)
{
	if (!IsReady())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Bridge not ready for requests"));
		return false;
	}

	// Build JSON request
	FString RequestJson = BuildRequestJson(RequestType, RequestData);

	UE_LOG(LogAdastreaDirector, Verbose, TEXT("Sending request: %s"), *RequestJson);

	// Send request and receive response
	if (!IPCClient->SendRequest(RequestJson, OutResponse))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to send request or receive response"));
		return false;
	}

	UE_LOG(LogAdastreaDirector, Verbose, TEXT("Received response: %s"), *OutResponse);
	return true;
}

bool FPythonBridge::Reconnect()
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("Attempting to reconnect..."));

	// Disconnect current connection
	if (IPCClient->IsConnected())
	{
		IPCClient->Disconnect();
	}

	// Check if process is still running
	if (!ProcessManager->IsProcessRunning())
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Python process not running, restarting..."));
		
		if (!ProcessManager->RestartProcess())
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to restart Python process"));
			return false;
		}

		// Wait for process to initialize
		FPlatformProcess::Sleep(1.0f);
	}

	// Attempt to reconnect
	return ConnectWithRetries(MaxConnectionRetries);
}

FString FPythonBridge::GetStatus() const
{
	if (!ProcessManager->IsProcessRunning())
	{
		return TEXT("Python process not running");
	}

	if (!IPCClient->IsConnected())
	{
		return TEXT("IPC not connected");
	}

	return FString::Printf(TEXT("Ready (PID: %d, Port: %d)"), 
		ProcessManager->GetProcessId(), IPCPort);
}

bool FPythonBridge::ConnectWithRetries(int32 MaxRetries)
{
	for (int32 Retry = 0; Retry < MaxRetries; ++Retry)
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("Connection attempt %d/%d..."), Retry + 1, MaxRetries);

		if (IPCClient->Connect(TEXT("127.0.0.1"), IPCPort, 5.0f))
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Successfully connected to IPC server"));
			return true;
		}

		if (Retry < MaxRetries - 1)
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("Connection failed, retrying in %.1f seconds..."), 
				ConnectionRetryDelay);
			FPlatformProcess::Sleep(ConnectionRetryDelay);
		}
	}

	UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to connect after %d attempts"), MaxRetries);
	return false;
}

FString FPythonBridge::BuildRequestJson(const FString& RequestType, const FString& RequestData) const
{
	// Create JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("type"), RequestType);
	JsonObject->SetStringField(TEXT("data"), RequestData);

	// Serialize to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		return OutputString;
	}

	UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to serialize request JSON"));
	return TEXT("{}");
}
