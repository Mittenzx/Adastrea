// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaHUD.h"
#include "Ships/Spaceship.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Stations/SpaceStation.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

// Palette (subtle sci-fi, on-brand for a teal/cyan accent theme)
static const FLinearColor kBg      (0.02f, 0.03f, 0.05f, 0.72f); // deep space panel
static const FLinearColor kBorder  (0.10f, 0.65f, 0.72f, 0.90f); // teal accent
static const FLinearColor kHeader  (0.60f, 0.85f, 0.90f, 1.00f); // bright cyan-white
static const FLinearColor kLabel   (0.55f, 0.62f, 0.68f, 1.00f); // muted grey-blue
static const FLinearColor kCredit  (0.95f, 0.78f, 0.30f, 1.00f); // gold
static const FLinearColor kCargo   (0.35f, 0.85f, 0.75f, 1.00f); // teal-green
static const FLinearColor kSpeed   (0.35f, 0.72f, 0.95f, 1.00f); // cyan
static const FLinearColor kThrottle(0.80f, 0.55f, 0.90f, 1.00f); // violet
static const FLinearColor kPos     (0.75f, 0.75f, 0.80f, 1.00f); // soft white

void AAdastreaHUD::DrawHUD()
{
	Super::DrawHUD();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn());
	if (!Ship)
	{
		return; // only draw once we're flying the ship
	}

	// ---- Gather live data ----
	const FVector P = Ship->GetActorLocation();
	const float Speed = Ship->MovementComponent ? Ship->MovementComponent->Velocity.Size() : 0.0f;
	const int32 Credits = Ship->PlayerTraderComponent ? static_cast<int32>(Ship->PlayerTraderComponent->GetCredits()) : 0;
	const float CargoUsed = Ship->CargoComponent ? (Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace()) : 0.0f;
	const float CargoMax = Ship->CargoComponent ? FMath::Max(Ship->CargoComponent->CargoCapacity, 0.01f) : 1.0f;
	const float Throttle = Ship->ThrottlePercentage;

	UFont* TitleFont   = GEngine->GetLargeFont();
	UFont* BodyFont    = GEngine->GetSmallFont();

	// Panel origin (top-left, slightly inset)
	const float PanelX = 20.0f;
	const float PanelY = 20.0f;

	// We'll render into a fixed-size panel; rows of 22px.
	const float RowH  = 22.0f;
	const float PanelW = 380.0f;
	const float LabelX = PanelX + 14.0f;
	const float ValueX = LabelX + 130.0f;
	const float TitleY = PanelY + 10.0f;
	const float RowStartY = TitleY + 34.0f;

	const float PanelH = RowStartY + (PanelY) + 5 * RowH + 16.0f;

	// ---- Dark translucent panel ----
	DrawRect(kBg, PanelX, PanelY, PanelW, PanelH);

	// ---- Accent border (teal line along the left edge) ----
	DrawLine(PanelX, PanelY, PanelX, PanelY + PanelH, kBorder, 3.0f);
	DrawLine(PanelX + 2, PanelY + PanelH - 1, PanelX + PanelW - 2, PanelY + PanelH - 1, kBorder, 1.0f);

	// ---- Title + separator ----
	DrawText(TEXT("A D A S T R E A   //   FLIGHT TELEMETRY"),
		kHeader, PanelX + 14.0f, TitleY, TitleFont, 0.9f);
	DrawLine(PanelX + 12.0f, TitleY + 26.0f, PanelX + PanelW - 12.0f, TitleY + 26.0f, kBorder, 1.0f);

	float Y = RowStartY;
	auto Row = [&](const TCHAR* Label, const FString& Value, const FLinearColor& ValueColor)
	{
		DrawText(Label, kLabel, LabelX, Y, BodyFont, 0.9f);
		DrawText(Value, ValueColor, ValueX, Y, BodyFont, 0.9f);
		Y += RowH;
	};

	Row(TEXT("CREDITS"),   FString::Printf(TEXT("%d cr"), Credits), kCredit);
	Row(TEXT("CARGO"),     FString::Printf(TEXT("%.0f / %.0f"), CargoUsed, CargoMax), kCargo);
	Row(TEXT("VELOCITY"),  FString::Printf(TEXT("%.0f u/s"), Speed), kSpeed);
	Row(TEXT("THROTTLE"),  FString::Printf(TEXT("%.0f%%"), Throttle), kThrottle);

	// ---- Throttle bar (visual) ----
	const float BarX = LabelX;
	const float BarY = Y + 2.0f;
	const float BarW = PanelW - 28.0f;
	const float BarH = 10.0f;
	DrawRect(FLinearColor(0.08f, 0.10f, 0.12f, 0.9f), BarX, BarY, BarW, BarH);            // track
	const float Fill = FMath::Clamp(Throttle / 100.0f, 0.0f, 1.0f);
	DrawRect(FLinearColor::LerpUsingHSV(FLinearColor(0.2f,0.6f,0.9f), FLinearColor(0.9f,0.3f,0.4f), Fill),
		BarX, BarY, FMath::Max(BarW * Fill, 2.0f), BarH);                                  // fill
	Y += RowH;

	// ---- Position (bottom of panel) ----
	Y += 4.0f;
	DrawText(TEXT("POSITION"), kLabel, LabelX, Y, BodyFont, 0.8f);
	DrawText(FString::Printf(TEXT("X %8.0f   Y %8.0f   Z %8.0f"), P.X, P.Y, P.Z),
		kPos, ValueX, Y, BodyFont, 0.8f);
	Y += RowH;

	// ---- Locked target (from controller targeting) ----
	AAdastreaPlayerController* AController = Cast<AAdastreaPlayerController>(PC);
	AActor* LockedTarget = AController ? AController->GetLockedTarget() : nullptr;
	if (LockedTarget)
	{
		DrawText(TEXT("TARGET"), kLabel, LabelX, Y, BodyFont, 0.8f);
		const FString TgtName = LockedTarget->GetActorLabel();
		const float TgtDist = FVector::Dist(P, LockedTarget->GetActorLocation());
		DrawText(FString::Printf(TEXT("%s  (%.0f)"), *TgtName, TgtDist),
			FLinearColor(0.95f, 0.55f, 0.35f, 1.0f), ValueX, Y, BodyFont, 0.8f);
	}
	if (AController && AController->IsTargetingModeActive())
	{
		Y += RowH;
		DrawText(TEXT("TARGETING MODE - click a station / Tab to exit"), kThrottle, LabelX, Y, BodyFont, 0.7f);
	}
}
