#include "CargoComponent.h"
#include "TradeItemDataAsset.h"

UCargoComponent::UCargoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CargoCapacity = 10.0f;  // Default 10 units
}

bool UCargoComponent::AddCargo(UTradeItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Invalid item or quantity"));
		return false;
	}

	// Check if we have space
	if (!HasSpaceFor(Item, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Not enough space for %d x %s"), Quantity, *Item->ItemName.ToString());
		return false;
	}

	// Find existing entry or create new one
	int32 EntryIndex = FindCargoEntryIndex(Item);

	if (EntryIndex != INDEX_NONE)
	{
		// Add to existing entry
		CargoInventory[EntryIndex].Quantity += Quantity;
	}
	else
	{
		// Create new entry
		CargoInventory.Add(FCargoEntry(Item, Quantity));
	}

	UE_LOG(LogTemp, Log, TEXT("CargoComponent: Added %d x %s (total: %d, space: %.1f/%.1f)"), 
		Quantity, *Item->ItemName.ToString(), GetItemQuantity(Item), GetUsedCargoSpace(), CargoCapacity);

	// Broadcast events
	OnCargoAdded.Broadcast(Item, Quantity);
	OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());

	return true;
}

bool UCargoComponent::RemoveCargo(UTradeItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Invalid item or quantity"));
		return false;
	}

	// Check if we have the item
	if (!HasItem(Item, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Don't have %d x %s in cargo"), Quantity, *Item->ItemName.ToString());
		return false;
	}

	// Find entry
	int32 EntryIndex = FindCargoEntryIndex(Item);

	if (EntryIndex != INDEX_NONE)
	{
		CargoInventory[EntryIndex].Quantity -= Quantity;

		// Remove entry if quantity is now zero
		if (CargoInventory[EntryIndex].Quantity <= 0)
		{
			CargoInventory.RemoveAt(EntryIndex);
		}

		UE_LOG(LogTemp, Log, TEXT("CargoComponent: Removed %d x %s (remaining: %d, space: %.1f/%.1f)"), 
			Quantity, *Item->ItemName.ToString(), GetItemQuantity(Item), GetUsedCargoSpace(), CargoCapacity);

		// Broadcast events
		OnCargoRemoved.Broadcast(Item, Quantity);
		OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());

		return true;
	}

	return false;
}

void UCargoComponent::ClearCargo()
{
	CargoInventory.Empty();
	UE_LOG(LogTemp, Log, TEXT("CargoComponent: Cleared all cargo"));
	OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
}

float UCargoComponent::GetAvailableCargoSpace() const
{
	return FMath::Max(0.0f, CargoCapacity - GetUsedCargoSpace());
}

float UCargoComponent::GetUsedCargoSpace() const
{
	float UsedSpace = 0.0f;

	for (const FCargoEntry& Entry : CargoInventory)
	{
		if (Entry.Item)
		{
			UsedSpace += Entry.Item->GetTotalVolume(Entry.Quantity);
		}
	}

	return UsedSpace;
}

float UCargoComponent::GetCargoUtilization() const
{
	if (CargoCapacity <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetUsedCargoSpace() / CargoCapacity, 0.0f, 1.0f);
}

bool UCargoComponent::HasSpaceFor(UTradeItemDataAsset* Item, int32 Quantity) const
{
	if (!Item || Quantity <= 0)
	{
		return false;
	}

	float RequiredSpace = Item->GetTotalVolume(Quantity);
	return GetAvailableCargoSpace() >= RequiredSpace;
}

bool UCargoComponent::HasItem(UTradeItemDataAsset* Item, int32 Quantity) const
{
	if (!Item)
	{
		return false;
	}

	int32 CurrentQuantity = GetItemQuantity(Item);
	return CurrentQuantity >= Quantity;
}

int32 UCargoComponent::GetItemQuantity(UTradeItemDataAsset* Item) const
{
	if (!Item)
	{
		return 0;
	}

	int32 EntryIndex = FindCargoEntryIndex(Item);
	
	if (EntryIndex != INDEX_NONE)
	{
		return CargoInventory[EntryIndex].Quantity;
	}

	return 0;
}

int32 UCargoComponent::FindCargoEntryIndex(UTradeItemDataAsset* Item) const
{
	if (!Item)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < CargoInventory.Num(); ++i)
	{
		if (CargoInventory[i].Item == Item)
		{
			return i;
		}
	}

	return INDEX_NONE;
}
