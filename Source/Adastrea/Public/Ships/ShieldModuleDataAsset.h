// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Ships/ShipModuleDataAsset.h"
#include "ShieldModuleDataAsset.generated.h"

/**
 * Data Asset for shield generator module configurations
 * 
 * Defines shield specifications including strength, recharge rate,
 * resistance types, and coverage. Allows designers to create various
 * shield types from basic deflectors to advanced energy barriers.
 * 
 * Usage:
 * 1. Create Blueprint Data Asset based on this class
 * 2. Configure shield properties
 * 3. Assign to UShieldModuleComponent
 * 4. Install on ship via UShipCustomizationComponent
 */
UCLASS(BlueprintType)
class ADASTREA_API UShieldModuleDataAsset : public UShipModuleDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// SHIELD CAPACITY
	// ====================

	/** Maximum shield strength (hitpoints) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Capacity", meta=(ClampMin="0", ClampMax="100000"))
	float MaxShieldStrength;

	/** Shield recharge rate per second when not taking damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Capacity", meta=(ClampMin="0", ClampMax="10000"))
	float RechargeRate;

	/** Delay before recharge starts after taking damage (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Capacity", meta=(ClampMin="0", ClampMax="30"))
	float RechargeDelay;

	/** Shield regeneration efficiency (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Capacity", meta=(ClampMin="0", ClampMax="100"))
	float RegenerationEfficiency;

	// ====================
	// DAMAGE RESISTANCE
	// ====================

	/** Resistance to kinetic damage (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage Resistance", meta=(ClampMin="0", ClampMax="100"))
	float KineticResistance;

	/** Resistance to energy damage (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage Resistance", meta=(ClampMin="0", ClampMax="100"))
	float EnergyResistance;

	/** Resistance to explosive damage (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage Resistance", meta=(ClampMin="0", ClampMax="100"))
	float ExplosiveResistance;

	/** Resistance to thermal damage (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage Resistance", meta=(ClampMin="0", ClampMax="100"))
	float ThermalResistance;

	/** Resistance to EMP damage (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage Resistance", meta=(ClampMin="0", ClampMax="100"))
	float EMPResistance;

	// ====================
	// SPECIAL FEATURES
	// ====================

	/** Shield coverage percentage (100 = full coverage) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features", meta=(ClampMin="0", ClampMax="100"))
	float ShieldCoverage;

	/** Can adapt resistance based on incoming damage type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bAdaptiveShields;

	/** Adaptation rate (how quickly shields adapt, 1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features", meta=(ClampMin="1", ClampMax="10", EditCondition="bAdaptiveShields"))
	int32 AdaptationRate;

	/** Supports shield boosting (temporary strength increase) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bSupportsShieldBoost;

	/** Shield boost multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features", meta=(ClampMin="1.0", ClampMax="5.0", EditCondition="bSupportsShieldBoost"))
	float ShieldBoostMultiplier;

	/** Can phase shift (become invulnerable briefly) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Features")
	bool bSupportsPhaseShift;

	// ====================
	// VISUAL EFFECTS
	// ====================

	/** Shield bubble visual effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class UNiagaraSystem* ShieldBubbleEffect;

	/** Shield impact effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class UNiagaraSystem* ShieldImpactEffect;

	/** Shield material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	UMaterialInterface* ShieldMaterial;

	/** Shield activation sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class USoundBase* ActivationSound;

	/** Shield impact sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual Effects")
	class USoundBase* ImpactSound;

	// ====================
	// Constructor
	// ====================

	UShieldModuleDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Get the overall shield rating (0-100)
	 * @return Shield quality rating
	 */
	virtual float GetModuleRating() const override;

	/**
	 * Calculate effective damage after shield resistance
	 * @param IncomingDamage Base damage amount
	 * @param DamageType Type of damage
	 * @return Damage after shield resistance applied
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shield Stats")
	float CalculateEffectiveDamage(float IncomingDamage, uint8 DamageType) const;

	/**
	 * Get average resistance across all damage types
	 * @return Average resistance percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shield Stats")
	float GetAverageResistance() const;

	/**
	 * Get time to fully recharge from 0
	 * @return Time in seconds
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shield Stats")
	float GetFullRechargeTime() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
