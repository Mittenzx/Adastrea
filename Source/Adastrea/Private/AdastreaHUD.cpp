// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaHUD.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Player/WorldInteractable.h"
#include "Player/PlayerInteractableComponent.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "Stations/SpaceStation.h"
#include "Stations/MarketplaceModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/DirectionalLight.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"

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

		// Tick and draw the transient message (only over the normal flight HUD).
		if (MessageDuration > 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				MessageElapsed += World->GetDeltaSeconds();
			}
			if (MessageElapsed >= MessageDuration)
			{
				PendingMessage = TEXT("");
				MessageDuration = 0.0f;
				MessageElapsed = 0.0f;
			}
			else if (!PendingMessage.IsEmpty())
			{
				DrawTransientMessage(PC);
			}
		}

		// Draw the worldwide interactable prompt (only when not flying a ship).
		if (CurrentInteractable && !Ship)
		{
			DrawInteractPrompt(PC);
		}

		// Docked trading screen draws over everything when shown.
		if (bShowTradeScreen)
		{
			if (Ship)
			{
				DrawTradeScreen(PC, Cast<AAdastreaPlayerController>(PC), Ship);
			}
			return;
		}

	// Ship-select screen draws over everything when shown.
	if (bShowShipSelect)
	{
		DrawShipSelectScreen(PC);
		return;
	}

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

	const float PanelH = RowStartY + (PanelY) + 5 * RowH + 16.0f + 64.0f; // extra room for the 3D compass

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

	// ---- 3D compass (bearing + pitch) under the ship position ----
	{
		// Ship facing from its world rotation.
		const FRotator ShipRot = Ship->GetActorRotation();
		const float YawDeg   = ShipRot.Yaw;           // heading: 0 = +X, 90 = +Y
		const float PitchDeg = ShipRot.Pitch;          // +up / -down

		// Compass geometry: a ring centered under the panel, near the position row.
		const float Cx = PanelX + PanelW * 0.5f;
		const float Cy = Y + 6.0f;
		const float R  = 22.0f;                        // compass ring radius

		// ---- Bearing ring (N/E/S/W projected from current yaw) ----
		// World heading (degrees, 0..360) where +X=0, +Y=90; flip so it reads clockwise.
		const float HeadingClock = FMath::Fmod(YawDeg + 360.0f, 360.0f);
		// Rotate the four compass points opposite the heading so the point we face
		// stays fixed at the "forward" (top) of the ring.
		const float Fwd = HeadingClock;                // degrees, clockwise from +X
		// Points: N=0, E=90, S=180, W=270 (world). Screen offset = -(heading - point).
		const FLinearColor PtCol = FLinearColor(0.62f, 0.78f, 0.85f, 0.9f); // cyan-ish
		const FLinearColor NCol  = FLinearColor(0.95f, 0.75f, 0.45f, 1.0f); // gold = north

		// Draw ring (circle) via short line segments.
		const int32 Segs = 40;
		for (int32 i = 0; i < Segs; ++i)
		{
			const float A0 = (float)i / Segs * 2.0f * PI;
			const float A1 = (float)(i + 1) / Segs * 2.0f * PI;
			const FVector2D P0(Cx + FMath::Cos(A0 + PI) * R, Cy + FMath::Sin(A0 + PI) * R);
			const FVector2D P1(Cx + FMath::Cos(A1 + PI) * R, Cy + FMath::Sin(A1 + PI) * R);
			DrawLine(P0.X, P0.Y, P1.X, P1.Y, FLinearColor(0.25f, 0.35f, 0.42f, 0.9f), 1.0f);
		}

		// Projected compass point labels (N/E/S/W), displaced by heading so the
		// direction you face sits at the top.
		auto DrawCompassPoint = [&](const TCHAR* Label, float PointDeg, const FLinearColor& Col)
		{
			const float AngleRad = FMath::DegreesToRadians(HeadingClock - PointDeg) + PI;
			const float Sx = Cx + FMath::Cos(AngleRad) * R;
			const float Sy = Cy + FMath::Sin(AngleRad) * R;
			DrawText(Label, Col, Sx - 7.0f, Sy - 9.0f, BodyFont, 0.75f);
		};
		DrawCompassPoint(TEXT("N"), 0.0f,   NCol);
				DrawCompassPoint(TEXT("E"), 90.0f,  PtCol);
				DrawCompassPoint(TEXT("S"), 180.0f, PtCol);
				DrawCompassPoint(TEXT("W"), 270.0f, PtCol);

				// ---- Station bearings (gold dots on the ring pointing to each station) ----
				const FVector ShipLoc = Ship->GetActorLocation();
				TArray<AActor*> StationActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceStation::StaticClass(), StationActors);
				// Draw only the near/far-relevant stations for clarity (skip huge distance).
				for (AActor* SA : StationActors)
				{
					if (!SA) { continue; }
					const FVector Delta = SA->GetActorLocation() - ShipLoc;
					const float Dist = Delta.Size();
					// Skip stations beyond a reasonable flux-cone (optional) — 150k units.
					if (Dist < 1.0f || Dist > 200000.0f) { continue; }
					// Clockwise world angle from +X (the "incident" direction). +X=0, +Y=90.
					const float DotWorld = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
					const float BearingClock = FMath::Fmod(DotWorld + 360.0f, 360.0f);
					// Place the dot on the ring, displaced by -heading like compass points:
					// the direction you face is at the top.
					const float AngleRad = FMath::DegreesToRadians(HeadingClock - BearingClock) + PI;
					const float Sx = Cx + FMath::Cos(AngleRad) * R;
					const float Sy = Cy + FMath::Sin(AngleRad) * R;
					// Gold dot (same family as N marker) sized by closeness.
					const float DotR = FMath::Clamp(40000.0f / Dist, 1.5f, 3.5f);
					DrawRect(FLinearColor(0.95f, 0.75f, 0.35f, 0.95f), Sx - DotR, Sy - DotR, DotR*2.0f, DotR*2.0f);
				}

				// Fixed forward tick at the top of the ring (the heading you face).
				DrawLine(Cx - 1.0f, Cy - R - 3.0f, Cx + 1.0f, Cy - R - 1.0f, kBorder, 2.0f);

		// ---- Pitch ladder (vertical bar to the right of the ring) ----
		const float Lx = Cx + R + 10.0f;
		const float PitchHalves = FMath::Clamp(PitchDeg / 90.0f, -1.0f, 1.0f);
		// Draw a vertical scale from -45 (down) at bottom to +45 (up) at top.
		const float Lh = R * 2.0f;
		DrawLine(Lx, Cy - R, Lx, Cy + R, FLinearColor(0.25f,0.35f,0.42f,0.9f), 1.0f);
		DrawLine(Lx - 4.0f, Cy, Lx + 4.0f, Cy + 1.0f, FLinearColor(0.35f,0.5f,0.6f,0.9f), 1.0f); // mid (0)
		// Marker that rises/falls with pitch.
		const float MarkY = Cy + R - (PitchHalves + 1.0f) * 0.5f * Lh;
		DrawLine(Lx - 5.0f, MarkY, Lx + 5.0f, MarkY, kBorder, 2.0f);
		// up/down arrows
		DrawText(TEXT("^"), FLinearColor(0.5f,0.9f,0.7f,1.0f), Lx + 7.0f, Cy - R - 2.0f, BodyFont, 0.6f);
		DrawText(TEXT("v"), FLinearColor(0.9f,0.5f,0.5f,1.0f), Lx + 8.0f, Cy + R - 6.0f, BodyFont, 0.6f);

		// ---- Heading readout (degrees) ----
		const FString HeadingStr = FString::Printf(TEXT("%03.0f."), HeadingClock);
		DrawText(HeadingStr, kPos, Cx - 24.0f, Cy + R + 3.0f, BodyFont, 0.8f);

		Y += RowH + 24.0f;
	}

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
		// Target screen position + in-view/off-view determination.
		const FVector TgtLoc = LockedTarget->GetActorLocation();
		const FString TgtName = LockedTarget->GetActorLabel();
		const float TgtDist = FVector::Dist(P, TgtLoc);

		int32 VSizeX = 0, VSizeY = 0;
		if (PC) { PC->GetViewportSize(VSizeX, VSizeY); }
		const float VW = (float)VSizeX, VH = (float)VSizeY;
		const float Margin = 40.0f; // edge margin for off-screen clamp + "on screen" test

		FVector2D ScreenPt(0.0f, 0.0f);
		const bool bProjected = PC && PC->ProjectWorldLocationToScreen(TgtLoc, ScreenPt);
		const bool bOnScreen = bProjected
			&& ScreenPt.X >= Margin && ScreenPt.X <= VW - Margin
			&& ScreenPt.Y >= Margin && ScreenPt.Y <= VH - Margin
			&& ScreenPt.X > 0 && ScreenPt.Y > 0; // valid (not behind camera when out of view)

		const FLinearColor Reticle = FLinearColor(0.15f, 0.9f, 0.6f, 1.0f); // teal-green

		if (bOnScreen)
		{
			// On-screen corner-box reticle + crosshair around the locked target.
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
		else if (PC)
		{
			// Off-screen: draw an arrow at the viewport edge pointing at the target.
			// Clamp the target's screen position to the edge box (with margin) to get
			// the arrow base, then orient a triangle toward the true on-screen direction.
			FVector2D EdgePt = ScreenPt;
			EdgePt.X = FMath::Clamp(EdgePt.X, Margin, VW - Margin);
			EdgePt.Y = FMath::Clamp(EdgePt.Y, Margin, VH - Margin);

			// Arrow direction = from screen center to the (possibly off-screen) target.
			const FVector2D Center(VW * 0.5f, VH * 0.5f);
			FVector2D Dir = ScreenPt - Center;
			const float Ln = FMath::Max(Dir.Size(), KINDA_SMALL_NUMBER);
			Dir /= Ln;
			const FVector2D N(-Dir.Y, Dir.X); // perpendicular

			const float ArrowLen = 24.0f;
			const float HalfW = 9.0f;
			const FVector2D Tip = EdgePt + Dir * ArrowLen;
			const FVector2D BaseLeft = EdgePt - N * HalfW;
			const FVector2D BaseRight = EdgePt + N * HalfW;
			const FVector2D Back = EdgePt + Dir * (ArrowLen * 0.45f);

			DrawLine(Tip.X, Tip.Y, BaseLeft.X, BaseLeft.Y, Reticle, 2.5f);
			DrawLine(Tip.X, Tip.Y, BaseRight.X, BaseRight.Y, Reticle, 2.5f);
			DrawLine(BaseLeft.X, BaseLeft.Y, Back.X, Back.Y, Reticle, 2.5f);
			DrawLine(BaseRight.X, BaseRight.Y, Back.X, Back.Y, Reticle, 2.5f);

			// Distance label under the arrow.
			DrawText(FString::Printf(TEXT("%.0f"), TgtDist), Reticle,
				EdgePt.X - 14.0f, EdgePt.Y + 12.0f, BodyFont, 0.6f);
		}

		// ---- Right-side target info panel ----
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

	// ---- Object icons (type-distinct glyphs) ----
	TArray<AActor*> Ships;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceship::StaticClass(), Ships);
	TArray<AActor*> Stations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), Stations);

	// Station glyph: hollow gold square (ring) so it reads clearly as a station/landmark
	auto DrawStationIcon = [&](float X, float Y, float R, const FLinearColor& Col, float Thick)
	{
		DrawLine(X - R, Y - R, X + R, Y - R, Col, Thick);
		DrawLine(X + R, Y - R, X + R, Y + R, Col, Thick);
		DrawLine(X + R, Y + R, X - R, Y + R, Col, Thick);
		DrawLine(X - R, Y + R, X - R, Y - R, Col, Thick);
	};

	// Ship glyph: hollow diamond (other ships)
	auto DrawShipIcon = [&](float X, float Y, float R, const FLinearColor& Col, float Thick)
	{
		DrawLine(X, Y - R, X + R, Y, Col, Thick);
		DrawLine(X + R, Y, X, Y + R, Col, Thick);
		DrawLine(X, Y + R, X - R, Y, Col, Thick);
		DrawLine(X - R, Y, X, Y - R, Col, Thick);
	};

	// Stations: gold hollow-square icon + label
	if (bShowStations)
	{
		for (AActor* A : Stations)
		{
			if (!A) continue;
			FVector2D SP;
			if (!Project(A->GetActorLocation(), SP)) continue;
			DrawStationIcon(SP.X, SP.Y, 6.0f, FLinearColor(0.95f, 0.78f, 0.30f, 1.0f), 2.0f);
			DrawText(A->GetActorLabel(), FLinearColor(0.9f, 0.85f, 0.6f, 1.0f), SP.X + 9.0f, SP.Y - 6.0f, MiniFont, 0.55f);
		}
	}
	// Ships: cyan hollow diamond icon + label
	if (bShowShips)
	{
		for (AActor* A : Ships)
		{
			if (!A || A == PC->GetPawn()) continue; // skip the player's own ship (shown as YOU)
			FVector2D SP;
			if (!Project(A->GetActorLocation(), SP)) continue;
			DrawShipIcon(SP.X, SP.Y, 6.0f, FLinearColor(0.3f, 0.8f, 0.9f, 1.0f), 2.0f);
			DrawText(A->GetActorLabel(), FLinearColor(0.6f, 0.85f, 0.95f, 1.0f), SP.X + 8.0f, SP.Y - 6.0f, MiniFont, 0.55f);
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
	// hollow gold square (station)
	DrawLine(BoxX + BoxW - 160.0f, LegY + 1.0f, BoxX + BoxW - 148.0f, LegY + 1.0f, FLinearColor(0.95f,0.78f,0.30f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 148.0f, LegY + 1.0f, BoxX + BoxW - 148.0f, LegY + 11.0f, FLinearColor(0.95f,0.78f,0.30f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 148.0f, LegY + 11.0f, BoxX + BoxW - 160.0f, LegY + 11.0f, FLinearColor(0.95f,0.78f,0.30f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 160.0f, LegY + 11.0f, BoxX + BoxW - 160.0f, LegY + 1.0f, FLinearColor(0.95f,0.78f,0.30f,1.0f), 2.0f);
	LegY += 16.0f;
	DrawText(TEXT("Ships"), FLinearColor(0.8f,0.9f,1.0f,1.0f), BoxX + BoxW - 130.0f, LegY, MiniFont, 0.6f);
	// hollow cyan diamond (ship)
	DrawLine(BoxX + BoxW - 154.0f, LegY + 1.0f, BoxX + BoxW - 147.0f, LegY + 6.0f, FLinearColor(0.3f,0.8f,0.9f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 147.0f, LegY + 6.0f, BoxX + BoxW - 154.0f, LegY + 11.0f, FLinearColor(0.3f,0.8f,0.9f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 154.0f, LegY + 11.0f, BoxX + BoxW - 161.0f, LegY + 6.0f, FLinearColor(0.3f,0.8f,0.9f,1.0f), 2.0f);
	DrawLine(BoxX + BoxW - 161.0f, LegY + 6.0f, BoxX + BoxW - 154.0f, LegY + 1.0f, FLinearColor(0.3f,0.8f,0.9f,1.0f), 2.0f);

	// Filters (bottom-left)
	DrawText(FString::Printf(TEXT("[1] Ships:%s   [2] Stations:%s"),
		bShowShips ? TEXT("ON") : TEXT("OFF"), bShowStations ? TEXT("ON") : TEXT("OFF")),
		FLinearColor(0.6f,0.7f,0.8f,0.9f), Margin, VH - 30.0f, MiniFont, 0.7f);
}

void AAdastreaHUD::MoveTradeSelection(int32 Step)
{
	// We need the item count to clamp; the controller passes Step, and we cap
	// against the docked market's inventory size via a helper.
	if (AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(GetOwningPlayerController()))
	{
		if (ASpaceStation* Station = AdPC->GetNearestTradableStation())
		{
			if (AMarketplaceModule* Marketplace = Station->GetMarketplaceModule())
			{
				if (UMarketDataAsset* Market = Marketplace->GetMarketData())
				{
					int32 N = FMath::Max(Market->Inventory.Num(), 1);
					SelectedTradeIndex = FMath::Clamp(SelectedTradeIndex + Step, 0, N - 1);
					return;
				}
			}
		}
	}
	SelectedTradeIndex = FMath::Max(SelectedTradeIndex + Step, 0);
}

void AAdastreaHUD::DrawTradeScreen(APlayerController* PC, AAdastreaPlayerController* AdController, ASpaceship* Ship)
{
	if (!Ship || !AdController)
	{
		return;
	}

	// Locate the docked station's market.
	ASpaceStation* Station = AdController->GetNearestTradableStation();
	if (!Station || !Station->GetMarketplaceModule())
	{
		UFont* BF = GEngine->GetSmallFont();
		DrawText(TEXT("No active market at this dock."), FLinearColor(0.9f,0.6f,0.4f,1.0f), 200.0f, 200.0f, BF, 1.0f);
		return;
	}
	UMarketDataAsset* Market = Station->GetMarketplaceModule()->GetMarketData();
	if (!Market)
	{
		UFont* BF = GEngine->GetSmallFont();
		DrawText(TEXT("Station market is not configured."), FLinearColor(0.9f,0.6f,0.4f,1.0f), 200.0f, 200.0f, BF, 1.0f);
		return;
	}

	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;

	// Full-screen dim backdrop.
	DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.94f), 0.0f, 0.0f, VW, VH);

	UFont* TitleFont = GEngine->GetLargeFont();
	UFont* BodyFont  = GEngine->GetSmallFont();

	// Credits + cargo header row.
	const int32 Credits = Ship->PlayerTraderComponent ? Ship->PlayerTraderComponent->GetCredits() : 0;
	const float CargoUsed = Ship->CargoComponent ? (Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace()) : 0.0f;
	const float CargoMax = Ship->CargoComponent ? FMath::Max(Ship->CargoComponent->CargoCapacity, 0.01f) : 1.0f;

	DrawText(TEXT("TRADING"), FLinearColor(0.15f,0.9f,0.6f,1.0f), VW*0.5f-60.0f, 18.0f, TitleFont, 1.1f);
	DrawText(Market->GetName(), FLinearColor(0.8f,0.9f,1.0f,1.0f), VW*0.5f-60.0f, 52.0f, BodyFont, 0.8f);
	DrawText(FString::Printf(TEXT("CREDITS: %d"), Credits), FLinearColor(0.95f,0.78f,0.30f,1.0f), 40.0f, 40.0f, BodyFont, 0.9f);
	DrawText(FString::Printf(TEXT("CARGO: %.0f / %.0f"), CargoUsed, CargoMax), FLinearColor(0.15f,0.9f,0.6f,1.0f), 40.0f, 66.0f, BodyFont, 0.9f);
	DrawText(bBuyMode ? TEXT("MODE: BUY") : TEXT("MODE: SELL"),
		bBuyMode ? FLinearColor(0.3f,0.9f,0.4f,1.0f) : FLinearColor(0.9f,0.5f,0.3f,1.0f),
		VW-220.0f, 40.0f, BodyFont, 0.9f);

	// Column headers.
	const float ListX = 60.0f;
	const float PriceX = VW*0.45f;
	const float StockX = VW*0.68f;
	DrawText(TEXT("ITEM"), FLinearColor(0.6f,0.7f,0.8f,1.0f), ListX, 100.0f, BodyFont, 0.7f);
	DrawText(TEXT("PRICE"), FLinearColor(0.6f,0.7f,0.8f,1.0f), PriceX, 100.0f, BodyFont, 0.7f);
	DrawText(TEXT("STOCK"), FLinearColor(0.6f,0.7f,0.8f,1.0f), StockX, 100.0f, BodyFont, 0.7f);

	// Item rows (scroll window around selection).
	const int32 InventoryCount = Market->Inventory.Num();
	constexpr int32 VisibleRows = 12;
	const int32 Half = VisibleRows / 2;
	int32 StartIndex = FMath::Clamp(SelectedTradeIndex - Half, 0, FMath::Max(InventoryCount - VisibleRows, 0));
	int32 EndIndex = FMath::Min(StartIndex + VisibleRows, InventoryCount);
	float RowY = 126.0f;
	const float RowH = 26.0f;

	for (int32 i = StartIndex; i < EndIndex; ++i)
	{
		const FMarketInventoryEntry& Entry = Market->Inventory[i];
		if (!Entry.TradeItem)
		{
			continue;
		}
		// Row background highlight for the selected index.
		if (i == SelectedTradeIndex)
		{
			DrawRect(FLinearColor(0.15f, 0.32f, 0.35f, 0.5f), ListX-10.0f, RowY-2.0f, VW*0.8f, RowH-4.0f);
		}
		const FString ItemName = Entry.TradeItem->ItemName.ToString();
		const int32 ItemPrice = bBuyMode
			? Ship->PlayerTraderComponent->GetBuyCost(Market, Entry.TradeItem, 1)
			: Ship->PlayerTraderComponent->GetSellValue(Market, Entry.TradeItem, 1);
		// dim items not in stock for buy mode
		FLinearColor NameCol = FLinearColor(0.9f,0.95f,1.0f,1.0f);
		if (bBuyMode && Entry.CurrentStock <= 0) { NameCol = FLinearColor(0.4f,0.45f,0.5f,1.0f); }
		DrawText(ItemName, NameCol, ListX, RowY, BodyFont, 0.75f);
		DrawText(FString::Printf(TEXT("%d cr"), ItemPrice), FLinearColor(0.95f,0.78f,0.30f,1.0f), PriceX, RowY, BodyFont, 0.75f);
		DrawText(FString::Printf(TEXT("%d"), Entry.CurrentStock), FLinearColor(0.7f,0.8f,0.9f,1.0f), StockX, RowY, BodyFont, 0.75f);
		RowY += RowH;
	}

	// Footer controls.
	DrawText(TEXT("Up/Down: select    [B]/[S]: toggle Buy/Sell    [Space]: trade 1    [Esc]: close    [Q]: trade 5"),
		FLinearColor(0.6f,0.7f,0.8f,0.9f), VW*0.5f - 380.0f, VH - 40.0f, BodyFont, 0.7f);
}

// ========================================================================
// SHIP SELECT SCREEN (concept prototype — canvas + SceneCapture2D preview)
// ========================================================================

// Ship roster: order = the ship Blueprints (spawnable pawns, selectable/testable).
// This is the list the screen cycles. Later this will come from a data table /
// the crafting tree / ship-construction facility data.
static const TCHAR* ShipRosterClassPaths[] = {
	TEXT("/Game/Blueprints/Ships/BP_Ship_Fighter"),
	TEXT("/Game/Blueprints/Ships/BP_Ship_Freighter"),
	TEXT("/Game/Blueprints/Ships/BP_Ship_Corvette"),
	TEXT("/Game/Blueprints/Ships/BP_Ship_Cruiser"),
	TEXT("/Game/Blueprints/Ships/BP_Ship_Destroyer"),
};
static const int32 ShipRosterCount = UE_ARRAY_COUNT(ShipRosterClassPaths);

// Preview mesh per roster entry (an _Assembled static mesh). The roster pawns
// don't all have meshes assigned in-editor, so we render a mesh directly.
static const TCHAR* ShipRosterMeshPaths[] = {
	TEXT("/AdastreaShips/Meshes/Ships/SM_Ship_Fighter_01_Assembled.SM_Ship_Fighter_01_Assembled"),
	TEXT("/AdastreaShips/Meshes/Ships/SM_Ship_Freighter_01_Assembled.SM_Ship_Freighter_01_Assembled"),
	TEXT("/AdastreaShips/Meshes/Ships/SM_Ship_Corvette_01_Assembled.SM_Ship_Corvette_01_Assembled"),
	TEXT("/AdastreaShips/Meshes/Ships/SM_Ship_Gunship_02_Assembled.SM_Ship_Gunship_02_Assembled"),
	TEXT("/AdastreaShips/Meshes/Ships/SM_Ship_Miner_01_Assembled.SM_Ship_Miner_01_Assembled"),
};
static_assert(UE_ARRAY_COUNT(ShipRosterMeshPaths) == ShipRosterCount, "roster mesh mismatch");

// For each roster entry, optionally associate a data asset whose stats we show.
// This decouples the readout from whatever the live pawn happens to expose.
static const TCHAR* ShipRosterDataAssets[] = {
	TEXT("/Game/DataAssets/Ships/DA_Fighter_ViperInterceptor"),
	TEXT("/Game/DataAssets/Ships/DA_Transport_BehemothFreighter"),
	TEXT("/Game/DataAssets/Ships/DA_Corvette_RaptorAssault"),
	TEXT("/Game/DataAssets/Ships/DA_Cruiser_LifelineMedical"),
	TEXT("/Game/DataAssets/Ships/DA_Transport_GenesisColony"),
};
static_assert(UE_ARRAY_COUNT(ShipRosterDataAssets) == ShipRosterCount, "roster data mismatch");

static TSubclassOf<AActor> LoadShipRosterClass(int32 Index)
{
	if (Index < 0 || Index >= ShipRosterCount)
	{
		return nullptr;
	}
	// Resolve the '/Game/...' path to a Blueprint class. FSoftClassPath needs the
	// full '<asset>/<folder>/Name.Name_C' form, so append '.<name>_C'.
		const FString ObjPath = FString(ShipRosterClassPaths[Index]);
		const FString Path = ObjPath + TEXT(".") + FPaths::GetBaseFilename(ObjPath) + TEXT("_C");
		const FSoftClassPath SoftPath(Path);
		if (!SoftPath.IsValid())
		{
			return nullptr;
		}
		return SoftPath.TryLoadClass<AActor>();
}

void AAdastreaHUD::ShowShipSelect()
{
	bShowShipSelect = true;
	ShipSelectIndex = 0;
	bShipCaptureReady = false;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		RebuildShipPreview(PC);
	}
	// Enter UI-ish input so keyboard nav works without fighting mouse-look.
	if (APlayerController* PC = GetOwningPlayerController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void AAdastreaHUD::HideShipSelect()
{
	bShowShipSelect = false;
	bShipCaptureReady = false;
	// Destroy preview actor + capture.
	if (ShipPreviewActor)
	{
		ShipPreviewActor->Destroy();
		ShipPreviewActor = nullptr;
	}
	if (ShipPreviewCapture)
	{
		ShipPreviewCapture->DestroyComponent();
		ShipPreviewCapture = nullptr;
	}
	if (ShipPreviewRT)
	{
		ShipPreviewRT = nullptr;
	}
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC);
		if (AdPC) { AdPC->bLockMouseLook = false; }
	}
	UE_LOG(LogTemp, Log, TEXT("ShipSelect: screen hidden"));
}

void AAdastreaHUD::RebuildShipPreview(APlayerController* PC)
{
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	UWorld* World = PC->GetWorld();

	// Tear down any existing preview.
	if (ShipPreviewActor)
	{
		ShipPreviewActor->Destroy();
		ShipPreviewActor = nullptr;
	}
	if (ShipPreviewCapture)
	{
		ShipPreviewCapture->DestroyComponent();
		ShipPreviewCapture = nullptr;
	}

	// Load this roster entry's ship class.
	const TSubclassOf<AActor> ShipClass = LoadShipRosterClass(ShipSelectIndex);
	if (!ShipClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipSelect: no class for index %d"), ShipSelectIndex);
		bShipCaptureReady = false;
		return;
	}

	// Spawn a static-mesh actor holding the roster mesh, far from the play area so
		// its mesh doesn't appear in the main view, but within the world for the
		// SceneCapture to see. A StaticMeshActor renders reliably (unlike a detached
		// bare component), which is what the SceneCapture needs to pick it up.
		const FVector PreviewLoc(90000.0f, -90000.0f, 10000.0f);
		UStaticMesh* PreviewMesh = nullptr;
		if (ShipSelectIndex >= 0 && ShipSelectIndex < ShipRosterCount)
		{
			PreviewMesh = LoadObject<UStaticMesh>(nullptr, ShipRosterMeshPaths[ShipSelectIndex]);
		}
		ShipPreviewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PreviewLoc, FRotator::ZeroRotator);
		ShipPreviewMeshComp = nullptr;
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(ShipPreviewActor))
		{
			UStaticMeshComponent* SMComp = SMA->GetStaticMeshComponent();
			if (SMComp && PreviewMesh)
			{
				SMComp->SetStaticMesh(PreviewMesh);
				SMComp->SetHiddenInGame(false);
				SMComp->SetVisibility(true, true);
				// Normalize the preview mesh to a display radius ~1000u.
				const FBoxSphereBounds OrigBounds = PreviewMesh->GetBounds();
				const FVector OrigSize = OrigBounds.BoxExtent * 2.0f;
				const float OrigRadius = OrigSize.Size() * 0.5f;
				if (OrigRadius > 1.0f)
				{
					SMComp->SetWorldScale3D(FVector(1000.0f / OrigRadius));
				}
				ShipPreviewMeshComp = SMComp;
				UE_LOG(LogTemp, Log, TEXT("ShipSelect: preview mesh size=%s scaled=%s"),
					*OrigSize.ToString(), *SMComp->GetComponentScale().ToString());
			}
		}
		if (!PreviewMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShipSelect: no mesh at path for index %d"), ShipSelectIndex);
		}

	// Render target for the capture.
	if (!ShipPreviewRT)
	{
		ShipPreviewRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, 512, 512, RTF_RGBA8);
	}
	if (ShipPreviewRT)
	{
		ShipPreviewRT->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Scene capture: spawn a dedicated ASceneCapture2D actor at the preview area
			// (most reliable — bare HUD-attached components are fragile in PIE).
			ASceneCapture2D* CapActor = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(),
				PreviewLoc + FVector(-2400.0f, 0, 0), FRotator(0, 0, 0));
		if (CapActor)
		{
			ShipPreviewCapture = CapActor->GetCaptureComponent2D();
		}
		else
		{
			ShipPreviewCapture = nullptr;
		}
		if (ShipPreviewCapture)
			{
				ShipPreviewCapture->TextureTarget = ShipPreviewRT;
				ShipPreviewCapture->ShowFlags.SetFog(false);
				ShipPreviewCapture->ShowFlags.SetSkyLighting(false);
				ShipPreviewCapture->ShowFlags.SetDynamicShadows(false);
				ShipPreviewCapture->FOVAngle = 20.0f;
				ShipPreviewCapture->CaptureSource = SCS_SceneColorHDR;
				ShipPreviewCapture->bCaptureEveryFrame = true;
				ShipPreviewCapture->bUseRayTracingIfEnabled = false;
				// Keep capture seeing the WHOLE world (not ShowOnly) so a near preview
				// directional light can illuminate the mesh.
				ShipPreviewCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
				ShipPreviewCapture->CaptureScene();
			}

			// Add a small directional light near the preview so the mesh is lit (the far
			// world location has no scene lights, which made the preview render black).
			ADirectionalLight* PreviewLight = World->SpawnActor<ADirectionalLight>(
				ADirectionalLight::StaticClass(), PreviewLoc + FVector(0, 0, 3000.0f), FRotator(-45.0f, 45.0f, 0.0f));
			if (PreviewLight)
			{
				PreviewLight->SetActorScale3D(FVector(1, 1, 1));
			}

	bShipCaptureReady = ShipPreviewCapture && ShipPreviewRT;
	UE_LOG(LogTemp, Log, TEXT("ShipSelect: preview rebuilt for roster index %d (ready=%d)"),
		ShipSelectIndex, bShipCaptureReady ? 1 : 0);
}

