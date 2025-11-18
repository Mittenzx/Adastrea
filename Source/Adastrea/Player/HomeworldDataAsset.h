#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HomeworldDataAsset.generated.h"

/**
 * Data Asset for storing homeworld information.
 * Homeworlds define the player's starting location.
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

	// Starting location coordinates
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Homeworld Info")
	FVector StartingLocation;

	UHomeworldDataAsset();
};
