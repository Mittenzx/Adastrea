#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SpaceshipParticleComponent.generated.h"

/**
 * Enum for engine damage states
 */
UENUM(BlueprintType)
enum class EEngineDamageState : uint8
{
	Normal      UMETA(DisplayName = "Normal"),      // Full operational
	Damaged     UMETA(DisplayName = "Damaged"),     // Sputtering, irregular
	Critical    UMETA(DisplayName = "Critical")     // Smoke, fire, emergency shutdown
};

/**
 * Enum for RCS thruster directions
 */
UENUM(BlueprintType)
enum class ERCSThrusterAxis : uint8
{
	Forward     UMETA(DisplayName = "Forward"),
	Backward    UMETA(DisplayName = "Backward"),
	Left        UMETA(DisplayName = "Left"),
	Right       UMETA(DisplayName = "Right"),
	Up          UMETA(DisplayName = "Up"),
	Down        UMETA(DisplayName = "Down")
};

/**
 * Spaceship Particle Component
 * 
 * Manages all particle effects for spaceship propulsion systems including:
 * - Main engine thrust plumes (throttle-responsive)
 * - RCS thruster puffs for attitude control
 * - Jump drive effects (charge, jump, exit)
 * - Engine damage state visuals
 * 
 * Usage:
 * - Add component to spaceship Blueprint
 * - Assign particle system templates in editor
 * - Call UpdateThrottle() to update engine intensity
 * - Call ActivateRCSThruster() for attitude control effects
 * - Call TriggerJumpSequence() for jump effects
 */
UCLASS(ClassGroup=(Ships), meta=(BlueprintSpawnableComponent))
class ADASTREA_API USpaceshipParticleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceshipParticleComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// MAIN ENGINE EFFECTS
	// ====================

	/** Main engine particle system template (GPU particles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Engine")
	UParticleSystem* MainEngineTemplate;

	/** Engine glow material instance for dynamic parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Engine")
	UMaterialInstanceDynamic* EngineMaterialInstance;

	/** Afterburner effect template for boost mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Engine")
	UParticleSystem* AfterburnerTemplate;

	/** Main engine particle component instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Main Engine")
	UParticleSystemComponent* MainEngineComponent;

	/** Afterburner particle component instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Main Engine")
	UParticleSystemComponent* AfterburnerComponent;

	/** Current throttle value (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Main Engine")
	float CurrentThrottle;

	/** Is boost/afterburner active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Main Engine")
	bool bBoostActive;

	/** Minimum particle spawn rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Engine", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinParticleRateMultiplier;

	/** Maximum particle spawn rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Engine", meta=(ClampMin="1.0", ClampMax="5.0"))
	float MaxParticleRateMultiplier;

	/**
	 * Update throttle intensity for main engine effects
	 * @param Throttle Normalized throttle value (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category="Main Engine")
	void UpdateThrottle(float Throttle);

	/**
	 * Activate or deactivate boost mode
	 * @param bActivate Whether to activate boost
	 */
	UFUNCTION(BlueprintCallable, Category="Main Engine")
	void SetBoostActive(bool bActivate);

	// ====================
	// RCS THRUSTER EFFECTS
	// ====================

	/** RCS thruster particle system template (CPU particles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RCS Thrusters")
	UParticleSystem* RCSThrusterTemplate;

	/** RCS particle components for each axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RCS Thrusters")
	TMap<ERCSThrusterAxis, UParticleSystemComponent*> RCSThrusterComponents;

	/** RCS effect duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RCS Thrusters", meta=(ClampMin="0.1", ClampMax="2.0"))
	float RCSThrusterDuration;

	/**
	 * Activate RCS thruster effect for specific axis
	 * @param Axis The thruster axis to activate
	 * @param Intensity Effect intensity (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category="RCS Thrusters")
	void ActivateRCSThruster(ERCSThrusterAxis Axis, float Intensity = 1.0f);

	// ====================
	// JUMP DRIVE EFFECTS
	// ====================

	/** Jump charge-up particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Drive")
	UParticleSystem* JumpChargeTemplate;

	/** Jump activation flash particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Drive")
	UParticleSystem* JumpActivationTemplate;

	/** Jump tunnel/wormhole effect particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Drive")
	UParticleSystem* JumpTunnelTemplate;

	/** Jump exit flash particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Drive")
	UParticleSystem* JumpExitTemplate;

	/** Jump effect component instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jump Drive")
	UParticleSystemComponent* JumpChargeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jump Drive")
	UParticleSystemComponent* JumpActivationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jump Drive")
	UParticleSystemComponent* JumpTunnelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jump Drive")
	UParticleSystemComponent* JumpExitComponent;

	/** Jump charge duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Drive", meta=(ClampMin="0.5", ClampMax="10.0"))
	float JumpChargeDuration;

	/** Is jump sequence currently active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Jump Drive")
	bool bJumpSequenceActive;

	/**
	 * Trigger complete jump sequence
	 * @param ChargeDuration How long to charge before jump (uses default if 0)
	 */
	UFUNCTION(BlueprintCallable, Category="Jump Drive")
	void TriggerJumpSequence(float ChargeDuration = 0.0f);

	/**
	 * Jump sequence completed callback
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Jump Drive")
	void OnJumpSequenceComplete();

	// ====================
	// ENGINE DAMAGE STATES
	// ====================

	/** Current engine damage state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage States")
	EEngineDamageState DamageState;

	/** Damaged engine particle system (sputtering) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage States")
	UParticleSystem* DamagedEngineTemplate;

	/** Critical damage particle system (smoke/fire) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage States")
	UParticleSystem* CriticalDamageTemplate;

	/** Damage effect component instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Damage States")
	UParticleSystemComponent* DamageEffectComponent;

	/**
	 * Set engine damage state
	 * @param NewState The new damage state to apply
	 */
	UFUNCTION(BlueprintCallable, Category="Damage States")
	void SetDamageState(EEngineDamageState NewState);

	// ====================
	// PARTICLE MANAGEMENT
	// ====================

	/** Maximum particles per ship (performance budget) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance", meta=(ClampMin="1000", ClampMax="20000"))
	int32 MaxParticlesPerShip;

	/** Get current total particle count */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Performance")
	int32 GetCurrentParticleCount() const;

protected:
	/** Initialize particle system components */
	void InitializeParticleSystems();

	/** Update engine glow material based on throttle */
	void UpdateEngineGlow(float DeltaTime);

	/** Handle jump sequence state machine */
	void UpdateJumpSequence(float DeltaTime);

	/** Apply damage state effects */
	void ApplyDamageEffects();

	/** Current jump sequence timer */
	float JumpSequenceTimer;

	/** Jump sequence current stage (0=charge, 1=activation, 2=tunnel, 3=exit, 4=complete) */
	int32 JumpSequenceStage;
};
