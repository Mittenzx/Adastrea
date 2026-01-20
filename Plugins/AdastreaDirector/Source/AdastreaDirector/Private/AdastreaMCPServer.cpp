// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "AdastreaMCPServer.h"
#include "AdastreaDirectorModule.h"
#include "AdastreaToolSystem.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "HttpServerRequest.h"

#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FAdastreaMCPServer::FAdastreaMCPServer()
{
}

FAdastreaMCPServer::~FAdastreaMCPServer()
{
	Stop();
}

bool FAdastreaMCPServer::Start(int32 Port)
{
	if (bIsRunning)
	{
		UE_LOG(LogAdastreaDirector, Warning, TEXT("MCP Server already running on port %d"), ServerPort);
		return true;
	}

	// Get HTTP server module
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	
	// Create router
	HttpRouter = HttpServerModule.GetHttpRouter(Port);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to create HTTP router for MCP server"));
		return false;
	}

	// Register routes
	FHttpRouteHandle ListToolsHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/tools/list")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {
			return HandleListTools(Request, OnComplete);
		})
	);
	RouteHandles.Add(ListToolsHandle);

	FHttpRouteHandle ExecuteToolHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/tools/call")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {
			return HandleExecuteTool(Request, OnComplete);
		})
	);
	RouteHandles.Add(ExecuteToolHandle);

	FHttpRouteHandle GetResourcesHandle = HttpRouter->BindRoute(
		FHttpPath(TEXT("/mcp/resources")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {
			return HandleGetResources(Request, OnComplete);
		})
	);
	RouteHandles.Add(GetResourcesHandle);

	// Start HTTP server
	HttpServerModule.StartAllListeners();

	bIsRunning = true;
	ServerPort = Port;

	UE_LOG(LogAdastreaDirector, Log, TEXT("MCP Server started on port %d"), Port);
	UE_LOG(LogAdastreaDirector, Log, TEXT("  POST http://localhost:%d/mcp/tools/list"), Port);
	UE_LOG(LogAdastreaDirector, Log, TEXT("  POST http://localhost:%d/mcp/tools/call"), Port);
	UE_LOG(LogAdastreaDirector, Log, TEXT("  POST http://localhost:%d/mcp/resources"), Port);

	return true;
}

void FAdastreaMCPServer::Stop()
{
	if (!bIsRunning)
	{
		return;
	}

	// Unbind routes
	if (HttpRouter.IsValid())
	{
		for (const FHttpRouteHandle& Handle : RouteHandles)
		{
			HttpRouter->UnbindRoute(Handle);
		}
		RouteHandles.Empty();
	}

	// Stop HTTP server
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpServerModule.StopAllListeners();

	bIsRunning = false;
	ServerPort = 0;

	UE_LOG(LogAdastreaDirector, Log, TEXT("MCP Server stopped"));
}

