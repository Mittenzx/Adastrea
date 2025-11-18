#include "Combat/CombatVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"

UCombatVFXComponent::UCombatVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Set performance defaults
	MaxActiveEffects = 50;
	CurrentActiveEffects = 0;
	bEnableDistanceCulling = true;
	MaxEffectDistance = 10000.0f;

	// Set component defaults
	CombatVFXData = nullptr;
	ImpactEffectData = nullptr;
}

void UCombatVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-detect weapon components if not set
	if (WeaponComponents.Num() == 0)
	{
		TArray<UWeaponComponent*> FoundWeapons;
		GetOwner()->GetComponents<UWeaponComponent>(FoundWeapons);
		WeaponComponents = FoundWeapons;
	}

	// Register all weapon components
	for (UWeaponComponent* Weapon : WeaponComponents)
	{
		if (Weapon)
		{
			RegisterWeaponComponent(Weapon);
		}
	}
}

void UCombatVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cleanup finished effects
	CleanupFinishedEffects();

	// Update weapon heat visuals
	for (UWeaponComponent* Weapon : WeaponComponents)
	{
		if (Weapon)
		{
			float HeatPercentage = Weapon->GetHeatPercentage();
			UpdateWeaponHeatVisuals(Weapon, HeatPercentage);
		}
	}
}

void UCombatVFXComponent::RegisterWeaponComponent(UWeaponComponent* WeaponComponent)
{
	if (!WeaponComponent)
	{
		return;
	}

	// In a full implementation, bind to weapon's OnWeaponFired event
	// For now, we provide the function that would be called
}

void UCombatVFXComponent::HandleWeaponFired(UWeaponComponent* WeaponComponent, AActor* Target)
{
	if (!WeaponComponent || !WeaponComponent->WeaponData)
	{
		return;
	}

	// Get weapon properties
	EWeaponType WeaponType = WeaponComponent->WeaponData->WeaponType;
	FVector MuzzleLocation = WeaponComponent->GetWeaponWorldPosition();
	FRotator MuzzleRotation = WeaponComponent->GetAimDirection().Rotation();
	FVector TargetLocation = Target ? Target->GetActorLocation() : FVector::ZeroVector;

	// Play appropriate effect
	PlayWeaponFireEffect(WeaponType, MuzzleLocation, MuzzleRotation, TargetLocation);

	// Trigger Blueprint event
	OnWeaponFired(WeaponType, MuzzleLocation);
}

