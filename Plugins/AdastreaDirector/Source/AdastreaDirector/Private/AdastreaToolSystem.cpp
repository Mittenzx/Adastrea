// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaToolSystem.h"
#include "AdastreaDirectorModule.h"

FAdastreaToolSystem& FAdastreaToolSystem::Get()
{
	static FAdastreaToolSystem Instance;
	return Instance;
}

void FAdastreaToolSystem::RegisterTool(const FAdastreaToolInfo& ToolInfo)
{
	if (ToolInfo.Name.IsEmpty())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Cannot register tool with empty name"));
		return;
	}
	
	if (RegisteredTools.Contains(ToolInfo.Name))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Overwriting existing tool: %s"), *ToolInfo.Name);
	}
	
	RegisteredTools.Add(ToolInfo.Name, ToolInfo);
	UE_LOG(LogAdastreaDirector, Log, TEXT("Registered tool: %s"), *ToolInfo.Name);
}

void FAdastreaToolSystem::UnregisterTool(const FString& ToolName)
{
	RegisteredTools.Remove(ToolName);
}

FToolExecutionResult FAdastreaToolSystem::ExecuteTool(
	const FString& ToolName,
	const TSharedPtr<FJsonObject>& Arguments)
{
	FToolExecutionResult Result;
	
	if (!RegisteredTools.Contains(ToolName))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Tool not found: %s"), *ToolName);
		return Result;
	}
	
	const FAdastreaToolInfo& ToolInfo = RegisteredTools[ToolName];
	
	if (!ToolInfo.Executor.IsBound())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Tool has no executor: %s"), *ToolName);
		return Result;
	}
	
	UE_LOG(LogAdastreaDirector, Log, TEXT("Executing tool: %s"), *ToolName);
	
	// Execute the tool
	Result = ToolInfo.Executor.Execute(Arguments);
	
	return Result;
}

TArray<FToolDefinition> FAdastreaToolSystem::GetAllToolDefinitions() const
{
	TArray<FToolDefinition> Definitions;
	
	for (const auto& Pair : RegisteredTools)
	{
		FToolDefinition Def;
		Def.Name = Pair.Value.Name;
		Def.Description = Pair.Value.Description;
		Def.Parameters = Pair.Value.ParameterSchema;
		Definitions.Add(Def);
	}
	
	return Definitions;
}

TArray<FToolDefinition> FAdastreaToolSystem::GetToolsByCategory(const FString& Category) const
{
	TArray<FToolDefinition> Definitions;
	
	for (const auto& Pair : RegisteredTools)
	{
		if (Pair.Value.Category == Category)
		{
			FToolDefinition Def;
			Def.Name = Pair.Value.Name;
			Def.Description = Pair.Value.Description;
			Def.Parameters = Pair.Value.ParameterSchema;
			Definitions.Add(Def);
		}
	}
	
	return Definitions;
}

bool FAdastreaToolSystem::HasTool(const FString& ToolName) const
{
	return RegisteredTools.Contains(ToolName);
}

TSharedPtr<FJsonObject> FToolExecutionResult::ToJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetBoolField(TEXT("success"), bSuccess);
	Json->SetStringField(TEXT("output"), Output);
	
	if (!ErrorMessage.IsEmpty())
	{
		Json->SetStringField(TEXT("error"), ErrorMessage);
	}
	
	if (Data.IsValid())
	{
		Json->SetObjectField(TEXT("data"), Data);
	}
	
	return Json;
}
