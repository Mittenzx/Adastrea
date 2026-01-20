// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpRouteHandle.h"
#include "Dom/JsonObject.h"

/**
 * MCP (Model Context Protocol) Server Implementation
 * Exposes Adastrea tools to external AI clients via HTTP
 * 
 * Standard endpoints:
 * - POST /mcp/tools/list - Get list of available tools
 * - POST /mcp/tools/call - Execute a tool
 * - POST /mcp/resources - Get available resources (assets, etc.)
 */
class ADASTREADIRECTOR_API FAdastreaMCPServer
{
public:
	FAdastreaMCPServer();
	~FAdastreaMCPServer();

	/**
	 * Start the MCP server on the specified port
	 * @param Port Port number (default: 8088)
	 * @return True if server started successfully
	 */
	bool Start(int32 Port = 8088);

	/**
	 * Stop the MCP server
	 */
	void Stop();

	/**
	 * Check if server is running
	 * @return True if server is active
	 */
	bool IsRunning() const { return bIsRunning; }

	/**
	 * Get the port the server is listening on
	 * @return Port number, or 0 if not running
	 */
	int32 GetPort() const { return ServerPort; }

private:
	/** HTTP router for handling requests */
	TSharedPtr<IHttpRouter> HttpRouter;

	/** Server running state */
	bool bIsRunning = false;

	/** Port number */
	int32 ServerPort = 0;

	/** Route handles for cleanup */
	TArray<FHttpRouteHandle> RouteHandles;

	/**
	 * Handle POST /mcp/tools/list
	 * Returns list of available tools in MCP format
	 */
	bool HandleListTools(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/**
	 * Handle POST /mcp/tools/call
	 * Execute a tool and return results
	 */
	bool HandleExecuteTool(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/**
	 * Handle POST /mcp/resources
	 * Return available resources (assets, blueprints, etc.)
	 */
	bool HandleGetResources(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/**
	 * Parse JSON body from HTTP request
	 */
	TSharedPtr<FJsonObject> ParseRequestBody(const FHttpServerRequest& Request);

	/**
	 * Create JSON response
	 */
	TUniquePtr<FHttpServerResponse> CreateJsonResponse(const TSharedPtr<FJsonObject>& JsonObject, int32 StatusCode = 200);

	/**
	 * Create error response
	 */
	TUniquePtr<FHttpServerResponse> CreateErrorResponse(const FString& Error, int32 StatusCode = 400);
};
