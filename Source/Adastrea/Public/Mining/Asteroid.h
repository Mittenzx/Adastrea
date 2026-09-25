#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ITargetable.h"
#include "Asteroid.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UAsteroidDataAsset;
class UMaterialInstanceDynamic;
class AAsteroid;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsteroidDepleted, AAsteroid*, Asteroid);

/**
 * A harvestable, targetable asteroid. Ore is drained by a mining laser via
 * ExtractOre(); the rock shrinks as it empties and is destroyed when depleted.
 */
UCLASS()
class ADASTREA_API AAsteroid : public AActor, public ITargetable
{
	GENERATED_BODY()

public:
	AAsteroid();

	/**
	 * Configure the rock. Call right after spawning.
	 * @param InScale     Actual actor scale at full size
	 * @param InRelScale  Size relative to the type's reference rock; drives total ore (scale^3)
	 * @param OreOverride Ore left, or < 0 for a full rock
	 */
	UFUNCTION(BlueprintCallable, Category="Asteroid")
	void InitAsteroid(UAsteroidDataAsset* InType, UStaticMesh* InMesh, float InScale, float InRelScale, float OreOverride = -1.0f);

	/**
	 * Drain ore from the rock.
	 * @param MiningPower Laser power (1.0 = reference)
	 * @param DeltaTime   Seconds of beam contact
	 * @return Ore units removed this call (fractional)
	 */
	float ExtractOre(float MiningPower, float DeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Asteroid")
	float GetOreFraction() const { return TotalOre > 0.0f ? RemainingOre / TotalOre : 0.0f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Asteroid")
	float GetRemainingOre() const { return RemainingOre; }

	/** Ore units the rock held when full. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Asteroid")
	float GetTotalOre() const { return TotalOre; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Asteroid")
	UAsteroidDataAsset* GetAsteroidType() const { return AsteroidType; }

	/** Approximate radius (cm) used for range checks and lock brackets. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Asteroid")
	float GetRadius() const;

	/** Index of this rock inside its owning field (-1 if standalone). */
	UPROPERTY(BlueprintReadOnly, Category="Asteroid")
	int32 FieldIndex;

	UPROPERTY(BlueprintAssignable, Category="Asteroid")
	FOnAsteroidDepleted OnDepleted;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// ITargetable
	virtual bool CanBeTargeted_Implementation() const override;
	virtual int32 GetTargetPriority_Implementation() const override { return 0; }
	virtual FText GetTargetDisplayName_Implementation() const override;
	virtual UTexture2D* GetTargetIcon_Implementation() const override { return nullptr; }
	virtual FVector GetAimPoint_Implementation() const override { return GetActorLocation(); }
	virtual float GetTargetSignature_Implementation() const override { return 1.0f; }
	virtual float GetDistanceFromLocation_Implementation(FVector FromLocation) const override;
	virtual bool IsHostileToActor_Implementation(AActor* Observer) const override { return false; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
	TObjectPtr<UAsteroidDataAsset> AsteroidType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
	float TotalOre;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
	float RemainingOre;

	/** Scale the rock had when full. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Asteroid")
	float FullScale;

	/** Smallest fraction of FullScale before the rock crumbles away. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Asteroid", meta=(ClampMin="0.05", ClampMax="1.0"))
	float MinShrinkScale;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MID;

	float MeshBaseRadius;

	void UpdateVisuals();
};
