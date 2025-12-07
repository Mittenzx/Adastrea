// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AdastreaPlayerController.generated.h"

// Forward declarations
class UStationEditorWidget;
class UStationModuleCatalog;
class ASpaceStation;

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
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Player|Station Editor")
	TSubclassOf<UStationEditorWidget> StationEditorWidgetClass;

	/**
	 * Module catalog data asset containing available modules
	 * Set this in Blueprint to specify which modules are available
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Station Editor")
	UStationModuleCatalog* ModuleCatalog;

	/**
	 * Maximum distance to search for nearby stations (in world units)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Station Editor", meta=(ClampMin=100.0f))
	float StationSearchRadius = 5000.0f;

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
	UStationEditorWidget* GetStationEditorWidget() const;

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
	UStationEditorWidget* CreateStationEditorWidget();

	/**
	 * Show the station editor for a specific station
	 * @param Station The station to edit
	 */
	void ShowStationEditor(ASpaceStation* Station);

	/**
	 * Hide and cleanup the station editor
	 */
	void HideStationEditor();

private:
	/** The currently active station editor widget instance */
	UPROPERTY()
	UStationEditorWidget* StationEditorWidget;

	/** Whether the station editor is currently open */
	bool bIsStationEditorOpen;
};