UNiagaraComponent* UCombatVFXComponent::PlayWeaponFireEffect(
	EWeaponType WeaponType,
	FVector MuzzleLocation,
	FRotator MuzzleRotation,
	FVector TargetLocation)
{
	if (!CombatVFXData || !CanSpawnEffect(CombatVFXData->EffectPriority))
	{
		return nullptr;
	}

	if (bEnableDistanceCulling && !IsWithinRenderDistance(MuzzleLocation))
	{
		return nullptr;
	}

	UNiagaraComponent* SpawnedEffect = nullptr;

	// Play muzzle flash for all weapon types
	PlayMuzzleFlash(MuzzleLocation, MuzzleRotation);

	// Play type-specific effects
	switch (WeaponType)
	{
	case EWeaponType::Energy:
		// Laser beam with glow and heat distortion
		if (TargetLocation != FVector::ZeroVector)
		{
			SpawnedEffect = PlayBeamEffect(
				MuzzleLocation,
				TargetLocation,
				CombatVFXData->FireEffect.BeamWidth,
				CombatVFXData->FireEffect.BeamColor,
				0.1f  // Instant beam
			);
		}
		break;

	case EWeaponType::Projectile:
		// Tracer rounds with muzzle flash
		{
			FVector Direction = MuzzleRotation.Vector();
			FVector Velocity = Direction * 5000.0f;  // Fast projectile
			SpawnedEffect = PlayProjectileTrail(MuzzleLocation, MuzzleRotation, Velocity, WeaponType);
		}
		break;

	case EWeaponType::Missile:
		// Smoke trail and engine glow
		{
			FVector Direction = MuzzleRotation.Vector();
			FVector Velocity = Direction * 2000.0f;  // Slower, homing missile
			SpawnedEffect = PlayProjectileTrail(MuzzleLocation, MuzzleRotation, Velocity, WeaponType);
		}
		break;

	case EWeaponType::Torpedo:
		// Plasma trail effect
		{
			FVector Direction = MuzzleRotation.Vector();
			FVector Velocity = Direction * 1500.0f;  // Heavy, slow torpedo
			SpawnedEffect = PlayProjectileTrail(MuzzleLocation, MuzzleRotation, Velocity, WeaponType);
		}
		break;

	case EWeaponType::Beam:
		// Sustained beam with start/end points
		if (TargetLocation != FVector::ZeroVector)
		{
			SpawnedEffect = PlayBeamEffect(
				MuzzleLocation,
				TargetLocation,
				CombatVFXData->FireEffect.BeamWidth,
				CombatVFXData->FireEffect.BeamColor,
				CombatVFXData->FireEffect.BeamDuration  // Sustained
			);
		}
		break;

	case EWeaponType::Plasma:
		// Ball lightning with arcing
		{
			FVector Direction = MuzzleRotation.Vector();
			FVector Velocity = Direction * 3000.0f;  // Medium speed plasma bolt
			SpawnedEffect = PlayProjectileTrail(MuzzleLocation, MuzzleRotation, Velocity, WeaponType);
		}
		break;
	}

	// Play fire sound
	if (CombatVFXData->FireEffect.FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CombatVFXData->FireEffect.FireSound, MuzzleLocation);
	}

	// Apply camera shake if available
	if (CombatVFXData->FireEffect.CameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			CombatVFXData->FireEffect.CameraShake,
			MuzzleLocation,
			0.0f,      // Inner radius
			1000.0f    // Outer radius
		);
	}

	if (SpawnedEffect)
	{
		ActiveEffects.Add(SpawnedEffect);
		CurrentActiveEffects = ActiveEffects.Num();
	}

	return SpawnedEffect;
}

UNiagaraComponent* UCombatVFXComponent::PlayMuzzleFlash(FVector MuzzleLocation, FRotator MuzzleRotation)
{
	if (!CombatVFXData || !CombatVFXData->FireEffect.MuzzleFlashEffect)
	{
		return nullptr;
	}

	UNiagaraComponent* FlashEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CombatVFXData->FireEffect.MuzzleFlashEffect,
		MuzzleLocation,
		MuzzleRotation
	);

	if (FlashEffect)
	{
		FlashEffect->SetFloatParameter(FName("Intensity"), CombatVFXData->FireEffect.GlowIntensity);
		ActiveEffects.Add(FlashEffect);
		CurrentActiveEffects = ActiveEffects.Num();
	}

	return FlashEffect;
}

UNiagaraComponent* UCombatVFXComponent::PlayBeamEffect(
	FVector StartLocation,
	FVector EndLocation,
	float BeamWidth,
	FLinearColor BeamColor,
	float Duration)
{
	if (!CombatVFXData || !CombatVFXData->FireEffect.BeamEffect)
	{
		return nullptr;
	}

	UNiagaraComponent* BeamEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CombatVFXData->FireEffect.BeamEffect,
		StartLocation,
		(EndLocation - StartLocation).Rotation()
	);

	if (BeamEffect)
	{
		BeamEffect->SetVectorParameter(FName("BeamStart"), StartLocation);
		BeamEffect->SetVectorParameter(FName("BeamEnd"), EndLocation);
		BeamEffect->SetFloatParameter(FName("BeamWidth"), BeamWidth);
		BeamEffect->SetColorParameter(FName("BeamColor"), BeamColor);
		BeamEffect->SetFloatParameter(FName("Duration"), Duration);
		BeamEffect->SetFloatParameter(FName("HeatDistortion"), CombatVFXData->FireEffect.HeatDistortionStrength);

		ActiveEffects.Add(BeamEffect);
		CurrentActiveEffects = ActiveEffects.Num();
	}

	return BeamEffect;
}

