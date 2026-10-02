#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/RaidSubsystem.h"
#include "LootDrone.generated.h"

class ASpaceship;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ELootDroneState : uint8
{
	/** Flying from the raider to the target's hull. */
	Outbound,
	/** Clamped on, emptying the hold. */
	Working,
	/** Flying the load back to a raider. */
	Returning
};

/**
 * A pirate cargo drone. Ships have no tractor beams: the drone flies across, clamps
 * onto the target's hull, takes its share of the demand (or everything, from a wreck),
 * and flies it back to a raider, where the load reaches the gang. Kinematic, no
 * collision. Spawned and fed by URaidSubsystem.
 */
UCLASS()
class ADASTREA_API ALootDrone : public AActor
{
	GENERATED_BODY()

public:
	ALootDrone();

	/** Start the job: fly from Home to Target for raid RaidId. */
	void StartJob(int32 InRaidId, ASpaceship* InHome, ASpaceship* InTarget, int32 InSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Loot Drone")
	ELootDroneState GetDroneState() const { return State; }

	int32 GetRaidId() const { return RaidId; }
	int32 GetSlot() const { return Slot; }
	ASpaceship* GetTarget() const { return Target.Get(); }

	/** What the drone is carrying home. */
	TArray<FRaidGoods> Goods;
	int32 Credits = 0;
	float SalvagePoints = 0.0f;

	bool IsCarrying() const { return !Goods.IsEmpty() || Credits > 0 || SalvagePoints > 0.0f; }

	UPROPERTY(EditDefaultsOnly, Category="Loot Drone")
	float Speed;

	UPROPERTY(EditDefaultsOnly, Category="Loot Drone")
	float Acceleration;

	/** Seconds clamped on the hull while emptying the hold. */
	UPROPERTY(EditDefaultsOnly, Category="Loot Drone")
	float WorkSeconds;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Loot Drone")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Loot Drone")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Loot Drone")
	TObjectPtr<UStaticMeshComponent> Pod;

	UPROPERTY(VisibleAnywhere, Category="Loot Drone")
	TObjectPtr<UStaticMeshComponent> Lamp;

private:
	/** Where on the target the drone clamps: above the hull, spread out by slot. */
	FVector GetWorkPoint() const;

	int32 RaidId = INDEX_NONE;
	int32 Slot = 0;
	TWeakObjectPtr<ASpaceship> Home;
	TWeakObjectPtr<ASpaceship> Target;
	ELootDroneState State = ELootDroneState::Outbound;
	FVector Velocity = FVector::ZeroVector;
	float StateTime = 0.0f;
};
