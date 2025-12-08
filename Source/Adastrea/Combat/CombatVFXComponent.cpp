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
	ComponentPoolSize = 20;  // Phase 2: Object pooling

	// Set component defaults
	CombatVFXData = nullptr;
	ImpactEffectData = nullptr;
}

void UCombatVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	// Phase 2: Initialize component pool for performance
	InitializeComponentPool();

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

	// Phase 2: Use pooled component instead of spawning new
	UNiagaraComponent* FlashEffect = GetPooledNiagaraComponent();
	if (!FlashEffect)
	{
		return nullptr;
	}

	// Configure pooled component
	FlashEffect->SetAsset(CombatVFXData->FireEffect.MuzzleFlashEffect);
	FlashEffect->SetWorldLocation(MuzzleLocation);
	FlashEffect->SetWorldRotation(MuzzleRotation);
	FlashEffect->SetFloatParameter(FName("Intensity"), CombatVFXData->FireEffect.GlowIntensity);
	FlashEffect->Activate();
	FlashEffect->SetVisibility(true);

	ActiveEffects.Add(FlashEffect);
	CurrentActiveEffects = ActiveEffects.Num();

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

	// Phase 2: Use pooled component instead of spawning new
	UNiagaraComponent* BeamEffect = GetPooledNiagaraComponent();
	if (!BeamEffect)
	{
		return nullptr;
	}

	// Configure pooled component
	BeamEffect->SetAsset(CombatVFXData->FireEffect.BeamEffect);
	BeamEffect->SetWorldLocation(StartLocation);
	BeamEffect->SetWorldRotation((EndLocation - StartLocation).Rotation());
	BeamEffect->SetVectorParameter(FName("BeamStart"), StartLocation);
	BeamEffect->SetVectorParameter(FName("BeamEnd"), EndLocation);
	BeamEffect->SetFloatParameter(FName("BeamWidth"), BeamWidth);
	BeamEffect->SetColorParameter(FName("BeamColor"), BeamColor);
	BeamEffect->SetFloatParameter(FName("Duration"), Duration);
	BeamEffect->SetFloatParameter(FName("HeatDistortion"), CombatVFXData->FireEffect.HeatDistortionStrength);
	BeamEffect->Activate();
	BeamEffect->SetVisibility(true);

	ActiveEffects.Add(BeamEffect);
	CurrentActiveEffects = ActiveEffects.Num();

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

	// Phase 2: Use pooled component instead of spawning new
	UNiagaraComponent* TrailEffect = GetPooledNiagaraComponent();
	if (!TrailEffect)
	{
		return nullptr;
	}

	// Configure pooled component
	TrailEffect->SetAsset(CombatVFXData->FireEffect.ProjectileTrailEffect);
	TrailEffect->SetWorldLocation(SpawnLocation);
	TrailEffect->SetWorldRotation(SpawnRotation);
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

	TrailEffect->Activate();
	TrailEffect->SetVisibility(true);

	ActiveEffects.Add(TrailEffect);
	CurrentActiveEffects = ActiveEffects.Num();

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

	// Play ripple effect - Phase 2: Use pooling
	if (ImpactEffectData->ShieldImpact.RippleEffect)
	{
		UNiagaraComponent* RippleEffect = GetPooledNiagaraComponent();
		if (RippleEffect)
		{
			RippleEffect->SetAsset(ImpactEffectData->ShieldImpact.RippleEffect);
			RippleEffect->SetWorldLocation(ImpactLocation);
			RippleEffect->SetWorldRotation(ImpactNormal.Rotation());
			
			FLinearColor ImpactColor = ImpactEffectData->CalculateShieldImpactColor(DamageAmount);
			RippleEffect->SetColorParameter(FName("RippleColor"), ImpactColor);
			RippleEffect->SetFloatParameter(FName("RippleSpeed"), ImpactEffectData->ShieldImpact.RippleSpeed);
			RippleEffect->SetFloatParameter(FName("MaxRadius"), ImpactEffectData->ShieldImpact.MaxRippleRadius);
			RippleEffect->Activate();
			RippleEffect->SetVisibility(true);
			
			ActiveEffects.Add(RippleEffect);
		}
	}

	// Play dissipation effect - Phase 2: Use pooling
	if (ImpactEffectData->ShieldImpact.DissipationEffect)
	{
		UNiagaraComponent* DissipationEffect = GetPooledNiagaraComponent();
		if (DissipationEffect)
		{
			DissipationEffect->SetAsset(ImpactEffectData->ShieldImpact.DissipationEffect);
			DissipationEffect->SetWorldLocation(ImpactLocation);
			DissipationEffect->SetWorldRotation(ImpactNormal.Rotation());
			DissipationEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->ShieldImpact.DissipationIntensity);
			DissipationEffect->Activate();
			DissipationEffect->SetVisibility(true);
			
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

	// Play spark effect - Phase 2: Use pooling
	if (ImpactEffectData->HullImpact.SparkEffect)
	{
		UNiagaraComponent* SparkEffect = GetPooledNiagaraComponent();
		if (SparkEffect)
		{
			SparkEffect->SetAsset(ImpactEffectData->HullImpact.SparkEffect);
			SparkEffect->SetWorldLocation(ImpactLocation);
			SparkEffect->SetWorldRotation(ImpactNormal.Rotation());
			SparkEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->HullImpact.SparkIntensity);
			SparkEffect->Activate();
			SparkEffect->SetVisibility(true);
			
			ActiveEffects.Add(SparkEffect);
		}
	}

	// Play debris effect - Phase 2: Use pooling
	if (ImpactEffectData->HullImpact.DebrisEffect)
	{
		UNiagaraComponent* DebrisEffect = GetPooledNiagaraComponent();
		if (DebrisEffect)
		{
			DebrisEffect->SetAsset(ImpactEffectData->HullImpact.DebrisEffect);
			DebrisEffect->SetWorldLocation(ImpactLocation);
			DebrisEffect->SetWorldRotation(ImpactNormal.Rotation());
			DebrisEffect->SetFloatParameter(FName("Intensity"), ImpactEffectData->HullImpact.DebrisIntensity);
			DebrisEffect->Activate();
			DebrisEffect->SetVisibility(true);
			
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

	// Play main explosion - Phase 2: Use pooling
	if (ExplosionConfig.ExplosionEffect)
	{
		UNiagaraComponent* Explosion = GetPooledNiagaraComponent();
		if (Explosion)
		{
			Explosion->SetAsset(ExplosionConfig.ExplosionEffect);
			Explosion->SetWorldLocation(ExplosionLocation);
			Explosion->SetFloatParameter(FName("ExplosionRadius"), ExplosionRadius);
			Explosion->SetFloatParameter(FName("FlashIntensity"), ExplosionConfig.FlashIntensity);
			Explosion->Activate();
			Explosion->SetVisibility(true);
			
			ActiveEffects.Add(Explosion);
		}
	}

	// Play shockwave - Phase 2: Use pooling
	if (ExplosionConfig.ShockwaveEffect)
	{
		UNiagaraComponent* Shockwave = GetPooledNiagaraComponent();
		if (Shockwave)
		{
			Shockwave->SetAsset(ExplosionConfig.ShockwaveEffect);
			Shockwave->SetWorldLocation(ExplosionLocation);
			Shockwave->SetFloatParameter(FName("ShockwaveSpeed"), ExplosionConfig.ShockwaveSpeed);
			Shockwave->SetFloatParameter(FName("MaxRadius"), ExplosionRadius);
			Shockwave->Activate();
			Shockwave->SetVisibility(true);
			
			ActiveEffects.Add(Shockwave);
		}
	}

	// Play fireball for large explosions - Phase 2: Use pooling
	if (ExplosionSize == EExplosionSize::Large && ExplosionConfig.FireballEffect)
	{
		UNiagaraComponent* Fireball = GetPooledNiagaraComponent();
		if (Fireball)
		{
			Fireball->SetAsset(ExplosionConfig.FireballEffect);
			Fireball->SetWorldLocation(ExplosionLocation);
			Fireball->SetFloatParameter(FName("FireballRadius"), ExplosionRadius * 0.8f);
			Fireball->Activate();
			Fireball->SetVisibility(true);
			
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
	// Phase 2: Return finished effects to pool instead of destroying
	// Iterate backwards for safe removal
	for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; i--)
	{
		UNiagaraComponent* Component = ActiveNiagaraComponents[i];
		if (Component && !Component->IsActive())
		{
			ReturnNiagaraComponentToPool(Component);
		}
	}

	// Also cleanup legacy ActiveEffects array
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

// ====================
// Object Pooling Implementation (Phase 2 Optimization)
// ====================

void UCombatVFXComponent::InitializeComponentPool()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("CombatVFXComponent: Cannot initialize pool - no owner"));
		return;
	}

	// Pre-allocate pool for better memory management
	NiagaraComponentPool.Reserve(ComponentPoolSize);
	ActiveNiagaraComponents.Reserve(ComponentPoolSize);

	// Create pooled components
	for (int32 i = 0; i < ComponentPoolSize; i++)
	{
		UNiagaraComponent* Component = NewObject<UNiagaraComponent>(Owner);
		if (Component)
		{
			Component->RegisterComponent();
			Component->SetAutoActivate(false);
			Component->SetVisibility(false);
			NiagaraComponentPool.Add(Component);
		}
	}

	UE_LOG(LogAdastreaCombat, Log, TEXT("CombatVFXComponent: Initialized Niagara pool with %d components"), ComponentPoolSize);
}

