// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "StationEditorWidgetCpp.generated.h"

// Forward declarations
class UStationEditorManager;
class UStationModuleCatalog;
class ASpaceStation;
class ASpaceStationModule;
class UStationEditorButton;
class UBorder;
class UWidget;
struct FStationStatistics;
struct FConstructionQueueItem;
struct FStationNotification;
enum class EModulePlacementResult : uint8;

/**
 * Station Editor Widget - C++ Implementation
 *
 * This class provides full C++ implementation of the Station Editor UI.
 * All logic is in C++, Blueprint is only used for visual layout.
 *
 * Features:
 * - Module list population from catalog
 * - Station statistics display (power, module count)
 * - Construction queue management
 * - Module placement at cursor position (snaps onto the face of the module
 *   under the cursor, or onto the station's build plane in open space)
 * - Keyboard: R / Shift+R rotate, Esc cancel placement (or close), Ctrl+Z undo,
 *   Ctrl+Y / Ctrl+Shift+Z redo, Delete removes the module under the cursor,
 *   Shift+Click keeps building the same module
 * - Event-driven UI updates
 *
 * Layout: built in C++ (BuildPlanLayout) as a light overlay on the 3D plan
 * camera - a stats readout top-left, Close top-right, the construction queue
 * bottom-right, and the module palette as a strip along the bottom. The
 * controller creates this class directly; no widget Blueprint is involved.
 */
UCLASS()
class STATIONEDITOR_API UStationEditorWidgetCpp : public UUserWidget
{
	GENERATED_BODY()

public:
	UStationEditorWidgetCpp(const FObjectInitializer& ObjectInitializer);

	// =====================
	// UI Widget References (BindWidget)
	// =====================

	/** Scroll box containing the list of available modules */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UScrollBox* ModuleListScrollBox;

	/** Text block displaying power generation/consumption */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* PowerDisplayText;

	/** Text block displaying current/max module count */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* ModuleCountDisplay;

	/** Progress bar showing power balance */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UProgressBar* PowerBalanceBar;

	/** Button to close the editor */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UButton* CloseButton;

	/** Scroll box containing construction queue items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UScrollBox* QueueScrollBox;

	/**
	 * Optional status line: why the current placement is blocked, the latest
	 * editor notification, and key hints. Add a TextBlock with this name to show it.
	 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* PlacementStatusText;

	/** Optional credits readout */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* CreditsText;

	/** Name of the station being edited */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* StationNameText;

	/** How long an editor notification stays on the status line (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=0.5f))
	float NotificationDisplayTime = 4.0f;

	// =====================
	// Configuration
	// =====================

	/** Module catalog containing available modules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor")
	UStationModuleCatalog* ModuleCatalog;

	/** The station currently being edited */
	UPROPERTY(BlueprintReadWrite, Category = "Station Editor")
	ASpaceStation* CurrentStation;

	/** Default player tech level used when initializing the editor manager (5 = mid-tier, allows testing all module types) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=1, ClampMax=10))
	int32 DefaultPlayerTechLevel = 5;

	/** Maximum distance from station for module placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=100.0f))
	float MaxPlacementDistance = 5000.0f;

	/** Maximum line trace distance for module placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=1000.0f))
	float MaxTraceDistance = 10000.0f;

	// =====================
	// 3D plan camera (X4-style): while the editor is open the view leaves the
	// ship and orbits the station. RMB drag orbits, MMB drag / WASD pans,
	// wheel zooms, Q/E turn, F reframes the station.
	// =====================

	/** Degrees of orbit per pixel of right-mouse drag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=0.01f))
	float OrbitSensitivity = 0.25f;

	/** Fraction of the camera distance zoomed per wheel notch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=0.01f, ClampMax=0.5f))
	float ZoomStep = 0.12f;

	/** WASD pan speed, in camera distances per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=0.01f))
	float KeyPanSpeed = 0.6f;

	/** Q/E turn speed (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=1.0f))
	float KeyOrbitSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=100.0f))
	float MinCameraDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=1000.0f))
	float MaxCameraDistance = 60000.0f;

	/** Blend time when switching between the ship view and the plan camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor|Camera", meta=(ClampMin=0.0f))
	float CameraBlendTime = 0.5f;

	// =====================
	// Public Functions
	// =====================

	/**
	 * Initialize the editor with a station
	 * @param Station The station to edit
	 * @param Catalog The module catalog to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Editor")
	void InitializeEditor(ASpaceStation* Station, UStationModuleCatalog* Catalog);

	/**
	 * Refresh the module list from the catalog
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Editor")
	void RefreshModuleList();

	/**
	 * Refresh station statistics display
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Editor")
	void RefreshStatistics();

	/**
	 * Update the construction queue display
	 */
	UFUNCTION(BlueprintCallable, Category = "Station Editor")
	void UpdateConstructionQueue();

protected:
	// =====================
	// Widget Lifecycle
	// =====================

