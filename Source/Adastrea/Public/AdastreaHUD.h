// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AdastreaHUD.generated.h"

/**
 * Adastrea in-game HUD.
 *
 * Uses the classic engine AHUD::DrawHUD() canvas path to draw test telemetry
 * (coords/speed/throttle/credits/cargo) directly over the game viewport every
 * frame. This is independent of UMG/CommonUI and always renders during PIE,
 * making it the dependable mechanism for the MVP test HUD.
 */
UCLASS()
class ADASTREA_API AAdastreaHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};