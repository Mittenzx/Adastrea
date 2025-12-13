// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AdastreaPlayerController.generated.h"

// Forward declarations
class UUserWidget;
class UDataAsset;
class ASpaceStation;

/**
 * Parameters for SetStation function call via reflection
 * Used with ProcessEvent to safely pass station reference to Blueprint
 */
USTRUCT()
struct FSetStationParams
{
	GENERATED_BODY()
	
	UPROPERTY()
	ASpaceStation* Station = nullptr;
};

/**
 * Delegate for station editor toggle event
 * @deprecated Since December 2025 (v2.0), will be removed in v3.0 (mid-2026). 
 * Use C++ ToggleStationEditor() implementation instead. See Assets/StationEditorMigrationGuide.md
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStationEditorToggle);

/**
 * Player Controller for Adastrea
 * 
 * This class manages player input, interaction with the game world, and player-specific
 * state within the Adastrea game. It serves as the primary interface between the player
 * and their controlled entities (spaceships, characters, etc.).
 * 
 * Key Responsibilities:
 * - Processing player input for spaceship flight controls
 * - Managing camera and viewport settings
 * - Handling UI interactions and HUD updates
 * - Coordinating with AdastreaGameInstance for persistent player data
 * - Managing player reputation and faction relationships
 * - Managing station editor UI and state
 * 
 * Usage:
 * - Set as the Player Controller Class in the Game Mode
 * - Create Blueprints derived from this class for project-specific functionality
 * - Override input handling functions to customize controls
 * - Use Blueprint events to extend behavior without C++ changes
 * 
 * Integration Points:
 * - Works with ASpaceship for vehicle control
 * - Communicates with UPlayerReputationComponent for faction standing
 * - Accesses UAdastreaGameInstance for save/load operations
 * - Updates UI widgets through the HUD system
 * - Manages station editor via UStationEditorWidget
 */
UCLASS(BlueprintType)
class ADASTREA_API AAdastreaPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAdastreaPlayerController();

	// ====================
	// Configuration
	// ====================

	/**
	 * Widget class to use for the station editor UI
	 * Set this in Blueprint to specify your custom WBP_StationEditor widget
	 * Note: Should be a UStationEditorWidget class from StationEditor module
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Station Editor")
	TSubclassOf<UUserWidget> StationEditorWidgetClass;

	/**
	 * Module catalog data asset containing available modules
	 * Set this in Blueprint to specify which modules are available
	 * Note: Should be a UStationModuleCatalog from StationEditor module
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Station Editor")
	UDataAsset* ModuleCatalog;

	/**
	 * Maximum distance to search for nearby stations (in world units)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Station Editor", meta=(ClampMin=100.0f))
	float StationSearchRadius = 5000.0f;
	// HUD Configuration
	// ====================

	/**
	 * Widget class to use for the main HUD
	 * Set this in Blueprint or editor to display the HUD
	 * Should be a class derived from UAdastreaHUDWidget
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
	TSubclassOf<class UAdastreaHUDWidget> HUDWidgetClass;

	/**
	 * Current HUD widget instance
	 * Automatically created on BeginPlay if HUDWidgetClass is set
	 */
	UPROPERTY(BlueprintReadOnly, Category="HUD")
	class UAdastreaHUDWidget* HUDWidget;

	// ====================
	// Ship Status Configuration
	// ====================

	/**
	 * Widget class to use for the ship status screen
	 * Set this in Blueprint or editor to display ship information
	 * Should be a class derived from UShipStatusWidget
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Status")
	TSubclassOf<class UShipStatusWidget> ShipStatusWidgetClass;

	/**
	 * Current ship status widget instance
	 * Created on demand when ToggleShipStatus is called
	 */
	UPROPERTY(BlueprintReadOnly, Category="Ship Status")
	class UShipStatusWidget* ShipStatusWidget;

	// ====================
	// Main Menu Configuration
	// ====================

	/**
	 * Widget class to use for the main menu/pause menu
	 * Set this in Blueprint or editor to display the in-game menu
	 * Should be a class derived from UUserWidget (e.g., UMainMenuWidget)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	/**
	 * Current main menu widget instance
	 * Created on demand when ToggleMainMenu is called
	 */
	UPROPERTY(BlueprintReadOnly, Category="Main Menu")
	UUserWidget* MainMenuWidget;

	// ====================
	// Events
	// ====================

	/**
	 * Called when the station editor should be toggled (default: M key when in spaceship mode)
	 * @deprecated Since December 2025 (v2.0), will be removed in v3.0 (mid-2026). 
	 * Use C++ ToggleStationEditor() implementation instead. This event is kept for backward compatibility.
	 * See Assets/StationEditorMigrationGuide.md for migration details.
	 */
	UPROPERTY(BlueprintAssignable, Category="Player|Events")
	FOnStationEditorToggle OnStationEditorToggle;

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

