#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Python execution commands
 */
class UNREALMCP_API FUnrealMCPPythonCommands
{
public:
    FUnrealMCPPythonCommands();

    // Handle Python commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Specific Python command handlers
    TSharedPtr<FJsonObject> HandleExecutePython(const TSharedPtr<FJsonObject>& Params);
};