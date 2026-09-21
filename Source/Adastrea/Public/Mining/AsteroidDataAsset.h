#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AsteroidDataAsset.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UTradeItemDataAsset;

/**
 * Data-driven asteroid type (rocky iron, copper-bearing, titanium-rich, icy...).
 * The ore it yields is a real trade item data asset, so mined ore is the same
 * item the station markets buy and sell.
 */
UCLASS(BlueprintType)
class ADASTREA_API UAsteroidDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAsteroidDataAsset();

	/** Name shown on the targeting HUD (e.g. "Iron-Rich Asteroid"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid")
	FText DisplayName;

	/** Trade item delivered into the cargo hold when this rock is mined. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ore")
	TObjectPtr<UTradeItemDataAsset> OreItem;

	/** Ore units a rock of this type yields per second at mining power 1.0 and hardness 1.0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ore", meta=(ClampMin="0.01"))
	float OreYieldPerSecond;

	/** Ore units contained in a rock at scale 1.0. Total ore scales with volume (scale^3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ore", meta=(ClampMin="1.0"))
	float OreUnitsAtUnitScale;

	/** Divides the laser's mining power. 1 = soft, 3 = very tough rock. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ore", meta=(ClampMin="0.1"))
	float Hardness;

	/** Relative spawn weight inside an asteroid field (higher = more common). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Field", meta=(ClampMin="0.0"))
	float Rarity;

	/** Random size multiplier range applied on top of each mesh's own size. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Field", meta=(ClampMin="0.01"))
	float MinScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Field", meta=(ClampMin="0.01"))
	float MaxScale;

	/** Rock meshes to pick from. Falls back to the engine sphere when empty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	TArray<TSoftObjectPtr<UStaticMesh>> Meshes;

	/** Material (instance of the parameterised asteroid master) for this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	TSoftObjectPtr<UMaterialInterface> Material;

	/** Colour of the ore veins / HUD tint for this type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
	FLinearColor OreTint;

	/** Total ore units for a rock at the given scale. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ore")
	float GetTotalOreForScale(float Scale) const;
};
