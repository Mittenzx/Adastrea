// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaHUD.h"
#include "AdastreaNames.h"
#include "Combat/ShipHealthComponent.h"
#include "AI/HostileFighterController.h"
#include "AdastreaGameMode.h"
#include "AdastreaHUDStyle.h"
#include "AdastreaHUD_MapStyle.h"
#include "Stations/StationInterior.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Ships/SpaceshipInterior.h"
#include "Player/WorldInteractable.h"
#include "Player/PlayerInteractableComponent.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Camera/PlayerCameraManager.h"
#include "Drones/DroneBayComponent.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Player/AdastreaPlayerController.h"
#include "Stations/SpaceStation.h"
#include "Stations/StationModuleTypes.h"
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
#include "Kismet/KismetSystemLibrary.h"
#include "Audio/AudioMixSubsystem.h"
#include "Audio/AdastreaAudioSettings.h"
#include "Audio/AudioCatalogSubsystem.h"
#include "Audio/AudioEventLibrary.h"
#include "Universe/GalaxySubsystem.h"
#include "Universe/JumpGate.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<float> CVarHUDScale(
	TEXT("adastrea.HUDScale"),
	1.0f,
	TEXT("Size multiplier for the cockpit flight HUD (1 = default; try 0.8 for a smaller HUD)."),
	ECVF_Default);

float AdastreaHUDStyle::LayoutScale(float CanvasW, float CanvasH)
{
	const float Fit = FMath::Min(CanvasW / 1920.0f, CanvasH / 1080.0f);
	return FMath::Clamp(Fit, 0.8f, 2.5f) * FMath::Clamp(CVarHUDScale.GetValueOnGameThread(), 0.5f, 2.0f);
}

void AAdastreaHUD::DrawCentredText(const FString& Text, const FLinearColor& Color, float CentreX, float Y, float Scale)
{
	float W = 0.0f, H = 0.0f;
	GetTextSize(Text, W, H, HudType::Font(), Scale);
	DrawText(Text, Color, CentreX - W * 0.5f, Y, HudType::Font(), Scale);
}

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

// Names come from game data, not actor labels: packaged builds have no labels, and
// object names ("BP_TradeStation_C_2") mean nothing to the player.
static FString HudActorName(const AActor* Actor)
{
	return AdastreaNames::ForActor(Actor);
}

void AAdastreaHUD::UpdateMenuAudio()
{
	FMenuAudioState Now;
	Now.bInitialized = true;
	Now.bMap = bShowMap;
	Now.bTradeScreen = bShowTradeScreen;
	Now.bOutfitting = bShowOutfitting;
	Now.bCrafting = bShowCrafting;
	Now.bStationMenu = bShowStationMenu;
	Now.bShipSelect = bShowShipSelect;
	Now.bStationInfo = bShowStationInfo;
	Now.bBuyMode = bBuyMode;
	Now.StationMenuIndex = StationMenuIndex;
	Now.TradeIndex = SelectedTradeIndex;
	Now.OutfittingCategory = OutfittingCategoryIndex;
	Now.OutfittingRow = OutfittingRowIndex;
	Now.CraftingRow = bCraftingJobsFocus ? -1 - CraftingJobIndex : CraftingRowIndex + 1000 * CraftingFacilityIndex;
	Now.ShipSelectIndex = ShipSelectIndex;

	const FMenuAudioState Was = MenuAudioState;
	MenuAudioState = Now;
	if (!Was.bInitialized)
	{
		return;
	}

	const bool bOpened = (Now.bMap && !Was.bMap) || (Now.bTradeScreen && !Was.bTradeScreen)
		|| (Now.bStationMenu && !Was.bStationMenu) || (Now.bShipSelect && !Was.bShipSelect)
		|| (Now.bStationInfo && !Was.bStationInfo) || (Now.bOutfitting && !Was.bOutfitting)
		|| (Now.bCrafting && !Was.bCrafting);
	const bool bClosed = (!Now.bMap && Was.bMap) || (!Now.bTradeScreen && Was.bTradeScreen)
		|| (!Now.bStationMenu && Was.bStationMenu) || (!Now.bShipSelect && Was.bShipSelect)
		|| (!Now.bStationInfo && Was.bStationInfo) || (!Now.bOutfitting && Was.bOutfitting)
		|| (!Now.bCrafting && Was.bCrafting);

	// Secondary: docking's clamp, a menu confirm click, a door etc. already cover these.
	if (bOpened)
	{
		UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Open"), 0.1f);
	}
	else if (bClosed)
	{
		UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Close"), 0.1f);
	}

	if (Now.bTradeScreen && Was.bTradeScreen && Now.bBuyMode != Was.bBuyMode)
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"), 0.08f);
	}

	const bool bHoverMoved =
		(Now.bStationMenu && Was.bStationMenu && Now.StationMenuIndex != Was.StationMenuIndex)
		|| (Now.bTradeScreen && Was.bTradeScreen && Now.TradeIndex != Was.TradeIndex)
		|| (Now.bOutfitting && Was.bOutfitting
			&& (Now.OutfittingRow != Was.OutfittingRow || Now.OutfittingCategory != Was.OutfittingCategory))
		|| (Now.bCrafting && Was.bCrafting && Now.CraftingRow != Was.CraftingRow)
		|| (Now.bShipSelect && Was.bShipSelect && Now.ShipSelectIndex != Was.ShipSelectIndex);
	if (bHoverMoved)
	{
		UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Hover"), 0.05f);
	}
}

void AAdastreaHUD::DrawHUD()
{
	Super::DrawHUD();

	UpdateMenuAudio();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	SyncShipWidgetsForMap(PC);

	// Another screen took over (it owns the cursor now): drop out of the HUD layout editor.
	if (bHudEditMode && (IsOtherScreenOpen() || bShowPauseMenu || !Cast<ASpaceship>(PC->GetPawn())))
	{
		SetHudEditMode(false);
	}

	// Full-screen sector map (toggled by M) draws over everything.
		if (bShowMap)
		{
			MapHitRects.Reset();
			if (MapView == EAdastreaMapView::System)
			{
				DrawSystemMap(PC);
			}
			else if (MapView == EAdastreaMapView::Universe)
			{
				DrawUniverseMap(PC);
			}
			else if (APawn* Pawn = PC->GetPawn())
			{
				DrawSectorMap(PC, Pawn->GetActorLocation());
			}
		}

		// Pause menu (volume sliders) draws over everything when shown.
		if (bShowPauseMenu)
		{
			DrawPauseMenu(PC);
			return;
		}

		// The map is full-screen and opaque: no flight HUD, prompts or panels over it.
		if (bShowMap)
		{
			return;
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

		// Docked station menu draws over everything when shown.
		if (bShowStationMenu)
		{
			if (Ship)
			{
				DrawStationMenu(PC, Cast<AAdastreaPlayerController>(PC), Ship);
			}
			// On top of the menu's dimming (e.g. the customs report on docking).
			if (!PendingMessage.IsEmpty())
			{
				DrawTransientMessage(PC);
			}
			return;
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

		// Docked outfitting screen draws over everything when shown.
		if (bShowOutfitting)
		{
			if (Ship)
			{
				DrawOutfittingScreen(PC, Ship);
			}
			return;
		}

		// Docked production screen draws over everything when shown.
		if (bShowCrafting)
		{
			if (Ship)
			{
				DrawCraftingScreen(PC, Ship);
			}
			if (!PendingMessage.IsEmpty())
			{
				DrawTransientMessage(PC);
			}
			return;
		}

	// Ship-select screen draws over everything when shown.
		if (bShowShipSelect)
		{
			DrawShipSelectScreen(PC);
			// Again on top: the screen covers the one drawn above ("Already flying ...").
			if (!PendingMessage.IsEmpty())
			{
				DrawTransientMessage(PC);
			}
			return;
		}

		// Station info screen draws over everything when shown.
		if (bShowStationInfo)
		{
			if (AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
						{
							ASpaceStation* Station = AdPC->GetNearestStation();
				const FVector Observer = Ship ? Ship->GetActorLocation() : FVector::ZeroVector;
				if (Station)
				{
					DrawStationInfoScreen(PC, Station, Observer);
				}
				else
				{
					DrawText(TEXT("No station within range."), FLinearColor(0.9f,0.6f,0.4f,1.0f), 200.0f, 200.0f, HudType::Font(), HudType::Body);
				}
			}
			return;
		}

		if (!Ship)
		{
			return; // only draw once we're flying the ship
		}

		// The Station Editor's 3D plan view replaces the ship view; no flight HUD over it.
		if (const AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
		{
			if (AdPC->IsStationEditorOpen())
			{
				return;
			}
		}

	// ---- Docking prompt (shown only while RequestDocking would succeed; the cockpit HUD draws its own) ----
	if (!bCyberpunkFlightHUD)
	{
		float DockDist = 0.0f;
		FString DockName;
		if (Ship->CanRequestDocking(DockDist, DockName))
		{
			const FString Line = FString::Printf(TEXT("[E]  DOCK   -   %s   (%.0f m)"), *DockName, DockDist / 100.0f);
			float W = 0.0f, H = 0.0f;
			GetTextSize(Line, W, H, HudType::Font(), HudType::Heading);
			const float X = (Canvas->SizeX - W) * 0.5f;
			const float Y = Canvas->SizeY * 0.72f;
			DrawRect(kBg, X - 16.0f, Y - 8.0f, W + 32.0f, H + 16.0f);
			DrawLine(X - 16.0f, Y + H + 8.0f, X + W + 16.0f, Y + H + 8.0f, kBorder, 2.0f);
			DrawText(Line, FLinearColor(0.15f, 0.9f, 0.6f, 1.0f), X, Y, HudType::Font(), HudType::Heading);
		}
	}

	// ---- Flight HUD: neon cockpit layout, or the legacy telemetry panel ----
	if (bCyberpunkFlightHUD)
	{
		DrawCyberpunkFlightHUD(PC, Ship);
	}
	else
	{
		// Mining panel stacks under the telemetry panel (hidden unless an asteroid is locked).
		DrawMiningHUD(PC, Ship, DrawTelemetryPanel(Ship) + 12.0f);
	}

	// ---- Jump gates: world markers + approach prompt ----
	DrawJumpGateMarkers(PC, Ship);

	// ---- Combat: gun crosshair, lead pip, hit marker ----
	DrawCombatOverlay(PC, Ship);

	// ---- Pirate hail: comms panel with the demand (J comply / K refuse) ----
	DrawRaidComms(PC);

	// ---- HUD layout editor (H): outlines and drag/scale handling over the cockpit panels ----
	DrawHudLayoutEditor(PC);

	const FVector P = Ship->GetActorLocation();

	// ---- Locked target reticle (world-space box around the locked target) ----
	AAdastreaPlayerController* AController = Cast<AAdastreaPlayerController>(PC);
	AActor* LockedTarget = AController ? AController->GetLockedTarget() : nullptr;

	// Hover highlight: while targeting, show which target the cursor is over.
	AActor* HoverTarget = nullptr;
	if (AController && AController->IsTargetingModeActive())
	{
		HoverTarget = AController->GetTargetUnderCursor();
	}

	// Draw reticle on the hovered target (if any, and not already locked).
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
		FString TgtName = HudActorName(LockedTarget);
		if (const ASpaceship* TgtShip = Cast<ASpaceship>(LockedTarget))
		{
			TgtName = TgtShip->GetShipName().ToString();
		}
		else if (AAsteroid* TgtRock = Cast<AAsteroid>(LockedTarget))
		{
			TgtName = ITargetable::Execute_GetTargetDisplayName(TgtRock).ToString();
		}
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

		// Teal-green; red for a hostile.
		const FLinearColor Reticle = AHostileFighterController::IsHostileShip(LockedTarget)
			? FLinearColor(1.0f, 0.3f, 0.25f, 1.0f) : FLinearColor(0.15f, 0.9f, 0.6f, 1.0f);

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
				EdgePt.X - 14.0f, EdgePt.Y + 12.0f, HudType::Font(), HudType::Caption);
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

			DrawText(TEXT("TARGET LOCKED"), FLinearColor(0.15f, 0.9f, 0.6f, 1.0f), LX, PY + 8.0f, HudType::Font(), HudType::Heading);
			DrawLine(PX + 10.0f, PY + 30.0f, PX + PW - 10.0f, PY + 30.0f, kBorder, 1.0f);

			DrawText(TEXT("NAME"), kLabel, LX, PY + 40.0f, HudType::Font(), HudType::Label);
			DrawText(TgtName, FLinearColor::White, VX, PY + 40.0f, HudType::Font(), HudType::Label);

			DrawText(TEXT("DISTANCE"), kLabel, LX, PY + 62.0f, HudType::Font(), HudType::Label);
			DrawText(FString::Printf(TEXT("%.0f u"), TgtDist), kSpeed, VX, PY + 62.0f, HudType::Font(), HudType::Label);

			// Station module info (if it's a station)
			if (ASpaceStation* Station = Cast<ASpaceStation>(LockedTarget))
			{
				int32 ModuleCount = Station->GetModuleCount();
				int32 Docks = Station->GetDockingBayModules().Num();
				DrawText(TEXT("MODULES"), kLabel, LX, PY + 84.0f, HudType::Font(), HudType::Label);
				DrawText(FString::Printf(TEXT("%d  (%d docks)"), ModuleCount, Docks),
					kCargo, VX, PY + 84.0f, HudType::Font(), HudType::Label);
			}
			else if (const ASpaceship* TgtShipStats = Cast<ASpaceship>(LockedTarget))
			{
				// Shield and hull bars, so you can watch a target go down.
				const UShipHealthComponent* TgtHealth = TgtShipStats->HealthComponent;
				if (TgtShipStats->IsWrecked())
				{
					DrawText(TEXT("STATUS"), kLabel, LX, PY + 84.0f, HudType::Font(), HudType::Label);
					DrawText(TEXT("WRECK - DISABLED"), FLinearColor(0.65f, 0.6f, 0.55f, 1.0f), VX, PY + 84.0f, HudType::Font(), HudType::Label);
				}
				else
				{
					const float ShieldMax = TgtHealth ? TgtHealth->GetMaxShield() : 0.0f;
					const float HullMax = FMath::Max(TgtShipStats->GetMaxHullIntegrity(), 1.0f);
					auto Bar = [&](const TCHAR* Label, float Y, float Value, float Max, const FLinearColor& Col)
					{
						const float BW = PW - (VX - PX) - 14.0f;
						const float Frac = Max > 0.0f ? FMath::Clamp(Value / Max, 0.0f, 1.0f) : 0.0f;
						DrawText(Label, kLabel, LX, PY + Y, HudType::Font(), HudType::Label);
						DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.08f), VX, PY + Y + 3.0f, BW, 10.0f);
						DrawRect(Col, VX, PY + Y + 3.0f, BW * Frac, 10.0f);
					};
					Bar(TEXT("SHIELD"), 84.0f, TgtHealth ? TgtHealth->GetShield() : 0.0f, ShieldMax, FLinearColor(0.3f, 0.75f, 1.0f, 0.9f));
					Bar(TEXT("HULL"), 104.0f, TgtShipStats->GetCurrentHullIntegrity(), HullMax, FLinearColor(1.0f, 0.62f, 0.2f, 0.9f));
				}
			}
			else if (const AAsteroid* Rock = Cast<AAsteroid>(LockedTarget))
			{
				DrawText(TEXT("ORE"), kLabel, LX, PY + 84.0f, HudType::Font(), HudType::Label);
				DrawText(FString::Printf(TEXT("%.0f / %.0f  (%.0f%%)"), Rock->GetRemainingOre(), Rock->GetTotalOre(),
					Rock->GetOreFraction() * 100.0f), kCargo, VX, PY + 84.0f, HudType::Font(), HudType::Label);
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
					const FString Hint = TEXT("TARGETING ACTIVE - Click to lock   [ ] cycle   Y nearest   Z clear   Tab exit");
					float HintW = 0.0f, HintH = 0.0f;
					GetTextSize(Hint, HintW, HintH, HudType::Font(), HudType::Label);
					DrawText(Hint, FLinearColor(0.6f, 0.7f, 0.75f, 0.9f),
						(VSizeX - HintW) * 0.5f, VSizeY - 40.0f, HudType::Font(), HudType::Label);
				}
			}
	}


