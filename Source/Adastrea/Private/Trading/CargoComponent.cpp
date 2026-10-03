#include "Trading/CargoComponent.h"
#include "Trading/TradeItemDataAsset.h"

namespace
{
	// Hand-authored and crafting-tree items describe the same goods under different IDs, so
	// cargo is interchangeable between them (mined ore sells at stations and feeds recipes).
	bool IdsMatch(FName A, FName B)
	{
		return UTradeItemDataAsset::ItemIdsMatch(A, B);
	}
}

UCargoComponent::UCargoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CargoCapacity = 10.0f;  // Default 10 units
}

bool UCargoComponent::AddCargo(UTradeItemDataAsset* Item, int32 Quantity)
{
	return AddStolenCargo(Item, Quantity, NAME_None);
}

bool UCargoComponent::AddStolenCargo(UTradeItemDataAsset* Item, int32 Quantity, FName Owner)
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

	// Find existing entry (same goods, same stolen tag) or create new one
	const int32 EntryIndex = FindCargoEntryIndex(Item, Owner);
	if (EntryIndex != INDEX_NONE)
	{
		CargoInventory[EntryIndex].Quantity += Quantity;
	}
	else
	{
		CargoInventory.Add(FCargoEntry(Item, Quantity, Owner));
	}

	UE_LOG(LogTemp, Log, TEXT("CargoComponent: Added %d x %s%s (total: %d, available space: %.1f)"),
		Quantity, *Item->ItemName.ToString(), Owner.IsNone() ? TEXT("") : *FString::Printf(TEXT(" (stolen from %s)"), *Owner.ToString()),
		GetItemQuantity(Item), GetAvailableCargoSpace());

	// Broadcast events
	OnCargoAdded.Broadcast(Item, Quantity);
	OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());

	return true;
}

bool UCargoComponent::AddEntry(const FCargoEntry& Entry)
{
	return AddStolenCargo(Entry.Item, Entry.Quantity, Entry.StolenFrom);
}

TArray<FCargoEntry> UCargoComponent::TakeLoad(float MaxVolume)
{
	TArray<FCargoEntry> Taken;
	float Room = MaxVolume;
	for (int32 i = 0; i < CargoInventory.Num() && Room > KINDA_SMALL_NUMBER; ++i)
	{
		FCargoEntry& Entry = CargoInventory[i];
		if (!Entry.Item || Entry.Quantity <= 0)
		{
			continue;
		}
		const float Volume = Entry.Item->VolumePerUnit;
		const int32 Units = Volume > KINDA_SMALL_NUMBER ? FMath::Min(Entry.Quantity, FMath::FloorToInt(Room / Volume + KINDA_SMALL_NUMBER)) : Entry.Quantity;
		if (Units <= 0)
		{
			continue;
		}
		Taken.Add(FCargoEntry(Entry.Item, Units, Entry.StolenFrom));
		Room -= Volume * Units;
		Entry.Quantity -= Units;
		OnCargoRemoved.Broadcast(Entry.Item, Units);
		if (Entry.Quantity <= 0)
		{
			CargoInventory.RemoveAt(i--);
		}
	}
	if (!Taken.IsEmpty())
	{
		OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
	}
	return Taken;
}

TArray<FCargoEntry> UCargoComponent::RemoveAllStolen()
{
	TArray<FCargoEntry> Removed;
	for (int32 i = CargoInventory.Num() - 1; i >= 0; --i)
	{
		if (CargoInventory[i].IsStolen())
		{
			Removed.Insert(CargoInventory[i], 0);
			OnCargoRemoved.Broadcast(CargoInventory[i].Item, CargoInventory[i].Quantity);
			CargoInventory.RemoveAt(i);
		}
	}
	if (!Removed.IsEmpty())
	{
		OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
	}
	return Removed;
}

int32 UCargoComponent::RemoveStolenCargo(UTradeItemDataAsset* Item, int32 Quantity)
{
	int32 Removed = 0;
	for (int32 i = 0; Item && i < CargoInventory.Num() && Removed < Quantity; ++i)
	{
		FCargoEntry& Entry = CargoInventory[i];
		if (Entry.IsStolen() && Entry.Item && (Entry.Item == Item || IdsMatch(Entry.Item->ItemID, Item->ItemID)))
		{
			const int32 Take = FMath::Min(Entry.Quantity, Quantity - Removed);
			Entry.Quantity -= Take;
			Removed += Take;
			if (Entry.Quantity <= 0)
			{
				CargoInventory.RemoveAt(i--);
			}
		}
	}
	if (Removed > 0)
	{
		OnCargoRemoved.Broadcast(Item, Removed);
		OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
	}
	return Removed;
}

