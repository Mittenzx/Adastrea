#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatProjectileSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;
class UShipHealthComponent;

/** One bolt in flight. Bolts are plain data, not actors. */
struct FCombatBolt
{
	FVector Position;
	FVector Velocity;
	float Life;
	float Damage;
	float Length;
	/** Seconds the bolt has already flown when it is first stepped (fired part-way through a frame). */
	float PendingTime;
	TWeakObjectPtr<AActor> Instigator;
};

/** A short impact flash where a bolt struck. */
struct FCombatFlash
{
	FVector Position;
	float Age;
	float Size;
};

/**
 * Flies every projectile in the world. Each tick a bolt sweeps its path against the
 * registered ship hulls (UShipHealthComponent::SegmentHit; hulls have no collision)
 * and against world geometry (stations, asteroids) with a visibility trace; the
 * nearest hit wins. Ships take damage, anything else just stops the bolt.
 *
 * Bolts and impact flashes are drawn as two instanced meshes on one transient actor,
 * so a busy fight costs two draw calls rather than an actor per shot.
 */
UCLASS()
class ADASTREA_API UCombatProjectileSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UCombatProjectileSubsystem();

	static UCombatProjectileSubsystem* Get(const UObject* WorldContext);

	/**
	 * Launch a bolt. Velocity is absolute (include the shooter's own velocity). Age is how long
	 * ago it was fired within this frame, so shots fired together at a low frame rate spread out
	 * along the path instead of flying in a clump.
	 */
	void FireBolt(const FVector& Start, const FVector& Velocity, float Damage, float Life, float Length, AActor* Instigator, float Age = 0.0f);

	void RegisterTarget(UShipHealthComponent* Target);
	void UnregisterTarget(UShipHealthComponent* Target);

	/** Hits landed by this instigator since play began (HUD hit marker). */
	int32 GetHitCount(const AActor* Instigator) const;

	int32 GetBoltCount() const { return Bolts.Num(); }

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	void StepBolts(float DeltaTime);
	void UpdateVisuals(float DeltaTime);
	void EnsureVisualActor();

	TArray<FCombatBolt> Bolts;
	TArray<FCombatFlash> Flashes;
	TArray<TWeakObjectPtr<UShipHealthComponent>> Targets;
	TMap<TWeakObjectPtr<const AActor>, int32> HitCounts;

	UPROPERTY()
	TObjectPtr<UStaticMesh> BoltMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BoltMaterial;

	UPROPERTY()
	TObjectPtr<UStaticMesh> FlashMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> FlashMaterial;

	UPROPERTY(Transient)
	TObjectPtr<AActor> VisualActor;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> BoltMeshes;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> FlashMeshes;
};
