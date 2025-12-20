// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Ships/ShipModuleComponent.h"
#include "EngineModuleComponent.generated.h"

// Forward declarations
class UEngineModuleDataAsset;
class UNiagaraComponent;
class UAudioComponent;

/**
 * Component for engine modules on spaceships
 * 
 * Handles engine functionality including thrust, heat management, fuel consumption,
 * and visual/audio effects. Multiple engines can be installed on a single ship,
 * with their outputs combined for total ship performance.
 * 
 * Features:
 * - Thrust and speed contribution
 * - Heat and fuel management
 * - Boost and travel mode support
 * - Visual and audio effects
 * - Integration with ship movement
 * 
 * Usage:
 * 1. Create instance and add to ship
 * 2. Assign UEngineModuleDataAsset
 * 3. Install via UShipCustomizationComponent
 * 4. Engine automatically contributes to ship performance
 */
UCLASS(ClassGroup=(Ship), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UEngineModuleComponent : public UShipModuleComponent
{
	GENERATED_BODY()

public:
	UEngineModuleComponent();

	// ====================
	// CONFIGURATION
	// ====================

	/** Engine data asset (typed for convenience) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	UEngineModuleDataAsset* EngineData;

	// ====================
	// STATE
	// ====================

	/** Current throttle percentage (0-100) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine State")
	float CurrentThrottle;

	/** Current heat level (0-MaxHeat) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine State")
	float CurrentHeat;

	/** Whether engine is currently overheated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine State")
	bool bIsOverheated;

	/** Whether boost is active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine State")
	bool bBoostActive;

	/** Whether travel mode is active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine State")
	bool bTravelModeActive;

protected:
	/** Niagara component for engine trail effect */
	UPROPERTY()
	UNiagaraComponent* EngineTrailComponent;

	/** Niagara component for boost effect */
	UPROPERTY()
	UNiagaraComponent* BoostEffectComponent;

	/** Audio component for engine sound */
	UPROPERTY()
	UAudioComponent* EngineSoundComponent;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// IShipModule Interface Overrides
	// ====================

	virtual void OnModuleInstalled_Implementation(const FShipModuleSlot& Slot) override;
	virtual void OnModuleRemoved_Implementation(const FShipModuleSlot& Slot) override;

	// ====================
	// ENGINE OPERATIONS
	// ====================

	/**
	 * Set engine throttle
	 * @param ThrottlePercent Target throttle (0-100)
	 */
	UFUNCTION(BlueprintCallable, Category="Engine")
	void SetThrottle(float ThrottlePercent);

	/**
	 * Activate boost mode
	 * @return True if boost activated successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Engine")
	bool ActivateBoost();

	/**
	 * Deactivate boost mode
	 */
	UFUNCTION(BlueprintCallable, Category="Engine")
	void DeactivateBoost();

	/**
	 * Activate travel mode
	 * @return True if travel mode activated successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Engine")
	bool ActivateTravelMode();

	/**
	 * Deactivate travel mode
	 */
	UFUNCTION(BlueprintCallable, Category="Engine")
	void DeactivateTravelMode();

	/**
	 * Get current thrust output
	 * @return Thrust in Newtons
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	float GetCurrentThrust() const;

	/**
	 * Get current max speed contribution
	 * @return Max speed in m/s
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	float GetCurrentMaxSpeed() const;

	/**
	 * Get current fuel consumption rate
	 * @return Fuel consumption (units per second)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	float GetCurrentFuelConsumption() const;

	/**
	 * Get heat percentage (0-1)
	 * @return Heat as percentage of maximum
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	float GetHeatPercentage() const;

	/**
	 * Check if engine can activate boost
	 * @return True if boost can be activated
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	bool CanActivateBoost() const;

	/**
	 * Check if engine can activate travel mode
	 * @return True if travel mode can be activated
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine")
	bool CanActivateTravelMode() const;

protected:
	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	virtual void OnModuleEnabled_Implementation() override;
	virtual void OnModuleDisabled_Implementation() override;
	virtual void UpdateModule_Implementation(float DeltaTime) override;

	/**
	 * Called when engine overheats
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Engine")
	void OnEngineOverheated();

	/**
	 * Called when engine cools down from overheat
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Engine")
	void OnEngineCooledDown();

	/**
	 * Called when boost is activated
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Engine")
	void OnBoostActivated();

	/**
	 * Called when boost is deactivated
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Engine")
	void OnBoostDeactivated();

private:
	/**
	 * Update heat based on current throttle and conditions
	 * @param DeltaTime Time since last update
	 */
	void UpdateHeat(float DeltaTime);

	/**
	 * Update visual and audio effects
	 */
	void UpdateEffects();

	/**
	 * Initialize visual and audio components
	 */
	void InitializeEffectComponents();

	/**
	 * Cleanup effect components
	 */
	void CleanupEffectComponents();
};
