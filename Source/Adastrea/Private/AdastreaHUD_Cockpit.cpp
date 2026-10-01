// Copyright Epic Games, Inc. All Rights Reserved.

// Neon cockpit flight HUD for AAdastreaHUD (canvas-drawn, same path as the rest of the HUD).
//
// Layout (authored at 1920x1080 and scaled by HudType::LayoutScale; text sizes come from the
// shared type ramp in AdastreaHUDStyle.h):
//   top-centre    heading tape (N/E/S/W, station bearings, pitch)
//   centre        boresight + flight-path marker (where the ship is actually going)
//   top-left      ident strip: ship, credits, cargo, coords (mining panel stacks below)
//   bottom-left   DEFENCE GRID: segmented shield + hull bars, hit flash, low-hull alarm
//   bottom-right  PROPULSION: big speed readout, speed/throttle bars, FA/BOOST/CRUISE tags

#include "AdastreaHUD.h"
#include "Combat/ShipHealthComponent.h"
#include "AdastreaHUDStyle.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Drones/DroneBayComponent.h"
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

		void TextSize(const FString& T, float Scale, float& OutW, float& OutH) const
		{
			C->TextSize(HudType::Font(), T, OutW, OutH, Scale * S, Scale * S);
		}

		/** Scale is a HudType role (Caption, Label, ...); the viewport scale is applied here. */
		void Text(const FString& T, const FLinearColor& Col, float X, float Y, float Scale) const
		{
			FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(T), HudType::Font(), Col);
			Item.Scale = FVector2D(Scale * S);
			Item.BlendMode = SE_BLEND_Translucent;
			C->DrawItem(Item);
		}

		/** Text with a magenta/cyan chromatic split; occasionally jitters sideways (glitch). */
		void GlitchText(const FString& T, const FLinearColor& Col, float X, float Y, float Scale, float Seed = 0.0f) const
		{
			const float Slot = FMath::FloorToFloat(Now * 9.0f) + Seed * 31.0f;
			const bool bGlitch = Hash01(Slot) > 0.94f;
			const float Jx = bGlitch ? (Hash01(Slot + 1.7f) - 0.5f) * 10.0f * S : 0.0f;
			const float Split = (bGlitch ? 3.0f : 1.2f) * S;
			Text(T, WithAlpha(kMagenta, 0.55f), X + Jx - Split, Y, Scale);
			Text(T, WithAlpha(kCyan, 0.55f), X + Jx + Split, Y, Scale);
			Text(T, Col, X + Jx, Y, Scale);
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
		float Tag(const FString& T, float X, float Y, const FLinearColor& Col, bool bOn) const
		{
			float TW = 0.0f, TH = 0.0f;
			TextSize(T, HudType::Label, TW, TH);
			const float W = TW + 14.0f * S, H = TH + 4.0f * S;
			const float K = 5.0f * S;
			if (bOn)
			{
				Poly({ { X + K, Y }, { X + W, Y }, { X + W - K, Y + H }, { X, Y + H } }, WithAlpha(Col, 0.85f));
				Text(T, FLinearColor(0.02f, 0.0f, 0.05f, 1.0f), X + 7.0f * S, Y + 2.0f * S, HudType::Label);
			}
			else
			{
				const FLinearColor Dim = WithAlpha(Col, 0.3f);
				Line({ X + K, Y }, { X + W, Y }, Dim); Line({ X + W, Y }, { X + W - K, Y + H }, Dim);
				Line({ X + W - K, Y + H }, { X, Y + H }, Dim); Line({ X, Y + H }, { X + K, Y }, Dim);
				Text(T, Dim, X + 7.0f * S, Y + 2.0f * S, HudType::Label);
			}
			return W + 6.0f * S;
		}

		/** Filled key-cap ("E", "LMB") with a glow, followed by a label. Returns total width. */
		float KeyHint(const FString& Key, const FString& Label, float X, float Y, const FLinearColor& Col) const
		{
			float KW = 0.0f, KH = 0.0f;
			TextSize(Key, HudType::Body, KW, KH);
			const float W = FMath::Max(KW + 12.0f * S, KH + 6.0f * S), H = KH + 6.0f * S;
			Rect(X - 2.0f * S, Y - 2.0f * S, W + 4.0f * S, H + 4.0f * S, WithAlpha(Col, 0.2f));
			Rect(X, Y, W, H, Col);
			Text(Key, FLinearColor(0.02f, 0.0f, 0.05f, 1.0f), X + (W - KW) * 0.5f, Y + 3.0f * S, HudType::Body);
			float LW = 0.0f, LH = 0.0f;
			TextSize(Label, HudType::Label, LW, LH);
			Text(Label, kInk, X + W + 8.0f * S, Y + (H - LH) * 0.5f, HudType::Label);
			return W + 8.0f * S + LW;
		}
	};

	/** Applies a panel's layout-editor offset/scale to everything drawn in its scope. */
	struct FHudElementScope
	{
		AAdastreaHUD& HUD;
		FHudElementScope(AAdastreaHUD& InHUD, EHudElement Element) : HUD(InHUD) { HUD.BeginHudElement(Element); }
		~FHudElementScope() { HUD.EndHudElement(); }
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
	const FNeon N{ Canvas, HudType::LayoutScale(SW, SH), World ? World->GetRealTimeSeconds() : 0.0f };
	const float S = N.S;

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
	const UShipHealthComponent* Health = Ship->HealthComponent;
	const float ShieldMax = Health ? Health->GetMaxShield() : Ship->GetMaxShieldStrength();
	const float Shield = Health ? FMath::Clamp(Health->GetShield(), 0.0f, ShieldMax) : ShieldMax;
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
		const FHudElementScope Layout(*this, EHudElement::HeadingTape);
		const float TW = 460.0f * S, TH = 26.0f * S;
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
				N.TextSize(Label, HudType::Label, LW, LH);
				N.Text(Label, WithAlpha(Cardinal ? kYellow : kInk, Fade), X - LW * 0.5f, TY + 3.0f * S, HudType::Label);
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
		N.TextSize(Hdg, HudType::Heading, HW, HH);
		N.GlitchText(Hdg, kInk, Cx - HW * 0.5f, CY + 14.0f * S, HudType::Heading, 1.0f);
		N.Text(TEXT("HDG"), kMute, TX, CY + 16.0f * S, HudType::Label);
		const FString Pit = FString::Printf(TEXT("PITCH %+03.0f"), Rot.Pitch);
		float PW = 0.0f, PH = 0.0f;
		N.TextSize(Pit, HudType::Label, PW, PH);
		N.Text(Pit, kMute, TX + TW - PW, CY + 16.0f * S, HudType::Label);
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
		const FHudElementScope Layout(*this, EHudElement::Ident);
		const float X = 24.0f * S, Y = 24.0f * S, W = 270.0f * S, H = 90.0f * S;
		N.Panel(X, Y, W, H, kMagenta);
		const float LX = X + 14.0f * S;
		N.Text(TEXT("ADASTREA::NAV_OS  v2.0.77"), WithAlpha(kMagenta, 0.8f), LX, Y + 5.0f * S, HudType::Caption);
		N.GlitchText(Ship->GetShipName().ToString().ToUpper(), kInk, LX, Y + 19.0f * S, HudType::Body, 2.0f);

		const int64 Credits = Ship->PlayerTraderComponent ? static_cast<int64>(Ship->PlayerTraderComponent->GetCredits()) : 0;
		N.Text(TEXT("CR"), kMute, LX, Y + 42.0f * S, HudType::Label);
		N.Text(FString::Printf(TEXT("%lld"), Credits), kYellow, LX + 24.0f * S, Y + 41.0f * S, HudType::Body);

		if (UCargoComponent* Cargo = Ship->CargoComponent)
		{
			const float CargoMax = FMath::Max(Cargo->CargoCapacity, 0.01f);
			const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
			N.Text(TEXT("HOLD"), kMute, LX + 118.0f * S, Y + 42.0f * S, HudType::Label);
			N.Text(FString::Printf(TEXT("%.0f/%.0f"), Used, CargoMax), kInk, LX + 156.0f * S, Y + 42.0f * S, HudType::Label);
			N.SegBar(LX + 118.0f * S, Y + 59.0f * S, 112.0f * S, 4.0f * S, Used / CargoMax, 14, kCyan, kYellow);
		}

		N.Text(FString::Printf(TEXT("X %+08.0f  Y %+08.0f  Z %+08.0f"), Loc.X / 100.0f, Loc.Y / 100.0f, Loc.Z / 100.0f),
			kMute, LX, Y + H - 18.0f * S, HudType::Caption);
		IdentBottom = Y + H;
	}

	// ---- Defence grid (bottom-left): shield + hull ----
	{
		const FHudElementScope Layout(*this, EHudElement::Defence);
		const float W = 330.0f * S, H = 112.0f * S;
		const float X = 24.0f * S, Y = SH - H - 24.0f * S;
		const FLinearColor Edge = HullHitFlash > 0.0f ? kRed : (bCritical ? FLinearColor::LerpUsingHSV(kMagenta, kRed, AlarmPulse) : kMagenta);
		N.Panel(X, Y, W, H, Edge);
		const float LX = X + 14.0f * S;
		N.GlitchText(TEXT("// DEFENCE GRID"), kMagenta, LX, Y + 6.0f * S, HudType::Body, 3.0f);

		auto Row = [&](float RY, const TCHAR* Label, float Frac, float Cur, float Max,
			const FLinearColor& From, const FLinearColor& To, bool bFlash, bool bOffline)
		{
			N.Text(Label, kMute, LX, RY, HudType::Label);
			const float BX = LX + 44.0f * S, BW = W - 124.0f * S, BH = 11.0f * S;
			const float Alpha = bFlash ? 0.5f + 0.5f * AlarmPulse : 1.0f;
			N.SegBar(BX, RY + 1.0f * S, BW, BH, bOffline ? 0.0f : Frac, 20, From, To, Alpha);
			const FString Val = bOffline ? FString(TEXT("OFFLINE")) : FString::Printf(TEXT("%3.0f%%"), Frac * 100.0f);
			N.Text(Val, bOffline ? kMute : To, BX + BW + 8.0f * S, RY - 1.0f * S, HudType::Body);
			if (!bOffline)
			{
				N.Text(FString::Printf(TEXT("%.0f / %.0f"), Cur, Max), WithAlpha(kMute, 0.8f), BX, RY + BH + 3.0f * S, HudType::Caption);
			}
		};

		const FLinearColor ShieldTo = ShieldHitFlash > 0.0f ? FLinearColor::White : kCyan;
		Row(Y + 30.0f * S, TEXT("SHLD"), ShieldFrac, Shield, ShieldMax, FLinearColor(0.1f, 0.4f, 1.0f, 1.0f), ShieldTo, false, ShieldMax <= 0.0f);

		const FLinearColor HullCol = HullFrac > 0.5f ? kMagenta : (HullFrac > 0.25f ? kYellow : kRed);
		Row(Y + 62.0f * S, TEXT("HULL"), HullFrac, Hull, HullMax, WithAlpha(kRed, 1.0f), HullCol, bCritical, false);

		const TCHAR* Status = bCritical ? TEXT("HULL CRITICAL") : (HullFrac < 0.6f ? TEXT("DAMAGED") : TEXT("NOMINAL"));
		const FString Foot = FString::Printf(TEXT("ARMOR %.0f   //   %s"), Armor, Status);
		N.Text(Foot, bCritical ? WithAlpha(kRed, 0.5f + 0.5f * AlarmPulse) : kMute, LX, Y + H - 17.0f * S, HudType::Caption);
	}

	// ---- Propulsion (bottom-right): speed, speed bar, throttle, mode tags ----
	{
		const FHudElementScope Layout(*this, EHudElement::Propulsion);
		const float W = 330.0f * S, H = 112.0f * S;
		const float X = SW - W - 24.0f * S, Y = SH - H - 24.0f * S;
		N.Panel(X, Y, W, H, kCyan, false);
		const float LX = X + 14.0f * S;
		const float RX = X + W - 14.0f * S;

		const FString Head = TEXT("PROPULSION //");
		float TW = 0.0f, TH = 0.0f;
		N.TextSize(Head, HudType::Body, TW, TH);
		N.GlitchText(Head, kCyan, RX - TW, Y + 6.0f * S, HudType::Body, 4.0f);

		// Big speed readout in m/s (world units are cm)
		const FString Spd = FString::Printf(TEXT("%04d"), FMath::RoundToInt(Speed / 100.0f));
		N.GlitchText(Spd, kInk, LX, Y + 2.0f * S, HudType::Display, 5.0f);
		float SpW = 0.0f, SpH = 0.0f;
		N.TextSize(Spd, HudType::Display, SpW, SpH);
		N.Text(TEXT("M/S"), kCyan, LX + SpW + 6.0f * S, Y + 10.0f * S, HudType::Label);
		N.Text(FString::Printf(TEXT("MAX %d"), FMath::RoundToInt(MaxSpeed / 100.0f)), kMute, LX + SpW + 6.0f * S, Y + 25.0f * S, HudType::Caption);

		// Speed vs. effective max, then throttle setting
		const float BX = LX + 36.0f * S, BW = W - 64.0f * S;
		N.Text(TEXT("SPD"), kMute, LX, Y + 45.0f * S, HudType::Label);
		N.SegBar(BX, Y + 48.0f * S, BW, 6.0f * S, Speed / MaxSpeed, 30, kCyan, kCyan);
		N.Text(TEXT("THR"), kMute, LX, Y + 61.0f * S, HudType::Label);
		N.SegBar(BX, Y + 63.0f * S, BW, 10.0f * S, Throttle, 30, kCyan, Ship->bBoostActive ? kYellow : kMagenta);
		const FString Thr = FString::Printf(TEXT("%3.0f%%"), Throttle * 100.0f);
		float ThW = 0.0f, ThH = 0.0f;
		N.TextSize(Thr, HudType::Label, ThW, ThH);
		N.Text(Thr, kInk, RX - ThW, Y + 77.0f * S, HudType::Label);

		// Mode tags
		float TagX = LX;
		const float TagY = Y + H - 23.0f * S;
		TagX += N.Tag(TEXT("FA"), TagX, TagY, kCyan, Ship->bFlightAssistEnabled);
		TagX += N.Tag(TEXT("BOOST"), TagX, TagY, kYellow, Ship->bBoostActive);
		TagX += N.Tag(TEXT("CRUISE"), TagX, TagY, kMagenta, Ship->bTravelModeActive);
	}

	// ---- Docking prompt (lower centre; only while RequestDocking would succeed) ----
	float DockDist = 0.0f;
	FString DockName;
	if (Ship->CanRequestDocking(DockDist, DockName))
	{
		const FHudElementScope Layout(*this, EHudElement::Docking);
		const FString Station = DockDisplayName(Ship, DockName).Replace(TEXT("_"), TEXT(" ")).ToUpper();
		const FString Dist = FString::Printf(TEXT("%.0f M"), DockDist / 100.0f);
		float NW = 0.0f, NH = 0.0f, DW = 0.0f, DH = 0.0f;
		N.TextSize(Station, HudType::Heading, NW, NH);
		N.TextSize(Dist, HudType::Body, DW, DH);

		const float W = FMath::Max(NW + DW + 130.0f * S, 380.0f * S), H = 64.0f * S;
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

		const float LX = X + 16.0f * S;
		N.GlitchText(TEXT("DOCKING CLEARANCE // AVAILABLE"), kMagenta, LX, Y + 6.0f * S, HudType::Label, 6.0f);
		const float KeyW = N.KeyHint(TEXT("E"), TEXT(""), LX, Y + 26.0f * S, kCyan);
		N.GlitchText(Station, kInk, LX + KeyW + 6.0f * S, Y + 26.0f * S, HudType::Heading, 7.0f);
		N.Text(Dist, kCyan, X + W - DW - 16.0f * S, Y + 28.0f * S, HudType::Body);

		// Approach bar: fills as you close in on the docking point
		const float Range = FMath::Max(Ship->GetEffectiveDockingRange(), 1.0f);
		N.SegBar(LX, Y + H - 10.0f * S, W - 32.0f * S, 3.0f * S, 1.0f - DockDist / Range, 32, kMagenta, kCyan);
	}

	// ---- Drone controls hint (bottom centre; while mining drones are carried and nothing is locked) ----
	if (const UDroneBayComponent* Bay = Ship->DroneBay)
	{
		if (Bay->bMiningEnabled && !Bay->GetTarget())
		{
			const FHudElementScope Layout(*this, EHudElement::DroneHint);
			const float Y = SH - 58.0f * S;
			float Hx = SW * 0.5f - 170.0f * S;
			const int32 Out = Bay->GetDronesOut();
			N.Text(Out > 0 ? FString::Printf(TEXT("MINING DRONES // %d RETURNING"), Out) : FString(TEXT("MINING DRONES //")),
				WithAlpha(kYellow, 0.9f), Hx - 10.0f * S, Y - 20.0f * S, HudType::Label);
			Hx += N.KeyHint(TEXT("T"), TEXT("LOCK ASTEROID"), Hx, Y, kYellow) + 26.0f * S;
			N.KeyHint(TEXT("L"), TEXT("LAUNCH / RECALL"), Hx, Y, kYellow);
		}
	}

	// Mining panel (hidden unless an asteroid is locked) stacks under the ident strip.
	DrawMiningHUD(PC, Ship, IdentBottom + 12.0f * S);
}

