// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaLLMClient.h"
#include "AdastreaDirectorModule.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FAdastreaLLMClient::FAdastreaLLMClient()
	: Provider(ELLMProvider::Gemini)
	, ModelName(TEXT("gemini-1.5-flash"))
	, Temperature(0.7f)
{
}

FAdastreaLLMClient::~FAdastreaLLMClient()
{
	CancelRequest();
}

void FAdastreaLLMClient::SetProvider(ELLMProvider InProvider, const FString& InApiKey)
{
	Provider = InProvider;
	ApiKey = InApiKey;
}

void FAdastreaLLMClient::SetModel(const FString& InModelName)
{
	ModelName = InModelName;
}

void FAdastreaLLMClient::SetTemperature(float InTemperature)
{
	Temperature = FMath::Clamp(InTemperature, 0.0f, 1.0f);
}

void FAdastreaLLMClient::SendChatRequest(
	const TArray<FChatMessage>& Messages,
	const TArray<FToolDefinition>& Tools,
	FOnStreamChunk OnStreamChunk,
	FOnLLMComplete OnComplete)
{
	// Cancel any existing request
	CancelRequest();

	// Route to provider-specific implementation
	switch (Provider)
	{
		case ELLMProvider::Gemini:
			SendGeminiRequest(Messages, Tools, OnStreamChunk, OnComplete);
			break;
		case ELLMProvider::OpenAI:
			SendOpenAIRequest(Messages, Tools, OnStreamChunk, OnComplete);
			break;
	}
}

void FAdastreaLLMClient::SendGeminiRequest(
	const TArray<FChatMessage>& Messages,
	const TArray<FToolDefinition>& Tools,
	FOnStreamChunk OnStreamChunk,
	FOnLLMComplete OnComplete)
{
	// Create HTTP request
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	// Gemini API endpoint (streaming)
	FString Endpoint = FString::Printf(
		TEXT("https://generativelanguage.googleapis.com/v1beta/models/%s:streamGenerateContent?key=%s"),
		*ModelName,
		*ApiKey
	);
	
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Build JSON payload
	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	
	// Convert messages to Gemini format
	TArray<TSharedPtr<FJsonValue>> ContentsArray;
	for (const FChatMessage& Message : Messages)
	{
		TSharedPtr<FJsonObject> ContentObj = MakeShared<FJsonObject>();
		
		// Gemini uses "user" and "model" roles
		FString Role = Message.Role == TEXT("assistant") ? TEXT("model") : TEXT("user");
		ContentObj->SetStringField(TEXT("role"), Role);
		
		// Parts array with text
		TArray<TSharedPtr<FJsonValue>> PartsArray;
		TSharedPtr<FJsonObject> Part = MakeShared<FJsonObject>();
		Part->SetStringField(TEXT("text"), Message.Content);
		PartsArray.Add(MakeShared<FJsonValueObject>(Part));
		
		ContentObj->SetArrayField(TEXT("parts"), PartsArray);
		ContentsArray.Add(MakeShared<FJsonValueObject>(ContentObj));
	}
	Payload->SetArrayField(TEXT("contents"), ContentsArray);

	// Generation config
	TSharedPtr<FJsonObject> GenerationConfig = MakeShared<FJsonObject>();
	GenerationConfig->SetNumberField(TEXT("temperature"), Temperature);
	Payload->SetObjectField(TEXT("generationConfig"), GenerationConfig);

	// Tools (if any)
	if (Tools.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ToolsArray;
		TSharedPtr<FJsonObject> ToolsWrapper = MakeShared<FJsonObject>();
		
		TArray<TSharedPtr<FJsonValue>> FunctionDeclarations;
		for (const FToolDefinition& Tool : Tools)
		{
			FunctionDeclarations.Add(MakeShared<FJsonValueObject>(Tool.ToJson()));
		}
		
		ToolsWrapper->SetArrayField(TEXT("functionDeclarations"), FunctionDeclarations);
		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolsWrapper));
		Payload->SetArrayField(TEXT("tools"), ToolsArray);
	}

	// Serialize to JSON string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);
	
	Request->SetContentAsString(JsonString);

	// Setup callbacks with weak pointer to prevent dangling pointer if object is destroyed
	TWeakPtr<FAdastreaLLMClient> WeakSelf = AsShared();
	
	if (OnStreamChunk.IsBound())
	{
		// Streaming mode - UE 5.6+ uses OnRequestProgress64 with uint64 parameters
		Request->OnRequestProgress64().BindLambda(
			[WeakSelf, OnStreamChunk](FHttpRequestPtr Req, uint64 BytesSent, uint64 BytesReceived)
			{
				TSharedPtr<FAdastreaLLMClient> Pinned = WeakSelf.Pin();
				if (!Pinned.IsValid())
				{
					return;
				}
				
				Pinned->OnStreamDataReceived(Req, BytesSent, BytesReceived, OnStreamChunk);
			}
		);
	}

	Request->OnProcessRequestComplete().BindLambda(
		[WeakSelf, OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			TSharedPtr<FAdastreaLLMClient> Pinned = WeakSelf.Pin();
			if (!Pinned.IsValid())
			{
				return;
			}
			
			Pinned->OnResponseReceived(Req, Response, bWasSuccessful, OnComplete);
		}
	);

	// Send request
	CurrentRequest = Request;
	Request->ProcessRequest();

	UE_LOG(LogAdastreaDirector, Log, TEXT("Sent Gemini API request"));
}