UNiagaraComponent* UCombatVFXComponent::PlayProjectileTrail(
	FVector SpawnLocation,
	FRotator SpawnRotation,
	FVector ProjectileVelocity,
	EWeaponType WeaponType)
{
	if (!CombatVFXData || !CombatVFXData->FireEffect.ProjectileTrailEffect)
	{
		return nullptr;
	}

	UNiagaraComponent* TrailEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CombatVFXData->FireEffect.ProjectileTrailEffect,
		SpawnLocation,
		SpawnRotation
	);

	if (TrailEffect)
	{
		TrailEffect->SetVectorParameter(FName("Velocity"), ProjectileVelocity);
		TrailEffect->SetColorParameter(FName("TrailColor"), CombatVFXData->FireEffect.BeamColor);
		
		// Weapon-specific trail properties
		switch (WeaponType)
		{
		case EWeaponType::Missile:
			TrailEffect->SetFloatParameter(FName("TrailWidth"), 20.0f);
			TrailEffect->SetFloatParameter(FName("EngineGlow"), 2.0f);
			break;
		case EWeaponType::Torpedo:
			TrailEffect->SetFloatParameter(FName("TrailWidth"), 30.0f);
			TrailEffect->SetFloatParameter(FName("PlasmaIntensity"), 3.0f);
			break;
		case EWeaponType::Plasma:
			TrailEffect->SetFloatParameter(FName("TrailWidth"), 15.0f);
			TrailEffect->SetFloatParameter(FName("ArcingIntensity"), 1.5f);
			break;
		default:
			TrailEffect->SetFloatParameter(FName("TrailWidth"), 5.0f);
			break;
		}

		ActiveEffects.Add(TrailEffect);
		CurrentActiveEffects = ActiveEffects.Num();
	}

	return TrailEffect;
}

void UCombatVFXComponent::UpdateWeaponHeatVisuals(UWeaponComponent* WeaponComponent, float HeatPercentage)
{
	if (!WeaponComponent || !CombatVFXData)
	{
		return;
	}

	// Get or create material instance for this weapon
	UMaterialInstanceDynamic* HeatMaterial = nullptr;
	if (WeaponHeatMaterials.Contains(WeaponComponent))
	{
		HeatMaterial = WeaponHeatMaterials[WeaponComponent];
	}
	else if (CombatVFXData->HeatVisuals.HeatGradientMaterial)
	{
		HeatMaterial = UMaterialInstanceDynamic::Create(CombatVFXData->HeatVisuals.HeatGradientMaterial, this);
		WeaponHeatMaterials.Add(WeaponComponent, HeatMaterial);
	}

	if (HeatMaterial)
	{
		// Update heat color gradient
		FLinearColor HeatColor = CombatVFXData->GetHeatColor(HeatPercentage);
		HeatMaterial->SetVectorParameterValue(FName("HeatColor"), HeatColor);
		HeatMaterial->SetScalarParameterValue(FName("HeatPercentage"), HeatPercentage);
		
		// Update shimmer intensity
		float ShimmerIntensity = HeatPercentage * CombatVFXData->HeatVisuals.MaxShimmerIntensity;
		HeatMaterial->SetScalarParameterValue(FName("ShimmerIntensity"), ShimmerIntensity);
	}

	// Play heat vent effects when cooling
	float PreviousHeat = PreviousHeatPercentages.Contains(WeaponComponent) ? PreviousHeatPercentages[WeaponComponent] : HeatPercentage;
	if (HeatPercentage > 0.5f && HeatPercentage < PreviousHeat)
	{
		FVector VentLocation = WeaponComponent->GetWeaponWorldPosition();
		PlayHeatVentEffect(VentLocation, HeatPercentage);
	}
	
	// Store current heat for next frame comparison
	PreviousHeatPercentages.Add(WeaponComponent, HeatPercentage);
}

void UCombatVFXComponent::PlayHeatVentEffect(FVector VentLocation, float Intensity)
{
	if (!CombatVFXData || !CombatVFXData->HeatVisuals.HeatVentEffect)
	{
		return;
	}

	UNiagaraComponent* VentEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CombatVFXData->HeatVisuals.HeatVentEffect,
		VentLocation
	);

	if (VentEffect)
	{
		VentEffect->SetFloatParameter(FName("Intensity"), Intensity);
		ActiveEffects.Add(VentEffect);
		CurrentActiveEffects = ActiveEffects.Num();
	}
}

