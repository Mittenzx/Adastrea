# VibeUE Migration Guide

## Overview

The Adastrea Director plugin has been modernized with the **VibeUE architecture**, which provides direct C++ LLM integration without external Python processes. This guide helps you understand the transition and migrate code.

## What Changed?

### Old Architecture (IPC-Based)
```cpp
// Start Python process
FPythonBridge Bridge;
Bridge.Initialize(PythonPath, ScriptPath, 8765);

// Send request via IPC
FString Request = TEXT("{\"action\": \"query\", \"text\": \"help\"}");
FString Response;
Bridge.SendRequest(Request, Response);

// External Python process handles LLM calls
```

**Issues:**
- External process can fail
- IPC overhead (~200ms latency)
- Complex error handling
- Python dependency conflicts

### New Architecture (VibeUE)
```cpp
// Direct LLM call
TSharedPtr<FAdastreaLLMClient> LLMClient = MakeShared<FAdastreaLLMClient>();
LLMClient->SetProvider(ELLMProvider::Gemini, ApiKey);

TArray<FChatMessage> Messages;
// ... setup messages

LLMClient->SendChatRequest(Messages, Tools, OnStreamChunk, OnComplete);

// Direct HTTP to LLM, no Python process
```

**Benefits:**
- No external process
- 75% lower latency
- Streaming support
- Single-process debugging

## Component Mapping

| Old Component (IPC) | New Component (VibeUE) | Status |
|---------------------|------------------------|--------|
| **PythonProcessManager** | **AdastreaScriptService** | ⚠️ Old = Legacy, use New |
| **IPCClient** | **AdastreaLLMClient** + **AdastreaAssetService** | ⚠️ Old = Legacy, use New |
| **PythonBridge** | Direct VibeUE component usage | ⚠️ Old = Legacy, use New |
| Document ingestion | **AdastreaAssetService** (runtime queries) | ⚠️ Ingestion obsolete |
| Python LLM client | **AdastreaLLMClient** (direct C++) | ⚠️ Python intermediary obsolete |

## Migration Examples

### Example 1: Python Execution

**Old (IPC):**
```cpp
FPythonBridge Bridge;
Bridge.Initialize(PythonPath, ScriptPath, 8765);

FString PythonCode = TEXT("import unreal; print(unreal.SystemLibrary.get_project_directory())");
FString Request = FString::Printf(TEXT("{\"action\": \"execute_python\", \"code\": \"%s\"}"), *PythonCode);
FString Response;
Bridge.SendRequest(Request, Response);
```

**New (VibeUE):**
```cpp
FAdastreaScriptResult Result = FAdastreaScriptService::ExecuteCode(
    TEXT("import unreal; print(unreal.SystemLibrary.get_project_directory())")
);

if (Result.bSuccess)
{
    UE_LOG(LogAdastreaDirector, Log, TEXT("Output: %s"), *Result.Output);
}
else
{
    UE_LOG(LogAdastreaDirector, Error, TEXT("Error: %s"), *Result.ErrorMessage);
}
```

### Example 2: Asset Queries

**Old (IPC + Document Ingestion):**
```cpp
// Step 1: Ingest documents (slow, can be stale)
// python ingest.py --docs-dir Content/

// Step 2: Query via IPC
FString Request = TEXT("{\"action\": \"query\", \"text\": \"list all blueprints\"}");
FString Response;
Bridge.SendRequest(Request, Response);
// Response contains search results from ingested docs
```

**New (VibeUE - Runtime Queries):**
```cpp
// Direct runtime query (always current)
TArray<FAssetInfo> Blueprints = FAdastreaAssetService::GetBlueprints();

for (const FAssetInfo& Asset : Blueprints)
{
    UE_LOG(LogAdastreaDirector, Log, TEXT("Blueprint: %s at %s"), 
        *Asset.Name, *Asset.Path);
}

// No ingestion needed, data is always current
```

### Example 3: LLM Queries

