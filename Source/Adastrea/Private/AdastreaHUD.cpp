// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaHUD.h"
#include "Ships/Spaceship.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void AAdastreaHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw live test telemetry over the game viewport every frame via the classic
	// AHUD canvas. This path does not depend on UMG/CommonUI and always renders
	// during PIE so it can't be swallowed by the widget layer.
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn());
	if (!Ship)
	{
		// No ship possessed yet; still show a stub so the HUD is visibly working.
		DrawText(TEXT("Adastrea HUD: awaiting ship possession..."),
			FLinearColor::White, 16.0f, 16.0f);
		return;
	}

	const FVector P = Ship->GetActorLocation();
	const float Speed = Ship->MovementComponent ? Ship->MovementComponent->Velocity.Size() : 0.0f;
	const int32 Credits = Ship->PlayerTraderComponent ? static_cast<int32>(Ship->PlayerTraderComponent->GetCredits()) : 0;
	const float CargoUsed = Ship->CargoComponent ? (Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace()) : 0.0f;
	const float CargoMax = Ship->CargoComponent ? Ship->CargoComponent->CargoCapacity : 0.0f;

	const FString Msg = FString::Printf(
		TEXT("Adastrea Telemetry\nCredits: %d  Cargo: %.0f/%.0f\nSpeed: %.0f  Throttle: %.0f%%\nPos: X=%.0f  Y=%.0f  Z=%.0f"),
		Credits, CargoUsed, CargoMax, Speed, Ship->ThrottlePercentage,
		P.X, P.Y, P.Z);

	// Semi-transparent backing box for readability, then the text (top-left).
	const float TextX = 16.0f, TextY = 16.0f;
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f), TextX - 4.0f, TextY - 4.0f, 420.0f, 120.0f);
	DrawText(Msg, FLinearColor::White, TextX, TextY, nullptr, 1.0f, true);
}