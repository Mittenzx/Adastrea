#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipParticleComponent.h"
#include "InputActionValue.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "Interfaces/IFactionMember.h"
#include "Spaceship.generated.h"

// Forward declarations
class ASpaceshipInterior;
class UInputAction;
class UFactionDataAsset;

/**
 * Base spaceship actor class for player and NPC ships
 * 
 * This is the primary actor class for all spaceships in the game. It provides:
 * - Basic ship functionality and controls
 * - Interior space management for boarding/exploration
 * - Integration point for SpaceshipDataAsset configuration
 * 
 * Implements:
 * - IDamageable: Can receive damage from weapons
 * - ITargetable: Can be targeted by weapons and sensors
 * - IFactionMember: Belongs to a faction for diplomacy and AI
 * 
 * Usage:
 * - Create Blueprint based on this class
 * - Configure ship properties and appearance
 * - Assign SpaceshipDataAsset for detailed stats
 * - Add mesh components for visual representation
 */
UCLASS()
class ADASTREA_API ASpaceship : public APawn, public IDamageable, public ITargetable, public IFactionMember
{
    GENERATED_BODY()

public:
    ASpaceship();

    // Movement component for floating pawn movement in space
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
    UFloatingPawnMovement* MovementComponent;

    // Particle system component for engine and thruster effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
    USpaceshipParticleComponent* ParticleComponent;

    // Default maximum movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultMaxSpeed;

    // Default acceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultAcceleration;

    // Default deceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultDeceleration;

    // Default turning boost multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultTurningBoost;

    // Reference to the walkable interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    ASpaceshipInterior* InteriorInstance;

    // Turn rate for ship rotation (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control", meta=(ClampMin="0.0"))
    float TurnRate;

    // Reference to the ship's data asset with detailed stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Data")
    class USpaceshipDataAsset* ShipDataAsset;

    // Faction this ship belongs to (affects AI behavior and diplomacy)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Data")
    UFactionDataAsset* ShipFaction;

    // Current hull integrity (health)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Status", meta=(ClampMin="0.0"))
    float CurrentHullIntegrity;

    // Maximum hull integrity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Status", meta=(ClampMin="0.0"))
    float MaxHullIntegrity;

    // Flag indicating if ship is destroyed
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship Status")
    bool bIsDestroyed;

    /**
     * Get the ship's display name
     * @return The ship's name from DataAsset or actor label
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    FText GetShipName() const;

    /**
     * Get the ship's class/type
     * @return The ship's class from DataAsset or default
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    FText GetShipClass() const;

    /**
     * Get the current hull integrity
     * @return The current hull integrity value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    float GetCurrentHullIntegrity() const;

    /**
     * Get the maximum hull integrity
     * @return The maximum hull integrity value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    float GetMaxHullIntegrity() const;
    // ==========================================
    // X4-STYLE FLIGHT CONTROL PARAMETERS
    // ==========================================

    /**
     * Flight assist enabled - provides rotation damping and auto-leveling for smoother control.
     * When enabled, rotation smoothly damps to zero when no input is given, and the ship
     * automatically levels its roll. The throttle system controls forward velocity independently.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Flight Assist")
    bool bFlightAssistEnabled;

    /** Rotation damping factor for smooth rotation interpolation (0-1, higher = more damping) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Flight Assist", meta=(ClampMin="0.0", ClampMax="1.0"))
    float RotationDampingFactor;

    /** Auto-level strength - how strongly ship levels to ecliptic plane (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Flight Assist", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AutoLevelStrength;

    /** Flight assist responsiveness - how quickly ship responds to input changes (higher = more responsive) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Flight Assist", meta=(ClampMin="0.1", ClampMax="10.0"))
    float FlightAssistResponsiveness;

    /** Current throttle percentage (0-100) - controls target velocity */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Throttle")
    float ThrottlePercentage;

    /** Throttle increment/decrement amount per input (percentage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Throttle", meta=(ClampMin="1.0", ClampMax="25.0"))
    float ThrottleStep;

    /** Boost mode active - temporary speed increase */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Boost")
    bool bBoostActive;

    /** Boost speed multiplier when active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Boost", meta=(ClampMin="1.0", ClampMax="5.0"))
    float BoostMultiplier;

    /** Travel mode active - high-speed cruise for long distances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Travel")
    bool bTravelModeActive;

    /** Travel mode speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Travel", meta=(ClampMin="2.0", ClampMax="20.0"))
    float TravelModeMultiplier;

    /** Strafe independence - lateral/vertical strafing independent from forward motion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Advanced", meta=(ClampMin="0.0", ClampMax="1.0"))
    float StrafeIndependence;

    /** Mouse flight sensitivity for rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight Control|Advanced", meta=(ClampMin="0.1", ClampMax="5.0"))
    float MouseFlightSensitivity;

    /**
     * Transition player into the ship's interior space
     * @param PlayerController The controller to transition into the interior
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship")
    void EnterInterior(class APlayerController* PlayerController);

    /**
     * Begin controlling the ship from interior console
     * Saves the walking pawn and possesses the ship
     * @param PC The player controller taking control
     * @param ExternalPawn The walking pawn to save for later restoration
     */
    UFUNCTION(BlueprintCallable, Category="Control")
    void BeginControl(class APlayerController* PC, class APawn* ExternalPawn);

