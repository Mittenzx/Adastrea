// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPythonScriptPlugin.h"
#include "PythonScriptTypes.h"

struct FAdastreaScriptResult
{
	bool bSuccess = false;
	FString Output;
	FString ErrorMessage;
	float ExecutionTimeMs = 0.0f;
};

/**
 * Service for executing Python code in Unreal Engine using IPythonScriptPlugin
 * 
 * ⚠️ SECURITY WARNING:
 * This service executes arbitrary Python code directly in the Unreal Editor process.
 * Never execute untrusted code, including LLM-generated code, without human review.
 * 
 * Recommended safety measures:
 * 1. Require explicit user confirmation before executing any LLM-generated Python
 * 2. Implement a whitelist of allowed operations/modules
 * 3. Display the code to the user for review before execution
 * 4. Consider sandboxing or running in a restricted environment
 * 5. Validate and sanitize all inputs
 * 
 * See "Security Considerations" section below for detailed mitigation strategies.
 */
class ADASTREADIRECTOR_API FAdastreaScriptService
{
public:
	/**
	 * Execute Python code
	 * @param Code Python code to execute
	 * @param bPrivateScope If true, uses isolated scope. If false, uses shared console state
	 * @return Execution result with output and errors
	 */
	static FAdastreaScriptResult ExecuteCode(
		const FString& Code,
		bool bPrivateScope = true
	);

	/**
	 * Evaluate a Python expression and return the result
	 * @param Expression Single Python expression (e.g., "2 + 2")
	 * @return Execution result with expression value
	 */
	static FAdastreaScriptResult EvaluateExpression(const FString& Expression);

	/**
	 * Check if Python is available
	 * @return True if Python plugin is initialized and ready
	 */
	static bool IsPythonAvailable();

	/**
	 * Get Python version and information
	 * @return Python version string
	 */
	static FString GetPythonInfo();

private:
	static FAdastreaScriptResult ConvertResult(
		const FPythonCommandEx& CommandEx,
		float ExecutionTimeMs,
		bool bExecutionSuccess
	);
};
