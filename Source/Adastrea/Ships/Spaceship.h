#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipParticleComponent.h"
#include "Spaceship.generated.h"

/**
 * Base spaceship actor class for player and NPC ships
 * 
 * This is the primary actor class for all spaceships in the game. It provides:
 * - Basic ship functionality and controls
 * - Interior space management for boarding/exploration
 * - Integration point for SpaceshipDataAsset configuration
 * 
 * Usage:
 * - Create Blueprint based on this class
 * - Configure ship properties and appearance
 * - Assign SpaceshipDataAsset for detailed stats
 * - Add mesh components for visual representation
 */
UCLASS()
class ADASTREA_API ASpaceship : public APawn
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
     * Binds axis and action inputs for movement and rotation
     */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Saved reference to the walking pawn when controlling the ship
    UPROPERTY()
    APawn* SavedExternalPawn;

    // Movement input handlers
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);

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