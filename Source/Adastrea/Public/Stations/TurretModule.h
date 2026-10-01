// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "TurretModule.generated.h"

/**
 * Turret module for space stations
 *
 * Automated anti-ship emplacement on the civil side (CombatTeam::Civil): it picks the
 * nearest live hostile ship in EngagementRange, slews its head onto the lead point at
 * TurnRate, and fires bolts from alternate barrels (Muzzle_L / Muzzle_R) through
 * UCombatProjectileSubsystem whenever the barrels are on target. Disabled when the
 * module is destroyed.
 *
 * Power Consumption: 25 units
 * Module Group: Defence
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API ATurretModule : public ASpaceStationModule
{
	GENERATED_BODY()

public:
	ATurretModule();

	/**
	 * Rotating head (SM_StationModule_TurretHead_01), attached to the base mesh at its
	 * TurretHead socket location. Yaw = local Z, barrels along +X; head sockets
	 * Muzzle_L / Muzzle_R / PitchAxis. Empty if the head mesh isn't imported.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turret")
	TObjectPtr<UStaticMeshComponent> TurretHeadComponent;

	/** Damage per shot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="0.0"))
	float DamagePerShot = 10.0f;

	/** Effective engagement range (units). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="1.0"))
	float EngagementRange = 12000.0f;

	/** Shots per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="0.05"))
	float FireRate = 2.0f;

	/** Bolt speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="1000.0"))
	float BoltSpeed = 25000.0f;

	/** Head slew rate (degrees per second, yaw and pitch). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="1.0"))
	float TurnRate = 90.0f;

	/** Fire when the barrels are within this angle of the lead point (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="0.1"))
	float FireConeDegrees = 4.0f;

	/** Random scatter around the barrel line (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret", meta=(ClampMin="0.0"))
	float SpreadDegrees = 0.8f;

	/** Elevation limits of the head, relative to the base (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret")
	float MinPitch = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Turret")
	float MaxPitch = 80.0f;

	/** The ship the turret is engaging, if any. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Turret")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/** Bolts fired since play began. */
	int32 GetShotsFired() const { return ShotsFired; }

	virtual void Tick(float DeltaSeconds) override;

	/** Whether the turret is currently able to fire. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Turret")
	bool CanFire() const;

	/** DPS while active = DamagePerShot * FireRate. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Turret")
	float GetDps() const;

private:
	/** Nearest live hostile ship in range of the head, or null. */
	AActor* FindTarget() const;
	FVector GetHeadPivot() const;
	void Fire(const FVector& Direction);

	TWeakObjectPtr<AActor> CurrentTarget;
	float RetargetClock = 0.0f;
	float Cooldown = 0.0f;
	int32 ShotsFired = 0;
	bool bLeftBarrel = true;
};