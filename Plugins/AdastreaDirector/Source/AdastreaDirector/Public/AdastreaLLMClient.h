// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"

/**
 * Chat message structure
 */
struct FChatMessage
{
	FString Role;      // "system", "user", "assistant", "tool"
	FString Content;
	FString ToolCallId; // For tool responses
	
	TSharedPtr<FJsonObject> ToJson() const;
	static FChatMessage FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Tool call from LLM
 */
struct FToolCall
{
	FString Id;
	FString ToolName;
	TSharedPtr<FJsonObject> Arguments;
	
	static FToolCall FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Tool definition
 */
struct FToolDefinition
{
	FString Name;
	FString Description;
	TSharedPtr<FJsonObject> Parameters;
	
	TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * Streaming callback for incremental responses
 */
DECLARE_DELEGATE_OneParam(FOnStreamChunk, const FString& /* Chunk */);

/**
 * Completion callback
 */
DECLARE_DELEGATE_ThreeParams(FOnLLMComplete, bool /* bSuccess */, const FString& /* Content */, const TArray<FToolCall>& /* ToolCalls */);

/**
 * LLM Provider types
 */
enum class ELLMProvider : uint8
{
	Gemini,
	OpenAI
};

/**
 * Direct C++ client for LLM APIs (Gemini, OpenAI)
 * 
 * Note: This class should inherit from TSharedFromThis<FAdastreaLLMClient>
 * to safely use weak pointers in async callbacks.
 */
class ADASTREADIRECTOR_API FAdastreaLLMClient : public TSharedFromThis<FAdastreaLLMClient>
{
public:
	FAdastreaLLMClient();
	~FAdastreaLLMClient();

	/**
	 * Send a chat completion request
	 * @param Messages Conversation history
	 * @param Tools Available tools
	 * @param OnStreamChunk Called for each streamed chunk (optional)
	 * @param OnComplete Called when request completes
	 */
	void SendChatRequest(
		const TArray<FChatMessage>& Messages,
		const TArray<FToolDefinition>& Tools,
		FOnStreamChunk OnStreamChunk,
		FOnLLMComplete OnComplete
	);

	/**
	 * Set API provider and key
	 */
	void SetProvider(ELLMProvider Provider, const FString& ApiKey);

	/**
	 * Set model name (e.g., "gemini-1.5-flash", "gpt-4")
	 */
	void SetModel(const FString& ModelName);

	/**
	 * Set temperature (0.0 - 1.0)
	 */
	void SetTemperature(float Temperature);

	/**
	 * Cancel any in-progress request
	 */
	void CancelRequest();

private:
	ELLMProvider Provider;
	FString ApiKey;
	FString ModelName;
	float Temperature;
	
	TSharedPtr<IHttpRequest> CurrentRequest;
	FString StreamBuffer;
	
	// Provider-specific implementations
	void SendGeminiRequest(
		const TArray<FChatMessage>& Messages,
		const TArray<FToolDefinition>& Tools,
		FOnStreamChunk OnStreamChunk,
		FOnLLMComplete OnComplete
	);
	
	void SendOpenAIRequest(
		const TArray<FChatMessage>& Messages,
		const TArray<FToolDefinition>& Tools,
		FOnStreamChunk OnStreamChunk,
		FOnLLMComplete OnComplete
	);
	
	// HTTP callbacks
	void OnResponseReceived(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful,
		FOnLLMComplete OnComplete
	);
	
	void OnStreamDataReceived(
		FHttpRequestPtr Request,
		uint64 BytesSent,
		uint64 BytesReceived,
		FOnStreamChunk OnStreamChunk
	);
	
	// Parsing helpers
	void ParseSSEChunk(const FString& Chunk, FOnStreamChunk OnStreamChunk);
	TArray<FToolCall> ExtractToolCalls(const TSharedPtr<FJsonObject>& Response);
};
