#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "ShipUpgradeComponent.generated.h"

class AOutfittingModule;
class UPlayerTraderComponent;

/**
 * Installed upgrade tracking
 */
USTRUCT(BlueprintType)
struct FInstalledUpgrade
{
	GENERATED_BODY()

	/** The upgrade installed */
	UPROPERTY(BlueprintReadOnly, Category="Upgrade")
	UShipUpgradeDataAsset* Upgrade;

	/** Number of stacks (for non-unique upgrades); each stack takes one slot */
	UPROPERTY(BlueprintReadOnly, Category="Upgrade", meta=(ClampMin="1"))
	int32 StackCount;

	/** When this upgrade was installed */
	UPROPERTY(BlueprintReadOnly, Category="Upgrade")
	FDateTime InstallTimestamp;

	FInstalledUpgrade()
		: Upgrade(nullptr)
		, StackCount(1)
		, InstallTimestamp(FDateTime::Now())
	{}

	FInstalledUpgrade(UShipUpgradeDataAsset* InUpgrade, int32 InStackCount = 1)
		: Upgrade(InUpgrade)
		, StackCount(InStackCount)
		, InstallTimestamp(FDateTime::Now())
	{}
};

/**
 * Ship Upgrade Component
 *
 * The ship's fitted upgrades: which are installed, how many slots each category has,
 * and what they do to the ship's stats. ASpaceship listens to OnUpgradesChanged and
 * re-applies GetStatModifier() to its base stats (ASpaceship::RecalculateUpgradedStats).
 *
 * Upgrades are bought and removed at a docked station's outfitting module:
 * - PurchaseUpgrade(): the module must service the upgrade's category and tier, the ship
 *   needs a free slot in that category and the wallet must cover the module's price
 * - SellUpgrade(): the module must service the category; refunds SellBackFraction of list price
 * - InstallUpgrade()/UninstallUpgrade(): no station or credits involved (loading a save, scripts)
 *
 * Stat maths: Value = (Base + sum of additive bonuses) * product of multipliers, over every
 * installed stack, so the order upgrades were fitted in doesn't matter.
 */
