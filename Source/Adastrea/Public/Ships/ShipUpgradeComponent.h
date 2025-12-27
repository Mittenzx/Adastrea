#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "ShipUpgradeComponent.generated.h"

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

	/** Number of stacks (for non-unique upgrades) */
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
 * Manages ship upgrades and applies stat modifiers.
 * Tracks installed upgrades and calculates cumulative bonuses.
 * 
 * Usage:
 * - Attach to spaceship actor
 * - Install upgrades via InstallUpgrade()
 * - Remove upgrades via UninstallUpgrade()
 * - Query stat bonuses via GetStatModifier()
 * - Check compatibility via CanInstallUpgrade()
 * 
 * Example:
 * - Install upgrade: InstallUpgrade(UpgradeDataAsset)
 * - Check bonus: GetStatModifier("MaxSpeed", 100.0f) returns 120.0f (+20%)
 * - Remove upgrade: UninstallUpgrade(UpgradeID)
 * - List upgrades: GetInstalledUpgrades()
 * 
 * Integration:
 * - Spaceship system applies upgrade bonuses to stats
 * - PlayerUnlockComponent gates upgrade availability
 * - Trading system provides upgrade purchases
 * - Material system provides upgrade materials
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

	/** Maximum number of upgrades that can be installed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Upgrades", meta=(ClampMin="1", ClampMax="50"))
	int32 MaxUpgradeSlots;

	// ====================
	// Upgrade Management
	// ====================

	/**
	 * Install an upgrade on this ship
	 * @param Upgrade Upgrade to install
	 * @param bIgnoreRequirements Bypass requirement checks
	 * @return True if installation successful
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool InstallUpgrade(UShipUpgradeDataAsset* Upgrade, bool bIgnoreRequirements = false);

	/**
	 * Uninstall an upgrade from this ship
	 * @param UpgradeID ID of upgrade to remove
	 * @return True if uninstallation successful
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool UninstallUpgrade(FName UpgradeID);

	/**
	 * Check if upgrade can be installed
	 * @param Upgrade Upgrade to check
	 * @param OutReason Reason if cannot install
	 * @return True if can install
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	bool CanInstallUpgrade(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const;

	/**
	 * Check if upgrade is installed
	 * @param UpgradeID Upgrade to check
	 * @return True if installed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
	bool IsUpgradeInstalled(FName UpgradeID) const;

	/**
	 * Get stack count for an upgrade
	 * @param UpgradeID Upgrade to check
	 * @return Stack count (0 if not installed)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
	int32 GetUpgradeStackCount(FName UpgradeID) const;

	/**
	 * Uninstall all upgrades
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades")
	void UninstallAllUpgrades();

	// ====================
	// Stat Modifiers
	// ====================

	/**
	 * Get modified stat value with all upgrade bonuses applied
	 * @param StatName Name of stat
	 * @param BaseValue Base value before modifiers
	 * @return Modified value after all upgrades
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Stats")
	float GetStatModifier(FName StatName, float BaseValue) const;

	/**
	 * Get total bonus percentage for a stat (as decimal, 0.25 = 25%)
	 * @param StatName Name of stat
	 * @return Total bonus percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Stats")
	float GetStatBonusPercentage(FName StatName) const;

	/**
	 * Get all stat modifiers from upgrades
	 * @return Map of stat name to total modifier value
	 */
	UFUNCTION(BlueprintCallable, Category="Upgrades|Stats")
	TMap<FName, float> GetAllStatModifiers() const;

	// ====================
	// Query Functions
	// ====================

	/**
	 * Get all installed upgrades
	 * @return Array of installed upgrades
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	TArray<FInstalledUpgrade> GetInstalledUpgrades() const { return InstalledUpgrades; }

	/**
	 * Get installed upgrades by category
	 * @param Category Category to filter
	 * @return Array of installed upgrades in category
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	TArray<FInstalledUpgrade> GetUpgradesByCategory(EShipUpgradeCategory Category) const;

	/**
	 * Get number of installed upgrades
	 * @return Upgrade count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	int32 GetInstalledUpgradeCount() const { return InstalledUpgrades.Num(); }

	/**
	 * Get remaining upgrade slots
	 * @return Number of available slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades|Query")
	int32 GetRemainingUpgradeSlots() const;

	/**
	 * Get total value of all installed upgrades
	 * @return Total credit value
	 */
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

protected:
	virtual void BeginPlay() override;

	/**
	 * Find installed upgrade by ID
	 * @param UpgradeID ID to find
	 * @return Pointer to installed upgrade, or nullptr
	 */
	FInstalledUpgrade* FindInstalledUpgrade(FName UpgradeID);

	/**
	 * Find installed upgrade (const version)
	 * @param UpgradeID ID to find
	 * @return Pointer to installed upgrade, or nullptr
	 */
	const FInstalledUpgrade* FindInstalledUpgrade(FName UpgradeID) const;

	/**
	 * Check if upgrade conflicts with installed upgrades
	 * @param Upgrade Upgrade to check
	 * @return True if conflicts exist
	 */
	bool HasUpgradeConflicts(UShipUpgradeDataAsset* Upgrade) const;

	/**
	 * Check requirements for installing upgrade
	 * @param Upgrade Upgrade to check
	 * @param OutReason Reason if requirements not met
	 * @return True if requirements met
	 */
	bool CheckUpgradeRequirements(UShipUpgradeDataAsset* Upgrade, FText& OutReason) const;
};