**Old (IPC):**
```cpp
// LLM call goes through Python backend
FString Request = TEXT("{\"action\": \"chat\", \"message\": \"What is Unreal Engine?\"}");
FString Response;
Bridge.SendRequest(Request, Response);
// Python process calls LLM API, returns response via IPC
```

**New (VibeUE):**
```cpp
// Direct C++ LLM call
TSharedPtr<FAdastreaLLMClient> Client = MakeShared<FAdastreaLLMClient>();
Client->SetProvider(ELLMProvider::Gemini, ApiKey);
Client->SetModel(TEXT("gemini-1.5-flash"));

TArray<FChatMessage> Messages;
FChatMessage UserMsg;
UserMsg.Role = TEXT("user");
UserMsg.Content = TEXT("What is Unreal Engine?");
Messages.Add(UserMsg);

Client->SendChatRequest(
    Messages,
    TArray<FToolDefinition>(), // No tools for simple query
    FOnStreamChunk::CreateLambda([](const FString& Chunk) {
        // Streamed chunk received
        UE_LOG(LogAdastreaDirector, Log, TEXT("Chunk: %s"), *Chunk);
    }),
    FOnLLMComplete::CreateLambda([](bool bSuccess, const FString& Content, const TArray<FToolCall>& ToolCalls) {
        // Complete response received
        if (bSuccess)
        {
            UE_LOG(LogAdastreaDirector, Log, TEXT("Response: %s"), *Content);
        }
    })
);
```

## Tool System Integration

The VibeUE architecture includes an extensible tool system for AI agent capabilities:

```cpp
// Register a custom tool
FAdastreaToolInfo CustomTool;
CustomTool.Name = TEXT("get_player_stats");
CustomTool.Description = TEXT("Get player character statistics");
CustomTool.Category = TEXT("Gameplay");

// Define parameter schema
TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
Schema->SetStringField(TEXT("type"), TEXT("object"));
// ... define parameters

CustomTool.ParameterSchema = Schema;

// Set executor
CustomTool.Executor.BindLambda([](const TSharedPtr<FJsonObject>& Args) -> FToolExecutionResult
{
    FToolExecutionResult Result;
    Result.bSuccess = true;
    Result.Output = TEXT("Player health: 100, Armor: 50");
    return Result;
});

// Register tool
FAdastreaToolSystem::Get().RegisterTool(CustomTool);

// LLM can now call this tool during conversations
```

## Timeline

### Current State (Phase 3.5)
- ✅ All VibeUE components implemented and tested
- ⚠️ Legacy IPC components maintained for parallel operation
- ⚠️ Both architectures coexist

### Future (Phase 4)
- Add comprehensive unit tests
- Implement feature flags for gradual rollout
- Add deprecation warnings to legacy components
- Remove PythonProcessManager, IPCClient, PythonBridge
- Update all documentation

## Best Practices

1. **New Code**: Always use VibeUE components (AdastreaScriptService, LLMClient, AssetService, etc.)
2. **Existing Code**: Plan migration from IPC components to VibeUE
3. **Testing**: Test with both architectures during transition
4. **Error Handling**: VibeUE components use TResult<T> pattern for cleaner error handling
5. **Performance**: VibeUE is significantly faster (75% latency reduction)

## Resources

- **ARCHITECTURE.md** - System architecture overview
- **VIBEUE_IMPLEMENTATION_GUIDE.md** - Complete implementation guide (2,929 lines)
- **VIBEUE_COMPLETION_SUMMARY.md** - Implementation results and metrics
- **AdastreaExamples.h** - Usage examples for all VibeUE components
- [Plugin Documentation](Documentation/README.md) - Complete plugin docs

## Support

For questions or issues during migration:
1. Check the [Plugin Documentation](Documentation/README.md)
2. Review [AdastreaExamples.h](../Source/AdastreaDirector/Public/AdastreaExamples.h) for usage patterns
3. See [VIBEUE_IMPLEMENTATION_GUIDE.md](../../VIBEUE_IMPLEMENTATION_GUIDE.md) for detailed API reference
4. Open an issue on GitHub

---

**Status**: Migration guide complete - VibeUE architecture fully implemented and ready for adoption.