void FAdastreaLLMClient::SendOpenAIRequest(
	const TArray<FChatMessage>& Messages,
	const TArray<FToolDefinition>& Tools,
	FOnStreamChunk OnStreamChunk,
	FOnLLMComplete OnComplete)
{
	// Create HTTP request
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	// OpenAI API endpoint
	Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));

	// Build JSON payload
	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("model"), ModelName);
	
	// Convert messages to OpenAI format
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	for (const FChatMessage& Message : Messages)
	{
		MessagesArray.Add(MakeShared<FJsonValueObject>(Message.ToJson()));
	}
	Payload->SetArrayField(TEXT("messages"), MessagesArray);

	// Generation config
	Payload->SetNumberField(TEXT("temperature"), Temperature);
	Payload->SetBoolField(TEXT("stream"), OnStreamChunk.IsBound());

	// Tools (if any)
	if (Tools.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ToolsArray;
		for (const FToolDefinition& Tool : Tools)
		{
			TSharedPtr<FJsonObject> ToolObj = MakeShared<FJsonObject>();
			ToolObj->SetStringField(TEXT("type"), TEXT("function"));
			
			TSharedPtr<FJsonObject> FunctionObj = MakeShared<FJsonObject>();
			FunctionObj->SetStringField(TEXT("name"), Tool.Name);
			FunctionObj->SetStringField(TEXT("description"), Tool.Description);
			FunctionObj->SetObjectField(TEXT("parameters"), Tool.Parameters);
			
			ToolObj->SetObjectField(TEXT("function"), FunctionObj);
			ToolsArray.Add(MakeShared<FJsonValueObject>(ToolObj));
		}
		Payload->SetArrayField(TEXT("tools"), ToolsArray);
		Payload->SetStringField(TEXT("tool_choice"), TEXT("auto"));
	}

	// Serialize to JSON string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);
	
	Request->SetContentAsString(JsonString);

	// Setup callbacks with weak pointer
	TWeakPtr<FAdastreaLLMClient> WeakSelf = AsShared();
	
	if (OnStreamChunk.IsBound())
	{
		// Streaming mode - UE 5.6+ uses OnRequestProgress64 with uint64 parameters
		Request->OnRequestProgress64().BindLambda(
			[WeakSelf, OnStreamChunk](FHttpRequestPtr Req, uint64 BytesSent, uint64 BytesReceived)
			{
				TSharedPtr<FAdastreaLLMClient> Pinned = WeakSelf.Pin();
				if (!Pinned.IsValid())
				{
					return;
				}
				
				Pinned->OnStreamDataReceived(Req, BytesSent, BytesReceived, OnStreamChunk);
			}
		);
	}

	Request->OnProcessRequestComplete().BindLambda(
		[WeakSelf, OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			TSharedPtr<FAdastreaLLMClient> Pinned = WeakSelf.Pin();
			if (!Pinned.IsValid())
			{
				return;
			}
			
			Pinned->OnResponseReceived(Req, Response, bWasSuccessful, OnComplete);
		}
	);

	// Send request
	CurrentRequest = Request;
	Request->ProcessRequest();

	UE_LOG(LogAdastreaDirector, Log, TEXT("Sent OpenAI API request"));
}

