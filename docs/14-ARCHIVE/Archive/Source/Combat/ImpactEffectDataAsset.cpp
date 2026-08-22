#include "Combat/ImpactEffectDataAsset.h"

UImpactEffectDataAsset::UImpactEffectDataAsset()
{
	// Set sensible defaults
	EffectName = FText::FromString("Impact Effect Configuration");
	Description = FText::FromString("Visual effects for weapon impacts and explosions");
	MaxParticles = 2000;
	EffectPriority = 7;

	// Configure explosion sizes
	SmallExplosion.ExplosionSize = EExplosionSize::Small;
	SmallExplosion.ExplosionRadius = 200.0f;
	SmallExplosion.FlashIntensity = 2.0f;
	SmallExplosion.ShockwaveSpeed = 1000.0f;

	MediumExplosion.ExplosionSize = EExplosionSize::Medium;
	MediumExplosion.ExplosionRadius = 500.0f;
	MediumExplosion.FlashIntensity = 5.0f;
	MediumExplosion.ShockwaveSpeed = 2000.0f;

	LargeExplosion.ExplosionSize = EExplosionSize::Large;
	LargeExplosion.ExplosionRadius = 1000.0f;
	LargeExplosion.FlashIntensity = 10.0f;
	LargeExplosion.ShockwaveSpeed = 3000.0f;
}

FExplosionEffect UImpactEffectDataAsset::GetExplosionBySize(EExplosionSize Size) const
{
	switch (Size)
	{
	case EExplosionSize::Small:
		return SmallExplosion;
	case EExplosionSize::Medium:
		return MediumExplosion;
	case EExplosionSize::Large:
		return LargeExplosion;
	default:
		return MediumExplosion;
	}
}

FLinearColor UImpactEffectDataAsset::CalculateShieldImpactColor(float DamageAmount) const
{
	// Color intensity based on damage
	// Low damage: Blue
	// Medium damage: Cyan
	// High damage: White/Yellow

	float Intensity = FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f);

	if (Intensity < 0.33f)
	{
		// Low damage: Blue
		return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
	}
	else if (Intensity < 0.66f)
	{
		// Medium damage: Cyan
		return FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		// High damage: White/Yellow
		return FLinearColor(1.0f, 1.0f, 0.5f, 1.0f);
	}
}
