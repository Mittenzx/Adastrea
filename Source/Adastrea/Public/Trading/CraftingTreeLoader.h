// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Trading/TradeItemDataAsset.h"
#include "CraftingTreeLoader.generated.h"

class UMarketDataAsset;
class UCargoComponent;

/** A single ingredient required to craft an item. */
USTRUCT(BlueprintType)
struct FCraftIngredient
{
	GENERATED_BODY()

	/** Crafting-tree ItemID (matches Items map / trade item ItemID). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName ItemID;

	/** How many units are required. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe", meta=(ClampMin=0))
	int32 Quantity = 0;

	FCraftIngredient() {}
	FCraftIngredient(FName InItemID, int32 InQuantity) : ItemID(InItemID), Quantity(InQuantity) {}
};

/** A crafted recipe: consumes Ingredients to produce OutputItem. */
USTRUCT(BlueprintType)
struct FCraftingRecipe
{
	GENERATED_BODY()

	/** Recipe ID (e.g. "RCP_038_SteelAlloy"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FString RecipeID;

	/** What this recipe produces. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName OutputItem;

	/** Number of output units per craft. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe", meta=(ClampMin=1))
	int32 OutputQuantity = 1;

	/** Crafting tier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Tier = 1;

	/** Where this is produced (Fabrication / Processing / a lab ...). Must match
	 *  the crafting module's ModuleType / ProducedIn tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FString ProducedIn;

	/** Ingredient requirements. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	TArray<FCraftIngredient> Ingredients;

	/** Convenience: look up the ingredient quantity for an ItemID (0 if absent). */
	int32 GetIngredientQuantity(FName ItemID) const
	{
		for (const FCraftIngredient& Ing : Ingredients)
		{
			if (Ing.ItemID == ItemID)
			{
				return Ing.Quantity;
			}
		}
		return 0;
	}
};

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

	/**
	 * Load the craft Recipes from Content/Data/CraftingTree.json. Call after
	 * LoadCraftingTree(); populates the recipe pool alongside the item pool.
	 * Returns the number of recipes loaded, or 0 on failure.
	 */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	int32 LoadRecipes();

	/**
	 * Get all recipes that are produced in the given facility (ProducedIn tag).
	 * @param ProducedIn e.g. "Fabrication", "Processing", "Reactor", "ScienceLab"
	 * @return Matching recipes (empty if none / not loaded)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	TArray<FCraftingRecipe> GetRecipesForFacility(const FString& ProducedIn) const;

	/** Find a recipe by its output ItemID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	bool FindRecipe(FName OutputItem, FCraftingRecipe& OutRecipe) const;

	/**
	 * Check whether crafting the given recipe is possible: the player cargo holds
	 * every ingredient at the required quantity.
	 * @param Recipe The recipe to check
	 * @param Cargo The cargo hold (player's ship)
	 * @return True if all ingredients are present
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	static bool CanCraftRecipe(const FCraftingRecipe& Recipe, UCargoComponent* Cargo);

	/**
	 * Attempt to craft the given recipe: consumes its ingredients from the cargo
	 * and adds the output item. Returns true if crafted (ingredients consumed and
	 * output added).
	 * @param Recipe The recipe to craft
	 * @param Cargo The cargo hold (player's ship)
	 * @return True on success
	 */
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool CraftRecipe(const FCraftingRecipe& Recipe, UCargoComponent* Cargo);

	/** Number of recipes loaded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Crafting")
	int32 GetLoadedRecipeCount() const { return Recipes.Num(); }

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

	/** All craft recipes loaded from the crafting tree (index = recipe order). */
	UPROPERTY()
	TArray<FCraftingRecipe> Recipes;

	/** Whether LoadCraftingTree() succeeded. */
	bool bLoaded = false;

	/** Map a crafting-tree Category string to a trade-item category. */
	static ETradeItemCategory MapTradeCategory(const FString& Category);

	/** Map a crafting-tree MaterialCategory to a legality status. */
	static ELegalityStatus MapLegality(const FString& MaterialCategory);
};