UNiagaraComponent* UCombatVFXComponent::GetPooledNiagaraComponent()
{
	// Try to get from pool
	if (NiagaraComponentPool.Num() > 0)
	{
		UNiagaraComponent* Component = NiagaraComponentPool.Pop();
		ActiveNiagaraComponents.Add(Component);
		return Component;
	}

	// Pool exhausted - create new component if owner is valid
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogAdastreaCombat, Error, TEXT("CombatVFXComponent: Cannot create component - no owner"));
		return nullptr;
	}

	// Log pool exhaustion (use VeryVerbose to avoid spam)
	UE_LOG(LogAdastreaCombat, VeryVerbose, 
		TEXT("CombatVFXComponent: Niagara pool exhausted, creating new component (pool size: %d)"), 
		ComponentPoolSize);

	UNiagaraComponent* Component = NewObject<UNiagaraComponent>(Owner);
	if (Component)
	{
		Component->RegisterComponent();
		ActiveNiagaraComponents.Add(Component);
	}
	return Component;
}

void UCombatVFXComponent::ReturnNiagaraComponentToPool(UNiagaraComponent* Component)
{
	if (!Component)
	{
		return;
	}

	// Deactivate and reset component
	Component->Deactivate();
	Component->SetVisibility(false);
	Component->ResetSystem();

	// Remove from active list
	ActiveNiagaraComponents.Remove(Component);

	// Return to pool if space available
	if (NiagaraComponentPool.Num() < ComponentPoolSize)
	{
		NiagaraComponentPool.Add(Component);
	}
	else
	{
		// Pool full - destroy excess components
		Component->DestroyComponent();
	}
}

// ====================
// Blueprint Event Implementations
// ====================

void UCombatVFXComponent::OnWeaponFired_Implementation(EWeaponType WeaponType, FVector MuzzleLocation)
{
	// Default implementation - can be overridden in Blueprint
}

void UCombatVFXComponent::OnImpact_Implementation(bool bShieldImpact, FVector ImpactLocation)
{
	// Default implementation - can be overridden in Blueprint
}
