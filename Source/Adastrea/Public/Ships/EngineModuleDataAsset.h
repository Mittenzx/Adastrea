// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Ships/ShipModuleDataAsset.h"
#include "EngineModuleDataAsset.generated.h"

/**
 * Data Asset for engine module configurations
 * 
 * Defines engine specifications including thrust, speed, acceleration,
 * and fuel consumption. Allows designers to create various engine types
 * from small maneuvering thrusters to large capital ship drives.
 * 
 * Usage:
 * 1. Create Blueprint Data Asset based on this class
 * 2. Configure engine properties
 * 3. Assign to UEngineModuleComponent
 * 4. Install on ship via UShipCustomizationComponent
 */
UCLASS(BlueprintType)
class ADASTREA_API UEngineModuleDataAsset : public UShipModuleDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// ENGINE PERFORMANCE
	// ====================

	/** Maximum thrust output in Newtons */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Performance", meta=(ClampMin="0", ClampMax="10000000"))
	float MaxThrust;

	/** Maximum speed contribution in m/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Performance", meta=(ClampMin="0", ClampMax="10000"))
	float MaxSpeed;

	/** Acceleration rate in m/s² */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Performance", meta=(ClampMin="0", ClampMax="1000"))
	float Acceleration;

	/** Boost speed multiplier when boost is active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Performance", meta=(ClampMin="1.0", ClampMax="5.0"))
	float BoostMultiplier;

	/** Turn rate contribution (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Engine Performance", meta=(ClampMin="0", ClampMax="360"))
	float TurnRate;

	// ====================
	// FUEL AND EFFICIENCY
	// ====================

	/** Fuel consumption rate (units per second at full thrust) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fuel", meta=(ClampMin="0", ClampMax="1000"))
	float FuelConsumptionRate;

	/** Fuel consumption during boost (multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fuel", meta=(ClampMin="1.0", ClampMax="10.0"))
	float BoostFuelMultiplier;

	/** Engine efficiency rating (0-100, affects actual output vs theoretical) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fuel", meta=(ClampMin="0", ClampMax="100"))
	float Efficiency;

	// ====================
	// HEAT AND DURABILITY
	// ====================

	/** Heat generation rate (units per second at full thrust) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat", meta=(ClampMin="0", ClampMax="1000"))
	float HeatGeneration;

	/** Maximum heat before overheating */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat", meta=(ClampMin="0", ClampMax="1000"))
	float MaxHeat;

	/** Cooling rate (heat units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat", meta=(ClampMin="0", ClampMax="100"))
	float CoolingRate;

	/** Reliability rating (1-10, affects breakdown chance) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Durability", meta=(ClampMin="1", ClampMax="10"))
	int32 Reliability;

	// ====================
	// SPECIAL FEATURES
	// ====================

	/** Can activate supercruise/travel mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bSupportsTravelMode;

	/** Travel mode speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features", meta=(ClampMin="1.0", ClampMax="20.0", EditCondition="bSupportsTravelMode"))
	float TravelModeMultiplier;

	/** Supports FTL/jump drive functionality */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bSupportsJumpDrive;

	/** Jump range in light years (if jump capable) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features", meta=(ClampMin="0", ClampMax="1000", EditCondition="bSupportsJumpDrive"))
	float JumpRange;

	/** Silent running capable (reduced heat signature) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bSupportsSilentRunning;

	// ====================
	// VISUAL EFFECTS
	// ====================

	/** Engine trail particle system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class UNiagaraSystem* EngineTrailEffect;

	/** Engine glow material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	UMaterialInterface* EngineGlowMaterial;

	/** Boost visual effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class UNiagaraSystem* BoostEffect;

	/** Engine sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class USoundBase* EngineSound;

	// ====================
	// Constructor
	// ====================

	UEngineModuleDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Get the overall engine rating (0-100)
	 * Based on thrust, speed, efficiency, and special features
	 * @return Engine quality rating
	 */
	virtual float GetModuleRating() const override;

	/**
	 * Calculate effective thrust at given throttle and conditions
	 * @param ThrottlePercent Throttle percentage (0-100)
	 * @param bBoostActive Whether boost is active
	 * @return Effective thrust output in Newtons
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine Stats")
	float GetEffectiveThrust(float ThrottlePercent, bool bBoostActive) const;

	/**
	 * Calculate effective max speed with modifiers
	 * @param bBoostActive Whether boost is active
	 * @param bTravelMode Whether travel mode is active
	 * @return Effective maximum speed in m/s
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine Stats")
	float GetEffectiveMaxSpeed(bool bBoostActive, bool bTravelMode) const;

	/**
	 * Get estimated time to reach max speed from standstill
	 * @return Time in seconds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine Stats")
	float GetTimeToMaxSpeed() const;

	/**
	 * Get fuel consumption at current throttle
	 * @param ThrottlePercent Throttle percentage (0-100)
	 * @param bBoostActive Whether boost is active
	 * @return Fuel consumption rate (units per second)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine Stats")
	float GetCurrentFuelConsumption(float ThrottlePercent, bool bBoostActive) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
