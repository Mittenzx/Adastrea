// Copyright Epic Games, Inc. All Rights Reserved.

// Neon cockpit flight HUD for AAdastreaHUD (canvas-drawn, same path as the rest of the HUD).
//
// Layout (all sizes scale with viewport height, authored at 1080p):
//   top-centre    heading tape (N/E/S/W, station bearings, pitch)
//   centre        boresight + flight-path marker (where the ship is actually going)
//   top-left      ident strip: ship, credits, cargo, coords (mining panel stacks below)
//   bottom-left   DEFENCE GRID: segmented shield + hull bars, hit flash, low-hull alarm
//   bottom-right  PROPULSION: big speed readout, speed/throttle bars, FA/BOOST/CRUISE tags

#include "AdastreaHUD.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Mining/MiningLaserComponent.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "CanvasItem.h"
#include "GlobalRenderResources.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Neon palette
	const FLinearColor kCyan    (0.00f, 0.90f, 1.00f, 1.00f);
	const FLinearColor kCyanDim (0.00f, 0.45f, 0.55f, 0.55f);
	const FLinearColor kMagenta (1.00f, 0.08f, 0.60f, 1.00f);
	const FLinearColor kYellow  (1.00f, 0.86f, 0.05f, 1.00f);
	const FLinearColor kRed     (1.00f, 0.12f, 0.18f, 1.00f);
	const FLinearColor kInk     (0.85f, 0.95f, 1.00f, 1.00f);
	const FLinearColor kMute    (0.45f, 0.58f, 0.68f, 1.00f);
	const FLinearColor kPanel   (0.03f, 0.00f, 0.06f, 0.62f);
	const FLinearColor kSegOff  (0.05f, 0.12f, 0.16f, 0.80f);
	const FLinearColor kGreen   (0.20f, 1.00f, 0.55f, 1.00f);

	FLinearColor WithAlpha(FLinearColor C, float A) { C.A = A; return C; }

	/** Cheap deterministic hash in [0,1) for glitch timing. */
	float Hash01(float X) { return FMath::Frac(FMath::Sin(X * 12.9898f) * 43758.5453f); }

	/** Thin wrapper over UCanvas with viewport scale + neon drawing primitives. */
	struct FNeon
	{
		UCanvas* C;
		float S;     // viewport scale (1 at 1080p)
		float Now;   // real time, for pulses/glitches

		void Line(FVector2D A, FVector2D B, const FLinearColor& Col, float Thick = 1.0f) const
		{
			FCanvasLineItem L(A, B);
			L.SetColor(Col);
			L.LineThickness = Thick * S;
			L.BlendMode = SE_BLEND_Translucent;
			C->DrawItem(L);
		}

		/** Line with a soft wide underlay so it reads as glowing. */
		void GlowLine(FVector2D A, FVector2D B, const FLinearColor& Col, float Thick = 1.5f) const
		{
			Line(A, B, WithAlpha(Col, Col.A * 0.18f), Thick * 4.0f);
			Line(A, B, Col, Thick);
		}

		void Poly(const TArray<FVector2D>& Pts, const FLinearColor& Col) const
		{
			if (Pts.Num() < 3) { return; }
			FVector2D Centre = FVector2D::ZeroVector;
			for (const FVector2D& P : Pts) { Centre += P; }
			Centre /= Pts.Num();

			TArray<FCanvasUVTri> Tris;
			for (int32 i = 0; i < Pts.Num(); ++i)
			{
				FCanvasUVTri T;
				T.V0_Pos = Centre; T.V1_Pos = Pts[i]; T.V2_Pos = Pts[(i + 1) % Pts.Num()];
				T.V0_Color = T.V1_Color = T.V2_Color = Col;
				Tris.Add(T);
			}
			FCanvasTriangleItem Item(Tris, GWhiteTexture);
			Item.BlendMode = SE_BLEND_Translucent;
			C->DrawItem(Item);
		}

		void Quad(FVector2D A, FVector2D B, FVector2D Cc, FVector2D D, const FLinearColor& Col) const
		{
			Poly({ A, B, Cc, D }, Col);
		}

		void Rect(float X, float Y, float W, float H, const FLinearColor& Col) const
		{
			Quad({ X, Y }, { X + W, Y }, { X + W, Y + H }, { X, Y + H }, Col);
		}

		/** Panel with the top-left and bottom-right corners clipped, neon edge, accent tab and scanlines. */
		void Panel(float X, float Y, float W, float H, const FLinearColor& Edge, bool bCutLeft = true) const
		{
			const float Cut = 14.0f * S;
			TArray<FVector2D> Pts = bCutLeft
				? TArray<FVector2D>{ { X + Cut, Y }, { X + W, Y }, { X + W, Y + H - Cut }, { X + W - Cut, Y + H }, { X, Y + H }, { X, Y + Cut } }
				: TArray<FVector2D>{ { X, Y }, { X + W - Cut, Y }, { X + W, Y + Cut }, { X + W, Y + H }, { X + Cut, Y + H }, { X, Y + H - Cut } };
			Poly(Pts, kPanel);

			// Scanlines
			for (float SY = Y + 2.0f * S; SY < Y + H - 1.0f; SY += 3.0f * S)
			{
				Rect(X + 2.0f * S, SY, W - 4.0f * S, FMath::Max(1.0f, 0.8f * S), FLinearColor(0.0f, 0.9f, 1.0f, 0.035f));
			}

			for (int32 i = 0; i < Pts.Num(); ++i)
			{
				Line(Pts[i], Pts[(i + 1) % Pts.Num()], WithAlpha(Edge, 0.55f), 1.0f);
			}
			// Bright accent along the clipped corner + a tab on the top edge
			GlowLine(Pts[bCutLeft ? 5 : 1], Pts[bCutLeft ? 0 : 2], Edge, 2.0f);
			const float TabX = bCutLeft ? X + W - 70.0f * S : X + 10.0f * S;
			Rect(TabX, Y - 3.0f * S, 60.0f * S, 3.0f * S, Edge);
		}

		void TextSize(UFont* F, const FString& T, float Scale, float& OutW, float& OutH) const
		{
			C->TextSize(F, T, OutW, OutH, Scale * S, Scale * S);
		}

		void Text(const FString& T, const FLinearColor& Col, float X, float Y, UFont* F, float Scale) const
		{
			FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(T), F, Col);
			Item.Scale = FVector2D(Scale * S);
			Item.BlendMode = SE_BLEND_Translucent;
			C->DrawItem(Item);
		}

		/** Text with a magenta/cyan chromatic split; occasionally jitters sideways (glitch). */
		void GlitchText(const FString& T, const FLinearColor& Col, float X, float Y, UFont* F, float Scale, float Seed = 0.0f) const
		{
			const float Slot = FMath::FloorToFloat(Now * 9.0f) + Seed * 31.0f;
			const bool bGlitch = Hash01(Slot) > 0.94f;
			const float Jx = bGlitch ? (Hash01(Slot + 1.7f) - 0.5f) * 10.0f * S : 0.0f;
			const float Split = (bGlitch ? 3.0f : 1.2f) * S;
			Text(T, WithAlpha(kMagenta, 0.55f), X + Jx - Split, Y, F, Scale);
			Text(T, WithAlpha(kCyan, 0.55f), X + Jx + Split, Y, F, Scale);
			Text(T, Col, X + Jx, Y, F, Scale);
		}

		/**
		 * Row of slanted segments filled to Frac. Lit segments blend From->To along the bar;
		 * the partially-filled segment is cut proportionally.
		 */
		void SegBar(float X, float Y, float W, float H, float Frac, int32 Segs,
			const FLinearColor& From, const FLinearColor& To, float Alpha = 1.0f) const
		{
			const float Gap = 2.0f * S;
			const float Skew = H * 0.45f;
			const float SegW = (W - Skew - Gap * (Segs - 1)) / Segs;
			Frac = FMath::Clamp(Frac, 0.0f, 1.0f);
			for (int32 i = 0; i < Segs; ++i)
			{
				const float X0 = X + i * (SegW + Gap);
				Quad({ X0 + Skew, Y }, { X0 + Skew + SegW, Y }, { X0 + SegW, Y + H }, { X0, Y + H }, kSegOff);

				const float Lit = FMath::Clamp(Frac * Segs - i, 0.0f, 1.0f);
				if (Lit <= 0.0f) { continue; }
				FLinearColor Col = FLinearColor::LerpUsingHSV(From, To, Segs > 1 ? (float)i / (Segs - 1) : 0.0f);
				Col.A = Alpha;
				const float LW = SegW * Lit;
				// Glow halo, then the segment itself
				Quad({ X0 + Skew - Gap, Y - Gap }, { X0 + Skew + LW + Gap, Y - Gap }, { X0 + LW + Gap, Y + H + Gap }, { X0 - Gap, Y + H + Gap }, WithAlpha(Col, 0.16f * Alpha));
				Quad({ X0 + Skew, Y }, { X0 + Skew + LW, Y }, { X0 + LW, Y + H }, { X0, Y + H }, Col);
			}
		}

		/** Small boxed mode tag (lit or dim). */
		float Tag(const FString& T, float X, float Y, const FLinearColor& Col, bool bOn, UFont* F) const
		{
			float TW = 0.0f, TH = 0.0f;
			TextSize(F, T, 0.75f, TW, TH);
			const float W = TW + 14.0f * S, H = TH + 4.0f * S;
			const float K = 5.0f * S;
			if (bOn)
			{
				Poly({ { X + K, Y }, { X + W, Y }, { X + W - K, Y + H }, { X, Y + H } }, WithAlpha(Col, 0.85f));
				Text(T, FLinearColor(0.02f, 0.0f, 0.05f, 1.0f), X + 7.0f * S, Y + 2.0f * S, F, 0.75f);
			}
			else
			{
				const FLinearColor Dim = WithAlpha(Col, 0.3f);
				Line({ X + K, Y }, { X + W, Y }, Dim); Line({ X + W, Y }, { X + W - K, Y + H }, Dim);
				Line({ X + W - K, Y + H }, { X, Y + H }, Dim); Line({ X, Y + H }, { X + K, Y }, Dim);
				Text(T, Dim, X + 7.0f * S, Y + 2.0f * S, F, 0.75f);
			}
			return W + 6.0f * S;
		}

		/** Filled key-cap ("E", "LMB") with a glow, followed by a label. Returns total width. */
		float KeyHint(const FString& Key, const FString& Label, float X, float Y, const FLinearColor& Col, UFont* F) const
		{
			float KW = 0.0f, KH = 0.0f;
			TextSize(F, Key, 0.85f, KW, KH);
			const float W = FMath::Max(KW + 12.0f * S, KH + 6.0f * S), H = KH + 6.0f * S;
			Rect(X - 2.0f * S, Y - 2.0f * S, W + 4.0f * S, H + 4.0f * S, WithAlpha(Col, 0.2f));
			Rect(X, Y, W, H, Col);
			Text(Key, FLinearColor(0.02f, 0.0f, 0.05f, 1.0f), X + (W - KW) * 0.5f, Y + 3.0f * S, F, 0.85f);
			float LW = 0.0f, LH = 0.0f;
			TextSize(F, Label, 0.8f, LW, LH);
			Text(Label, kInk, X + W + 8.0f * S, Y + (H - LH) * 0.5f, F, 0.8f);
			return W + 8.0f * S + LW;
		}
	};

	/** Readable name for a docking module: its station's StationName, else a tidied actor name. */
	FString DockDisplayName(const ASpaceship* Ship, const FString& Fallback)
	{
		if (const AActor* Module = Ship->GetNearbyStation())
		{
			const AActor* Parent = Module->GetParentActor() ? Module->GetParentActor() : Module->GetAttachParentActor();
			if (const ASpaceStation* Station = Cast<ASpaceStation>(Parent))
			{
				const FText Name = ITargetable::Execute_GetTargetDisplayName(Station);
				if (!Name.IsEmpty())
				{
					return Name.ToString();
				}
			}
		}
		return Fallback.IsEmpty() ? FString(TEXT("Docking Bay")) : Fallback;
	}
}

