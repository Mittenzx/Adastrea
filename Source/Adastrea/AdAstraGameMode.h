// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdAstraGameMode.generated.h"

/**
 * Base game mode for AdAstra
 * This is the primary game mode that controls gameplay rules and state
 */
UCLASS(minimalapi)
class AAdAstraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAdAstraGameMode();
};
