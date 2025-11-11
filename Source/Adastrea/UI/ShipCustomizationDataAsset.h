#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipCustomizationDataAsset.generated.h"

/**
 * Type of ship customization option
 */
UENUM(BlueprintType)
enum class EShipCustomizationType : uint8
{
	WeaponModule UMETA(DisplayName="Weapon Module"),
	EngineModule UMETA(DisplayName="Engine Module"),
	ShieldModule UMETA(DisplayName="Shield Module"),
	ArmorPlating UMETA(DisplayName="Armor Plating"),
	CargoExpansion UMETA(DisplayName="Cargo Expansion"),
	SensorArray UMETA(DisplayName="Sensor Array"),
	LifeSupport UMETA(DisplayName="Life Support"),
	PowerCore UMETA(DisplayName="Power Core"),
	Cosmetic UMETA(DisplayName="Cosmetic")
};

/**
 * Ship hardpoint/slot type for module installation
 */
UENUM(BlueprintType)
enum class EShipHardpointType : uint8
{
	Small UMETA(DisplayName="Small"),
	Medium UMETA(DisplayName="Medium"),
	Large UMETA(DisplayName="Large"),
	Utility UMETA(DisplayName="Utility"),
	Internal UMETA(DisplayName="Internal")
};

/**
 * Data Asset defining a ship customization option
 * 
 * This data asset configures modules and components that can be installed on ships:
 * - Module type and hardpoint requirements
 * - Stat modifications and bonuses
 * - Power and resource requirements
 * - Visual appearance changes
 * - Installation requirements and restrictions
 * 
 * Usage:
 * - Create a Data Asset based on this class
 * - Configure module properties and effects
 * - Reference in ship customization systems
 * - Use in trading/crafting for module acquisition
 * 
 * Examples:
 * - Weapon modules (lasers, missiles, railguns)
 * - Engine upgrades (speed boost, efficiency)
 * - Shield generators (capacity, recharge rate)
 * - Armor plating (damage resistance)
 */
UCLASS(BlueprintType)
class ADASTREA_API UShipCustomizationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of the customization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText CustomizationName;

	/** Detailed description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	UTexture2D* Icon;

	/** Type of customization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Classification")
	EShipCustomizationType CustomizationType;

	/** Required hardpoint type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Installation")
	EShipHardpointType RequiredHardpoint;

	/** Power consumption of this module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="0.0"))
	float PowerConsumption;

	/** Mass/weight of the module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="0.0"))
	float Mass;

	/** Base value in credits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy", meta=(ClampMin="0"))
	int32 BaseValue;

	/** Stat modifiers (name -> value pairs) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	TMap<FName, float> StatModifiers;

	/** Custom properties for module-specific behavior */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced")
	TMap<FName, FString> CustomProperties;

	/** Static mesh for visual representation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	UStaticMesh* ModuleMesh;

	/** Material override for customization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	UMaterialInterface* MaterialOverride;

	/** Minimum tech level required to install */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements", meta=(ClampMin="1", ClampMax="10"))
	int32 MinimumTechLevel;

	/** Faction restrictions (empty = available to all) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
	TArray<FString> RestrictedToFactions;

	/**
	 * Get a stat modifier value
	 * @param StatName Name of the stat
	 * @param DefaultValue Default value if stat not found
	 * @return The stat modifier value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetStatModifier(FName StatName, float DefaultValue) const;

	/**
	 * Get a custom property value
	 * @param PropertyName Name of the property
	 * @param DefaultValue Default value if property not found
	 * @return The property value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	FString GetCustomProperty(FName PropertyName, const FString& DefaultValue) const;

	/**
	 * Check if this customization is compatible with a specific ship class
	 * @param ShipClassName The ship class to check
	 * @return True if compatible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool IsCompatibleWithShip(const FString& ShipClassName) const;
};