float AAdastreaHUD::DrawTelemetryPanel(ASpaceship* Ship)
{
	// ---- Gather live data ----
	const FVector P = Ship->GetActorLocation();
	const float Speed = Ship->MovementComponent ? Ship->MovementComponent->Velocity.Size() : 0.0f;
	const int32 Credits = Ship->PlayerTraderComponent ? static_cast<int32>(Ship->PlayerTraderComponent->GetCredits()) : 0;
	const float CargoUsed = Ship->CargoComponent ? (Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace()) : 0.0f;
	const float CargoMax = Ship->CargoComponent ? FMath::Max(Ship->CargoComponent->CargoCapacity, 0.01f) : 1.0f;
	const float Throttle = Ship->ThrottlePercentage;


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
		kHeader, PanelX + 14.0f, TitleY, HudType::Font(), HudType::Heading);
	DrawLine(PanelX + 12.0f, TitleY + 26.0f, PanelX + PanelW - 12.0f, TitleY + 26.0f, kBorder, 1.0f);

	float Y = RowStartY;
	auto Row = [&](const TCHAR* Label, const FString& Value, const FLinearColor& ValueColor)
	{
		DrawText(Label, kLabel, LabelX, Y, HudType::Font(), HudType::Body);
		DrawText(Value, ValueColor, ValueX, Y, HudType::Font(), HudType::Body);
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
	DrawText(TEXT("POSITION"), kLabel, LabelX, Y, HudType::Font(), HudType::Label);
	DrawText(FString::Printf(TEXT("X %8.0f   Y %8.0f   Z %8.0f"), P.X, P.Y, P.Z),
		kPos, ValueX, Y, HudType::Font(), HudType::Label);
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
			DrawText(Label, Col, Sx - 7.0f, Sy - 9.0f, HudType::Font(), HudType::Label);
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
		DrawText(TEXT("^"), FLinearColor(0.5f,0.9f,0.7f,1.0f), Lx + 7.0f, Cy - R - 2.0f, HudType::Font(), HudType::Caption);
		DrawText(TEXT("v"), FLinearColor(0.9f,0.5f,0.5f,1.0f), Lx + 8.0f, Cy + R - 6.0f, HudType::Font(), HudType::Caption);

		// ---- Heading readout (degrees) ----
		const FString HeadingStr = FString::Printf(TEXT("%03.0f."), HeadingClock);
		DrawText(HeadingStr, kPos, Cx - 24.0f, Cy + R + 3.0f, HudType::Font(), HudType::Label);

		Y += RowH + 24.0f;
	}

	return PanelY + PanelH;
}


