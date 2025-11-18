#include "Combat/CombatVFXDataAsset.h"

UCombatVFXDataAsset::UCombatVFXDataAsset()
{
	// Set sensible defaults
	VFXName = FText::FromString("Combat VFX Configuration");
	Description = FText::FromString("Visual effects for weapon fire");
	WeaponType = EWeaponType::Energy;
	MaxParticles = 500;
	EffectPriority = 5;
	MaxRenderDistance = 10000.0f;
}

FLinearColor UCombatVFXDataAsset::GetBeamColor() const
{
	return FireEffect.BeamColor;
}

FLinearColor UCombatVFXDataAsset::GetHeatColor(float HeatPercentage) const
{
	// Clamp heat percentage
	HeatPercentage = FMath::Clamp(HeatPercentage, 0.0f, 1.0f);

	// Interpolate between cool, warning, and overheat colors
	if (HeatPercentage < 0.5f)
	{
		// Cool to Warning (0% - 50%)
		float Alpha = HeatPercentage * 2.0f;
		return FMath::Lerp(HeatVisuals.CoolColor, HeatVisuals.WarningColor, Alpha);
	}
	else
	{
		// Warning to Overheat (50% - 100%)
		float Alpha = (HeatPercentage - 0.5f) * 2.0f;
		return FMath::Lerp(HeatVisuals.WarningColor, HeatVisuals.OverheatColor, Alpha);
	}
}

bool UCombatVFXDataAsset::IsCompatibleWithWeaponType(EWeaponType Type) const
{
	return WeaponType == Type;
}
