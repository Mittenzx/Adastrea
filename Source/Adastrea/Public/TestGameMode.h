// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AdastreaGameMode.h"
#include "TestGameMode.generated.h"

// Forward declarations
class USpaceshipDataAsset;
class UInputConfigDataAsset;
class UUserWidget;

/**
 * Test Game Mode for pre-game spaceship and control configuration
 * 
 * This game mode extends AAdastreaGameMode to provide a menu widget that allows
 * players to configure their spaceship selection and input controls before
 * starting gameplay.
 * 
 * Features:
 * - Pre-game menu widget display
 * - Spaceship selection from available data assets
 * - Input control configuration selection
 * - Blueprint-extensible confirmation and initialization flow
 * 
 * Usage:
 * 1. Create a Widget Blueprint for the menu (WBP_TestGameModeMenu)
 * 2. Set MenuWidgetClass to your widget Blueprint
 * 3. Populate AvailableSpaceships with SpaceshipDataAsset references
 * 4. Populate AvailableInputConfigs with InputConfigDataAsset references
 * 5. Override BlueprintNativeEvent functions for custom behavior
 * 
 * Integration:
 * - Works with existing SpaceshipDataAsset for ship configuration
 * - Works with InputConfigDataAsset for control configuration
 * - Inherits all functionality from AAdastreaGameMode
 */
UCLASS(BlueprintType)
class ATestGameMode : public AAdastreaGameMode
{
	GENERATED_BODY()

public:
	ATestGameMode();

	// ====================
	// MENU CONFIGURATION
	// ====================

	/**
	 * Widget class to display for spaceship and control selection
	 * Must be a UUserWidget subclass that handles the menu UI
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode|Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	/**
	 * Z-order for the menu widget in viewport
	 * Higher values render on top of other widgets
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode|Menu", meta=(ClampMin="0", ClampMax="1000"))
	int32 MenuWidgetZOrder;

	/**
	 * Whether to display the configuration menu on game start
	 * If false, will use default selections and proceed directly
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode|Menu")
	bool bShowMenuOnStartup;

	// ====================
	// SPACESHIP OPTIONS
	// ====================

	/**
	 * List of available spaceship data assets for selection
	 * Designers should populate this with SpaceshipDataAsset references
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode|Spaceships")
	TArray<USpaceshipDataAsset*> AvailableSpaceships;

	/**
	 * Default spaceship data asset selection index
	 * Used when no selection is made or as initial selection
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode|Spaceships", meta=(ClampMin="0"))
	int32 DefaultSpaceshipIndex;

	// ====================
	// INPUT CONFIG OPTIONS
	// ====================

	/**
	 * List of available input configuration data assets for selection
	 * Designers should populate this with InputConfigDataAsset references
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode")
	TArray<UInputConfigDataAsset*> AvailableInputConfigs;

	/**
	 * Default input configuration selection index
	 * Used when no selection is made or as initial selection
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Game Mode", meta=(ClampMin="0"))
	int32 DefaultInputConfigIndex;

	// ====================
	// CURRENT SELECTION STATE
	// ====================

	/**
	 * Currently selected spaceship data asset
	 * Updated by menu selection, used for spawning
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Test Game Mode|Selection")
	USpaceshipDataAsset* SelectedSpaceship;

	/**
	 * Currently selected input configuration data asset
	 * Updated by menu selection, applied on confirmation
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Test Game Mode|Selection")
	UInputConfigDataAsset* SelectedInputConfig;

	// ====================
	// MENU INTERACTION FUNCTIONS
	// ====================

	/**
	 * Called when the player selects a spaceship from the menu
	 * @param SpaceshipIndex Index in the AvailableSpaceships array
	 * @return True if selection was valid and applied
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Game Mode|Selection")
	bool SelectSpaceship(int32 SpaceshipIndex);
	virtual bool SelectSpaceship_Implementation(int32 SpaceshipIndex);

	/**
	 * Called when the player selects an input configuration from the menu
	 * @param InputConfigIndex Index in the AvailableInputConfigs array
	 * @return True if selection was valid and applied
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Game Mode|Selection")
	bool SelectInputConfig(int32 InputConfigIndex);
	virtual bool SelectInputConfig_Implementation(int32 InputConfigIndex);

	/**
	 * Called when the player confirms their selections and is ready to start
	 * Validates selections, spawns spaceship, and applies input configuration
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Game Mode|Menu")
	void OnMenuConfirm();
	virtual void OnMenuConfirm_Implementation();

	/**
	 * Called when the player cancels the menu (optional behavior)
	 * Can be used to exit game or return to previous menu
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Game Mode|Menu")
	void OnMenuCancel();
	virtual void OnMenuCancel_Implementation();

	// ====================
	// QUERY FUNCTIONS
	// ====================

	/**
	 * Get array of spaceship display names for UI population
	 * @return Array of display names from available spaceships
	 */
	UFUNCTION(BlueprintPure, Category="Test Game Mode|Query")
	TArray<FText> GetAvailableSpaceshipNames() const;

