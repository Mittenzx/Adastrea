#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowDrone.generated.h"

class ASpaceship;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETowDroneState : uint8
{
	/** Flying out from the station to the wreck. */
	Outbound,
	/** Clamped on, turning the wreck toward the station. */
	Clamping,
	/** Hauling the wreck to the drop point outside the bay. */
	Towing,
	/** Wreck delivered; holding it at the drop point. */
	Arrived,
	/** Let go; flying home to be stowed. */
	Returning
};

/**
 * A station tow drone. Ships have no tractor beams: the drone flies out, clamps onto
 * the wreck's hull and physically hauls it to a point just outside the station's
 * docking bay. Kinematic, no collision. Spawned by UPlayerRescueSubsystem.
 */
UCLASS()
class ADASTREA_API ATowDrone : public AActor
{
	GENERATED_BODY()

public:
	ATowDrone();

	/** Start the job: fetch Wreck and bring it to DropPoint; Home is where the drone stows afterwards. */
	void StartTow(ASpaceship* InWreck, const FVector& InDropPoint, const FVector& InHome);

	/** Let go of the wreck and fly home (destroyed on arrival). */
	void Release();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Tow")
	ETowDroneState GetTowState() const { return State; }

	bool HasDelivered() const { return State == ETowDroneState::Arrived; }

	/** Cruise speed out to the wreck and home (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category="Tow")
	float FreeSpeed;

	/** Speed while hauling a wreck (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category="Tow")
	float TowSpeed;

	UPROPERTY(EditDefaultsOnly, Category="Tow")
	float Acceleration;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Tow")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Tow")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Tow")
	TObjectPtr<UStaticMeshComponent> ClampLeft;

	UPROPERTY(VisibleAnywhere, Category="Tow")
	TObjectPtr<UStaticMeshComponent> ClampRight;

	UPROPERTY(VisibleAnywhere, Category="Tow")
	TObjectPtr<UStaticMeshComponent> Lamp;

private:
	/** Where on the wreck the drone clamps: the hull's nose, in the wreck's frame. */
	FVector GetClampPointLocal() const;
	FVector GetClampPointWorld() const;

	TWeakObjectPtr<ASpaceship> Wreck;
	ETowDroneState State;
	FVector Velocity;
	FVector DropPoint;
	FVector Home;
	float StateTime;
};