void UCombatVFXComponent::SetWeaponOverheatWarning(UWeaponComponent* WeaponComponent, bool bOverheating)
{
	if (!WeaponComponent || !WeaponHeatMaterials.Contains(WeaponComponent))
	{
		return;
	}

	UMaterialInstanceDynamic* HeatMaterial = WeaponHeatMaterials[WeaponComponent];
	if (HeatMaterial)
	{
		HeatMaterial->SetScalarParameterValue(FName("OverheatWarning"), bOverheating ? 1.0f : 0.0f);
	}
}

void UCombatVFXComponent::PlayShieldImpact(FVector ImpactLocation, FVector ImpactNormal, float DamageAmount)
{
	if (!ImpactEffectData)
	{
		return;
	}

	// Play ripple effect
	if (ImpactEffectData->ShieldImpact.RippleEffect)
	{
		UNiagaraComponent* RippleEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactEffectData->ShieldImpact.RippleEffect,
			ImpactLocation,
			ImpactNormal.Rotation()
		);

		if (RippleEffect)
		{
			FLinearColor ImpactColor = ImpactEffectData->CalculateShieldImpactColor(DamageAmount);
			RippleEffect->SetColorParameter(FName("RippleColor"), ImpactColor);
			RippleEffect->SetFloatParameter(FName("RippleSpeed"), ImpactEffectData->ShieldImpact.RippleSpeed);
			RippleEffect->SetFloatParameter(FName("MaxRadius"), ImpactEffectData->ShieldImpact.MaxRippleRadius);
			
			ActiveEffects.Add(RippleEffect);
		}
	}

	// Play dissipation effect
	if (ImpactEffectData->ShieldImpact.DissipationEffect)
	{
		UNiagaraComponent* DissipationEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactEffectData->ShieldImpact.DissipationEffect,
			ImpactLocation,
			ImpactNormal.Rotation()
		);

		if (DissipationEffect)
		{
			DissipationEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->ShieldImpact.DissipationIntensity);
			ActiveEffects.Add(DissipationEffect);
		}
	}

	// Play sound
	if (ImpactEffectData->ShieldImpact.ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEffectData->ShieldImpact.ImpactSound, ImpactLocation);
	}

	// Trigger Blueprint event
	OnImpact(true, ImpactLocation);

	CurrentActiveEffects = ActiveEffects.Num();
}

void UCombatVFXComponent::PlayHullImpact(FVector ImpactLocation, FVector ImpactNormal, UPrimitiveComponent* SurfaceComponent)
{
	if (!ImpactEffectData)
	{
		return;
	}

	// Play spark effect
	if (ImpactEffectData->HullImpact.SparkEffect)
	{
		UNiagaraComponent* SparkEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactEffectData->HullImpact.SparkEffect,
			ImpactLocation,
			ImpactNormal.Rotation()
		);

		if (SparkEffect)
		{
			SparkEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->HullImpact.SparkIntensity);
			ActiveEffects.Add(SparkEffect);
		}
	}

	// Play debris effect
	if (ImpactEffectData->HullImpact.DebrisEffect)
	{
		UNiagaraComponent* DebrisEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactEffectData->HullImpact.DebrisEffect,
			ImpactLocation,
			ImpactNormal.Rotation()
		);

		if (DebrisEffect)
		{
			DebrisEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->HullImpact.DebrisIntensity);
			ActiveEffects.Add(DebrisEffect);
		}
	}

	// Apply damage decal
	if (ImpactEffectData->HullImpact.DamageDecal && SurfaceComponent)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
			ImpactEffectData->HullImpact.DamageDecal,
			FVector(ImpactEffectData->HullImpact.DecalSize),
			SurfaceComponent,
			NAME_None,
			ImpactLocation,
			ImpactNormal.Rotation(),
			EAttachLocation::KeepWorldPosition
		);

		if (Decal && ImpactEffectData->HullImpact.DecalLifetime > 0.0f)
		{
			Decal->SetFadeScreenSize(0.001f);
			Decal->SetFadeOut(ImpactEffectData->HullImpact.DecalLifetime, 1.0f, false);
		}
	}

	// Play sound
	if (ImpactEffectData->HullImpact.ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactEffectData->HullImpact.ImpactSound, ImpactLocation);
	}

	// Trigger Blueprint event
	OnImpact(false, ImpactLocation);

	CurrentActiveEffects = ActiveEffects.Num();
}

