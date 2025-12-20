// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IShipModule.generated.h"

// Forward declarations
class UShipModuleDataAsset;

/**
 * Enum defining ship module categories
 */
UENUM(BlueprintType)
enum class EShipModuleCategory : uint8
{
	Engine		UMETA(DisplayName = "Engine"),
	Weapon		UMETA(DisplayName = "Weapon"),
	Shield		UMETA(DisplayName = "Shield Generator"),
	Utility		UMETA(DisplayName = "Utility/Sensors"),
	Hull		UMETA(DisplayName = "Hull Modification"),
	PowerPlant	UMETA(DisplayName = "Power Plant"),
	Storage		UMETA(DisplayName = "Cargo/Storage"),
	Special		UMETA(DisplayName = "Special Equipment")
};

/**
 * Enum defining module sizes for slot compatibility
 */
UENUM(BlueprintType)
enum class EShipModuleSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	Capital		UMETA(DisplayName = "Capital")
};

/**
 * Struct representing a module connection point/slot on a ship
 */
USTRUCT(BlueprintType)
struct FShipModuleSlot
{
	GENERATED_BODY()

	/** Unique identifier for this slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	FName SlotID;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	FText SlotName;

	/** Category of modules this slot accepts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	EShipModuleCategory AcceptedCategory;

	/** Maximum size of module this slot can hold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	EShipModuleSize MaxSize;

	/** Relative location of the slot on the ship */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	FVector SlotLocation;

	/** Relative rotation of the slot on the ship */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module Slot")
	FRotator SlotRotation;

	/** Whether this slot is currently occupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Module Slot")
	bool bIsOccupied;

	/** Component currently installed in this slot (if any) */
	UPROPERTY()
	TWeakObjectPtr<UActorComponent> InstalledModule;

	FShipModuleSlot()
		: SlotID(NAME_None)
		, SlotName(FText::FromString("Empty Slot"))
		, AcceptedCategory(EShipModuleCategory::Utility)
		, MaxSize(EShipModuleSize::Medium)
		, SlotLocation(FVector::ZeroVector)
		, SlotRotation(FRotator::ZeroRotator)
		, bIsOccupied(false)
	{
	}
};

/**
 * UInterface class for IShipModule
 * This is required by Unreal's interface system
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UShipModule : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for all ship modules
 * 
 * This interface defines the contract that all ship modules must implement,
 * allowing for a consistent way to manage, attach, and detach modules from ships.
 * 
 * Implements:
 * - Module identification and categorization
 * - Installation and removal logic
 * - Power and resource management
 * - Slot compatibility checking
 * 
 * Usage (C++):
 * ```cpp
 * class UMyModuleComponent : public UActorComponent, public IShipModule
 * {
 *     virtual EShipModuleCategory GetModuleCategory_Implementation() const override;
 *     virtual bool CanInstallInSlot_Implementation(const FShipModuleSlot& Slot) const override;
 *     // ... implement other interface methods
 * };
 * ```
 * 
 * Usage (Blueprint):
 * - Create a Blueprint based on a component class that implements IShipModule
 * - Override BlueprintNativeEvent functions as needed
 * - The interface provides default implementations for basic functionality
 */
class ADASTREA_API IShipModule
{
	GENERATED_BODY()

public:
	/**
	 * Get the category of this module
	 * @return Module category (Engine, Weapon, Shield, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	EShipModuleCategory GetModuleCategory() const;

	/**
	 * Get the size of this module
	 * @return Module size (Small, Medium, Large, Capital)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	EShipModuleSize GetModuleSize() const;

	/**
	 * Get the display name of this module
	 * @return User-friendly module name
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	FText GetModuleName() const;

	/**
	 * Get the description of this module
	 * @return Module description for UI
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	FText GetModuleDescription() const;

	/**
	 * Get the power consumption of this module
	 * @return Power required to operate (negative for power generation)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	float GetPowerRequirement() const;

	/**
	 * Get the mass of this module
	 * @return Mass in kilograms (affects ship performance)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	float GetModuleMass() const;

	/**
	 * Check if this module can be installed in the given slot
	 * @param Slot The slot to check compatibility with
	 * @return True if module can be installed in this slot
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	bool CanInstallInSlot(const FShipModuleSlot& Slot) const;

	/**
	 * Called when module is installed in a slot
	 * Override to initialize module-specific functionality
	 * @param Slot The slot this module was installed into
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Module")
	void OnModuleInstalled(const FShipModuleSlot& Slot);

	/**
	 * Called when module is removed from a slot
	 * Override to cleanup module-specific functionality
	 * @param Slot The slot this module was removed from
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Module")
	void OnModuleRemoved(const FShipModuleSlot& Slot);

	/**
	 * Get the visual mesh for this module (if any)
	 * @return Static mesh to display, or nullptr for no visual
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	UStaticMesh* GetModuleMesh() const;

	/**
	 * Check if this module is currently installed
	 * @return True if module is installed in a slot
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	bool IsInstalled() const;

	/**
	 * Get the slot this module is currently installed in
	 * @return Slot data, or empty slot if not installed
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	FShipModuleSlot GetInstalledSlot() const;

	/**
	 * Enable or disable the module
	 * @param bEnabled True to enable, false to disable
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	void SetModuleEnabled(bool bEnabled);

	/**
	 * Check if module is currently enabled
	 * @return True if module is enabled and operational
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Module")
	bool IsModuleEnabled() const;
};