void AAdastreaHUD::DrawSectorMap(APlayerController* PC, const FVector& ShipPos)
{
	using namespace GalaxyMap;
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	UWorld* World = PC->GetWorld();
	SectorMapPicks.Reset();

	// ---- Layout: 3D view on the left, info panel on the right (same frame as the System/Universe layers) ----
	const FLayout Lay = MakeLayout(Canvas, PC);
	const float VW = Lay.VW, VH = Lay.VH;
	const float BoxX = Lay.AreaX;
	const float BoxY = Lay.BoxY;
	const float BoxW = Lay.AreaW;
	const float BoxH = Lay.BoxH;

	DrawRect(Backdrop, 0.0f, 0.0f, VW, VH);
	DrawRect(BoxFill, Lay.BoxX, Lay.BoxY, Lay.BoxW, Lay.BoxH);

	float MouseX = 0.0f, MouseY = 0.0f;
	const bool bMouse = PC->GetMousePosition(MouseX, MouseY);
	const FVector2D Mouse(MouseX, MouseY);

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

	// ---- Grid on the map plane (Z = MapCenter.Z; hierarchical: minor + major lines for perspective) ----
	// Adaptive minor spacing so the grid always reads at any zoom.
	float GridStep = FMath::Pow(10.0f, FMath::FloorToFloat(FMath::LogX(10.0f, MapZoom * 0.25f)));
	GridStep = FMath::Clamp(GridStep, 5000.0f, 50000.0f);
	{
		const FLinearColor kMinor = FLinearColor(0.13f, 0.20f, 0.26f, 0.6f);
		const FLinearColor kMajor = FLinearColor(0.35f, 0.60f, 0.68f, 0.85f);
		const float G = GridStep;
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

	// ---- Box border (the fill is drawn before the grid) ----
	DrawRect(BoxEdge, Lay.BoxX, Lay.BoxY, 3.0f, Lay.BoxH);                  // left
	DrawRect(BoxEdge, Lay.BoxX, Lay.BoxY, Lay.BoxW, 3.0f);                  // top
	DrawRect(BoxEdge, Lay.BoxX, Lay.BoxY + Lay.BoxH - 3.0f, Lay.BoxW, 3.0f); // bottom

	// Altitude stem: a faint line from an object down (or up) to the map plane, so
	// height above/below you reads in the perspective view. Skipped when nearly level.
	auto DrawStem = [&](const FVector& W, const FVector2D& SP, const FLinearColor& Col)
	{
		if (FMath::Abs(W.Z - MapCenter.Z) < 1000.0f)
		{
			return;
		}
		FVector2D Foot;
		if (!ProjectRaw(FVector(W.X, W.Y, MapCenter.Z), Foot))
		{
			return;
		}
		const FLinearColor C(Col.R, Col.G, Col.B, 0.35f);
		DrawGridLine(SP, Foot, C, 1.0f);
		DrawGridLine(Foot - FVector2D(4.0f, 0.0f), Foot + FVector2D(4.0f, 0.0f), C, 1.0f);
	};

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

	// Labels are queued and placed after every icon is drawn, most important first
	// (gates, then stations, then ships), nudged up/down to avoid overlaps and dropped
	// when there's no room. Hovering an icon always shows its name.
	struct FMapLabel { FString Text; FLinearColor Color; FVector2D At; int32 Priority; };
	TArray<FMapLabel> Labels;

	// Asteroids: small dots, no labels (a field has hundreds); hovering shows the name.
	int32 NumAsteroids = 0;
	for (TActorIterator<AAsteroid> It(World); It; ++It)
	{
		++NumAsteroids;
		FVector2D SP;
		if (!Project(It->GetActorLocation(), SP)) continue;
		DrawRect(GalaxyMap::Asteroid, SP.X - 1.5f, SP.Y - 1.5f, 3.0f, 3.0f);
		SectorMapPicks.Add({ SP, *It });
	}

	// Stations: gold hollow-square icon + label
	if (bShowStations)
	{
		for (AActor* A : Stations)
		{
			if (!A) continue;
			FVector2D SP;
			if (!Project(A->GetActorLocation(), SP)) continue;
			DrawStem(A->GetActorLocation(), SP, GalaxyMap::Station);
			DrawStationIcon(SP.X, SP.Y, 6.0f, GalaxyMap::Station, 2.0f);
			Labels.Add({ HudActorName(A), StationText, FVector2D(SP.X + 9.0f, SP.Y - 6.0f), 1 });
			SectorMapPicks.Add({ SP, A });
		}
	}
	// Ships: cyan hollow diamond icon + label
	int32 NumOtherShips = 0;
	for (AActor* A : Ships)
	{
		if (!A || A == PC->GetPawn()) continue; // skip the player's own ship (shown as YOU)
		++NumOtherShips;
		if (!bShowShips) continue;
		FVector2D SP;
		if (!Project(A->GetActorLocation(), SP)) continue;
		DrawStem(A->GetActorLocation(), SP, GalaxyMap::Ship);
		DrawShipIcon(SP.X, SP.Y, 6.0f, GalaxyMap::Ship, 2.0f);
		Labels.Add({ HudActorName(A), ShipText, FVector2D(SP.X + 8.0f, SP.Y - 6.0f), 2 });
		SectorMapPicks.Add({ SP, A });
	}

	// Jump gates: violet ring + destination (always shown; they're how you leave the sector)
	// Off-screen marker: a triangle on the box edge pointing towards something outside the view.
	auto DrawEdgeMarker = [&](const FVector& W, const FLinearColor& Col, const FString& Label)
	{
		FVector2D Raw;
		if (!ProjectRaw(W, Raw))
		{
			return;
		}
		const FVector2D C(BoxX + BoxW * 0.5f, BoxY + BoxH * 0.5f);
		const FVector2D D = Raw - C;
		if (D.IsNearlyZero())
		{
			return;
		}
		const float HalfW = BoxW * 0.5f - 18.0f, HalfH = BoxH * 0.5f - 18.0f;
		const float T = FMath::Min(FMath::Abs(D.X) > 1.e-3f ? HalfW / FMath::Abs(D.X) : 1.e6f,
			FMath::Abs(D.Y) > 1.e-3f ? HalfH / FMath::Abs(D.Y) : 1.e6f);
		const FVector2D E = C + D * T;
		const FVector2D N = D.GetSafeNormal();
		const FVector2D Perp(-N.Y, N.X);
		const FVector2D Tip = E + N * 8.0f, B1 = E - N * 4.0f + Perp * 7.0f, B2 = E - N * 4.0f - Perp * 7.0f;
		DrawLine(Tip.X, Tip.Y, B1.X, B1.Y, Col, 2.0f);
		DrawLine(B1.X, B1.Y, B2.X, B2.Y, Col, 2.0f);
		DrawLine(B2.X, B2.Y, Tip.X, Tip.Y, Col, 2.0f);
		float LW = 0.0f, LH = 0.0f;
		GetTextSize(Label, LW, LH, HudType::Font(), HudType::Caption);
		const float LX = FMath::Clamp(E.X - N.X * 16.0f - (N.X > 0.3f ? LW : N.X < -0.3f ? 0.0f : LW * 0.5f), BoxX + 6.0f, BoxX + BoxW - LW - 6.0f);
		const float LY = FMath::Clamp(E.Y - N.Y * 16.0f - LH * 0.5f, BoxY + 6.0f, BoxY + BoxH - LH - 6.0f);
		DrawRect(BoxFill, LX - 3.0f, LY - 1.0f, LW + 6.0f, LH + 2.0f);
		DrawText(Label, Col, LX, LY, HudType::Font(), HudType::Caption);
	};

	TArray<AJumpGate*> Gates;
	for (TActorIterator<AJumpGate> It(World); It; ++It)
	{
		Gates.Add(*It);
		const FLinearColor GateCol = It->IsOnline() ? GateOnline : GateOffline;
		FVector2D SP;
		if (!Project(It->GetActorLocation(), SP))
		{
			DrawEdgeMarker(It->GetActorLocation(), GateCol, FString::Printf(TEXT("%s  %s"), *It->GetDisplayName(),
				*FormatDistance(FVector::Dist(ShipPos, It->GetActorLocation()))));
			continue;
		}
		DrawStem(It->GetActorLocation(), SP, GateCol);
		DrawCircleOutline(SP.X, SP.Y, 8.0f, GateCol, 2.0f, 12);
		Labels.Add({ It->GetDisplayName(), GateCol, FVector2D(SP.X + 11.0f, SP.Y - 6.0f), 0 });
		SectorMapPicks.Add({ SP, *It });
	}

	// ---- Place the queued labels ----
	{
		Labels.StableSort([](const FMapLabel& A, const FMapLabel& B) { return A.Priority < B.Priority; });
		TArray<FBox2D> Placed;
		for (const FMapLabel& Lbl : Labels)
		{
			float LW = 0.0f, LH = 0.0f;
			GetTextSize(Lbl.Text, LW, LH, HudType::Font(), HudType::Caption);
			for (const float Nudge : { 0.0f, 12.0f, -12.0f, 24.0f, -24.0f })
			{
				const FBox2D Box(FVector2D(Lbl.At.X - 1.0f, Lbl.At.Y + Nudge), FVector2D(Lbl.At.X + LW + 1.0f, Lbl.At.Y + Nudge + LH));
				if (Box.Max.X > BoxX + BoxW || Box.Min.Y < BoxY || Box.Max.Y > BoxY + BoxH)
				{
					continue;
				}
				if (!Placed.ContainsByPredicate([&Box](const FBox2D& Other) { return Box.Intersect(Other); }))
				{
					Placed.Add(Box);
					DrawText(Lbl.Text, Lbl.Color, Lbl.At.X, Lbl.At.Y + Nudge, HudType::Font(), HudType::Caption);
					break;
				}
			}
		}
	}

	// ---- Locked target: bracket + a dashed line from you with the range ----
	AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC);
	AActor* Target = AdPC ? AdPC->GetLockedTarget() : nullptr;
	if (Target)
	{
		FVector2D TP;
		if (Project(Target->GetActorLocation(), TP))
		{
			const float R = 11.0f;
			DrawLine(TP.X - R, TP.Y - R, TP.X + R, TP.Y - R, You, 2.0f);
			DrawLine(TP.X + R, TP.Y - R, TP.X + R, TP.Y + R, You, 2.0f);
			DrawLine(TP.X + R, TP.Y + R, TP.X - R, TP.Y + R, You, 2.0f);
			DrawLine(TP.X - R, TP.Y + R, TP.X - R, TP.Y - R, You, 2.0f);
		}
		else
		{
			DrawEdgeMarker(Target->GetActorLocation(), You, FString::Printf(TEXT("TARGET  %s"),
				*FormatDistance(FVector::Dist(ShipPos, Target->GetActorLocation()))));
		}
		FVector2D PP, RawT;
		if (ProjectRaw(ShipPos, PP) && ProjectRaw(Target->GetActorLocation(), RawT))
		{
			// Clipped to the box, so an off-screen target still shows its direction.
			const FLinearColor LineCol(You.R, You.G, You.B, 0.55f);
			const float Len = FVector2D::Distance(PP, RawT);
			const FVector2D Dir = Len > 1.0f ? (RawT - PP) / Len : FVector2D::ZeroVector;
			for (float D = 14.0f; D < Len - 14.0f; D += 14.0f)
			{
				DrawGridLine(PP + Dir * D, PP + Dir * FMath::Min(D + 8.0f, Len - 14.0f), LineCol, 1.5f);
			}
			const FVector2D Mid = (PP + RawT) * 0.5f;
			if (Mid.X > BoxX && Mid.X < BoxX + BoxW - 60.0f && Mid.Y > BoxY && Mid.Y < BoxY + BoxH - 16.0f)
			{
				const FString Range = FormatDistance(FVector::Dist(ShipPos, Target->GetActorLocation()));
				float RW = 0.0f, RH = 0.0f;
				GetTextSize(Range, RW, RH, HudType::Font(), HudType::Caption);
				DrawRect(BoxFill, Mid.X - 3.0f, Mid.Y - 1.0f, RW + 6.0f, RH + 2.0f);
				DrawText(Range, You, Mid.X, Mid.Y, HudType::Font(), HudType::Caption);
			}
		}
	}

	// ---- Player marker: teal arrow pointing along the ship's heading, "YOU" ----
	{
		FVector2D PPt;
		if (Project(ShipPos, PPt))
		{
			const APawn* Pawn = PC->GetPawn();
			FVector2D Dir(0.0f, -1.0f);
			FVector2D Ahead;
			if (Pawn && ProjectRaw(ShipPos + Pawn->GetActorForwardVector() * MapZoom * 0.05f, Ahead)
				&& FVector2D::Distance(Ahead, PPt) > 2.0f)
			{
				Dir = (Ahead - PPt).GetSafeNormal();
			}
			const FVector2D Perp(-Dir.Y, Dir.X);
			const float R = 10.0f;
			const FVector2D Tip = PPt + Dir * R;
			const FVector2D BaseL = PPt - Dir * R * 0.6f + Perp * R * 0.7f;
			const FVector2D BaseR = PPt - Dir * R * 0.6f - Perp * R * 0.7f;
			const FVector2D Notch = PPt - Dir * R * 0.2f;
			DrawStem(ShipPos, PPt, You);
			DrawLine(Tip.X, Tip.Y, BaseL.X, BaseL.Y, You, 2.0f);
			DrawLine(Tip.X, Tip.Y, BaseR.X, BaseR.Y, You, 2.0f);
			DrawLine(BaseL.X, BaseL.Y, Notch.X, Notch.Y, You, 2.0f);
			DrawLine(BaseR.X, BaseR.Y, Notch.X, Notch.Y, You, 2.0f);
			DrawText(TEXT("YOU"), You, PPt.X - 7.0f, PPt.Y + 13.0f, HudType::Font(), HudType::Caption);
		}
	}

	// Short type line for an object on the map ("Station", "Jump gate to X", ...).
	auto DescribeActor = [](const AActor* A) -> FString
	{
		if (const AJumpGate* G = Cast<AJumpGate>(A))
		{
			return G->IsOnline() ? FString::Printf(TEXT("Jump gate to %s"), *G->GetDestinationName())
				: FString::Printf(TEXT("Jump gate to %s (offline)"), *G->GetDestinationName());
		}
		if (A->IsA<ASpaceStation>()) { return TEXT("Station"); }
		if (A->IsA<ASpaceship>()) { return TEXT("Ship"); }
		if (A->IsA<AAsteroid>()) { return TEXT("Asteroid"); }
		return TEXT("Object");
	};

	// ---- Hover: ring the object under the cursor and show its name, type and range ----
	if (AActor* Hovered = bMouse ? PickSectorMapActor(Mouse) : nullptr)
	{
		FVector2D HP;
		if (Project(Hovered->GetActorLocation(), HP))
		{
			DrawCircleOutline(HP.X, HP.Y, 14.0f, Select, 1.5f, 24);
			const FString Name = HudActorName(Hovered);
			const FString Info = FString::Printf(TEXT("%s   %s"), *DescribeActor(Hovered),
				*FormatDistance(FVector::Dist(ShipPos, Hovered->GetActorLocation())));
			const FString Hint = Hovered == Target ? FString(TEXT("Targeted")) : FString(TEXT("Click to target"));
			float NW = 0, NH = 0, IW = 0, IH = 0, HW = 0, HH = 0;
			GetTextSize(Name, NW, NH, HudType::Font(), HudType::Label);
			GetTextSize(Info, IW, IH, HudType::Font(), HudType::Caption);
			GetTextSize(Hint, HW, HH, HudType::Font(), HudType::Caption);
			const float TipW = FMath::Max3(NW, IW, HW) + 16.0f;
			const float TipH = NH + IH + HH + 14.0f;
			float TipX = HP.X + 18.0f, TipY = HP.Y + 14.0f;
			if (TipX + TipW > BoxX + BoxW) { TipX = HP.X - 18.0f - TipW; }
			if (TipY + TipH > BoxY + BoxH) { TipY = HP.Y - 14.0f - TipH; }
			DrawRect(FLinearColor(0.02f, 0.035f, 0.05f, 0.95f), TipX, TipY, TipW, TipH);
			DrawRect(BoxEdge, TipX, TipY, 2.0f, TipH);
			DrawText(Name, FLinearColor::White, TipX + 8.0f, TipY + 5.0f, HudType::Font(), HudType::Label);
			DrawText(Info, Body, TipX + 8.0f, TipY + 7.0f + NH, HudType::Font(), HudType::Caption);
			DrawText(Hint, Dim, TipX + 8.0f, TipY + 9.0f + NH + IH, HudType::Font(), HudType::Caption);
		}
	}

	// Grid scale (bottom-left of the 3D view).
	DrawText(FString::Printf(TEXT("Grid  %s"), *FormatDistance(GridStep)), Dim, BoxX + 16.0f, BoxY + BoxH - 26.0f, HudType::Font(), HudType::Caption);

	// ---- Info panel ----
	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	const FGalaxySectorDef* Sector = Galaxy ? Galaxy->FindSector(GetMapCurrentSectorId()) : nullptr;
	const FStarSystemDef* System = Galaxy ? Galaxy->FindSystem(GetMapCurrentSystemId()) : nullptr;
	{
		const float PX = Lay.PanelX, PW = Lay.PanelWidth, TX = PX + 14.0f, TW = PW - 28.0f;
		const FLinearColor Divider(0.2f, 0.4f, 0.5f, 0.5f);
		DrawMapInfoPanel(PX, Lay.PanelY, PW, Lay.PanelH,
			Sector ? Sector->Name.ToString().ToUpper() : FString(TEXT("UNASSIGNED LEVEL")), Sector ? Built : BoxEdge);
		float Y = Lay.PanelY + 44.0f;
		if (Sector)
		{
			DrawText(FString::Printf(TEXT("%s   Security: %s"), *Sector->Type, Sector->Security.IsEmpty() ? TEXT("?") : *Sector->Security), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			if (!Sector->Faction.IsEmpty()) { DrawText(FString::Printf(TEXT("Faction  %s"), *Sector->Faction), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f; }
		}
		if (System)
		{
			DrawText(FString::Printf(TEXT("System  %s  [%s]"), *System->Name.ToString(), *System->StarClass), Body, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
		}
		FString Counts = FString::Printf(TEXT("Stations %d   Ships %d   Gates %d"), Stations.Num(), NumOtherShips, Gates.Num());
		if (NumAsteroids > 0) { Counts += FString::Printf(TEXT("   Asteroids %d"), NumAsteroids); }
		Y = DrawWrappedText(Counts, Body, TX, Y, TW, HudType::Font(), HudType::Label);
		Y += 8.0f;
		DrawRect(Divider, TX, Y, TW, 1.0f);
		Y += 10.0f;

		// Target.
		DrawText(TEXT("TARGET"), GalaxyMap::Title, TX, Y, HudType::Font(), HudType::Label); Y += 18.0f;
		if (Target)
		{
			DrawText(HudActorName(Target), FLinearColor::White, TX, Y, HudType::Font(), HudType::Body); Y += 22.0f;
			Y = DrawWrappedText(DescribeActor(Target), Body, TX, Y, TW, HudType::Font(), HudType::Label);
			DrawText(FString::Printf(TEXT("Range  %s"), *FormatDistance(FVector::Dist(ShipPos, Target->GetActorLocation()))), You, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			const float DzM = (Target->GetActorLocation().Z - ShipPos.Z) / 100.0f;
			if (FMath::Abs(DzM) >= 10.0f)
			{
				DrawText(FString::Printf(TEXT("%.0f m %s you"), FMath::Abs(DzM), DzM > 0.0f ? TEXT("above") : TEXT("below")), Dim, TX, Y, HudType::Font(), HudType::Label); Y += 16.0f;
			}
		}
		else
		{
			Y = DrawWrappedText(TEXT("None. Click an object on the map to target it."), Dim, TX, Y, TW, HudType::Font(), HudType::Label);
		}
		Y += 8.0f;
		DrawRect(Divider, TX, Y, TW, 1.0f);
		Y += 10.0f;

		// Jump gates out of the sector, nearest first.
		const float LegendTop = Lay.PanelY + Lay.PanelH - 116.0f;
		DrawText(TEXT("JUMP GATES"), GalaxyMap::Title, TX, Y, HudType::Font(), HudType::Label); Y += 18.0f;
		Gates.Sort([&ShipPos](const AJumpGate& A, const AJumpGate& B)
		{
			return FVector::DistSquared(ShipPos, A.GetActorLocation()) < FVector::DistSquared(ShipPos, B.GetActorLocation());
		});
		for (const AJumpGate* G : Gates)
		{
			if (Y > LegendTop - 30.0f)
			{
				DrawText(TEXT("..."), Dim, TX, Y, HudType::Font(), HudType::Caption);
				break;
			}
			const FLinearColor GateCol = G->IsOnline() ? GateOnline : GateOffline;
			DrawCircleOutline(TX + 5.0f, Y + 7.0f, 4.0f, GateCol, 1.5f, 10);
			DrawText(G->IsOnline() ? G->GetDestinationName() : FString::Printf(TEXT("%s (offline)"), *G->GetDestinationName()),
				Body, TX + 16.0f, Y, HudType::Font(), HudType::Caption);
			const FString Dist = FormatDistance(FVector::Dist(ShipPos, G->GetActorLocation()));
			float DW = 0, DH = 0;
			GetTextSize(Dist, DW, DH, HudType::Font(), HudType::Caption);
			DrawText(Dist, GateCol, TX + TW - DW, Y, HudType::Font(), HudType::Caption);
			Y += 16.0f;
		}
		if (Gates.Num() == 0)
		{
			DrawText(TEXT("None in this sector."), Dim, TX, Y, HudType::Font(), HudType::Caption);
		}

		// Legend + filters (panel bottom).
		const float LY = LegendTop;
		DrawRect(Divider, TX, LY - 8.0f, TW, 1.0f);
		DrawStationIcon(TX + 6.0f, LY + 7.0f, 5.0f, GalaxyMap::Station, 2.0f);
		DrawText(FString::Printf(TEXT("Station   [2] %s"), bShowStations ? TEXT("shown") : TEXT("hidden")), Body, TX + 20.0f, LY, HudType::Font(), HudType::Caption);
		DrawShipIcon(TX + 6.0f, LY + 25.0f, 5.0f, GalaxyMap::Ship, 2.0f);
		DrawText(FString::Printf(TEXT("Ship   [1] %s"), bShowShips ? TEXT("shown") : TEXT("hidden")), Body, TX + 20.0f, LY + 18.0f, HudType::Font(), HudType::Caption);
		DrawCircleOutline(TX + 6.0f, LY + 43.0f, 5.0f, GateOnline, 2.0f, 12);
		DrawText(TEXT("Jump gate (amber: offline)"), Body, TX + 20.0f, LY + 36.0f, HudType::Font(), HudType::Caption);
		DrawRect(GalaxyMap::Asteroid, TX + 4.5f, LY + 59.5f, 3.0f, 3.0f);
		DrawText(TEXT("Asteroid"), Body, TX + 20.0f, LY + 54.0f, HudType::Font(), HudType::Caption);
		DrawLine(TX + 6.0f, LY + 73.0f, TX + 1.0f, LY + 83.0f, You, 2.0f);
		DrawLine(TX + 6.0f, LY + 73.0f, TX + 11.0f, LY + 83.0f, You, 2.0f);
		DrawText(TEXT("You (points along your heading)"), Body, TX + 20.0f, LY + 72.0f, HudType::Font(), HudType::Caption);
		const FLinearColor StemCol(0.6f, 0.7f, 0.8f, 0.6f);
		DrawLine(TX + 6.0f, LY + 90.0f, TX + 6.0f, LY + 101.0f, StemCol, 1.0f);
		DrawLine(TX + 2.0f, LY + 101.0f, TX + 10.0f, LY + 101.0f, StemCol, 1.0f);
		DrawText(TEXT("Height above / below the grid"), Body, TX + 20.0f, LY + 90.0f, HudType::Font(), HudType::Caption);
	}

	// ---- Header and controls help ----
	{
		const FString SectorLabel = Sector ? Sector->Name.ToString() : TEXT("Unassigned level");
		const FString SystemLabel = System ? System->Name.ToString() : TEXT("?");
		DrawMapHeader(PC, VW, TEXT("SECTOR MAP"),
			FString::Printf(TEXT("Universe  >  %s  >  %s"), *SystemLabel, *SectorLabel));
	}
	DrawCentredText(TEXT("LMB target   [Arrows] orbit   [=/-] zoom   [C] recenter   [1] ships   [2] stations   [4] system   [5]/[U] universe   [M] close"),
		Help, VW * 0.5f, VH - 30.0f, HudType::Label);
}

AActor* AAdastreaHUD::PickSectorMapActor(const FVector2D& ScreenPos, float Radius) const
{
	// Nearest icon within Radius. Picks are recorded in draw order (asteroids first),
	// so a station, ship or gate wins a near-tie against an asteroid drawn under it.
	AActor* Best = nullptr;
	float BestD = Radius * Radius;
	for (const FSectorMapPick& Pick : SectorMapPicks)
	{
		AActor* A = Pick.Actor.Get();
		const float D = FVector2D::DistSquared(Pick.Screen, ScreenPos);
		if (A && D <= BestD + 16.0f)
		{
			Best = A;
			BestD = FMath::Min(D, BestD);
		}
	}
	return Best;
}

namespace
{
	enum class EStationMenuAction : uint8 { Trading, Outfitting, Production, WalkStation, Maintenance, Habitation, Undock };

	struct FStationMenuOption
	{
		EStationMenuAction Action;
		const TCHAR* Label;
		const TCHAR* Blurb;
	};

	// Each option leads to a different part of the station. Outfitting is only
	// available where the station has an outfitting module (checked when drawn/used).
	const FStationMenuOption kStationMenuOptions[] =
	{
		{ EStationMenuAction::Trading,     TEXT("Trading Department"), TEXT("Buy and sell goods at the station market") },
		{ EStationMenuAction::Outfitting,  TEXT("Outfitting Bay"),     TEXT("Buy, fit and sell engine, weapon, shield, hull and cargo upgrades") },
		{ EStationMenuAction::Production,  TEXT("Production Floor"),   TEXT("Refine ore and fabricate parts at the station's modules") },
		{ EStationMenuAction::WalkStation, TEXT("Walk the Station"),   TEXT("Leave your ship and explore the station on foot") },
		{ EStationMenuAction::Maintenance, TEXT("Maintenance Dock"),   TEXT("Walk to the hangar bay: repairs and the ship refit kiosk") },
		{ EStationMenuAction::Habitation,  TEXT("Habitation"),         TEXT("Walk to the crew cabins and lounge") },
		{ EStationMenuAction::Undock,      TEXT("Undock"),             TEXT("Leave the station and return to flight") },
	};
	constexpr int32 kStationMenuCount = UE_ARRAY_COUNT(kStationMenuOptions);
}

void AAdastreaHUD::MoveStationMenuSelection(int32 Step)
{
	StationMenuIndex = (StationMenuIndex + Step + kStationMenuCount) % kStationMenuCount;
}

void AAdastreaHUD::UndockFromStationMenu(APlayerController* PC)
{
	bShowStationMenu = false;
	if (ASpaceship* Ship = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr)
	{
		Ship->Undock();
	}
}

void AAdastreaHUD::ConfirmStationMenuSelection(APlayerController* PC)
{
	const int32 Index = FMath::Clamp(StationMenuIndex, 0, kStationMenuCount - 1);
	const FStationMenuOption& Option = kStationMenuOptions[Index];
	UE_LOG(LogTemp, Log, TEXT("StationMenu: selected '%s'"), Option.Label);

	if (Option.Action == EStationMenuAction::Outfitting && !IsOutfittingAvailable(PC))
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Error"), 0.2f);
		ShowMessage(TEXT("This station has no outfitting modules"), 3.0f, true);
		return;
	}
	if (Option.Action == EStationMenuAction::Production && !IsCraftingAvailable(PC))
	{
		UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Error"), 0.2f);
		ShowMessage(TEXT("This station has no processing or fabrication modules"), 3.0f, true);
		return;
	}
	UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"), 0.08f);

	switch (Option.Action)
	{
	case EStationMenuAction::Trading:
		bShowStationMenu = false;
		ShowTradeScreen();
		break;
	case EStationMenuAction::Outfitting:
		ShowOutfitting();
		break;
	case EStationMenuAction::Production:
		ShowCrafting();
		break;
	case EStationMenuAction::WalkStation:
		if (AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
		{
			if (ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn()))
			{
				AdPC->EnterStationInterior(Ship);
			}
		}
		break;
	case EStationMenuAction::Maintenance:
	case EStationMenuAction::Habitation:
		if (AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(PC))
		{
			if (ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn()))
			{
				AdPC->EnterStationRoom(Ship, Option.Action == EStationMenuAction::Maintenance ? EStationRoom::Maintenance : EStationRoom::Habitation);
			}
		}
		break;
	case EStationMenuAction::Undock:
		UndockFromStationMenu(PC);
		break;
	default:
		break;
	}
}

void AAdastreaHUD::DrawStationMenu(APlayerController* PC, AAdastreaPlayerController* AdController, ASpaceship* Ship)
{
	const float VW = Canvas->SizeX;
	const float VH = Canvas->SizeY;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.02f, 0.72f), 0.0f, 0.0f, VW, VH);

	const float PanelW = 520.0f;
	const float RowH   = 64.0f;
	const float PanelH = 120.0f + RowH * kStationMenuCount + 44.0f;
	const float X = (VW - PanelW) * 0.5f;
	const float Y = (VH - PanelH) * 0.5f;

	DrawRect(kBg, X, Y, PanelW, PanelH);
	DrawLine(X, Y, X, Y + PanelH, kBorder, 3.0f);

	const ASpaceStation* Station = AdController ? AdController->GetNearestStation() : nullptr;
	DrawText(TEXT("STATION SERVICES"), kHeader, X + 24.0f, Y + 16.0f, HudType::Font(), HudType::Title);
	DrawText(Station ? HudActorName(Station) : FString(TEXT("Docked")), kLabel, X + 24.0f, Y + 52.0f, HudType::Font(), HudType::Body);
	DrawLine(X + 16.0f, Y + 80.0f, X + PanelW - 16.0f, Y + 80.0f, kBorder, 1.0f);

	const FLinearColor Accent(0.15f, 0.9f, 0.6f, 1.0f);
	const bool bOutfitting = IsOutfittingAvailable(PC);
	const bool bCrafting = IsCraftingAvailable(PC);
	for (int32 i = 0; i < kStationMenuCount; ++i)
	{
		const FStationMenuOption& Option = kStationMenuOptions[i];
		const bool bAvailable = (Option.Action != EStationMenuAction::Outfitting || bOutfitting)
			&& (Option.Action != EStationMenuAction::Production || bCrafting);
		const float RowY = Y + 92.0f + RowH * i;
		const bool bSel = (i == StationMenuIndex);
		if (bSel)
		{
			DrawRect(FLinearColor(0.15f, 0.9f, 0.6f, 0.18f), X + 12.0f, RowY, PanelW - 24.0f, RowH - 6.0f);
			DrawLine(X + 12.0f, RowY, X + 12.0f, RowY + RowH - 6.0f, Accent, 3.0f);
		}
		const FLinearColor LabelCol = !bAvailable ? FLinearColor(0.5f, 0.5f, 0.55f, 1.0f)
			: (bSel ? Accent : FLinearColor::White);
		DrawText(FString(Option.Label) + (bAvailable ? TEXT("") : TEXT("   (none at this station)")),
			LabelCol, X + 28.0f, RowY + 8.0f, HudType::Font(), HudType::Heading);
		DrawText(Option.Blurb, kLabel, X + 28.0f, RowY + 34.0f, HudType::Font(), HudType::Label);
	}

	DrawText(TEXT("[UP/DOWN] select     [ENTER] confirm     [ESC/BACKSPACE] back / undock"), kLabel,
		X + 24.0f, Y + PanelH - 30.0f, HudType::Font(), HudType::Label);
}

