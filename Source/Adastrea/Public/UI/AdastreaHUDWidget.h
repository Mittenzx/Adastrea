#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AdastreaHUDWidget.generated.h"

/**
 * Base HUD widget class for the main game HUD
 * 
 * This widget provides the main heads-up display for players during gameplay, showing:
 * - Ship health and shield status
 * - Speed and navigation information
 * - Target information and combat data
 * - Alert messages and notifications
 * - Minimap or sector information
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class
 * - Design the HUD layout in the Blueprint designer
 * - Override BlueprintNativeEvents to customize behavior
 * - Use Blueprint functions to update display elements
 * 
 * Integration:
 * - Automatically managed by AAdastreaPlayerController
 * - Updates based on possessed spaceship state
 * - Connects to combat and navigation systems
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UAdastreaHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAdastreaHUDWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the HUD widget
	 * Called when the widget is first created and added to viewport
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD")
	void InitializeHUD();
	virtual void InitializeHUD_Implementation();

	/**
	 * Update ship health display
	 * @param CurrentHealth The current health value
	 * @param MaxHealth The maximum health value
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Status")
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	virtual void UpdateHealth_Implementation(float CurrentHealth, float MaxHealth);

	/**
	 * Update ship shield display
	 * @param CurrentShield The current shield value
	 * @param MaxShield The maximum shield value
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Status")
	void UpdateShield(float CurrentShield, float MaxShield);
	virtual void UpdateShield_Implementation(float CurrentShield, float MaxShield);

	/**
	 * Update speed and velocity display
	 * @param CurrentSpeed The current speed in units per second
	 * @param MaxSpeed The maximum speed capability
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Navigation")
	void UpdateSpeed(float CurrentSpeed, float MaxSpeed);
	virtual void UpdateSpeed_Implementation(float CurrentSpeed, float MaxSpeed);

	/**
	 * Update target information display
	 * @param TargetName The name of the targeted object
	 * @param TargetDistance Distance to target in units
	 * @param bIsHostile Whether the target is hostile
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Targeting")
	void UpdateTargetInfo(const FText& TargetName, float TargetDistance, bool bIsHostile);
	virtual void UpdateTargetInfo_Implementation(const FText& TargetName, float TargetDistance, bool bIsHostile);

	/**
	 * Clear target information display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Targeting")
	void ClearTargetInfo();
	virtual void ClearTargetInfo_Implementation();

	/**
	 * Show an alert message on the HUD
	 * @param Message The message to display
	 * @param Duration How long to show the message in seconds (0 = indefinite)
	 * @param bIsWarning Whether this is a warning message (affects styling)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Messages")
	void ShowAlert(const FText& Message, float Duration, bool bIsWarning);
	virtual void ShowAlert_Implementation(const FText& Message, float Duration, bool bIsWarning);

	/**
	 * Update weapon energy/ammo display
	 * @param WeaponIndex Which weapon slot (0-based)
	 * @param CurrentAmmo Current ammunition or energy
	 * @param MaxAmmo Maximum ammunition or energy
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Combat")
	void UpdateWeaponStatus(int32 WeaponIndex, float CurrentAmmo, float MaxAmmo);
	virtual void UpdateWeaponStatus_Implementation(int32 WeaponIndex, float CurrentAmmo, float MaxAmmo);

	/**
	 * Toggle the visibility of the HUD
	 * @param bVisible Whether the HUD should be visible
	 */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleHUDVisibility(bool bVisible);

