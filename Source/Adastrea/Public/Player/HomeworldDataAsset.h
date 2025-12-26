#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HomeworldDataAsset.generated.h"

/**
 * Data Asset for storing homeworld information.
 * 
 * Homeworlds represent the player's origin within the Adastrea universe and are central to the Way system.
 * Each homeworld determines the player's initial location, available resources, lore background, and starting conditions.
 * In the current design, homeworlds are used to set up the player's initial context, influence narrative paths,
 * and provide unique gameplay modifiers or opportunities based on their attributes.
 * Designers can configure homeworlds to affect progression, available technologies, and story branches via Data Assets.
 * 
 * Usage Example:
 * - Create a new Homeworld Data Asset in the editor
 * - Set properties such as name, description, starting location, and unique ID
 * - Reference the Data Asset in player setup or Way system logic to initialize the game state
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
