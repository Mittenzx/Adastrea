// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaGameInstance.h"
#include "Factions/FactionDiplomacyManager.h"
#include "AdastreaLog.h"

UAdastreaGameInstance::UAdastreaGameInstance()
	: FactionDiplomacyManager(nullptr)
	, PlayerCredits(10000)
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
	// Create the faction diplomacy manager subsystem
	FactionDiplomacyManager = GetSubsystem<UFactionDiplomacyManager>();
}

bool UAdastreaGameInstance::SaveGame(const FString& SlotName)
{
	// TODO: Implement save game functionality
	// This would serialize player state, factions, and other persistent data
	UE_LOG(LogAdastrea, Warning, TEXT("SaveGame called but not yet implemented"));
	return false;
}

bool UAdastreaGameInstance::LoadGame(const FString& SlotName)
{
	// TODO: Implement load game functionality
	// This would deserialize player state, factions, and other persistent data
	UE_LOG(LogAdastrea, Warning, TEXT("LoadGame called but not yet implemented"));
	return false;
}

void UAdastreaGameInstance::ModifyPlayerCredits(int32 Amount)
{
	PlayerCredits = FMath::Max(0, PlayerCredits + Amount);
}