void AAdastreaHUD::DrawCyberpunkFlightHUD(APlayerController* PC, ASpaceship* Ship)
{
	if (!Canvas || !PC || !Ship)
	{
		return;
	}
	UWorld* World = GetWorld();
	const float Dt = World ? World->GetDeltaSeconds() : 0.016f;
	const float SW = Canvas->ClipX;
	const float SH = Canvas->ClipY;
	const FNeon N{ Canvas, FMath::Clamp(SH / 1080.0f, 0.85f, 2.5f), World ? World->GetRealTimeSeconds() : 0.0f };
	const float S = N.S;

	UFont* Small = GEngine->GetSmallFont();
	UFont* Large = GEngine->GetLargeFont();

	// ---- Live data ----
	const FVector Loc = Ship->GetActorLocation();
	const FVector Vel = Ship->GetVelocity();
	const float Speed = Vel.Size();
	const float MaxSpeed = FMath::Max(Ship->GetEffectiveMaxSpeed(), 1.0f);
	const float Throttle = FMath::Clamp(Ship->ThrottlePercentage / 100.0f, 0.0f, 1.0f);
	const FRotator Rot = Ship->GetActorRotation();
	const float Heading = FMath::Fmod(Rot.Yaw + 360.0f, 360.0f);

	const float HullMax = FMath::Max(Ship->GetMaxHullIntegrity(), 1.0f);
	const float Hull = FMath::Clamp(Ship->GetCurrentHullIntegrity(), 0.0f, HullMax);
	const float HullFrac = Hull / HullMax;
	// Ships have no runtime shield pool yet: show the rated capacity from the data asset as full.
	const float ShieldMax = Ship->ShipDataAsset ? Ship->ShipDataAsset->ShieldStrength : 0.0f;
	const float Shield = ShieldMax;
	const float ShieldFrac = ShieldMax > 0.0f ? Shield / ShieldMax : 0.0f;
	const float Armor = Ship->ShipDataAsset ? Ship->ShipDataAsset->ArmorRating : 0.0f;

	// Damage flash: trips when a value drops between frames, then fades out.
	if (LastHudHull >= 0.0f && Hull < LastHudHull - KINDA_SMALL_NUMBER) { HullHitFlash = 0.4f; }
	if (LastHudShield >= 0.0f && Shield < LastHudShield - KINDA_SMALL_NUMBER) { ShieldHitFlash = 0.4f; }
	LastHudHull = Hull;
	LastHudShield = Shield;
	HullHitFlash = FMath::Max(HullHitFlash - Dt, 0.0f);
	ShieldHitFlash = FMath::Max(ShieldHitFlash - Dt, 0.0f);

	const bool bCritical = HullFrac < 0.25f;
	const float AlarmPulse = 0.5f + 0.5f * FMath::Sin(N.Now * 10.0f);

	// ---- Screen frame: corner brackets (red + pulsing edge when hull is critical) ----
	{
		const FLinearColor FrameCol = bCritical ? WithAlpha(kRed, 0.4f + 0.5f * AlarmPulse) : WithAlpha(kCyan, 0.35f);
		const float M = 14.0f * S, L = 46.0f * S;
		auto Corner = [&](float X, float Y, float DX, float DY)
		{
			N.Line({ X, Y }, { X + DX * L, Y }, FrameCol, 2.0f);
			N.Line({ X, Y }, { X, Y + DY * L }, FrameCol, 2.0f);
		};
		Corner(M, M, 1, 1); Corner(SW - M, M, -1, 1); Corner(M, SH - M, 1, -1); Corner(SW - M, SH - M, -1, -1);
		if (bCritical || HullHitFlash > 0.0f)
		{
			const float A = FMath::Max(bCritical ? 0.10f * AlarmPulse : 0.0f, HullHitFlash * 0.5f);
			const float E = 22.0f * S;
			N.Rect(0, 0, SW, E, WithAlpha(kRed, A)); N.Rect(0, SH - E, SW, E, WithAlpha(kRed, A));
			N.Rect(0, E, E, SH - 2 * E, WithAlpha(kRed, A)); N.Rect(SW - E, E, E, SH - 2 * E, WithAlpha(kRed, A));
		}
	}

	// ---- Heading tape (top centre) ----
	{
		const float TW = 560.0f * S, TH = 30.0f * S;
		const float TX = (SW - TW) * 0.5f, TY = 22.0f * S;
		const float Cx = SW * 0.5f;
		const float Span = 60.0f; // degrees either side of centre
		const float PxPerDeg = (TW * 0.5f - 12.0f * S) / Span;

		N.Panel(TX, TY, TW, TH, kCyan);
		auto Wrap = [](float D) { return FMath::Fmod(D + 540.0f, 360.0f) - 180.0f; }; // -> [-180,180)

		for (int32 Deg = 0; Deg < 360; Deg += 5)
		{
			const float Off = Wrap(Deg - Heading);
			if (FMath::Abs(Off) > Span) { continue; }
			const float X = Cx + Off * PxPerDeg;
			const float Fade = 1.0f - FMath::Abs(Off) / Span * 0.7f;
			const bool bMajor = Deg % 30 == 0;
			const float Tick = (bMajor ? 10.0f : (Deg % 10 == 0 ? 6.0f : 3.0f)) * S;
			N.Line({ X, TY + TH - Tick }, { X, TY + TH }, WithAlpha(kCyan, 0.8f * Fade), 1.0f);
			if (bMajor)
			{
				const TCHAR* Cardinal = Deg == 0 ? TEXT("N") : Deg == 90 ? TEXT("E") : Deg == 180 ? TEXT("S") : Deg == 270 ? TEXT("W") : nullptr;
				const FString Label = Cardinal ? FString(Cardinal) : FString::Printf(TEXT("%02d"), Deg / 10);
				float LW = 0.0f, LH = 0.0f;
				N.TextSize(Small, Label, 0.8f, LW, LH);
				N.Text(Label, WithAlpha(Cardinal ? kYellow : kInk, Fade), X - LW * 0.5f, TY + 3.0f * S, Small, 0.8f);
			}
		}

		// Station bearings: yellow diamonds on the tape, clamped chevrons at the edges when off-tape.
		TArray<AActor*> Stations;
		UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), Stations);
		for (AActor* St : Stations)
		{
			if (!St) { continue; }
			const FVector D = St->GetActorLocation() - Loc;
			if (D.SizeSquared2D() < 1.0f) { continue; }
			const float Off = Wrap(FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)) - Heading);
			const float Clamped = FMath::Clamp(Off, -Span, Span);
			const float X = Cx + Clamped * PxPerDeg;
			const float Y = TY + TH + 7.0f * S;
			const float R = 4.0f * S;
			if (FMath::Abs(Off) <= Span)
			{
				N.Quad({ X, Y - R }, { X + R, Y }, { X, Y + R }, { X - R, Y }, kYellow);
			}
			else
			{
				const float Dir = Off > 0.0f ? 1.0f : -1.0f;
				N.Line({ X - Dir * R, Y - R }, { X, Y }, WithAlpha(kYellow, 0.6f), 1.5f);
				N.Line({ X - Dir * R, Y + R }, { X, Y }, WithAlpha(kYellow, 0.6f), 1.5f);
			}
		}

		// Centre caret + heading / pitch readouts
		const float CY = TY + TH;
		N.Quad({ Cx - 7.0f * S, CY + 14.0f * S }, { Cx, CY + 2.0f * S }, { Cx + 7.0f * S, CY + 14.0f * S }, { Cx, CY + 10.0f * S }, kMagenta);
		N.GlowLine({ Cx, TY }, { Cx, TY + TH }, kMagenta, 1.5f);
		const FString Hdg = FString::Printf(TEXT("%03.0f"), Heading);
		float HW = 0.0f, HH = 0.0f;
		N.TextSize(Large, Hdg, 0.9f, HW, HH);
		N.GlitchText(Hdg, kInk, Cx - HW * 0.5f, CY + 16.0f * S, Large, 0.9f, 1.0f);
		N.Text(TEXT("HDG"), kMute, TX, CY + 6.0f * S, Small, 0.75f);
		const FString Pit = FString::Printf(TEXT("PITCH %+03.0f"), Rot.Pitch);
		float PW = 0.0f, PH = 0.0f;
		N.TextSize(Small, Pit, 0.75f, PW, PH);
		N.Text(Pit, kMute, TX + TW - PW, CY + 6.0f * S, Small, 0.75f);
	}

	// ---- Boresight (where the nose points) + flight-path marker (where the ship is going) ----
	{
		// Chase camera sits behind/above the ship, so project the nose direction rather than using screen centre.
		FVector2D C(SW * 0.5f, SH * 0.5f);
		PC->ProjectWorldLocationToScreen(Loc + Ship->GetActorForwardVector() * 500000.0f, C);
		const float G = 10.0f * S, L = 9.0f * S;
		const FLinearColor Bore = WithAlpha(kCyan, 0.55f);
		N.Line({ C.X - G - L, C.Y }, { C.X - G, C.Y }, Bore, 1.5f);
		N.Line({ C.X + G, C.Y }, { C.X + G + L, C.Y }, Bore, 1.5f);
		N.Line({ C.X, C.Y + G }, { C.X, C.Y + G + L * 0.6f }, Bore, 1.5f);

		// Where the ship is actually travelling (drift shows up as the marker sliding off the boresight).
		FVector2D FP;
		if (Speed > 50.0f && PC->ProjectWorldLocationToScreen(Loc + Vel.GetSafeNormal() * 500000.0f, FP)
			&& FP.X > 0.0f && FP.X < SW && FP.Y > 0.0f && FP.Y < SH)
		{
			const float R = 6.0f * S;
			const int32 Segs = 16;
			for (int32 i = 0; i < Segs; ++i)
			{
				const float A0 = 2.0f * PI * i / Segs, A1 = 2.0f * PI * (i + 1) / Segs;
				N.Line(FP + FVector2D(FMath::Cos(A0), FMath::Sin(A0)) * R, FP + FVector2D(FMath::Cos(A1), FMath::Sin(A1)) * R, kMagenta, 1.5f);
			}
			N.Line({ FP.X - R - 10.0f * S, FP.Y }, { FP.X - R, FP.Y }, kMagenta, 1.5f);
			N.Line({ FP.X + R, FP.Y }, { FP.X + R + 10.0f * S, FP.Y }, kMagenta, 1.5f);
			N.Line({ FP.X, FP.Y - R }, { FP.X, FP.Y - R - 7.0f * S }, kMagenta, 1.5f);
		}
	}

	// ---- Ident strip (top-left) ----
	float IdentBottom = 0.0f;
	{
		const float X = 24.0f * S, Y = 24.0f * S, W = 300.0f * S, H = 104.0f * S;
		N.Panel(X, Y, W, H, kMagenta);
		const float LX = X + 16.0f * S;
		N.Text(TEXT("ADASTREA::NAV_OS  v2.0.77"), WithAlpha(kMagenta, 0.8f), LX, Y + 6.0f * S, Small, 0.65f);
		N.GlitchText(Ship->GetShipName().ToString().ToUpper(), kInk, LX, Y + 20.0f * S, Large, 0.75f, 2.0f);

		const int64 Credits = Ship->PlayerTraderComponent ? static_cast<int64>(Ship->PlayerTraderComponent->GetCredits()) : 0;
		N.Text(TEXT("CR"), kMute, LX, Y + 46.0f * S, Small, 0.75f);
		N.Text(FString::Printf(TEXT("%lld"), Credits), kYellow, LX + 26.0f * S, Y + 44.0f * S, Small, 0.95f);

		if (UCargoComponent* Cargo = Ship->CargoComponent)
		{
			const float CargoMax = FMath::Max(Cargo->CargoCapacity, 0.01f);
			const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
			N.Text(TEXT("HOLD"), kMute, LX + 130.0f * S, Y + 46.0f * S, Small, 0.75f);
			N.Text(FString::Printf(TEXT("%.0f/%.0f"), Used, CargoMax), kInk, LX + 170.0f * S, Y + 46.0f * S, Small, 0.75f);
			N.SegBar(LX + 130.0f * S, Y + 64.0f * S, 140.0f * S, 5.0f * S, Used / CargoMax, 14, kCyan, kYellow);
		}

		N.Text(FString::Printf(TEXT("X %+08.0f  Y %+08.0f  Z %+08.0f"), Loc.X / 100.0f, Loc.Y / 100.0f, Loc.Z / 100.0f),
			kMute, LX, Y + H - 22.0f * S, Small, 0.65f);
		IdentBottom = Y + H;
	}

	// ---- Defence grid (bottom-left): shield + hull ----
	{
		const float W = 400.0f * S, H = 130.0f * S;
		const float X = 24.0f * S, Y = SH - H - 24.0f * S;
		const FLinearColor Edge = HullHitFlash > 0.0f ? kRed : (bCritical ? FLinearColor::LerpUsingHSV(kMagenta, kRed, AlarmPulse) : kMagenta);
		N.Panel(X, Y, W, H, Edge);
		const float LX = X + 16.0f * S;
		N.GlitchText(TEXT("// DEFENCE GRID"), kMagenta, LX, Y + 8.0f * S, Small, 0.9f, 3.0f);

		auto Row = [&](float RY, const TCHAR* Label, float Frac, float Cur, float Max,
			const FLinearColor& From, const FLinearColor& To, bool bFlash, bool bOffline)
		{
			N.Text(Label, kMute, LX, RY + 2.0f * S, Small, 0.8f);
			const float BX = LX + 52.0f * S, BW = W - 150.0f * S, BH = 14.0f * S;
			const float Alpha = bFlash ? 0.5f + 0.5f * AlarmPulse : 1.0f;
			N.SegBar(BX, RY + 2.0f * S, BW, BH, bOffline ? 0.0f : Frac, 20, From, To, Alpha);
			const FString Val = bOffline ? FString(TEXT("OFFLINE")) : FString::Printf(TEXT("%3.0f%%"), Frac * 100.0f);
			N.Text(Val, bOffline ? kMute : To, BX + BW + 10.0f * S, RY - 2.0f * S, Large, 0.8f);
			if (!bOffline)
			{
				N.Text(FString::Printf(TEXT("%.0f / %.0f"), Cur, Max), WithAlpha(kMute, 0.8f), BX, RY + BH + 5.0f * S, Small, 0.6f);
			}
		};

		const FLinearColor ShieldTo = ShieldHitFlash > 0.0f ? FLinearColor::White : kCyan;
		Row(Y + 36.0f * S, TEXT("SHLD"), ShieldFrac, Shield, ShieldMax, FLinearColor(0.1f, 0.4f, 1.0f, 1.0f), ShieldTo, false, ShieldMax <= 0.0f);

		const FLinearColor HullCol = HullFrac > 0.5f ? kMagenta : (HullFrac > 0.25f ? kYellow : kRed);
		Row(Y + 76.0f * S, TEXT("HULL"), HullFrac, Hull, HullMax, WithAlpha(kRed, 1.0f), HullCol, bCritical, false);

		const TCHAR* Status = bCritical ? TEXT("HULL CRITICAL") : (HullFrac < 0.6f ? TEXT("DAMAGED") : TEXT("NOMINAL"));
		const FString Foot = FString::Printf(TEXT("ARMOR %.0f   //   %s"), Armor, Status);
		N.Text(Foot, bCritical ? WithAlpha(kRed, 0.5f + 0.5f * AlarmPulse) : kMute, LX, Y + H - 18.0f * S, Small, 0.65f);
	}

	// ---- Propulsion (bottom-right): speed, speed bar, throttle, mode tags ----
	{
		const float W = 400.0f * S, H = 130.0f * S;
		const float X = SW - W - 24.0f * S, Y = SH - H - 24.0f * S;
		N.Panel(X, Y, W, H, kCyan, false);
		const float LX = X + 16.0f * S;
		const float RX = X + W - 16.0f * S;

		const FString Head = TEXT("PROPULSION //");
		float TW = 0.0f, TH = 0.0f;
		N.TextSize(Small, Head, 0.9f, TW, TH);
		N.GlitchText(Head, kCyan, RX - TW, Y + 8.0f * S, Small, 0.9f, 4.0f);

		// Big speed readout in m/s (world units are cm)
		const FString Spd = FString::Printf(TEXT("%04d"), FMath::RoundToInt(Speed / 100.0f));
		N.GlitchText(Spd, kInk, LX, Y + 6.0f * S, Large, 2.0f, 5.0f);
		float SpW = 0.0f, SpH = 0.0f;
		N.TextSize(Large, Spd, 2.0f, SpW, SpH);
		N.Text(TEXT("M/S"), kCyan, LX + SpW + 6.0f * S, Y + 16.0f * S, Small, 0.8f);
		N.Text(FString::Printf(TEXT("MAX %d"), FMath::RoundToInt(MaxSpeed / 100.0f)), kMute, LX + SpW + 6.0f * S, Y + 32.0f * S, Small, 0.65f);

		// Speed vs. effective max, then throttle setting
		const float BX = LX + 44.0f * S, BW = W - 72.0f * S;
		N.Text(TEXT("SPD"), kMute, LX, Y + 62.0f * S, Small, 0.7f);
		N.SegBar(BX, Y + 64.0f * S, BW, 7.0f * S, Speed / MaxSpeed, 30, kCyan, kCyan);
		N.Text(TEXT("THR"), kMute, LX, Y + 78.0f * S, Small, 0.7f);
		N.SegBar(BX, Y + 80.0f * S, BW, 11.0f * S, Throttle, 30, kCyan, Ship->bBoostActive ? kYellow : kMagenta);
		const FString Thr = FString::Printf(TEXT("%3.0f%%"), Throttle * 100.0f);
		float ThW = 0.0f, ThH = 0.0f;
		N.TextSize(Small, Thr, 0.7f, ThW, ThH);
		N.Text(Thr, kInk, RX - ThW, Y + 94.0f * S, Small, 0.7f);

		// Mode tags
		float TagX = LX;
		const float TagY = Y + H - 24.0f * S;
		TagX += N.Tag(TEXT("FA"), TagX, TagY, kCyan, Ship->bFlightAssistEnabled, Small);
		TagX += N.Tag(TEXT("BOOST"), TagX, TagY, kYellow, Ship->bBoostActive, Small);
		TagX += N.Tag(TEXT("CRUISE"), TagX, TagY, kMagenta, Ship->bTravelModeActive, Small);
	}

	// ---- Docking prompt (lower centre; only while RequestDocking would succeed) ----
	float DockDist = 0.0f;
	FString DockName;
	if (Ship->CanRequestDocking(DockDist, DockName))
	{
		const FString Station = DockDisplayName(Ship, DockName).Replace(TEXT("_"), TEXT(" ")).ToUpper();
		const FString Dist = FString::Printf(TEXT("%.0f M"), DockDist / 100.0f);
		float NW = 0.0f, NH = 0.0f, DW = 0.0f, DH = 0.0f;
		N.TextSize(Large, Station, 0.95f, NW, NH);
		N.TextSize(Large, Dist, 0.8f, DW, DH);

		const float W = FMath::Max(NW + DW + 150.0f * S, 440.0f * S), H = 74.0f * S;
		const float X = (SW - W) * 0.5f, Y = SH * 0.66f;
		N.Panel(X, Y, W, H, kCyan);

		// Pulsing chevrons either side
		for (int32 i = 0; i < 3; ++i)
		{
			const float A = 0.25f + 0.75f * FMath::Clamp(FMath::Sin(N.Now * 6.0f - i * 0.9f), 0.0f, 1.0f);
			const float CX = 12.0f * S * i, CY = Y + H * 0.5f, R = 7.0f * S;
			N.Line({ X - 30.0f * S + CX, CY - R }, { X - 22.0f * S + CX, CY }, WithAlpha(kCyan, A), 2.0f);
			N.Line({ X - 30.0f * S + CX, CY + R }, { X - 22.0f * S + CX, CY }, WithAlpha(kCyan, A), 2.0f);
			N.Line({ X + W + 30.0f * S - CX, CY - R }, { X + W + 22.0f * S - CX, CY }, WithAlpha(kCyan, A), 2.0f);
			N.Line({ X + W + 30.0f * S - CX, CY + R }, { X + W + 22.0f * S - CX, CY }, WithAlpha(kCyan, A), 2.0f);
		}

		const float LX = X + 18.0f * S;
		N.GlitchText(TEXT("DOCKING CLEARANCE // AVAILABLE"), kMagenta, LX, Y + 7.0f * S, Small, 0.75f, 6.0f);
		const float KeyW = N.KeyHint(TEXT("E"), TEXT(""), LX, Y + 30.0f * S, kCyan, Large);
		N.GlitchText(Station, kInk, LX + KeyW + 6.0f * S, Y + 30.0f * S, Large, 0.95f, 7.0f);
		N.Text(Dist, kCyan, X + W - DW - 18.0f * S, Y + 32.0f * S, Large, 0.8f);

		// Approach bar: fills as you close in on the docking point
		const float Range = FMath::Max(Ship->GetEffectiveDockingRange(), 1.0f);
		N.SegBar(LX, Y + H - 12.0f * S, W - 36.0f * S, 4.0f * S, 1.0f - DockDist / Range, 32, kMagenta, kCyan);
	}

	// ---- Mining controls hint (bottom centre; while the laser is fitted and nothing is locked) ----
	if (const UMiningLaserComponent* Laser = Ship->MiningLaser)
	{
		if (Laser->bMiningEnabled && !Laser->GetTarget())
		{
			const float Y = SH - 58.0f * S;
			float Hx = SW * 0.5f - 170.0f * S;
			N.Text(TEXT("MINING LASER //"), WithAlpha(kYellow, 0.9f), Hx - 10.0f * S, Y - 20.0f * S, Small, 0.7f);
			Hx += N.KeyHint(TEXT("T"), TEXT("LOCK ASTEROID"), Hx, Y, kYellow, Small) + 26.0f * S;
			N.KeyHint(TEXT("LMB"), TEXT("HOLD TO FIRE"), Hx, Y, kYellow, Small);
		}
	}

	// Mining panel (hidden unless an asteroid is locked) stacks under the ident strip.
	DrawMiningHUD(PC, Ship, IdentBottom + 12.0f * S);
}

