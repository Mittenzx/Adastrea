// Copyright Mittenzx. All Rights Reserved.

#include "Trading/CraftingTreeLoader.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/CargoComponent.h"
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

int32 UCraftingTreeLoader::LoadRecipes()
{
	Recipes.Empty();

	// Re-use the same JSON file; LoadCraftingTree() may or may not have run first.
	const FString FullPath = FPaths::ProjectContentDir() + GetCraftingTreePath();
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: could not read %s for recipes"), *FullPath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: failed to parse JSON for recipes"));
		return 0;
	}

	const TArray<TSharedPtr<FJsonValue>>* RecipesArr = nullptr;
	if (!Root->TryGetArrayField(TEXT("Recipes"), RecipesArr))
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: no 'Recipes' array in crafting tree"));
		return 0;
	}

	for (const TSharedPtr<FJsonValue>& Val : *RecipesArr)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid())
		{
			continue;
		}

		FCraftingRecipe Recipe;
		Obj->TryGetStringField(TEXT("RecipeID"), Recipe.RecipeID);
		Obj->TryGetStringField(TEXT("ProducedIn"), Recipe.ProducedIn);
		FString OutputItemStr;
		Obj->TryGetStringField(TEXT("OutputItem"), OutputItemStr);
		Recipe.OutputItem = FName(*OutputItemStr);
		Obj->TryGetNumberField(TEXT("OutputQty"), Recipe.OutputQuantity);
		Obj->TryGetNumberField(TEXT("Tier"), Recipe.Tier);

		const TArray<TSharedPtr<FJsonValue>>* IngArr = nullptr;
		if (Obj->TryGetArrayField(TEXT("Ingredients"), IngArr))
		{
			for (const TSharedPtr<FJsonValue>& IngVal : *IngArr)
			{
				const TSharedPtr<FJsonObject>& IngObj = IngVal->AsObject();
				if (!IngObj.IsValid())
				{
					continue;
				}
				FString ItemIDStr;
				int32 Qty = 0;
				IngObj->TryGetStringField(TEXT("ItemID"), ItemIDStr);
				IngObj->TryGetNumberField(TEXT("Qty"), Qty);
				Recipe.Ingredients.Add(FCraftIngredient(FName(*ItemIDStr), Qty));
			}
		}

		Recipes.Add(Recipe);
	}

	UE_LOG(LogTemp, Log, TEXT("CraftingTreeLoader: loaded %d recipes from crafting tree"), Recipes.Num());
	return Recipes.Num();
}

TArray<FCraftingRecipe> UCraftingTreeLoader::GetRecipesForFacility(const FString& ProducedIn) const
{
	TArray<FCraftingRecipe> Out;
	for (const FCraftingRecipe& Recipe : Recipes)
	{
		if (Recipe.ProducedIn == ProducedIn)
		{
			Out.Add(Recipe);
		}
	}
	return Out;
}

bool UCraftingTreeLoader::FindRecipe(FName OutputItem, FCraftingRecipe& OutRecipe) const
{
	for (const FCraftingRecipe& Recipe : Recipes)
	{
		if (Recipe.OutputItem == OutputItem)
		{
			OutRecipe = Recipe;
			return true;
		}
	}
	return false;
}

bool UCraftingTreeLoader::CanCraftRecipe(const FCraftingRecipe& Recipe, UCargoComponent* Cargo)
{
	if (!Cargo)
	{
		return false;
	}
	for (const FCraftIngredient& Ing : Recipe.Ingredients)
	{
		if (Ing.Quantity > 0 && Cargo->GetItemQuantityByID(Ing.ItemID) < Ing.Quantity)
		{
			return false;
		}
	}
	return true;
}

bool UCraftingTreeLoader::CraftRecipe(const FCraftingRecipe& Recipe, UCargoComponent* Cargo)
{
	// Pre-validate ingredients are present before consuming anything.
	if (!CanCraftRecipe(Recipe, Cargo))
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: cannot craft %s - ingredients missing"),
			*Recipe.OutputItem.ToString());
		return false;
	}

	// Consume ingredients.
	for (const FCraftIngredient& Ing : Recipe.Ingredients)
	{
		if (Ing.Quantity > 0)
		{
			Cargo->RemoveCargoByID(Ing.ItemID, Ing.Quantity);
		}
	}

	// Add the output item(s). The output item must exist in the item pool to be
	// placeable in cargo; fall back to a fresh item if the pool lacks it.
	UTradeItemDataAsset* OutputItem = GetTradeItem(Recipe.OutputItem.ToString());
	if (!OutputItem)
	{
		OutputItem = NewObject<UTradeItemDataAsset>(this);
		if (OutputItem)
		{
			OutputItem->ItemID = Recipe.OutputItem;
			OutputItem->ItemName = FText::FromName(Recipe.OutputItem);
		}
	}
	if (!OutputItem)
	{
		return false;
	}
	const int32 Qty = FMath::Max(1, Recipe.OutputQuantity);
	if (!Cargo->AddCargo(OutputItem, Qty))
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingTreeLoader: crafted %s but cargo has no space for output"),
			*Recipe.OutputItem.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("CraftingTreeLoader: crafted %d x %s in %s"),
		Qty, *Recipe.OutputItem.ToString(), *Recipe.ProducedIn);
	return true;
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