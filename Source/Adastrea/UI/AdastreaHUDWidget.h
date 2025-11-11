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
};
