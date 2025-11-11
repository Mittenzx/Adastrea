#include "UI/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
	: MaxSlots(40)
	, MaxWeight(0.0f)
	, bAutoStack(true)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize slots
	Slots.SetNum(MaxSlots);
}

int32 UInventoryComponent::AddItem(UInventoryItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	// Check weight capacity
	if (!CanAddItemWeight(Item, Quantity))
	{
		return 0;
	}

	int32 RemainingToAdd = Quantity;
	int32 TotalAdded = 0;

	// Try to stack with existing items first if auto-stack is enabled
	if (bAutoStack && Item->MaxStackSize > 1)
	{
		for (int32 i = 0; i < Slots.Num() && RemainingToAdd > 0; i++)
		{
			if (!Slots[i].IsEmpty() && Slots[i].Item == Item)
			{
				int32 SpaceInSlot = Item->MaxStackSize - Slots[i].Quantity;
				int32 AmountToAdd = FMath::Min(RemainingToAdd, SpaceInSlot);
				
				if (AmountToAdd > 0)
				{
					Slots[i].Quantity += AmountToAdd;
					RemainingToAdd -= AmountToAdd;
					TotalAdded += AmountToAdd;
					OnItemAdded.Broadcast(Item, AmountToAdd, i);
				}
			}
		}
	}

	// Add to empty slots
	while (RemainingToAdd > 0)
	{
		int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == INDEX_NONE)
		{
			break; // No more space
		}

		int32 AmountToAdd = FMath::Min(RemainingToAdd, Item->MaxStackSize);
		Slots[EmptySlot].Item = Item;
		Slots[EmptySlot].Quantity = AmountToAdd;
		RemainingToAdd -= AmountToAdd;
		TotalAdded += AmountToAdd;
		OnItemAdded.Broadcast(Item, AmountToAdd, EmptySlot);
	}

	if (TotalAdded > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return TotalAdded;
}

int32 UInventoryComponent::RemoveItem(UInventoryItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	int32 RemainingToRemove = Quantity;
	int32 TotalRemoved = 0;

	for (int32 i = 0; i < Slots.Num() && RemainingToRemove > 0; i++)
	{
		if (!Slots[i].IsEmpty() && Slots[i].Item == Item)
		{
			int32 AmountToRemove = FMath::Min(RemainingToRemove, Slots[i].Quantity);
			Slots[i].Quantity -= AmountToRemove;
			RemainingToRemove -= AmountToRemove;
			TotalRemoved += AmountToRemove;

			if (Slots[i].Quantity <= 0)
			{
				Slots[i].Item = nullptr;
				Slots[i].Quantity = 0;
			}
		}
	}

	if (TotalRemoved > 0)
	{
		OnItemRemoved.Broadcast(Item, TotalRemoved);
		OnInventoryChanged.Broadcast();
	}

	return TotalRemoved;
}

int32 UInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Quantity)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		return 0;
	}

	int32 AmountToRemove = FMath::Min(Quantity, Slots[SlotIndex].Quantity);
	UInventoryItemDataAsset* RemovedItem = Slots[SlotIndex].Item;
	
	Slots[SlotIndex].Quantity -= AmountToRemove;
	
	if (Slots[SlotIndex].Quantity <= 0)
	{
		Slots[SlotIndex].Item = nullptr;
		Slots[SlotIndex].Quantity = 0;
	}

	if (AmountToRemove > 0)
	{
		OnItemRemoved.Broadcast(RemovedItem, AmountToRemove);
		OnInventoryChanged.Broadcast();
	}

	return AmountToRemove;
}

FInventorySlot UInventoryComponent::GetItemInSlot(int32 SlotIndex) const
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		return Slots[SlotIndex];
	}
	return FInventorySlot();
}

bool UInventoryComponent::HasItem(UInventoryItemDataAsset* Item, int32 Quantity) const
{
	return GetItemCount(Item) >= Quantity;
}

int32 UInventoryComponent::GetItemCount(UInventoryItemDataAsset* Item) const
{
	if (!Item)
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.Item == Item)
		{
			TotalCount += Slot.Quantity;
		}
	}
	return TotalCount;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float TotalWeight = 0.0f;
	for (const FInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty() && Slot.Item)
		{
			TotalWeight += Slot.Item->Weight * Slot.Quantity;
		}
	}
	return TotalWeight;
}

bool UInventoryComponent::CanAddItemWeight(UInventoryItemDataAsset* Item, int32 Quantity) const
{
	if (!Item || MaxWeight <= 0.0f)
	{
		return true; // No weight limit
	}

	float AdditionalWeight = Item->Weight * Quantity;
	return (GetCurrentWeight() + AdditionalWeight) <= MaxWeight;
}

int32 UInventoryComponent::GetFreeSlotCount() const
{
	int32 FreeSlots = 0;
	for (const FInventorySlot& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			FreeSlots++;
		}
	}
	return FreeSlots;
}

void UInventoryComponent::ClearInventory()
{
	for (FInventorySlot& Slot : Slots)
	{
		Slot.Item = nullptr;
		Slot.Quantity = 0;
	}
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::SortInventory()
{
	// Sort by category first, then by rarity
	Slots.Sort([](const FInventorySlot& A, const FInventorySlot& B)
	{
		// Empty slots go to the end
		if (A.IsEmpty() && !B.IsEmpty()) return false;
		if (!A.IsEmpty() && B.IsEmpty()) return true;
		if (A.IsEmpty() && B.IsEmpty()) return false;

		// Sort by category
		if (A.Item->Category != B.Item->Category)
		{
			return A.Item->Category < B.Item->Category;
		}

		// Then by rarity (higher rarity first)
		return A.Item->Rarity > B.Item->Rarity;
	});

	OnInventoryChanged.Broadcast();
}

TArray<FInventorySlot> UInventoryComponent::GetAllSlots() const
{
	return Slots;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		return false;
	}

	UInventoryItemDataAsset* Item = Slots[SlotIndex].Item;
	if (!Item || !Item->bCanBeUsed)
	{
		return false;
	}

	// Remove one item if it's consumable
	// Blueprint implementations can override this behavior
	RemoveItemFromSlot(SlotIndex, 1);
	
	return true;
}

int32 UInventoryComponent::FindSlotForItem(UInventoryItemDataAsset* Item, int32 Quantity)
{
	if (!Item)
	{
		return INDEX_NONE;
	}

	// First try to find a slot with the same item that has space
	if (bAutoStack && Item->MaxStackSize > 1)
	{
		for (int32 i = 0; i < Slots.Num(); i++)
		{
			if (!Slots[i].IsEmpty() && Slots[i].Item == Item)
			{
				if (Slots[i].Quantity < Item->MaxStackSize)
				{
					return i;
				}
			}
		}
	}

	// Find empty slot
	return FindEmptySlot();
}

int32 UInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}
	return INDEX_NONE;
}