void UCombatVFXComponent::PlayExplosion(FVector ExplosionLocation, EExplosionSize ExplosionSize, float ExplosionRadius)
{
	if (!ImpactEffectData)
	{
		return;
	}

	FExplosionEffect ExplosionConfig = ImpactEffectData->GetExplosionBySize(ExplosionSize);
	
	if (ExplosionRadius <= 0.0f)
	{
		ExplosionRadius = ExplosionConfig.ExplosionRadius;
	}

	// Play main explosion
	if (ExplosionConfig.ExplosionEffect)
	{
		UNiagaraComponent* Explosion = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionConfig.ExplosionEffect,
			ExplosionLocation
		);

		if (Explosion)
		{
			Explosion->SetFloatParameter(FName("ExplosionRadius"), ExplosionRadius);
			Explosion->SetFloatParameter(FName("FlashIntensity"), ExplosionConfig.FlashIntensity);
			ActiveEffects.Add(Explosion);
		}
	}

	// Play shockwave
	if (ExplosionConfig.ShockwaveEffect)
	{
		UNiagaraComponent* Shockwave = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionConfig.ShockwaveEffect,
			ExplosionLocation
		);

		if (Shockwave)
		{
			Shockwave->SetFloatParameter(FName("ShockwaveSpeed"), ExplosionConfig.ShockwaveSpeed);
			Shockwave->SetFloatParameter(FName("MaxRadius"), ExplosionRadius);
			ActiveEffects.Add(Shockwave);
		}
	}

	// Play fireball for large explosions
	if (ExplosionSize == EExplosionSize::Large && ExplosionConfig.FireballEffect)
	{
		UNiagaraComponent* Fireball = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionConfig.FireballEffect,
			ExplosionLocation
		);

		if (Fireball)
		{
			Fireball->SetFloatParameter(FName("FireballRadius"), ExplosionRadius * 0.8f);
			ActiveEffects.Add(Fireball);
		}
	}

	// Play sound
	if (ExplosionConfig.ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionConfig.ExplosionSound, ExplosionLocation);
	}

	// Apply camera shake
	if (ExplosionConfig.CameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			ExplosionConfig.CameraShake,
			ExplosionLocation,
			ExplosionRadius * 0.5f,  // Inner radius
			ExplosionRadius * 2.0f   // Outer radius
		);
	}

	CurrentActiveEffects = ActiveEffects.Num();
}

bool UCombatVFXComponent::CanSpawnEffect(int32 EffectPriority) const
{
	// Always allow high priority effects
	if (EffectPriority >= 8)
	{
		return true;
	}

	// Check if we're at capacity
	return CurrentActiveEffects < MaxActiveEffects;
}

void UCombatVFXComponent::GetPerformanceMetrics(int32& OutActiveEffects, int32& OutTotalParticles, float& OutFrameTime) const
{
	OutActiveEffects = CurrentActiveEffects;
	
	// Approximate particle count
	OutTotalParticles = 0;
	for (UNiagaraComponent* Effect : ActiveEffects)
	{
		if (Effect && Effect->IsActive())
		{
			OutTotalParticles += 100;  // Rough estimate per effect
		}
	}

	// Frame time would be measured by profiler in real implementation
	OutFrameTime = 0.0f;
}

void UCombatVFXComponent::CleanupFinishedEffects()
{
	// Remove inactive effects from tracking
	ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
	{
		return Effect == nullptr || !Effect->IsActive();
	});

	CurrentActiveEffects = ActiveEffects.Num();
}

bool UCombatVFXComponent::IsWithinRenderDistance(FVector Location) const
{
	if (!bEnableDistanceCulling)
	{
		return true;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return true;  // Can't check distance, allow spawn
	}

	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
	return Distance <= MaxEffectDistance;
}

void UCombatVFXComponent::OnWeaponFired_Implementation(EWeaponType WeaponType, FVector MuzzleLocation)
{
	// Default implementation - can be overridden in Blueprint
}

void UCombatVFXComponent::OnImpact_Implementation(bool bShieldImpact, FVector ImpactLocation)
{
	// Default implementation - can be overridden in Blueprint
}