void AAdastreaHUD::CycleShipSelect(int32 Step)
{
	const int32 Next = FMath::Clamp(ShipSelectIndex + Step, 0, ShipRosterCount - 1);
	if (Next != ShipSelectIndex)
	{
		ShipSelectIndex = Next;
		ShipPreviewYaw = -35.0f;
		ShipPreviewPitch = 8.0f;
		if (APlayerController* PC = GetOwningPlayerController())
		{
			RebuildShipPreview(PC);
		}
	}
}

void AAdastreaHUD::OrbitShipPreview(float DeltaYaw, float DeltaPitch)
{
	ShipPreviewYaw = FMath::Fmod(ShipPreviewYaw + DeltaYaw, 360.0f);
	ShipPreviewPitch = FMath::Clamp(ShipPreviewPitch + DeltaPitch, -60.0f, 60.0f);
}

USpaceshipDataAsset* AAdastreaHUD::GetPreviewShipDataAsset() const
{
	// Read stats from the roster's associated data asset (robust regardless of
	// whether the live pawn exposes ShipDataAsset).
	if (ShipSelectIndex < 0 || ShipSelectIndex >= ShipRosterCount)
	{
		return nullptr;
	}
	return LoadObject<USpaceshipDataAsset>(nullptr, ShipRosterDataAssets[ShipSelectIndex]);
}

