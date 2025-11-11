#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/InventoryItemDataAsset.h"
#include "InventoryComponent.generated.h"

/**
 * Structure representing a stack of items in an inventory slot
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	/** The item data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	UInventoryItemDataAsset* Item;

	/** Number of items in this stack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="0"))
	int32 Quantity;

	FInventorySlot()
		: Item(nullptr)
		, Quantity(0)
	{
	}

	FInventorySlot(UInventoryItemDataAsset* InItem, int32 InQuantity)
		: Item(InItem)
		, Quantity(InQuantity)
	{
	}

	/** Check if this slot is empty */
	bool IsEmpty() const
	{
		return Item == nullptr || Quantity <= 0;
	}

	/** Check if this slot can accept more of the given item */
	bool CanAcceptItem(UInventoryItemDataAsset* OtherItem, int32 Amount) const
	{
		if (IsEmpty())
		{
			return true;
		}
		if (Item != OtherItem)
		{
			return false;
		}
		if (Item && Item->MaxStackSize > 1)
		{
			return (Quantity + Amount) <= Item->MaxStackSize;
		}
		return false;
	}
};

/**
 * Component for managing inventory
 * 
 * This component provides inventory management for players and ships:
 * - Adding and removing items
 * - Stacking and splitting items
 * - Weight and capacity management
 * - Item usage and consumption
 * - Sorting and filtering
 * 
 * Usage:
 * - Add to PlayerController or Spaceship actor
 * - Configure slot count and weight capacity
 * - Use Blueprint functions to manipulate inventory
 * - Listen to delegates for inventory changes
 * 
 * Integration:
 * - Works with InventoryItemDataAsset for item definitions
 * - Connects to InventoryWidget for UI display
 * - Integrates with trading system for economy
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	/** Maximum number of inventory slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Settings", meta=(ClampMin="1", ClampMax="999"))
	int32 MaxSlots;

	/** Maximum weight capacity (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Settings", meta=(ClampMin="0.0"))
	float MaxWeight;

	/** Whether to auto-stack items when adding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Settings")
	bool bAutoStack;

	/**
	 * Add an item to the inventory
	 * @param Item The item to add
	 * @param Quantity How many to add
	 * @return Number of items actually added (may be less if inventory is full)
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 AddItem(UInventoryItemDataAsset* Item, int32 Quantity);

	/**
	 * Remove an item from the inventory
	 * @param Item The item to remove
	 * @param Quantity How many to remove
	 * @return Number of items actually removed
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 RemoveItem(UInventoryItemDataAsset* Item, int32 Quantity);

	/**
	 * Remove an item from a specific slot
	 * @param SlotIndex The slot index
	 * @param Quantity How many to remove
	 * @return Number of items actually removed
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 RemoveItemFromSlot(int32 SlotIndex, int32 Quantity);

	/**
	 * Get an item from a specific slot
	 * @param SlotIndex The slot index
	 * @return The inventory slot data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	FInventorySlot GetItemInSlot(int32 SlotIndex) const;

	/**
	 * Check if the inventory contains an item
	 * @param Item The item to check for
	 * @param Quantity Minimum quantity required
	 * @return True if inventory has at least the specified quantity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	bool HasItem(UInventoryItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Count total quantity of an item
	 * @param Item The item to count
	 * @return Total quantity across all slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	int32 GetItemCount(UInventoryItemDataAsset* Item) const;

	/**
	 * Get the current total weight of all items
	 * @return Total weight
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	float GetCurrentWeight() const;

	/**
	 * Check if adding an item would exceed weight capacity
	 * @param Item The item to check
	 * @param Quantity How many to add
	 * @return True if the item can be added without exceeding capacity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	bool CanAddItemWeight(UInventoryItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Get number of free slots
	 * @return Number of empty slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	int32 GetFreeSlotCount() const;

	/**
	 * Clear all items from inventory
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ClearInventory();

	/**
	 * Sort inventory by category and rarity
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SortInventory();

	/**
	 * Get all inventory slots
	 * @return Array of all slots
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
	TArray<FInventorySlot> GetAllSlots() const;

	/**
	 * Use/consume an item
	 * @param SlotIndex The slot containing the item to use
	 * @return True if the item was successfully used
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool UseItem(int32 SlotIndex);

	/** Called when an item is added to inventory */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, UInventoryItemDataAsset*, Item, int32, Quantity, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnItemAdded OnItemAdded;

	/** Called when an item is removed from inventory */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, UInventoryItemDataAsset*, Item, int32, Quantity);
	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	/** Called when inventory is modified */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

protected:
	/** All inventory slots */
	UPROPERTY()
	TArray<FInventorySlot> Slots;

	virtual void BeginPlay() override;

	/** Find the first slot that can accept the item */
	int32 FindSlotForItem(UInventoryItemDataAsset* Item, int32 Quantity);

	/** Find first empty slot */
	int32 FindEmptySlot() const;
};
