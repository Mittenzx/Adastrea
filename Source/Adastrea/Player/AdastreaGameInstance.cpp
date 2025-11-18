// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaGameInstance.h"
#include "Player/SaveGameSubsystem.h"
#include "AdastreaLog.h"

UAdastreaGameInstance::UAdastreaGameInstance()
	: PlayerCredits(10000)
{
}

void UAdastreaGameInstance::Init()
{
	Super::Init();
	
	InitializeGameSystems();
}

void UAdastreaGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UAdastreaGameInstance::InitializeGameSystems()
{
	// Initialize save game subsystem (will auto-initialize via subsystem system)
	USaveGameSubsystem* SaveSystem = GetSubsystem<USaveGameSubsystem>();
	if (SaveSystem)
	{
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaGameInstance: Save system initialized"));
	}
}

bool UAdastreaGameInstance::SaveGame(const FString& SlotName)
{
	USaveGameSubsystem* SaveSystem = GetSubsystem<USaveGameSubsystem>();
	if (!SaveSystem)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameInstance: SaveGameSubsystem not found"));
		return false;
	}

	return SaveSystem->SaveGame(SlotName, true);
}

bool UAdastreaGameInstance::LoadGame(const FString& SlotName)
{
	USaveGameSubsystem* SaveSystem = GetSubsystem<USaveGameSubsystem>();
	if (!SaveSystem)
	{
		UE_LOG(LogAdastrea, Error, TEXT("AdastreaGameInstance: SaveGameSubsystem not found"));
		return false;
	}

	return SaveSystem->LoadGame(SlotName);
}

void UAdastreaGameInstance::ModifyPlayerCredits(int32 Amount)
{
	PlayerCredits = FMath::Max(0, PlayerCredits + Amount);
}
