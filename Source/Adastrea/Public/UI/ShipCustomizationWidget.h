#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ShipCustomizationDataAsset.h"
#include "ShipCustomizationWidget.generated.h"

/**
 * Structure representing an installed module on a ship
 */
USTRUCT(BlueprintType)
struct FInstalledModule
{
	GENERATED_BODY()

	/** The customization data asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	UShipCustomizationDataAsset* Module;

	/** Hardpoint slot index where this is installed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	int32 HardpointSlotIndex;

	/** Whether this module is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	bool bIsActive;

	FInstalledModule()
		: Module(nullptr)
		, HardpointSlotIndex(-1)
		, bIsActive(true)
	{
	}

	FInstalledModule(UShipCustomizationDataAsset* InModule, int32 InSlotIndex)
		: Module(InModule)
		, HardpointSlotIndex(InSlotIndex)
		, bIsActive(true)
	{
	}
};

/**
 * Structure representing a hardpoint slot on a ship
 */
USTRUCT(BlueprintType)
struct FShipHardpoint
{
	GENERATED_BODY()

	/** Hardpoint type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	EShipHardpointType HardpointType;

	/** Display name for this hardpoint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	FText HardpointName;

	/** Currently installed module */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Customization")
	UShipCustomizationDataAsset* InstalledModule;

	FShipHardpoint()
		: HardpointType(EShipHardpointType::Small)
		, HardpointName(FText::FromString(TEXT("Hardpoint")))
		, InstalledModule(nullptr)
	{
	}
};

/**
 * Base widget class for ship customization UI
 * 
 * This widget provides the interface for customizing ships:
 * - View and manage hardpoint slots
 * - Install and remove modules
 * - Preview stat changes
 * - Manage power and weight budgets
 * - Visual preview of customizations
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class
 * - Design the customization interface
 * - Initialize with a spaceship reference
 * - Implement module selection and installation UI
 * - Override BlueprintNativeEvents for custom behavior
 * 
 * Integration:
 * - Works with ShipCustomizationDataAsset for modules
 * - Connects to Spaceship class for configuration
 * - Integrates with inventory for module storage
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UShipCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UShipCustomizationWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the customization widget with a spaceship
	 * @param InSpaceship The spaceship to customize
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	void InitializeCustomization(class ASpaceship* InSpaceship);
	virtual void InitializeCustomization_Implementation(class ASpaceship* InSpaceship);

	/**
	 * Refresh the entire customization display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	void RefreshCustomization();
	virtual void RefreshCustomization_Implementation();

	/**
	 * Install a module in a hardpoint
	 * @param Module The module to install
	 * @param HardpointIndex The hardpoint slot index
	 * @return True if installation was successful
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	bool InstallModule(UShipCustomizationDataAsset* Module, int32 HardpointIndex);
	virtual bool InstallModule_Implementation(UShipCustomizationDataAsset* Module, int32 HardpointIndex);

	/**
	 * Remove a module from a hardpoint
	 * @param HardpointIndex The hardpoint slot index
	 * @return True if removal was successful
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	bool RemoveModule(int32 HardpointIndex);
	virtual bool RemoveModule_Implementation(int32 HardpointIndex);

	/**
	 * Preview stat changes from installing a module
	 * @param Module The module to preview
	 * @param HardpointIndex The hardpoint where it would be installed
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	void PreviewModuleStats(UShipCustomizationDataAsset* Module, int32 HardpointIndex);
	virtual void PreviewModuleStats_Implementation(UShipCustomizationDataAsset* Module, int32 HardpointIndex);

	/**
	 * Clear the stat preview
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ship Customization")
	void ClearStatsPreview();
	virtual void ClearStatsPreview_Implementation();

	/**
	 * Get all hardpoints for the current ship
	 * @return Array of hardpoint slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FShipHardpoint> GetHardpoints() const;

	/**
	 * Get all installed modules
	 * @return Array of installed modules
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	TArray<FInstalledModule> GetInstalledModules() const;

	/**
	 * Calculate total power consumption of all modules
	 * @return Total power consumption
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetTotalPowerConsumption() const;

	/**
	 * Calculate total mass of all modules
	 * @return Total mass
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	float GetTotalModuleMass() const;

	/**
	 * Check if a module can be installed in a hardpoint
	 * @param Module The module to check
	 * @param HardpointIndex The hardpoint slot
	 * @return True if the module is compatible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ship Customization")
	bool CanInstallModule(UShipCustomizationDataAsset* Module, int32 HardpointIndex) const;

	/**
	 * Apply all customizations to the ship
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void ApplyCustomizations();

	/**
	 * Reset all customizations to defaults
	 */
	UFUNCTION(BlueprintCallable, Category="Ship Customization")
	void ResetCustomizations();

protected:
	/** Reference to the spaceship being customized */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	class ASpaceship* Spaceship;

	/** All hardpoint slots on the ship */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	TArray<FShipHardpoint> Hardpoints;

	/** Maximum power capacity of the ship */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	float MaxPowerCapacity;

	/** Whether a preview is currently active */
	UPROPERTY(BlueprintReadOnly, Category="Ship Customization")
	bool bIsPreviewActive;

	/** Native initialization */
	virtual void NativeConstruct() override;

	/**
	 * Initialize hardpoints for the current ship
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Ship Customization")
	void InitializeHardpoints();
	virtual void InitializeHardpoints_Implementation();
};