	/**
	 * Get array of input config display names for UI population
	 * @return Array of display names from available input configs
	 */
	UFUNCTION(BlueprintPure, Category="Test Game Mode|Query")
	TArray<FText> GetAvailableInputConfigNames() const;

	/**
	 * Get the currently selected spaceship index
	 * @return Index in AvailableSpaceships array, or -1 if none selected
	 */
	UFUNCTION(BlueprintPure, Category="Test Game Mode|Query")
	int32 GetSelectedSpaceshipIndex() const;

	/**
	 * Get the currently selected input config index
	 * @return Index in AvailableInputConfigs array, or -1 if none selected
	 */
	UFUNCTION(BlueprintPure, Category="Test Game Mode|Query")
	int32 GetSelectedInputConfigIndex() const;

	/**
	 * Check if selections are valid and ready to proceed
	 * @return True if valid spaceship and input config are selected
	 */
	UFUNCTION(BlueprintPure, Category="Test Game Mode|Query")
	bool AreSelectionsValid() const;

protected:
	virtual void BeginPlay() override;

	/**
	 * Display the configuration menu widget
	 * Creates and adds the menu widget to viewport
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Game Mode|Menu")
	void ShowConfigurationMenu();
	virtual void ShowConfigurationMenu_Implementation();

	/**
	 * Hide and cleanup the configuration menu widget
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Game Mode|Menu")
	void HideConfigurationMenu();
	virtual void HideConfigurationMenu_Implementation();

	/**
	 * Apply the selected input configuration to the player controller
	 * Adds mapping contexts and sets up input
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Game Mode|Input")
	void ApplyInputConfiguration();
	virtual void ApplyInputConfiguration_Implementation();

	/**
	 * Spawn the selected spaceship for the player
	 * Uses SelectedSpaceship data asset to configure the spawned ship
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Game Mode|Spawning")
	void SpawnSelectedSpaceship();
	virtual void SpawnSelectedSpaceship_Implementation();

	/**
	 * Called after menu is dismissed and game initialization should begin
	 * Override in Blueprint for custom post-menu logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Game Mode|Initialization")
	void OnGameInitialize();
	virtual void OnGameInitialize_Implementation();

	/**
	 * Apply defaults when no menu selection is made
	 * Sets SelectedSpaceship and SelectedInputConfig from default indices
	 * Can be called by Blueprints to reset selections to defaults
	 */
	UFUNCTION(BlueprintCallable, Category="Test Game Mode|Selection")
	void ApplyDefaults();

private:
	/**
	 * Reference to the active menu widget instance
	 */
	UPROPERTY()
	UUserWidget* MenuWidgetInstance;

	/**
	 * Flag indicating if menu is currently displayed
	 */
	bool bMenuDisplayed;
};