void AAdastreaHUD::SpawnSelectedShip(APlayerController* PC)
{
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	const TSubclassOf<AActor> ShipClass = LoadShipRosterClass(ShipSelectIndex);
	if (!ShipClass)
	{
		return;
	}
	UWorld* World = PC->GetWorld();
	const FVector SpawnLoc = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector(18000, 18000, 5000);
	const FRotator SpawnRot = PC->GetPawn() ? PC->GetPawn()->GetActorRotation() : FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* NewPawn = World->SpawnActor<AActor>(ShipClass, SpawnLoc, SpawnRot, Params);
	if (NewPawn)
	{
		// Destroy old pawn, possess new.
		AActor* Old = PC->GetPawn();
		PC->UnPossess();
		PC->Possess(Cast<APawn>(NewPawn));
		PC->SetViewTarget(NewPawn);
		if (Old) { Old->Destroy(); }
		UE_LOG(LogTemp, Log, TEXT("ShipSelect: spawned+possessed %llx"), (void*)NewPawn);
	}
	HideShipSelect();
}

void AAdastreaHUD::DrawShipSelectScreen(APlayerController* PC)
{
	if (!PC) { return; }
	// Ensure the preview capture is built (first draw / after opening).
	if (!bShipCaptureReady)
	{
		RebuildShipPreview(PC);
	}
	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;

	// Full-screen dim backdrop.
		DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.92f), 0.0f, 0.0f, VW, VH);

		// Apply current preview orbit to the preview ship + capture each frame.
			if (bShipCaptureReady && ShipPreviewCapture && ShipPreviewActor)
			{
				// Rotate the model to the orbit yaw/pitch.
				ShipPreviewActor->SetActorRotation(FRotator(ShipPreviewPitch, ShipPreviewYaw, 0.0f));
								const FVector ActorLoc = ShipPreviewActor->GetActorLocation();
								// We normalized the mesh to ~1000u display radius, so a fixed arm
								// frames every ship consistently.
								const float CamDist = 2400.0f;
								ShipPreviewCapture->SetWorldLocation(ActorLoc + FVector(-CamDist, 0, 0));
								ShipPreviewCapture->SetWorldRotation(FRotator(0, 0, 0));
			}

			UFont* TitleFont = GEngine->GetLargeFont();
	UFont* BodyFont  = GEngine->GetSmallFont();

	DrawText(TEXT("SHIP SELECT"), FLinearColor(0.15f,0.9f,0.6f,1.0f), VW*0.5f - 80.0f, 18.0f, TitleFont, 1.2f);

	// ---- Left: ship list ----
	const float LX = 40.0f, LY = 80.0f;
	for (int32 i = 0; i < ShipRosterCount; ++i)
	{
		const TSubclassOf<AActor> Cls = LoadShipRosterClass(i);
		FString Label = Cls ? Cls->GetName() : FString(TEXT("<unknown>"));
		Label.RemoveFromStart(TEXT("BP_"));
		const float RowY = LY + i * 30.0f;
		const bool bSelected = (i == ShipSelectIndex);
		if (bSelected)
		{
			DrawRect(FLinearColor(0.15f,0.32f,0.35f,0.5f), LX, RowY, 220.0f, 24.0f);
		}
		DrawText(Label, bSelected ? FLinearColor(1,1,1,1) : FLinearColor(0.7f,0.8f,0.9f,1), LX+8, RowY+2, BodyFont, 0.85f);
	}

	// ---- Center-right: 3D preview ----
	const float PX = VW*0.5f - 40.0f, PY = 60.0f, PW = 460.0f, PH = 460.0f;
	DrawRect(FLinearColor(0.03f,0.06f,0.09f,0.95f), PX, PY, PW, PH);   // preview stage
	DrawLine(PX, PY, PX+PW, PY, kBorder, 2.0f);
	DrawLine(PX, PY+PH, PX+PW, PY+PH, kBorder, 2.0f);
	DrawLine(PX, PY, PX, PY+PH, kBorder, 2.0f);
	DrawLine(PX+PW, PY, PX+PW, PY+PH, kBorder, 2.0f);

	if (bShipCaptureReady && ShipPreviewRT)
	{
		DrawTexture(ShipPreviewRT, PX+20, PY+20, PW-40, PH-40, 0, 0, 1, 1, FLinearColor::White);
	}
	else
	{
		DrawText(TEXT("[ preview unavailable ]"), FLinearColor(0.5f,0.6f,0.7f,1), PX+PW*0.5f-110, PY+PH*0.5f, BodyFont, 0.8f);
	}

	// ---- Right: stats ----
	const float SX = PX + PW + 30.0f, SY = 80.0f;
	USpaceshipDataAsset* DA = GetPreviewShipDataAsset();
	if (DA)
	{
		const FString ShipName = DA->ShipName.ToString();
		const FString ShipClass = DA->ShipClass.ToString();
		TArray<FString> Lbls;
		TArray<FString> Vals;
		Lbls.Add(TEXT("CLASS"));    Vals.Add(ShipClass);
		Lbls.Add(TEXT("MAX SPEED"));Vals.Add(FString::Printf(TEXT("%.0f u/s"), DA->MaxSpeed));
		Lbls.Add(TEXT("ACCEL"));    Vals.Add(FString::Printf(TEXT("%.0f u/s^2"), DA->Acceleration));
		Lbls.Add(TEXT("MANEUVER")); Vals.Add(FString::Printf(TEXT("%d/10"), DA->Maneuverability));
		Lbls.Add(TEXT("HULL"));     Vals.Add(FString::Printf(TEXT("%.0f"), DA->HullStrength));
		Lbls.Add(TEXT("SHIELD"));   Vals.Add(FString::Printf(TEXT("%.0f"), DA->ShieldStrength));
		Lbls.Add(TEXT("CARGO"));    Vals.Add(FString::Printf(TEXT("%.0f m^3"), DA->CargoCapacity));
		Lbls.Add(TEXT("JUMP RANGE"));Vals.Add(FString::Printf(TEXT("%.0f ly"), DA->JumpRange));
		Lbls.Add(TEXT("MOBILITY")); Vals.Add(FString::Printf(TEXT("%.0f"), DA->GetMobilityRating()));
		Lbls.Add(TEXT("COMBAT"));   Vals.Add(FString::Printf(TEXT("%.0f"), DA->GetCombatRating()));

		DrawText(ShipName, FLinearColor(0.95f,0.78f,0.30f,1), SX, SY, TitleFont, 1.0f);
		float Y = SY + 40.0f;
		for (int32 i = 0; i < Lbls.Num(); ++i)
		{
			DrawText(Lbls[i], kLabel, SX, Y, BodyFont, 0.85f);
			DrawText(Vals[i], FLinearColor(0.85f,0.9f,0.95f,1), SX+150, Y, BodyFont, 0.85f);
			Y += 24.0f;
		}
	}
	else
	{
		DrawText(TEXT("(no data asset on this pawn)"), FLinearColor(0.6f,0.7f,0.8f,1), SX, SY, BodyFont, 0.8f);
	}

	// ---- Footer controls ----
	DrawText(TEXT("Left/Right: rotate    Up/Down or A/D: cycle ship    [Space]: select & fly    [Esc]: close"),
		FLinearColor(0.6f,0.7f,0.8f,0.9f), VW*0.5f - 400.0f, VH - 40.0f, BodyFont, 0.7f);
}

