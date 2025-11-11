#include "UI/InventoryWidget.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryComponent(nullptr)
	, SelectedSlotIndex(-1)
	, ActiveCategoryFilter(EInventoryItemCategory::Weapon)
	, bIsFiltered(false)
{
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventoryWidget::NativeDestruct()
{
	// Unbind from inventory component events
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventory_Implementation(UInventoryComponent* InInventoryComponent)
{
	// Unbind from previous component if any
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
	}

	InventoryComponent = InInventoryComponent;
	SelectedSlotIndex = -1;
	bIsFiltered = false;

	// Bind to new component events
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);
		RefreshInventory_Implementation();
	}
}

void UInventoryWidget::RefreshInventory_Implementation()
{
	// Blueprint implementation handles the actual refresh
	// This default implementation just clears selection
	SelectedSlotIndex = -1;
}

void UInventoryWidget::UpdateSlot_Implementation(int32 SlotIndex)
{
	// Blueprint implementation handles slot update
}

void UInventoryWidget::OnSlotSelected_Implementation(int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;
}

void UInventoryWidget::OnUseItem_Implementation(int32 SlotIndex)
{
	if (InventoryComponent && InventoryComponent->UseItem(SlotIndex))
	{
		RefreshInventory_Implementation();
	}
}

void UInventoryWidget::OnDropItem_Implementation(int32 SlotIndex, int32 Quantity)
{
	if (InventoryComponent)
	{
		FInventorySlot Slot = InventoryComponent->GetItemInSlot(SlotIndex);
		if (!Slot.IsEmpty() && Slot.Item && Slot.Item->bCanBeDropped)
		{
			InventoryComponent->RemoveItemFromSlot(SlotIndex, Quantity);
			RefreshInventory_Implementation();
		}
	}
}

void UInventoryWidget::SortInventory()
{
	if (InventoryComponent)
	{
		InventoryComponent->SortInventory();
		RefreshInventory_Implementation();
	}
}

void UInventoryWidget::FilterByCategory_Implementation(EInventoryItemCategory Category)
{
	ActiveCategoryFilter = Category;
	bIsFiltered = true;
	RefreshInventory_Implementation();
}

void UInventoryWidget::ClearFilters_Implementation()
{
	bIsFiltered = false;
	RefreshInventory_Implementation();
}

void UInventoryWidget::ShowItemDetails_Implementation(UInventoryItemDataAsset* Item)
{
	// Blueprint implementation handles showing item details
}

void UInventoryWidget::HideItemDetails_Implementation()
{
	// Blueprint implementation handles hiding item details
}

int32 UInventoryWidget::GetSelectedSlotIndex() const
{
	return SelectedSlotIndex;
}

UInventoryComponent* UInventoryWidget::GetInventoryComponent() const
{
	return InventoryComponent;
}

void UInventoryWidget::OnInventoryChanged()
{
	RefreshInventory_Implementation();
}
