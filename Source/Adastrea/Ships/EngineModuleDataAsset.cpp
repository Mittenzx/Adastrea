// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Ships/EngineModuleDataAsset.h"

UEngineModuleDataAsset::UEngineModuleDataAsset()
{
	// Set category to Engine
	Category = EShipModuleCategory::Engine;

	// Default engine values
	MaxThrust = 10000.0f;
	MaxSpeed = 500.0f;
	Acceleration = 50.0f;
	BoostMultiplier = 2.0f;
	TurnRate = 45.0f;

	FuelConsumptionRate = 10.0f;
	BoostFuelMultiplier = 3.0f;
	Efficiency = 75.0f;

	HeatGeneration = 20.0f;
	BoostHeatMultiplier = 1.5f;
	MaxHeat = 100.0f;
	MaxHeatOverrunPercent = 20.0f;
	CoolingRate = 10.0f;
	Reliability = 5;

	bSupportsTravelMode = false;
	TravelModeMultiplier = 5.0f;
	bSupportsJumpDrive = false;
	JumpRange = 0.0f;
	bSupportsSilentRunning = false;

	EngineTrailEffect = nullptr;
	EngineGlowMaterial = nullptr;
	BoostEffect = nullptr;
	EngineSound = nullptr;
	
	// Default sound parameters
	EngineSoundMinPitch = 0.8f;
	EngineSoundMaxPitch = 1.2f;
	EngineSoundMinVolume = 0.5f;
	EngineSoundMaxVolume = 1.0f;

	// Default physical properties for engines
	Mass = 500.0f;
	PowerRequirement = 100.0f;
}

float UEngineModuleDataAsset::GetModuleRating() const
{
	// Calculate rating based on performance metrics
	float ThrustScore = FMath::Clamp(MaxThrust / 100000.0f, 0.0f, 1.0f) * 25.0f;
	float SpeedScore = FMath::Clamp(MaxSpeed / 1000.0f, 0.0f, 1.0f) * 25.0f;
	float AccelScore = FMath::Clamp(Acceleration / 100.0f, 0.0f, 1.0f) * 20.0f;
	float EfficiencyScore = (Efficiency / 100.0f) * 15.0f;
	
	float SpecialBonus = 0.0f;
	if (bSupportsTravelMode) SpecialBonus += 5.0f;
	if (bSupportsJumpDrive) SpecialBonus += 5.0f;
	if (bSupportsSilentRunning) SpecialBonus += 5.0f;

	return FMath::Clamp(ThrustScore + SpeedScore + AccelScore + EfficiencyScore + SpecialBonus, 0.0f, 100.0f);
}

float UEngineModuleDataAsset::GetEffectiveThrust(float ThrottlePercent, bool bBoostActive) const
{
	float BaseThrust = MaxThrust * (ThrottlePercent / 100.0f);
	
	if (bBoostActive)
	{
		BaseThrust *= BoostMultiplier;
	}

	// Apply efficiency
	BaseThrust *= (Efficiency / 100.0f);

	return BaseThrust;
}

float UEngineModuleDataAsset::GetEffectiveMaxSpeed(bool bBoostActive, bool bTravelMode) const
{
	float EffectiveSpeed = MaxSpeed;

	if (bBoostActive)
	{
		EffectiveSpeed *= BoostMultiplier;
	}

	if (bTravelMode && bSupportsTravelMode)
	{
		EffectiveSpeed *= TravelModeMultiplier;
	}

	return EffectiveSpeed;
}

float UEngineModuleDataAsset::GetTimeToMaxSpeed() const
{
	if (Acceleration <= 0.0f)
	{
		return 0.0f;
	}

	return MaxSpeed / Acceleration;
}

float UEngineModuleDataAsset::GetCurrentFuelConsumption(float ThrottlePercent, bool bBoostActive) const
{
	float Consumption = FuelConsumptionRate * (ThrottlePercent / 100.0f);

	if (bBoostActive)
	{
		Consumption *= BoostFuelMultiplier;
	}

	// Apply efficiency: Lower efficiency means higher fuel consumption
	// Formula: Scale consumption inversely with efficiency
	// At 100% efficiency: consumption *= 0.5 (50% base consumption)
	// At 0% efficiency: consumption *= 1.5 (150% base consumption)
	float EfficiencyMultiplier = 1.5f - (Efficiency / 100.0f);
	Consumption *= EfficiencyMultiplier;

	return Consumption;
}

#if WITH_EDITOR
EDataValidationResult UEngineModuleDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate engine-specific properties
	if (MaxThrust <= 0.0f)
	{
		Context.AddError(FText::FromString("Max Thrust must be greater than 0"));
		Result = EDataValidationResult::Invalid;
	}

	if (MaxSpeed <= 0.0f)
	{
		Context.AddError(FText::FromString("Max Speed must be greater than 0"));
		Result = EDataValidationResult::Invalid;
	}

	if (Acceleration <= 0.0f)
	{
		Context.AddError(FText::FromString("Acceleration must be greater than 0"));
		Result = EDataValidationResult::Invalid;
	}

	if (Efficiency < 0.0f || Efficiency > 100.0f)
	{
		Context.AddError(FText::FromString("Efficiency must be between 0 and 100"));
		Result = EDataValidationResult::Invalid;
	}

	if (bSupportsJumpDrive && JumpRange <= 0.0f)
	{
		Context.AddWarning(FText::FromString("Jump-capable engines must have Jump Range > 0"));
	}

	if (bSupportsTravelMode && TravelModeMultiplier <= 1.0f)
	{
		Context.AddWarning(FText::FromString("Travel Mode Multiplier should be > 1.0"));
	}

	return Result;
}
#endif
