#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InventoryComponent.h"
#include "InventoryWidget.generated.h"

/**
 * Base widget class for inventory UI
 * 
 * This widget displays and manages the player's inventory interface:
 * - Grid or list view of inventory items
 * - Item details and tooltips
 * - Drag and drop for item management
 * - Sorting and filtering options
 * - Item usage and actions
 * 
 * Usage:
 * - Create a Widget Blueprint based on this class
 * - Design the inventory layout in the Blueprint designer
 * - Bind to an InventoryComponent for data
 * - Implement item slot widgets for display
 * - Override BlueprintNativeEvents for custom behavior
 * 
 * Integration:
 * - Connects to UInventoryComponent for data
 * - Displays items from InventoryItemDataAsset
 * - Integrates with trading and crafting systems
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize the inventory widget with a component
	 * @param InInventoryComponent The inventory component to display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void InitializeInventory(UInventoryComponent* InInventoryComponent);
	virtual void InitializeInventory_Implementation(UInventoryComponent* InInventoryComponent);

	/**
	 * Refresh the entire inventory display
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void RefreshInventory();
	virtual void RefreshInventory_Implementation();

	/**
	 * Update display for a specific slot
	 * @param SlotIndex The slot index to update
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void UpdateSlot(int32 SlotIndex);
	virtual void UpdateSlot_Implementation(int32 SlotIndex);

	/**
	 * Handle item selection by player
	 * @param SlotIndex The slot that was selected
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void OnSlotSelected(int32 SlotIndex);
	virtual void OnSlotSelected_Implementation(int32 SlotIndex);

	/**
	 * Handle item usage request
	 * @param SlotIndex The slot containing the item to use
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void OnUseItem(int32 SlotIndex);
	virtual void OnUseItem_Implementation(int32 SlotIndex);

	/**
	 * Handle item drop request
	 * @param SlotIndex The slot containing the item to drop
	 * @param Quantity How many to drop
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void OnDropItem(int32 SlotIndex, int32 Quantity);
	virtual void OnDropItem_Implementation(int32 SlotIndex, int32 Quantity);

	/**
	 * Sort the inventory and refresh display
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory Widget")
	void SortInventory();

	/**
	 * Filter inventory by category
	 * @param Category The category to filter by
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void FilterByCategory(EInventoryItemCategory Category);
	virtual void FilterByCategory_Implementation(EInventoryItemCategory Category);

	/**
	 * Clear any active filters
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void ClearFilters();
	virtual void ClearFilters_Implementation();

	/**
	 * Show item details/tooltip
	 * @param Item The item to show details for
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void ShowItemDetails(UInventoryItemDataAsset* Item);
	virtual void ShowItemDetails_Implementation(UInventoryItemDataAsset* Item);

	/**
	 * Hide item details/tooltip
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
	void HideItemDetails();
	virtual void HideItemDetails_Implementation();

	/**
	 * Get the currently selected slot index
	 * @return The selected slot index, or -1 if none selected
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory Widget")
	int32 GetSelectedSlotIndex() const;

	/**
	 * Get the inventory component this widget is displaying
	 * @return The inventory component
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory Widget")
	UInventoryComponent* GetInventoryComponent() const;

protected:
	/** Reference to the inventory component being displayed */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Widget")
	UInventoryComponent* InventoryComponent;

	/** Currently selected slot index */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Widget")
	int32 SelectedSlotIndex;

	/** Active category filter */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Widget")
	EInventoryItemCategory ActiveCategoryFilter;

	/** Whether a category filter is active */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Widget")
	bool bIsFiltered;

	/** Called when the inventory component's contents change */
	UFUNCTION()
	void OnInventoryChanged();

	/** Native initialization */
	virtual void NativeConstruct() override;

	/** Native cleanup */
	virtual void NativeDestruct() override;
};