public:
	/**
	 * Called when the player possesses a new spaceship
	 * Override in Blueprints to handle custom possession logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Player")
	void OnPossessSpaceship(class ASpaceship* NewSpaceship);
	virtual void OnPossessSpaceship_Implementation(class ASpaceship* NewSpaceship);

	/**
	 * Get the currently controlled spaceship, if any
	 * @return The possessed spaceship actor, or nullptr if not controlling a ship
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player")
	class ASpaceship* GetControlledSpaceship() const;

	/**
	 * Check if the player is currently controlling a spaceship
	 * @return True if the player is possessing a spaceship actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player")
	bool IsControllingSpaceship() const;

	/**
	 * Toggle the station editor UI
	 * Called when StationEditorAction input is triggered
	 * Only works when controlling a spaceship
	 * 
	 * This function now handles all logic in C++:
	 * - Finds nearest station within search radius
	 * - Creates and shows/hides the editor widget
	 * - Initializes the editor manager with the station
	 * - Manages input mode (UI vs Game)
	 */
	UFUNCTION(BlueprintCallable, Category="Player|Station Editor")
	void ToggleStationEditor();

	/**
	 * Check if the station editor is currently open
	 * @return True if the editor widget is visible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Station Editor")
	bool IsStationEditorOpen() const;

	/**
	 * Get the currently active station editor widget
	 * @return The station editor widget, or nullptr if not created
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Station Editor")
	UUserWidget* GetStationEditorWidget() const;

	/**
	 * Toggle the ship status screen UI
	 * Shows/hides a detailed view of the current spaceship and its stats
	 * Only works when controlling a spaceship
	 */
	UFUNCTION(BlueprintCallable, Category="Player|Ship Status")
	void ToggleShipStatus();

	/**
	 * Check if the ship status screen is currently open
	 * @return True if the ship status widget is visible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Ship Status")
	bool IsShipStatusOpen() const;

	/**
	 * Get the currently active ship status widget
	 * @return The ship status widget, or nullptr if not created
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Ship Status")
	class UShipStatusWidget* GetShipStatusWidget() const;

	/**
	 * Toggle the main menu (pause menu) UI
	 * Shows/hides the in-game menu for pausing, settings, and returning to main menu
	 * Works in both spaceship mode and third-person mode
	 */
	UFUNCTION(BlueprintCallable, Category="Player|Main Menu")
	void ToggleMainMenu();

	/**
	 * Check if the main menu is currently open
	 * @return True if the main menu widget is visible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Main Menu")
	bool IsMainMenuOpen() const;

	/**
	 * Get the currently active main menu widget
	 * @return The main menu widget, or nullptr if not created
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player|Main Menu")
	UUserWidget* GetMainMenuWidget() const;

protected:
	/**
	 * Find the nearest space station within the search radius
	 * @return The nearest station, or nullptr if none found
	 */
	UFUNCTION(BlueprintCallable, Category="Player|Station Editor")
	ASpaceStation* FindNearestStation();

	/**
	 * Create the station editor widget if it doesn't exist
	 * @return The created or existing widget instance
	 */
	UUserWidget* CreateStationEditorWidget();

	/**
	 * Show the station editor for a specific station
	 * @param Station The station to edit
	 */
	void ShowStationEditor(ASpaceStation* Station);

	/**
	 * Hide and cleanup the station editor
	 */
	void HideStationEditor();

	/**
	 * Create the ship status widget if it doesn't exist
	 * @return The created or existing widget instance
	 */
	class UShipStatusWidget* CreateShipStatusWidget();

	/**
	 * Show the ship status screen
	 */
	void ShowShipStatus();

	/**
	 * Hide and cleanup the ship status screen
	 */
	void HideShipStatus();

	/**
	 * Create the main menu widget if it doesn't exist
	 * @return The created or existing widget instance
	 */
	UUserWidget* CreateMainMenuWidget();

	/**
	 * Show the main menu
	 */
	void ShowMainMenu();

	/**
	 * Hide and cleanup the main menu
	 */
	void HideMainMenu();

private:
	/** The currently active station editor widget instance */
	UPROPERTY()
	UUserWidget* StationEditorWidget;

	/** Whether the station editor is currently open */
	bool bIsStationEditorOpen;

	/** Whether the ship status screen is currently open */
	bool bIsShipStatusOpen;

	/** Whether the main menu is currently open */
	bool bIsMainMenuOpen;
};
