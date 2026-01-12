// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/ShieldModuleDataAsset.h"
#include "Interfaces/IDamageable.h"
#include "Misc/DataValidation.h"

UShieldModuleDataAsset::UShieldModuleDataAsset()
{
	// Set category to Shield
	Category = EShipModuleCategory::Shield;

	// Default shield values
	MaxShieldStrength = 1000.0f;
	RechargeRate = 50.0f;
	RechargeDelay = 5.0f;
	RegenerationEfficiency = 75.0f;

	// Default resistances (balanced)
	KineticResistance = 30.0f;
	EnergyResistance = 40.0f;
	ExplosiveResistance = 25.0f;
	ThermalResistance = 35.0f;
	EMPResistance = 20.0f;

	// Special features
	ShieldCoverage = 100.0f;
	bAdaptiveShields = false;
	AdaptationRate = 5;
	bSupportsShieldBoost = false;
	ShieldBoostMultiplier = 2.0f;
	bSupportsPhaseShift = false;

	// Effects
	ShieldBubbleEffect = nullptr;
	ShieldImpactEffect = nullptr;
	ShieldMaterial = nullptr;
	ActivationSound = nullptr;
	ImpactSound = nullptr;

	// Default physical properties for shields
	Mass = 200.0f;
	PowerRequirement = 150.0f;
}

float UShieldModuleDataAsset::GetModuleRating() const
{
	// Calculate rating based on shield metrics
	float StrengthScore = FMath::Clamp(MaxShieldStrength / 10000.0f, 0.0f, 1.0f) * 30.0f;
	float RechargeScore = FMath::Clamp(RechargeRate / 1000.0f, 0.0f, 1.0f) * 20.0f;
	float ResistanceScore = GetAverageResistance() * 0.3f;
	float CoverageScore = (ShieldCoverage / 100.0f) * 10.0f;
	
	float SpecialBonus = 0.0f;
	if (bAdaptiveShields) SpecialBonus += 5.0f;
	if (bSupportsShieldBoost) SpecialBonus += 3.0f;
	if (bSupportsPhaseShift) SpecialBonus += 7.0f;

	return FMath::Clamp(StrengthScore + RechargeScore + ResistanceScore + CoverageScore + SpecialBonus, 0.0f, 100.0f);
}

float UShieldModuleDataAsset::CalculateEffectiveDamage(float IncomingDamage, EDamageType DamageType) const
{
	float Resistance = 0.0f;

	// Map damage type to resistance
	switch (DamageType)
	{
		case EDamageType::Kinetic:
			Resistance = KineticResistance;
			break;
		case EDamageType::Energy:
			Resistance = EnergyResistance;
			break;
		case EDamageType::Explosive:
			Resistance = ExplosiveResistance;
			break;
		case EDamageType::Thermal:
			Resistance = ThermalResistance;
			break;
		case EDamageType::EMP:
			Resistance = EMPResistance;
			break;
		default:
			Resistance = 0.0f;
			break;
	}

	// Apply resistance
	float DamageMultiplier = 1.0f - (Resistance / 100.0f);
	return IncomingDamage * DamageMultiplier;
}

float UShieldModuleDataAsset::GetAverageResistance() const
{
	return (KineticResistance + EnergyResistance + ExplosiveResistance + ThermalResistance + EMPResistance) / 5.0f;
}

float UShieldModuleDataAsset::GetFullRechargeTime() const
{
	if (RechargeRate <= 0.0f)
	{
		return 0.0f;
	}

	float EffectiveRate = RechargeRate * (RegenerationEfficiency / 100.0f);
	return RechargeDelay + (MaxShieldStrength / EffectiveRate);
}

#if WITH_EDITOR
EDataValidationResult UShieldModuleDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate shield-specific properties
	if (MaxShieldStrength <= 0.0f)
	{
		Context.AddError(FText::FromString("Max Shield Strength must be greater than 0"));
		Result = EDataValidationResult::Invalid;
	}

	if (RechargeRate < 0.0f)
	{
		Context.AddError(FText::FromString("Recharge Rate cannot be negative"));
		Result = EDataValidationResult::Invalid;
	}

	if (RegenerationEfficiency < 0.0f || RegenerationEfficiency > 100.0f)
	{
		Context.AddError(FText::FromString("Regeneration Efficiency must be between 0 and 100"));
		Result = EDataValidationResult::Invalid;
	}

	// Validate resistances
	if (KineticResistance < 0.0f || KineticResistance > 100.0f)
	{
		Context.AddError(FText::FromString("Kinetic Resistance must be between 0 and 100"));
		Result = EDataValidationResult::Invalid;
	}

	if (ShieldCoverage < 0.0f || ShieldCoverage > 100.0f)
	{
		Context.AddError(FText::FromString("Shield Coverage must be between 0 and 100"));
		Result = EDataValidationResult::Invalid;
	}

	if (bAdaptiveShields && AdaptationRate < 1)
	{
		Context.AddWarning(FText::FromString("Adaptation Rate must be at least 1 for adaptive shields"));
	}

	return Result;
}
#endif
