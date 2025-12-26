// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MaterialDataAsset.generated.h"

/**
 * Enum for material categories
 * Defines the type of material for classification and filtering
 */
UENUM(BlueprintType)
enum class EMaterialCategory : uint8
{
    Mineral UMETA(DisplayName = "Mineral"),
    RefinedMineral UMETA(DisplayName = "Refined Mineral"),
    Component UMETA(DisplayName = "Component"),
    Electronics UMETA(DisplayName = "Electronics"),
    ConstructionPart UMETA(DisplayName = "Construction Part"),
    ShipPart UMETA(DisplayName = "Ship Part"),
    Food UMETA(DisplayName = "Food"),
    Research UMETA(DisplayName = "Research"),
    Organic UMETA(DisplayName = "Organic"),
    Synthetic UMETA(DisplayName = "Synthetic"),
    Other UMETA(DisplayName = "Other")
};

/**
 * Enum for storage types
 * Defines how the material must be stored
 */
UENUM(BlueprintType)
enum class EStorageType : uint8
{
    Solid UMETA(DisplayName = "Solid"),
    Liquid UMETA(DisplayName = "Liquid"),
    Gas UMETA(DisplayName = "Gas"),
    Refrigerated UMETA(DisplayName = "Refrigerated"),
    Hazardous UMETA(DisplayName = "Hazardous"),
    Other UMETA(DisplayName = "Other")
};

/**
 * Enum for material rarity
 * Defines how rare and valuable the material is
 */
UENUM(BlueprintType)
enum class EMaterialRarity : uint8
{
    Common UMETA(DisplayName = "Common"),
    Uncommon UMETA(DisplayName = "Uncommon"),
    Rare UMETA(DisplayName = "Rare"),
    VeryRare UMETA(DisplayName = "Very Rare"),
    Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Data Asset for storing material information.
 * This allows designers to create material configurations as Blueprint Data Assets.
 */
UCLASS(BlueprintType)
class ADASTREA_API UMaterialDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Display name of the material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Info")
    FText MaterialName;

    // Brief description of the material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Info", meta=(MultiLine=true))
    FText Description;

    // Material category (Mineral, Component, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Info")
    EMaterialCategory Category;

    // Storage type required for this material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Storage")
    EStorageType StorageType;

    // Rarity of the material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Stats")
    EMaterialRarity Rarity;

    // Base value per unit (in credits)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Stats", meta=(ClampMin="0"))
    float Value;

    // Mass per unit (in kilograms)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Stats", meta=(ClampMin="0.0"))
    float Mass;

    // General gameplay tags for filtering and querying
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Tags")
    TArray<FName> Tags;

    // Module tags - which station modules can process/use this material
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Material Tags")
    TArray<FName> ModuleTags;

    UMaterialDataAsset();

    // ====================
    // Material Query Functions
    // ====================

    /**
     * Check if material has a specific tag
     * @param Tag The tag to check for
     * @return True if material has the tag
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    bool HasTag(FName Tag) const;

    /**
     * Check if material can be processed by a specific module type
     * @param ModuleTag The module tag to check for
     * @return True if material can be processed by this module
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    bool CanBeProcessedByModule(FName ModuleTag) const;

    /**
     * Get all tags for this material
     * @return Array of all tags
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    TArray<FName> GetTags() const;

    /**
     * Get all module tags for this material
     * @return Array of all module tags
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    TArray<FName> GetModuleTags() const;

    /**
     * Get the storage volume required per unit (based on mass and category)
     * @return Storage volume in cubic meters
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    float GetStorageVolume() const;

    /**
     * Check if this material matches a specific category
     * @param CheckCategory The category to check against
     * @return True if material is of the specified category
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    bool IsCategory(EMaterialCategory CheckCategory) const;

    /**
     * Check if this material requires special storage
     * @return True if storage type is Refrigerated, Hazardous, or requires special handling
     */
    UFUNCTION(BlueprintCallable, Category="Material")
    bool RequiresSpecialStorage() const;
};
