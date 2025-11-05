#pragma once

#include "CoreMinimal.h"
#include "Ships/SpaceshipDataAsset.h"
#include "MittenzxMk1DataAsset.generated.h"

/**
 * Mittenzx Mk1 spaceship data asset.
 * A custom spaceship based on the MyFirstSpaceship template with unique characteristics.
 * This ship is designed to spawn on game start and be possessable by the player.
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on this class in the editor
 * - Configure the DefaultSpaceshipClass in the Game Mode to spawn this ship
 * - The ship will automatically have an interior and be possessable
 */
UCLASS(BlueprintType)
class ADASTREA_API UMittenzxMk1DataAsset : public USpaceshipDataAsset
{
	GENERATED_BODY()

public:
	// Constructor sets custom values for Mittenzx Mk1
	UMittenzxMk1DataAsset();
};
