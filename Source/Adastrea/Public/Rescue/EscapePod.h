#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EscapePod.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EEscapePodState : uint8
{
	/** Blasting clear of the wreck. */
	Ejecting,
	/** Flying to the pickup (a station bay or a friendly ship). */
	Transit,
	/** Arrived: taken aboard (hidden). */
	Aboard
};

/**
 * The player's escape pod: a small drone that carries the pilot from a disabled ship
 * to a friendly station or ship. The player possesses it (the camera follows it) but
 * has no controls; UPlayerRescueSubsystem sets where it goes.
 */
UCLASS()
class ADASTREA_API AEscapePod : public APawn
{
	GENERATED_BODY()

public:
	AEscapePod();

	/** Launch from the wreck with the wreck's velocity plus a kick along Up. */
	void Launch(const FVector& InheritedVelocity, const FVector& Up);

	/** Fly to Target (moving or not); arrival is reached within Tolerance cm. */
	void SetDestination(AActor* Target, float Tolerance);

	AActor* GetDestination() const { return Destination.Get(); }


	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Escape Pod")
	EEscapePodState GetPodState() const { return State; }

	bool HasArrived() const { return State == EEscapePodState::Aboard; }

	/** Cruise speed (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category="Escape Pod")
	float Speed;

	UPROPERTY(EditDefaultsOnly, Category="Escape Pod")
	float Acceleration;

	virtual FVector GetVelocity() const override { return PodVelocity; }

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<UStaticMeshComponent> Thruster;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<UStaticMeshComponent> Beacon;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, Category="Escape Pod")
	TObjectPtr<UCameraComponent> Camera;

private:
	TWeakObjectPtr<AActor> Destination;
	EEscapePodState State;
	FVector PodVelocity;
	float ArriveTolerance;
	float StateTime;
};