UCLASS(BlueprintType, ClassGroup=(Ships), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShipUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipUpgradeComponent();

	// ====================
	// Core Properties
	// ====================

	/** All installed upgrades */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Upgrades")
	TArray<FInstalledUpgrade> InstalledUpgrades;

	/** Ship type ID for compatibility checking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrades")
	FName ShipTypeID;

	/** Maximum number of upgrade stacks across all categories */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrades", meta=(ClampMin="1", ClampMax="50"))
	int32 MaxUpgradeSlots;

	/** Slots per category; a category missing from the map has no slots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrades")
	TMap<EShipUpgradeCategory, int32> CategorySlots;

	/** Fraction of list price refunded when an upgrade is sold back */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrades", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SellBackFraction;

	// ====================
	// Upgrade Management
	// ====================

	/**
	 * Install an upgrade on this ship (no station or credits involved)
	 * @param Upgrade Upgrade to install
	 * @param bIgnoreRequirements Bypass slot, compatibility and requirement checks
	 * @return True if installation successful
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool InstallUpgrade(UShipUpgradeDataAsset* Upgrade, bool bIgnoreRequirements = false);

	/**
	 * Remove one stack of an upgrade (the whole entry when it was the last stack)
	 * @param UpgradeID ID of upgrade to remove
	 * @return True if a stack was removed
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool UninstallUpgrade(FName UpgradeID);

	/**
	 * Check if upgrade can be installed: slots, stacks, compatibility, conflicts, prerequisites
	 * @param Upgrade Upgrade to check
	 * @param OutReason Reason if cannot install
	 * @return True if can install
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool CanInstallUpgrade(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const;

	/** Check if upgrade is installed */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
	bool IsUpgradeInstalled(FName UpgradeID) const;

	/** Stack count for an upgrade (0 if not installed) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
	int32 GetUpgradeStackCount(FName UpgradeID) const;

	/** Remove every upgrade */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	void UninstallAllUpgrades();

	// ====================
	// Outfitting (buying and selling at a station)
	// ====================

	/**
	 * Whether Upgrade can be bought and fitted at Module with the credits in Wallet.
	 * @param OutReason Why not, for the outfitting screen
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades|Outfitting")
	bool CanPurchaseUpgrade(UShipUpgradeDataAsset* Upgrade, const AOutfittingModule* Module, const UPlayerTraderComponent* Wallet, FText& OutReason) const;

	/** Buy Upgrade at Module, paying from Wallet, and install it. */
	UFUNCTION(BlueprintCallable, Category="Upgrades|Outfitting")
	bool PurchaseUpgrade(UShipUpgradeDataAsset* Upgrade, AOutfittingModule* Module, UPlayerTraderComponent* Wallet, FText& OutReason);

	/** Whether one stack of UpgradeID can be removed at Module. */
	UFUNCTION(BlueprintCallable, Category="Upgrades|Outfitting")
	bool CanSellUpgrade(FName UpgradeID, const AOutfittingModule* Module, FText& OutReason) const;

	/** Remove one stack of UpgradeID at Module and refund GetSellPrice() into Wallet. */
	UFUNCTION(BlueprintCallable, Category="Upgrades|Outfitting")
	bool SellUpgrade(FName UpgradeID, AOutfittingModule* Module, UPlayerTraderComponent* Wallet, int32& OutRefund, FText& OutReason);

	/** Credits refunded for selling one stack of Upgrade. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Outfitting")
	int32 GetSellPrice(const UShipUpgradeDataAsset* Upgrade) const;

	// ====================
	// Slots
	// ====================

	/** Slots this ship has for Category */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Slots")
	int32 GetSlotCount(EShipUpgradeCategory Category) const;

	/** Slots in use for Category (one per installed stack) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Slots")
	int32 GetUsedSlotCount(EShipUpgradeCategory Category) const;

	/** Remaining slots across all categories (MaxUpgradeSlots minus installed stacks) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Slots")
	int32 GetRemainingUpgradeSlots() const;

	// ====================
	// Stat Modifiers
	// ====================

	/**
	 * Get modified stat value with all upgrade bonuses applied
	 * @param StatName Name of stat (see ShipUpgradeStats)
	 * @param BaseValue Base value before modifiers
	 * @return Modified value after all upgrades
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Stats")
	float GetStatModifier(FName StatName, float BaseValue) const;

	/**
	 * A stat as it would be with one more stack of AddedUpgrade and/or one fewer of
	 * RemovedUpgradeID - the outfitting screen's before/after preview.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Stats")
	float PreviewStat(FName StatName, float BaseValue, const UShipUpgradeDataAsset* AddedUpgrade, FName RemovedUpgradeID) const;

	/** Get total bonus percentage for a stat (as decimal, 0.25 = 25%) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Stats")
	float GetStatBonusPercentage(FName StatName) const;

	/** Map of every modified stat to its total bonus percentage */
	TMap<FName, float> GetAllStatModifiers() const;

	// ====================
	// Query Functions
	// ====================

	/** Get all installed upgrades */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	TArray<FInstalledUpgrade> GetInstalledUpgrades() const { return InstalledUpgrades; }

	/** Get installed upgrades in one category */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	TArray<FInstalledUpgrade> GetUpgradesByCategory(EShipUpgradeCategory Category) const;

	/** Number of installed upgrade entries */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	int32 GetInstalledUpgradeCount() const { return InstalledUpgrades.Num(); }

	/** Total list value of all installed upgrades */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	int32 GetTotalUpgradeValue() const;

	// ====================
	// Events
	// ====================

	/** Event fired when upgrade is installed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradeInstalled, UShipUpgradeDataAsset*, Upgrade, int32, StackCount);
	UPROPERTY(BlueprintAssignable, Category="Upgrades|Events")
	FOnUpgradeInstalled OnUpgradeInstalled;

	/** Event fired when upgrade is uninstalled */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeUninstalled, FName, UpgradeID);
	UPROPERTY(BlueprintAssignable, Category="Upgrades|Events")
	FOnUpgradeUninstalled OnUpgradeUninstalled;

	/** Event fired when installation fails */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradeInstallFailed, UShipUpgradeDataAsset*, Upgrade, FText, Reason);
	UPROPERTY(BlueprintAssignable, Category="Upgrades|Events")
	FOnUpgradeInstallFailed OnUpgradeInstallFailed;

	/** Fired after any install, uninstall or clear: the ship's stats need recalculating */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradesChanged);
	UPROPERTY(BlueprintAssignable, Category="Upgrades|Events")
	FOnUpgradesChanged OnUpgradesChanged;

protected:
	virtual void BeginPlay() override;

	/** Find installed upgrade by ID */
	FInstalledUpgrade* FindInstalledUpgrade(FName UpgradeID);

	/** Find installed upgrade (const version) */
	const FInstalledUpgrade* FindInstalledUpgrade(FName UpgradeID) const;

	/** Check if upgrade conflicts with installed upgrades */
	bool HasUpgradeConflicts(UShipUpgradeDataAsset* Upgrade) const;

	/** Prerequisites and player level */
	bool CheckUpgradeRequirements(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const;
};
