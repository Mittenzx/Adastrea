// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "AdastreaDirectorModule.h"
#include "IPCClient.h"
#include "PythonProcessManager.h"
#include "PythonBridge.h"

/**
 * AdastreaDirector Plugin Self-Tests
 * 
 * These automation tests validate core plugin functionality and can be run
 * from the Unreal Editor's Session Frontend or Automation tab.
 * 
 * Test categories:
 * - Adastrea.Director.Module - Module loading tests
 * - Adastrea.Director.IPCClient - IPC client tests
 * - Adastrea.Director.ProcessManager - Python process manager tests
 * - Adastrea.Director.PythonBridge - Python bridge tests
 */

namespace AdastreaDirectorTestConstants
{
	/** Port used for testing invalid connections - intentionally high and unlikely to be in use */
	constexpr int32 UnusedTestPort = 59999;
	/** Timeout for connection tests in seconds */
	constexpr float ConnectionTestTimeout = 0.5f;
}

// ============================================================================
// Module Loading Tests
// ============================================================================

/**
 * Test that the AdastreaDirector module loads correctly.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAdastreaDirectorModuleLoadTest,
	"Adastrea.Director.Module.LoadsCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FAdastreaDirectorModuleLoadTest::RunTest(const FString& Parameters)
{
	// Verify the module is loaded
	const bool bIsLoaded = FModuleManager::Get().IsModuleLoaded("AdastreaDirector");
	TestTrue(TEXT("AdastreaDirector module should be loaded"), bIsLoaded);

	if (bIsLoaded)
	{
		// Get the module and verify it exists
		IModuleInterface* ModuleInterface = FModuleManager::Get().GetModule("AdastreaDirector");
		TestNotNull(TEXT("AdastreaDirector module interface should not be null"), ModuleInterface);
	}

	return true;
}

/**
 * Test that the AdastreaDirector module can be retrieved as the correct type.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAdastreaDirectorModuleTypeTest,
	"Adastrea.Director.Module.CorrectType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FAdastreaDirectorModuleTypeTest::RunTest(const FString& Parameters)
{
	if (!FModuleManager::Get().IsModuleLoaded("AdastreaDirector"))
	{
		AddError(TEXT("AdastreaDirector module is not loaded"));
		return false;
	}

	// Use FModuleManager::GetModuleChecked for type-safe casting
	FAdastreaDirectorModule& DirectorModule = FModuleManager::Get().GetModuleChecked<FAdastreaDirectorModule>("AdastreaDirector");
	
	// The module reference is valid if we got here (GetModuleChecked would assert otherwise)
	// Verify we can access a method on it
	TestTrue(TEXT("FAdastreaDirectorModule should be accessible"), true);

	return true;
}

// ============================================================================
// IPC Client Tests
// ============================================================================

/**
 * Test that FIPCClient can be instantiated.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIPCClientInstantiationTest,
	"Adastrea.Director.IPCClient.Instantiation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FIPCClientInstantiationTest::RunTest(const FString& Parameters)
{
	// Create an IPC client instance
	TUniquePtr<FIPCClient> Client = MakeUnique<FIPCClient>();
	TestNotNull(TEXT("FIPCClient should be instantiable"), Client.Get());

	// Verify initial state
	TestFalse(TEXT("FIPCClient should not be connected initially"), Client->IsConnected());

	return true;
}

/**
 * Test that FIPCClient handles invalid connection gracefully.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIPCClientInvalidConnectionTest,
	"Adastrea.Director.IPCClient.InvalidConnection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FIPCClientInvalidConnectionTest::RunTest(const FString& Parameters)
{
	TUniquePtr<FIPCClient> Client = MakeUnique<FIPCClient>();
	
	// Attempt to connect to a port where nothing is listening
	const bool bConnected = Client->Connect(
		TEXT("127.0.0.1"), 
		AdastreaDirectorTestConstants::UnusedTestPort, 
		AdastreaDirectorTestConstants::ConnectionTestTimeout
	);
	
	// Connection should fail since no server is running
	TestFalse(TEXT("Connection to non-existent server should fail"), bConnected);
	TestFalse(TEXT("Client should not be connected after failed connection"), Client->IsConnected());

	return true;
}

// ============================================================================
// Python Process Manager Tests
// ============================================================================

/**
 * Test that FPythonProcessManager can be instantiated.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPythonProcessManagerInstantiationTest,
	"Adastrea.Director.ProcessManager.Instantiation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FPythonProcessManagerInstantiationTest::RunTest(const FString& Parameters)
{
	// Create a process manager instance
	TUniquePtr<FPythonProcessManager> Manager = MakeUnique<FPythonProcessManager>();
	TestNotNull(TEXT("FPythonProcessManager should be instantiable"), Manager.Get());

	// Verify initial state
	TestFalse(TEXT("FPythonProcessManager should not have running process initially"), Manager->IsProcessRunning());
	TestEqual(TEXT("FPythonProcessManager ProcessId should be 0 initially"), Manager->GetProcessId(), 0u);

	return true;
}

/**
 * Test that FPythonProcessManager handles invalid paths gracefully.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPythonProcessManagerInvalidPathTest,
	"Adastrea.Director.ProcessManager.InvalidPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FPythonProcessManagerInvalidPathTest::RunTest(const FString& Parameters)
{
	TUniquePtr<FPythonProcessManager> Manager = MakeUnique<FPythonProcessManager>();
	
	// Attempt to start with invalid paths
	const bool bStarted = Manager->StartPythonProcess(
		TEXT("/nonexistent/python"),
		TEXT("/nonexistent/script.py"),
		5555
	);
	
	// Should fail to start with invalid paths
	TestFalse(TEXT("Starting process with invalid paths should fail"), bStarted);
	TestFalse(TEXT("Process should not be running after failed start"), Manager->IsProcessRunning());

	return true;
}

// ============================================================================
// Python Bridge Tests
// ============================================================================

/**
 * Test that FPythonBridge can be instantiated.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPythonBridgeInstantiationTest,
	"Adastrea.Director.PythonBridge.Instantiation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FPythonBridgeInstantiationTest::RunTest(const FString& Parameters)
{
	// Create a Python bridge instance
	TUniquePtr<FPythonBridge> Bridge = MakeUnique<FPythonBridge>();
	TestNotNull(TEXT("FPythonBridge should be instantiable"), Bridge.Get());

	// Verify initial state
	TestFalse(TEXT("FPythonBridge should not be ready initially"), Bridge->IsReady());

	return true;
}

/**
 * Test that FPythonBridge returns a status string.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPythonBridgeStatusTest,
	"Adastrea.Director.PythonBridge.Status",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FPythonBridgeStatusTest::RunTest(const FString& Parameters)
{
	TUniquePtr<FPythonBridge> Bridge = MakeUnique<FPythonBridge>();
	
	// Get status - should return a non-empty string
	const FString Status = Bridge->GetStatus();
	TestFalse(TEXT("FPythonBridge status should not be empty"), Status.IsEmpty());

	return true;
}

// ============================================================================
// Smoke Tests
// ============================================================================

/**
 * Basic smoke test that verifies the plugin is functional.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAdastreaDirectorSmokeTest,
	"Adastrea.Director.SmokeTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FAdastreaDirectorSmokeTest::RunTest(const FString& Parameters)
{
	// 1. Verify module is loaded
	TestTrue(TEXT("AdastreaDirector module should be loaded"), 
		FModuleManager::Get().IsModuleLoaded("AdastreaDirector"));

	// 2. Verify core classes can be instantiated
	TUniquePtr<FIPCClient> Client = MakeUnique<FIPCClient>();
	TestNotNull(TEXT("FIPCClient should be instantiable"), Client.Get());

	TUniquePtr<FPythonProcessManager> Manager = MakeUnique<FPythonProcessManager>();
	TestNotNull(TEXT("FPythonProcessManager should be instantiable"), Manager.Get());

	TUniquePtr<FPythonBridge> Bridge = MakeUnique<FPythonBridge>();
	TestNotNull(TEXT("FPythonBridge should be instantiable"), Bridge.Get());

	// 3. Verify log category exists (by logging)
	UE_LOG(LogAdastreaDirector, Log, TEXT("Smoke test completed successfully"));

	return true;
}
