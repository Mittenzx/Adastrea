// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Trading/TradeItemDataAsset.h"
#include "CraftingTreeLoader.generated.h"

class UMarketDataAsset;

/**
 * Crafts items from the machine-readable CraftingTree.json at runtime.
 *
 * Reads Content/Data/CraftingTree.json (authored by the Crafting Tree Agent)
 * and builds a pool of transient UTradeItemDataAsset objects keyed by item ID,
 * so every material in the crafting tree becomes tradeable in all markets
 * without needing per-item editor assets.
 *
 * Usage (once at startup):
 *   UCraftingTreeLoader* Loader = NewObject<...>(...);
 *   Loader->LoadCraftingTree();
 *   Loader->PopulateMarketInventory(Market);
 */
UCLASS(BlueprintType)
class ADASTREA_API UCraftingTreeLoader : public UObject
{
	GENERATED_BODY()

public:
	/** The relative path (under Content/) to the crafting tree JSON. */
	static const TCHAR* GetCraftingTreePath();

	/**
	 * Parse CraftingTree.json and build the item pool. Returns the number of
	 * items loaded, or 0 on failure.
	 */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	int32 LoadCraftingTree();

	/**
	 * Populate the given market's inventory with a broad mixed set drawn from
	 * the loaded crafting tree (all categories/tiers). Clears existing entries.
	 */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void PopulateMarketInventory(UMarketDataAsset* Market) const;

	/** Get a transient trade item by crafting item ID (e.g. "IronOre"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	UTradeItemDataAsset* GetTradeItem(const FString& ItemID) const;

	/** Number of trade items loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	int32 GetLoadedItemCount() const { return ItemPool.Num(); }

	/** Whether the tree has been loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	bool IsLoaded() const { return bLoaded; }

private:
	/** Map of crafting item ID -> transient trade item (owned, keep alive). */
	UPROPERTY()
	TMap<FString, TObjectPtr<UTradeItemDataAsset>> ItemPool;

	/** Whether LoadCraftingTree() succeeded. */
	bool bLoaded = false;

	/** Map a crafting-tree Category string to a trade-item category. */
	static ETradeItemCategory MapTradeCategory(const FString& Category);

	/** Map a crafting-tree MaterialCategory to a legality status. */
	static ELegalityStatus MapLegality(const FString& MaterialCategory);
};