void AAdastreaHUD::ShowMessage(const FString& InMessage, float DurationSecs, bool bIsWarning)
{
	PendingMessage = InMessage;
	MessageDuration = FMath::Max(0.1f, DurationSecs);
	MessageElapsed = 0.0f;
	bMessageIsWarning = bIsWarning;
}

void AAdastreaHUD::DrawTransientMessage(APlayerController* PC)
{
	if (!Canvas || PendingMessage.IsEmpty())
	{
		return;
	}

	UFont* MsgFont = GEngine->GetSmallFont();

	int32 VW = 0;
	int32 VH = 0;
	if (PC) { PC->GetViewportSize(VW, VH); }

	const FString Message = PendingMessage;
	const FLinearColor Colour = bMessageIsWarning ? FLinearColor(1.0f, 0.4f, 0.35f, 1.0f) : FLinearColor(0.4f, 0.9f, 1.0f, 1.0f);

	// Measure the text so we can center it and size the backing bar.
	float TextW = 0.0f;
	float TextH = 0.0f;
	Canvas->StrLen(MsgFont, Message, TextW, TextH);

	const float X = (VW - TextW) * 0.5f;
	const float Y = 120.0f;
	const float Pad = 10.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f), X - Pad, Y - Pad, TextW + Pad * 2.0f, TextH + Pad * 2.0f);
	DrawText(Message, Colour, X, Y, MsgFont, 0.8f);
}

