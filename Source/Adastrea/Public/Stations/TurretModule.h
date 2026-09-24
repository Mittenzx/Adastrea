// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "SpaceStationModule.h"
#include "TurretModule.generated.h"

/**
 * Turret module for space stations
 *
 * Automated point-defence / anti-ship weapon emplacement. Provides firepower
 * that the station's combat layer can dispatch; disabled when destroyed or out
 * of power.
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

	/** Whether the turret is currently able to fire. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Turret")
	bool CanFire() const;

	/** DPS while active = DamagePerShot * FireRate. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Turret")
	float GetDps() const;
};