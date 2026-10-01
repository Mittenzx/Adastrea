#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShipWeaponComponent.generated.h"

class ASpaceship;

/**
 * The ship's fixed forward guns (X4-style): they point where the nose points and the
 * pilot aims by flying. Hold the trigger and the guns fire in turn. They converge at
 * the locked target's range (or ConvergenceDistance with nothing locked), so the
 * bolts meet on the lead pip when the nose is on it.
 *
 * Bolts inherit the ship's velocity and fly as data in UCombatProjectileSubsystem.
 * Damage scales with weapon upgrades (ASpaceship::GetWeaponDamageMultiplier).
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShipWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipWeaponComponent();

	/** Damage per bolt before upgrades. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="0.0"))
	float BoltDamage;

	/** Shots per second from each gun. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="0.1"))
	float FireRate;

	/** Bolt speed relative to the ship (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="1000.0"))
	float BoltSpeed;

	/** How far a bolt flies before it fizzles (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="1000.0"))
	float Range;

	/** Visual length of a bolt (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="10.0"))
	float BoltLength;

	/** Random scatter around the aim line (degrees, half-angle). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="0.0"))
	float SpreadDegrees;

	/** Where the guns converge with nothing locked (cm ahead of the hull centre). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="100.0"))
	float ConvergenceDistance;

	/** Guns on the hull. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapons", meta=(ClampMin="1", ClampMax="8"))
	int32 GunCount;

	UFUNCTION(BlueprintCallable, Category="Weapons")
	void SetTriggerHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapons")
	bool IsTriggerHeld() const { return bTriggerHeld; }

	/** The actor the guns converge on (the pilot's locked target). */
	void SetAimTarget(AActor* Target) { AimTarget = Target; }
	AActor* GetAimTarget() const { return AimTarget.Get(); }

	/** World point where the guns' aim lines meet. */
	FVector GetConvergencePoint() const;

	/** Where to put the nose so bolts meet Target (world). False when it can't be caught. */
	bool GetLeadPoint(const AActor* Target, FVector& OutPoint) const;

	/** Intercept for a projectile of BoltSpeed fired from Shooter (moving at ShooterVel, which bolts inherit). */
	static bool ComputeLeadPoint(const FVector& Shooter, const FVector& ShooterVel, const FVector& Target,
		const FVector& TargetVel, float ProjectileSpeed, FVector& OutPoint);

	/** Bolts fired since play began. */
	int32 GetShotsFired() const { return ShotsFired; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ASpaceship* GetShip() const;
	void BuildMuzzles();
	void FireNextGun(float Age);
	FTransform GetShipFrame() const;

	/** Gun muzzles in the ShipRoot frame (unscaled). */
	TArray<FVector> Muzzles;
	/** Hull centre in the ShipRoot frame. */
	FVector HullCentre;

	TWeakObjectPtr<AActor> AimTarget;
	float Cooldown;
	int32 NextGun;
	int32 ShotsFired;
	bool bTriggerHeld;
};