void AAdastreaHUD::SetCurrentInteractable(AActor* InActor)
{
	CurrentInteractable = InActor;
}

void AAdastreaHUD::DrawInteractPrompt(APlayerController* PC)
{
	if (!Canvas || !CurrentInteractable)
	{
		return;
	}

	const IWorldInteractable* Interactable = Cast<IWorldInteractable>(CurrentInteractable);
	if (!Interactable)
	{
		// Actor may carry a UPlayerInteractableComponent.
		if (UPlayerInteractableComponent* Comp = CurrentInteractable->FindComponentByClass<UPlayerInteractableComponent>())
		{
			Interactable = Comp;
		}
	}
	if (!Interactable)
	{
		return;
	}

	const FText Prompt = Interactable->GetInteractPrompt_Implementation();
	UFont* MsgFont = GEngine->GetSmallFont();

	int32 VW = 0;
	int32 VH = 0;
	if (PC) { PC->GetViewportSize(VW, VH); }

	const FString Line = FString::Printf(TEXT("%s   [E]"), *Prompt.ToString());
	float TextW = 0.0f;
	float TextH = 0.0f;
	Canvas->StrLen(MsgFont, Line, TextW, TextH);

	const float X = (VW - TextW) * 0.5f;
	const float Y = VH - 140.0f;
	const float Pad = 10.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), X - Pad, Y - Pad, TextW + Pad * 2.0f, TextH + Pad * 2.0f);
	DrawText(Line, FLinearColor(0.4f, 0.95f, 1.0f, 1.0f), X, Y, MsgFont, 0.85f);
}
