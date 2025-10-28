#pragma once

#include "CoreMinimal.h"
#include "Ships/SpaceshipDataAsset.h"
#include "MyFirstSpaceshipDataAsset.generated.h"

/**
 * Example spaceship data asset for new users to customize.
 * This provides a starting point with sensible defaults for a light scout ship.
 * Inherits all properties and functionality from USpaceshipDataAsset.
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on this class in the editor
 * - Customize the provided example values to create your own unique spaceship
 * - Use the inherited rating functions to evaluate ship performance
 */
UCLASS(BlueprintType)
class ADASTREA_API UMyFirstSpaceshipDataAsset : public USpaceshipDataAsset
{
	GENERATED_BODY()

public:
	// Constructor sets example values for a light scout ship
	UMyFirstSpaceshipDataAsset();
};
