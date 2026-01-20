// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "AdastreaLLMClient.h"

/**
 * Tool execution result
 */
struct FToolExecutionResult
{
	bool bSuccess = false;
	FString Output;
	FString ErrorMessage;
	TSharedPtr<FJsonObject> Data;
	
	TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * Delegate for tool execution
 * @param Arguments Tool arguments as JSON
 * @return Execution result
 */
DECLARE_DELEGATE_RetVal_OneParam(FToolExecutionResult, FToolExecutor, const TSharedPtr<FJsonObject>& /* Arguments */);

/**
 * Tool registration information
 */
struct FAdastreaToolInfo
{
	FString Name;
	FString Description;
	TSharedPtr<FJsonObject> ParameterSchema;
	FToolExecutor Executor;
	FString Category; // e.g., "Asset", "Python", "Debug"
};

/**
 * Central tool registry and execution system
 */
class ADASTREADIRECTOR_API FAdastreaToolSystem
{
public:
	static FAdastreaToolSystem& Get();
	
	/**
	 * Register a new tool
	 */
	void RegisterTool(const FAdastreaToolInfo& ToolInfo);
	
	/**
	 * Unregister a tool
	 */
	void UnregisterTool(const FString& ToolName);
	
	/**
	 * Execute a tool by name
	 */
	FToolExecutionResult ExecuteTool(const FString& ToolName, const TSharedPtr<FJsonObject>& Arguments);
	
	/**
	 * Get all registered tools (for LLM context)
	 */
	TArray<FToolDefinition> GetAllToolDefinitions() const;
	
	/**
	 * Get tools by category
	 */
	TArray<FToolDefinition> GetToolsByCategory(const FString& Category) const;
	
	/**
	 * Check if tool exists
	 */
	bool HasTool(const FString& ToolName) const;

private:
	FAdastreaToolSystem() = default;
	
	TMap<FString, FAdastreaToolInfo> RegisteredTools;
};
