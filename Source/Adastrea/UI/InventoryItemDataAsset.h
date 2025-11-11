#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryItemDataAsset.generated.h"

/**
 * Item category for inventory organization
 */
UENUM(BlueprintType)
enum class EInventoryItemCategory : uint8
{
	Weapon UMETA(DisplayName="Weapon"),
	ShipModule UMETA(DisplayName="Ship Module"),
	Consumable UMETA(DisplayName="Consumable"),
	Resource UMETA(DisplayName="Resource"),
	TradeGood UMETA(DisplayName="Trade Good"),
	Quest UMETA(DisplayName="Quest Item"),
	Equipment UMETA(DisplayName="Equipment"),
	Ammunition UMETA(DisplayName="Ammunition"),
	Misc UMETA(DisplayName="Miscellaneous")
};

/**
 * Item rarity for display and filtering
 */
UENUM(BlueprintType)
enum class EInventoryItemRarity : uint8
{
	Common UMETA(DisplayName="Common"),
	Uncommon UMETA(DisplayName="Uncommon"),
	Rare UMETA(DisplayName="Rare"),
	Epic UMETA(DisplayName="Epic"),
	Legendary UMETA(DisplayName="Legendary"),
	Unique UMETA(DisplayName="Unique")
};

/**
 * Data Asset defining an inventory item
 * 
 * This data asset configures an item that can be stored in player or ship inventories:
 * - Basic information (name, description, icon)
 * - Category and rarity classification
 * - Stack size and weight
 * - Value for trading
 * - Gameplay effects and usage
 * 
 * Usage:
 * - Create a Data Asset based on this class in the Content Browser
 * - Configure all item properties
 * - Reference this asset in inventory systems and trading
 * - Use in Blueprint logic for item effects
 * 
 * Examples:
 * - Weapons for ship combat
 * - Ship modules for customization
 * - Trade goods for economy
 * - Quest items for missions
 * - Resources for crafting
 */
UCLASS(BlueprintType)
class ADASTREA_API UInventoryItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText ItemName;

	/** Detailed description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Icon texture for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	UTexture2D* Icon;

	/** Item category for organization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Classification")
	EInventoryItemCategory Category;

	/** Item rarity level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Classification")
	EInventoryItemRarity Rarity;

	/** Maximum stack size (1 = no stacking) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Properties", meta=(ClampMin="1", ClampMax="9999"))
	int32 MaxStackSize;

	/** Weight per unit for cargo capacity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Properties", meta=(ClampMin="0.0"))
	float Weight;

	/** Base value in credits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trading", meta=(ClampMin="0"))
	int32 BaseValue;

	/** Whether this item can be sold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trading")
	bool bCanBeSold;

	/** Whether this item can be dropped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Properties")
	bool bCanBeDropped;

	/** Whether this is a quest item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Properties")
	bool bIsQuestItem;

	/** Whether this item can be used/consumed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Usage")
	bool bCanBeUsed;

	/** Cooldown time after use in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Usage", meta=(ClampMin="0.0", EditCondition="bCanBeUsed", EditConditionHides))
	float UsageCooldown;

	/** Custom data for item-specific behavior */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Advanced")
	TMap<FName, FString> CustomProperties;

	/**
	 * Get the item's display color based on rarity
	 * @return Color for UI display
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory Item")
	FLinearColor GetRarityColor() const;

	/**
	 * Get a custom property value by name
	 * @param PropertyName Name of the custom property
	 * @param DefaultValue Default value if property doesn't exist
	 * @return The property value or default
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory Item")
	FString GetCustomProperty(FName PropertyName, const FString& DefaultValue) const;

	/**
	 * Check if the item can stack with another
	 * @param OtherItem The item to check stacking with
	 * @return True if items can stack together
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory Item")
	bool CanStackWith(const UInventoryItemDataAsset* OtherItem) const;
};
