#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "ShipUpgradeCatalogSubsystem.generated.h"

/**
 * Every ship upgrade outfitting modules can sell.
 *
 * Built at startup from Content/Data/ShipUpgradeCatalog.json (same runtime-JSON pattern
 * as the crafting tree): each entry becomes a transient UShipUpgradeDataAsset owned by
 * this subsystem. Authored UShipUpgradeDataAsset assets can still be sold by adding them
 * to an AOutfittingModule's AdditionalStock.
 *
 * The save system resolves installed upgrades by ID through FindUpgrade().
 */
UCLASS()
class ADASTREA_API UShipUpgradeCatalogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** The catalog subsystem of WorldContext's game instance, or null (e.g. a bare test world). */
	static UShipUpgradeCatalogSubsystem* Get(const UObject* WorldContext);

	/** Catalog path relative to the project Content directory. */
	static const TCHAR* GetCatalogPath() { return TEXT("Data/ShipUpgradeCatalog.json"); }

	/**
	 * Parse catalog JSON into upgrade objects created in Outer.
	 * @param OutErrors Optional; receives one line per malformed entry (those entries are skipped)
	 * @return Number of upgrades parsed
	 */
	static int32 ParseCatalog(const FString& Json, UObject* Outer, TArray<UShipUpgradeDataAsset*>& OutUpgrades, TArray<FString>* OutErrors = nullptr);

	/** The upgrade with this ID, or null. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Catalog")
	UShipUpgradeDataAsset* FindUpgrade(FName UpgradeID) const;

	/** Upgrades of one category up to and including MaxTier, cheapest tier first. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Catalog")
	TArray<UShipUpgradeDataAsset*> GetUpgrades(EShipUpgradeCategory Category, EUpgradeTier MaxTier) const;

	/** Every upgrade in the catalog. */
	const TArray<TObjectPtr<UShipUpgradeDataAsset>>& GetAllUpgrades() const { return Upgrades; }

	/** Replace the catalog contents (tests, or tools that build upgrades in code). */
	void SetUpgrades(const TArray<UShipUpgradeDataAsset*>& InUpgrades);

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UShipUpgradeDataAsset>> Upgrades;
};