bool UCargoComponent::RemoveCargo(UTradeItemDataAsset* Item, int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Invalid item or quantity"));
		return false;
	}

	// Check if we have the item
	const int32 CurrentQuantity = GetItemQuantity(Item);
	if (CurrentQuantity < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Don't have %d x %s in cargo (have %d)"), Quantity, *Item->ItemName.ToString(), CurrentQuantity);
		return false;
	}

	// Clean units first, then stolen ones (selling decides for itself which it may sell).
	int32 Remaining = Quantity;
	for (const bool bStolenPass : { false, true })
	{
		for (int32 i = 0; i < CargoInventory.Num() && Remaining > 0; ++i)
		{
			FCargoEntry& Entry = CargoInventory[i];
			if (Entry.IsStolen() != bStolenPass || !Entry.Item || (Entry.Item != Item && !IdsMatch(Entry.Item->ItemID, Item->ItemID)))
			{
				continue;
			}
			const int32 Take = FMath::Min(Entry.Quantity, Remaining);
			Entry.Quantity -= Take;
			Remaining -= Take;
			if (Entry.Quantity <= 0)
			{
				CargoInventory.RemoveAt(i--);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("CargoComponent: Removed %d x %s (remaining: %d, available space: %.1f)"),
		Quantity, *Item->ItemName.ToString(), GetItemQuantity(Item), GetAvailableCargoSpace());

	// Broadcast events
	OnCargoRemoved.Broadcast(Item, Quantity);
	OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());

	return true;
}

void UCargoComponent::ClearCargo()
{
	CargoInventory.Empty();
	UE_LOG(LogTemp, Log, TEXT("CargoComponent: Cleared all cargo"));
	OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
}

float UCargoComponent::GetAvailableCargoSpace() const
{
	float UsedSpace = 0.0f;

	for (const FCargoEntry& Entry : CargoInventory)
	{
		if (Entry.Item)
		{
			UsedSpace += Entry.Item->GetTotalVolume(Entry.Quantity);
		}
	}

	return FMath::Max(0.0f, CargoCapacity - UsedSpace);
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

int32 UCargoComponent::GetItemQuantity(UTradeItemDataAsset* Item) const
{
	return CountItem(Item, true, true);
}

int32 UCargoComponent::GetCleanQuantity(UTradeItemDataAsset* Item) const
{
	return CountItem(Item, true, false);
}

int32 UCargoComponent::GetStolenQuantity(UTradeItemDataAsset* Item) const
{
	return CountItem(Item, false, true);
}

int32 UCargoComponent::GetTotalStolenUnits() const
{
	int32 Total = 0;
	for (const FCargoEntry& Entry : CargoInventory)
	{
		Total += Entry.IsStolen() ? Entry.Quantity : 0;
	}
	return Total;
}

int32 UCargoComponent::GetTotalUnits() const
{
	int32 Total = 0;
	for (const FCargoEntry& Entry : CargoInventory)
	{
		Total += Entry.Quantity;
	}
	return Total;
}

int32 UCargoComponent::CountItem(const UTradeItemDataAsset* Item, bool bClean, bool bStolen) const
{
	int32 Total = 0;
	for (const FCargoEntry& Entry : CargoInventory)
	{
		if (Item && Entry.Item && (Entry.IsStolen() ? bStolen : bClean)
			&& (Entry.Item == Item || IdsMatch(Entry.Item->ItemID, Item->ItemID)))
		{
			Total += Entry.Quantity;
		}
	}
	return Total;
}

int32 UCargoComponent::GetItemQuantityByID(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		return 0;
	}

	int32 Total = 0;
	for (const FCargoEntry& Entry : CargoInventory)
	{
		if (Entry.Item && IdsMatch(Entry.Item->ItemID, ItemID))
		{
			Total += Entry.Quantity;
		}
	}

	return Total;
}

bool UCargoComponent::RemoveCargoByID(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	// Check we hold enough first.
	if (GetItemQuantityByID(ItemID) < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: don't hold %d x %s in cargo (have %d)"),
			Quantity, *ItemID.ToString(), GetItemQuantityByID(ItemID));
		return false;
	}

	// Apply across all entries carrying this ID (front-first).
	int32 Remaining = Quantity;
	for (int32 i = 0; i < CargoInventory.Num() && Remaining > 0; ++i)
	{
		FCargoEntry& Entry = CargoInventory[i];
		if (Entry.Item && IdsMatch(Entry.Item->ItemID, ItemID))
		{
			int32 Take = FMath::Min(Entry.Quantity, Remaining);
			Entry.Quantity -= Take;
			Remaining -= Take;
			if (Entry.Quantity <= 0)
			{
				CargoInventory.RemoveAt(i);
				--i;
			}
		}
	}

	if (Remaining == 0)
	{
		OnCargoRemoved.Broadcast(nullptr, Quantity);
		OnCargoSpaceChanged.Broadcast(GetAvailableCargoSpace());
		return true;
	}

	return false;
}

int32 UCargoComponent::FindCargoEntryIndex(UTradeItemDataAsset* Item, FName StolenFrom) const
{
	if (!Item)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < CargoInventory.Num(); ++i)
	{
		if (CargoInventory[i].Item == Item && CargoInventory[i].StolenFrom == StolenFrom)
		{
			return i;
		}
	}
	for (int32 i = 0; i < CargoInventory.Num(); ++i)
	{
		if (CargoInventory[i].Item && CargoInventory[i].StolenFrom == StolenFrom && IdsMatch(CargoInventory[i].Item->ItemID, Item->ItemID))
		{
			return i;
		}
	}

	return INDEX_NONE;
}
