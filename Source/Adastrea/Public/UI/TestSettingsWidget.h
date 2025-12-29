#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestSettingsWidget.generated.h"

/**
 * Test Settings Pre-Screen widget for configuring testing parameters
 * 
 * This widget provides a configuration screen displayed before the main menu:
 * - Dropdown menus for test settings (ship type, difficulty, debug modes)
 * - Apply/Continue button to proceed to main menu
 * - Reset to defaults button
 * - Settings are stored for use during gameplay testing
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class (e.g., WBP_TestSettings)
 * - Add ComboBox widgets for each setting option
 * - Bind dropdown selections to the provided event functions
 * - Override BlueprintNativeEvents to customize behavior
 * - Display this before the main menu for testing purposes
 * 
 * Integration:
 * - Can be shown before MainMenuWidget
 * - Settings can be read by GameMode or PlayerController
 * - Useful for QA testing and development builds
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UTestSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTestSettingsWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the test settings widget
	 * Called when the widget is first created and added to viewport
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void InitializeTestSettings();
	virtual void InitializeTestSettings_Implementation();

	/**
	 * Handle Apply/Continue button click
	 * Saves settings and transitions to main menu
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void OnContinueClicked();
	virtual void OnContinueClicked_Implementation();

	/**
	 * Handle Reset button click
	 * Resets all settings to default values
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void OnResetClicked();
	virtual void OnResetClicked_Implementation();

	/**
	 * Handle Ship Type dropdown selection changed
	 * @param SelectedShip The selected ship type
	 * @param SelectionIndex The index of the selected option
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void OnShipTypeChanged(const FString& SelectedShip, int32 SelectionIndex);
	virtual void OnShipTypeChanged_Implementation(const FString& SelectedShip, int32 SelectionIndex);

	/**
	 * Handle Difficulty dropdown selection changed
	 * @param SelectedDiff The selected difficulty level
	 * @param SelectionIndex The index of the selected option
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void OnDifficultyChanged(const FString& SelectedDiff, int32 SelectionIndex);
	virtual void OnDifficultyChanged_Implementation(const FString& SelectedDiff, int32 SelectionIndex);

	/**
	 * Handle Debug Mode dropdown selection changed
	 * @param SelectedMode The selected debug mode
	 * @param SelectionIndex The index of the selected option
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Test Settings")
	void OnDebugModeChanged(const FString& SelectedMode, int32 SelectionIndex);
	virtual void OnDebugModeChanged_Implementation(const FString& SelectedMode, int32 SelectionIndex);

	/**
	 * Get the currently selected ship type
	 * @return The selected ship type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	FString GetSelectedShipType() const { return SelectedShipType; }

	/**
	 * Get the currently selected difficulty
	 * @return The selected difficulty level
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	FString GetSelectedDifficulty() const { return SelectedDifficulty; }

	/**
	 * Get the currently selected debug mode
	 * @return The selected debug mode
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	FString GetSelectedDebugMode() const { return SelectedDebugMode; }

	/**
	 * Get whether auto-spawn ship is enabled
	 * @return True if auto-spawn is enabled
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	bool GetAutoSpawnEnabled() const { return bAutoSpawnShip; }

	/**
	 * Set whether auto-spawn ship is enabled
	 * @param bEnabled Whether to enable auto-spawn
	 */
	UFUNCTION(BlueprintCallable, Category="Test Settings")
	void SetAutoSpawnEnabled(bool bEnabled);

	/**
	 * Get all available ship types for the dropdown
	 * @return Array of ship type names
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	TArray<FString> GetAvailableShipTypes() const { return AvailableShipTypes; }

	/**
	 * Get all available difficulty levels for the dropdown
	 * @return Array of difficulty level names
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	TArray<FString> GetAvailableDifficulties() const { return AvailableDifficulties; }

	/**
	 * Get all available debug modes for the dropdown
	 * @return Array of debug mode names
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Test Settings")
	TArray<FString> GetAvailableDebugModes() const { return AvailableDebugModes; }

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Native destruction */
	virtual void NativeDestruct() override;

	// Configuration Properties

	/**
	 * Available ship types for selection
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Test Settings|Configuration")
	TArray<FString> AvailableShipTypes;

	/**
	 * Available difficulty levels
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Test Settings|Configuration")
	TArray<FString> AvailableDifficulties;

	/**
	 * Available debug modes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Test Settings|Configuration")
	TArray<FString> AvailableDebugModes;

	/**
	 * Whether to show the main menu after applying settings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Test Settings|Configuration")
	bool bShowMainMenuAfter;

	// Current Selection State

	/**
	 * Currently selected ship type
	 */
	UPROPERTY(BlueprintReadOnly, Category="Test Settings|State")
	FString SelectedShipType;

	/**
	 * Currently selected difficulty
	 */
	UPROPERTY(BlueprintReadOnly, Category="Test Settings|State")
	FString SelectedDifficulty;

	/**
	 * Currently selected debug mode
	 */
	UPROPERTY(BlueprintReadOnly, Category="Test Settings|State")
	FString SelectedDebugMode;

	/**
	 * Whether to automatically spawn the selected ship
	 */
	UPROPERTY(BlueprintReadWrite, Category="Test Settings|State")
	bool bAutoSpawnShip;

	/**
	 * Whether the settings widget is currently visible
	 */
	UPROPERTY(BlueprintReadOnly, Category="Test Settings|State")
	bool bIsSettingsVisible;

	/**
	 * Transition to main menu widget
	 * Override in Blueprint to customize transition behavior
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Settings")
	void TransitionToMainMenu();
	virtual void TransitionToMainMenu_Implementation();

	/**
	 * Apply current settings to game configuration
	 * Override in Blueprint to add custom setting application logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Settings")
	void ApplySettings();
	virtual void ApplySettings_Implementation();

	/**
	 * Load default settings
	 * Override in Blueprint to customize default values
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Test Settings")
	void LoadDefaultSettings();
	virtual void LoadDefaultSettings_Implementation();
};
