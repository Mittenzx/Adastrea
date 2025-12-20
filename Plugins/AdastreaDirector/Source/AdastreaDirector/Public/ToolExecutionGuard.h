// Copyright (c) 2024 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Safety guardrails for tool execution to prevent infinite loops and excessive API costs.
 * Implements multiple layers of protection:
 * - Maximum iteration limits
 * - Result size caps
 * - Duplicate execution prevention
 * - Task completion detection
 */
class ADASTREADIRECTOR_API FToolExecutionGuard
{
public:
	FToolExecutionGuard();

	/**
	 * Check if a tool can be executed based on safety guardrails.
	 * @param ToolName Name of the tool to execute
	 * @param Arguments Tool arguments as JSON string
	 * @return true if tool execution is allowed
	 */
	bool CanExecuteTool(const FString& ToolName, const FString& Arguments);

	/**
	 * Record a tool execution and its result.
	 * @param ToolName Name of the executed tool
	 * @param Arguments Tool arguments as JSON string
	 * @param Result Tool execution result
	 */
	void RecordExecution(const FString& ToolName, const FString& Arguments, const FString& Result);

	/**
	 * Reset the guard state for a new conversation.
	 */
	void Reset();

	/**
	 * Check if the iteration limit has been reached.
	 * @return true if max iterations reached
	 */
	bool HasReachedIterationLimit() const;

	/**
	 * Truncate a result string to the maximum allowed size.
	 * @param Result Result string to truncate
	 * @return Truncated result string
	 */
	FString TruncateResult(const FString& Result) const;

	/**
	 * Detect if task appears to be complete based on recent tool executions.
	 * @param RecentToolNames List of recently executed tool names
	 * @param RecentResults List of recent tool results
	 * @return true if task completion is detected
	 */
	bool DetectTaskCompletion(const TArray<FString>& RecentToolNames, const TArray<FString>& RecentResults) const;

	/**
	 * Get current iteration count.
	 * @return Number of tools executed in this conversation
	 */
	int32 GetIterationCount() const { return IterationCount; }

private:
	/** Maximum number of tool iterations per conversation */
	static constexpr int32 MaxIterations = 25;

	/** Maximum size for tool results in characters */
	static constexpr int32 MaxResultSize = 10000;

	/** Current iteration count */
	int32 IterationCount;

	/** Set of executed tool signatures (tool name + arguments hash) to prevent duplicates */
	TSet<FString> ExecutedSignatures;

	/** Last tool name executed */
	FString LastToolName;

	/** Track if last tool was python_execute */
	bool bLastToolWasPythonExecute;

	/** Track if last scene query found results */
	bool bLastSceneQueryFoundResults;

	/** History of recent tool executions (last 5) */
	TArray<FString> RecentToolHistory;

	/** History of recent results (last 5) */
	TArray<FString> RecentResultHistory;

	/**
	 * Generate a signature for a tool execution.
	 * @param ToolName Tool name
	 * @param Arguments Tool arguments
	 * @return Unique signature string
	 */
	FString GenerateSignature(const FString& ToolName, const FString& Arguments) const;

	/**
	 * Check if executing python_execute again would create a loop.
	 * @param ToolName Tool to check
	 * @return true if loop detected
	 */
	bool WouldCreatePythonLoop(const FString& ToolName) const;

	/**
	 * Check if scene query indicates task is complete.
	 * @param ToolName Tool name
	 * @param Result Tool result
	 * @return true if completion indicated
	 */
	bool CheckSceneQueryCompletion(const FString& ToolName, const FString& Result);
};
