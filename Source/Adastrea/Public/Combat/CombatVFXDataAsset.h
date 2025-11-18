#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/WeaponDataAsset.h"
#include "NiagaraSystem.h"
#include "CombatVFXDataAsset.generated.h"

/**
 * Enum for VFX attachment points on weapons
 */
UENUM(BlueprintType)
enum class EVFXAttachmentPoint : uint8
{
	MuzzleFlash     UMETA(DisplayName = "Muzzle Flash"),
	BarrelTip       UMETA(DisplayName = "Barrel Tip"),
	WeaponBody      UMETA(DisplayName = "Weapon Body"),
	HeatVent        UMETA(DisplayName = "Heat Vent")
};

/**
 * Enum for beam effect types
 */
UENUM(BlueprintType)
enum class EBeamEffectType : uint8
{
	Instant         UMETA(DisplayName = "Instant Hit"),
	Sustained       UMETA(DisplayName = "Sustained Beam"),
	Projectile      UMETA(DisplayName = "Projectile")
};

/**
 * Struct for weapon fire effect configuration
 */
USTRUCT(BlueprintType)
struct FWeaponFireEffect
{
	GENERATED_BODY()

	/** Niagara system for muzzle flash */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	UNiagaraSystem* MuzzleFlashEffect;

	/** Niagara system for beam/tracer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	UNiagaraSystem* BeamEffect;

	/** Niagara system for projectile trail */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	UNiagaraSystem* ProjectileTrailEffect;

	/** Sound to play when firing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	USoundBase* FireSound;

	/** Beam effect type (instant, sustained, projectile) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	EBeamEffectType BeamType;

	/** Duration for sustained beams in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect", meta=(ClampMin="0.1", ClampMax="10.0"))
	float BeamDuration;

	/** Beam width in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect", meta=(ClampMin="1", ClampMax="100"))
	float BeamWidth;

	/** Beam color */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	FLinearColor BeamColor;

	/** Glow intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect", meta=(ClampMin="0.1", ClampMax="10.0"))
	float GlowIntensity;

	/** Heat distortion strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HeatDistortionStrength;

	/** Camera shake to apply on fire */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effect")
	TSubclassOf<UCameraShakeBase> CameraShake;

	FWeaponFireEffect()
		: MuzzleFlashEffect(nullptr)
		, BeamEffect(nullptr)
		, ProjectileTrailEffect(nullptr)
		, FireSound(nullptr)
		, BeamType(EBeamEffectType::Instant)
		, BeamDuration(0.5f)
		, BeamWidth(5.0f)
		, BeamColor(FLinearColor::White)
		, GlowIntensity(1.0f)
		, HeatDistortionStrength(0.3f)
		, CameraShake(nullptr)
	{}
};

/**
 * Struct for weapon heat visual configuration
 */
USTRUCT(BlueprintType)
struct FWeaponHeatVisuals
{
	GENERATED_BODY()

	/** Heat vent particle effect (steam/shimmer) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat")
	UNiagaraSystem* HeatVentEffect;

	/** Material for weapon heat gradient */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat")
	UMaterialInterface* HeatGradientMaterial;

	/** Cool color (0% heat) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat")
	FLinearColor CoolColor;

	/** Warning color (50% heat) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat")
	FLinearColor WarningColor;

	/** Overheat color (100% heat) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat")
	FLinearColor OverheatColor;

	/** Heat shimmer intensity at max heat */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxShimmerIntensity;

	FWeaponHeatVisuals()
		: HeatVentEffect(nullptr)
		, HeatGradientMaterial(nullptr)
		, CoolColor(FLinearColor(0.1f, 0.3f, 0.5f, 1.0f))  // Blue
		, WarningColor(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f))  // Orange
		, OverheatColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f))  // Red
		, MaxShimmerIntensity(0.8f)
	{}
};

/**
 * Data Asset for Combat Visual Effects
 * 
 * Defines visual effects for weapon fire, including:
 * - Muzzle flashes and beam effects
 * - Projectile trails
 * - Heat management visuals
 * - Camera shake and audio integration
 * 
 * Supports all 6 weapon types:
 * - Energy weapons: Laser beam with glow and heat distortion
 * - Projectile weapons: Tracer rounds with muzzle flash
 * - Missile weapons: Smoke trail and engine glow
 * - Torpedo weapons: Plasma trail effect
 * - Beam weapons: Sustained beam with start/end points
 * - Plasma weapons: Ball lightning with arcing
 * 
 * Usage:
 * - Create a Blueprint based on this class
 * - Configure fire effects for weapon type
 * - Assign to CombatVFXComponent on ships
 * - Effects will be automatically played when weapon fires
 */
UCLASS(BlueprintType)
class ADASTREA_API UCombatVFXDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UCombatVFXDataAsset();

	// ====================
	// BASIC INFO
	// ====================

	/** Display name of the VFX configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText VFXName;

	/** Brief description of this VFX configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	/** Weapon type this VFX is designed for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	EWeaponType WeaponType;

	// ====================
	// WEAPON FIRE EFFECTS
	// ====================

	/** Weapon fire effect configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fire Effects")
	FWeaponFireEffect FireEffect;

	// ====================
	// HEAT MANAGEMENT VISUALS
	// ====================

	/** Weapon heat visual configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Heat Visuals")
	FWeaponHeatVisuals HeatVisuals;

	// ====================
	// PERFORMANCE SETTINGS
	// ====================

	/** Maximum particles for this effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Performance", meta=(ClampMin="10", ClampMax="5000"))
	int32 MaxParticles;

	/** Effect priority (higher = more important, won't be culled first) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Performance", meta=(ClampMin="0", ClampMax="10"))
	int32 EffectPriority;

	/** Maximum render distance before culling (in meters) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Performance", meta=(ClampMin="100", ClampMax="50000"))
	float MaxRenderDistance;

	// ====================
	// BLUEPRINT CALLABLE FUNCTIONS
	// ====================

	/**
	 * Get the appropriate beam color based on weapon type
	 * @return Beam color
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat VFX")
	FLinearColor GetBeamColor() const;

	/**
	 * Get heat color based on current heat percentage
	 * @param HeatPercentage Current heat level (0-1)
	 * @return Interpolated heat color
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat VFX")
	FLinearColor GetHeatColor(float HeatPercentage) const;

	/**
	 * Check if this VFX configuration is suitable for a weapon type
	 * @param Type The weapon type to check
	 * @return True if compatible
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Combat VFX")
	bool IsCompatibleWithWeaponType(EWeaponType Type) const;
};
