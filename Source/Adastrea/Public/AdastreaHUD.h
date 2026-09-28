// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AdastreaHUD.generated.h"

class AAdastreaPlayerController;
class ASpaceship;
class ASpaceStation;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USpaceshipDataAsset;
class UStaticMeshComponent;
class UMiningLaserComponent;
class AAsteroid;
class AActor;
class UGalaxySubsystem;

/** Which layer of the full-screen map (M) is shown. */
UENUM(BlueprintType)
enum class EAdastreaMapView : uint8
{
	/** The loaded level in 3D: ships, stations, the player. */
	Sector,
	/** One star system: its sectors, gates and jump-lane exits. */
	System,
	/** Every star system and the jump lanes between them. */
	Universe
};

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

	// ========================
	// PAUSE MENU (canvas-drawn; Esc / F10 when no other screen is open)
	// Resume, the Master/SFX/UI volume sliders (UAudioMixSubsystem), Quit.
	// The simulation keeps running so slider changes can be heard live.
	// ========================

	/** Whether the pause menu is shown. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|PauseMenu")
	bool bShowPauseMenu = false;

	/** Highlighted pause-menu row. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|PauseMenu")
	int32 PauseMenuIndex = 0;

	UFUNCTION(BlueprintCallable, Category="HUD|PauseMenu")
	void ShowPauseMenu() { bShowPauseMenu = true; PauseMenuIndex = 0; }

	UFUNCTION(BlueprintCallable, Category="HUD|PauseMenu")
	void HidePauseMenu() { bShowPauseMenu = false; }

	/** True while any other full-screen canvas screen (map, station menu, trade, ship select, station info) is up. */
	bool IsOtherScreenOpen() const { return bShowMap || bShowStationMenu || bShowTradeScreen || bShowShipSelect || bShowStationInfo; }

	/** Move the highlighted row up/down (wraps). */
	void MovePauseMenuSelection(int32 Step);

	/** Left/Right on a slider row: change that volume by one step (5%). */
	void AdjustPauseMenuValue(int32 Direction);

	/** Enter: Resume or Quit (slider rows ignore Enter). Returns true if the menu should close. */
	bool ConfirmPauseMenuSelection(APlayerController* PC);

	/** Draw the pause menu. */
	void DrawPauseMenu(APlayerController* PC);

	// ========================
	// FLIGHT HUD
	// ========================

	/** Draw the neon cockpit flight HUD (false = the legacy top-left telemetry panel). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Flight")
	bool bCyberpunkFlightHUD = true;

	/** Neon cockpit HUD: heading tape, flight reticle, defence (shield/hull) and propulsion panels. */
	void DrawCyberpunkFlightHUD(APlayerController* PC, ASpaceship* Ship);

	/** Neon lock brackets + mining uplink panel for a locked asteroid (cockpit HUD counterpart of DrawMiningHUD). */
	void DrawCyberMiningHUD(APlayerController* PC, ASpaceship* Ship, UMiningLaserComponent* Laser, AAsteroid* Rock, float PanelTop);

	/** Legacy telemetry panel + compass (top-left). Returns the panel's bottom edge Y. */
	float DrawTelemetryPanel(ASpaceship* Ship);

	/** Draw the X4-style 3D sector map (orbitable, ships+stations as icons). */
	void DrawSectorMap(APlayerController* PC, const FVector& ShipPos);

	/** Map camera control helpers (called from controller input). */
	void MapOrbit(float DeltaYaw, float DeltaPitch) { MapYaw += DeltaYaw; MapPitch = FMath::Clamp(MapPitch + DeltaPitch, 10.0f, 85.0f); }
	void MapZoomBy(float Delta) { MapZoom = FMath::Clamp(MapZoom + Delta, 20000.0f, 800000.0f); }
	void MapPan(const FVector2D& WorldDelta) { MapCenter.X += WorldDelta.X; MapCenter.Y += WorldDelta.Y; }
	void MapRecenter(const FVector& WorldPos) { MapCenter = WorldPos; }

	// ---- Map layers: Sector (the level) / System / Universe (UGalaxySubsystem data) ----

	/** Layer shown when the map is open. Kept between openings. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	EAdastreaMapView MapView = EAdastreaMapView::Sector;

	/** Star system shown in the System layer (None = the player's system). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	FName MapSystemId;

	/** Sector highlighted in the System layer. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	FName MapSelectedSectorId;

	/** System highlighted in the Universe layer. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|Map")
	FName MapSelectedSystemId;

	/** Switch map layer. Entering System/Universe focuses the player's system if nothing is chosen. */
	UFUNCTION(BlueprintCallable, Category="HUD|Map")
	void SetMapView(EAdastreaMapView NewView);

	/** Up one layer: Sector -> System -> Universe. */
	void MapViewUp();

	/** Down one layer into the selection: Universe (selected system) -> System -> Sector (only the loaded one). */
	void MapDrillDown();

	/** A click while the map is open. Returns true if it hit a map control (tab, system, sector, button). */
	bool HandleMapClick(const FVector2D& ScreenPos);

	/** Galaxy sector of the loaded level (None if the level isn't in Galaxy.json / has no SectorId marker). */
	FName GetMapCurrentSectorId() const;

	/** Star system the player is in (falls back to the galaxy's start system). */
	FName GetMapCurrentSystemId() const;

	/** Hide the ship's UMG widgets (HUD widget, docking prompt) while the map is open; restore them after. */
	void SyncShipWidgetsForMap(APlayerController* PC);

	/** Flight HUD: on-screen markers for jump gates, and a prompt when one is close. */
	void DrawJumpGateMarkers(APlayerController* PC, ASpaceship* Ship);

	// ========================
	// TRANSIENT MESSAGE (canvas)
	// ========================

	/** Set a transient message to draw on the HUD canvas. */
	UFUNCTION(BlueprintCallable, Category="HUD|Message")
	void ShowMessage(const FString& InMessage, float DurationSecs, bool bIsWarning);

	/** Draw the current transient message (called from DrawHUD while active). */
	void DrawTransientMessage(APlayerController* PC);

	/** Set the currently-prompted interactable actor (or null to clear the prompt). */
	UFUNCTION(BlueprintCallable, Category="HUD|Interaction")
	void SetCurrentInteractable(AActor* InActor);

	/** The actor currently being prompted for interaction (may be null). */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Interaction")
	AActor* CurrentInteractable;

	/** Draw the interactable prompt near the avatar (called from DrawHUD while on foot). */
	void DrawInteractPrompt(APlayerController* PC);

	/** The current transient message (cleared when expired). */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Message")
	FString PendingMessage;

	/** Elapsed time of the current message. */
	float MessageElapsed = 0.0f;

	/** Message display duration in seconds. */
	float MessageDuration = 0.0f;

	/** Whether the current message is a warning (different colour). */
	bool bMessageIsWarning = false;

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

	// ========================
	// STATION MENU (shown on docking; each option leads to a part of the station)
	// ========================

	/** Whether the docked station option menu is shown. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|StationMenu")
	bool bShowStationMenu = false;

	/** Index of the highlighted station menu option. */
	UPROPERTY(BlueprintReadWrite, Category="HUD|StationMenu")
	int32 StationMenuIndex = 0;

	/** Open the station menu (called on docking, and when leaving a sub-screen). */
	UFUNCTION(BlueprintCallable, Category="HUD|StationMenu")
	void ShowStationMenu() { bShowStationMenu = true; bShowTradeScreen = false; StationMenuIndex = 0; }

	UFUNCTION(BlueprintCallable, Category="HUD|StationMenu")
	void HideStationMenu() { bShowStationMenu = false; }

	/** Move the highlighted option up/down (wraps). */
	void MoveStationMenuSelection(int32 Step);

	/** Activate the highlighted option (Trading Department, Maintenance Dock, Habitation, Undock). */
	void ConfirmStationMenuSelection(APlayerController* PC);

	/** Leave the station (closes menu and undocks the ship). */
	void UndockFromStationMenu(APlayerController* PC);

	/** Draw the station option menu. */
	void DrawStationMenu(APlayerController* PC, AAdastreaPlayerController* AdController, ASpaceship* Ship);

	/** Draw the mining lock brackets and the target / laser / hold panel at PanelTop (flight HUD; hidden with no asteroid locked). */
	void DrawMiningHUD(APlayerController* PC, ASpaceship* Ship, float PanelTop);

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

	// ========================
	// STATION INFO SCREEN (canvas-drawn, reliable in PIE)
	// Overview of the nearest space station's live systems: power, shields,
	// population, capabilities, storage/fuel, defence. Toggled by N. Uses the
	// aggregate getters on ASpaceStation added in the module-behaviour pass.
	// ========================

	/** Whether the station information screen is shown (toggled by N). */
	UPROPERTY(BlueprintReadWrite, Category="HUD|StationInfo")
	bool bShowStationInfo = false;

	/** Show the station info screen (locks in the current nearest station). */
	UFUNCTION(BlueprintCallable, Category="HUD|StationInfo")
	void ShowStationInfo();

	/** Hide the station info screen. */
	UFUNCTION(BlueprintCallable, Category="HUD|StationInfo")
	void HideStationInfo();

	/** Draw the full station overview panel (power/shields/crew/capabilities). */
	void DrawStationInfoScreen(APlayerController* PC, ASpaceStation* Station, const FVector& ObserverPos);

