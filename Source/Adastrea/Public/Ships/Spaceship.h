#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipParticleComponent.h"
#include "InputActionValue.h"
#include "Spaceship.generated.h"

// Forward declarations
class ASpaceshipInterior;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class USpaceshipDataAsset;
class UDockingSettingsDataAsset;
class ASpaceStationModule;
class UUserWidget;

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
    TObjectPtr<UFloatingPawnMovement> MovementComponent;

    // Particle system component for engine and thruster effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
    TObjectPtr<USpaceshipParticleComponent> ParticleComponent;

    // Spring arm component for camera positioning
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraSpringArm;

    // Camera component for player view
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> Camera;

    // Default maximum movement speed
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultMaxSpeed;

    // Default acceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultAcceleration;

    // Default deceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultDeceleration;

    // Default turning boost multiplier
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultTurningBoost;

    // Reference to the walkable interior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interior")
    TWeakObjectPtr<ASpaceshipInterior> InteriorInstance;

    // Turn rate for ship rotation (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control", meta=(ClampMin="0.0"))
    float TurnRate;

    // Reference to the ship's data asset with detailed stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Data")
    TObjectPtr<USpaceshipDataAsset> ShipDataAsset;

    // Current hull integrity (health)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship Status")
    float CurrentHullIntegrity;

    // Maximum hull integrity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ship Status", meta=(ClampMin="0.0"))
    float MaxHullIntegrity;

    /**
     * Get the ship's display name
     * @return The ship's name from DataAsset or actor label
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    FText GetShipName() const;

    /**
     * Get the ship's class/type
     * @return The ship's class from DataAsset or default
     * 
     * @note POST-MVP: Deferred - ship classification not critical for MVP trading
     */
    // UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship") // DEFERRED: Post-MVP ship classification
    FText GetShipClass() const;

    /**
     * Get the current hull integrity
     * @return The current hull integrity value
     * 
     * @note POST-MVP: Deferred - damage/health system not in MVP (no combat)
     */
    // UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship") // DEFERRED: Post-MVP damage system
    float GetCurrentHullIntegrity() const;

    /**
     * Get the maximum hull integrity
     * @return The maximum hull integrity value
     * 
     * @note POST-MVP: Deferred - damage/health system not in MVP (no combat)
     */
    // UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship") // DEFERRED: Post-MVP damage system
    float GetMaxHullIntegrity() const;
    // ==========================================
    // X4-STYLE FLIGHT CONTROL PARAMETERS
    // ==========================================

    /**
     * Flight assist enabled - provides rotation damping and auto-leveling for smoother control.
     * When enabled, rotation smoothly damps to zero when no input is given, and the ship
     * automatically levels its roll. The throttle system controls forward velocity independently.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Flight Assist")
    bool bFlightAssistEnabled;

    /** Rotation damping factor for smooth rotation interpolation (0-1, higher = more damping) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Flight Assist", meta=(ClampMin="0.0", ClampMax="1.0"))
    float RotationDampingFactor;

    /** Auto-level strength - how strongly ship levels to ecliptic plane (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Flight Assist", meta=(ClampMin="0.0", ClampMax="1.0"))
    float AutoLevelStrength;

    /** Flight assist responsiveness - how quickly ship responds to input changes (higher = more responsive) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Flight Assist", meta=(ClampMin="0.1", ClampMax="10.0"))
    float FlightAssistResponsiveness;

    /** Current throttle percentage (0-100) - controls target velocity */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Throttle")
    float ThrottlePercentage;

    /** Throttle increment/decrement amount per input (percentage) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Throttle", meta=(ClampMin="1.0", ClampMax="25.0"))
    float ThrottleStep;
    
    /** Throttle adjustment rate limit (seconds between adjustments when button held) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Throttle", meta=(ClampMin="0.05", ClampMax="0.5"))
    float ThrottleAdjustmentCooldown;

    /** Boost mode active - temporary speed increase */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Boost")
    bool bBoostActive;

    /** Boost speed multiplier when active */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Boost", meta=(ClampMin="1.0", ClampMax="5.0"))
    float BoostMultiplier;

    /** Travel mode active - high-speed cruise for long distances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Control|Travel")
    bool bTravelModeActive;

    /** Travel mode speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Travel", meta=(ClampMin="2.0", ClampMax="20.0"))
    float TravelModeMultiplier;

    /** Strafe independence - lateral/vertical strafing independent from forward motion */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", meta=(ClampMin="0.0", ClampMax="1.0"))
    float StrafeIndependence;

    /** Mouse flight sensitivity for rotation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", meta=(ClampMin="0.1", ClampMax="5.0"))
    float MouseFlightSensitivity;

    /** Enable X4-style mouse position flight (cursor distance from center controls rotation speed) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced")
    bool bUseMousePositionFlight;

    /** Deadzone radius for mouse position flight (pixels from screen center where no rotation occurs) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", meta=(ClampMin="0.0", ClampMax="500.0"))
    float MouseDeadzoneRadius;

    /** Maximum effective radius for mouse position flight (pixels from center at which rotation speed is 100%) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flight Control|Advanced", meta=(ClampMin="100.0", ClampMax="2000.0"))
    float MouseMaxRadius;

    // ==========================================
    // FREE LOOK CAMERA PARAMETERS
    // ==========================================

    /** Free look camera mode active - when true, camera rotation is independent from ship rotation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera|Free Look")
    bool bFreeLookActive;

    /** Free look sensitivity - camera rotation speed during free look */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Free Look", meta=(ClampMin="0.1", ClampMax="5.0"))
    float FreeLookSensitivity;

    /** Camera spring arm target length */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Settings", meta=(ClampMin="100.0", ClampMax="5000.0"))
    float CameraDistance;

    /** Camera lag speed for smooth position following */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Settings", meta=(ClampMin="0.0", ClampMax="25.0"))
    float CameraLagSpeed;

    /** Camera rotation lag speed for smooth rotation following - prevents snapping */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Settings", meta=(ClampMin="0.0", ClampMax="25.0"))
    float CameraRotationLagSpeed;

    /** Double-click time threshold for resetting camera (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Free Look", meta=(ClampMin="0.1", ClampMax="1.0"))
    float DoubleClickThreshold;

    /**
     * Transition player into the ship's interior space
     * @param PlayerController The controller to transition into the interior
     * 
     * @note POST-MVP: Deferred - interior exploration not needed for MVP trading
     */
    // UFUNCTION(BlueprintCallable, Category="Spaceship") // DEFERRED: Post-MVP ship interior system
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

    // Enhanced Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* FreeLookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* ThrottleUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* ThrottleDownAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    class UInputAction* DockAction;

    // Enhanced Input callbacks
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void FreeLookStarted();
    void FreeLookCompleted();
    void FreeLookCamera(const FInputActionValue& Value);

    /**
     * Toggle flight assist on/off
     * Flight assist maintains ship orientation and velocity when no input is given
     * 
     * @note POST-MVP: Deferred - basic flight sufficient for MVP, advanced controls post-MVP
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP advanced flight control
    void ToggleFlightAssist();

    /**
     * Increase throttle by ThrottleStep percentage
     * 
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP - use Controls component
    void ThrottleUp();

    /**
     * Decrease throttle by ThrottleStep percentage
     * 
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP - use Controls component
    void ThrottleDown();

    /**
     * Set throttle to specific percentage (0-100)
     * @param Percentage Target throttle percentage
     * 
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP - use Controls component
    void SetThrottle(float Percentage);

    /**
     * Activate boost mode for temporary speed increase
     * 
     * @note POST-MVP: Deferred - boost mechanic not needed for basic MVP trading
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP advanced flight
    void ActivateBoost();

    /**
     * Deactivate boost mode
     * 
     * @note POST-MVP: Deferred - boost mechanic not needed for basic MVP trading
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP advanced flight
    void DeactivateBoost();

    /**
     * Toggle travel mode for high-speed cruise
     * 
     * @note POST-MVP: Deferred - travel mode not needed for basic MVP trading
     */
    // UFUNCTION(BlueprintCallable, Category="Flight Control") // DEFERRED: Post-MVP advanced flight
    void ToggleTravelMode();

    /**
     * Get current effective max speed with boost/travel mode multipliers
     * @return Current max speed considering all speed modifiers
     * 
     * @note POST-MVP: Deferred - advanced speed calculation not needed for MVP
     */
    // UFUNCTION(BlueprintCallable, BlueprintPure, Category="Flight Control") // DEFERRED: Post-MVP advanced stats
    float GetEffectiveMaxSpeed() const;

    // ===== DOCKING FUNCTIONS =====
    
    /**
     * Called by docking module when ship enters interaction range
     * @param Station The nearby docking module (DockingBay or DockingPort)
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void SetNearbyStation(ASpaceStationModule* Station);
    
    /**
     * Show or hide the docking prompt UI
     * @param bShow True to show prompt, false to hide
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void ShowDockingPrompt(bool bShow);
    
    /**
     * Request docking at nearby station (called by input)
     * Validates station availability and initiates docking sequence
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void RequestDocking();
    
    /**
     * Instantly dock at assigned docking point
     * Simple teleport for MVP - no fancy animation
     * @param DockingPoint The target docking point scene component
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void NavigateToDockingPoint(USceneComponent* DockingPoint);
    
    /**
     * Finalize docking: disable controls, open trading UI
     * Called when docking timeline completes
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void CompleteDocking();
    
    /**
     * Undock from station and restore control
     * Closes trading UI and gives ship forward impulse
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void Undock();
    
    /**
     * Check if ship is currently docked
     * @return True if ship is docked at a station
     */
    UFUNCTION(BlueprintPure, Category="Docking")
    bool IsDocked() const { return bIsDocked; }
    
    /**
     * Check if ship is in docking sequence
     * @return True if ship is actively moving to docking point
     */
    UFUNCTION(BlueprintPure, Category="Docking")
    bool IsDocking() const { return bIsDocking; }
    
    /**
     * Get the effective docking range
     * Returns value from DockingSettings if set, otherwise returns DockingRange
     * @return Maximum distance for docking in cm
     */
    UFUNCTION(BlueprintPure, Category="Docking")
    float GetEffectiveDockingRange() const;
    
    /**
     * Get the effective docking prompt widget class
     * Returns class from DockingSettings if set, otherwise returns DockingPromptWidgetClass
     * @return Widget class for docking prompt UI
     */
    UFUNCTION(BlueprintPure, Category="Docking")
    TSubclassOf<UUserWidget> GetEffectiveDockingPromptWidgetClass() const;
    
    /**
     * Get the effective trading interface class
     * Returns class from DockingSettings if set, otherwise returns TradingInterfaceClass
     * @return Widget class for trading interface UI
     */
    UFUNCTION(BlueprintPure, Category="Docking")
    TSubclassOf<UUserWidget> GetEffectiveTradingInterfaceClass() const;


protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Saved reference to the walking pawn when controlling the ship (GC tracked) */
    UPROPERTY(Transient)
    TObjectPtr<APawn> SavedExternalPawn;

    // Movement input handlers
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void Roll(float Value);

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
    
    /**
     * Update X4-style mouse position flight controls
     * Continuously checks mouse cursor position and rotates ship accordingly
     * Called every frame when bUseMousePositionFlight is true
     * @param DeltaTime Time since last frame
     */
    void UpdateMousePositionFlight(float DeltaTime);
    
    /**
     * Check if throttle can be adjusted based on cooldown
     * @return True if enough time has passed since last adjustment
     */
    bool CanAdjustThrottle();

    // ===== DOCKING SYSTEM =====
    
    /**
     * Centralized docking configuration settings
     * 
     * If set, this Data Asset provides all docking configuration (UI widgets, range, etc.)
     * making it easy to share settings across multiple ships without configuring each one.
     * 
     * If not set, the ship will fall back to individual properties (DockingPromptWidgetClass,
     * TradingInterfaceClass, DockingRange) for backward compatibility.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking",
        meta=(ToolTip="Centralized docking settings. If set, overrides individual docking properties."))
    TObjectPtr<UDockingSettingsDataAsset> DockingSettings;
    
    /** Reference to nearby station module in docking range */
    UPROPERTY(BlueprintReadWrite, Category="Docking")
    TObjectPtr<ASpaceStationModule> NearbyStation;
    
    /** Currently assigned docking point scene component */
    UPROPERTY(BlueprintReadWrite, Category="Docking")
    TObjectPtr<USceneComponent> CurrentDockingPoint;
    
    /** Is the ship currently docked at a station? */
    UPROPERTY(BlueprintReadOnly, Category="Docking")
    bool bIsDocked = false;
    
    /** Is the ship currently in a docking sequence? */
    UPROPERTY(BlueprintReadOnly, Category="Docking")
    bool bIsDocking = false;
    
    /** 
     * Maximum distance from docking point to allow docking
     * NOTE: If DockingSettings is set, this value is overridden by settings.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Docking|Fallback", meta=(ClampMin="100.0", ClampMax="10000.0"))
    float DockingRange = 2000.0f;
    
    /** Reference to active docking prompt widget */
    UPROPERTY(BlueprintReadOnly, Category="Docking|UI")
    TObjectPtr<UUserWidget> DockingPromptWidget;
    
    /** 
     * Widget class for docking prompt UI
     * NOTE: If DockingSettings is set, this value is overridden by settings.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Docking|Fallback")
    TSubclassOf<UUserWidget> DockingPromptWidgetClass;
    
    /** 
     * Widget class for trading interface UI
     * NOTE: If DockingSettings is set, this value is overridden by settings.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Docking|Fallback")
    TSubclassOf<UUserWidget> TradingInterfaceClass;
    
    /** Active trading widget instance */
    UPROPERTY(BlueprintReadOnly, Category="Docking|UI")
    TObjectPtr<UUserWidget> TradingWidget;

private:
    // Current velocity for inertia-based movement
    FVector CurrentVelocity;

    // Target velocity based on throttle percentage
    FVector TargetVelocity;

    // Current rotation velocity for smooth rotation
    FRotator RotationVelocity;

    /** Input values for smooth interpolation (transient runtime state) */
    UPROPERTY(Transient)
    float ForwardInput;
    UPROPERTY(Transient)
    float RightInput;
    UPROPERTY(Transient)
    float UpInput;
    UPROPERTY(Transient)
    float YawInput;
    UPROPERTY(Transient)
    float PitchInput;
    UPROPERTY(Transient)
    float RollInput;

    /** Free look camera state (transient runtime state) */
    UPROPERTY(Transient)
    FRotator FreeLookRotation;
    
    /** Double-click detection for camera reset (transient runtime state) */
    UPROPERTY(Transient)
    float LastFreeLookClickTime;
    
    // Throttle adjustment rate limiting
    float LastThrottleAdjustmentTime;
};