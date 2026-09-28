// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

class UFont;

/**
 * Shared type ramp and layout scale for the canvas-drawn HUD (AdastreaHUD*.cpp).
 *
 * Every string the HUD draws picks one of these roles instead of an ad-hoc
 * scale, so the same kind of text is the same size on every screen. The
 * engine's Small/Medium/Large fonts are all the default Roboto 10 (about
 * 16 px per em at scale 1), so the role scale alone sets the size.
 *
 * Full-screen menus are laid out in pixels and use these scales directly;
 * the cockpit flight HUD multiplies them by LayoutScale().
 */
namespace AdastreaHUDStyle
{
	/** Coordinates, sub-values under a bar ("350 / 350"), legends, version strings. */
	constexpr float Caption = 0.65f;
	/** Field labels, tape numbers, mode tags, key/control hints, list rows in dense panels. */
	constexpr float Label   = 0.75f;
	/** Values, list rows, menu blurbs, cockpit panel tags. */
	constexpr float Body    = 0.85f;
	/** Panel titles, menu options, section headers, the heading readout. */
	constexpr float Heading = 1.0f;
	/** Full-screen titles (TRADING, SHIP SELECT, SECTOR MAP). */
	constexpr float Title   = 1.25f;
	/** The one big number (speed). */
	constexpr float Display = 2.0f;

	/** The HUD font. */
	inline UFont* Font() { return GEngine->GetSmallFont(); }

	/**
	 * Layout scale for the cockpit flight HUD, which is authored at 1920x1080.
	 * Fits by the tighter axis (so a tall or narrow PIE viewport doesn't blow the
	 * panels up), then multiplies by the adastrea.HUDScale console variable.
	 */
	float LayoutScale(float CanvasW, float CanvasH);
}

namespace HudType = AdastreaHUDStyle;