    /**
     * End controlling the ship and return to walking pawn
     * Restores the saved walking pawn
     * @param PC The player controller releasing control
     */
    UFUNCTION(BlueprintCallable, Category="Control")
    void EndControl(class APlayerController* PC);

    /**
     * Setup input component for ship control
     * Binds Enhanced Input actions (MoveAction, LookAction) for movement and rotation
     * Requires MoveAction and LookAction to be assigned in Blueprint
     */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Enhanced Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* ThrottleUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* ThrottleDownAction;

    // Enhanced Input callbacks
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    /**
     * Toggle flight assist on/off
     * Flight assist maintains ship orientation and velocity when no input is given
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void ToggleFlightAssist();

    /**
     * Increase throttle by ThrottleStep percentage
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void ThrottleUp();

    /**
     * Decrease throttle by ThrottleStep percentage
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void ThrottleDown();

    /**
     * Set throttle to specific percentage (0-100)
     * @param Percentage Target throttle percentage
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void SetThrottle(float Percentage);

    /**
     * Activate boost mode for temporary speed increase
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void ActivateBoost();

    /**
     * Deactivate boost mode
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void DeactivateBoost();

    /**
     * Toggle travel mode for high-speed cruise
     */
    UFUNCTION(BlueprintCallable, Category="Flight Control")
    void ToggleTravelMode();

    /**
     * Get current effective max speed with boost/travel mode multipliers
     * @return Current max speed considering all speed modifiers
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Flight Control")
    float GetEffectiveMaxSpeed() const;

    /**
     * Get current forward speed in the ship's forward direction
     * @return Current forward speed in units per second
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Flight Control")
    float GetForwardSpeed() const;

    // Movement input handlers - public so SpaceshipControlsComponent can call them
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);

    // ====================
    // INTERFACE IMPLEMENTATIONS
    // ====================

    // IDamageable Interface
    virtual float ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser) override;
    virtual bool CanTakeDamage_Implementation() const override;
    virtual float GetHealthPercentage_Implementation() const override;
    virtual bool IsDestroyed_Implementation() const override;
    virtual float GetMaxHealth_Implementation() const override;
    virtual float GetCurrentHealth_Implementation() const override;

    // ITargetable Interface
    virtual bool CanBeTargeted_Implementation() const override;
    virtual int32 GetTargetPriority_Implementation() const override;
    virtual FText GetTargetDisplayName_Implementation() const override;
    virtual UTexture2D* GetTargetIcon_Implementation() const override;
    virtual FVector GetAimPoint_Implementation() const override;
    virtual float GetTargetSignature_Implementation() const override;
    virtual float GetDistanceFromLocation_Implementation(FVector FromLocation) const override;
    virtual bool IsHostileToActor_Implementation(AActor* Observer) const override;

    // IFactionMember Interface
    virtual UFactionDataAsset* GetFaction_Implementation() const override;
    virtual bool IsAlliedWith_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual bool IsHostileTo_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual int32 GetRelationshipWith_Implementation(const TScriptInterface<IFactionMember>& Other) const override;
    virtual bool IsNeutral_Implementation() const override;
    virtual FText GetFactionDisplayName_Implementation() const override;
    virtual bool CanEngageInCombat_Implementation() const override;
    virtual float GetTradePriceModifier_Implementation(UFactionDataAsset* TraderFaction) const override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;

    // Saved reference to the walking pawn when controlling the ship
    UPROPERTY()
    APawn* SavedExternalPawn;

    /**
     * Apply X4-style flight assist physics
     * Handles inertia preservation, auto-leveling, and rotation damping
     * @param DeltaTime Time since last frame
     */
    void ApplyFlightAssist(float DeltaTime);

    /**
     * Apply auto-leveling to ecliptic plane
     * Gradually levels ship roll when no roll input is given
     * @param DeltaTime Time since last frame
     */
    void ApplyAutoLeveling(float DeltaTime);

    /**
     * Update throttle-based velocity
     * Adjusts ship velocity towards target velocity based on throttle
     * @param DeltaTime Time since last frame
     */
    void UpdateThrottleVelocity(float DeltaTime);

private:
    // Current velocity for inertia-based movement
    FVector CurrentVelocity;

    // Target velocity based on throttle percentage
    FVector TargetVelocity;

    // Current rotation velocity for smooth rotation
    FRotator RotationVelocity;

    // Input values for smooth interpolation
    float ForwardInput;
    float RightInput;
    float UpInput;
    float YawInput;
    float PitchInput;
};