// ---------------------------------------------------------------------------
// Pause menu
// ---------------------------------------------------------------------------

namespace
{
	// Rows: Resume, the three volume sliders, Quit. No music slider (music is a later session).
	enum class EPauseRow : int32 { Resume = 0, Master, SFX, UI, Quit, Count };
	constexpr int32 kPauseRowCount = static_cast<int32>(EPauseRow::Count);
	constexpr float kVolumeStep = 0.05f;

	bool PauseRowCategory(int32 Row, EAdastreaVolumeCategory& Out)
	{
		switch (static_cast<EPauseRow>(Row))
		{
		case EPauseRow::Master: Out = EAdastreaVolumeCategory::Master; return true;
		case EPauseRow::SFX:    Out = EAdastreaVolumeCategory::SFX; return true;
		case EPauseRow::UI:     Out = EAdastreaVolumeCategory::UI; return true;
		default: return false;
		}
	}
}

void AAdastreaHUD::MovePauseMenuSelection(int32 Step)
{
	PauseMenuIndex = (PauseMenuIndex + Step + kPauseRowCount) % kPauseRowCount;
	UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Hover"));
}

void AAdastreaHUD::AdjustPauseMenuValue(int32 Direction)
{
	EAdastreaVolumeCategory Category;
	if (!PauseRowCategory(PauseMenuIndex, Category))
	{
		return;
	}
	const float Current = UAdastreaAudioSettings::Get()->GetVolume(Category);
	// Snap to the 5% grid so repeated presses land on round numbers.
	const float Next = FMath::Clamp(FMath::RoundToFloat(Current / kVolumeStep + Direction) * kVolumeStep, 0.0f, 1.0f);
	if (UAudioMixSubsystem* Mix = UAudioMixSubsystem::Get(this))
	{
		Mix->SetCategoryVolume(Category, Next);
	}
	else
	{
		UAdastreaAudioSettings::Get()->SetVolume(Category, Next);
		UAdastreaAudioSettings::Get()->Save();
	}
	// Played after the change, so the UI slider previews its own new level.
	UAudioEventLibrary::PlayEvent2D(this, TEXT("UI.Click"));
}

bool AAdastreaHUD::ConfirmPauseMenuSelection(APlayerController* PC)
{
	switch (static_cast<EPauseRow>(FMath::Clamp(PauseMenuIndex, 0, kPauseRowCount - 1)))
	{
	case EPauseRow::Resume:
		return true;
	case EPauseRow::Quit:
		UE_LOG(LogTemp, Log, TEXT("PauseMenu: quit"));
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
		return true;
	default:
		return false;
	}
}

void AAdastreaHUD::DrawPauseMenu(APlayerController* PC)
{
	const float VW = Canvas->SizeX;
	const float VH = Canvas->SizeY;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.02f, 0.72f), 0.0f, 0.0f, VW, VH);

	const float PanelW = 560.0f;
	const float RowH   = 56.0f;
	const float PanelH = 110.0f + RowH * kPauseRowCount + 44.0f;
	const float X = (VW - PanelW) * 0.5f;
	const float Y = (VH - PanelH) * 0.5f;

	DrawRect(kBg, X, Y, PanelW, PanelH);
	DrawLine(X, Y, X, Y + PanelH, kBorder, 3.0f);
	DrawText(TEXT("PAUSED"), kHeader, X + 24.0f, Y + 16.0f, HudType::Font(), HudType::Title);
	DrawText(TEXT("Audio"), kLabel, X + 24.0f, Y + 50.0f, HudType::Font(), HudType::Body);
	DrawLine(X + 16.0f, Y + 76.0f, X + PanelW - 16.0f, Y + 76.0f, kBorder, 1.0f);

	const FLinearColor Accent(0.15f, 0.9f, 0.6f, 1.0f);
	const UAdastreaAudioSettings* Settings = UAdastreaAudioSettings::Get();
	for (int32 i = 0; i < kPauseRowCount; ++i)
	{
		const float RowY = Y + 88.0f + RowH * i;
		const bool bSel = (i == PauseMenuIndex);
		if (bSel)
		{
			DrawRect(FLinearColor(0.15f, 0.9f, 0.6f, 0.18f), X + 12.0f, RowY, PanelW - 24.0f, RowH - 6.0f);
			DrawLine(X + 12.0f, RowY, X + 12.0f, RowY + RowH - 6.0f, Accent, 3.0f);
		}
		const FLinearColor LabelCol = bSel ? Accent : FLinearColor::White;

		EAdastreaVolumeCategory Category;
		if (PauseRowCategory(i, Category))
		{
			const float Value = Settings->GetVolume(Category);
			DrawText(FString::Printf(TEXT("%s volume"), UAdastreaAudioSettings::GetCategoryLabel(Category)),
				LabelCol, X + 28.0f, RowY + 14.0f, HudType::Font(), HudType::Heading);

			// Slider track + fill + percentage.
			const float TrackX = X + 250.0f;
			const float TrackW = 220.0f;
			const float TrackY = RowY + 22.0f;
			DrawRect(FLinearColor(0.25f, 0.3f, 0.35f, 0.9f), TrackX, TrackY, TrackW, 6.0f);
			DrawRect(bSel ? Accent : kBorder, TrackX, TrackY, TrackW * Value, 6.0f);
			DrawText(FString::Printf(TEXT("%3.0f%%"), Value * 100.0f), LabelCol, TrackX + TrackW + 14.0f, RowY + 14.0f, HudType::Font(), HudType::Body);
		}
		else
		{
			const TCHAR* Label = (static_cast<EPauseRow>(i) == EPauseRow::Resume) ? TEXT("Resume") : TEXT("Quit to desktop");
			DrawText(Label, LabelCol, X + 28.0f, RowY + 14.0f, HudType::Font(), HudType::Heading);
		}
	}

	DrawText(TEXT("[UP/DOWN] select     [LEFT/RIGHT] adjust     [ENTER] confirm     [ESC/F10] resume"), kLabel,
		X + 24.0f, Y + PanelH - 30.0f, HudType::Font(), HudType::Label);
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
		DrawText(TEXT("No active market at this dock."), FLinearColor(0.9f,0.6f,0.4f,1.0f), 200.0f, 200.0f, HudType::Font(), HudType::Body);
		return;
	}
	UMarketDataAsset* Market = Station->GetMarketplaceModule()->GetMarketData();
	if (!Market)
	{
		DrawText(TEXT("Station market is not configured."), FLinearColor(0.9f,0.6f,0.4f,1.0f), 200.0f, 200.0f, HudType::Font(), HudType::Body);
		return;
	}

	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;

	// Full-screen dim backdrop.
	DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.94f), 0.0f, 0.0f, VW, VH);


	// Credits + cargo header row.
	const int32 Credits = Ship->PlayerTraderComponent ? Ship->PlayerTraderComponent->GetCredits() : 0;
	const float CargoUsed = Ship->CargoComponent ? (Ship->CargoComponent->CargoCapacity - Ship->CargoComponent->GetAvailableCargoSpace()) : 0.0f;
	const float CargoMax = Ship->CargoComponent ? FMath::Max(Ship->CargoComponent->CargoCapacity, 0.01f) : 1.0f;

	DrawCentredText(TEXT("TRADING"), FLinearColor(0.15f,0.9f,0.6f,1.0f), VW * 0.5f, 18.0f, HudType::Title);
	DrawCentredText(Market->GetName(), FLinearColor(0.8f,0.9f,1.0f,1.0f), VW * 0.5f, 52.0f, HudType::Body);
	DrawText(FString::Printf(TEXT("CREDITS: %d"), Credits), FLinearColor(0.95f,0.78f,0.30f,1.0f), 40.0f, 40.0f, HudType::Font(), HudType::Body);
	DrawText(FString::Printf(TEXT("CARGO: %.0f / %.0f"), CargoUsed, CargoMax), FLinearColor(0.15f,0.9f,0.6f,1.0f), 40.0f, 66.0f, HudType::Font(), HudType::Body);
	DrawText(bBuyMode ? TEXT("MODE: BUY") : TEXT("MODE: SELL"),
		bBuyMode ? FLinearColor(0.3f,0.9f,0.4f,1.0f) : FLinearColor(0.9f,0.5f,0.3f,1.0f),
		VW-220.0f, 40.0f, HudType::Font(), HudType::Body);

	// Column headers.
	const float ListX = 60.0f;
	const float PriceX = VW*0.45f;
	const float StockX = VW*0.68f;
	DrawText(TEXT("ITEM"), FLinearColor(0.6f,0.7f,0.8f,1.0f), ListX, 100.0f, HudType::Font(), HudType::Label);
	DrawText(TEXT("PRICE"), FLinearColor(0.6f,0.7f,0.8f,1.0f), PriceX, 100.0f, HudType::Font(), HudType::Label);
	DrawText(TEXT("STOCK"), FLinearColor(0.6f,0.7f,0.8f,1.0f), StockX, 100.0f, HudType::Font(), HudType::Label);

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
		DrawText(ItemName, NameCol, ListX, RowY, HudType::Font(), HudType::Body);
		DrawText(FString::Printf(TEXT("%d cr"), ItemPrice), FLinearColor(0.95f,0.78f,0.30f,1.0f), PriceX, RowY, HudType::Font(), HudType::Body);
		DrawText(FString::Printf(TEXT("%d"), Entry.CurrentStock), FLinearColor(0.7f,0.8f,0.9f,1.0f), StockX, RowY, HudType::Font(), HudType::Body);
		RowY += RowH;
	}

	// Footer controls.
	DrawCentredText(TEXT("Up/Down: select    [B]/[S]: toggle Buy/Sell    [Space]: trade 1    [Esc]: close    [Q]: trade 5    [X]: sell all"), FLinearColor(0.6f,0.7f,0.8f,0.9f), VW * 0.5f, VH - 40.0f, HudType::Label);
}

// ========================================================================
// SHIP SELECT SCREEN (concept prototype — canvas + SceneCapture2D preview)
// ========================================================================

// Ship roster: every ASpaceship Blueprint in ShipRosterFolder that has a hull mesh,
// smallest first. Each entry's preview mesh, hull material and stats are read from
// its own class defaults, so the screen can't drift from what the ship flies (the
// old hand-kept parallel arrays had the Cruiser and Destroyer showing other ships).
static const TCHAR* ShipRosterFolder = TEXT("/Game/Blueprints/Ships");

// Blueprints in that folder that are not player ships (dev/test pawns).
static const TCHAR* ShipRosterExcluded[] = { TEXT("BP_TestShip") };

static float ShipRosterMeshLength(const UStaticMeshComponent* Mesh)
{
	if (!Mesh || !Mesh->GetStaticMesh())
	{
		return 0.0f;
	}
	return (Mesh->GetStaticMesh()->GetBounds().BoxExtent * 2.0f * Mesh->GetRelativeScale3D().GetAbs()).GetMax();
}

