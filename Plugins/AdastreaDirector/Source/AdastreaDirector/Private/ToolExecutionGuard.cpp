// Copyright (c) 2024 Mittenzx. All Rights Reserved.

#include "ToolExecutionGuard.h"
#include "AdastreaDirectorModule.h"
#include "Misc/SecureHash.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FToolExecutionGuard::FToolExecutionGuard()
	: IterationCount(0)
	, bLastToolWasPythonExecute(false)
	, bLastSceneQueryFoundResults(false)
{
}

bool FToolExecutionGuard::CanExecuteTool(const FString& ToolName, const FString& Arguments)
{
	// Check iteration limit
	if (IterationCount >= MaxIterations)
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Tool execution blocked: iteration limit (%d) reached"), MaxIterations);
		return false;
	}

	// Check for python_execute loops
	if (WouldCreatePythonLoop(ToolName))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Tool execution blocked: python_execute loop detected"));
		return false;
	}

	// Check for duplicate executions
	FString Signature = GenerateSignature(ToolName, Arguments);
	if (ExecutedSignatures.Contains(Signature))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Tool execution blocked: duplicate signature detected"));
		return false;
	}

	// Check if task appears complete after scene verification
	if (bLastSceneQueryFoundResults && ToolName == TEXT("python_execute"))
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("Tool execution blocked: task appears complete after scene verification"));
		return false;
	}

	return true;
}

void FToolExecutionGuard::RecordExecution(const FString& ToolName, const FString& Arguments, const FString& Result)
{
	// Increment iteration count
	IterationCount++;

	// Record signature to prevent duplicates
	FString Signature = GenerateSignature(ToolName, Arguments);
	ExecutedSignatures.Add(Signature);

	// Update last tool tracking
	LastToolName = ToolName;
	bLastToolWasPythonExecute = (ToolName == TEXT("python_execute"));

	// Check scene query results
	CheckSceneQueryCompletion(ToolName, Result);

	// Update recent history (keep last 5)
	RecentToolHistory.Add(ToolName);
	RecentResultHistory.Add(Result);
	if (RecentToolHistory.Num() > 5)
	{
		RecentToolHistory.RemoveAt(0);
		RecentResultHistory.RemoveAt(0);
	}

	UE_LOG(LogAdastreaDirector, Verbose, TEXT("Tool execution recorded: %s (iteration %d/%d)"), 
		*ToolName, IterationCount, MaxIterations);
}

void FToolExecutionGuard::Reset()
{
	IterationCount = 0;
	ExecutedSignatures.Empty();
	LastToolName.Empty();
	bLastToolWasPythonExecute = false;
	bLastSceneQueryFoundResults = false;
	RecentToolHistory.Empty();
	RecentResultHistory.Empty();

	UE_LOG(LogAdastreaDirector, Log, TEXT("Tool execution guard reset for new conversation"));
}

bool FToolExecutionGuard::HasReachedIterationLimit() const
{
	return IterationCount >= MaxIterations;
}

FString FToolExecutionGuard::TruncateResult(const FString& Result) const
{
	if (Result.Len() <= MaxResultSize)
	{
		return Result;
	}

	// Truncate and add indicator
	FString Truncated = Result.Left(MaxResultSize - 50);
	Truncated += TEXT("\n\n[Result truncated - exceeded ") + FString::FromInt(MaxResultSize) + TEXT(" character limit]");
	
	UE_LOG(LogAdastreaDirector, Warning, TEXT("Tool result truncated from %d to %d characters"), 
		Result.Len(), Truncated.Len());
	
	return Truncated;
}

bool FToolExecutionGuard::DetectTaskCompletion(const TArray<FString>& RecentToolNames, const TArray<FString>& RecentResults) const
{
	// Need at least 2 recent executions to detect completion
	if (RecentToolNames.Num() < 2)
	{
		return false;
	}

	// Pattern 1: python_execute followed by scene_query/screenshot that confirms success
	if (RecentToolNames.Num() >= 2)
	{
		FString LastTool = RecentToolNames.Last();
		FString SecondLastTool = RecentToolNames[RecentToolNames.Num() - 2];
		
		if (SecondLastTool == TEXT("python_execute") && 
			(LastTool == TEXT("scene_query") || LastTool == TEXT("screenshot")))
		{
			// Check if last result indicates success
			FString LastResult = RecentResults.Last();
			if (LastResult.Contains(TEXT("\"status\":\"ok\"")) || 
				LastResult.Contains(TEXT("\"status\": \"ok\"")))
			{
				UE_LOG(LogAdastreaDirector, Log, TEXT("Task completion detected: python_execute followed by successful verification"));
				return true;
			}
		}
	}

	// Pattern 2: Multiple consecutive verification tools without changes
	if (RecentToolNames.Num() >= 3)
	{
		int32 ConsecutiveVerifications = 0;
		for (int32 i = RecentToolNames.Num() - 1; i >= 0 && i >= RecentToolNames.Num() - 3; i--)
		{
			FString Tool = RecentToolNames[i];
			if (Tool == TEXT("scene_query") || Tool == TEXT("screenshot") || Tool == TEXT("selected_actors"))
			{
				ConsecutiveVerifications++;
			}
			else
			{
				break;
			}
		}

		if (ConsecutiveVerifications >= 2)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Task completion detected: multiple verification attempts without action"));
			return true;
		}
	}

	return false;
}

FString FToolExecutionGuard::GenerateSignature(const FString& ToolName, const FString& Arguments) const
{
	// Create unique signature from tool name and arguments
	FString Combined = ToolName + TEXT("::") + Arguments;
	
	// Use Blake3 hash for compact signature (MD5 is deprecated)
	const FTCHARToUTF8 Utf8Combined(*Combined);
	const FBlake3Hash Hash = FBlake3::HashBuffer(Utf8Combined.Get(), Utf8Combined.Length());
	
	// Convert hash bytes to hex string
	const uint8* HashBytes = Hash.GetBytes();
	FString HashString;
	for (int32 i = 0; i < 32; ++i)
	{
		HashString += FString::Printf(TEXT("%02x"), HashBytes[i]);
	}
	return HashString;
}

bool FToolExecutionGuard::WouldCreatePythonLoop(const FString& ToolName) const
{
	// Prevent consecutive python_execute calls without verification in between
	if (ToolName == TEXT("python_execute") && bLastToolWasPythonExecute)
	{
		// Check if there was a verification tool in between
		if (RecentToolHistory.Num() >= 2)
		{
			FString LastTool = RecentToolHistory.Last();
			bool bHadVerification = (LastTool == TEXT("scene_query") || 
									LastTool == TEXT("screenshot") || 
									LastTool == TEXT("selected_actors"));
			
			if (!bHadVerification)
			{
				return true;
			}
		}
		else
		{
			// Not enough history, be conservative
			return true;
		}
	}

	return false;
}

bool FToolExecutionGuard::CheckSceneQueryCompletion(const FString& ToolName, const FString& Result)
{
	// Check if scene_query returned non-empty results
	if (ToolName == TEXT("scene_query"))
	{
		// Try to parse JSON result
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			// Check if results array exists and has items
			if (JsonObject->HasField(TEXT("results")))
			{
				const TArray<TSharedPtr<FJsonValue>>* ResultsArray;
				if (JsonObject->TryGetArrayField(TEXT("results"), ResultsArray))
				{
					bLastSceneQueryFoundResults = (ResultsArray->Num() > 0);
					return bLastSceneQueryFoundResults;
				}
			}
		}
	}

	return false;
}
