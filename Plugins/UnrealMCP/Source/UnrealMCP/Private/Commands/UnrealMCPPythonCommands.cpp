#include "Commands/UnrealMCPPythonCommands.h"
#include "Kismet/KismetSystemLibrary.h"

FUnrealMCPPythonCommands::FUnrealMCPPythonCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPPythonCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("execute_python"))
    {
        return HandleExecutePython(Params);
    }
    
    // Unknown command
    TSharedPtr<FJsonObject> ErrorResult = MakeShareable(new FJsonObject);
    ErrorResult->SetBoolField(TEXT("success"), false);
    ErrorResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown Python command: %s"), *CommandType));
    return ErrorResult;
}

TSharedPtr<FJsonObject> FUnrealMCPPythonCommands::HandleExecutePython(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    // Get Python code from params
    FString PythonCode;
    if (!Params->TryGetStringField(TEXT("code"), PythonCode))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing 'code' parameter"));
        return Result;
    }
    
    // Execute Python via console command
    // This uses the Python console subsystem that's available when Python plugin is enabled
    FString Command = FString::Printf(TEXT("py %s"), *PythonCode);
    
    // Try to execute the command
    bool bSuccess = false;
    
    #if WITH_EDITOR
        // Use GEngine to execute console command
        if (GEngine)
        {
            GEngine->Exec(nullptr, *Command);
            bSuccess = true;
            Result->SetBoolField(TEXT("success"), true);
            Result->SetStringField(TEXT("message"), TEXT("Python command executed"));
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("GEngine is not available"));
        }
    #else
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Python execution is only available in editor builds"));
    #endif
    
    return Result;
}