void FAdastreaLLMClient::OnResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bWasSuccessful,
	FOnLLMComplete OnComplete)
{
	CurrentRequest.Reset();

	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("LLM request failed"));
		OnComplete.ExecuteIfBound(false, TEXT("Request failed"), TArray<FToolCall>());
		return;
	}

	int32 StatusCode = Response->GetResponseCode();
	FString ResponseBody = Response->GetContentAsString();

	UE_LOG(LogAdastreaDirector, Log, TEXT("LLM response: %d, Body length: %d"), 
		StatusCode, ResponseBody.Len());

	if (StatusCode != 200)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("LLM API error: %s"), *ResponseBody);
		OnComplete.ExecuteIfBound(false, FString::Printf(TEXT("API error: %d"), StatusCode), 
			TArray<FToolCall>());
		return;
	}

	// Parse JSON response
	TSharedPtr<FJsonObject> JsonResponse;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to parse JSON response"));
		OnComplete.ExecuteIfBound(false, TEXT("Invalid JSON response"), TArray<FToolCall>());
		return;
	}

	// Extract content and tool calls
	FString Content;
	TArray<FToolCall> ToolCalls;

	// Try Gemini format first: candidates[0].content.parts[]
	const TArray<TSharedPtr<FJsonValue>>* Candidates;
	if (JsonResponse->TryGetArrayField(TEXT("candidates"), Candidates) && Candidates->Num() > 0)
	{
		// Gemini API response format
		TSharedPtr<FJsonObject> Candidate = (*Candidates)[0]->AsObject();
		if (!Candidate.IsValid())
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("Invalid candidate object in response"));
			OnComplete.ExecuteIfBound(false, TEXT("Invalid candidate format"), TArray<FToolCall>());
			return;
		}
		
		// UE 5.6+ TryGetObjectField signature changed to use const TSharedPtr<FJsonObject>*
		const TSharedPtr<FJsonObject>* ContentObjPtr;
		if (!Candidate->TryGetObjectField(TEXT("content"), ContentObjPtr) || !ContentObjPtr || !(*ContentObjPtr).IsValid())
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("No content field in candidate"));
			OnComplete.ExecuteIfBound(false, TEXT("No content in response"), TArray<FToolCall>());
			return;
		}
		
		const TSharedPtr<FJsonObject>& ContentObj = *ContentObjPtr;
		
		const TArray<TSharedPtr<FJsonValue>>* Parts;
		if (ContentObj->TryGetArrayField(TEXT("parts"), Parts))
		{
			for (const TSharedPtr<FJsonValue>& PartValue : *Parts)
			{
				TSharedPtr<FJsonObject> Part = PartValue->AsObject();
				if (!Part.IsValid())
				{
					continue;
				}
				
				// Text part
				FString Text;
				if (Part->TryGetStringField(TEXT("text"), Text))
				{
					Content += Text;
				}
				
				// Function call part
				const TSharedPtr<FJsonObject>* FunctionCallPtr;
				if (Part->TryGetObjectField(TEXT("functionCall"), FunctionCallPtr) && FunctionCallPtr && (*FunctionCallPtr).IsValid())
				{
					const TSharedPtr<FJsonObject>& FunctionCall = *FunctionCallPtr;
					FToolCall ToolCall;
					ToolCall.Id = FGuid::NewGuid().ToString();
					FunctionCall->TryGetStringField(TEXT("name"), ToolCall.ToolName);
					
					// Safely get args object
					const TSharedPtr<FJsonObject>* ArgsObjectPtr;
					if (FunctionCall->TryGetObjectField(TEXT("args"), ArgsObjectPtr) && ArgsObjectPtr && (*ArgsObjectPtr).IsValid())
					{
						ToolCall.Arguments = *ArgsObjectPtr;
					}
					else
					{
						ToolCall.Arguments = nullptr;
					}
					
					ToolCalls.Add(ToolCall);
				}
			}
		}
	}
	// Try OpenAI format: choices[0].message
	else
	{
		const TArray<TSharedPtr<FJsonValue>>* Choices;
		if (JsonResponse->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
		{
			// OpenAI API response format
			TSharedPtr<FJsonObject> Choice = (*Choices)[0]->AsObject();
			if (!Choice.IsValid())
			{
				UE_LOG(LogAdastreaDirector, Warning, TEXT("Invalid choice object in OpenAI response"));
				OnComplete.ExecuteIfBound(false, TEXT("Invalid choice format"), TArray<FToolCall>());
				return;
			}
			
			const TSharedPtr<FJsonObject>* MessagePtr;
			if (Choice->TryGetObjectField(TEXT("message"), MessagePtr) && MessagePtr && (*MessagePtr).IsValid())
			{
				const TSharedPtr<FJsonObject>& Message = *MessagePtr;
				// Get content
				FString MessageContent;
				if (Message->TryGetStringField(TEXT("content"), MessageContent))
				{
					Content = MessageContent;
				}
				
				// Get tool calls
				const TArray<TSharedPtr<FJsonValue>>* ToolCallsArray;
				if (Message->TryGetArrayField(TEXT("tool_calls"), ToolCallsArray))
				{
					for (const TSharedPtr<FJsonValue>& ToolCallValue : *ToolCallsArray)
					{
						TSharedPtr<FJsonObject> ToolCallObj = ToolCallValue->AsObject();
						if (!ToolCallObj.IsValid())
						{
							continue;
						}
						
						FToolCall ToolCall;
						ToolCallObj->TryGetStringField(TEXT("id"), ToolCall.Id);
						
						const TSharedPtr<FJsonObject>* FunctionObjPtr;
						if (ToolCallObj->TryGetObjectField(TEXT("function"), FunctionObjPtr) && FunctionObjPtr && (*FunctionObjPtr).IsValid())
						{
							const TSharedPtr<FJsonObject>& FunctionObj = *FunctionObjPtr;
							FunctionObj->TryGetStringField(TEXT("name"), ToolCall.ToolName);
							
							// Parse arguments from JSON string
							FString ArgsString;
							if (FunctionObj->TryGetStringField(TEXT("arguments"), ArgsString))
							{
								TSharedRef<TJsonReader<>> ArgsReader = TJsonReaderFactory<>::Create(ArgsString);
								FJsonSerializer::Deserialize(ArgsReader, ToolCall.Arguments);
							}
						}
						
						ToolCalls.Add(ToolCall);
					}
				}
			}
			else
			{
				UE_LOG(LogAdastreaDirector, Warning, TEXT("No message field in choice"));
				OnComplete.ExecuteIfBound(false, TEXT("No message in response"), TArray<FToolCall>());
				return;
			}
		}
		else
		{
			UE_LOG(LogAdastreaDirector, Warning, TEXT("Response has neither 'candidates' (Gemini) nor 'choices' (OpenAI) field"));
			OnComplete.ExecuteIfBound(false, TEXT("Unknown response format"), TArray<FToolCall>());
			return;
		}
	}

	UE_LOG(LogAdastreaDirector, Log, TEXT("Extracted content: %s, Tool calls: %d"), 
		*Content, ToolCalls.Num());

	OnComplete.ExecuteIfBound(true, Content, ToolCalls);
}

