// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AdastreaScriptService.h"
#include "AdastreaLLMClient.h"
#include "AdastreaAssetService.h"
#include "AdastreaToolSystem.h"
#include "AdastreaMCPServer.h"

/**
 * Example Usage for VibeUE-style Adastrea Director features
 * 
 * This file demonstrates how to use the new C++ APIs for:
 * - Python script execution
 * - Direct LLM API calls (Gemini & OpenAI)
 * - Runtime asset discovery
 * - Tool system
 * - MCP server
 * 
 * See VIBEUE_IMPLEMENTATION_GUIDE.md for full documentation
 */
namespace AdastreaExamples
{
	/**
	 * Example 1: Execute Python code
	 */
	inline void ExamplePythonExecution()
	{
		// Simple expression evaluation
		FAdastreaScriptResult Result1 = FAdastreaScriptService::EvaluateExpression(TEXT("2 + 2"));
		if (Result1.bSuccess)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("2 + 2 = %s"), *Result1.Output);
		}

		// Execute Python code with Unreal module access
		FString Code = TEXT(R"(
import unreal
editor_util = unreal.EditorUtilityLibrary()
assets = editor_util.get_selected_assets()
print(f'Selected {len(assets)} assets')
for asset in assets:
    print(f'  - {asset.get_name()}')
		)");
		
		FAdastreaScriptResult Result2 = FAdastreaScriptService::ExecuteCode(Code);
		if (Result2.bSuccess)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Python output:\n%s"), *Result2.Output);
		}
		else
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Python error: %s"), *Result2.ErrorMessage);
		}

		// Check Python availability
		if (FAdastreaScriptService::IsPythonAvailable())
		{
			FString PythonInfo = FAdastreaScriptService::GetPythonInfo();
			UE_LOG(LogAdastreaDirector, Log, TEXT("Python Info: %s"), *PythonInfo);
		}
	}

	/**
	 * Example 2: Call LLM API directly
	 */
	inline void ExampleLLMCall()
	{
		// Create LLM client (must be TSharedPtr for async callbacks)
		TSharedPtr<FAdastreaLLMClient> Client = MakeShared<FAdastreaLLMClient>();
		
		// Configure for Gemini
		Client->SetProvider(ELLMProvider::Gemini, TEXT("YOUR_GEMINI_API_KEY"));
		Client->SetModel(TEXT("gemini-1.5-flash"));
		Client->SetTemperature(0.7f);

		// Or configure for OpenAI
		// Client->SetProvider(ELLMProvider::OpenAI, TEXT("YOUR_OPENAI_API_KEY"));
		// Client->SetModel(TEXT("gpt-4"));

		// Build conversation
		TArray<FChatMessage> Messages;
		
		FChatMessage SystemMsg;
		SystemMsg.Role = TEXT("system");
		SystemMsg.Content = TEXT("You are a helpful Unreal Engine assistant.");
		Messages.Add(SystemMsg);
		
		FChatMessage UserMsg;
		UserMsg.Role = TEXT("user");
		UserMsg.Content = TEXT("What is a Blueprint in Unreal Engine?");
		Messages.Add(UserMsg);

		// No tools for this example
		TArray<FToolDefinition> Tools;

		// Send request with streaming
		Client->SendChatRequest(
			Messages,
			Tools,
			FOnStreamChunk::CreateLambda([](const FString& Chunk) {
				UE_LOG(LogAdastreaDirector, Log, TEXT("Stream chunk: %s"), *Chunk);
			}),
			FOnLLMComplete::CreateLambda([](bool bSuccess, const FString& Content, const TArray<FToolCall>& ToolCalls) {
				if (bSuccess)
				{
					UE_LOG(LogAdastreaDirector, Log, TEXT("LLM Response: %s"), *Content);
					UE_LOG(LogAdastreaDirector, Log, TEXT("Tool calls: %d"), ToolCalls.Num());
				}
				else
				{
					UE_LOG(LogAdastreaDirector, Error, TEXT("LLM failed: %s"), *Content);
				}
			})
		);
	}

	/**
	 * Example 3: Discover assets at runtime
	 */
	inline void ExampleAssetDiscovery()
	{
		// Check if asset registry is ready
		if (!FAdastreaAssetService::IsAssetRegistryReady())
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("Asset registry is still loading..."));
			return;
		}

		// Search for assets by pattern
		TArray<FAssetInfo> Results = FAdastreaAssetService::SearchAssets(TEXT("Character"), TEXT(""), 10);
		UE_LOG(LogAdastreaDirector, Log, TEXT("Found %d assets matching 'Character'"), Results.Num());
		for (const FAssetInfo& Asset : Results)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("  - %s (%s) at %s"), 
				*Asset.Name, *Asset.Class, *Asset.Path);
		}

		// Get all Blueprints
		TArray<FAssetInfo> Blueprints = FAdastreaAssetService::GetBlueprints();
		UE_LOG(LogAdastreaDirector, Log, TEXT("Total blueprints in project: %d"), Blueprints.Num());

		// Get all Materials
		TArray<FAssetInfo> Materials = FAdastreaAssetService::GetMaterials(TEXT("/Game/Materials"));
		UE_LOG(LogAdastreaDirector, Log, TEXT("Materials in /Game/Materials: %d"), Materials.Num());

		// Get all UMG Widgets
		TArray<FAssetInfo> Widgets = FAdastreaAssetService::GetWidgets();
		UE_LOG(LogAdastreaDirector, Log, TEXT("Total widgets in project: %d"), Widgets.Num());

		// Get specific asset by path
		TOptional<FAssetInfo> Asset = FAdastreaAssetService::GetAssetByPath(TEXT("/Game/MyBlueprint"));
		if (Asset.IsSet())
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Found asset: %s"), *Asset->Name);
		}
	}

	/**
	 * Example 4: Use the tool system
	 */
	inline void ExampleToolSystem()
	{
		// Check if tool exists
		if (FAdastreaToolSystem::Get().HasTool(TEXT("search_assets")))
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("search_assets tool is available"));
		}

		// Execute a tool
		TSharedPtr<FJsonObject> Args = MakeShared<FJsonObject>();
		Args->SetStringField(TEXT("pattern"), TEXT("Material"));
		Args->SetStringField(TEXT("class"), TEXT("Material"));

		FToolExecutionResult Result = FAdastreaToolSystem::Get().ExecuteTool(TEXT("search_assets"), Args);
		
		if (Result.bSuccess)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Tool executed: %s"), *Result.Output);
			
			// Access structured data
			if (Result.Data.IsValid())
			{
				int32 Count = 0;
				Result.Data->TryGetNumberField(TEXT("count"), Count);
				UE_LOG(LogAdastreaDirector, Log, TEXT("Found %d assets"), Count);
			}
		}
		else
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Tool failed: %s"), *Result.ErrorMessage);
		}

		// Get all available tools
		TArray<FToolDefinition> AllTools = FAdastreaToolSystem::Get().GetAllToolDefinitions();
		UE_LOG(LogAdastreaDirector, Log, TEXT("Available tools: %d"), AllTools.Num());
		for (const FToolDefinition& Tool : AllTools)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("  - %s: %s"), *Tool.Name, *Tool.Description);
		}

		// Get tools by category
		TArray<FToolDefinition> AssetTools = FAdastreaToolSystem::Get().GetToolsByCategory(TEXT("Asset"));
		UE_LOG(LogAdastreaDirector, Log, TEXT("Asset tools: %d"), AssetTools.Num());
	}

	/**
	 * Example 5: LLM with tool calling
	 */
	inline void ExampleLLMWithTools()
	{
		TSharedPtr<FAdastreaLLMClient> Client = MakeShared<FAdastreaLLMClient>();
		Client->SetProvider(ELLMProvider::Gemini, TEXT("YOUR_API_KEY"));

		// Build conversation
		TArray<FChatMessage> Messages;
		FChatMessage UserMsg;
		UserMsg.Role = TEXT("user");
		UserMsg.Content = TEXT("List all blueprints in the project");
		Messages.Add(UserMsg);

		// Get available tools
		TArray<FToolDefinition> Tools = FAdastreaToolSystem::Get().GetAllToolDefinitions();

		// Send request
		// Note: Capture Client by value to prevent dangling reference in async callback
		Client->SendChatRequest(
			Messages,
			Tools,
			FOnStreamChunk(),
			FOnLLMComplete::CreateLambda([Messages, Tools, Client](bool bSuccess, const FString& Content, const TArray<FToolCall>& ToolCalls) mutable {
				if (!bSuccess)
				{
					UE_LOG(LogAdastreaDirector, Error, TEXT("LLM failed: %s"), *Content);
					return;
				}

				// Add assistant response
				FChatMessage AssistantMsg;
				AssistantMsg.Role = TEXT("assistant");
				AssistantMsg.Content = Content;
				Messages.Add(AssistantMsg);

				// Execute tool calls
				if (ToolCalls.Num() > 0)
				{
					for (const FToolCall& ToolCall : ToolCalls)
					{
						UE_LOG(LogAdastreaDirector, Log, TEXT("LLM wants to call tool: %s"), *ToolCall.ToolName);

						// Execute the tool
						FToolExecutionResult Result = FAdastreaToolSystem::Get().ExecuteTool(
							ToolCall.ToolName,
							ToolCall.Arguments
						);

						// Add tool result to conversation
						FChatMessage ToolResultMsg;
						ToolResultMsg.Role = TEXT("tool");
						ToolResultMsg.ToolCallId = ToolCall.Id;
						
						// Serialize result to JSON string
						FString JsonString;
						TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
						FJsonSerializer::Serialize(Result.ToJson().ToSharedRef(), Writer);
						ToolResultMsg.Content = JsonString;
						
						Messages.Add(ToolResultMsg);

						UE_LOG(LogAdastreaDirector, Log, TEXT("Tool result: %s"), *Result.Output);
					}

					// Continue conversation with tool results
					// (In real code, you'd make another SendChatRequest here)
					UE_LOG(LogAdastreaDirector, Log, TEXT("Tool execution complete - would continue conversation"));
				}
				else
				{
					UE_LOG(LogAdastreaDirector, Log, TEXT("LLM Response: %s"), *Content);
				}
			})
		);
	}

	/**
	 * Example 6: Start MCP server
	 */
	inline void ExampleMCPServer()
	{
		// Create MCP server
		TSharedPtr<FAdastreaMCPServer> MCPServer = MakeShared<FAdastreaMCPServer>();

		// Start on port 8088
		if (MCPServer->Start(8088))
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("MCP Server started successfully"));
			UE_LOG(LogAdastreaDirector, Log, TEXT("External clients can now connect to:"));
			UE_LOG(LogAdastreaDirector, Log, TEXT("  http://localhost:8088/mcp/tools/list"));
			UE_LOG(LogAdastreaDirector, Log, TEXT("  http://localhost:8088/mcp/tools/call"));
			UE_LOG(LogAdastreaDirector, Log, TEXT("  http://localhost:8088/mcp/resources"));
		}
		else
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to start MCP Server"));
		}

		// Check if running
		if (MCPServer->IsRunning())
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("MCP Server is running on port %d"), MCPServer->GetPort());
		}

		// Stop when done
		// MCPServer->Stop();
	}

	/**
	 * Run all examples (for testing)
	 */
	inline void RunAllExamples()
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("=== Running Adastrea Examples ==="));
		
		UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 1: Python Execution ---"));
		ExamplePythonExecution();
		
		UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 3: Asset Discovery ---"));
		ExampleAssetDiscovery();
		
		UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 4: Tool System ---"));
		ExampleToolSystem();
		
		// Note: LLM examples require API keys, so they're commented out
		// UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 2: LLM Call ---"));
		// ExampleLLMCall();
		
		// UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 5: LLM with Tools ---"));
		// ExampleLLMWithTools();
		
		// UE_LOG(LogAdastreaDirector, Log, TEXT("\n--- Example 6: MCP Server ---"));
		// ExampleMCPServer();
		
		UE_LOG(LogAdastreaDirector, Log, TEXT("\n=== Examples Complete ==="));
	}
}