	/** Called when the widget is constructed */
	virtual void NativeConstruct() override;

	/** Builds the overlay layout (BuildPlanLayout) once the widget tree exists */
	virtual bool Initialize() override;

	/** Called when the widget is destroyed */
	virtual void NativeDestruct() override;

	/** Called every frame */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Called when a mouse button is pressed */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Ends a camera drag; a right-click without a drag cancels placement */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Camera orbit / pan while a mouse button drag is active */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Camera zoom */
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Editor hotkeys (rotate, cancel, undo/redo, delete) */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Releases held camera keys */
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Held camera keys get dropped when focus leaves the widget */
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;

	// =====================
	// Event Handlers
	// =====================

	/**
	 * Handle module button clicked in the list
	 * @param ModuleClass The class of module that was selected
	 */
	UFUNCTION()
	void OnModuleButtonClicked(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Handle close button clicked
	 */
	UFUNCTION()
	void OnCloseButtonClicked();

	/**
	 * Handle module placed event from manager
	 * @param Module The module that was placed
	 */
	UFUNCTION()
	void OnManagerModulePlaced(ASpaceStationModule* Module);

	/**
	 * Handle module removed event from manager
	 * @param Module The module that was removed
	 */
	UFUNCTION()
	void OnManagerModuleRemoved(ASpaceStationModule* Module);

	/**
	 * Handle statistics updated event from manager
	 * @param Statistics The updated statistics
	 */
	UFUNCTION()
	void OnManagerStatisticsUpdated(const FStationStatistics& Statistics);

	/**
	 * Handle construction queue changed event
	 */
	UFUNCTION()
	void OnManagerQueueChanged();

	/**
	 * Handle queue item cancel button clicked
	 * @param QueueId The ID of the item to cancel
	 */
	UFUNCTION()
	void OnQueueItemCancelled(int32 QueueId);

	/** Palette tile / queue row clicks from the overlay's buttons */
	void OnPaletteTileClicked(UStationEditorButton* Tile);
	void OnQueueRowClicked(UStationEditorButton* Row);

	// =====================
	// Overlay layout
	// =====================

	/** Replace the widget tree with the plan-mode overlay (see class comment) */
	void BuildPlanLayout();

	/** Light up the palette tile of the module being placed */
	void UpdatePaletteHighlight();

	/** Refresh the queue rows' progress text (rows are rebuilt only when the queue changes) */
	void UpdateQueueProgress();

	/** Show manager notifications (blocked removals, warnings, completions) on the status line */
	UFUNCTION()
	void OnManagerNotificationAdded(const FStationNotification& Notification);

	// =====================
	// Module Placement
	// =====================

	/**
	 * Enter placement mode with a module (shows preview)
	 * @param ModuleClass The module to preview
	 */
	void EnterPlacementMode(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Exit placement mode (hides preview)
	 */
	void ExitPlacementMode();

	/**
	 * Update preview position to follow cursor in 3D space.
	 *
	 * Over a module of this station: the preview attaches to the face under the
	 * cursor (UStationEditorManager::FindAttachPosition). Over nothing: it sits
	 * where the cursor ray crosses the station's horizontal build plane. The
	 * first module of an empty station always goes on the station origin.
	 */
	void UpdatePreviewPosition();

	/** Where the cursor wants the pending module, per the rules above */
	bool GetPlacementTarget(FVector& OutPosition);

	/** The module of the edited station under the cursor, if any */
	ASpaceStationModule* GetModuleUnderCursor(FHitResult* OutHit = nullptr);

	/** Rotate the pending module by 90 degrees (yaw) */
	void RotatePlacement(bool bClockwise);

	/** Refresh the optional status line */
	void UpdateStatusText();

	/** Refresh CreditsText from the player's trader wallet (only when the value changed) */
	void UpdateCreditsText();

	/** Last value written to CreditsText, so the per-tick refresh skips unchanged values */
	int32 LastShownCredits = INDEX_NONE;

	/**
	 * Handle click in 3D viewport to confirm placement
	 */
	UFUNCTION()
	void OnViewportClicked();

	/**
	 * Handle right-click to cancel placement
	 */
	UFUNCTION()
	void OnViewportRightClicked();

	/**
	 * Get world position from screen cursor position
	 * @param OutWorldPosition The world position
	 * @param OutWorldDirection The world direction
	 * @return True if successful
	 */
	bool GetCursorWorldPosition(FVector& OutWorldPosition, FVector& OutWorldDirection);

	/** How far cursor traces reach: the camera can sit well beyond MaxTraceDistance */
	float GetTraceDistance() const;

	// =====================
	// 3D plan camera
	// =====================

	/** Spawn the plan camera, frame the station and blend the view to it */
	void BeginPlanCamera();

	/** Blend the view back to the player's pawn and drop the plan camera */
	void EndPlanCamera();

	/** Point the orbit at the station and pick a distance that shows all of it */
	void FrameStation();

	/** Push CameraFocus/Yaw/Pitch/Distance onto the camera actor */
	void ApplyPlanCamera();

	/** WASD pan and Q/E turn from the held keys */
	void TickPlanCamera(float DeltaTime);

private:
	/**
	 * Ensure EditorManager exists and is valid
	 */
	void EnsureEditorManager();

	/** Overlay pieces updated after BuildPlanLayout */
	UPROPERTY()
	TObjectPtr<UBorder> QueuePanel;

	UPROPERTY()
	TArray<TObjectPtr<UStationEditorButton>> PaletteButtons;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> QueueRowTexts;

	/** The orbit camera used while the editor is open */
	UPROPERTY()
	TObjectPtr<class ACameraActor> PlanCamera;

	/** Orbit state: the camera looks at CameraFocus from (Yaw, Pitch) at CameraDistance */
	FVector CameraFocus = FVector::ZeroVector;
	float CameraYaw = -45.0f;
	float CameraPitch = -35.0f;
	float CameraDistance = 8000.0f;

	/** Mouse drags in progress, and how far the right button moved (to tell a click from a drag) */
	bool bOrbitDragging = false;
	bool bPanDragging = false;
	float RightDragPixels = 0.0f;

	/** Camera keys (WASD / Q / E) currently held */
	TSet<FKey> HeldCameraKeys;

	/** The editor manager instance */
	UPROPERTY()
	UStationEditorManager* EditorManager;

	/** Whether we're currently in placement mode */
	UPROPERTY()
	bool bIsInPlacementMode;

	/** Module class selected for placement */
	UPROPERTY()
	TSubclassOf<ASpaceStationModule> PendingPlacementModule;

	/** Whether the preview has been positioned at least once (not at world origin) */
	UPROPERTY()
	bool bPreviewPositioned;

	/** Rotation applied to the next placement (R / Shift+R) */
	FRotator PlacementRotation = FRotator::ZeroRotator;

	/** Validation result for the preview's current spot */
	EModulePlacementResult LastPlacementResult;

	/** Latest notification text and when it arrived (world seconds) */
	FText LastNotificationText;
	float LastNotificationTime = -1000.0f;
};