void AAdastreaHUD::DrawCyberMiningHUD(APlayerController* PC, ASpaceship* Ship, UDroneBayComponent* Bay, AAsteroid* Rock, float PanelTop)
{
	if (!Canvas || !PC || !Ship || !Bay || !Rock)
	{
		return;
	}
	UWorld* World = GetWorld();
	const FNeon N{ Canvas, HudType::LayoutScale(Canvas->ClipX, Canvas->ClipY), World ? World->GetRealTimeSeconds() : 0.0f };
	const float S = N.S;

	UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	UTradeItemDataAsset* Ore = Bay->GetTargetOre();
	FLinearColor Tint = Type ? Type->OreTint : kCyan;
	Tint.A = 1.0f;
	const EDroneBayStatus Status = Bay->GetStatus();
	const bool bFiring = Status == EDroneBayStatus::Mining;
	const bool bInRange = Bay->IsTargetInRange();
	const float Dist = Bay->GetTargetSurfaceDistance();
	const FString RockName = Rock->GetTargetDisplayName_Implementation().ToString().ToUpper();

	// ---- Lock brackets around the asteroid (ore tint; yellow when out of range; pulse + orbiting ticks while drones work it) ----
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
				// Four ticks orbiting the rock while the drones work it
				for (int32 i = 0; i < 4; ++i)
				{
					const float A = N.Now * 2.5f + i * HALF_PI;
					const FVector2D Dir(FMath::Cos(A), FMath::Sin(A));
					N.Line(Centre + Dir * R * 0.78f, Centre + Dir * R * 0.9f, WithAlpha(kGreen, 0.9f), 2.0f);
				}
			}

			// Tag at the bracket's top-right: name, distance, yield sliver
			const float TX = X1 + 8.0f * S, TY = Y0;
			N.Rect(TX, TY, 3.0f * S, 36.0f * S, BC);
			N.Text(RockName, BC, TX + 8.0f * S, TY, HudType::Label);
			N.Text(FString::Printf(TEXT("%.0f M%s"), Dist / 100.0f, bInRange ? TEXT("") : TEXT("  // OUT OF RANGE")),
				bInRange ? kInk : kYellow, TX + 8.0f * S, TY + 15.0f * S, HudType::Caption);
			N.SegBar(TX + 8.0f * S, TY + 31.0f * S, 90.0f * S, 4.0f * S, Rock->GetOreFraction(), 10, Tint, Tint);
		}
	}

	// ---- Drone mining uplink panel (stacked under the ident strip, same width) ----
	const FHudElementScope Layout(*this, EHudElement::Mining);
	const float X = 24.0f * S, W = 270.0f * S, H = 198.0f * S;
	const float Y = PanelTop;
	N.Panel(X, Y, W, H, Tint);
	const float LX = X + 14.0f * S, RX = X + W - 14.0f * S, BW = W - 28.0f * S;

	auto RightText = [&](const FString& Str, const FLinearColor& Col, float RY, float Scale)
	{
		float TW = 0.0f, TH = 0.0f;
		N.TextSize(Str, Scale, TW, TH);
		N.Text(Str, Col, RX - TW, RY, Scale);
	};

	N.GlitchText(TEXT("// MINING UPLINK"), Tint, LX, Y + 6.0f * S, HudType::Body, 8.0f);
	RightText(TEXT("LOCKED"), kGreen, Y + 7.0f * S, HudType::Label);
	N.GlitchText(RockName, kInk, LX, Y + 22.0f * S, HudType::Body, 9.0f);

	// Ore + hardness
	N.Text(TEXT("ORE"), kMute, LX, Y + 42.0f * S, HudType::Label);
	N.Text(Ore ? Ore->ItemName.ToString().ToUpper() : FString(TEXT("NONE")), Ore ? Tint : kYellow, LX + 34.0f * S, Y + 42.0f * S, HudType::Label);
	RightText(FString::Printf(TEXT("HARDNESS %.1f"), Type ? Type->Hardness : 1.0f), kMute, Y + 42.0f * S, HudType::Label);

	// Remaining yield
	N.Text(TEXT("YIELD"), kMute, LX, Y + 59.0f * S, HudType::Label);
	RightText(FString::Printf(TEXT("%.0f / %.0f  (%.0f%%)"), Rock->GetRemainingOre(), Rock->GetTotalOre(), Rock->GetOreFraction() * 100.0f),
		kInk, Y + 59.0f * S, HudType::Label);
	N.SegBar(LX, Y + 75.0f * S, BW, 5.0f * S, Rock->GetOreFraction(), 24, Tint, Tint);

	// Range
	N.Text(TEXT("RANGE"), kMute, LX, Y + 86.0f * S, HudType::Label);
	N.Text(FString::Printf(TEXT("%.0f M  //  %s"), Dist / 100.0f, bInRange ? TEXT("IN RANGE") : TEXT("CLOSE IN")),
		bInRange ? kGreen : kYellow, LX + 44.0f * S, Y + 86.0f * S, HudType::Label);
	RightText(FString::Printf(TEXT("MAX %.0f"), Bay->Range / 100.0f), kMute, Y + 86.0f * S, HudType::Label);

	// Drone state: lit tag with drones out / cutting, status text beside it
	{
		const bool bOut = Bay->GetDronesOut() > 0;
		const FString Tag = FString::Printf(TEXT("DRONES %d/%d"), Bay->GetDronesOut(), Bay->DroneCount);
		const FLinearColor TagCol = bFiring ? kGreen : (bOut || Bay->IsDeployed() ? kYellow : kMute);
		const float TagW = N.Tag(Tag, LX, Y + 104.0f * S, TagCol, bFiring || bOut);
		N.Text(UDroneBayComponent::StatusToText(Status).ToString().ToUpper(), bFiring ? kGreen : kInk, LX + TagW + 4.0f * S, Y + 106.0f * S, HudType::Label);
		if (bFiring)
		{
			const float Lamp = 0.4f + 0.6f * FMath::Abs(FMath::Sin(N.Now * 8.0f));
			N.Rect(RX - 10.0f * S, Y + 108.0f * S, 10.0f * S, 10.0f * S, WithAlpha(kGreen, Lamp));
		}
	}

	// Cutting rate, and ore on its way home in drone hoppers
	const float Transit = Bay->GetOreInTransit();
	const float TransitCap = FMath::Max(Bay->HopperCapacity * Bay->DroneCount, 1.0f);
	N.Text(TEXT("EXTRACT"), kMute, LX, Y + 129.0f * S, HudType::Label);
	RightText(FString::Printf(TEXT("%.1f U/S   %d CUTTING   %.0f U INBOUND"), Bay->GetExtractionRate(), Bay->GetDronesCutting(), Transit),
		bFiring ? kGreen : kMute, Y + 129.0f * S, HudType::Label);
	N.SegBar(LX, Y + 145.0f * S, BW, 5.0f * S, Transit / TransitCap, 24, kCyan, kGreen, bFiring ? 1.0f : 0.5f);

	// Hold fill + ore in hold, with a "+N" pop after each delivery
	if (UCargoComponent* Cargo = Ship->CargoComponent)
	{
		const float Cap = FMath::Max(Cargo->CargoCapacity, 0.01f);
		const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
		const float Frac = Used / Cap;
		const bool bFull = Frac > 0.9f;
		N.Text(TEXT("HOLD"), kMute, LX, Y + 156.0f * S, HudType::Label);
		RightText(FString::Printf(TEXT("%.0f / %.0f%s"), Used, Cap, bFull ? TEXT("  // FULL") : TEXT("")), bFull ? kRed : kCyan, Y + 156.0f * S, HudType::Label);
		N.SegBar(LX, Y + 172.0f * S, BW, 5.0f * S, Frac, 24, kCyan, bFull ? kRed : kMagenta);
		N.Text(Ore ? FString::Printf(TEXT("IN HOLD  %s x%d"), *Ore->ItemName.ToString().ToUpper(), Cargo->GetItemQuantity(Ore)) : FString(TEXT("IN HOLD  -")),
			kMute, LX, Y + 180.0f * S, HudType::Caption);

		const float Since = Bay->GetSecondsSinceLastMined();
		if (Bay->GetLastMinedOre() && Since < 1.5f)
		{
			const FString Plus = FString::Printf(TEXT("+%d"), Bay->GetLastMinedAmount());
			float PW = 0.0f, PH = 0.0f;
			N.TextSize(Plus, HudType::Heading, PW, PH);
			N.GlitchText(Plus, WithAlpha(kGreen, FMath::Clamp(1.0f - Since / 1.5f, 0.0f, 1.0f)),
				RX - PW, Y + 176.0f * S - Since * 14.0f * S, HudType::Heading, 10.0f);
		}
	}
	else
	{
		N.Text(TEXT("HOLD  // NO CARGO BAY"), kYellow, LX, Y + 156.0f * S, HudType::Label);
	}
}