bool FAdastreaMCPServer::HandleListTools(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("MCP: HandleListTools"));

	// Get all tool definitions
	TArray<FToolDefinition> Tools = FAdastreaToolSystem::Get().GetAllToolDefinitions();

	// Build MCP response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetNumberField(TEXT("id"), 1);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	for (const FToolDefinition& Tool : Tools)
	{
		TSharedPtr<FJsonObject> ToolObj = MakeShared<FJsonObject>();
		ToolObj->SetStringField(TEXT("name"), Tool.Name);
		ToolObj->SetStringField(TEXT("description"), Tool.Description);
		
		if (Tool.Parameters.IsValid())
		{
			ToolObj->SetObjectField(TEXT("inputSchema"), Tool.Parameters);
		}

		ToolsArray.Add(MakeShared<FJsonValueObject>(ToolObj));
	}

	Result->SetArrayField(TEXT("tools"), ToolsArray);
	Response->SetObjectField(TEXT("result"), Result);

	// Send response
	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FAdastreaMCPServer::HandleExecuteTool(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("MCP: HandleExecuteTool"));

	// Parse request body
	TSharedPtr<FJsonObject> RequestBody = ParseRequestBody(Request);
	if (!RequestBody.IsValid())
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON in request body"), 400));
		return true;
	}

	// Extract params - UE 5.6+ TryGetObjectField signature
	const TSharedPtr<FJsonObject>* ParamsPtr;
	if (!RequestBody->TryGetObjectField(TEXT("params"), ParamsPtr) || !ParamsPtr || !(*ParamsPtr).IsValid())
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'params' field"), 400));
		return true;
	}
	
	const TSharedPtr<FJsonObject>& Params = *ParamsPtr;

	// Get tool name
	FString ToolName;
	if (!Params->TryGetStringField(TEXT("name"), ToolName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'name' field in params"), 400));
		return true;
	}

	// Get arguments (default to empty object if not provided)
	TSharedPtr<FJsonObject> Arguments;
	const TSharedPtr<FJsonObject>* ArgumentsPtr;
	if (!Params->TryGetObjectField(TEXT("arguments"), ArgumentsPtr) || !ArgumentsPtr || !(*ArgumentsPtr).IsValid())
	{
		// Empty args if missing or null
		Arguments = MakeShared<FJsonObject>();
	}
	else
	{
		Arguments = *ArgumentsPtr;
	}

	// Execute tool
	FToolExecutionResult Result = FAdastreaToolSystem::Get().ExecuteTool(ToolName, Arguments);

	// Build MCP response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	
	// Get request ID
	int32 RequestId = 0;
	RequestBody->TryGetNumberField(TEXT("id"), RequestId);
	Response->SetNumberField(TEXT("id"), RequestId);

	if (Result.bSuccess)
	{
		TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
		
		// MCP format expects content array
		TArray<TSharedPtr<FJsonValue>> ContentArray;
		TSharedPtr<FJsonObject> ContentItem = MakeShared<FJsonObject>();
		ContentItem->SetStringField(TEXT("type"), TEXT("text"));
		ContentItem->SetStringField(TEXT("text"), Result.Output);
		ContentArray.Add(MakeShared<FJsonValueObject>(ContentItem));
		
		ResultObj->SetArrayField(TEXT("content"), ContentArray);
		
		// Include data if present
		if (Result.Data.IsValid())
		{
			// Serialize data to string for MCP
			FString DataStr;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DataStr);
			FJsonSerializer::Serialize(Result.Data.ToSharedRef(), Writer);
			
			TSharedPtr<FJsonObject> DataContent = MakeShared<FJsonObject>();
			DataContent->SetStringField(TEXT("type"), TEXT("text"));
			DataContent->SetStringField(TEXT("text"), DataStr);
			ContentArray.Add(MakeShared<FJsonValueObject>(DataContent));
			
			ResultObj->SetArrayField(TEXT("content"), ContentArray);
		}
		
		Response->SetObjectField(TEXT("result"), ResultObj);
	}
	else
	{
		// Error response
		TSharedPtr<FJsonObject> Error = MakeShared<FJsonObject>();
		Error->SetNumberField(TEXT("code"), -32000); // MCP error code
		Error->SetStringField(TEXT("message"), Result.ErrorMessage);
		Response->SetObjectField(TEXT("error"), Error);
	}

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FAdastreaMCPServer::HandleGetResources(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	UE_LOG(LogAdastreaDirector, Log, TEXT("MCP: HandleGetResources"));

	// Build MCP response for resources
	// This can list available assets, blueprints, etc.
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetNumberField(TEXT("id"), 1);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ResourcesArray;

	// Add a resource entry for project assets
	TSharedPtr<FJsonObject> AssetsResource = MakeShared<FJsonObject>();
	AssetsResource->SetStringField(TEXT("uri"), TEXT("adastrea://project/assets"));
	AssetsResource->SetStringField(TEXT("name"), TEXT("Project Assets"));
	AssetsResource->SetStringField(TEXT("description"), TEXT("All assets in the Unreal Engine project"));
	AssetsResource->SetStringField(TEXT("mimeType"), TEXT("application/json"));
	ResourcesArray.Add(MakeShared<FJsonValueObject>(AssetsResource));

	Result->SetArrayField(TEXT("resources"), ResourcesArray);
	Response->SetObjectField(TEXT("result"), Result);

	OnComplete(CreateJsonResponse(Response));
	return true;
}

TSharedPtr<FJsonObject> FAdastreaMCPServer::ParseRequestBody(const FHttpServerRequest& Request)
{
	const TArray<uint8>& BodyData = Request.Body;
	FString BodyString;
	
	// Convert bytes to string
	BodyString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(BodyData.GetData())));

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyString);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return JsonObject;
	}

	return nullptr;
}

TUniquePtr<FHttpServerResponse> FAdastreaMCPServer::CreateJsonResponse(const TSharedPtr<FJsonObject>& JsonObject, int32 StatusCode)
{
	// Serialize JSON to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Create response
	TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(JsonString, TEXT("application/json"));
	Response->Code = static_cast<EHttpServerResponseCodes>(StatusCode);
	
	return Response;
}

TUniquePtr<FHttpServerResponse> FAdastreaMCPServer::CreateErrorResponse(const FString& Error, int32 StatusCode)
{
	TSharedPtr<FJsonObject> ErrorObj = MakeShared<FJsonObject>();
	ErrorObj->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	ErrorObj->SetNumberField(TEXT("id"), 0);
	
	TSharedPtr<FJsonObject> ErrorDetail = MakeShared<FJsonObject>();
	ErrorDetail->SetNumberField(TEXT("code"), -32000);
	ErrorDetail->SetStringField(TEXT("message"), Error);
	
	ErrorObj->SetObjectField(TEXT("error"), ErrorDetail);

	return CreateJsonResponse(ErrorObj, StatusCode);
}
