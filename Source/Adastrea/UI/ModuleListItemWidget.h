// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ModuleListItemWidget.generated.h"

// Forward declarations
class ASpaceStationModule;
struct FStationModuleEntry;

/**
 * Delegate for module selection
 */
DECLARE_DELEGATE_OneParam(FOnModuleSelected, TSubclassOf<ASpaceStationModule>);

/**
 * Module List Item Widget
 * 
 * Displays a single module entry in the catalog list.
 * Shows module name, description, cost, and build button.
 * 
 * Usage:
 * 1. Create Blueprint widget based on this class
 * 2. Layout UI with named widgets (must match BindWidget properties)
 * 3. Set module data via SetModuleData()
 * 4. Bind OnModuleSelected delegate to handle selection
 */
UCLASS()
class ADASTREA_API UModuleListItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UModuleListItemWidget(const FObjectInitializer& ObjectInitializer);

	// =====================
	// UI Widget References (BindWidget)
	// =====================

	/** Text block displaying module name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ModuleNameText;

	/** Text block displaying module description */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* DescriptionText;

	/** Text block displaying build cost */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* CostText;

	/** Button to select/build this module */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BuildButton;

	// =====================
	// Data
	// =====================

	/** The module class this item represents */
	UPROPERTY()
	TSubclassOf<ASpaceStationModule> ModuleClass;

	/** Delegate called when module is selected */
	FOnModuleSelected OnModuleSelected;

	// =====================
	// Public Functions
	// =====================

	/**
	 * Set the module data for this item
	 * @param Entry The module entry data
	 */
	void SetModuleData(const FStationModuleEntry& Entry);

protected:
	/** Called when the widget is constructed */
	virtual void NativeConstruct() override;

	/** Called when the widget is destroyed */
	virtual void NativeDestruct() override;

	/**
	 * Handle build button clicked
	 */
	UFUNCTION()
	void OnBuildButtonClicked();
};
