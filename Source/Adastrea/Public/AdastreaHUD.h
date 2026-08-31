// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AdastreaHUD.generated.h"

class AAdastreaPlayerController;
class ASpaceship;

/**
 * Adastrea in-game HUD.
 *
 * Uses the classic engine AHUD::DrawHUD() canvas path to draw test telemetry
 * (coords/speed/throttle/credits/cargo) directly over the game viewport every
 * frame. This is independent of UMG/CommonUI and always renders during PIE,
 * making it the dependable mechanism for the MVP test HUD.
 */
UCLASS()
class ADASTREA_API AAdastreaHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** Whether the full-screen sector map is shown (toggled by M). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	bool bShowMap = false;

	// ---- X4-style 3D map camera state ----
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	float MapYaw = -45.0f;        // orbit yaw (deg)
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	float MapPitch = 55.0f;       // orbit pitch (deg)
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	float MapZoom = 150000.0f;    // distance from map center
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	FVector MapCenter = FVector::ZeroVector; // what the camera looks at
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	bool bShowShips = true;
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	bool bShowStations = true;

	/** Toggle the full-screen map on/off. */
	UFUNCTION(BlueprintCallable, Category="HUD|Map")
	void ToggleMap() { bShowMap = !bShowMap; }

	/** Set the map's visibility directly. */
	UFUNCTION(BlueprintCallable, Category="HUD|Map")
	void SetMapVisible(bool bVisible) { bShowMap = bVisible; }

	/** Draw the X4-style 3D sector map (orbitable, ships+stations as icons). */
	void DrawSectorMap(APlayerController* PC, const FVector& ShipPos);

	/** Map camera control helpers (called from controller input). */
	void MapOrbit(float DeltaYaw, float DeltaPitch) { MapYaw += DeltaYaw; MapPitch = FMath::Clamp(MapPitch + DeltaPitch, 10.0f, 85.0f); }
	void MapZoomBy(float Delta) { MapZoom = FMath::Clamp(MapZoom + Delta, 20000.0f, 800000.0f); }
	void MapPan(const FVector2D& WorldDelta) { MapCenter.X += WorldDelta.X; MapCenter.Y += WorldDelta.Y; }
	void MapRecenter(const FVector& WorldPos) { MapCenter = WorldPos; }

	// ========================
	// TRADING SCREEN (canvas-drawn, reliable in PIE)
	// ========================

	/** Whether the docked trading screen is shown. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Trading")
	bool bShowTradeScreen = false;

	/** Whether we're in buy mode (true) or sell mode (false). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Trading")
	bool bBuyMode = true;

	/** Index of the currently selected item row. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Trading")
	int32 SelectedTradeIndex = 0;

	/** Show the trading screen (set when docked at a market). */
	UFUNCTION(BlueprintCallable, Category="HUD|Trading")
	void ShowTradeScreen() { bShowTradeScreen = true; SelectedTradeIndex = 0; }

	/** Hide the trading screen (set when undocked / closing). */
	UFUNCTION(BlueprintCallable, Category="HUD|Trading")
	void HideTradeScreen() { bShowTradeScreen = false; }

	/** Draw the docked trading screen (market list, credits, cargo, buy/sell). */
	void DrawTradeScreen(APlayerController* PC, AAdastreaPlayerController* AdController, ASpaceship* Ship);

	/** Move the trade selection up/down (controller input). Step +1 or -1. */
	void MoveTradeSelection(int32 Step);

	/** Toggle buy/sell mode. */
	void ToggleBuySellMode() { bBuyMode = !bBuyMode; }
};