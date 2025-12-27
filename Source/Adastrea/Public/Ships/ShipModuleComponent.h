// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IShipModule.h"
#include "ShipModuleComponent.generated.h"

// Forward declarations
class UShipModuleDataAsset;

/**
 * Base component class for all ship modules
 * 
 * This is the base implementation of the IShipModule interface, providing
 * common functionality for all ship modules. Specific module types (engines,
 * weapons, shields, etc.) should inherit from this class.
 * 
 * Features:
 * - Data Asset integration for designer-friendly configuration
 * - Automatic mesh management and attachment
 * - Power and mass tracking
 * - Installation state management
 * 
 * Usage:
 * - Create derived classes for specific module types (UEngineModuleComponent, etc.)
 * - Configure via Data Assets in Blueprint
 * - Add to ships via UShipCustomizationComponent
 */
UCLASS(Abstract, ClassGroup=(Ship), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShipModuleComponent : public UActorComponent, public IShipModule
{
	GENERATED_BODY()

public:
	UShipModuleComponent();

	// ====================
	// CONFIGURATION
	// ====================

	/** Data asset defining this module's properties */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	TObjectPtr<UShipModuleDataAsset> ModuleData;

	/** Static mesh component for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module")
	TObjectPtr<UStaticMeshComponent> ModuleMeshComponent;

	// ====================
	// STATE
	// ====================

	/** Whether this module is currently enabled */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module State")
	bool bIsEnabled;

	/** Current slot this module is installed in */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module State")
	FShipModuleSlot CurrentSlot;

	/** Whether this module is currently installed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module State")
	bool bIsCurrentlyInstalled;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// IShipModule Interface Implementation
	// ====================

	virtual EShipModuleCategory GetModuleCategory_Implementation() const override;
	virtual EShipModuleSize GetModuleSize_Implementation() const override;
	virtual FText GetModuleName_Implementation() const override;
	virtual FText GetModuleDescription_Implementation() const override;
	virtual float GetPowerRequirement_Implementation() const override;
	virtual float GetModuleMass_Implementation() const override;
	virtual bool CanInstallInSlot_Implementation(const FShipModuleSlot& Slot) const override;
	virtual void OnModuleInstalled_Implementation(const FShipModuleSlot& Slot) override;
	virtual void OnModuleRemoved_Implementation(const FShipModuleSlot& Slot) override;
	virtual UStaticMesh* GetModuleMesh_Implementation() const override;
	virtual bool IsInstalled_Implementation() const override;
	virtual FShipModuleSlot GetInstalledSlot_Implementation() const override;
	virtual void SetModuleEnabled_Implementation(bool bEnabled) override;
	virtual bool IsModuleEnabled_Implementation() const override;

	// ====================
	// MODULE OPERATIONS
	// ====================

	/**
	 * Initialize the module with a data asset
	 * @param InModuleData Data asset to use for configuration
	 */
	UFUNCTION(BlueprintCallable, Category="Module")
	void InitializeWithData(UShipModuleDataAsset* InModuleData);

	/**
	 * Update visual mesh based on current data
	 */
	UFUNCTION(BlueprintCallable, Category="Module")
	void UpdateModuleMesh();

	/**
	 * Get the module's current operational status
	 * @return Status text for UI display
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module")
	FText GetModuleStatus() const;

protected:
	/**
	 * Called when module is enabled
	 * Override in derived classes for enable logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void OnModuleEnabled();

	/**
	 * Called when module is disabled
	 * Override in derived classes for disable logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void OnModuleDisabled();

	/**
	 * Update module logic per frame (if enabled)
	 * Override in derived classes for per-frame updates
	 * @param DeltaTime Time since last frame
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Module")
	void UpdateModule(float DeltaTime);
};
