// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/IShipModule.h"
#include "ModularShipCustomizationWidget.generated.h"

// Forward declarations
class UShipCustomizationComponent;
class UShipModuleComponent;
class UShipModuleDataAsset;

/**
 * Base widget class for ship customization UI
 * 
 * Provides the foundation for a ship customization interface where players
 * can view, install, and remove ship modules. Designed to be extended in
 * Blueprint for visual design and specific functionality.
 * 
 * Features:
 * - Module slot visualization
 * - Module inventory display
 * - Drag-and-drop module installation (extensible)
 * - Resource tracking (power, mass)
 * - Module information display
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Design UI layout in Blueprint editor
 * 3. Bind to OnModuleSelected, OnSlotSelected events
 * 4. Call Initialize() with target ship's customization component
 */
UCLASS(Abstract, BlueprintType)
class ADASTREA_API UModularShipCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ====================
	// CONFIGURATION
	// ====================

	/** The customization component this widget is managing */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	UShipCustomizationComponent* CustomizationComponent;

	// ====================
	// STATE
	// ====================

	/** Currently selected module slot (if any) */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	FShipModuleSlot SelectedSlot;

	/** Currently selected module from inventory (if any) */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	UShipModuleComponent* SelectedModule;

	/** Whether a slot is currently selected */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	bool bHasSlotSelected;

	/** Whether a module is currently selected */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	bool bHasModuleSelected;

public:
	// ====================
	// INITIALIZATION
	// ====================

	/**
	 * Initialize the widget with a customization component
	 * @param InCustomizationComponent The component to manage
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void Initialize(UShipCustomizationComponent* InCustomizationComponent);

	/**
	 * Refresh the UI with current data
	 * Call after any changes to modules or slots
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void RefreshUI();

	// ====================
	// SLOT OPERATIONS
	// ====================

	/**
	 * Select a slot by ID
	 * @param SlotID The ID of the slot to select
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void SelectSlot(FName SlotID);

	/**
	 * Clear slot selection
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void ClearSlotSelection();

	/**
	 * Get all available slots
	 * @return Array of all module slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipModuleSlot> GetAllSlots() const;

	/**
	 * Get slots filtered by category
	 * @param Category The category to filter by
	 * @return Array of matching slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipModuleSlot> GetSlotsByCategory(EShipModuleCategory Category) const;

	// ====================
	// MODULE OPERATIONS
	// ====================

	/**
	 * Select a module from inventory
	 * @param Module The module to select
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void SelectModule(UShipModuleComponent* Module);

	/**
	 * Clear module selection
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void ClearModuleSelection();

	/**
	 * Attempt to install the selected module into the selected slot
	 * @return True if installation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	bool InstallSelectedModule();

	/**
	 * Remove module from the selected slot
	 * @return The removed module, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	UShipModuleComponent* RemoveSelectedSlotModule();

	/**
	 * Check if selected module can be installed in selected slot
	 * @param OutReason Output parameter with failure reason
	 * @return True if installation is possible
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	bool CanInstallSelectedModule(FText& OutReason) const;

	// ====================
	// INFORMATION QUERIES
	// ====================

	/**
	 * Get current power usage statistics
	 * @param OutCurrent Current power consumption
	 * @param OutMax Maximum power capacity
	 * @param OutRemaining Remaining power capacity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	void GetPowerStats(float& OutCurrent, float& OutMax, float& OutRemaining) const;

	/**
	 * Get current mass statistics
	 * @param OutCurrent Current module mass
	 * @param OutMax Maximum mass capacity
	 * @param OutRemaining Remaining mass capacity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	void GetMassStats(float& OutCurrent, float& OutMax, float& OutRemaining) const;

	/**
	 * Get formatted power usage string for UI display
	 * @return Power usage text (e.g., "450 / 1000 (45%)")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	FText GetPowerUsageText() const;

	/**
	 * Get formatted mass usage string for UI display
	 * @return Mass usage text (e.g., "3500 / 10000 kg (35%)")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	FText GetMassUsageText() const;

	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	/**
	 * Called when UI should be refreshed
	 * Override in Blueprint to update visual elements
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnUIRefresh();

	/**
	 * Called when a slot is selected
	 * Override in Blueprint to highlight slot or show details
	 * @param ModuleSlot The selected slot
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnSlotSelected(const FShipModuleSlot& ModuleSlot);

	/**
	 * Called when a module is selected
	 * Override in Blueprint to show module details
	 * @param Module The selected module
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnModuleSelected(UShipModuleComponent* Module);

	/**
	 * Called when a module is installed
	 * Override in Blueprint for success feedback
	 * @param Module The installed module
	 * @param ModuleSlot The slot it was installed into
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnModuleInstalled(UShipModuleComponent* Module, const FShipModuleSlot& ModuleSlot);

	/**
	 * Called when a module is removed
	 * Override in Blueprint for feedback
	 * @param Module The removed module
	 * @param ModuleSlot The slot it was removed from
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnModuleRemoved(UShipModuleComponent* Module, const FShipModuleSlot& ModuleSlot);

	/**
	 * Called when installation fails
	 * Override in Blueprint to show error message
	 * @param Reason The failure reason
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnInstallationFailed(const FText& Reason);
};