void FAdastreaLLMClient::OnStreamDataReceived(
	FHttpRequestPtr Request,
	uint64 BytesSent,
	uint64 BytesReceived,
	FOnStreamChunk OnStreamChunk)
{
	// Ensure the request and response are valid before accessing content
	if (!Request.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("OnStreamDataReceived called with invalid Request"));
		return;
	}

	FHttpResponsePtr Response = Request->GetResponse();
	if (!Response.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("OnStreamDataReceived: Request has no valid response yet"));
		return;
	}

	// Get current response content
	FString ResponseSoFar = Response->GetContentAsString();
	
	// Process only new data since last call (incremental parsing)
	if (ResponseSoFar.Len() > StreamBuffer.Len())
	{
		// Extract only the new portion to avoid reprocessing
		FString NewData = ResponseSoFar.Mid(StreamBuffer.Len());
		
		// Update buffer to current position
		StreamBuffer = ResponseSoFar;
		
		// Parse only the new SSE chunks
		ParseSSEChunk(NewData, OnStreamChunk);
	}
}

void FAdastreaLLMClient::ParseSSEChunk(const FString& Chunk, FOnStreamChunk OnStreamChunk)
{
	// SSE format: data: {...}\n\n
	// Parse JSON from each data: line
	
	TArray<FString> Lines;
	Chunk.ParseIntoArray(Lines, TEXT("\n"), true);
	
	for (const FString& Line : Lines)
	{
		if (Line.StartsWith(TEXT("data: ")))
		{
			FString JsonStr = Line.Mid(6).TrimStartAndEnd();
			
			// Skip [DONE] marker
			if (JsonStr == TEXT("[DONE]"))
			{
				continue;
			}
			
			// Parse JSON
			TSharedPtr<FJsonObject> JsonObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
			
			if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
			{
				// Extract text content from chunk
				// (Format varies by provider - this is simplified)
				FString Text;
				if (JsonObj->TryGetStringField(TEXT("text"), Text))
				{
					OnStreamChunk.ExecuteIfBound(Text);
				}
			}
		}
	}
}

