// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaHUD.h"
#include "Ships/Spaceship.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Stations/SpaceStation.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

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

	// Full-screen sector map (toggled by M) draws over everything.
	if (bShowMap)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			DrawSectorMap(PC, Pawn->GetActorLocation());
		}
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

	// ---- Locked target reticle (world-space box around the locked target) ----
	AAdastreaPlayerController* AController = Cast<AAdastreaPlayerController>(PC);
	AActor* LockedTarget = AController ? AController->GetLockedTarget() : nullptr;

	// Hover highlight: while targeting, show which station the cursor is over.
	AActor* HoverTarget = nullptr;
	if (AController && AController->IsTargetingModeActive())
	{
		HoverTarget = AController->GetStationUnderCursor();
	}

	// Draw reticle on the hovered station (if any, and not already locked).
	if (HoverTarget && HoverTarget != LockedTarget && PC)
	{
		FVector2D SPt;
		if (PC->ProjectWorldLocationToScreen(HoverTarget->GetActorLocation(), SPt))
		{
			const FLinearColor HoverCol = FLinearColor(1.0f, 0.85f, 0.3f, 1.0f); // amber
			const float BH = 30.0f;
			DrawLine(SPt.X - BH, SPt.Y - BH, SPt.X - 10.0f, SPt.Y - BH, HoverCol, 2.0f);
			DrawLine(SPt.X + BH, SPt.Y - BH, SPt.X + 10.0f, SPt.Y - BH, HoverCol, 2.0f);
			DrawLine(SPt.X - BH, SPt.Y + BH, SPt.X - 10.0f, SPt.Y + BH, HoverCol, 2.0f);
			DrawLine(SPt.X + BH, SPt.Y + BH, SPt.X + 10.0f, SPt.Y + BH, HoverCol, 2.0f);
			DrawLine(SPt.X - BH, SPt.Y - BH, SPt.X - BH, SPt.Y - 10.0f, HoverCol, 2.0f);
			DrawLine(SPt.X + BH, SPt.Y - BH, SPt.X + BH, SPt.Y - 10.0f, HoverCol, 2.0f);
			DrawLine(SPt.X - BH, SPt.Y + BH, SPt.X - BH, SPt.Y + 10.0f, HoverCol, 2.0f);
			DrawLine(SPt.X + BH, SPt.Y + BH, SPt.X + BH, SPt.Y + 10.0f, HoverCol, 2.0f);
		}
	}

	if (LockedTarget)
	{
		// Screen-space corner-box reticle + crosshair around the locked target.
		const FVector TgtLoc = LockedTarget->GetActorLocation();
		const FString TgtName = LockedTarget->GetActorLabel();
		const float TgtDist = FVector::Dist(P, TgtLoc);

		FVector2D ScreenPt;
		if (PC && PC->ProjectWorldLocationToScreen(TgtLoc, ScreenPt))
		{
			const FLinearColor Reticle = FLinearColor(0.15f, 0.9f, 0.6f, 1.0f); // teal-green
			const float BoxHalf = FMath::Clamp(TgtDist * 0.01f, 20.0f, 60.0f);
			const float RX = ScreenPt.X;
			const float RY = ScreenPt.Y;
			const float L = 12.0f;
			DrawLine(RX - BoxHalf, RY - BoxHalf, RX - BoxHalf + L, RY - BoxHalf, Reticle, 2.0f);
			DrawLine(RX - BoxHalf, RY - BoxHalf, RX - BoxHalf, RY - BoxHalf + L, Reticle, 2.0f);
			DrawLine(RX + BoxHalf, RY - BoxHalf, RX + BoxHalf - L, RY - BoxHalf, Reticle, 2.0f);
			DrawLine(RX + BoxHalf, RY - BoxHalf, RX + BoxHalf, RY - BoxHalf + L, Reticle, 2.0f);
			DrawLine(RX - BoxHalf, RY + BoxHalf, RX - BoxHalf + L, RY + BoxHalf, Reticle, 2.0f);
			DrawLine(RX - BoxHalf, RY + BoxHalf, RX - BoxHalf, RY + BoxHalf - L, Reticle, 2.0f);
			DrawLine(RX + BoxHalf, RY + BoxHalf, RX + BoxHalf - L, RY + BoxHalf, Reticle, 2.0f);
			DrawLine(RX + BoxHalf, RY + BoxHalf, RX + BoxHalf, RY + BoxHalf - L, Reticle, 2.0f);
			DrawLine(RX - 6.0f, RY, RX + 6.0f, RY, Reticle, 1.0f);
			DrawLine(RX, RY - 6.0f, RX, RY + 6.0f, Reticle, 1.0f);
		}

		// ---- Right-side target info panel ----
		int32 VSizeX = 0, VSizeY = 0;
		if (PC)
		{
			PC->GetViewportSize(VSizeX, VSizeY);
			const float PX = VSizeX - 300.0f;
			const float PY = VSizeY * 0.45f;
			const float PW = 280.0f;
			const float PH = 130.0f;
			const float LX = PX + 12.0f;
			const float VX = PX + 100.0f;

			DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.75f), PX, PY, PW, PH);
			DrawLine(PX, PY, PX, PY + PH, kBorder, 3.0f); // teal accent edge

			DrawText(TEXT("TARGET LOCKED"), FLinearColor(0.15f, 0.9f, 0.6f, 1.0f), LX, PY + 8.0f, TitleFont, 0.9f);
			DrawLine(PX + 10.0f, PY + 30.0f, PX + PW - 10.0f, PY + 30.0f, kBorder, 1.0f);

			DrawText(TEXT("NAME"), kLabel, LX, PY + 40.0f, BodyFont, 0.7f);
			DrawText(TgtName, FLinearColor::White, VX, PY + 40.0f, BodyFont, 0.7f);

			DrawText(TEXT("DISTANCE"), kLabel, LX, PY + 62.0f, BodyFont, 0.7f);
			DrawText(FString::Printf(TEXT("%.0f u"), TgtDist), kSpeed, VX, PY + 62.0f, BodyFont, 0.7f);

			// Station module info (if it's a station)
			if (ASpaceStation* Station = Cast<ASpaceStation>(LockedTarget))
			{
				int32 ModuleCount = Station->GetModuleCount();
				int32 Docks = Station->GetDockingBayModules().Num();
				DrawText(TEXT("MODULES"), kLabel, LX, PY + 84.0f, BodyFont, 0.7f);
				DrawText(FString::Printf(TEXT("%d  (%d docks)"), ModuleCount, Docks),
					kCargo, VX, PY + 84.0f, BodyFont, 0.7f);
			}
		}
	}

	// Subtle targeting-mode indicator (bottom-center, unobtrusive).
			if (AController && AController->IsTargetingModeActive())
			{
				int32 VSizeX = 0, VSizeY = 0;
				if (PC)
				{
					PC->GetViewportSize(VSizeX, VSizeY);
					const FString Hint = TEXT("TARGETING ACTIVE - Tab to exit");
					DrawText(Hint, FLinearColor(0.6f, 0.7f, 0.75f, 0.9f),
						VSizeX * 0.5f - 80.0f, VSizeY - 40.0f, BodyFont, 0.6f);
				}
			}
	}

	void AAdastreaHUD::DrawSectorMap(APlayerController* PC, const FVector& ShipPos)
	{
		if (!PC)
		{
			return;
		}

		UWorld* World = PC->GetWorld();
		if (!World)
		{
			return;
		}

		// Gather stations.
		TArray<AActor*> Stations;
		UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), Stations);

		// Compute world bounds covering all stations + the player (X/Y plane).
		float MinX = ShipPos.X, MaxX = ShipPos.X;
		float MinY = ShipPos.Y, MaxY = ShipPos.Y;
		FVector Ship2D = FVector(ShipPos.X, ShipPos.Y, 0.0f);
		for (AActor* S : Stations)
		{
			if (!S)
			{
				continue;
			}
			FVector Loc = S->GetActorLocation();
			MinX = FMath::Min(MinX, Loc.X); MaxX = FMath::Max(MaxX, Loc.X);
			MinY = FMath::Min(MinY, Loc.Y); MaxY = FMath::Max(MaxY, Loc.Y);
		}
		// Ensure a minimum extent so a lone ship still has a useful view.
		float ExtentX = FMath::Max(MaxX - MinX, 20000.0f);
		float ExtentY = FMath::Max(MaxY - MinY, 20000.0f);
		float Cx = (MinX + MaxX) * 0.5f;
		float Cy = (MinY + MaxY) * 0.5f;
		MinX = Cx - ExtentX * 0.5f; MaxX = Cx + ExtentX * 0.5f;
		MinY = Cy - ExtentY * 0.5f; MaxY = Cy + ExtentY * 0.5f;

		// Retina/viewport size.
		int32 VX = 0, VY = 0;
		PC->GetViewportSize(VX, VY);
		float VW = (float)VX, VH = (float)VY;

		// Map area: full-screen overlay (with a small margin frame), preserving world aspect.
		const float Margin = 20.0f;
		float MapW = VW - Margin * 2.0f;
		float MapH = VH - Margin * 2.0f - 40.0f; // leave room for the title/footer
		// Fit extent into MapW x MapH by uniform scale.
		float WorldAspect = ExtentX / FMath::Max(ExtentY, 1.0f);
		float MapAspect = MapW / FMath::Max(MapH, 1.0f);
		float Scale;
		if (WorldAspect > MapAspect)
		{
			Scale = MapW / FMath::Max(ExtentX, 1.0f);
			MapH = ExtentY * Scale;
		}
		else
		{
			Scale = MapH / FMath::Max(ExtentY, 1.0f);
			MapW = ExtentX * Scale;
		}
		const float PX = Margin + (VW - Margin * 2.0f - MapW) * 0.5f;
		const float PY = Margin + (VH - Margin * 2.0f - MapH) * 0.5f;

		// Full-screen backing (solid, covers the play viewport while map is up).
		DrawRect(FLinearColor(0.01f, 0.015f, 0.025f, 1.0f), 0.0f, 0.0f, VW, VH);

		// Screen position for a world (X,Y) point, Y up -> screen Y down.
		auto ToScreen = [&](const FVector& W) -> FVector2D
		{
			float sx = PX + (W.X - MinX) * Scale;
			float sy = PY + (MaxY - W.Y) * Scale; // invert so +Y is up
			return FVector2D(sx, sy);
		};

		// ---- Panel background + border ----
		DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.88f), PX, PY, MapW, MapH);
		DrawRect(FLinearColor(0.10f, 0.65f, 0.72f, 0.9f), PX, PY, 3.0f, MapH);           // left accent
		DrawRect(FLinearColor(0.10f, 0.65f, 0.72f, 0.9f), PX, PY, MapW, 3.0f);           // top accent
		DrawRect(FLinearColor(0.10f, 0.65f, 0.72f, 0.7f), PX, PY + MapH - 3.0f, MapW, 3.0f);

		// Title
		UFont* TitleFont = GEngine->GetLargeFont();
		UFont* BodyFont  = GEngine->GetSmallFont();
		DrawText(TEXT("SECTOR MAP"), FLinearColor(0.60f, 0.85f, 0.90f, 1.0f), PX + 20.0f, PY - 28.0f, TitleFont, 0.9f);

		// Legend
		DrawText(TEXT("Y (up)"), FLinearColor(0.4f,0.5f,0.6f,1.0f), PX + MapW - 90.0f, PY + 10.0f, BodyFont, 0.6f);

		// ---- Station markers + names ----
		for (AActor* S : Stations)
		{
			if (!S)
			{
				continue;
			}
			FVector2D SP = ToScreen(S->GetActorLocation());
			// Clip to map rect.
			if (SP.X < PX || SP.X > PX + MapW || SP.Y < PY || SP.Y > PY + MapH)
			{
				continue;
			}
			// Small square marker.
			const float M = 6.0f;
			DrawRect(FLinearColor(0.95f, 0.78f, 0.30f, 1.0f), SP.X - M, SP.Y - M, M * 2.0f, M * 2.0f);
			// Name below marker.
			DrawText(S->GetActorLabel(), FLinearColor(0.85f, 0.9f, 0.95f, 1.0f), SP.X - 12.0f, SP.Y + 8.0f, BodyFont, 0.6f);
		}

		// ---- Player marker (teal arrow) ----
		FVector2D PPt = ToScreen(Ship2D);
		const float PA = 10.0f;
		// Simple triangle pointing up (ship "north" = +Y).
		DrawLine(PPt.X, PPt.Y - PA, PPt.X - PA * 0.8f, PPt.Y + PA * 0.5f, kBorder, 2.0f);
		DrawLine(PPt.X, PPt.Y - PA, PPt.X + PA * 0.8f, PPt.Y + PA * 0.5f, kBorder, 2.0f);
		DrawLine(PPt.X - PA * 0.8f, PPt.Y + PA * 0.5f, PPt.X + PA * 0.8f, PPt.Y + PA * 0.5f, kBorder, 2.0f);
		DrawText(TEXT("YOU"), FLinearColor(0.15f, 0.9f, 0.6f, 1.0f), PPt.X - 8.0f, PPt.Y + 12.0f, BodyFont, 0.6f);

		// Footer hint (bottom-center of screen)
		DrawText(TEXT("Press M to close map"), FLinearColor(0.5f,0.6f,0.7f,1.0f),
			VW * 0.5f - 70.0f, VH - 36.0f, BodyFont, 0.7f);

	// Legend (top-right)
		DrawText(TEXT("Y (up)"), FLinearColor(0.4f,0.5f,0.6f,1.0f), VW - 90.0f, 12.0f, BodyFont, 0.6f);
	}