// The component that carries a ship class's hull: the native ShipMeshComponent, or
// (BP_CommandXL, BP_Super) the largest mesh the Blueprint adds itself, found on its
// construction-script templates (kept in cooked builds too).
static const UStaticMeshComponent* ShipRosterHull(const UClass* ShipClass)
{
	const ASpaceship* Ship = ShipClass ? GetDefault<ASpaceship>(const_cast<UClass*>(ShipClass)) : nullptr;
	if (!Ship)
	{
		return nullptr;
	}
	if (Ship->ShipMeshComponent && Ship->ShipMeshComponent->GetStaticMesh())
	{
		return Ship->ShipMeshComponent;
	}
	const UStaticMeshComponent* Best = nullptr;
	for (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(ShipClass); BPClass;
		BPClass = Cast<UBlueprintGeneratedClass>(BPClass->GetSuperClass()))
	{
		if (!BPClass->SimpleConstructionScript)
		{
			continue;
		}
		for (const USCS_Node* Node : BPClass->SimpleConstructionScript->GetAllNodes())
		{
			const UStaticMeshComponent* Mesh = Node ? Cast<UStaticMeshComponent>(Node->ComponentTemplate) : nullptr;
			if (ShipRosterMeshLength(Mesh) > ShipRosterMeshLength(Best))
			{
				Best = Mesh;
			}
		}
	}
	return Best;
}

// Hull length (cm) as flown: mesh bounds times the Blueprint's mesh scale.
static float ShipRosterLength(const ASpaceship* Ship)
{
	return Ship ? ShipRosterMeshLength(ShipRosterHull(Ship->GetClass())) : 0.0f;
}

// X4-style list groups. X4's S/M/L/XL split doesn't fit this roster (every hull is
// 7-60 m), so ships group by purpose, read from the data asset's class ("Mining
// Barge", "Escort Carrier"); support roles are tested first so a "Medical Cruiser"
// isn't filed under combat.
static int32 ShipPurpose(const ASpaceship* Ship)
{
	const FString Role = Ship && Ship->ShipDataAsset ? Ship->ShipDataAsset->ShipClass.ToString() : FString();
	auto Has = [&Role](std::initializer_list<const TCHAR*> Words)
	{
		for (const TCHAR* Word : Words)
		{
			if (Role.Contains(Word)) { return true; }
		}
		return false;
	};
	if (Has({ TEXT("Mining"), TEXT("Salvage"), TEXT("Utility"), TEXT("Construct") })) { return 2; }
	if (Has({ TEXT("Research"), TEXT("Science"), TEXT("Medical"), TEXT("Multi") })) { return 3; }
	if (Has({ TEXT("Trad"), TEXT("Freight"), TEXT("Transport"), TEXT("Liner"), TEXT("Colony"), TEXT("Cargo") })) { return 1; }
	if (Has({ TEXT("Interceptor"), TEXT("Fighter"), TEXT("Gunship"), TEXT("Patrol"), TEXT("Corvette"), TEXT("Frigate"),
		TEXT("Destroyer"), TEXT("Cruiser"), TEXT("Battleship"), TEXT("Carrier"), TEXT("Assault") })) { return 0; }
	return 3;
}
static const TCHAR* ShipPurposeNames[] = { TEXT("COMBAT"), TEXT("TRADE & TRANSPORT"), TEXT("MINING & SALVAGE"), TEXT("SUPPORT & SPECIAL") };

// The preview fills the screen: horizontal FOV, and a render target the viewport's
// shape (long edge capped so a 4K screen doesn't capture at 4K every frame).
static constexpr float ShipPreviewFOV = 40.0f;
static TAutoConsoleVariable<float> CVarShipPreviewExposure(
	TEXT("adastrea.ShipPreviewExposure"),
	-3.0f,
	TEXT("Exposure bias (stops) of the ship-select preview's fixed exposure."),
	ECVF_Default);
static FIntPoint ShipPreviewTargetSize(const APlayerController* PC)
{
	int32 X = 1280, Y = 720;
	if (PC)
	{
		PC->GetViewportSize(X, Y);
	}
	const float Scale = FMath::Min(1.0f, 1600.0f / FMath::Max(1, FMath::Max(X, Y)));
	return FIntPoint(FMath::Max(64, FMath::RoundToInt(X * Scale)), FMath::Max(64, FMath::RoundToInt(Y * Scale)));
}

void AAdastreaHUD::BuildShipRoster()
{
	ShipRoster.Reset();
	TArray<FAssetData> Assets;
	FAssetRegistryModule::GetRegistry().GetAssetsByPath(FName(ShipRosterFolder), Assets, /*bRecursive*/ false);
	for (const FAssetData& Asset : Assets)
	{
		const FString Name = Asset.AssetName.ToString();
		bool bExcluded = false;
		for (const TCHAR* Skip : ShipRosterExcluded)
		{
			bExcluded |= Name.Equals(Skip);
		}
		if (bExcluded)
		{
			continue;
		}
		// '<package>.<Name>_C' is the Blueprint's generated class (also in cooked builds).
		const FSoftClassPath ClassPath(Asset.PackageName.ToString() + TEXT(".") + Name + TEXT("_C"));
		UClass* ShipClass = ClassPath.TryLoadClass<ASpaceship>();
		if (!ShipClass || ShipClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}
		if (ShipRosterLength(Cast<ASpaceship>(ShipClass->GetDefaultObject())) <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShipSelect: %s has no hull mesh, left off the roster"), *Name);
			continue;
		}
		ShipRoster.Add(ShipClass);
	}
	// Grouped by purpose (the list's sections), smallest hull first within each.
	ShipRoster.Sort([](const TSubclassOf<ASpaceship>& A, const TSubclassOf<ASpaceship>& B)
	{
		const ASpaceship* ShipA = GetDefault<ASpaceship>(A);
		const ASpaceship* ShipB = GetDefault<ASpaceship>(B);
		const int32 PurposeA = ShipPurpose(ShipA), PurposeB = ShipPurpose(ShipB);
		return PurposeA != PurposeB ? PurposeA < PurposeB : ShipRosterLength(ShipA) < ShipRosterLength(ShipB);
	});
	UE_LOG(LogTemp, Log, TEXT("ShipSelect: roster of %d ships from %s"), ShipRoster.Num(), ShipRosterFolder);
}

const ASpaceship* AAdastreaHUD::GetRosterShip(int32 Index) const
{
	return ShipRoster.IsValidIndex(Index) && ShipRoster[Index] ? GetDefault<ASpaceship>(ShipRoster[Index]) : nullptr;
}

void AAdastreaHUD::ShowShipSelect()
{
	bShowShipSelect = true;
	BuildShipRoster();
	// Open on the ship being flown, if it is on the roster.
	ShipSelectIndex = 0;
	if (const APlayerController* OwnerPC = GetOwningPlayerController())
	{
		if (const APawn* Pawn = OwnerPC->GetPawn())
		{
			const UClass* Flying = Pawn->GetClass();
			ShipSelectIndex = FMath::Max(0, ShipRoster.IndexOfByPredicate(
				[Flying](const TSubclassOf<ASpaceship>& C) { return C.Get() == Flying; }));
		}
	}
	ShipSelectScroll = 0;
	ShipPreviewDistance = 6500.0f;
	bShipPreviewDragging = false;
	ShipPreviewLastTouched = -100.0;
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
	DestroyShipPreview();
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

void AAdastreaHUD::DestroyShipPreview()
{
	// Destroy the whole rig. Only the capture *component* used to be destroyed, so
	// every rebuild (each ship cycled) left a capture actor and a directional light
	// behind - and those lights lit the whole level.
	for (TObjectPtr<AActor>* Actor : { &ShipPreviewActor, &ShipPreviewCaptureActor, &ShipPreviewLight })
	{
		if (IsValid(*Actor))
		{
			(*Actor)->Destroy();
		}
		*Actor = nullptr;
	}
	ShipPreviewCapture = nullptr;
	ShipPreviewMeshComp = nullptr;
}

void AAdastreaHUD::RebuildShipPreview(APlayerController* PC)
{
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	UWorld* World = PC->GetWorld();

	// Tear down any existing preview.
	DestroyShipPreview();

	// This roster entry's class defaults: its hull mesh and material are previewed.
	const ASpaceship* Ship = GetRosterShip(ShipSelectIndex);
	if (!Ship)
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
		const UStaticMeshComponent* ShipMesh = ShipRosterHull(Ship->GetClass());
		UStaticMesh* PreviewMesh = ShipMesh ? ShipMesh->GetStaticMesh() : nullptr;
		ShipPreviewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PreviewLoc, FRotator::ZeroRotator);
		ShipPreviewMeshComp = nullptr;
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(ShipPreviewActor))
		{
			UStaticMeshComponent* SMComp = SMA->GetStaticMeshComponent();
			if (SMComp && PreviewMesh)
			{
				// Spawned StaticMeshActors default to Static; the preview is re-meshed,
				// scaled and turned every frame, which warned each frame.
				SMComp->SetMobility(EComponentMobility::Movable);
				SMComp->SetStaticMesh(PreviewMesh);
				// Wear what the ship wears in flight: its Blueprint slot overrides, then
				// the hull material ApplyShipHullMaterial would put on slot 0.
				for (int32 Slot = 0; Slot < ShipMesh->OverrideMaterials.Num(); ++Slot)
				{
					if (ShipMesh->OverrideMaterials[Slot])
					{
						SMComp->SetMaterial(Slot, ShipMesh->OverrideMaterials[Slot]);
					}
				}
				// (Only the native hull component gets the class material in flight.)
				UMaterialInterface* Hull = ShipMesh == Ship->ShipMeshComponent
					? ASpaceship::ResolveHullMaterial(Ship->GetClass()->GetName(), Ship->HullMaterialOverride) : nullptr;
				if (Hull)
				{
					SMComp->SetMaterial(0, Hull);
				}
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

	// Render target for the capture: the screen's shape, as the preview fills it.
	if (!ShipPreviewRT)
	{
		const FIntPoint Size = ShipPreviewTargetSize(PC);
		ShipPreviewRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, Size.X, Size.Y, RTF_RGBA8);
	}
	if (ShipPreviewRT)
	{
		ShipPreviewRT->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Scene capture: spawn a dedicated ASceneCapture2D actor at the preview area
			// (most reliable — bare HUD-attached components are fragile in PIE).
			ASceneCapture2D* CapActor = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(),
				PreviewLoc + FVector(-2400.0f, 0, 0), FRotator(0, 0, 0));
		ShipPreviewCaptureActor = CapActor;
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
				ShipPreviewCapture->FOVAngle = ShipPreviewFOV;
				// Final colour: tonemapped like the game view (HDR scene colour blew the hull out to white).
				ShipPreviewCapture->CaptureSource = SCS_FinalColorLDR;
				ShipPreviewCapture->bAlwaysPersistRenderingState = true;
				// Fixed exposure: auto exposure adapts to the black backdrop and washes the hull out.
				FPostProcessSettings& PP = ShipPreviewCapture->PostProcessSettings;
				PP.bOverride_AutoExposureMethod = true;
				PP.AutoExposureMethod = AEM_Manual;
				PP.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
				PP.AutoExposureApplyPhysicalCameraExposure = false;
				PP.bOverride_AutoExposureBias = true;
				PP.bOverride_BloomIntensity = true;
				PP.BloomIntensity = 0.2f;
				ShipPreviewCapture->bCaptureEveryFrame = true;
				ShipPreviewCapture->bUseRayTracingIfEnabled = false;
				// Keep capture seeing the WHOLE world (not ShowOnly) so a near preview
				// directional light can illuminate the mesh.
				ShipPreviewCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
				// bCaptureEveryFrame renders it; an explicit CaptureScene() only warns.
			}

			// Add a small directional light near the preview so the mesh is lit (the far
			// world location has no scene lights, which made the preview render black).
			ADirectionalLight* PreviewLight = World->SpawnActor<ADirectionalLight>(
				ADirectionalLight::StaticClass(), PreviewLoc + FVector(0, 0, 3000.0f), FRotator(-45.0f, 45.0f, 0.0f));
			ShipPreviewLight = PreviewLight;

	bShipCaptureReady = ShipPreviewCapture && ShipPreviewRT;
	UE_LOG(LogTemp, Log, TEXT("ShipSelect: preview rebuilt for roster index %d (ready=%d)"),
		ShipSelectIndex, bShipCaptureReady ? 1 : 0);
}

void AAdastreaHUD::CycleShipSelect(int32 Step)
{
	const int32 Next = FMath::Clamp(ShipSelectIndex + Step, 0, FMath::Max(0, ShipRoster.Num() - 1));
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
	// The idle turntable holds off while the player is turning the ship themselves.
	ShipPreviewLastTouched = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	ShipPreviewYaw = FMath::Fmod(ShipPreviewYaw + DeltaYaw, 360.0f);
	ShipPreviewPitch = FMath::Clamp(ShipPreviewPitch + DeltaPitch, -60.0f, 60.0f);
}

USpaceshipDataAsset* AAdastreaHUD::GetPreviewShipDataAsset() const
{
	// The data asset the Blueprint actually flies (it drives speed and acceleration),
	// so the readout can't drift from the ship.
	const ASpaceship* Ship = GetRosterShip(ShipSelectIndex);
	return Ship ? Ship->ShipDataAsset.Get() : nullptr;
}

void AAdastreaHUD::SpawnSelectedShip(APlayerController* PC)
{
	if (!PC || !PC->GetWorld())
	{
		return;
	}
	if (!ShipRoster.IsValidIndex(ShipSelectIndex) || !ShipRoster[ShipSelectIndex])
	{
		return;
	}
	const TSubclassOf<ASpaceship> ShipClass = ShipRoster[ShipSelectIndex];
	const ASpaceship* Picked = GetRosterShip(ShipSelectIndex);

	// Swapping for the ship already being flown would respawn it in place: nothing to see.
	if (PC->GetPawn() && PC->GetPawn()->GetClass() == ShipClass)
	{
		const FString Name = Picked && Picked->ShipDataAsset && !Picked->ShipDataAsset->ShipName.IsEmpty()
			? Picked->ShipDataAsset->ShipName.ToString() : ShipClass->GetName();
		ShowMessage(FString::Printf(TEXT("Already flying the %s - pick another ship"), *Name), 2.5f, true);
		UE_LOG(LogTemp, Log, TEXT("ShipSelect: %s is already the flown ship, not respawned"), *ShipClass->GetName());
		return;
	}

	UWorld* World = PC->GetWorld();
	const FVector OldLoc = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector(18000, 18000, 5000);
	const FRotator SpawnRot = PC->GetPawn() ? PC->GetPawn()->GetActorRotation() : FRotator::ZeroRotator;
	// Out beside any station the ship would overlap; the new hull may be far longer than the old.
	const FVector SpawnLoc = AAdastreaGameMode::PushSpawnClearOfStations(World, OldLoc, ShipRosterLength(Picked) * 0.5f);

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
		// Show off the new ship: one camera orbit around it before settling behind.
		if (ASpaceship* NewShip = Cast<ASpaceship>(NewPawn))
		{
			NewShip->PlaySwapShowcase();
		}
		// Its interior is a separate actor; it would outlive the ship.
		if (const ASpaceship* OldShip = Cast<ASpaceship>(Old))
		{
			if (ASpaceshipInterior* OldInterior = OldShip->GetInteriorInstance())
			{
				OldInterior->Destroy();
			}
		}
		if (Old) { Old->Destroy(); }
		UE_LOG(LogTemp, Log, TEXT("ShipSelect: spawned+possessed %llx"), (void*)NewPawn);
	}
	HideShipSelect();
}

