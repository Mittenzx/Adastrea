#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CargoComponent.generated.h"

// Forward declarations
class UTradeItemDataAsset;

/**
 * Cargo hold entry
 * Tracks a specific item and quantity in cargo
 */
USTRUCT(BlueprintType)
struct FCargoEntry
{
	GENERATED_BODY()

	// The trade item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cargo")
	TObjectPtr<UTradeItemDataAsset> Item;

	// Quantity in cargo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cargo")
	int32 Quantity;

	/**
	 * Organisation the goods were taken from by force (stripped from its wreck), or None
	 * for clean cargo. Lawful stations confiscate stolen goods on docking; fences buy them
	 * cheap. Stolen and clean units of one item are kept as separate entries.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cargo")
	FName StolenFrom;

	bool IsStolen() const { return !StolenFrom.IsNone(); }

	FCargoEntry()
		: Item(nullptr)
		, Quantity(0)
	{}

	FCargoEntry(UTradeItemDataAsset* InItem, int32 InQuantity, FName InStolenFrom = NAME_None)
		: Item(InItem)
		, Quantity(InQuantity)
		, StolenFrom(InStolenFrom)
	{}
};

/**
 * Cargo Component
 * Manages ship's cargo hold and inventory
 *
 * Usage:
 * 1. Add to ship actor
 * 2. Set CargoCapacity
 * 3. Use AddCargo/RemoveCargo to manage items
 * 4. Query with GetCargoSpace, HasItem, etc.
 */
UCLASS(ClassGroup=(Trading), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UCargoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// ====================
	// CARGO CONFIGURATION
	// ====================

	// Maximum cargo capacity (volume units)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cargo", meta=(ClampMin="1"))
	float CargoCapacity;

	// Current cargo inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cargo")
	TArray<FCargoEntry> CargoInventory;

	// ====================
	// CONSTRUCTOR
	// ====================

	UCargoComponent();

	// ====================
	// CARGO OPERATIONS
	// ====================

	/**
	 * Add cargo to hold
	 * @param Item The item to add
	 * @param Quantity Number of units to add
	 * @return True if successfully added
	 */
	UFUNCTION(BlueprintCallable, Category="Cargo|Operations")
	bool AddCargo(UTradeItemDataAsset* Item, int32 Quantity);

	/**
	 * Remove cargo from hold
	 * @param Item The item to remove
	 * @param Quantity Number of units to remove
	 * @return True if successfully removed
	 */
	UFUNCTION(BlueprintCallable, Category="Cargo|Operations")
	bool RemoveCargo(UTradeItemDataAsset* Item, int32 Quantity);

	/**
	 * Add goods taken by force from Owner's ship (tagged stolen). Owner None adds clean cargo.
	 */
	UFUNCTION(BlueprintCallable, Category="Cargo|Operations")
	bool AddStolenCargo(UTradeItemDataAsset* Item, int32 Quantity, FName Owner);

	/** Add an entry as it is (clean or stolen). Used when goods move between holds. */
	bool AddEntry(const FCargoEntry& Entry);

	/**
	 * Take up to MaxVolume worth of goods out of the hold, front first, keeping their
	 * stolen tags. Returns what was taken (whole units only).
	 */
	TArray<FCargoEntry> TakeLoad(float MaxVolume);

	/** Remove every stolen entry and return them (customs confiscation). */
	TArray<FCargoEntry> RemoveAllStolen();

	/** Remove up to Quantity stolen units of Item. Returns units removed. */
	int32 RemoveStolenCargo(UTradeItemDataAsset* Item, int32 Quantity);

	/**
	 * Clear all cargo
	 */
	UFUNCTION(BlueprintCallable, Category="Cargo|Operations")
	void ClearCargo();

	// ====================
	// CARGO QUERIES
	// ====================

	/**
	 * Get available cargo space
	 * @return Available volume in cargo hold
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	float GetAvailableCargoSpace() const;

	/**
	 * Check if cargo has space for item
	 * @param Item The item to check
	 * @param Quantity Number of units
	 * @return True if space is available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	bool HasSpaceFor(UTradeItemDataAsset* Item, int32 Quantity) const;

	/**
		 * Get quantity of specific item in cargo
		 * @param Item The item to check
		 * @return Quantity in cargo (0 if not found)
		 */
		UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
		int32 GetItemQuantity(UTradeItemDataAsset* Item) const;

		/**
		 * Get quantity of a cargo item by its ItemID (FName). Matches the catalog /
		 * crafting-tree materials keys so the station builder can check parts held.
		 * @param ItemID The inventory/crafting ID of the item
		 * @return Quantity in cargo (0 if nothing with that ID)
		 */
		UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
		int32 GetItemQuantityByID(FName ItemID) const;

		/**
		 * Remove cargo by ItemID. Returns true only if at least Quantity of an item
		 * with that ID was held and removed.
		 * @param ItemID The inventory/crafting ID of the item
		 * @param Quantity Number of units to remove
		 * @return True if removed, false if not enough held or not found
		 */
		UFUNCTION(BlueprintCallable, Category="Cargo|Operations")
		bool RemoveCargoByID(FName ItemID, int32 Quantity);

	/** Clean (not stolen) units of Item. GetItemQuantity counts both. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetCleanQuantity(UTradeItemDataAsset* Item) const;

	/** Stolen units of Item. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetStolenQuantity(UTradeItemDataAsset* Item) const;

	/** Stolen units of everything in the hold. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetTotalStolenUnits() const;

	/** Total units of everything in the hold. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetTotalUnits() const;

	/**
	 * Get all cargo entries
	 * @return Array of cargo entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	TArray<FCargoEntry> GetCargoContents() const { return CargoInventory; }

	// ====================
	// EVENTS
	// ====================

	// Called when cargo is added
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCargoAdded, UTradeItemDataAsset*, Item, int32, Quantity);
	UPROPERTY(BlueprintAssignable, Category="Cargo|Events")
	FOnCargoAdded OnCargoAdded;

	// Called when cargo is removed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCargoRemoved, UTradeItemDataAsset*, Item, int32, Quantity);
	UPROPERTY(BlueprintAssignable, Category="Cargo|Events")
	FOnCargoRemoved OnCargoRemoved;

	// Called when cargo space changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCargoSpaceChanged, float, AvailableSpace);
	UPROPERTY(BlueprintAssignable, Category="Cargo|Events")
	FOnCargoSpaceChanged OnCargoSpaceChanged;

private:
	// Find the entry holding Item with this stolen tag (the same object first, then the same goods by ID)
	int32 FindCargoEntryIndex(UTradeItemDataAsset* Item, FName StolenFrom = NAME_None) const;

	/** Sum of entries holding Item; bClean/bStolen pick which. */
	int32 CountItem(const UTradeItemDataAsset* Item, bool bClean, bool bStolen) const;
};
