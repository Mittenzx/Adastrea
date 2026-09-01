// Copyright Mittenzx. All Rights Reserved.

#include "Trading/CraftingTreeLoader.h"
#include "Trading/MarketDataAsset.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "UObject/ConstructorHelpers.h"

const TCHAR* UCraftingTreeLoader::GetCraftingTreePath()
{
	// Content/Data/CraftingTree.json
	return TEXT("Data/CraftingTree.json");
}

ETradeItemCategory UCraftingTreeLoader::MapTradeCategory(const FString& Category)
{
	// Map crafting-tree recipe Category strings to the trade-item enum.
	if (Category == TEXT("RawMaterials")) return ETradeItemCategory::RawMaterials;
	if (Category == TEXT("RefinedGoods"))  return ETradeItemCategory::RefinedGoods;
	if (Category == TEXT("Components"))    return ETradeItemCategory::Components;
	if (Category == TEXT("Technology"))    return ETradeItemCategory::Technology;
	if (Category == TEXT("Food"))          return ETradeItemCategory::Food;
	if (Category == TEXT("Luxury"))        return ETradeItemCategory::Luxury;
	if (Category == TEXT("Contraband"))    return ETradeItemCategory::Contraband;
	if (Category == TEXT("Military"))      return ETradeItemCategory::Military;
	if (Category == TEXT("Medical"))       return ETradeItemCategory::Medical;
	if (Category == TEXT("Data"))          return ETradeItemCategory::Data;
	// "Other" and unknowns default to RefinedGoods (generic tradeable).
	return ETradeItemCategory::RefinedGoods;
}

ELegalityStatus UCraftingTreeLoader::MapLegality(const FString& MaterialCategory)
{
	// Contraband-category materials are illegal; everything else legal in MVP.
	if (MaterialCategory == TEXT("Contraband")) return ELegalityStatus::Contraband;
	return ELegalityStatus::Legal;
}

int32 UCraftingTreeLoader::LoadCraftingTree()
{
	ItemPool.Empty();
	bLoaded = false;

	// Resolve the JSON file in the project's Content dir.
	const FString FullPath = FPaths::ProjectContentDir() + GetCraftingTreePath();
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: could not read %s"), *FullPath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: failed to parse static data JSON"));
		return 0;
	}

	// The Items map holds per-item metadata: {ItemName, Description, WeightKg, VolumeM3, StorageType, Rarity, BaseValue, MaterialCategory}.
	const TSharedPtr<FJsonObject>* ItemsObj = nullptr;
	if (Root->TryGetObjectField(TEXT("Items"), ItemsObj) && ItemsObj->IsValid())
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*ItemsObj)->Values)
		{
			const FString& ItemID = Pair.Key;
			const TSharedPtr<FJsonObject>& ItemObj = Pair.Value->AsObject();
			if (!ItemObj.IsValid())
			{
				continue;
			}

			// Build a transient trade item.
			UTradeItemDataAsset* Item = NewObject<UTradeItemDataAsset>(this);
			if (!Item)
			{
				continue;
			}

			Item->ItemID = FName(*ItemID);

			FString ItemName = ItemID;
			ItemObj->TryGetStringField(TEXT("ItemName"), ItemName);
			Item->ItemName = FText::FromString(ItemName);

			FString Desc = TEXT("");
			ItemObj->TryGetStringField(TEXT("Description"), Desc);
			Item->Description = FText::FromString(Desc);

			// Base value (credits). Default scales with rarity if missing.
			double BaseValue = 30.0;
			ItemObj->TryGetNumberField(TEXT("BaseValue"), BaseValue);
			Item->BasePrice = (float)BaseValue;

			// Volume / mass.
			double Vol = 0.1, Mass = 1.0;
			ItemObj->TryGetNumberField(TEXT("VolumeM3"), Vol);
			ItemObj->TryGetNumberField(TEXT("WeightKg"), Mass);
			Item->VolumePerUnit = (float)Vol;
			Item->MassPerUnit = (float)Mass;
			Item->StandardLotSize = 10;

			// Legality from material category (e.g. Contraband).
			FString MatCat = TEXT("");
			ItemObj->TryGetStringField(TEXT("MaterialCategory"), MatCat);
			Item->LegalityStatus = MapLegality(MatCat);

			// Broad availability for any market.
			Item->bAffectedBySupplyDemand = true;
			Item->bAffectedByMarketEvents = true;

			// Derive trade category from material category where possible, else the item name.
			FString TradeCat = TEXT("RefinedGoods");
			if (ItemObj->TryGetStringField(TEXT("Category"), TradeCat))
			{
				// no-op: some items carry Category, fall through to mapping below
			}
			Item->Category = MapTradeCategory(TradeCat);

			ItemPool.Add(ItemID, Item);
		}
	}

	bLoaded = ItemPool.Num() > 0;
	UE_LOG(LogTemp, Log, TEXT("CraftingTreeLoader: loaded %d trade items from crafting tree"), ItemPool.Num());
	return ItemPool.Num();
}

UTradeItemDataAsset* UCraftingTreeLoader::GetTradeItem(const FString& ItemID) const
{
	const TObjectPtr<UTradeItemDataAsset>* Found = ItemPool.Find(ItemID);
	return Found ? Found->Get() : nullptr;
}

void UCraftingTreeLoader::PopulateMarketInventory(UMarketDataAsset* Market) const
{
	if (!Market)
	{
		return;
	}
	// Broad mix: put every loaded item into the market at reference stock levels.
	Market->Inventory.Empty();
	int32 Index = 0;
	for (const TPair<FString, TObjectPtr<UTradeItemDataAsset>>& Pair : ItemPool)
	{
		FMarketInventoryEntry Entry;
		Entry.TradeItem = Pair.Value.Get();
		Entry.MaxStock = 100000;                    // generous MVP stock
		Entry.CurrentStock = 10000 + (Index * 251) % 50000; // deterministic variety
		Entry.SupplyLevel = 1.0f;
		Entry.DemandLevel = 1.0f;
		Entry.LastTradePrice = Entry.TradeItem ? Entry.TradeItem->BasePrice : 0.0f;
		Entry.bInStock = true;
		Market->Inventory.Add(Entry);
		++Index;
	}
	UE_LOG(LogTemp, Log, TEXT("CraftingTreeLoader: populated market '%s' with %d items"),
		*Market->GetName(), Market->Inventory.Num());
}