// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaScriptService.h"
#include "AdastreaDirectorModule.h"
#include "Misc/DateTime.h"

FAdastreaScriptResult FAdastreaScriptService::ExecuteCode(
	const FString& Code,
	bool bPrivateScope)
{
	if (!IsPythonAvailable())
	{
		FAdastreaScriptResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Python plugin is not available");
		return Result;
	}

	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	
	// Setup command
	FPythonCommandEx Command;
	Command.Command = Code;
	Command.ExecutionMode = EPythonCommandExecutionMode::ExecuteFile;
	Command.FileExecutionScope = bPrivateScope 
		? EPythonFileExecutionScope::Private 
		: EPythonFileExecutionScope::Public;

	// Execute with timing
	double StartTime = FPlatformTime::Seconds();
	bool bSuccess = PythonPlugin->ExecPythonCommandEx(Command);
	double ExecutionTimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	return ConvertResult(Command, ExecutionTimeMs, bSuccess);
}

FAdastreaScriptResult FAdastreaScriptService::EvaluateExpression(const FString& Expression)
{
	if (!IsPythonAvailable())
	{
		FAdastreaScriptResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Python plugin is not available");
		return Result;
	}

	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	
	FPythonCommandEx Command;
	Command.Command = Expression;
	Command.ExecutionMode = EPythonCommandExecutionMode::EvaluateStatement;

	double StartTime = FPlatformTime::Seconds();
	bool bSuccess = PythonPlugin->ExecPythonCommandEx(Command);
	double ExecutionTimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	return ConvertResult(Command, ExecutionTimeMs, bSuccess);
}

bool FAdastreaScriptService::IsPythonAvailable()
{
	return IPythonScriptPlugin::Get() != nullptr;
}

FString FAdastreaScriptService::GetPythonInfo()
{
	if (!IsPythonAvailable())
	{
		return TEXT("Python not available");
	}

	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	
	// Execute version check
	FPythonCommandEx Command;
	Command.Command = TEXT("import sys; print(f'Python {sys.version}')");
	Command.ExecutionMode = EPythonCommandExecutionMode::ExecuteFile;
	
	if (PythonPlugin->ExecPythonCommandEx(Command))
	{
		FString Output = Command.CommandResult;
		Output.TrimStartAndEndInline();
		return Output;
	}
	
	return TEXT("Unable to get Python version");
}

FAdastreaScriptResult FAdastreaScriptService::ConvertResult(
	const FPythonCommandEx& CommandEx,
	float ExecutionTimeMs,
	bool bExecutionSuccess)
{
	FAdastreaScriptResult Result;
	Result.Output = CommandEx.CommandResult;
	Result.ExecutionTimeMs = ExecutionTimeMs;

	// Extract error messages and detect error entries
	bool bHasError = false;
	for (const FPythonLogOutputEntry& Entry : CommandEx.LogOutput)
	{
		if (Entry.Type == EPythonLogOutputType::Error)
		{
			bHasError = true;
			Result.ErrorMessage += Entry.Output + TEXT("\n");
		}
	}

	Result.ErrorMessage.TrimEndInline();

	// Success if the Python command executed successfully and there are no error log entries,
	// regardless of whether any output was produced.
	Result.bSuccess = bExecutionSuccess && !bHasError;

	return Result;
}