// X4-style ship select: the ship fills the screen on a slow turntable, with translucent
// panels over it - hulls grouped by purpose on the left, ship information on the
// right (bars against the best on the roster, deltas and a tick for the ship being
// flown) and a confirm bar along the bottom. Mouse: click a row or button, drag the
// ship to orbit it, wheel to zoom (over the list, to step through it).
namespace ShipSelectUI
{
	static const FLinearColor Panel    (0.004f, 0.008f, 0.014f, 0.86f);
	static const FLinearColor Strip    (0.012f, 0.025f, 0.040f, 0.96f);
	static const FLinearColor Group    (0.010f, 0.030f, 0.050f, 0.95f);
	static const FLinearColor Row      (0.010f, 0.018f, 0.030f, 0.60f);
	static const FLinearColor Hover    (0.020f, 0.060f, 0.090f, 0.90f);
	static const FLinearColor Selected (0.020f, 0.150f, 0.200f, 0.95f);
	static const FLinearColor Text     (0.90f, 0.94f, 0.97f, 1.00f);
	static const FLinearColor Dim      (0.62f, 0.70f, 0.76f, 1.00f);
	static const FLinearColor Better   (0.40f, 0.90f, 0.50f, 1.00f);
	static const FLinearColor Worse    (0.95f, 0.45f, 0.40f, 1.00f);
	static const FLinearColor BarBack  (0.030f, 0.050f, 0.070f, 0.95f);
	static const FLinearColor BarFill  (0.22f, 0.72f, 0.82f, 1.00f);
	static const FLinearColor Confirm  (0.015f, 0.200f, 0.240f, 0.95f);
	static const FLinearColor ConfirmHi(0.030f, 0.320f, 0.380f, 1.00f);

	// One bar row of the information panel; a new Section starts a new heading.
	struct FStat
	{
		const TCHAR* Section;
		const TCHAR* Label;
		const TCHAR* Unit;
		float (*Get)(const USpaceshipDataAsset&);
	};
	static const FStat Stats[] =
	{
		{ TEXT("DEFENSE"),    TEXT("Hull"),          TEXT(""),       [](const USpaceshipDataAsset& D) { return D.HullStrength; } },
		{ TEXT("DEFENSE"),    TEXT("Armor"),         TEXT(""),       [](const USpaceshipDataAsset& D) { return D.ArmorRating; } },
		{ TEXT("DEFENSE"),    TEXT("Shield"),        TEXT(""),       [](const USpaceshipDataAsset& D) { return D.ShieldStrength; } },
		{ TEXT("DEFENSE"),    TEXT("Shield regen"),  TEXT(" /s"),    [](const USpaceshipDataAsset& D) { return D.ShieldRechargeRate; } },
		{ TEXT("PROPULSION"), TEXT("Max speed"),     TEXT(" u/s"),   [](const USpaceshipDataAsset& D) { return D.MaxSpeed; } },
		{ TEXT("PROPULSION"), TEXT("Acceleration"),  TEXT(" u/s2"),  [](const USpaceshipDataAsset& D) { return D.Acceleration; } },
		{ TEXT("PROPULSION"), TEXT("Maneuver"),      TEXT(" /10"),   [](const USpaceshipDataAsset& D) { return (float)D.Maneuverability; } },
		{ TEXT("PROPULSION"), TEXT("Jump range"),    TEXT(" ly"),    [](const USpaceshipDataAsset& D) { return D.JumpRange; } },
		{ TEXT("ARMAMENT"),   TEXT("Weapon slots"),  TEXT(""),       [](const USpaceshipDataAsset& D) { return (float)D.WeaponSlots; } },
		{ TEXT("ARMAMENT"),   TEXT("Weapon power"),  TEXT(""),       [](const USpaceshipDataAsset& D) { return D.WeaponPowerCapacity; } },
		{ TEXT("ARMAMENT"),   TEXT("Point defense"), TEXT(""),       [](const USpaceshipDataAsset& D) { return D.PointDefenseRating; } },
		{ TEXT("STORAGE"),    TEXT("Cargo"),         TEXT(" m3"),    [](const USpaceshipDataAsset& D) { return D.CargoCapacity; } },
		{ TEXT("STORAGE"),    TEXT("Fuel"),          TEXT(""),       [](const USpaceshipDataAsset& D) { return D.FuelCapacity; } },
		{ TEXT("STORAGE"),    TEXT("Drones"),        TEXT(""),       [](const USpaceshipDataAsset& D) { return (float)D.DroneCapacity; } },
		{ TEXT("STORAGE"),    TEXT("Hangar"),        TEXT(""),       [](const USpaceshipDataAsset& D) { return (float)D.HangarCapacity; } },
	};
}

