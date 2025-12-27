// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IShipModule.h"
#include "ShipCustomizationComponent.generated.h"

// Forward declarations
class UShipModuleComponent;
class UShipModuleDataAsset;

/**
 * Component for managing ship module installation and customization
 * 
 * This component handles the complete ship customization system, including:
 * - Module slot management
 * - Module installation/removal
 * - Power and mass calculations
 * - Slot compatibility checking
 * - Loadout saving/loading
 * 
 * Usage:
 * - Add to ship actor (ASpaceship)
 * - Define module slots in editor
 * - Use Blueprint or C++ functions to install/remove modules
 * - Query current configuration for gameplay effects
 */
UCLASS(ClassGroup=(Ship), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShipCustomizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipCustomizationComponent();

	// ====================
	// CONFIGURATION
	// ====================

	/** Available module slots on this ship */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	TArray<FShipModuleSlot> ModuleSlots;

	/** Maximum total power capacity of the ship */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization", meta=(ClampMin="0", ClampMax="100000"))
	float MaxPowerCapacity;

	/** Maximum total mass the ship can carry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization", meta=(ClampMin="0", ClampMax="1000000"))
	float MaxMassCapacity;

protected:
	/** Currently installed modules (tracked for cleanup and queries) */
	UPROPERTY()
	TArray<UShipModuleComponent*> InstalledModules;

	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// MODULE MANAGEMENT
	// ====================

	/**
	 * Install a module in a specific slot
	 * @param Module The module component to install
	 * @param SlotID The ID of the slot to install into
	 * @return True if installation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	bool InstallModule(UShipModuleComponent* Module, FName SlotID);

	/**
	 * Remove a module from a slot
	 * @param SlotID The ID of the slot to remove module from
	 * @return The removed module component, or nullptr if slot was empty
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	UShipModuleComponent* RemoveModule(FName SlotID);

	/**
	 * Replace a module in a slot with a new one
	 * @param NewModule The new module to install
	 * @param SlotID The slot to replace module in
	 * @return The old module that was replaced, or nullptr if slot was empty
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	UShipModuleComponent* ReplaceModule(UShipModuleComponent* NewModule, FName SlotID);

	/**
	 * Get the module installed in a specific slot
	 * @param SlotID The slot to query
	 * @return The installed module, or nullptr if slot is empty
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	UShipModuleComponent* GetModuleInSlot(FName SlotID) const;

	/**
	 * Get all installed modules of a specific category
	 * @param Category The category to filter by
	 * @return Array of modules in that category
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<UShipModuleComponent*> GetModulesByCategory(EShipModuleCategory Category) const;

	/**
	 * Get all installed modules
	 * @return Array of all installed modules
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<UShipModuleComponent*> GetAllInstalledModules() const { return InstalledModules; }

	/**
	 * Check if a specific slot is occupied
	 * @param SlotID The slot to check
	 * @return True if slot has a module installed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool IsSlotOccupied(FName SlotID) const;

	// ====================
	// SLOT QUERIES
	// ====================

	/**
	 * Find a slot by its ID
	 * @param SlotID The ID to search for
	 * @param OutSlot The found slot (only valid if function returns true)
	 * @return True if a slot with the given ID was found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool FindSlotByID(FName SlotID, FShipModuleSlot& OutSlot) const;

	/**
	 * Get all slots of a specific category
	 * @param Category The category to filter by
	 * @return Array of slots that accept this category
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipModuleSlot> GetSlotsByCategory(EShipModuleCategory Category) const;

	/**
	 * Get all empty slots
	 * @return Array of unoccupied slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipModuleSlot> GetEmptySlots() const;

	/**
	 * Get all empty slots that can accept a specific module
	 * @param Module The module to check compatibility for
	 * @return Array of compatible empty slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipModuleSlot> GetCompatibleEmptySlots(UShipModuleComponent* Module) const;

	// ====================
	// RESOURCE CALCULATIONS
	// ====================

	/**
	 * Get total power consumption of all installed modules
	 * @return Current power draw (negative for net generation)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetTotalPowerConsumption() const;

	/**
	 * Get total mass of all installed modules
	 * @return Current total mass in kilograms
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetTotalModuleMass() const;

	/**
	 * Get remaining power capacity
	 * @return Available power (MaxPowerCapacity - CurrentConsumption)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetRemainingPowerCapacity() const;

	/**
	 * Get remaining mass capacity
	 * @return Available mass capacity in kilograms
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetRemainingMassCapacity() const;

	/**
	 * Check if there's enough power for a module
	 * @param Module The module to check
	 * @return True if ship has enough power capacity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool HasSufficientPower(UShipModuleComponent* Module) const;

	/**
	 * Check if there's enough mass capacity for a module
	 * @param Module The module to check
	 * @return True if ship has enough mass capacity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool HasSufficientMassCapacity(UShipModuleComponent* Module) const;

	/**
	 * Validate if a module can be installed (checks power, mass, slot compatibility)
	 * @param Module The module to validate
	 * @param SlotID The target slot
	 * @param OutReason Output parameter for failure reason
	 * @return True if module can be installed
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	bool CanInstallModule(UShipModuleComponent* Module, FName SlotID, FText& OutReason) const;

	// ====================
	// LOADOUT MANAGEMENT
	// ====================

	/**
	 * Get a string representation of the current loadout
	 * @return JSON or similar format string of installed modules
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	FString GetLoadoutString() const;

	/**
	 * Clear all installed modules
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void ClearAllModules();

	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	/**
	 * Called when a module is installed
	 * @param Module The module that was installed
	 * @param Slot The slot it was installed into
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnModuleInstalled(UShipModuleComponent* Module, const FShipModuleSlot& Slot);

	/**
	 * Called when a module is removed
	 * @param Module The module that was removed
	 * @param Slot The slot it was removed from
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnModuleRemoved(UShipModuleComponent* Module, const FShipModuleSlot& Slot);

	/**
	 * Called when power capacity is exceeded
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnPowerCapacityExceeded();

	/**
	 * Called when mass capacity is exceeded
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void OnMassCapacityExceeded();

private:
	/**
	 * Internal helper to update a slot's occupied status
	 * @param SlotID The slot to update
	 * @param bOccupied New occupied status
	 * @param Module The module to associate (or nullptr to clear)
	 */
	void UpdateSlotOccupancy(FName SlotID, bool bOccupied, UShipModuleComponent* Module);
};
