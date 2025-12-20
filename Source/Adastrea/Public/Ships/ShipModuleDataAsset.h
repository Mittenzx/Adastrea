// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interfaces/IShipModule.h"
#include "ShipModuleDataAsset.generated.h"

/**
 * Base Data Asset for ship module configurations
 * 
 * This Data Asset stores all configuration for a ship module, allowing designers
 * to create module variants without C++ knowledge. Specific module types should
 * inherit from this class to add type-specific properties.
 * 
 * Features:
 * - Designer-friendly configuration
 * - Common properties for all modules
 * - Support for visual meshes
 * - Power and mass specifications
 * 
 * Usage:
 * 1. Create derived class for specific module type (UEngineModuleDataAsset, etc.)
 * 2. Create Blueprint Data Asset based on derived class
 * 3. Configure properties in editor
 * 4. Assign to UShipModuleComponent instances
 */
UCLASS(Abstract, BlueprintType)
class ADASTREA_API UShipModuleDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// BASIC INFO
	// ====================

	/** Display name of the module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText ModuleName;

	/** Brief description of the module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Unique identifier for this module type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FName ModuleID;

	/** Category of this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EShipModuleCategory Category;

	/** Size classification of this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EShipModuleSize Size;

	// ====================
	// PHYSICAL PROPERTIES
	// ====================

	/** Mass of the module in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical Properties", meta=(ClampMin="0", ClampMax="1000000"))
	float Mass;

	/** Power consumption (positive) or generation (negative) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical Properties", meta=(ClampMin="-10000", ClampMax="10000"))
	float PowerRequirement;

	/** Visual mesh for this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical Properties")
	UStaticMesh* ModuleMesh;

	/** Material to apply to the module mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physical Properties")
	UMaterialInterface* ModuleMaterial;

	// ====================
	// REQUIREMENTS
	// ====================

	/** Minimum tech level required to use this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="1", ClampMax="10"))
	int32 RequiredTechLevel;

	/** Credits cost to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="0", ClampMax="10000000"))
	int32 PurchaseCost;

	/** Whether this module is currently available for purchase */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
	bool bIsAvailable;

	// ====================
	// LORE/FLAVOR
	// ====================

	/** Manufacturer of this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
	FText Manufacturer;

	/** Rarity tier (Common, Uncommon, Rare, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
	FText RarityTier;

	/** Lore notes or flavor text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore", meta=(MultiLine=true))
	FText LoreNotes;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lore")
	UTexture2D* ModuleIcon;

	// ====================
	// Constructor
	// ====================

	UShipModuleDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Get the effective rating/quality of this module (0-100)
	 * Override in derived classes for type-specific calculations
	 * @return Overall quality rating
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module Stats")
	virtual float GetModuleRating() const;

	/**
	 * Check if module is suitable for a specific ship class
	 * @param ShipClass The ship class to check
	 * @return True if module is appropriate for this ship class
	 */
	UFUNCTION(BlueprintCallable, Category="Module Stats")
	virtual bool IsSuitableForShipClass(const FString& ShipClass) const;

	/**
	 * Get a summary string for UI display
	 * @return Formatted summary text
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Module Stats")
	FText GetModuleSummary() const;

#if WITH_EDITOR
	/**
	 * Validate Data Asset properties
	 * Called when asset is saved or validated in editor
	 */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
