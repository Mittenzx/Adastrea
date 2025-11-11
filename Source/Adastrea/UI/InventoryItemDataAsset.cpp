#include "UI/InventoryItemDataAsset.h"

FLinearColor UInventoryItemDataAsset::GetRarityColor() const
{
	switch (Rarity)
	{
		case EInventoryItemRarity::Common:
			return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
		case EInventoryItemRarity::Uncommon:
			return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // Green
		case EInventoryItemRarity::Rare:
			return FLinearColor(0.2f, 0.4f, 1.0f, 1.0f); // Blue
		case EInventoryItemRarity::Epic:
			return FLinearColor(0.7f, 0.2f, 0.9f, 1.0f); // Purple
		case EInventoryItemRarity::Legendary:
			return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
		case EInventoryItemRarity::Unique:
			return FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
		default:
			return FLinearColor::White;
	}
}

FString UInventoryItemDataAsset::GetCustomProperty(FName PropertyName, const FString& DefaultValue) const
{
	const FString* FoundValue = CustomProperties.Find(PropertyName);
	return FoundValue ? *FoundValue : DefaultValue;
}

bool UInventoryItemDataAsset::CanStackWith(const UInventoryItemDataAsset* OtherItem) const
{
	// Items can stack if they are the exact same data asset and max stack > 1
	return OtherItem == this && MaxStackSize > 1;
}
