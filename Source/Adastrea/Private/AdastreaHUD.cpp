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
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	UWorld* World = PC->GetWorld();

	// ---- Viewport ----
	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	float VW = (float)VX, VH = (float)VY;

	// Full-screen backing.
	DrawRect(FLinearColor(0.008f, 0.012f, 0.02f, 1.0f), 0.0f, 0.0f, VW, VH);

	// Map view box (X4 style: big centered 3D viewport).
	const float Margin = 40.0f;
	const float BoxW = VW - Margin * 2.0f;
	const float BoxH = VH - Margin * 2.0f - 70.0f;
	const float BoxX = Margin;
	const float BoxY = Margin + 30.0f;

	// ---- Camera / projection ----
	// Orbit camera looking at MapCenter from (yaw,pitch,zoom).
	float YawR = FMath::DegreesToRadians(MapYaw);
	float PitchR = FMath::DegreesToRadians(MapPitch);
	FVector Eye = MapCenter
		+ FVector(FMath::Cos(PitchR) * FMath::Cos(YawR),
				  FMath::Cos(PitchR) * FMath::Sin(YawR),
				  FMath::Sin(PitchR)) * MapZoom;
	FVector Fwd = (MapCenter - Eye).GetSafeNormal();
	FVector WorldUp(0, 0, 1);
	FVector Right = Fwd.Cross(WorldUp).GetSafeNormal();
	if (Right.IsNearlyZero()) { Right = FVector(1, 0, 0); }
	FVector Up = Right.Cross(Fwd).GetSafeNormal();

	// Project world -> camera-space -> screen.
	// Center of box = optical axis; scale so MapZoom frames the box.
	float Focal = BoxW * 0.5f;

	auto Project = [&](const FVector& W, FVector2D& Out) -> bool
	{
		FVector Rel = W - Eye;
		float CamX = Rel.Dot(Right);
		float CamY = Rel.Dot(Up);
		float CamZ = Rel.Dot(Fwd);
		if (CamZ < 1000.0f) { return false; } // behind/near too close
		float S = Focal / CamZ;
		float sx = BoxX + BoxW * 0.5f + CamX * S;
		float sy = BoxY + BoxH * 0.5f - CamY * S;
		Out = FVector2D(sx, sy);
		return (sx >= BoxX && sx <= BoxX + BoxW && sy >= BoxY && sy <= BoxY + BoxH);
	};

	// Non-clipping projector (for grid lines whose endpoints may be off-box).
	auto ProjectRaw = [&](const FVector& W, FVector2D& Out) -> bool
	{
		FVector Rel = W - Eye;
		float CamX = Rel.Dot(Right);
		float CamY = Rel.Dot(Up);
		float CamZ = Rel.Dot(Fwd);
		if (CamZ < 1000.0f) { return false; }
		float S = Focal / CamZ;
		Out = FVector2D(BoxX + BoxW * 0.5f + CamX * S, BoxY + BoxH * 0.5f - CamY * S);
		return true;
	};

	// Draw a screen-space line clipped to the map box (Liang-Barsky).
	auto DrawGridLine = [&](FVector2D A, FVector2D B, const FLinearColor& Col, float Thick)
	{
		const float XMin = BoxX, XMax = BoxX + BoxW;
		const float YMin = BoxY, YMax = BoxY + BoxH;
		const float x0 = A.X, y0 = A.Y, x1 = B.X, y1 = B.Y;
		const float dx = x1 - x0, dy = y1 - y0;

		const float P[4] = { -dx, dx, -dy, dy };
		const float Q[4] = { x0 - XMin, XMax - x0, y0 - YMin, YMax - y0 };

		float u0 = 0.0f, u1 = 1.0f;
		bool bVisible = true;
		for (int32 i = 0; i < 4; ++i)
		{
			if (FMath::Abs(P[i]) < 1.e-6f)
			{
				if (Q[i] < 0.0f) { bVisible = false; break; }
			}
			else
			{
				float r = Q[i] / P[i];
				if (P[i] < 0.0f) { u0 = FMath::Max(u0, r); }
				else             { u1 = FMath::Min(u1, r); }
			}
		}
		if (!bVisible || u0 > u1) { return; }

		const FVector2D C(x0 + u0 * dx, y0 + u0 * dy);
		const FVector2D D(x0 + u1 * dx, y0 + u1 * dy);
		DrawLine(C.X, C.Y, D.X, D.Y, Col, Thick);
	};

	// ---- Box background (drawn before the grid so the grid stays visible) ----
	DrawRect(FLinearColor(0.03f, 0.05f, 0.07f, 0.9f), BoxX, BoxY, BoxW, BoxH);

	// ---- Grid on Z=0 plane (hierarchical: minor + major lines for perspective) ----
	{
		const FLinearColor kMinor = FLinearColor(0.13f, 0.20f, 0.26f, 0.6f);
		const FLinearColor kMajor = FLinearColor(0.35f, 0.60f, 0.68f, 0.85f);
		// Adaptive minor spacing so the grid always reads at any zoom.
		float G = FMath::Pow(10.0f, FMath::FloorToFloat(FMath::LogX(10.0f, MapZoom * 0.25f)));
		G = FMath::Clamp(G, 5000.0f, 50000.0f);
		const float Major = G * 4.0f;
		float R = MapZoom * 1.6f;

		auto DrawAxis = [&](float Coord, bool bX)
		{
			bool bIsMajor = (FMath::Abs(Coord) < 1.0f) || (FMath::Abs(FMath::Fmod(Coord, Major)) < 1.0f);
			const FLinearColor& Col = bIsMajor ? kMajor : kMinor;
			float Thick = bIsMajor ? 2.0f : 1.0f;
			FVector2D A, B;
			if (bX)
			{
				if (ProjectRaw(MapCenter + FVector(Coord, -R, 0), A) && ProjectRaw(MapCenter + FVector(Coord, R, 0), B))
					DrawGridLine(A, B, Col, Thick);
			}
			else
			{
				if (ProjectRaw(MapCenter + FVector(-R, Coord, 0), A) && ProjectRaw(MapCenter + FVector(R, Coord, 0), B))
					DrawGridLine(A, B, Col, Thick);
			}
		};

		float Start = -FMath::FloorToFloat(R / G) * G;
		for (float gx = Start; gx <= R; gx += G) DrawAxis(gx, true);
		for (float gy = Start; gy <= R; gy += G) DrawAxis(gy, false);
	}

	UFont* TitleFont = GEngine->GetLargeFont();
	UFont* MiniFont  = GEngine->GetSmallFont();

	// ---- Box border (outline; the bg fill is drawn before the grid) ----
	DrawRect(FLinearColor(0.10f, 0.55f, 0.60f, 0.9f), BoxX, BoxY, 3.0f, BoxH);      // left
	DrawRect(FLinearColor(0.10f, 0.55f, 0.60f, 0.9f), BoxX, BoxY, BoxW, 3.0f);      // top
	DrawRect(FLinearColor(0.10f, 0.55f, 0.60f, 0.9f), BoxX, BoxY + BoxH - 3.0f, BoxW, 3.0f); // bottom

	// ---- Object icons ----
	TArray<AActor*> Ships;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceship::StaticClass(), Ships);
	TArray<AActor*> Stations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), Stations);

	// Stations: gold square + label
	if (bShowStations)
	{
		for (AActor* A : Stations)
		{
			if (!A) continue;
			FVector2D SP;
			if (!Project(A->GetActorLocation(), SP)) continue;
			const float R = 5.0f;
			DrawRect(FLinearColor(0.95f, 0.78f, 0.30f, 1.0f), SP.X - R, SP.Y - R, R * 2.0f, R * 2.0f);
			DrawText(A->GetActorLabel(), FLinearColor(0.8f, 0.9f, 1.0f, 0.95f), SP.X + 8.0f, SP.Y - 6.0f, MiniFont, 0.55f);
		}
	}
	// Ships: cyan triangle + label
	if (bShowShips)
	{
		for (AActor* A : Ships)
		{
			if (!A || A == PC->GetPawn()) continue; // skip the player's own ship (shown as YOU)
			FVector2D SP;
			if (!Project(A->GetActorLocation(), SP)) continue;
			const float R = 6.0f;
			DrawLine(SP.X, SP.Y - R, SP.X - R, SP.Y + R * 0.6f, FLinearColor(0.3f, 0.8f, 0.9f, 1.0f), 2.0f);
			DrawLine(SP.X, SP.Y - R, SP.X + R, SP.Y + R * 0.6f, FLinearColor(0.3f, 0.8f, 0.9f, 1.0f), 2.0f);
			DrawLine(SP.X - R, SP.Y + R * 0.6f, SP.X + R, SP.Y + R * 0.6f, FLinearColor(0.3f, 0.8f, 0.9f, 1.0f), 2.0f);
			DrawText(A->GetActorLabel(), FLinearColor(0.8f, 0.9f, 1.0f, 0.95f), SP.X + 8.0f, SP.Y - 6.0f, MiniFont, 0.55f);
		}
	}

	// ---- Player marker (bright teal arrow, "YOU") ----
	{
		FVector2D PPt;
		if (Project(ShipPos, PPt))
		{
			const float R = 9.0f;
			DrawLine(PPt.X, PPt.Y - R, PPt.X - R, PPt.Y + R * 0.6f, FLinearColor(0.15f, 0.95f, 0.6f, 1.0f), 2.0f);
			DrawLine(PPt.X, PPt.Y - R, PPt.X + R, PPt.Y + R * 0.6f, FLinearColor(0.15f, 0.95f, 0.6f, 1.0f), 2.0f);
			DrawLine(PPt.X - R, PPt.Y + R * 0.6f, PPt.X + R, PPt.Y + R * 0.6f, FLinearColor(0.15f, 0.95f, 0.6f, 1.0f), 2.0f);
			DrawText(TEXT("YOU"), FLinearColor(0.15f, 0.95f, 0.6f, 1.0f), PPt.X - 7.0f, PPt.Y + 11.0f, MiniFont, 0.6f);
		}
	}

	// ---- Locked target highlight on map ----
	if (AAdastreaPlayerController* AController = Cast<AAdastreaPlayerController>(PC))
	{
		if (AActor* T = AController->GetLockedTarget())
		{
			FVector2D TP;
			if (Project(T->GetActorLocation(), TP))
			{
				const float R = 11.0f;
				const FLinearColor Rc(0.15f, 0.9f, 0.6f, 1.0f);
				DrawLine(TP.X - R, TP.Y - R, TP.X + R, TP.Y - R, Rc, 2.0f);
				DrawLine(TP.X + R, TP.Y - R, TP.X + R, TP.Y + R, Rc, 2.0f);
				DrawLine(TP.X + R, TP.Y + R, TP.X - R, TP.Y + R, Rc, 2.0f);
				DrawLine(TP.X - R, TP.Y + R, TP.X - R, TP.Y - R, Rc, 2.0f);
			}
		}
	}

	// ---- HUD overlay (title, controls, legend) ----
	DrawText(TEXT("SECTOR MAP"), FLinearColor(0.6f, 0.9f, 1.0f, 1.0f), BoxX + 16.0f, 12.0f, TitleFont, 0.9f);
	// Controls help (bottom)
	DrawText(TEXT("[Q/E] orbit    [A/D] pan X   [W/S] pan Y   [R/F] zoom    LMB target    [M] close"),
		FLinearColor(0.6f,0.7f,0.8f,0.9f), VW*0.5f - 360.0f, VH - 30.0f, MiniFont, 0.7f);
	// Legend (top-right)
	float LegY = 16.0f;
	DrawText(TEXT("Stations"), FLinearColor(0.8f,0.9f,1.0f,1.0f), BoxX + BoxW - 130.0f, LegY, MiniFont, 0.6f);
	DrawRect(FLinearColor(0.95f,0.78f,0.30f,1.0f), BoxX + BoxW - 160.0f, LegY + 1.0f, 8.0f, 8.0f);
	LegY += 16.0f;
	DrawText(TEXT("Ships"), FLinearColor(0.8f,0.9f,1.0f,1.0f), BoxX + BoxW - 130.0f, LegY, MiniFont, 0.6f);
	DrawText(TEXT("<|"), FLinearColor(0.3f,0.8f,0.9f,1.0f), BoxX + BoxW - 158.0f, LegY - 1.0f, MiniFont, 0.7f);

	// Filters (bottom-left)
	DrawText(FString::Printf(TEXT("[1] Ships:%s   [2] Stations:%s"),
		bShowShips ? TEXT("ON") : TEXT("OFF"), bShowStations ? TEXT("ON") : TEXT("OFF")),
		FLinearColor(0.6f,0.7f,0.8f,0.9f), Margin, VH - 30.0f, MiniFont, 0.7f);
}
