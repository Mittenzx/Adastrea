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

	FCargoEntry()
		: Item(nullptr)
		, Quantity(0)
	{}

	FCargoEntry(UTradeItemDataAsset* InItem, int32 InQuantity)
		: Item(InItem)
		, Quantity(InQuantity)
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
	 * Get used cargo space
	 * @return Used volume in cargo hold
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	float GetUsedCargoSpace() const;

	/**
	 * Get cargo capacity utilization percentage
	 * @return Percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	float GetCargoUtilization() const;

	/**
	 * Check if cargo has space for item
	 * @param Item The item to check
	 * @param Quantity Number of units
	 * @return True if space is available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	bool HasSpaceFor(UTradeItemDataAsset* Item, int32 Quantity) const;

	/**
	 * Check if cargo contains item
	 * @param Item The item to check
	 * @param Quantity Minimum quantity needed (0 = any amount)
	 * @return True if item is in cargo with sufficient quantity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	bool HasItem(UTradeItemDataAsset* Item, int32 Quantity = 0) const;

	/**
	 * Get quantity of specific item in cargo
	 * @param Item The item to check
	 * @return Quantity in cargo (0 if not found)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetItemQuantity(UTradeItemDataAsset* Item) const;

	/**
	 * Get all cargo entries
	 * @return Array of cargo entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	TArray<FCargoEntry> GetCargoContents() const { return CargoInventory; }

	/**
	 * Get number of unique items in cargo
	 * @return Count of unique items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	int32 GetUniqueItemCount() const { return CargoInventory.Num(); }

	/**
	 * Check if cargo is empty
	 * @return True if no items in cargo
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	bool IsEmpty() const { return CargoInventory.Num() == 0; }

	/**
	 * Check if cargo is full
	 * @return True if no space available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo|Queries")
	bool IsFull() const { return GetAvailableCargoSpace() <= 0.0f; }

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
	// Find cargo entry index for item
	int32 FindCargoEntryIndex(UTradeItemDataAsset* Item) const;
};
