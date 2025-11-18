#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatVFXDataAsset.h"
#include "Combat/ImpactEffectDataAsset.h"
#include "Combat/WeaponComponent.h"
#include "NiagaraComponent.h"
#include "CombatVFXComponent.generated.h"

/**
 * Combat VFX Component
 * 
 * Manages all combat visual effects for spaceships including:
 * - Weapon fire effects (6 weapon types)
 * - Weapon heat management visuals
 * - Impact effects (shield and hull)
 * - Explosion effects (small/medium/large)
 * 
 * **Weapon Fire Effects:**
 * - Energy weapons: Laser beam with glow and heat distortion
 * - Projectile weapons: Tracer rounds with muzzle flash
 * - Missile weapons: Smoke trail and engine glow
 * - Torpedo weapons: Plasma trail effect
 * - Beam weapons: Sustained beam with start/end points
 * - Plasma weapons: Ball lightning with arcing
 * 
 * **Integration:**
 * - Automatically monitors WeaponComponent for fire events
 * - Spawns appropriate effects based on weapon type
 * - Manages effect lifecycle and performance
 * 
 * **Performance:**
 * - Particle budget management (<5ms per effect target)
 * - Distance-based LOD and culling
 * - Effect pooling for frequently used effects
 * 
 * Usage:
 * - Add component to spaceship Blueprint
 * - Assign CombatVFXDataAsset for weapon effects
 * - Assign ImpactEffectDataAsset for impact effects
 * - Component automatically handles all visual feedback
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UCombatVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatVFXComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// CONFIGURATION
	// ====================

	/** Combat VFX data asset defining weapon effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat VFX")
	UCombatVFXDataAsset* CombatVFXData;

	/** Impact effect data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat VFX")
	UImpactEffectDataAsset* ImpactEffectData;

	/** Weapon components to monitor (auto-detected if empty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat VFX")
	TArray<UWeaponComponent*> WeaponComponents;

	// ====================
	// WEAPON FIRE EFFECTS
	// ====================

	/**
	 * Play weapon fire effect
	 * @param WeaponType Type of weapon firing
	 * @param MuzzleLocation World location of weapon muzzle
	 * @param MuzzleRotation Rotation of weapon muzzle
	 * @param TargetLocation Optional target location for beam effects
	 * @return Spawned effect component (nullptr if failed)
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	UNiagaraComponent* PlayWeaponFireEffect(
		EWeaponType WeaponType,
		FVector MuzzleLocation,
		FRotator MuzzleRotation,
		FVector TargetLocation = FVector::ZeroVector
	);

	/**
	 * Play muzzle flash effect
	 * @param MuzzleLocation World location of weapon muzzle
	 * @param MuzzleRotation Rotation of weapon muzzle
	 * @return Spawned effect component
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	UNiagaraComponent* PlayMuzzleFlash(FVector MuzzleLocation, FRotator MuzzleRotation);

	/**
	 * Play beam effect (laser, sustained beam)
	 * @param StartLocation Beam start point
	 * @param EndLocation Beam end point
	 * @param BeamWidth Width of beam in cm
	 * @param BeamColor Color of beam
	 * @param Duration Duration of beam effect
	 * @return Spawned effect component
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	UNiagaraComponent* PlayBeamEffect(
		FVector StartLocation,
		FVector EndLocation,
		float BeamWidth = 5.0f,
		FLinearColor BeamColor = FLinearColor::White,
		float Duration = 0.5f
	);

	/**
	 * Play projectile trail effect (tracer, missile trail, plasma)
	 * @param SpawnLocation Starting location
	 * @param SpawnRotation Starting rotation
	 * @param ProjectileVelocity Velocity vector
	 * @param WeaponType Type of weapon for appropriate trail
	 * @return Spawned effect component
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	UNiagaraComponent* PlayProjectileTrail(
		FVector SpawnLocation,
		FRotator SpawnRotation,
		FVector ProjectileVelocity,
		EWeaponType WeaponType
	);

	// ====================
	// WEAPON HEAT EFFECTS
	// ====================

	/**
	 * Update weapon heat visual feedback
	 * @param WeaponComponent Weapon to update
	 * @param HeatPercentage Current heat level (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void UpdateWeaponHeatVisuals(UWeaponComponent* WeaponComponent, float HeatPercentage);

	/**
	 * Play heat vent effect (steam/shimmer)
	 * @param VentLocation Location of cooling vent
	 * @param Intensity Effect intensity
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void PlayHeatVentEffect(FVector VentLocation, float Intensity = 1.0f);

	/**
	 * Set weapon overheat warning state
	 * @param WeaponComponent Weapon to update
	 * @param bOverheating Whether weapon is overheating
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void SetWeaponOverheatWarning(UWeaponComponent* WeaponComponent, bool bOverheating);

	// ====================
	// IMPACT EFFECTS
	// ====================

	/**
	 * Play shield impact effect
	 * @param ImpactLocation World location of impact
	 * @param ImpactNormal Surface normal at impact point
	 * @param DamageAmount Amount of damage for color intensity
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void PlayShieldImpact(FVector ImpactLocation, FVector ImpactNormal, float DamageAmount);

	/**
	 * Play hull impact effect
	 * @param ImpactLocation World location of impact
	 * @param ImpactNormal Surface normal at impact point
	 * @param SurfaceComponent Component that was hit (for decals)
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void PlayHullImpact(FVector ImpactLocation, FVector ImpactNormal, UPrimitiveComponent* SurfaceComponent);

	/**
	 * Play explosion effect
	 * @param ExplosionLocation World location of explosion
	 * @param ExplosionSize Size category of explosion
	 * @param ExplosionRadius Radius of explosion effect
	 */
	UFUNCTION(BlueprintCallable, Category="Combat VFX")
	void PlayExplosion(FVector ExplosionLocation, EExplosionSize ExplosionSize, float ExplosionRadius = 0.0f);

	// ====================
	// PERFORMANCE MANAGEMENT
	// ====================

	/** Maximum active effects allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance", meta=(ClampMin="10", ClampMax="100"))
	int32 MaxActiveEffects;

	/** Current number of active effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Performance")
	int32 CurrentActiveEffects;

	/** Enable distance-based culling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance")
	bool bEnableDistanceCulling;

	/** Maximum distance for effects (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance", meta=(ClampMin="1000", ClampMax="50000"))
	float MaxEffectDistance;

	/**
	 * Check if we can spawn new effect (performance budget)
	 * @param EffectPriority Priority of effect to spawn
	 * @return True if effect can be spawned
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Performance")
	bool CanSpawnEffect(int32 EffectPriority = 5) const;

	/**
	 * Get current performance metrics
	 * @param OutActiveEffects Number of active effects
	 * @param OutTotalParticles Approximate total particles
	 * @param OutFrameTime Frame time in ms
	 */
	UFUNCTION(BlueprintCallable, Category="Performance")
	void GetPerformanceMetrics(int32& OutActiveEffects, int32& OutTotalParticles, float& OutFrameTime) const;

	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	/**
	 * Called when weapon fires
	 * Override in Blueprint for custom effects
	 * @param WeaponType Type of weapon that fired
	 * @param MuzzleLocation Location of muzzle
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Combat VFX")
	void OnWeaponFired(EWeaponType WeaponType, FVector MuzzleLocation);

	/**
	 * Called when impact occurs
	 * Override in Blueprint for custom reactions
	 * @param bShieldImpact True if hit shield, false if hit hull
	 * @param ImpactLocation Location of impact
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Combat VFX")
	void OnImpact(bool bShieldImpact, FVector ImpactLocation);

protected:
	/**
	 * Register weapon component for monitoring
	 * @param WeaponComponent Weapon to monitor
	 */
	void RegisterWeaponComponent(UWeaponComponent* WeaponComponent);

	/**
	 * Handle weapon fired event
	 * @param WeaponComponent Weapon that fired
	 * @param Target Target actor (may be nullptr)
	 */
	UFUNCTION()
	void HandleWeaponFired(UWeaponComponent* WeaponComponent, AActor* Target);

	/**
	 * Cleanup finished effects
	 */
	void CleanupFinishedEffects();

	/**
	 * Check if location is within render distance
	 * @param Location Location to check
	 * @return True if within render distance
	 */
	bool IsWithinRenderDistance(FVector Location) const;

	/** Active effect components */
	UPROPERTY()
	TArray<UNiagaraComponent*> ActiveEffects;

	/** Map of weapon components to their heat materials */
	UPROPERTY()
	TMap<UWeaponComponent*, UMaterialInstanceDynamic*> WeaponHeatMaterials;
};
