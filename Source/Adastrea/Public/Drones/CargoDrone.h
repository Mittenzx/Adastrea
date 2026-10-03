#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Trading/CargoComponent.h"
#include "CargoDrone.generated.h"

class ASpaceship;
class UDroneBayComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ECargoDroneState : uint8
{
	/** Dropping out of the hatch. */
	Launching,
	/** Flying to the other ship's hull. */
	Outbound,
	/** Clamped on, moving the load through the other ship's cargo hatch. */
	Working,
	/** Flying back to the hatch approach point. */
	Returning,
	/** Final climb into the bay; unloads, then goes out again or stows. */
	Landing
};

/**
 * A cargo drone from a UDroneBayComponent. Ships have no tractor beams: to move
 * goods between two ships in space, drones fly across, clamp onto the other hull
 * and carry the load over a pod at a time. Taking (from a wreck's hold, or a ship
 * of the same owner) fills the pod at the other ship and empties it into the home
 * hold; giving fills it at launch and empties it at the other ship. Kinematic, no
 * collision. Spawned and fed by the bay (UDroneBayComponent::StartCargoTransfer).
 */
UCLASS()
class ADASTREA_API ACargoDrone : public AActor
{
	GENERATED_BODY()

public:
	ACargoDrone();

	/** Called by the bay right after spawning. Slot spreads drones along the other hull. */
	void InitDrone(UDroneBayComponent* InBay, ASpaceship* InOther, int32 InSlot);

	/** Goods in the pod. */
	TArray<FCargoEntry> Load;

	float GetLoadVolume() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Cargo Drone")
	ECargoDroneState GetDroneState() const { return State; }

	/** Land at once wherever it is and hand the pod to the bay (the ship is docking or the job ended). */
	void ForceStow();

	UPROPERTY(EditDefaultsOnly, Category="Cargo Drone")
	float Speed;

	UPROPERTY(EditDefaultsOnly, Category="Cargo Drone")
	float Acceleration;

	/** Seconds clamped on the other hull per load. */
	UPROPERTY(EditDefaultsOnly, Category="Cargo Drone")
	float WorkSeconds;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Cargo Drone")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Cargo Drone")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Cargo Drone")
	TObjectPtr<UStaticMeshComponent> Pod;

	UPROPERTY(VisibleAnywhere, Category="Cargo Drone")
	TObjectPtr<UStaticMeshComponent> Lamp;

private:
	/** Where the drone clamps on the other ship: on top of the hull, spread by slot. */
	FVector GetWorkPoint() const;
	void SetState(ECargoDroneState NewState);

	TWeakObjectPtr<UDroneBayComponent> Bay;
	TWeakObjectPtr<ASpaceship> Other;
	int32 Slot = 0;
	ECargoDroneState State = ECargoDroneState::Launching;
	FVector Velocity = FVector::ZeroVector;
	float StateTime = 0.0f;
};