void AAdastreaHUD::DrawShipSelectScreen(APlayerController* PC)
{
	namespace SS = ShipSelectUI;
	if (!PC) { return; }
	// Ensure the preview capture is built (first draw / after opening).
	if (!bShipCaptureReady)
	{
		RebuildShipPreview(PC);
	}
	int32 VX = 0, VY = 0;
	PC->GetViewportSize(VX, VY);
	const float VW = (float)VX, VH = (float)VY;
	UFont* Font = HudType::Font();
	const UWorld* World = GetWorld();
	const double Now = World ? World->GetRealTimeSeconds() : 0.0;

	// ---- Layout ----
	const float Margin = 24.0f, TopH = 56.0f, FootH = 64.0f, StripH = 34.0f, RowH = 32.0f;
	const float ListW = FMath::Clamp(VW * 0.22f, 280.0f, 400.0f);
	const float InfoW = FMath::Clamp(VW * 0.25f, 320.0f, 440.0f);
	const float PanelY = TopH + 16.0f, PanelB = VH - FootH - 16.0f;
	const FBox2D ListRect(FVector2D(Margin, PanelY), FVector2D(Margin + ListW, PanelB));
	const FBox2D InfoRect(FVector2D(VW - Margin - InfoW, PanelY), FVector2D(VW - Margin, PanelB));
	const FBox2D FootRect(FVector2D(0.0f, VH - FootH), FVector2D(VW, VH));
	const float BtnH = 40.0f, BtnY = VH - FootH + (FootH - BtnH) * 0.5f;
	const FBox2D ConfirmRect(FVector2D(VW - Margin - 230.0f, BtnY), FVector2D(VW - Margin, BtnY + BtnH));
	const FBox2D CloseRect(FVector2D(ConfirmRect.Min.X - 12.0f - 150.0f, BtnY), FVector2D(ConfirmRect.Min.X - 12.0f, BtnY + BtnH));

	const UClass* FlownClass = PC->GetPawn() ? PC->GetPawn()->GetClass() : nullptr;

	// ---- List rows: a header per purpose group, then its ships (the roster is sorted that way) ----
	struct FListRow { int32 Roster; int32 Group; }; // Roster INDEX_NONE: group header
	TArray<FListRow> Rows;
	int32 GroupCounts[UE_ARRAY_COUNT(ShipPurposeNames)] = {};
	int32 SelectedRow = 0;
	for (int32 i = 0, LastClass = INDEX_NONE; i < ShipRoster.Num(); ++i)
	{
		const int32 Group = ShipPurpose(GetRosterShip(i));
		++GroupCounts[Group];
		if (Group != LastClass)
		{
			Rows.Add({ INDEX_NONE, Group });
			LastClass = Group;
		}
		if (i == ShipSelectIndex)
		{
			SelectedRow = Rows.Num();
		}
		Rows.Add({ i, Group });
	}
	const float RowsTop = PanelY + StripH + 6.0f;
	const int32 VisibleRows = FMath::Max(3, FMath::FloorToInt((PanelB - RowsTop - 6.0f) / RowH));
	// Keep the selection in view, along with its group header when it leads the group.
	const int32 SelectedTop = (SelectedRow > 0 && Rows[SelectedRow - 1].Roster == INDEX_NONE) ? SelectedRow - 1 : SelectedRow;
	ShipSelectScroll = FMath::Clamp(ShipSelectScroll, SelectedRow - VisibleRows + 1, SelectedTop);
	ShipSelectScroll = FMath::Clamp(ShipSelectScroll, 0, FMath::Max(0, Rows.Num() - VisibleRows));
	const int32 LastRow = FMath::Min(Rows.Num(), ShipSelectScroll + VisibleRows);
	auto RowRect = [&](int32 Row)
	{
		const float Y = RowsTop + (Row - ShipSelectScroll) * RowH;
		return FBox2D(FVector2D(ListRect.Min.X + 6.0f, Y), FVector2D(ListRect.Max.X - 10.0f, Y + RowH - 4.0f));
	};

	// ---- Mouse ----
	float MX = -1.0f, MY = -1.0f;
	const bool bHasMouse = PC->GetMousePosition(MX, MY);
	const FVector2D Mouse(MX, MY);
	auto Inside = [&](const FBox2D& R) { return bHasMouse && R.IsInsideOrOn(Mouse); };
	const bool bOverUI = Inside(ListRect) || Inside(InfoRect) || Inside(FootRect) || MY < TopH;
	int32 HoverRoster = INDEX_NONE;
	for (int32 r = ShipSelectScroll; r < LastRow; ++r)
	{
		if (Rows[r].Roster != INDEX_NONE && Inside(RowRect(r)))
		{
			HoverRoster = Rows[r].Roster;
		}
	}
	const bool bHoverConfirm = Inside(ConfirmRect), bHoverClose = Inside(CloseRect);
	if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		if (bHoverConfirm)
		{
			SpawnSelectedShip(PC);
			if (!bShowShipSelect) { return; }
		}
		else if (bHoverClose)
		{
			HideShipSelect();
			return;
		}
		else if (HoverRoster != INDEX_NONE)
		{
			CycleShipSelect(HoverRoster - ShipSelectIndex);
		}
		else if (bHasMouse && !bOverUI)
		{
			bShipPreviewDragging = true;
			ShipPreviewDragLast = Mouse;
		}
	}
	if (bShipPreviewDragging)
	{
		if (bHasMouse && PC->IsInputKeyDown(EKeys::LeftMouseButton))
		{
			// The near side of the hull follows the cursor; dragging down looks from above.
			const FVector2D Delta = Mouse - ShipPreviewDragLast;
			ShipPreviewDragLast = Mouse;
			OrbitShipPreview(-Delta.X * 0.35f, Delta.Y * 0.25f);
		}
		else
		{
			bShipPreviewDragging = false;
		}
	}
	const int32 Wheel = (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp) ? 1 : 0)
		- (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown) ? 1 : 0);
	if (Wheel != 0 && bHasMouse)
	{
		if (Inside(ListRect))
		{
			CycleShipSelect(-Wheel);
		}
		else if (!bOverUI)
		{
			ShipPreviewDistance = FMath::Clamp(ShipPreviewDistance * (Wheel > 0 ? 0.88f : 1.0f / 0.88f), 3200.0f, 14000.0f);
			ShipPreviewLastTouched = Now;
		}
	}

	// ---- Preview: a turntable that idles round once the player lets go ----
	if (!bShipPreviewDragging && Now - ShipPreviewLastTouched > 3.0 && World)
	{
		ShipPreviewYaw = FMath::Fmod(ShipPreviewYaw + 9.0f * World->GetDeltaSeconds(), 360.0f);
	}
	if (ShipPreviewRT)
	{
		const FIntPoint Want = ShipPreviewTargetSize(PC);
		if (ShipPreviewRT->SizeX != Want.X || ShipPreviewRT->SizeY != Want.Y)
		{
			ShipPreviewRT->ResizeTarget(Want.X, Want.Y);
		}
	}
	if (bShipCaptureReady && ShipPreviewCapture && ShipPreviewActor)
	{
		// The ship spins on the turntable; the camera orbits up and down around it,
		// aimed at the hull's bounds centre (pivots are often at the stern or keel).
		ShipPreviewActor->SetActorRotation(FRotator(0.0f, ShipPreviewYaw, 0.0f));
		const FVector Target = ShipPreviewMeshComp ? ShipPreviewMeshComp->Bounds.Origin : ShipPreviewActor->GetActorLocation();
		const FRotator CamRot(-ShipPreviewPitch, 0.0f, 0.0f);
		ShipPreviewCapture->SetWorldLocationAndRotation(Target - CamRot.Vector() * ShipPreviewDistance, CamRot);
		ShipPreviewCapture->PostProcessSettings.AutoExposureBias = CVarShipPreviewExposure.GetValueOnGameThread();
	}

	// ---- Backdrop: the preview fills the screen ----
	DrawRect(FLinearColor(0.01f, 0.015f, 0.025f, 1.0f), 0.0f, 0.0f, VW, VH);
	if (bShipCaptureReady && ShipPreviewRT)
	{
		// Opaque: a SceneColorHDR capture stores INVERSE opacity in alpha (0 on the
		// ship), so the default translucent blend drew the ship itself invisible.
		DrawTexture(ShipPreviewRT, 0.0f, 0.0f, VW, VH, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
	}
	else
	{
		DrawCentredText(TEXT("[ preview unavailable ]"), SS::Dim, VW * 0.5f, VH * 0.5f, HudType::Label);
	}

	auto DrawBox = [this](const FBox2D& R, const FLinearColor& Fill, const FLinearColor& Edge)
	{
		DrawRect(Fill, R.Min.X, R.Min.Y, R.GetSize().X, R.GetSize().Y);
		DrawLine(R.Min.X, R.Min.Y, R.Max.X, R.Min.Y, Edge, 1.0f);
		DrawLine(R.Min.X, R.Max.Y, R.Max.X, R.Max.Y, Edge, 1.0f);
		DrawLine(R.Min.X, R.Min.Y, R.Min.X, R.Max.Y, Edge, 1.0f);
		DrawLine(R.Max.X, R.Min.Y, R.Max.X, R.Max.Y, Edge, 1.0f);
	};
	auto DrawRight = [this, Font](const FString& S, const FLinearColor& Color, float RightX, float Y, float Scale)
	{
		float W = 0.0f, H = 0.0f;
		GetTextSize(S, W, H, Font, Scale);
		DrawText(S, Color, RightX - W, Y, Font, Scale);
		return W;
	};
	auto DrawPanelStrip = [&](const FBox2D& R, const TCHAR* Title, const FString& Right)
	{
		DrawRect(SS::Panel, R.Min.X, R.Min.Y, R.GetSize().X, R.GetSize().Y);
		DrawRect(SS::Strip, R.Min.X, R.Min.Y, R.GetSize().X, StripH);
		DrawLine(R.Min.X, R.Min.Y, R.Max.X, R.Min.Y, kBorder, 2.0f);
		DrawText(Title, SS::Text, R.Min.X + 12.0f, R.Min.Y + 8.0f, Font, HudType::Body);
		DrawRight(Right, SS::Dim, R.Max.X - 12.0f, R.Min.Y + 10.0f, HudType::Label);
	};
	const FLinearColor EdgeDim(kBorder.R, kBorder.G, kBorder.B, 0.35f);

	// ---- Top bar ----
	DrawRect(FLinearColor(SS::Strip.R, SS::Strip.G, SS::Strip.B, 0.85f), 0.0f, 0.0f, VW, TopH);
	DrawLine(0.0f, TopH, VW, TopH, EdgeDim, 1.0f);
	DrawText(TEXT("SHIP SELECT"), kHeader, Margin, 16.0f, Font, HudType::Title);
	DrawRight(TEXT("Choose a hull to fly"), SS::Dim, VW - Margin, 20.0f, HudType::Label);

	// Row labels: the data asset's ship name ("Viper Interceptor"), else the Blueprint's;
	// two Blueprints sharing one data asset get their Blueprint name added.
	auto BlueprintLabel = [](const UClass* Cls)
	{
		FString Name = Cls ? Cls->GetName() : FString();
		Name.RemoveFromStart(TEXT("BP_Ship_"));
		Name.RemoveFromStart(TEXT("BP_"));
		Name.RemoveFromEnd(TEXT("_C"));
		return Name.Replace(TEXT("_"), TEXT(" "));
	};
	TArray<FString> Labels;
	for (int32 i = 0; i < ShipRoster.Num(); ++i)
	{
		const ASpaceship* Ship = GetRosterShip(i);
		Labels.Add((Ship && Ship->ShipDataAsset && !Ship->ShipDataAsset->ShipName.IsEmpty())
			? Ship->ShipDataAsset->ShipName.ToString() : BlueprintLabel(ShipRoster[i]));
	}
	TMap<FString, int32> LabelCount;
	for (const FString& L : Labels)
	{
		++LabelCount.FindOrAdd(L);
	}
	for (int32 i = 0; i < ShipRoster.Num(); ++i)
	{
		if (LabelCount[Labels[i]] > 1)
		{
			Labels[i] += FString::Printf(TEXT(" (%s)"), *BlueprintLabel(ShipRoster[i]));
		}
	}

	// ---- Left: ships by purpose ----
	DrawPanelStrip(ListRect, TEXT("AVAILABLE SHIPS"), FString::Printf(TEXT("%d"), ShipRoster.Num()));
	for (int32 r = ShipSelectScroll; r < LastRow; ++r)
	{
		const FBox2D R = RowRect(r);
		const FListRow& Entry = Rows[r];
		if (Entry.Roster == INDEX_NONE)
		{
			DrawRect(SS::Group, R.Min.X, R.Min.Y, R.GetSize().X, R.GetSize().Y);
			DrawText(ShipPurposeNames[Entry.Group], kHeader, R.Min.X + 8.0f, R.Min.Y + 7.0f, Font, HudType::Label);
			DrawRight(FString::FromInt(GroupCounts[Entry.Group]), SS::Dim, R.Max.X - 8.0f, R.Min.Y + 7.0f, HudType::Label);
			continue;
		}
		const ASpaceship* Ship = GetRosterShip(Entry.Roster);
		const bool bSelected = Entry.Roster == ShipSelectIndex;
		DrawRect(bSelected ? SS::Selected : Entry.Roster == HoverRoster ? SS::Hover : SS::Row,
			R.Min.X, R.Min.Y, R.GetSize().X, R.GetSize().Y);
		if (bSelected)
		{
			DrawLine(R.Min.X, R.Min.Y, R.Min.X, R.Max.Y, kBorder, 3.0f);
		}
		DrawText(Labels[Entry.Roster], bSelected ? FLinearColor::White : SS::Text, R.Min.X + 14.0f, R.Min.Y + 5.0f, Font, HudType::Body);
		FString RoleText = Ship && Ship->ShipDataAsset ? Ship->ShipDataAsset->ShipClass.ToString() : FString();
		RoleText += FString::Printf(TEXT("%s%.0f m"), RoleText.IsEmpty() ? TEXT("") : TEXT("   "), ShipRosterLength(Ship) / 100.0f);
		const float RoleW = DrawRight(RoleText, bSelected ? SS::Text : SS::Dim, R.Max.X - 8.0f, R.Min.Y + 7.0f, HudType::Label);
		if (ShipRoster[Entry.Roster].Get() == FlownClass)
		{
			DrawRight(TEXT("IN USE"), kCargo, R.Max.X - 8.0f - RoleW - 12.0f, R.Min.Y + 7.0f, HudType::Caption);
		}
	}
	if (Rows.Num() > VisibleRows)
	{
		// Scrollbar along the list's right edge.
		const float TrackY = RowsTop, TrackH = VisibleRows * RowH - 4.0f;
		const float ThumbH = FMath::Max(24.0f, TrackH * VisibleRows / Rows.Num());
		const float ThumbY = TrackY + (TrackH - ThumbH) * ShipSelectScroll / FMath::Max(1, Rows.Num() - VisibleRows);
		DrawRect(SS::BarBack, ListRect.Max.X - 7.0f, TrackY, 3.0f, TrackH);
		DrawRect(kBorder, ListRect.Max.X - 7.0f, ThumbY, 3.0f, ThumbH);
	}
	if (ShipRoster.Num() == 0)
	{
		DrawText(TEXT("(no ships found)"), SS::Dim, ListRect.Min.X + 14.0f, RowsTop + 6.0f, Font, HudType::Body);
	}

	// ---- Right: ship information ----
	DrawPanelStrip(InfoRect, TEXT("SHIP INFORMATION"), FString());
	const ASpaceship* Ship = GetRosterShip(ShipSelectIndex);
	const USpaceshipDataAsset* DA = Ship ? Ship->ShipDataAsset.Get() : nullptr;
	const ASpaceship* Flown = FlownClass && FlownClass->IsChildOf(ASpaceship::StaticClass())
		? GetDefault<ASpaceship>(const_cast<UClass*>(FlownClass)) : nullptr;
	const USpaceshipDataAsset* FlownDA = Flown ? Flown->ShipDataAsset.Get() : nullptr;
	const bool bCompare = DA && FlownDA && Ship && Flown->GetClass() != Ship->GetClass();
	const float IX = InfoRect.Min.X + 16.0f, IR = InfoRect.Max.X - 16.0f;
	float Y = PanelY + StripH + 12.0f;
	if (Ship && ShipRoster.IsValidIndex(ShipSelectIndex))
	{
		DrawText(Labels[ShipSelectIndex], SS::Text, IX, Y, Font, HudType::Heading);
		Y += 26.0f;
		FString Sub = DA && !DA->ShipClass.IsEmpty() ? DA->ShipClass.ToString() : FString(ShipPurposeNames[ShipPurpose(Ship)]);
		if (DA && !DA->Manufacturer.IsEmpty()) { Sub += TEXT("  |  ") + DA->Manufacturer.ToString(); }
		DrawText(Sub, kHeader, IX, Y, Font, HudType::Label);
		Y += 22.0f;
		if (DA && !DA->Description.IsEmpty())
		{
			FString Desc = DA->Description.ToString();
			if (Desc.Len() > 220) { Desc = Desc.Left(217).TrimEnd() + TEXT("..."); }
			Y = DrawWrappedText(Desc, SS::Dim, IX, Y + 2.0f, IR - IX, Font, HudType::Caption) + 6.0f;
		}

		// General facts (no bars).
		auto Fact = [&](const TCHAR* Label, const FString& Value)
		{
			DrawText(Label, kLabel, IX, Y, Font, HudType::Label);
			DrawRight(Value, SS::Text, IR, Y, HudType::Label);
			Y += 20.0f;
		};
		Y += 6.0f;
		Fact(TEXT("Length"), FString::Printf(TEXT("%.0f m"), ShipRosterLength(Ship) / 100.0f));
		if (DA) { Fact(TEXT("Crew"), FString::Printf(TEXT("%d / %d"), DA->CrewRequired, DA->MaxCrew)); }
		Fact(TEXT("Interior"), Ship->InteriorFamily != EShipInteriorFamily::None
			? StaticEnum<EShipInteriorFamily>()->GetDisplayNameTextByValue((int64)Ship->InteriorFamily).ToString()
			: FString(TEXT("-")));
	}
	if (DA)
	{
		// Bars run against the best on the roster; the white tick and the +/- are the ship being flown.
		constexpr int32 NumStats = UE_ARRAY_COUNT(SS::Stats);
		float Best[NumStats] = {};
		for (int32 i = 0; i < ShipRoster.Num(); ++i)
		{
			const ASpaceship* Other = GetRosterShip(i);
			if (const USpaceshipDataAsset* OtherDA = Other ? Other->ShipDataAsset.Get() : nullptr)
			{
				for (int32 s = 0; s < NumStats; ++s)
				{
					Best[s] = FMath::Max(Best[s], SS::Stats[s].Get(*OtherDA));
				}
			}
		}
		const float BarX = IX + 120.0f, BarW = FMath::Max(40.0f, IR - 120.0f - BarX);
		const TCHAR* Section = nullptr;
		for (int32 s = 0; s < NumStats; ++s)
		{
			const SS::FStat& Stat = SS::Stats[s];
			if (!Section || FCString::Strcmp(Section, Stat.Section) != 0)
			{
				if (Y + 46.0f > PanelB - 8.0f) { break; }
				Section = Stat.Section;
				Y += 8.0f;
				DrawText(Section, kHeader, IX, Y, Font, HudType::Caption);
				DrawLine(IX, Y + 16.0f, IR, Y + 16.0f, EdgeDim, 1.0f);
				Y += 22.0f;
			}
			if (Y + 20.0f > PanelB - 8.0f) { break; }
			const float Value = Stat.Get(*DA);
			DrawText(Stat.Label, kLabel, IX, Y, Font, HudType::Label);
			DrawRect(SS::BarBack, BarX, Y + 6.0f, BarW, 5.0f);
			if (Best[s] > 0.0f)
			{
				DrawRect(SS::BarFill, BarX, Y + 6.0f, BarW * FMath::Clamp(Value / Best[s], 0.0f, 1.0f), 5.0f);
				if (bCompare)
				{
					const float TickX = BarX + BarW * FMath::Clamp(Stat.Get(*FlownDA) / Best[s], 0.0f, 1.0f);
					DrawLine(TickX, Y + 3.0f, TickX, Y + 14.0f, FLinearColor::White, 2.0f);
				}
			}
			DrawRight(FString::Printf(TEXT("%.0f%s"), Value, Stat.Unit), SS::Text, IR - 50.0f, Y, HudType::Label);
			if (bCompare)
			{
				const float Delta = Value - Stat.Get(*FlownDA);
				if (FMath::Abs(Delta) >= 0.5f)
				{
					DrawRight(FString::Printf(TEXT("%+.0f"), Delta), Delta > 0.0f ? SS::Better : SS::Worse, IR, Y + 1.0f, HudType::Caption);
				}
			}
			Y += 20.0f;
		}
	}
	else
	{
		DrawText(TEXT("(no data asset on this ship)"), SS::Dim, IX, Y, Font, HudType::Label);
	}

	// ---- Footer: hints, close and confirm ----
	DrawRect(FLinearColor(SS::Strip.R, SS::Strip.G, SS::Strip.B, 0.90f), FootRect.Min.X, FootRect.Min.Y, VW, FootH);
	DrawLine(0.0f, FootRect.Min.Y, VW, FootRect.Min.Y, EdgeDim, 1.0f);
	DrawText(TEXT("[Drag] rotate    [Wheel] zoom    [Up/Down] ship    [Left/Right] turn"),
		SS::Dim, Margin, FootRect.Min.Y + FootH * 0.5f - 7.0f, Font, HudType::Label);
	if (bCompare)
	{
		DrawText(TEXT("+/- and white ticks compare with the ship you are flying"), kLabel,
			Margin, FootRect.Min.Y + FootH * 0.5f + 9.0f, Font, HudType::Caption);
	}
	auto Button = [&](const FBox2D& R, const FString& Label, const FLinearColor& Fill, const FLinearColor& TextColor)
	{
		DrawBox(R, Fill, kBorder);
		float W = 0.0f, H = 0.0f;
		GetTextSize(Label, W, H, Font, HudType::Body);
		DrawText(Label, TextColor, R.GetCenter().X - W * 0.5f, R.GetCenter().Y - H * 0.5f, Font, HudType::Body);
	};
	Button(CloseRect, TEXT("CLOSE  [Esc]"), bHoverClose ? SS::Hover : SS::Group, SS::Text);
	const bool bAlreadyFlown = Ship && Ship->GetClass() == FlownClass;
	Button(ConfirmRect, bAlreadyFlown ? TEXT("CURRENT SHIP") : TEXT("CONFIRM  [Space]"),
		bAlreadyFlown ? SS::Group : bHoverConfirm ? SS::ConfirmHi : SS::Confirm, bAlreadyFlown ? SS::Dim : FLinearColor::White);
}

