// Copyright Epic Games, Inc. All Rights Reserved.

// Flight combat overlay for AAdastreaHUD (canvas-drawn):
//   gun crosshair   where the forward guns converge (the nose's aim point)
//   lead pip        where to put the crosshair so bolts meet the locked target
//   hit marker      ticks around the crosshair when one of your bolts lands

#include "AdastreaHUD.h"
#include "AdastreaHUDStyle.h"
#include "Combat/CombatProjectileSubsystem.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Ships/Spaceship.h"
#include "Player/AdastreaPlayerController.h"
#include "Engine/Canvas.h"

namespace
{
	const FLinearColor kGunCross(0.55f, 0.95f, 1.0f, 0.85f);
	const FLinearColor kLeadPip(1.0f, 0.62f, 0.2f, 1.0f);
	const FLinearColor kLeadOnTarget(0.35f, 1.0f, 0.45f, 1.0f);
	const FLinearColor kHitMarker(1.0f, 1.0f, 1.0f, 1.0f);

	constexpr float HitMarkerTime = 0.15f;

	FLinearColor Faded(const FLinearColor& C, float A) { return FLinearColor(C.R, C.G, C.B, A); }
}

void AAdastreaHUD::DrawCombatOverlay(APlayerController* PC, ASpaceship* Ship)
{
	UShipWeaponComponent* Guns = Ship ? Ship->WeaponComponent.Get() : nullptr;
	if (!PC || !Guns || !Canvas)
	{
		return;
	}
	const float S = HudType::LayoutScale(Canvas->ClipX, Canvas->ClipY);
	const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;

	auto Circle = [this](const FVector2D& C, float R, const FLinearColor& Col, float Thick)
	{
		constexpr int32 Segs = 16;
		for (int32 i = 0; i < Segs; ++i)
		{
			const float A0 = 2.0f * PI * i / Segs, A1 = 2.0f * PI * (i + 1) / Segs;
			DrawLine(C.X + R * FMath::Cos(A0), C.Y + R * FMath::Sin(A0), C.X + R * FMath::Cos(A1), C.Y + R * FMath::Sin(A1), Col, Thick);
		}
	};

	// ---- Gun crosshair ----
	FVector2D Cross;
	const bool bCross = PC->ProjectWorldLocationToScreen(Guns->GetConvergencePoint(), Cross);
	if (bCross)
	{
		const float R = 11.0f * S;
		Circle(Cross, R, kGunCross, 1.5f);
		DrawLine(Cross.X - R * 1.9f, Cross.Y, Cross.X - R * 1.2f, Cross.Y, kGunCross, 1.5f);
		DrawLine(Cross.X + R * 1.2f, Cross.Y, Cross.X + R * 1.9f, Cross.Y, kGunCross, 1.5f);
		DrawLine(Cross.X, Cross.Y + R * 1.2f, Cross.X, Cross.Y + R * 1.9f, kGunCross, 1.5f);
	}

	// ---- Hit marker ----
	if (const UCombatProjectileSubsystem* Combat = UCombatProjectileSubsystem::Get(this))
	{
		const int32 Hits = Combat->GetHitCount(Ship);
		if (Hits > CombatLastHitCount)
		{
			CombatHitMarker = HitMarkerTime;
		}
		CombatLastHitCount = Hits;
	}
	if (bCross && CombatHitMarker > 0.0f)
	{
		const float In = 16.0f * S, Out = 26.0f * S;
		for (const FVector2D D : { FVector2D(1, 1), FVector2D(-1, 1), FVector2D(1, -1), FVector2D(-1, -1) })
		{
			DrawLine(Cross.X + D.X * In, Cross.Y + D.Y * In, Cross.X + D.X * Out, Cross.Y + D.Y * Out, kHitMarker, 2.0f);
		}
	}
	CombatHitMarker = FMath::Max(CombatHitMarker - Dt, 0.0f);

	// ---- Lead pip on the locked target (ships only: stations and rocks don't move) ----
	const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC);
	const ASpaceship* Target = AdPC ? Cast<ASpaceship>(AdPC->GetLockedTarget()) : nullptr;
	if (!Target || Target == Ship || Target->IsHidden())
	{
		return;
	}
	FVector Lead;
	FVector2D Pip, TargetPt;
	if (!Guns->GetLeadPoint(Target, Lead) || FVector::Dist(Ship->GetActorLocation(), Lead) > Guns->Range * 1.5f
		|| !PC->ProjectWorldLocationToScreen(Lead, Pip))
	{
		return;
	}
	const float PipR = 9.0f * S;
	const bool bOnTarget = bCross && FVector2D::Distance(Pip, Cross) < PipR * 1.6f;
	const FLinearColor PipCol = bOnTarget ? kLeadOnTarget : kLeadPip;

	if (PC->ProjectWorldLocationToScreen(Target->GetActorLocation(), TargetPt) && FVector2D::Distance(Pip, TargetPt) > PipR * 2.0f)
	{
		DrawLine(TargetPt.X, TargetPt.Y, Pip.X, Pip.Y, Faded(PipCol, 0.35f), 1.0f);
	}
	DrawLine(Pip.X, Pip.Y - PipR, Pip.X + PipR, Pip.Y, PipCol, 2.0f);
	DrawLine(Pip.X + PipR, Pip.Y, Pip.X, Pip.Y + PipR, PipCol, 2.0f);
	DrawLine(Pip.X, Pip.Y + PipR, Pip.X - PipR, Pip.Y, PipCol, 2.0f);
	DrawLine(Pip.X - PipR, Pip.Y, Pip.X, Pip.Y - PipR, PipCol, 2.0f);
	if (bOnTarget)
	{
		DrawRect(PipCol, Pip.X - 2.0f * S, Pip.Y - 2.0f * S, 4.0f * S, 4.0f * S);
	}
}