protected:
	/** Native tick for automatic updates */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Current health percentage for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	float HealthPercent;

	/** Current shield percentage for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	float ShieldPercent;

	/** Current speed value for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	float CurrentSpeedValue;

	/** Whether a target is currently selected */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	bool bHasTarget;

	/** Cached reference to the player's controlled spaceship */
	UPROPERTY(BlueprintReadOnly, Category="HUD|References")
	class ASpaceship* ControlledSpaceship;

	/** Cached reference to the player controller for crosshair tracking */
	UPROPERTY()
	class APlayerController* CachedPlayerController;

	/**
	 * Called each frame to update HUD based on current game state
	 * Override in Blueprint to implement custom auto-update logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="HUD")
	void UpdateHUDFromGameState(float DeltaTime);
	virtual void UpdateHUDFromGameState_Implementation(float DeltaTime);

	/**
	 * Get the currently controlled spaceship for automatic updates
	 * @return The player's spaceship, or nullptr if not in a ship
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="HUD")
	class ASpaceship* GetControlledSpaceship() const;

	// ====================
	// PLAYER INFO DISPLAY
	// ====================

	/**
	 * Update player name display
	 * @param PlayerName The player's name
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Player Info")
	void UpdatePlayerName(const FText& PlayerName);
	virtual void UpdatePlayerName_Implementation(const FText& PlayerName);

	/**
	 * Update player level display
	 * @param Level The player's current level
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Player Info")
	void UpdatePlayerLevel(int32 Level);
	virtual void UpdatePlayerLevel_Implementation(int32 Level);

	/**
	 * Update player credits display
	 * @param Credits The player's current credits
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Player Info")
	void UpdatePlayerCredits(int32 Credits);
	virtual void UpdatePlayerCredits_Implementation(int32 Credits);

	/**
	 * Update all player information at once
	 * @param PlayerName The player's name
	 * @param Level The player's level
	 * @param Credits The player's credits
	 */
	UFUNCTION(BlueprintCallable, Category="HUD|Player Info")
	void UpdatePlayerInfo(const FText& PlayerName, int32 Level, int32 Credits);

	// ====================
	// SHIP INFO DISPLAY
	// ====================

	/**
	 * Update ship name display
	 * @param ShipName The name of the ship
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Info")
	void UpdateShipName(const FText& ShipName);
	virtual void UpdateShipName_Implementation(const FText& ShipName);

	/**
	 * Update ship class display
	 * @param ShipClass The class/type of the ship
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Info")
	void UpdateShipClass(const FText& ShipClass);
	virtual void UpdateShipClass_Implementation(const FText& ShipClass);

	/**
	 * Update ship integrity (hull strength) display
	 * @param CurrentIntegrity Current hull integrity
	 * @param MaxIntegrity Maximum hull integrity
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Ship Info")
	void UpdateShipIntegrity(float CurrentIntegrity, float MaxIntegrity);
	virtual void UpdateShipIntegrity_Implementation(float CurrentIntegrity, float MaxIntegrity);

	/**
	 * Update all ship information at once
	 * @param ShipName The ship's name
	 * @param ShipClass The ship's class
	 * @param CurrentIntegrity Current hull integrity
	 * @param MaxIntegrity Maximum hull integrity
	 */
	UFUNCTION(BlueprintCallable, Category="HUD|Ship Info")
	void UpdateShipInfo(const FText& ShipName, const FText& ShipClass, float CurrentIntegrity, float MaxIntegrity);

	/**
	 * Get player information from game instance
	 * Automatically updates player info displays
	 */
	UFUNCTION(BlueprintCallable, Category="HUD|Player Info")
	void RefreshPlayerInfo();

	/**
	 * Get ship information from controlled spaceship
	 * Automatically updates ship info displays
	 */
	UFUNCTION(BlueprintCallable, Category="HUD|Ship Info")
	void RefreshShipInfo();