void AAdastreaHUD::ShowMessage(const FString& InMessage, float DurationSecs, bool bIsWarning)
{
	// Secondary: when the message reports something that already made its own
	// sound (quicksave chime, trade denied, door), the toast stays quiet.
	UAudioEventLibrary::PlaySecondary2D(this, TEXT("UI.Toast"), 0.5f);

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


	int32 VW = 0;
	int32 VH = 0;
	if (PC) { PC->GetViewportSize(VW, VH); }

	const FString Message = PendingMessage;
	const FLinearColor Colour = bMessageIsWarning ? FLinearColor(1.0f, 0.4f, 0.35f, 1.0f) : FLinearColor(0.4f, 0.9f, 1.0f, 1.0f);

	// Measure the text so we can center it and size the backing bar.
	float TextW = 0.0f;
	float TextH = 0.0f;
	GetTextSize(Message, TextW, TextH, HudType::Font(), HudType::Body);

	const float X = (VW - TextW) * 0.5f;
	const float Y = 120.0f;
	const float Pad = 10.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f), X - Pad, Y - Pad, TextW + Pad * 2.0f, TextH + Pad * 2.0f);
	DrawText(Message, Colour, X, Y, HudType::Font(), HudType::Body);
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

	int32 VW = 0;
	int32 VH = 0;
	if (PC) { PC->GetViewportSize(VW, VH); }

	const FString Line = FString::Printf(TEXT("%s   [E]"), *Prompt.ToString());
	float TextW = 0.0f;
	float TextH = 0.0f;
	GetTextSize(Line, TextW, TextH, HudType::Font(), HudType::Body);

	const float X = (VW - TextW) * 0.5f;
	const float Y = VH - 140.0f;
	const float Pad = 10.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), X - Pad, Y - Pad, TextW + Pad * 2.0f, TextH + Pad * 2.0f);
		DrawText(Line, FLinearColor(0.4f, 0.95f, 1.0f, 1.0f), X, Y, HudType::Font(), HudType::Body);
	}

	// ========================================================================
	// STATION INFO SCREEN
	// ========================================================================

	void AAdastreaHUD::ShowStationInfo()
	{
		bShowStationInfo = true;
	}

	void AAdastreaHUD::HideStationInfo()
	{
		bShowStationInfo = false;
	}

	void AAdastreaHUD::DrawStationInfoScreen(APlayerController* PC, ASpaceStation* Station, const FVector& ObserverPos)
	{
		if (!PC || !Station)
		{
			return;
		}

		int32 VX = 0, VY = 0;
		PC->GetViewportSize(VX, VY);
		const float VW = (float)VX, VH = (float)VY;

		// Full-screen dim backdrop.
		DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.94f), 0.0f, 0.0f, VW, VH);


		// Title + name.
		DrawCentredText(TEXT("STATION OVERVIEW"), FLinearColor(0.15f,0.9f,0.6f,1.0f), VW * 0.5f, 18.0f, HudType::Title);
		DrawCentredText(Station->GetTargetDisplayName_Implementation().ToString(), FLinearColor(0.8f,0.9f,1.0f,1.0f), VW * 0.5f, 52.0f, HudType::Body);

		// Distance to observer.
		const float Dist = FVector::Dist(Station->GetActorLocation(), ObserverPos);
		DrawText(FString::Printf(TEXT("DISTANCE: %.0f u"), Dist), FLinearColor(0.6f,0.7f,0.8f,1.0f), 40.0f, 40.0f, HudType::Font(), HudType::Label);

		// ---- Left column: POWER + SHIELDS ----
		const float CX = 40.0f;
		const float CY = 90.0f;
		const float ColW = 380.0f;
		const float RowH = 26.0f;

		DrawText(TEXT("— POWER —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), CX, CY, HudType::Font(), HudType::Heading);
		float Y = CY + 26.0f;
		DrawText(FString::Printf(TEXT("Generation:   %.0f"), Station->GetTotalPowerGeneration()),
			kCargo, CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Consumption:  %.0f"), Station->GetTotalPowerConsumption()),
			kThrottle, CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		// Net balance; colour green on surplus, red on deficit.
		const float Balance = Station->GetPowerBalance();
		const FLinearColor BalCol = Balance >= 0.0f ? FLinearColor(0.3f,0.9f,0.4f,1.0f) : FLinearColor(1.0f,0.4f,0.35f,1.0f);
		DrawText(FString::Printf(TEXT("Net balance:  %+.0f"), Balance), BalCol, CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		// Breakdown of generation sources.
		DrawText(FString::Printf(TEXT("        Reactor: %.0f   Solar: %.0f"),
			Station->GetTotalReactorOutput(), Station->GetTotalSolarOutput()),
			kLabel, CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH + 6.0f;

		DrawText(TEXT("— SHIELDS —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), CX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		const float Cur = Station->GetTotalCurrentShieldStrength();
		const float Max = Station->GetTotalShieldStrength();
		const float ShieldFrac = Max > 0.0f ? FMath::Clamp(Cur / Max, 0.0f, 1.0f) : 0.0f;
		DrawText(FString::Printf(TEXT("Shield:  %.0f / %.0f  (%.0f%%)"), Cur, Max, ShieldFrac*100.0f),
			FLinearColor(0.45f,0.85f,0.95f,1.0f), CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Turret DPS:  %.0f"), Station->GetTotalTurretDps()),
			FLinearColor(0.9f,0.5f,0.3f,1.0f), CX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH + 6.0f;

		// ---- Middle column: POPULATION ----
		const float MX = CX + ColW + 40.0f;
		Y = CY;
		DrawText(TEXT("— POPULATION —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), MX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		DrawText(FString::Printf(TEXT("Residents:  %d"), Station->GetTotalResidents()),
			kPos, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Crew berths: %d  (barracks)"), Station->GetTotalCrewCapacity()),
			kPos, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH + 6.0f;

		DrawText(TEXT("— STORAGE —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), MX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		DrawText(FString::Printf(TEXT("Cargo: %d / %d stored"), Station->GetTotalCargoStored(), Station->GetTotalStorageCapacity()),
			kCargo, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Fuel: %.0f / %.0f L"), Station->GetTotalFuelStored(), Station->GetTotalFuelCapacity()),
			kCargo, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH + 6.0f;

		DrawText(TEXT("— COMMERCE —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), MX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		const int32 Open = Station->GetOpenMarketplaceCount();
		DrawText(FString::Printf(TEXT("Markets: %d open / %d total"), Open, Station->GetTotalMarketplaceCount()),
			kCredit, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Docking: %d points / %d capacity"), Station->GetTotalDockingPoints(), Station->GetTotalDockingCapacity()),
			kPos, MX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;

		// ---- Right column: CAPABILITIES + MODULES ----
		const float RX = MX + ColW + 40.0f;
		Y = CY;
		DrawText(TEXT("— CAPABILITIES —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), RX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		const FLinearColor CapYes(0.3f,0.85f,0.5f,1.0f);
		const FLinearColor CapNo (0.6f,0.4f,0.4f,1.0f);
		DrawText(FString::Printf(TEXT("Docking bay:   %s"), Station->HasDockingCapability() ? TEXT("YES") : TEXT("no")),
			Station->HasDockingCapability() ? CapYes : CapNo, RX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Marketplace:   %s"), Station->HasMarketplace() ? TEXT("YES") : TEXT("no")),
			Station->HasMarketplace() ? CapYes : CapNo, RX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH;
		DrawText(FString::Printf(TEXT("Cargo storage: %s"), Station->HasCargoStorage() ? TEXT("YES") : TEXT("no")),
			Station->HasCargoStorage() ? CapYes : CapNo, RX+14.0f, Y, HudType::Font(), HudType::Label); Y += RowH + 6.0f;

		DrawText(TEXT("— MODULES BY TYPE —"), FLinearColor(0.35f,0.8f,0.9f,1.0f), RX, Y, HudType::Font(), HudType::Heading); Y += 26.0f;
		constexpr EStationModuleGroup Groups[] = {
			EStationModuleGroup::Docking, EStationModuleGroup::Power, EStationModuleGroup::Storage,
			EStationModuleGroup::Processing, EStationModuleGroup::Defence, EStationModuleGroup::Habitation,
			EStationModuleGroup::Public, EStationModuleGroup::Connection,
		};
		const TCHAR* GroupNames[] = {
			TEXT("Docking"), TEXT("Power"), TEXT("Storage"), TEXT("Processing"),
			TEXT("Defence"), TEXT("Habitation"), TEXT("Public"), TEXT("Connection"),
		};
		for (int32 i = 0; i < UE_ARRAY_COUNT(Groups); ++i)
		{
			const int32 Count = Station->GetModuleCountByGroup(Groups[i]);
			FLinearColor CountCol = kLabel;
			if (Count > 0) { CountCol = FLinearColor(0.7f,0.85f,0.9f,1.0f); }
			DrawText(FString::Printf(TEXT("%-12s  %d"), GroupNames[i], Count), CountCol, RX+14.0f, Y, HudType::Font(), HudType::Label);
			Y += 22.0f;
		}

		// Footer controls.
		DrawCentredText(TEXT("[N]: close    (live readout from current station systems)"),
			FLinearColor(0.6f,0.7f,0.8f,0.9f), VW * 0.5f, VH - 40.0f, HudType::Label);
	}


void AAdastreaHUD::DrawMiningHUD(APlayerController* PC, ASpaceship* Ship, float PanelTop)
{
	UDroneBayComponent* Bay = Ship ? Ship->DroneBay : nullptr;
	if (!Bay || !Bay->bMiningEnabled || !Canvas || !PC)
	{
		return;
	}
	const FLinearColor kTrack(0.08f, 0.10f, 0.12f, 0.9f);
	const FLinearColor kGood(0.30f, 1.00f, 0.50f, 1.0f);
	const FLinearColor kWarn(1.00f, 0.55f, 0.25f, 1.0f);

	AAsteroid* Rock = Cast<AAsteroid>(Bay->GetTarget());
	if (!Rock)
	{
		// Nothing locked: no panel, just the controls hint (the cockpit HUD draws its own).
		if (bCyberpunkFlightHUD)
		{
			return;
		}
		const FString Hint = Bay->GetDronesOut() > 0
			? FString::Printf(TEXT("MINING DRONES   %d returning to the bay"), Bay->GetDronesOut())
			: FString(TEXT("MINING DRONES   [T] lock asteroid ahead    [L] launch / recall"));
		float HW = 0.0f, HH = 0.0f;
		GetTextSize(Hint, HW, HH, HudType::Font(), HudType::Label);
		DrawText(Hint, FLinearColor(0.6f, 0.7f, 0.75f, 0.85f), (Canvas->SizeX - HW) * 0.5f, Canvas->SizeY - 64.0f, HudType::Font(), HudType::Label);
		return;
	}

	if (bCyberpunkFlightHUD)
	{
		DrawCyberMiningHUD(PC, Ship, Bay, Rock, PanelTop);
		return;
	}

	UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	UTradeItemDataAsset* Ore = Bay->GetTargetOre();
	FLinearColor Tint = Type ? Type->OreTint : kCargo;
	Tint.A = 1.0f;
	const EDroneBayStatus Status = Bay->GetStatus();
	const bool bFiring = Status == EDroneBayStatus::Mining;
	const bool bInRange = Bay->IsTargetInRange();
	const float Dist = Bay->GetTargetSurfaceDistance();
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// ---- Lock brackets around the asteroid (ore tint, orange when out of range, pulse while drones work it) ----
	{
		FVector2D Centre, Edge;
		const FVector CamRight = PC->PlayerCameraManager ? FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetScaledAxis(EAxis::Y) : FVector::RightVector;
		if (PC->ProjectWorldLocationToScreen(Rock->GetActorLocation(), Centre) &&
			PC->ProjectWorldLocationToScreen(Rock->GetActorLocation() + CamRight * Rock->GetRadius(), Edge))
		{
			const float Pulse = bFiring ? 1.0f + 0.06f * FMath::Sin(Now * 12.0f) : 1.0f;
			const float R = FMath::Clamp(FVector2D::Distance(Centre, Edge) * 1.15f, 24.0f, 400.0f) * Pulse;
			const float L = FMath::Max(R * 0.35f, 10.0f);
			const float X0 = Centre.X - R, X1 = Centre.X + R, Y0 = Centre.Y - R, Y1 = Centre.Y + R;
			const FLinearColor BC = bInRange ? Tint : kWarn;
			const float T = bFiring ? 3.0f : 2.0f;
			DrawLine(X0, Y0, X0 + L, Y0, BC, T); DrawLine(X0, Y0, X0, Y0 + L, BC, T);
			DrawLine(X1, Y0, X1 - L, Y0, BC, T); DrawLine(X1, Y0, X1, Y0 + L, BC, T);
			DrawLine(X0, Y1, X0 + L, Y1, BC, T); DrawLine(X0, Y1, X0, Y1 - L, BC, T);
			DrawLine(X1, Y1, X1 - L, Y1, BC, T); DrawLine(X1, Y1, X1, Y1 - L, BC, T);
			DrawText(FString::Printf(TEXT("%.0f m"), Dist / 100.0f), BC, X1 + 6.0f, Y1 - 12.0f, HudType::Font(), HudType::Label);
		}
	}

	// ---- Mining panel (same look as the flight telemetry panel, stacked under it) ----
	const float PanelX = 20.0f, PanelW = 380.0f, RowH = 22.0f;
	const float LabelX = PanelX + 14.0f, ValueX = LabelX + 130.0f;
	const float BarW = PanelW - 28.0f;
	const float PanelH = 44.0f + 8.0f * RowH + 3.0f * 14.0f + 8.0f;
	DrawRect(kBg, PanelX, PanelTop, PanelW, PanelH);
	DrawLine(PanelX, PanelTop, PanelX, PanelTop + PanelH, Tint, 3.0f);
	DrawLine(PanelX + 2, PanelTop + PanelH - 1, PanelX + PanelW - 2, PanelTop + PanelH - 1, kBorder, 1.0f);
	DrawText(TEXT("MINING   //   TARGET"), kHeader, LabelX, PanelTop + 10.0f, HudType::Font(), HudType::Heading);
	DrawLine(PanelX + 12.0f, PanelTop + 36.0f, PanelX + PanelW - 12.0f, PanelTop + 36.0f, kBorder, 1.0f);

	float Y = PanelTop + 44.0f;
	auto Row = [&](const TCHAR* Label, const FString& Value, const FLinearColor& ValueColor)
	{
		DrawText(Label, kLabel, LabelX, Y, HudType::Font(), HudType::Body);
		DrawText(Value, ValueColor, ValueX, Y, HudType::Font(), HudType::Body);
		Y += RowH;
	};
	auto Bar = [&](float Frac, const FLinearColor& Col)
	{
		DrawRect(kTrack, LabelX, Y, BarW, 8.0f);
		DrawRect(Col, LabelX, Y, FMath::Max(BarW * FMath::Clamp(Frac, 0.0f, 1.0f), 2.0f), 8.0f);
		Y += 14.0f;
	};

	// Target identity.
	Row(TEXT("ASTEROID"), Rock->GetTargetDisplayName_Implementation().ToString(), Tint);
	Row(TEXT("ORE"), Ore ? FString::Printf(TEXT("%s   (hardness %.1f)"), *Ore->ItemName.ToString(), Type ? Type->Hardness : 1.0f)
		: FString(TEXT("none")), Ore ? Tint : kWarn);

	// Remaining yield.
	Row(TEXT("YIELD"), FString::Printf(TEXT("%.0f / %.0f u   (%.0f%%)"), Rock->GetRemainingOre(), Rock->GetTotalOre(),
		Rock->GetOreFraction() * 100.0f), kPos);
	Bar(Rock->GetOreFraction(), Tint);

	// Range.
	Row(TEXT("DISTANCE"), FString::Printf(TEXT("%.0f m   %s   (max %.0f m)"), Dist / 100.0f,
		bInRange ? TEXT("IN RANGE") : TEXT("OUT OF RANGE"), Bay->Range / 100.0f), bInRange ? kGood : kWarn);

	// Drone state.
	const FLinearColor StatusColor = bFiring ? kGood : (Status == EDroneBayStatus::Stowed ? kSpeed : kWarn);
	Row(TEXT("DRONES"), FString::Printf(TEXT("%s   -   %d/%d out, %d cutting"), *Bay->GetStatusText().ToString(),
		Bay->GetDronesOut(), Bay->DroneCount, Bay->GetDronesCutting()), StatusColor);
	if (bFiring)
	{
		// Blinking drones-working lamp at the right edge of the DRONES row.
		const float Lamp = 0.6f + 0.4f * FMath::Abs(FMath::Sin(Now * 8.0f));
		DrawRect(FLinearColor(kGood.R, kGood.G, kGood.B, Lamp), PanelX + PanelW - 26.0f, Y - RowH + 3.0f, 12.0f, 12.0f);
	}

	// Extraction: ore being cut right now, and how much is on its way home in drone hoppers.
	const float Transit = Bay->GetOreInTransit();
	const float TransitCap = FMath::Max(Bay->HopperCapacity * Bay->DroneCount, 1.0f);
	Row(TEXT("EXTRACTION"), FString::Printf(TEXT("%.1f u/s   in transit %.0f u"), Bay->GetExtractionRate(), Transit),
		bFiring ? kGood : kLabel);
	Bar(Transit / TransitCap, bFiring ? kGood : kLabel);

	// Hold fill + the ore currently being added.
	if (UCargoComponent* Cargo = Ship->CargoComponent)
	{
		const float Cap = FMath::Max(Cargo->CargoCapacity, 0.01f);
		const float Used = Cargo->CargoCapacity - Cargo->GetAvailableCargoSpace();
		const float Frac = Used / Cap;
		const FLinearColor HoldCol = Frac > 0.9f ? kWarn : kCargo;
		Row(TEXT("HOLD"), FString::Printf(TEXT("%.0f / %.0f   (%.0f%%)"), Used, Cap, Frac * 100.0f), HoldCol);
		Bar(Frac, HoldCol);
		Row(TEXT("IN HOLD"), Ore ? FString::Printf(TEXT("%s  x%d"), *Ore->ItemName.ToString(), Cargo->GetItemQuantity(Ore))
			: FString(TEXT("-")), kCargo);

		// "+N" flash beside the IN HOLD row for a moment after each delivery.
		const float Since = Bay->GetSecondsSinceLastMined();
		if (Bay->GetLastMinedOre() && Since < 1.5f)
		{
			FLinearColor FlashCol = Tint;
			FlashCol.A = FMath::Clamp(1.0f - Since / 1.5f, 0.0f, 1.0f);
			const FString Plus = FString::Printf(TEXT("+%d"), Bay->GetLastMinedAmount());
			float FW = 0.0f, FH = 0.0f;
			GetTextSize(Plus, FW, FH, HudType::Font(), HudType::Heading);
			DrawText(Plus, FlashCol, PanelX + PanelW - FW - 14.0f, Y - RowH - 4.0f - Since * 10.0f, HudType::Font(), HudType::Heading);
		}
	}
	else
	{
		Row(TEXT("HOLD"), TEXT("no cargo hold"), kWarn);
	}
}