void AAdastreaHUD::DrawCyberMiningHUD(APlayerController* PC, ASpaceship* Ship, UMiningLaserComponent* Laser, AAsteroid* Rock, float PanelTop)
{
	if (!Canvas || !PC || !Ship || !Laser || !Rock)
	{
		return;
	}
	UWorld* World = GetWorld();
	const FNeon N{ Canvas, FMath::Clamp(Canvas->ClipY / 1080.0f, 0.85f, 2.5f), World ? World->GetRealTimeSeconds() : 0.0f };
	const float S = N.S;
	UFont* Small = GEngine->GetSmallFont();
	UFont* Large = GEngine->GetLargeFont();

	UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	UTradeItemDataAsset* Ore = Laser->GetTargetOre();
	FLinearColor Tint = Type ? Type->OreTint : kCyan;
	Tint.A = 1.0f;
	const EMiningStatus Status = Laser->GetStatus();
	const bool bFiring = Status == EMiningStatus::Mining;
	const bool bInRange = Laser->IsTargetInRange();
	const float Dist = Laser->GetTargetSurfaceDistance();
	const FString RockName = Rock->GetTargetDisplayName_Implementation().ToString().ToUpper();

	// ---- Lock brackets around the asteroid (ore tint; yellow when out of range; pulse + orbiting ticks while firing) ----
	{
		FVector2D Centre, Edge;
		const FVector CamRight = PC->PlayerCameraManager ? FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetScaledAxis(EAxis::Y) : FVector::RightVector;
		if (PC->ProjectWorldLocationToScreen(Rock->GetActorLocation(), Centre) &&
			PC->ProjectWorldLocationToScreen(Rock->GetActorLocation() + CamRight * Rock->GetRadius(), Edge))
		{
			const float Pulse = bFiring ? 1.0f + 0.06f * FMath::Sin(N.Now * 12.0f) : 1.0f;
			const float R = FMath::Clamp(FVector2D::Distance(Centre, Edge) * 1.15f, 28.0f * S, 400.0f) * Pulse;
			const float L = FMath::Max(R * 0.3f, 10.0f * S);
			const float X0 = Centre.X - R, X1 = Centre.X + R, Y0 = Centre.Y - R, Y1 = Centre.Y + R;
			const FLinearColor BC = bInRange ? Tint : kYellow;
			const float T = bFiring ? 2.5f : 1.5f;
			N.GlowLine({ X0, Y0 }, { X0 + L, Y0 }, BC, T); N.GlowLine({ X0, Y0 }, { X0, Y0 + L }, BC, T);
			N.GlowLine({ X1, Y0 }, { X1 - L, Y0 }, BC, T); N.GlowLine({ X1, Y0 }, { X1, Y0 + L }, BC, T);
			N.GlowLine({ X0, Y1 }, { X0 + L, Y1 }, BC, T); N.GlowLine({ X0, Y1 }, { X0, Y1 - L }, BC, T);
			N.GlowLine({ X1, Y1 }, { X1 - L, Y1 }, BC, T); N.GlowLine({ X1, Y1 }, { X1, Y1 - L }, BC, T);

			if (bFiring)
			{
				// Four ticks orbiting the rock while the beam is on
				for (int32 i = 0; i < 4; ++i)
				{
					const float A = N.Now * 2.5f + i * HALF_PI;
					const FVector2D Dir(FMath::Cos(A), FMath::Sin(A));
					N.Line(Centre + Dir * R * 0.78f, Centre + Dir * R * 0.9f, WithAlpha(kGreen, 0.9f), 2.0f);
				}
			}

			// Tag at the bracket's top-right: name, distance, yield sliver
			const float TX = X1 + 8.0f * S, TY = Y0;
			N.Rect(TX, TY, 3.0f * S, 34.0f * S, BC);
			N.Text(RockName, BC, TX + 8.0f * S, TY, Small, 0.75f);
			N.Text(FString::Printf(TEXT("%.0f M%s"), Dist / 100.0f, bInRange ? TEXT("") : TEXT("  // OUT OF RANGE")),
				bInRange ? kInk : kYellow, TX + 8.0f * S, TY + 13.0f * S, Small, 0.7f);
			N.SegBar(TX + 8.0f * S, TY + 28.0f * S, 90.0f * S, 4.0f * S, Rock->GetOreFraction(), 10, Tint, Tint);
		}
	}

	// ---- Mining uplink panel (stacked under the ident strip) ----
	const float X = 24.0f * S, W = 300.0f * S, H = 212.0f * S;
	const float Y = PanelTop;
	N.Panel(X, Y, W, H, Tint);
	const float LX = X + 16.0f * S, RX = X + W - 16.0f * S, BW = W - 32.0f * S;

	auto RightText = [&](const FString& Str, const FLinearColor& Col, float RY, UFont* F, float Scale)
	{
		float TW = 0.0f, TH = 0.0f;
		N.TextSize(F, Str, Scale, TW, TH);
		N.Text(Str, Col, RX - TW, RY, F, Scale);
	};

	N.GlitchText(TEXT("// MINING UPLINK"), Tint, LX, Y + 8.0f * S, Small, 0.8f, 8.0f);
	RightText(TEXT("LOCKED"), kGreen, Y + 8.0f * S, Small, 0.7f);
	N.GlitchText(RockName, kInk, LX, Y + 22.0f * S, Large, 0.8f, 9.0f);

	// Ore + hardness
	N.Text(TEXT("ORE"), kMute, LX, Y + 46.0f * S, Small, 0.7f);
	N.Text(Ore ? Ore->ItemName.ToString().ToUpper() : FString(TEXT("NONE")), Ore ? Tint : kYellow, LX + 34.0f * S, Y + 46.0f * S, Small, 0.7f);
	RightText(FString::Printf(TEXT("HARDNESS %.1f"), Type ? Type->Hardness : 1.0f), kMute, Y + 46.0f * S, Small, 0.7f);

	// Remaining yield
	N.Text(TEXT("YIELD"), kMute, LX, Y + 64.0f * S, Small, 0.7f);
	RightText(FString::Printf(TEXT("%.0f / %.0f  (%.0f%%)"), Rock->GetRemainingOre(), Rock->GetTotalOre(), Rock->GetOreFraction() * 100.0f),
		kInk, Y + 64.0f * S, Small, 0.7f);
	N.SegBar(LX, Y + 79.0f * S, BW, 6.0f * S, Rock->GetOreFraction(), 24, Tint, Tint);

	// Range
	N.Text(TEXT("RANGE"), kMute, LX, Y + 94.0f * S, Small, 0.7f);
	N.Text(FString::Printf(TEXT("%.0f M  //  %s"), Dist / 100.0f, bInRange ? TEXT("IN RANGE") : TEXT("CLOSE IN")),
		bInRange ? kGreen : kYellow, LX + 48.0f * S, Y + 94.0f * S, Small, 0.7f);
	RightText(FString::Printf(TEXT("MAX %.0f"), Laser->Range / 100.0f), kMute, Y + 94.0f * S, Small, 0.7f);

	// Laser state: lit tag while the beam is on (or blocked), status text beside it
	{
		const bool bHeld = Laser->IsTriggerHeld();
		const FString Beam = bFiring ? TEXT("BEAM ON") : (bHeld ? TEXT("BLOCKED") : TEXT("BEAM OFF"));
		const FLinearColor BeamCol = bFiring ? kGreen : (bHeld ? kYellow : kMute);
		const float TagW = N.Tag(Beam, LX, Y + 112.0f * S, BeamCol, bFiring || bHeld, Small);
		N.Text(UMiningLaserComponent::StatusToText(Status).ToString().ToUpper(), bFiring ? kGreen : kInk, LX + TagW + 4.0f * S, Y + 114.0f * S, Small, 0.7f);
		if (bFiring)
		{
			const float Lamp = 0.4f + 0.6f * FMath::Abs(FMath::Sin(N.Now * 8.0f));
			N.Rect(RX - 10.0f * S, Y + 115.0f * S, 10.0f * S, 10.0f * S, WithAlpha(kGreen, Lamp));
		}
	}

	// Extraction progress toward the next whole unit
	N.Text(TEXT("EXTRACT"), kMute, LX, Y + 138.0f * S, Small, 0.7f);
	RightText(FString::Printf(TEXT("%.1f U/S   NEXT %.0f%%"), bFiring ? Laser->GetExtractionRate() : 0.0f, Laser->GetUnitProgress() * 100.0f),
		bFiring ? kGreen : kMute, Y + 138.0f * S, Small, 0.7f);
	N.SegBar(LX, Y + 153.0f * S, BW, 6.0f * S, Laser->GetUnitProgress(), 24, kCyan, kGreen, bFiring ? 1.0f : 0.5f);

	// Hold fill + ore in hold, with a "+N" pop after each delivery
	if (UCargoComponent* Cargo = Ship->CargoComponent)
	{
		const float Cap = FMath::Max(Cargo->CargoCapacity, 0.01f);
		const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
		const float Frac = Used / Cap;
		const bool bFull = Frac > 0.9f;
		N.Text(TEXT("HOLD"), kMute, LX, Y + 168.0f * S, Small, 0.7f);
		RightText(FString::Printf(TEXT("%.0f / %.0f%s"), Used, Cap, bFull ? TEXT("  // FULL") : TEXT("")), bFull ? kRed : kCyan, Y + 168.0f * S, Small, 0.7f);
		N.SegBar(LX, Y + 183.0f * S, BW, 6.0f * S, Frac, 24, kCyan, bFull ? kRed : kMagenta);
		N.Text(Ore ? FString::Printf(TEXT("IN HOLD  %s x%d"), *Ore->ItemName.ToString().ToUpper(), Cargo->GetItemQuantity(Ore)) : FString(TEXT("IN HOLD  -")),
			kMute, LX, Y + 193.0f * S, Small, 0.65f);

		const float Since = Laser->GetSecondsSinceLastMined();
		if (Laser->GetLastMinedOre() && Since < 1.5f)
		{
			const FString Plus = FString::Printf(TEXT("+%d"), Laser->GetLastMinedAmount());
			float PW = 0.0f, PH = 0.0f;
			N.TextSize(Large, Plus, 0.9f, PW, PH);
			N.GlitchText(Plus, WithAlpha(kGreen, FMath::Clamp(1.0f - Since / 1.5f, 0.0f, 1.0f)),
				RX - PW, Y + 188.0f * S - Since * 14.0f * S, Large, 0.9f, 10.0f);
		}
	}
	else
	{
		N.Text(TEXT("HOLD  // NO CARGO BAY"), kYellow, LX, Y + 168.0f * S, Small, 0.7f);
	}
}
