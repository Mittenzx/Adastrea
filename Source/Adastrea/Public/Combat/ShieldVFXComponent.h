#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatHealthComponent.h"
#include "NiagaraComponent.h"
#include "ShieldVFXComponent.generated.h"

/**
 * Enum for shield visual states
 */
UENUM(BlueprintType)
enum class EShieldVisualState : uint8
{
	Full        UMETA(DisplayName = "Full Shield"),
	Recharging  UMETA(DisplayName = "Recharging"),
	Down        UMETA(DisplayName = "Shield Down"),
	Disabled    UMETA(DisplayName = "Disabled")
};

/**
 * Struct for shield facing visual data
 */
USTRUCT(BlueprintType)
struct FShieldFacingVisual
{
	GENERATED_BODY()

	/** Current visual state */
	UPROPERTY(BlueprintReadOnly)
	EShieldVisualState State;

	/** Niagara component for this facing */
	UPROPERTY(BlueprintReadOnly)
	UNiagaraComponent* EffectComponent;

	/** Material instance for shield surface */
	UPROPERTY(BlueprintReadOnly)
	UMaterialInstanceDynamic* ShieldMaterial;

	/** Current glow intensity */
	UPROPERTY(BlueprintReadOnly)
	float GlowIntensity;

	/** Pulse animation time */
	UPROPERTY(BlueprintReadOnly)
	float PulseTime;

	FShieldFacingVisual()
		: State(EShieldVisualState::Full)
		, EffectComponent(nullptr)
		, ShieldMaterial(nullptr)
		, GlowIntensity(1.0f)
		, PulseTime(0.0f)
	{}
};

/**
 * Component for Shield Visual Effects
 * 
 * Manages visual feedback for shield states:
 * 
 * **Shield States:**
 * - Full: Green steady glow indicating full protection
 * - Recharging: Blue pulse animation showing recharge progress
 * - Down: Red glow on damaged facing indicating vulnerability
 * - Disabled: No visual effect
 * 
 * **Directional Feedback:**
 * - Supports 4-facing directional shields (Forward, Aft, Port, Starboard)
 * - Each facing shows independent state
 * - Omnidirectional shields show single unified state
 * 
 * **Integration:**
 * - Automatically monitors CombatHealthComponent for shield state changes
 * - Updates visual feedback based on shield strength and recharge state
 * - Provides Blueprint events for custom reactions
 * 
 * Usage:
 * - Add component to spaceship Blueprint
 * - Configure shield visual materials and effects
 * - Component automatically updates based on shield state
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UShieldVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldVFXComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// CONFIGURATION
	// ====================

	/** Shield visual material (base material) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Visuals")
	UMaterialInterface* ShieldBaseMaterial;

	/** Niagara system for shield effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Visuals")
	UNiagaraSystem* ShieldEffectTemplate;

	/** Combat health component reference (auto-detected if null) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Visuals")
	UCombatHealthComponent* CombatHealthComponent;

	// ====================
	// VISUAL STATE COLORS
	// ====================

	/** Color for full shield (steady glow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors")
	FLinearColor FullShieldColor;

	/** Color for recharging shield (pulsing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors")
	FLinearColor RechargingColor;

	/** Color for shield down (warning) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors")
	FLinearColor ShieldDownColor;

	/** Pulse speed for recharging animation (cycles per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors", meta=(ClampMin="0.5", ClampMax="5.0"))
	float RechargePulseSpeed;

	/** Minimum intensity during pulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors", meta=(ClampMin="0.1", ClampMax="1.0"))
	float MinPulseIntensity;

	/** Maximum intensity during pulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield Colors", meta=(ClampMin="1.0", ClampMax="3.0"))
	float MaxPulseIntensity;

	// ====================
	// SHIELD FACING VISUALS
	// ====================

	/** Visual data for forward facing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield Facings")
	FShieldFacingVisual ForwardFacing;

	/** Visual data for aft facing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield Facings")
	FShieldFacingVisual AftFacing;

	/** Visual data for port facing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield Facings")
	FShieldFacingVisual PortFacing;

	/** Visual data for starboard facing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield Facings")
	FShieldFacingVisual StarboardFacing;

	/** Visual data for omnidirectional shield */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield Facings")
	FShieldFacingVisual OmnidirectionalShield;

	// ====================
	// BLUEPRINT CALLABLE FUNCTIONS
	// ====================

	/**
	 * Manually set shield visual state for a facing
	 * @param Facing The shield facing to update
	 * @param NewState The new visual state
	 */
	UFUNCTION(BlueprintCallable, Category="Shield VFX")
	void SetFacingState(EShieldFacing Facing, EShieldVisualState NewState);

	/**
	 * Get current shield visual state for a facing
	 * @param Facing The shield facing to query
	 * @return Current visual state
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shield VFX")
	EShieldVisualState GetFacingState(EShieldFacing Facing) const;

	/**
	 * Update all shield facings based on combat health component
	 * Called automatically each frame
	 */
	UFUNCTION(BlueprintCallable, Category="Shield VFX")
	void UpdateShieldVisuals();

	/**
	 * Play shield impact effect at location
	 * @param ImpactLocation World location of impact
	 * @param ImpactNormal Normal vector at impact point
	 * @param Facing Shield facing that was hit
	 */
	UFUNCTION(BlueprintCallable, Category="Shield VFX")
	void PlayShieldImpact(FVector ImpactLocation, FVector ImpactNormal, EShieldFacing Facing);

	// ====================
	// BLUEPRINT NATIVE EVENTS
	// ====================

	/**
	 * Called when shield state changes for a facing
	 * Override in Blueprint for custom reactions
	 * @param Facing The facing that changed
	 * @param OldState Previous state
	 * @param NewState New state
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Shield VFX")
	void OnShieldStateChanged(EShieldFacing Facing, EShieldVisualState OldState, EShieldVisualState NewState);

	/**
	 * Called when shields are fully recharged
	 * Override in Blueprint for custom reactions
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Shield VFX")
	void OnShieldsFullyRecharged();

	/**
	 * Called when all shields are down
	 * Override in Blueprint for custom reactions
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Shield VFX")
	void OnAllShieldsDown();

protected:
	/**
	 * Initialize shield visual components
	 */
	void InitializeShieldVisuals();

	/**
	 * Update a single shield facing visual
	 * @param FacingVisual The facing visual data to update
	 * @param Facing The shield facing being updated
	 * @param ShieldStrength Current shield strength (0-1)
	 * @param IsRecharging Whether shield is recharging
	 * @param DeltaTime Time since last update
	 */
	void UpdateFacingVisual(FShieldFacingVisual& FacingVisual, EShieldFacing Facing, float ShieldStrength, bool IsRecharging, float DeltaTime);

	/**
	 * Get color for current shield state
	 * @param State Visual state
	 * @param PulseTime Current pulse animation time
	 * @return Color to apply
	 */
	FLinearColor GetColorForState(EShieldVisualState State, float PulseTime) const;

	/**
	 * Get facing visual by enum
	 * @param Facing Shield facing enum
	 * @return Reference to facing visual data
	 */
	FShieldFacingVisual& GetFacingVisual(EShieldFacing Facing);

	/**
	 * Get facing visual by enum (const)
	 * @param Facing Shield facing enum
	 * @return Const reference to facing visual data
	 */
	const FShieldFacingVisual& GetFacingVisual(EShieldFacing Facing) const;

	/** Track if all shields were down last frame */
	bool bWereAllShieldsDown;

	/** Track if shields were full last frame */
	bool bWereShieldsFull;
};
