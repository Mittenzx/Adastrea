// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AdastreaHUD.generated.h"

class AAdastreaPlayerController;
class ASpaceship;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USpaceshipDataAsset;
class UStaticMeshComponent;
class AActor;

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

	// ========================
	// SHIP SELECT SCREEN (concept prototype — later reused at construction
	// facilities / map object-inspection; canvas-drawn + SceneCapture2D preview)
	// ========================

	/** Whether the ship-select screen is shown. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|ShipSelect")
	bool bShowShipSelect = false;

	/** Index of currently selected ship in the roster. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|ShipSelect")
	int32 ShipSelectIndex = 0;

	/** Preview orbit yaw (deg). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|ShipSelect")
	float ShipPreviewYaw = -35.0f;

	/** Preview orbit pitch (deg). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|ShipSelect")
	float ShipPreviewPitch = 8.0f;

	/** Whether the SceneCapture preview has been initialized. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|ShipSelect")
	bool bShipCaptureReady = false;

	/** The render target the preview is drawn into. */
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> ShipPreviewRT;

	/** Show the ship-select screen (creates preview capture on first show). */
	UFUNCTION(BlueprintCallable, Category="HUD|ShipSelect")
	void ShowShipSelect();

	/** Hide the ship-select screen and tear down the preview capture. */
	UFUNCTION(BlueprintCallable, Category="HUD|ShipSelect")
	void HideShipSelect();

	/** Draw the ship-select screen (stats + 3D preview + list + controls). */
	void DrawShipSelectScreen(APlayerController* PC);

	/** Select the next/prev ship in the roster (Step +1/-1). Rebuilds preview. */
	void CycleShipSelect(int32 Step);

	/** Rotate the preview model (DeltaYaw, DeltaPitch). */
	void OrbitShipPreview(float DeltaYaw, float DeltaPitch);

	/** Spawn the currently selected ship as the player's pawn. */
	void SpawnSelectedShip(APlayerController* PC);

	/** Get the preview pawn's data asset (for stats readout). */
	USpaceshipDataAsset* GetPreviewShipDataAsset() const;

	/** Build the 3D preview capture + render target for the current roster index. */
	void RebuildShipPreview(APlayerController* PC);

private:
	/** The camera that renders the preview ship into ShipPreviewRT. */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> ShipPreviewCapture;

	/** The spawned preview ship actor (not the player pawn). */
	UPROPERTY()
	TObjectPtr<AActor> ShipPreviewActor;

	/** The preview mesh component (for sizing/camera framing). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ShipPreviewMeshComp;
};