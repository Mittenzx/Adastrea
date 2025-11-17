// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PythonProcessManager.h"
#include "IPCClient.h"

/**
 * High-level bridge for managing Python backend communication.
 * Combines process management and IPC communication into a single interface.
 */
class ADASTREADIRECTOR_API FPythonBridge
{
public:
	FPythonBridge();
	~FPythonBridge();

	/**
	 * Initializes and starts the Python bridge.
	 * @param PythonExecutable Path to Python executable
	 * @param BackendScript Path to the IPC server script
	 * @param Port Port for IPC communication (default: 5555)
	 * @return true if initialization was successful
	 */
	bool Initialize(const FString& PythonExecutable, const FString& BackendScript, int32 Port = 5555);

	/**
	 * Shuts down the Python bridge gracefully.
	 */
	void Shutdown();

	/**
	 * Checks if the bridge is ready for communication.
	 * @return true if process is running and IPC is connected
	 */
	bool IsReady() const;

	/**
	 * Sends a JSON request to the Python backend.
	 * @param RequestType Type of request (e.g., "query", "plan", "analyze")
	 * @param RequestData JSON data for the request
	 * @param OutResponse Response JSON from the backend
	 * @return true if request completed successfully
	 */
	bool SendRequest(const FString& RequestType, const FString& RequestData, FString& OutResponse);

	/**
	 * Attempts to reconnect to the Python backend.
	 * @return true if reconnection was successful
	 */
	bool Reconnect();

	/**
	 * Gets the current status of the bridge.
	 * @return Status string describing the bridge state
	 */
	FString GetStatus() const;

private:
	/** Process manager for Python subprocess */
	TUniquePtr<FPythonProcessManager> ProcessManager;

	/** IPC client for socket communication */
	TUniquePtr<FIPCClient> IPCClient;

	/** Configuration */
	FString PythonPath;
	FString ScriptPath;
	int32 IPCPort;

	/** Connection retry parameters */
	static constexpr int32 MaxConnectionRetries = 5;
	static constexpr float ConnectionRetryDelay = 1.0f;

	/** Helper to attempt IPC connection with retries */
	bool ConnectWithRetries(int32 MaxRetries);

	/** Helper to build JSON request */
	FString BuildRequestJson(const FString& RequestType, const FString& RequestData) const;
};
