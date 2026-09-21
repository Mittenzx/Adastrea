#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsteroidField.generated.h"

class UAsteroidDataAsset;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class AAsteroid;

/** One rock slot in the field: rendered as a cheap instance until the player comes near. */
struct FFieldRock
{
	TObjectPtr<UAsteroidDataAsset> Type = nullptr;
	TObjectPtr<UStaticMesh> Mesh = nullptr;
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Instances = nullptr;
	int32 InstanceIndex = INDEX_NONE;
	FTransform Home;          // full-size placement
	float Scale = 1.0f;       // uniform scale at full size
	float Ore = 0.0f;         // ore left (carried across promote/demote)
	TWeakObjectPtr<AAsteroid> Actor;
	double RespawnAt = 0.0;   // world time when a depleted rock regrows (0 = alive)
};

/**
 * Scatters mixed-type, mixed-size asteroids in a volume. Every rock is a hierarchical
 * instance (cheap); rocks within PromoteRadius of the player become real harvestable
 * AAsteroid actors and go back to instances when the player leaves. Depleted rocks
 * regrow after RespawnSeconds. The scatter is deterministic from Seed, so it is
 * also previewed in the editor viewport.
 */
UCLASS()
class ADASTREA_API AAsteroidField : public AActor
{
	GENERATED_BODY()

public:
	AAsteroidField();

	/** Asteroid types to scatter; each is picked with weight = its Rarity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field")
	TArray<TObjectPtr<UAsteroidDataAsset>> AsteroidTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="1", ClampMax="2000"))
	int32 AsteroidCount;

	/** Field half-extent (cm) on X/Y; Z uses VerticalFraction of it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="1000.0"))
	float FieldRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="0.05", ClampMax="1.0"))
	float VerticalFraction;

	/** Minimum gap (cm) kept between rock surfaces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="0.0"))
	float MinSeparation;

	/** Multiplier on every rock's random size (use to match the mining ship's scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="0.01"))
	float GlobalScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field")
	int32 Seed;

	/** Rocks closer than this to the player become real, harvestable actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="1000.0"))
	float PromoteRadius;

	/** Seconds before a depleted rock regrows. 0 disables regrowth. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Field", meta=(ClampMin="0.0"))
	float RespawnSeconds;

	/** Rebuild the instanced scatter (editor button / construction). */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Field")
	void Regenerate();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Field")
	int32 GetRockCount() const { return Rocks.Num(); }

	/** Number of rocks currently promoted to real actors. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Field")
	int32 GetPromotedCount() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	TArray<FFieldRock> Rocks;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> InstanceComps;

	float UpdateTimer;

	void ClearField();
	UHierarchicalInstancedStaticMeshComponent* FindOrCreateInstanceComp(UStaticMesh* Mesh, UAsteroidDataAsset* Type);
	void SetInstanceVisible(FFieldRock& Rock, bool bVisible, float ScaleFraction = 1.0f);
	void Promote(int32 Index);
	void Demote(int32 Index);

	UFUNCTION()
	void HandleAsteroidDepleted(AAsteroid* Asteroid);
};
