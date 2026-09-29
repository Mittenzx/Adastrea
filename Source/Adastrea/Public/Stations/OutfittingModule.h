// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "OutfittingModule.generated.h"

/**
 * Outfitting bay for space stations: where docked ships buy, fit and remove upgrades.
 *
 * A station can only work on the upgrade categories its outfitting modules service, and
 * only up to their tier. This general bay handles every category at a low tier; the
 * specialist workshops below handle one category at a higher tier. Each station-editor
 * upgrade level of the module (ASpaceStationModule::UpgradeLevel) raises its tier by one.
 *
 * Stock is the ship upgrade catalog (UShipUpgradeCatalogSubsystem) filtered by category
 * and tier, plus any authored upgrades in AdditionalStock.
 *
 * Power Consumption: 60 units
 * Module Group: Docking
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AOutfittingModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	AOutfittingModule();

	/** Upgrade categories this module can fit and remove. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfitting")
	TArray<EShipUpgradeCategory> ServicedCategories;

	/** Highest upgrade tier this module can fit before station-editor upgrades. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfitting")
	EUpgradeTier BaseMaxTier = EUpgradeTier::Tier2;

	/** Multiplier on catalog prices here (1.0 = list price). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfitting", meta=(ClampMin="0.1", ClampMax="10.0"))
	float PriceMultiplier = 1.0f;

	/** Authored upgrades sold here on top of the catalog (still limited to serviced categories). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfitting")
	TArray<TObjectPtr<UShipUpgradeDataAsset>> AdditionalStock;

	/** Whether this module can work on Category (and isn't destroyed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Outfitting")
	bool ServicesCategory(EShipUpgradeCategory Category) const;

	/** Highest tier this module can fit: BaseMaxTier raised by its UpgradeLevel. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Outfitting")
	EUpgradeTier GetMaxTier() const;

	/** Whether this module can fit Upgrade: right category and within tier. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Outfitting")
	bool CanFit(const UShipUpgradeDataAsset* Upgrade) const;

	/** Upgrades of Category for sale here, lowest tier first. Empty if not serviced. */
	UFUNCTION(BlueprintCallable, Category="Outfitting")
	TArray<UShipUpgradeDataAsset*> GetStock(EShipUpgradeCategory Category) const;

	/** What this module charges for Upgrade. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Outfitting")
	int32 GetPrice(const UShipUpgradeDataAsset* Upgrade) const;
};

/** Engine workshop: thrusters, drives and boost injectors. Power: 80 units. */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AEngineWorkshopModule : public AOutfittingModule
{
	GENERATED_BODY()
public:
	AEngineWorkshopModule();
};

/** Armoury: weapon hardpoints and beam emitters. Power: 70 units. */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AArmouryModule : public AOutfittingModule
{
	GENERATED_BODY()
public:
	AArmouryModule();
};

/** Shield workshop: shield generators and capacitors. Power: 90 units. */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AShieldWorkshopModule : public AOutfittingModule
{
	GENERATED_BODY()
public:
	AShieldWorkshopModule();
};

/** Hull works: armour plating and structural reinforcement. Power: 60 units. */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AHullWorksModule : public AOutfittingModule
{
	GENERATED_BODY()
public:
	AHullWorksModule();
};

/** Cargo refit yard: hold extensions and compression racks. Power: 40 units. */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ACargoRefitModule : public AOutfittingModule
{
	GENERATED_BODY()
public:
	ACargoRefitModule();
};