void FAdastreaLLMClient::CancelRequest()
{
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
	StreamBuffer.Empty();
}

// Tool definition serialization
TSharedPtr<FJsonObject> FToolDefinition::ToJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("name"), Name);
	Json->SetStringField(TEXT("description"), Description);
	Json->SetObjectField(TEXT("parameters"), Parameters);
	return Json;
}

// Message serialization helpers
TSharedPtr<FJsonObject> FChatMessage::ToJson() const
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("role"), Role);
	Json->SetStringField(TEXT("content"), Content);
	
	if (!ToolCallId.IsEmpty())
	{
		Json->SetStringField(TEXT("tool_call_id"), ToolCallId);
	}
	
	return Json;
}

FChatMessage FChatMessage::FromJson(const TSharedPtr<FJsonObject>& Json)
{
	FChatMessage Message;
	Json->TryGetStringField(TEXT("role"), Message.Role);
	Json->TryGetStringField(TEXT("content"), Message.Content);
	Json->TryGetStringField(TEXT("tool_call_id"), Message.ToolCallId);
	return Message;
}

FToolCall FToolCall::FromJson(const TSharedPtr<FJsonObject>& Json)
{
	FToolCall ToolCall;
	Json->TryGetStringField(TEXT("id"), ToolCall.Id);
	
	const TSharedPtr<FJsonObject>* FunctionObjPtr;
	if (Json->TryGetObjectField(TEXT("function"), FunctionObjPtr) && FunctionObjPtr && (*FunctionObjPtr).IsValid())
	{
		const TSharedPtr<FJsonObject>& FunctionObj = *FunctionObjPtr;
		FunctionObj->TryGetStringField(TEXT("name"), ToolCall.ToolName);
		
		FString ArgsString;
		if (FunctionObj->TryGetStringField(TEXT("arguments"), ArgsString))
		{
			// Parse arguments JSON string
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ArgsString);
			FJsonSerializer::Deserialize(Reader, ToolCall.Arguments);
		}
	}
	
	return ToolCall;
}
