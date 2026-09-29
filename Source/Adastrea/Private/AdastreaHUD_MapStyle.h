// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Layout and palette shared by every layer of the full-screen map (M):
// the Sector view in AdastreaHUD.cpp and the System/Universe views in
// AdastreaHUD_GalaxyMap.cpp.

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

namespace GalaxyMap
{
	constexpr float Margin = 40.0f;
	constexpr float HeaderH = 30.0f;
	constexpr float FooterH = 70.0f;
	constexpr float PanelW = 340.0f;
	constexpr float PanelGap = 12.0f;

	const FLinearColor Backdrop(0.008f, 0.012f, 0.02f, 1.0f);
	const FLinearColor BoxFill(0.03f, 0.05f, 0.07f, 0.9f);
	const FLinearColor BoxEdge(0.10f, 0.55f, 0.60f, 0.9f);
	const FLinearColor Title(0.6f, 0.9f, 1.0f, 1.0f);
	const FLinearColor Body(0.78f, 0.86f, 0.92f, 1.0f);
	const FLinearColor Dim(0.5f, 0.6f, 0.68f, 0.9f);
	const FLinearColor Help(0.6f, 0.7f, 0.8f, 0.9f);
	const FLinearColor You(0.15f, 0.95f, 0.6f, 1.0f);
	const FLinearColor Built(0.25f, 0.88f, 0.82f, 1.0f);
	const FLinearColor Planned(0.5f, 0.56f, 0.62f, 1.0f);
	const FLinearColor Gate(0.95f, 0.72f, 0.3f, 0.6f);
	const FLinearColor Lane(0.35f, 0.6f, 0.72f, 0.55f);
	const FLinearColor LaneHot(0.55f, 0.9f, 1.0f, 0.95f);
	const FLinearColor Select(1.0f, 1.0f, 1.0f, 0.95f);
	/** The plotted route from the player to the selection (System and Universe layers). */
	const FLinearColor Route(0.15f, 0.95f, 0.6f, 0.9f);

	// Sector view object colours.
	const FLinearColor Station(0.95f, 0.78f, 0.30f, 1.0f);
	const FLinearColor StationText(0.9f, 0.85f, 0.6f, 1.0f);
	const FLinearColor Ship(0.3f, 0.8f, 0.9f, 1.0f);
	const FLinearColor ShipText(0.6f, 0.85f, 0.95f, 1.0f);
	const FLinearColor GateOnline(0.7f, 0.5f, 1.0f, 1.0f);
	const FLinearColor GateOffline(1.0f, 0.62f, 0.2f, 1.0f);
	const FLinearColor Asteroid(0.62f, 0.56f, 0.5f, 0.8f);

	struct FLayout
	{
		float VW = 0, VH = 0;
		float BoxX = 0, BoxY = 0, BoxW = 0, BoxH = 0;
		float AreaX = 0, AreaW = 0;              // map drawing area (left of the panel)
		float PanelX = 0, PanelY = 0, PanelH = 0, PanelWidth = 0;
	};

	inline FLayout MakeLayout(const UCanvas* Canvas, APlayerController* PC)
	{
		FLayout L;
		int32 VX = 0, VY = 0;
		PC->GetViewportSize(VX, VY);
		// Canvas size, not viewport size: they differ for high-res screenshots.
		L.VW = Canvas ? Canvas->ClipX : (float)VX;
		L.VH = Canvas ? Canvas->ClipY : (float)VY;
		L.BoxX = Margin;
		L.BoxY = Margin + HeaderH;
		L.BoxW = FMath::Max(L.VW - Margin * 2.0f, 200.0f);
		L.BoxH = FMath::Max(L.VH - Margin * 2.0f - FooterH, 200.0f);
		L.PanelWidth = FMath::Min(PanelW, L.BoxW * 0.4f);
		L.PanelX = L.BoxX + L.BoxW - L.PanelWidth - PanelGap;
		L.PanelY = L.BoxY + PanelGap;
		L.PanelH = L.BoxH - PanelGap * 2.0f;
		L.AreaX = L.BoxX;
		L.AreaW = L.PanelX - L.BoxX - PanelGap;
		return L;
	}

	/** Centimetres as "850 m" / "12.4 km". */
	inline FString FormatDistance(float Cm)
	{
		return Cm < 100000.0f ? FString::Printf(TEXT("%.0f m"), Cm / 100.0f) : FString::Printf(TEXT("%.1f km"), Cm / 100000.0f);
	}
}
