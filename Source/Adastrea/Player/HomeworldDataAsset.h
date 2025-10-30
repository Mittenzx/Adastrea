#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Public/Factions/FactionDataAsset.h"
#include "HomeworldDataAsset.generated.h"

/**
 * Faction relationship entry for homeworld
 * Defines the starting reputation with a specific faction
 */
USTRUCT(BlueprintType)
struct FFactionRelationEntry
{
	GENERATED_BODY()

	// The faction this relationship applies to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Relations")
	UFactionDataAsset* Faction;

	// Starting reputation with this faction (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Faction Relations", meta=(ClampMin="-100", ClampMax="100"))
	int32 StartingReputation;

	FFactionRelationEntry()
		: Faction(nullptr)
		, StartingReputation(0)
	{}
};

/**
 * Data Asset for storing homeworld information.
 * Homeworlds define the player's starting location and initial faction relationships.
 * Each homeworld sets up unique diplomatic standings with various factions.
 */
UCLASS(BlueprintType)
class ADASTREA_API UHomeworldDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Display name of the homeworld
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Homeworld Info")
	FText HomeworldName;

	// Description of the homeworld and its significance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Homeworld Info", meta=(MultiLine=true))
	FText Description;

	// Unique identifier for the homeworld
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Homeworld Info")
	FName HomeworldID;

	// Initial faction relationships for players from this homeworld
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Relations")
	TArray<FFactionRelationEntry> FactionRelations;

	UHomeworldDataAsset();

	/**
	 * Get the starting reputation with a specific faction
	 * @param Faction The faction to get starting reputation for
	 * @param OutReputation The starting reputation value (if found)
	 * @return True if a relationship entry exists for this faction
	 */
	UFUNCTION(BlueprintCallable, Category="Faction Relations")
	bool GetStartingReputation(const UFactionDataAsset* Faction, int32& OutReputation) const;

	/**
	 * Get all faction relationships for this homeworld
	 * @return Array of faction relation entries
	 */
	UFUNCTION(BlueprintCallable, Category="Faction Relations")
	TArray<FFactionRelationEntry> GetFactionRelations() const;

	/**
	 * Check if this homeworld has a defined relationship with a faction
	 * @param Faction The faction to check
	 * @return True if a relationship exists
	 */
	UFUNCTION(BlueprintCallable, Category="Faction Relations")
	bool HasRelationshipWith(const UFactionDataAsset* Faction) const;
};