private:
	/** Last-frame shield/hull values and hit-flash timers for the cockpit HUD damage flash. */
	float LastHudShield = -1.0f;
	float LastHudHull = -1.0f;
	float ShieldHitFlash = 0.0f;
	float HullHitFlash = 0.0f;

	/** The camera that renders the preview ship into ShipPreviewRT. */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> ShipPreviewCapture;

	/** The spawned preview ship actor (not the player pawn). */
	UPROPERTY()
	TObjectPtr<AActor> ShipPreviewActor;

	/** The preview mesh component (for sizing/camera framing). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ShipPreviewMeshComp;

	/** Actors owning the preview capture and its light (destroyed with the preview). */
	UPROPERTY()
	TObjectPtr<AActor> ShipPreviewCaptureActor;

	UPROPERTY()
	TObjectPtr<AActor> ShipPreviewLight;

	/** Destroy the preview ship, its capture and its light. */
	void DestroyShipPreview();

	/** Canvas-menu state last frame, diffed by UpdateMenuAudio(). */
	struct FMenuAudioState
	{
		bool bInitialized = false;
		bool bMap = false;
		bool bTradeScreen = false;
		bool bStationMenu = false;
		bool bShipSelect = false;
		bool bStationInfo = false;
		bool bBuyMode = true;
		int32 StationMenuIndex = 0;
		int32 TradeIndex = 0;
		int32 ShipSelectIndex = 0;
	};
	FMenuAudioState MenuAudioState;

	/**
	 * UI.Open / UI.Close when a canvas screen appears or goes away, UI.Hover when
	 * its highlighted row moves, UI.Click on the buy/sell toggle. Diffing state
	 * once a frame catches every path that opens or closes a screen (keys,
	 * docking, undocking, kiosks) without hooking each call site.
	 */
	void UpdateMenuAudio();

	// ---- Galaxy map layers ----

	enum class EMapHitKind : uint8 { Tab, System, Sector, JumpExit, OpenSystem, OpenSector };

	/** A clickable area drawn this frame (rebuilt every map draw; later entries are on top). */
	struct FMapHitRect
	{
		FVector2D Min;
		FVector2D Max;
		EMapHitKind Kind;
		FName Id;
		EAdastreaMapView View = EAdastreaMapView::Sector;
	};
	TArray<FMapHitRect> MapHitRects;

	/** Widgets hidden by SyncShipWidgetsForMap, with the ESlateVisibility to restore. */
	TMap<TWeakObjectPtr<class UUserWidget>, uint8> WidgetsHiddenForMap;

	void AddMapHit(float X, float Y, float W, float H, EMapHitKind Kind, FName Id, EAdastreaMapView View = EAdastreaMapView::Sector);

	/** Topmost hit rect under ScreenPos, or nullptr. */
	const FMapHitRect* FindMapHit(const FVector2D& ScreenPos) const;

	/** Title, Sector/System/Universe tabs and breadcrumb (shared by all layers). */
	void DrawMapHeader(APlayerController* PC, float VW, const FString& Title, const FString& Breadcrumb);

	void DrawSystemMap(APlayerController* PC);
	void DrawUniverseMap(APlayerController* PC);

	/** Right-hand info panel frame with a heading. */
	void DrawMapInfoPanel(float X, float Y, float W, float H, const FString& Heading, const FLinearColor& Accent);

	/** Draws Text horizontally centred on CentreX (Scale is an AdastreaHUDStyle role). */
	void DrawCentredText(const FString& Text, const FLinearColor& Color, float CentreX, float Y, float Scale);

	/** Draws word-wrapped text; returns the Y below it. */
	float DrawWrappedText(const FString& Text, const FLinearColor& Color, float X, float Y, float MaxW, UFont* Font, float Scale);

	void DrawCircleOutline(float X, float Y, float R, const FLinearColor& Color, float Thick, int32 Segments = 48);
	void DrawFilledDisc(float X, float Y, float R, const FLinearColor& Color);

	/** A text button (hit rect registered). Returns its width. */
	float DrawMapButton(const FString& Label, float X, float Y, bool bHover, bool bActive, EMapHitKind Kind, FName Id, EAdastreaMapView View = EAdastreaMapView::Sector);
};