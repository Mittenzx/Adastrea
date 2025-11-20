#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * Main Menu widget class for the game start screen
 * 
 * This widget provides the main menu interface displayed when the game starts:
 * - Start Game button to begin gameplay
 * - Settings button to configure game options
 * - Quit button to exit the game
 * - Title display and version information
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class (e.g., WBP_MainMenu)
 * - Design the menu layout in the Blueprint designer
 * - Bind buttons to the provided event functions
 * - Override BlueprintNativeEvents to customize behavior
 * - Display on the MainMenu level
 * 
 * Integration:
 * - Set MainMenu as the Game Default Map in Project Settings
 * - Use level blueprints or GameMode to display this widget on level start
 * - Connects to game instance for save/load operations
 * - Handles level transitions to gameplay maps
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the main menu widget
	 * Called when the widget is first created and added to viewport
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Menu")
	void InitializeMenu();
	virtual void InitializeMenu_Implementation();

	/**
	 * Handle Start Game button click
	 * Starts a new game or continues existing save
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Menu")
	void OnStartGameClicked();
	virtual void OnStartGameClicked_Implementation();

	/**
	 * Handle Settings button click
	 * Opens the settings menu
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Menu")
	void OnSettingsClicked();
	virtual void OnSettingsClicked_Implementation();

	/**
	 * Handle Quit button click
	 * Exits the game application
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Menu")
	void OnQuitClicked();
	virtual void OnQuitClicked_Implementation();

	/**
	 * Update the version text display
	 * @param VersionText The version string to display
	 */
	UFUNCTION(BlueprintCallable, Category="Main Menu")
	void UpdateVersionText(const FText& VersionText);

	/**
	 * Show or hide the main menu
	 * @param bVisible Whether the menu should be visible
	 */
	UFUNCTION(BlueprintCallable, Category="Main Menu")
	void SetMenuVisibility(bool bVisible);

	/**
	 * Get the default gameplay level name
	 * @return The name of the level to load when starting the game
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Main Menu")
	FString GetGameplayLevelName() const;

protected:
	/** Native construction */
	virtual void NativeConstruct() override;

	/** Native destruction */
	virtual void NativeDestruct() override;

	/**
	 * Level name to load when starting the game
	 * Can be overridden in Blueprint or project settings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu|Configuration")
	FString GameplayLevelName;

	/**
	 * Whether to check for existing save game on start
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Menu|Configuration")
	bool bCheckForExistingSave;

	/**
	 * Version string to display on the menu
	 */
	UPROPERTY(BlueprintReadOnly, Category="Main Menu|Display")
	FText VersionText;

	/**
	 * Whether the menu is currently visible
	 */
	UPROPERTY(BlueprintReadOnly, Category="Main Menu|State")
	bool bIsMenuVisible;

	/**
	 * Transition to gameplay level
	 * @param LevelName The level to load
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Main Menu")
	void TransitionToGameplay(const FString& LevelName);
	virtual void TransitionToGameplay_Implementation(const FString& LevelName);

	/**
	 * Show a confirmation dialog before quitting
	 * Override in Blueprint to add custom quit confirmation UI
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Main Menu")
	void ShowQuitConfirmation();
	virtual void ShowQuitConfirmation_Implementation();
};
