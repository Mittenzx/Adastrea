// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdastreaGameMode.generated.h"

/**
 * Base game mode for Adastrea
 * This is the primary game mode that controls gameplay rules and state
 */
UCLASS(minimalapi)
class AAdastreaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAdastreaGameMode();
};
