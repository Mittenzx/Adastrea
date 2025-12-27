#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipUpgradeDataAsset.generated.h"

/**
 * Ship upgrade categories
 */
UENUM(BlueprintType)
enum class EShipUpgradeCategory : uint8
{
	Weapons         UMETA(DisplayName = "Weapons"),        // Weapon systems
	Engines         UMETA(DisplayName = "Engines"),        // Propulsion and speed
	Shields         UMETA(DisplayName = "Shields"),        // Defensive systems
	Hull            UMETA(DisplayName = "Hull"),           // Armor and structure
	Systems         UMETA(DisplayName = "Systems"),        // Ship systems
	Cargo           UMETA(DisplayName = "Cargo"),          // Cargo capacity
	Power           UMETA(DisplayName = "Power"),          // Power generation
	Sensors         UMETA(DisplayName = "Sensors")         // Scanning and detection
};

/**
 * Upgrade tier/level
 */
UENUM(BlueprintType)
enum class EUpgradeTier : uint8
{
	Tier1       UMETA(DisplayName = "Tier 1"),     // Basic upgrade
	Tier2       UMETA(DisplayName = "Tier 2"),     // Improved upgrade
	Tier3       UMETA(DisplayName = "Tier 3"),     // Advanced upgrade
	Tier4       UMETA(DisplayName = "Tier 4"),     // Elite upgrade
	Tier5       UMETA(DisplayName = "Tier 5")      // Legendary upgrade
};

/**
 * Stat modifier applied by upgrade
 */
USTRUCT(BlueprintType)
struct FShipUpgradeStatModifier
{
	GENERATED_BODY()

	/** Name of stat to modify (e.g., "MaxSpeed", "ShieldCapacity", "WeaponDamage") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modifier")
	FName StatName;

	/** Additive bonus value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modifier")
	float AdditiveBonus;

	/** Multiplicative bonus (1.0 = no change, 1.5 = 50% increase) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modifier", meta=(ClampMin="0.1", ClampMax="10.0"))
	float MultiplicativeBonus;

	FShipUpgradeStatModifier()
		: StatName(NAME_None)
		, AdditiveBonus(0.0f)
		, MultiplicativeBonus(1.0f)
	{}
};

/**
 * Upgrade requirement (cost and prerequisites)
 */
USTRUCT(BlueprintType)
struct FShipUpgradeRequirement
{
	GENERATED_BODY()

	/** Credit cost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement", meta=(ClampMin="0"))
	int32 CreditCost;

	/** Required player level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement", meta=(ClampMin="1"))
	int32 RequiredPlayerLevel;

	/** Required reputation with faction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement")
	FName RequiredFactionID;

	/** Minimum reputation value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement", meta=(ClampMin="-100", ClampMax="100"))
	int32 MinimumReputation;

	/** Required materials (MaterialID -> quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement")
	TMap<FName, int32> RequiredMaterials;

	/** Prerequisite upgrades that must be installed first */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirement")
	TArray<class UShipUpgradeDataAsset*> PrerequisiteUpgrades;

	FShipUpgradeRequirement()
		: CreditCost(0)
		, RequiredPlayerLevel(1)
		, RequiredFactionID(NAME_None)
		, MinimumReputation(0)
	{}
};

/**
 * Ship Upgrade Data Asset
 * 
 * Defines an upgrade that can be installed on ships to improve their performance.
 * Uses Data Asset pattern for designer-friendly configuration.
 * 
 * Usage:
 * - Create Blueprint Data Asset based on UShipUpgradeDataAsset
 * - Configure upgrade category, tier, and stat modifiers
 * - Set requirements (cost, level, materials)
 * - Define prerequisite upgrade chains
 * - Apply to ships via ShipUpgradeComponent
 * 
 * Example Upgrades:
 * - "Enhanced Thrusters Mk2": +20% speed, Tier 2, 5000 credits
 * - "Reinforced Hull Plating": +500 armor, Tier 1, 2000 credits
 * - "Advanced Shield Generator": +50% shield capacity, Tier 3, 10000 credits
 * - "Overcharged Weapons": +25% weapon damage, Tier 4, 15000 credits
 * 
 * Integration:
 * - ShipUpgradeComponent tracks installed upgrades
 * - Spaceship system applies stat modifiers
 * - Trading/Material system provides upgrade materials
 * - PlayerUnlockComponent can gate upgrade availability
 */
UCLASS(BlueprintType)
class ADASTREA_API UShipUpgradeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UShipUpgradeDataAsset();

	// ====================
	// Basic Info
	// ====================

	/** Unique upgrade ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FName UpgradeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EShipUpgradeCategory Category;

	/** Tier/Level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EUpgradeTier Tier;

	/** Icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	class UTexture2D* Icon;

	// ====================
	// Stat Modifiers
	// ====================

	/** Stat modifiers applied by this upgrade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	TArray<FShipUpgradeStatModifier> StatModifiers;

	// ====================
	// Requirements
	// ====================

	/** Requirements to purchase/install this upgrade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
	FShipUpgradeRequirement Requirements;

	// ====================
	// Restrictions
	// ====================

	/** Ship types that can use this upgrade (empty = all ships) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Restrictions")
	TArray<FName> CompatibleShipTypes;

	/** Whether this upgrade is unique (only one can be installed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Restrictions")
	bool bIsUnique;

	/** Maximum stack count (if not unique) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Restrictions", meta=(ClampMin="1", ClampMax="10"))
	int32 MaxStackCount;

	/** Mutually exclusive upgrades (can't have these installed at same time) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Restrictions")
	TArray<UShipUpgradeDataAsset*> MutuallyExclusiveWith;

	// ====================
	// Blueprint Functions
	// ====================

	/**
	 * Get total stat bonus for a specific stat
	 * @param StatName Name of stat to query
	 * @param BaseValue Base value before modifiers
	 * @return Modified value after applying upgrade
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade")
	float CalculateStatBonus(FName StatName, float BaseValue) const;

	/**
	 * Check if upgrade is compatible with ship type
	 * @param ShipTypeID Ship type to check
	 * @return True if compatible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade")
	bool IsCompatibleWithShipType(FName ShipTypeID) const;

	/**
	 * Get upgrade tier display name
	 * @return Localized tier name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade")
	FText GetTierDisplayName() const;

	/**
	 * Get category display name
	 * @return Localized category name
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade")
	FText GetCategoryDisplayName() const;

	/**
	 * Get formatted requirements description
	 * @return Text describing all requirements
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade")
	FText GetRequirementsDescription() const;
};
