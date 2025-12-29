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
 * 
 * ========================================
 * POST-MVP STATUS: ALL FUNCTIONS DEFERRED
 * ========================================
 * This entire customization system has been deferred to post-MVP phase.
 * Reason: Cosmetic/vanity system not required for trading MVP gameplay loop.
 * 
 * All 12 BlueprintCallable functions have been commented out to reduce
 * Blueprint API surface area. Functions remain available for C++ internal
 * use and can be easily reactivated post-MVP by uncommenting UFUNCTION macros.
 * 
 * See: PHASE2_SHIPS_SYSTEM_CATEGORIZATION.md for full analysis.
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
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	bool InstallModule(UShipModuleComponent* Module, FName SlotID);

	/**
	 * Remove a module from a slot
	 * @param SlotID The ID of the slot to remove module from
	 * @return The removed module component, or nullptr if slot was empty
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	UShipModuleComponent* RemoveModule(FName SlotID);

	/**
	 * Replace a module in a slot with a new one
	 * @param NewModule The new module to install
	 * @param SlotID The slot to replace module in
	 * @return The old module that was replaced, or nullptr if slot was empty
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	UShipModuleComponent* ReplaceModule(UShipModuleComponent* NewModule, FName SlotID);

	/**
	 * Get the module installed in a specific slot
	 * @param SlotID The slot to query
	 * @return The installed module, or nullptr if slot is empty
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	UShipModuleComponent* GetModuleInSlot(FName SlotID) const;

	/**
	 * Get all installed modules of a specific category
	 * @param Category The category to filter by
	 * @return Array of modules in that category
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	TArray<UShipModuleComponent*> GetModulesByCategory(EShipModuleCategory Category) const;

	/**
	 * Get all installed modules
	 * @return Array of all installed modules
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	TArray<UShipModuleComponent*> GetAllInstalledModules() const { return InstalledModules; }

	/**
	 * Check if a specific slot is occupied
	 * @param SlotID The slot to check
	 * @return True if slot has a module installed
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	bool IsSlotOccupied(FName SlotID) const;

	// ====================
	// SLOT QUERIES
	// ====================

	/**
	 * Find a slot by its ID
	 * @param SlotID The ID to search for
	 * @param OutSlot The found slot (only valid if function returns true)
	 * @return True if a slot with the given ID was found
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	bool FindSlotByID(FName SlotID, FShipModuleSlot& OutSlot) const;

	/**
	 * Get all slots of a specific category
	 * @param Category The category to filter by
	 * @return Array of slots that accept this category
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	TArray<FShipModuleSlot> GetSlotsByCategory(EShipModuleCategory Category) const;

	/**
	 * Get all empty slots
	 * @return Array of unoccupied slots
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	TArray<FShipModuleSlot> GetEmptySlots() const;

	/**
	 * Validate if a module can be installed (checks power, mass, slot compatibility)
	 * @param Module The module to validate
	 * @param SlotID The target slot
	 * @param OutReason Output parameter for failure reason
	 * @return True if module can be installed
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	bool CanInstallModule(UShipModuleComponent* Module, FName SlotID, FText& OutReason) const;

	// ====================
	// LOADOUT MANAGEMENT
	// ====================

	/**
	 * Clear all installed modules
	 * 
	 * @note POST-MVP: Deferred to post-MVP phase (cosmetic system not needed for trading MVP)
	 */
	// UFUNCTION(BlueprintCallable, Category="Ship Customization") // DEFERRED: Post-MVP cosmetic system
	void ClearAllModules();

private:
	// ====================
	// INTERNAL RESOURCE CALCULATIONS
	// ====================

	/**
	 * Get remaining power capacity
	 * Internal calculation - used by CanInstallModule
	 */
	float GetRemainingPowerCapacity() const;

	/**
	 * Get remaining mass capacity
	 * Internal calculation - used by CanInstallModule
	 */
	float GetRemainingMassCapacity() const;

	/**
	 * Check if there's enough power for a module
	 * Internal validation - used by CanInstallModule
	 */
	bool HasSufficientPower(UShipModuleComponent* Module) const;

	/**
	 * Check if there's enough mass capacity for a module
	 * Internal validation - used by CanInstallModule
	 */
	bool HasSufficientMassCapacity(UShipModuleComponent* Module) const;

	/**
	 * Get a string representation of the current loadout
	 * Internal serialization function
	 */
	FString GetLoadoutString() const;

private:
	/**
	 * Internal helper to update a slot's occupied status
	 * @param SlotID The slot to update
	 * @param bOccupied New occupied status
	 * @param Module The module to associate (or nullptr to clear)
	 */
	void UpdateSlotOccupancy(FName SlotID, bool bOccupied, UShipModuleComponent* Module);
};
