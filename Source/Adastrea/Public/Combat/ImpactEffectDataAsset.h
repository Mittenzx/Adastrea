#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "ImpactEffectDataAsset.generated.h"

/**
 * Enum for explosion sizes
 */
UENUM(BlueprintType)
enum class EExplosionSize : uint8
{
	Small       UMETA(DisplayName = "Small"),
	Medium      UMETA(DisplayName = "Medium"),
	Large       UMETA(DisplayName = "Large")
};

/**
 * Struct for shield impact effect configuration
 */
USTRUCT(BlueprintType)
struct FShieldImpactEffect
{
	GENERATED_BODY()

	/** Hexagonal ripple effect on shield surface */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact")
	UNiagaraSystem* RippleEffect;

	/** Energy dissipation particles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact")
	UNiagaraSystem* DissipationEffect;

	/** Shield flash material (dynamic) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact")
	UMaterialInterface* ShieldFlashMaterial;

	/** Impact sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact")
	USoundBase* ImpactSound;

	/** Ripple expansion speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact", meta=(ClampMin="100", ClampMax="5000"))
	float RippleSpeed;

	/** Ripple duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact", meta=(ClampMin="0.1", ClampMax="5.0"))
	float RippleDuration;

	/** Ripple color based on damage type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact")
	FLinearColor RippleColor;

	/** Maximum ripple radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact", meta=(ClampMin="10", ClampMax="1000"))
	float MaxRippleRadius;

	/** Energy dissipation intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Impact", meta=(ClampMin="0.1", ClampMax="10.0"))
	float DissipationIntensity;

	FShieldImpactEffect()
		: RippleEffect(nullptr)
		, DissipationEffect(nullptr)
		, ShieldFlashMaterial(nullptr)
		, ImpactSound(nullptr)
		, RippleSpeed(1000.0f)
		, RippleDuration(1.0f)
		, RippleColor(FLinearColor::Blue)
		, MaxRippleRadius(200.0f)
		, DissipationIntensity(1.0f)
	{}
};

/**
 * Struct for hull impact effect configuration
 */
USTRUCT(BlueprintType)
struct FHullImpactEffect
{
	GENERATED_BODY()

	/** Spark particles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact")
	UNiagaraSystem* SparkEffect;

	/** Debris particles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact")
	UNiagaraSystem* DebrisEffect;

	/** Damage decal material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact")
	UMaterialInterface* DamageDecal;

	/** Impact sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact")
	USoundBase* ImpactSound;

	/** Spark count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact", meta=(ClampMin="0.1", ClampMax="5.0"))
	float SparkIntensity;

	/** Debris count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact", meta=(ClampMin="0.1", ClampMax="5.0"))
	float DebrisIntensity;

	/** Decal size in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact", meta=(ClampMin="5", ClampMax="500"))
	float DecalSize;

	/** Decal lifetime in seconds (0 = permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact", meta=(ClampMin="0", ClampMax="300"))
	float DecalLifetime;

	/** Impact flash intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Impact", meta=(ClampMin="0.0", ClampMax="10.0"))
	float FlashIntensity;

	FHullImpactEffect()
		: SparkEffect(nullptr)
		, DebrisEffect(nullptr)
		, DamageDecal(nullptr)
		, ImpactSound(nullptr)
		, SparkIntensity(1.0f)
		, DebrisIntensity(1.0f)
		, DecalSize(50.0f)
		, DecalLifetime(60.0f)
		, FlashIntensity(1.0f)
	{}
};

/**
 * Struct for explosion effect configuration
 */
USTRUCT(BlueprintType)
struct FExplosionEffect
{
	GENERATED_BODY()

	/** Main explosion particle effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	UNiagaraSystem* ExplosionEffect;

	/** Shockwave effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	UNiagaraSystem* ShockwaveEffect;

	/** Fireball effect for large explosions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	UNiagaraSystem* FireballEffect;

	/** Explosion sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	USoundBase* ExplosionSound;

	/** Explosion size category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	EExplosionSize ExplosionSize;

	/** Explosion radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="10", ClampMax="10000"))
	float ExplosionRadius;

	/** Flash intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.1", ClampMax="20.0"))
	float FlashIntensity;

	/** Camera shake class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion")
	TSubclassOf<UCameraShakeBase> CameraShake;

	/** Shockwave expansion speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="100", ClampMax="10000"))
	float ShockwaveSpeed;

	FExplosionEffect()
		: ExplosionEffect(nullptr)
		, ShockwaveEffect(nullptr)
		, FireballEffect(nullptr)
		, ExplosionSound(nullptr)
		, ExplosionSize(EExplosionSize::Medium)
		, ExplosionRadius(500.0f)
		, FlashIntensity(5.0f)
		, CameraShake(nullptr)
		, ShockwaveSpeed(2000.0f)
	{}
};

/**
 * Data Asset for Impact and Explosion Effects
 * 
 * Defines visual effects for weapon impacts and explosions:
 * 
 * **Shield Impacts:**
 * - Hexagonal ripple pattern with energy dissipation
 * - Damage-type specific colors
 * - Dynamic shield flash materials
 * 
 * **Hull Impacts:**
 * - Spark and debris particles
 * - Damage decals (permanent or temporary)
 * - Impact flash effects
 * 
 * **Explosions:**
 * - Small/Medium/Large scale explosions
 * - Shockwave effects
 * - Camera shake integration
 * - Fireball for large explosions
 * 
 * Usage:
 * - Create a Blueprint based on this class
 * - Configure impact effects for different scenarios
 * - Assign to CombatVFXComponent
 * - Effects are triggered automatically on damage events
 */
UCLASS(BlueprintType)
class ADASTREA_API UImpactEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UImpactEffectDataAsset();

	// ====================
	// BASIC INFO
	// ====================

	/** Display name of the impact effect configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
	FText EffectName;

	/** Brief description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
	FText Description;

	// ====================
	// SHIELD IMPACT EFFECTS
	// ====================

	/** Shield impact effect configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield Effects")
	FShieldImpactEffect ShieldImpact;

	// ====================
	// HULL IMPACT EFFECTS
	// ====================

	/** Hull impact effect configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull Effects")
	FHullImpactEffect HullImpact;

	// ====================
	// EXPLOSION EFFECTS
	// ====================

	/** Small explosion configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosions")
	FExplosionEffect SmallExplosion;

	/** Medium explosion configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosions")
	FExplosionEffect MediumExplosion;

	/** Large explosion configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosions")
	FExplosionEffect LargeExplosion;

	// ====================
	// PERFORMANCE SETTINGS
	// ====================

	/** Maximum particles for all impact effects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Performance", meta=(ClampMin="10", ClampMax="10000"))
	int32 MaxParticles;

	/** Effect priority (higher = more important) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Performance", meta=(ClampMin="0", ClampMax="10"))
	int32 EffectPriority;

	// ====================
	// BLUEPRINT CALLABLE FUNCTIONS
	// ====================

	/**
	 * Get explosion effect by size
	 * @param Size The explosion size category
	 * @return Explosion effect configuration
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Impact Effects")
	FExplosionEffect GetExplosionBySize(EExplosionSize Size) const;

	/**
	 * Calculate shield impact color based on damage amount
	 * @param DamageAmount Amount of damage dealt
	 * @return Color for shield ripple effect
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Impact Effects")
	FLinearColor CalculateShieldImpactColor(float DamageAmount) const;
};
