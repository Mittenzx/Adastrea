#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipDrone.generated.h"

class AAsteroid;
class UDroneBayComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UTradeItemDataAsset;

UENUM(BlueprintType)
enum class EShipDroneState : uint8
{
	/** Dropping out of the hatch. */
	Launching,
	/** Flying to a spot on the rock. */
	Outbound,
	/** Clamped to the rock, drilling ore into the hopper. */
	Cutting,
	/** Flying back to the hatch approach point. */
	Returning,
	/** Final climb into the hatch; unloads and relaunches or stows. */
	Landing
};

/**
 * A mining drone launched from a UDroneBayComponent. Flies kinematically (no
 * collision), drills a spot on its bay's target rock until the hopper is full
 * or the rock is spent, then flies home and unloads. Destroyed when stowed.
 */
UCLASS()
class ADASTREA_API AShipDrone : public AActor
{
	GENERATED_BODY()

public:
	AShipDrone();

	/** Called by the bay right after spawning. */
	void InitDrone(UDroneBayComponent* InBay);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone")
	EShipDroneState GetDroneState() const { return State; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone")
	float GetOreAboard() const { return OreAboard; }

	/** Land immediately wherever it is: unload into the bay and stow (used when the ship docks). */
	void ForceStow();

	/** Drone size (cm across the body). */
	UPROPERTY(EditDefaultsOnly, Category="Drone")
	float BodySize;

	UPROPERTY(EditDefaultsOnly, Category="Drone|Visual")
	TSoftObjectPtr<UNiagaraSystem> DrillDust;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Drone")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Drone")
	TObjectPtr<UStaticMeshComponent> Body;

	/** Clamp frame the drone braces against the rock with. */
	UPROPERTY(VisibleAnywhere, Category="Drone")
	TObjectPtr<UStaticMeshComponent> Clamp;

	/** Drill head; spins while cutting. */
	UPROPERTY(VisibleAnywhere, Category="Drone")
	TObjectPtr<UStaticMeshComponent> Drill;

	UPROPERTY(VisibleAnywhere, Category="Drone")
	TObjectPtr<UStaticMeshComponent> Lamp;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UDroneBayComponent> Bay;

	UPROPERTY(Transient)
	TWeakObjectPtr<AAsteroid> WorkRock;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> DustFX;

	UPROPERTY(Transient)
	TWeakObjectPtr<UTradeItemDataAsset> HopperOre;

	EShipDroneState State;
	FVector Velocity;
	/** Work spot as a direction in the rock's local frame (the rock shrinks, so the distance is recomputed). */
	FVector WorkDirLocal;
	/** Surface distance along WorkDirLocal as a fraction of the rock's radius (found by a trace). */
	float WorkSurfaceFraction;
	float OreAboard;
	float StateTime;

	void SetState(EShipDroneState NewState);
	void SetCutting(bool bCutting);

	/** Pick a spot on the bay's target facing the ship; false if there is no rock to work. */
	bool ChooseWorkSpot();
	FVector GetWorkPoint() const;

	/** Steer toward Goal (moving at GoalVelocity); returns the remaining distance. */
	float FlyToward(const FVector& Goal, const FVector& GoalVelocity, float DeltaSeconds, bool bArrive);
	float GetMaxSpeed() const;

	void TickCutting(float DeltaSeconds);
	void Stow();
};
