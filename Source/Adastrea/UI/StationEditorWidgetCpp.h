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
class UModuleListItemWidget;
class UConstructionQueueItemWidget;
struct FStationStatistics;
struct FConstructionQueueItem;

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
 * - Module placement at cursor position
 * - Event-driven UI updates
 * 
 * Usage:
 * 1. Create Blueprint widget based on this class
 * 2. Layout UI with named widgets (must match BindWidget properties)
 * 3. NO Blueprint logic needed - all handled in C++
 * 4. Assign to SpaceshipPlayerController's StationEditorWidgetClass
 */
UCLASS()
class ADASTREA_API UStationEditorWidgetCpp : public UUserWidget
{
	GENERATED_BODY()

public:
	UStationEditorWidgetCpp(const FObjectInitializer& ObjectInitializer);

	// =====================
	// UI Widget References (BindWidget)
	// =====================

	/** Scroll box containing the list of available modules */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ModuleListScrollBox;

	/** Text block displaying power generation/consumption */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PowerDisplayText;

	/** Text block displaying current/max module count */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ModuleCountDisplay;

	/** Progress bar showing power balance */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PowerBalanceBar;

	/** Button to close the editor */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CloseButton;

	/** Scroll box containing construction queue items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* QueueScrollBox;

	// =====================
	// Configuration
	// =====================

	/** Module catalog containing available modules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor")
	UStationModuleCatalog* ModuleCatalog;

	/** The station currently being edited */
	UPROPERTY(BlueprintReadWrite, Category = "Station Editor")
	ASpaceStation* CurrentStation;

	/** Widget class to use for module list items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor")
	TSubclassOf<UModuleListItemWidget> ModuleListItemClass;

	/** Widget class to use for construction queue items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor")
	TSubclassOf<UConstructionQueueItemWidget> QueueItemClass;

	/** Default player tech level used when initializing the editor manager */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=1, ClampMax=10))
	int32 DefaultPlayerTechLevel = 5;

	/** Default player credits used when initializing the editor manager */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=0))
	int32 DefaultPlayerCredits = 100000;

	/** Maximum distance from station for module placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=100.0f))
	float MaxPlacementDistance = 5000.0f;

	/** Maximum line trace distance for module placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Editor", meta=(ClampMin=1000.0f))
	float MaxTraceDistance = 10000.0f;

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

	/** Called when the widget is destroyed */
	virtual void NativeDestruct() override;

	/** Called every frame */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

	// =====================
	// Module Placement
	// =====================

	/**
	 * Place a module at the current cursor position
	 * @param ModuleClass The class of module to place
	 */
	void PlaceModuleAtCursor(TSubclassOf<ASpaceStationModule> ModuleClass);

	/**
	 * Get world position from screen cursor position
	 * @param OutWorldPosition The world position
	 * @param OutWorldDirection The world direction
	 * @return True if successful
	 */
	bool GetCursorWorldPosition(FVector& OutWorldPosition, FVector& OutWorldDirection);

private:
	/**
	 * Ensure EditorManager exists and is valid
	 */
	void EnsureEditorManager();

	/** The editor manager instance */
	UPROPERTY()
	UStationEditorManager* EditorManager;

	/** Currently selected module class for placement */
	UPROPERTY()
	TSubclassOf<ASpaceStationModule> SelectedModuleClass;
};
