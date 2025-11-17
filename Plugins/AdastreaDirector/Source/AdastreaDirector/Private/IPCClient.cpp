// Copyright (c) 2025 Mittenzx. All Rights Reserved.

#include "IPCClient.h"
#include "AdastreaDirectorModule.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "HAL/PlatformTime.h"

FIPCClient::FIPCClient()
	: Socket(nullptr)
	, SocketSubsystem(nullptr)
{
	SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
}

FIPCClient::~FIPCClient()
{
	Disconnect();
}

bool FIPCClient::Connect(const FString& Host, int32 Port, float TimeoutSeconds)
{
	// Disconnect any existing connection
	if (Socket != nullptr)
	{
		Disconnect();
	}

	if (SocketSubsystem == nullptr)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Socket subsystem not available"));
		return false;
	}

	// Create socket
	Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("AdastreaDirector IPC Client"), false);
	if (Socket == nullptr)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to create socket"));
		return false;
	}

	// Set socket to non-blocking for timeout support
	Socket->SetNonBlocking(true);

	// Parse address
	FIPv4Address IPv4Address;
	if (!FIPv4Address::Parse(Host, IPv4Address))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Invalid host address: %s"), *Host);
		Disconnect();
		return false;
	}

	// Create address
	ServerAddress = SocketSubsystem->CreateInternetAddr();
	ServerAddress->SetIp(IPv4Address.Value);
	ServerAddress->SetPort(Port);

	UE_LOG(LogAdastreaDirector, Log, TEXT("Attempting to connect to %s:%d"), *Host, Port);

	// Attempt connection
	if (!Socket->Connect(*ServerAddress))
	{
		ESocketErrors Error = SocketSubsystem->GetLastErrorCode();
		
		// EINPROGRESS/EWOULDBLOCK is expected for non-blocking sockets
		if (Error != SE_EINPROGRESS && Error != SE_EWOULDBLOCK)
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to connect: %s"), 
				SocketSubsystem->GetSocketError(Error));
			Disconnect();
			return false;
		}
	}

	// Wait for connection to complete
	double StartTime = FPlatformTime::Seconds();
	while (FPlatformTime::Seconds() - StartTime < TimeoutSeconds)
	{
		ESocketConnectionState State = Socket->GetConnectionState();
		
		if (State == SCS_Connected)
		{
			UE_LOG(LogAdastreaDirector, Log, TEXT("Successfully connected to %s:%d"), *Host, Port);
			return true;
		}
		else if (State == SCS_ConnectionError)
		{
			UE_LOG(LogAdastreaDirector, Error, TEXT("Connection error"));
			Disconnect();
			return false;
		}

		FPlatformProcess::Sleep(0.01f);
	}

	UE_LOG(LogAdastreaDirector, Error, TEXT("Connection timeout after %.1f seconds"), TimeoutSeconds);
	Disconnect();
	return false;
}

void FIPCClient::Disconnect()
{
	if (Socket != nullptr)
	{
		UE_LOG(LogAdastreaDirector, Log, TEXT("Disconnecting IPC client"));
		
		Socket->Close();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}

	ServerAddress = nullptr;
}

bool FIPCClient::IsConnected() const
{
	return IsSocketValid() && Socket->GetConnectionState() == SCS_Connected;
}

bool FIPCClient::SendRequest(const FString& RequestJson, FString& OutResponse, float TimeoutSeconds)
{
	if (!IsConnected())
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Not connected to IPC server"));
		return false;
	}

	// Convert request to UTF-8 bytes
	FTCHARToUTF8 UTF8String(*RequestJson);
	TArray<uint8> RequestData;
	RequestData.Append((uint8*)UTF8String.Get(), UTF8String.Length());

	// Add newline as message delimiter
	RequestData.Add('\n');

	// Send request
	if (!Send(RequestData))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to send request"));
		return false;
	}

	// Receive response
	TArray<uint8> ResponseData;
	if (!Receive(ResponseData, TimeoutSeconds))
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to receive response"));
		return false;
	}

	// Convert response to string
	OutResponse = FString(UTF8_TO_TCHAR(ResponseData.GetData()));
	
	return true;
}

bool FIPCClient::Send(const TArray<uint8>& Data)
{
	if (!IsSocketValid())
	{
		return false;
	}

	int32 BytesSent = 0;
	int32 TotalSent = 0;
	const uint8* DataPtr = Data.GetData();
	int32 DataSize = Data.Num();

	while (TotalSent < DataSize)
	{
		if (!Socket->Send(DataPtr + TotalSent, DataSize - TotalSent, BytesSent))
		{
			ESocketErrors Error = SocketSubsystem->GetLastErrorCode();
			UE_LOG(LogAdastreaDirector, Error, TEXT("Send failed: %s"), 
				SocketSubsystem->GetSocketError(Error));
			return false;
		}

		TotalSent += BytesSent;

		// Small delay to avoid overwhelming the socket
		if (TotalSent < DataSize)
		{
			FPlatformProcess::Sleep(0.001f);
		}
	}

	return true;
}

bool FIPCClient::Receive(TArray<uint8>& OutData, float TimeoutSeconds)
{
	if (!IsSocketValid())
	{
		return false;
	}

	OutData.Reset();
	TArray<uint8> Buffer;
	Buffer.SetNumUninitialized(4096);

	double StartTime = FPlatformTime::Seconds();
	bool bFoundDelimiter = false;

	while (FPlatformTime::Seconds() - StartTime < TimeoutSeconds)
	{
		if (!WaitForSocket(0.1f, true))
		{
			continue;
		}

		int32 BytesRead = 0;
		if (Socket->Recv(Buffer.GetData(), Buffer.Num(), BytesRead))
		{
			if (BytesRead > 0)
			{
				// Append received data
				OutData.Append(Buffer.GetData(), BytesRead);

				// Check for message delimiter (newline)
				for (int32 i = OutData.Num() - BytesRead; i < OutData.Num(); i++)
				{
					if (OutData[i] == '\n')
					{
						bFoundDelimiter = true;
						// Null-terminate for string conversion
						OutData[i] = '\0';
						OutData.SetNum(i + 1);
						break;
					}
				}

				if (bFoundDelimiter)
				{
					break;
				}
			}
		}
		else
		{
			ESocketErrors Error = SocketSubsystem->GetLastErrorCode();
			if (Error != SE_EWOULDBLOCK && Error != SE_NO_ERROR)
			{
				UE_LOG(LogAdastreaDirector, Error, TEXT("Receive failed: %s"), 
					SocketSubsystem->GetSocketError(Error));
				return false;
			}
		}

		FPlatformProcess::Sleep(0.01f);
	}

	if (!bFoundDelimiter)
	{
		UE_LOG(LogAdastreaDirector, Error, TEXT("Receive timeout after %.1f seconds"), TimeoutSeconds);
		return false;
	}

	return OutData.Num() > 0;
}

bool FIPCClient::IsSocketValid() const
{
	return Socket != nullptr && Socket->GetConnectionState() == SCS_Connected;
}

bool FIPCClient::WaitForSocket(float TimeoutSeconds, bool bWaitForRead) const
{
	if (!IsSocketValid())
	{
		return false;
	}

	double StartTime = FPlatformTime::Seconds();
	
	while (FPlatformTime::Seconds() - StartTime < TimeoutSeconds)
	{
		if (bWaitForRead)
		{
			uint32 PendingDataSize = 0;
			if (Socket->HasPendingData(PendingDataSize))
			{
				return true;
			}
		}
		else
		{
			// For write, just check if socket is still valid
			return true;
		}

		FPlatformProcess::Sleep(0.01f);
	}

	return false;
}