protected:
	/** Current player name for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	FText CurrentPlayerName;

	/** Current player level for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	int32 CurrentPlayerLevel;

	/** Current player credits for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	int32 CurrentPlayerCredits;

	/** Current ship name for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	FText CurrentShipName;

	/** Current ship class for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	FText CurrentShipClass;

	/** Current ship integrity percentage for display */
	UPROPERTY(BlueprintReadOnly, Category="HUD|State")
	float ShipIntegrityPercent;

	// ====================
	// WEAPON AIM CROSSHAIR
	// ====================

	/**
	 * Update weapon aim crosshair position based on mouse location
	 * This provides visual feedback for where weapons will aim
	 * @param ScreenPosition The screen position (0-1 range for X and Y)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Crosshair")
	void UpdateAimCrosshair(FVector2D ScreenPosition);
	virtual void UpdateAimCrosshair_Implementation(FVector2D ScreenPosition);

	/**
	 * Show or hide the weapon aim crosshair
	 * @param bVisible Whether the aim crosshair should be visible
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Crosshair")
	void SetAimCrosshairVisible(bool bVisible);
	virtual void SetAimCrosshairVisible_Implementation(bool bVisible);

	/** Current weapon aim position in screen space (0-1 range) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Crosshair")
	FVector2D WeaponAimPosition;

	/** Whether the weapon aim crosshair is currently visible */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Crosshair")
	bool bAimCrosshairVisible;

	// ====================
	// X4-STYLE FLIGHT CROSSHAIR SYSTEM
	// ====================

	/**
	 * Update X4-style flight crosshair showing ship rotation direction
	 * 
	 * X4 Flight System Visualization:
	 * - Mouse cursor position = where player wants to turn
	 * - Ship direction indicator = where ship is actually pointing/turning
	 * - Visual connection shows the rotation intent
	 * - Deadzone circle shows no-rotation area
	 * 
	 * @param MouseScreenPosition Mouse position in screen space (0-1 range)
	 * @param ShipRotationDirection Ship's current rotation direction vector in screen space (0-1 range)
	 * @param RotationSpeed Current rotation speed (0-1, where 1 = maximum rotation speed)
	 * @param bInDeadzone Whether mouse is in the deadzone (no rotation)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Flight Crosshair")
	void UpdateFlightCrosshair(FVector2D MouseScreenPosition, FVector2D ShipRotationDirection, float RotationSpeed, bool bInDeadzone);
	virtual void UpdateFlightCrosshair_Implementation(FVector2D MouseScreenPosition, FVector2D ShipRotationDirection, float RotationSpeed, bool bInDeadzone);

	/**
	 * Show or hide the X4-style flight crosshair system
	 * @param bVisible Whether the flight crosshair should be visible
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Flight Crosshair")
	void SetFlightCrosshairVisible(bool bVisible);
	virtual void SetFlightCrosshairVisible_Implementation(bool bVisible);

	/**
	 * Update deadzone visualization on HUD
	 * @param DeadzoneRadiusPercent Deadzone radius as percentage of screen (0-1)
	 * @param MaxRadiusPercent Maximum effective radius as percentage of screen (0-1)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="HUD|Flight Crosshair")
	void UpdateDeadzoneVisualization(float DeadzoneRadiusPercent, float MaxRadiusPercent);
	virtual void UpdateDeadzoneVisualization_Implementation(float DeadzoneRadiusPercent, float MaxRadiusPercent);

	/**
	 * Enable or disable deadzone visualization
	 * @param bShow Whether to show the deadzone circle
	 */
	UFUNCTION(BlueprintCallable, Category="HUD|Flight Crosshair")
	void ShowDeadzoneVisualization(bool bShow);

	/** Mouse cursor position in screen space (0-1 range) for flight control */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	FVector2D FlightMousePosition;

	/** Ship rotation direction indicator position in screen space (0-1 range) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	FVector2D ShipRotationIndicator;

	/** Current rotation speed (0-1, normalized) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	float CurrentRotationSpeed;

	/** Whether mouse is in the deadzone (no rotation area) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	bool bMouseInDeadzone;

	/** Whether the flight crosshair system is visible */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	bool bFlightCrosshairVisible;

	/** Deadzone radius as percentage of screen (0-1) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	float DeadzoneRadiusPercent;

	/** Maximum effective radius as percentage of screen (0-1) */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	float MaxRadiusPercent;

	/** Whether to show the deadzone circle visualization */
	UPROPERTY(BlueprintReadOnly, Category="HUD|Flight Crosshair")
	bool bShowDeadzoneCircle;
};
