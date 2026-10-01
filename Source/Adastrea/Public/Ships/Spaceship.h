#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipParticleComponent.h"
#include "Ships/SpaceshipInterior.h"
#include "InputActionValue.h"
#include "Spaceship.generated.h"

// Forward declarations
class ASpaceshipInterior;
class UInputAction;
class USpringArmComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UCameraComponent;
class USpaceshipDataAsset;
class UDockingSettingsDataAsset;
class ASpaceStationModule;
class UUserWidget;
class UCargoComponent;
class UDroneBayComponent;
class UPlayerTraderComponent;
class UShipEngineAudioComponent;
class UShipUpgradeComponent;
class UShipUpgradeDataAsset;
class ASpaceStation;

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

    // Root scene component - provides proper pivot point for ship rotation
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
        TObjectPtr<USceneComponent> ShipRoot;

        // Visual static mesh for the ship hull (assembled ship silhouette). Each ship
        // Blueprint overrides the mesh asset (e.g. SM_Ship_Fighter_01_Assembled).
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
        TObjectPtr<UStaticMeshComponent> ShipMeshComponent;

        /** Hull material for ShipMesh slot 0. When set, it replaces the per-class tiled
         * hull material that ApplyShipHullMaterial picks from the actor name. Set it on
         * ships that use a unique-UV baked hull (SM_*_Assembled_UniqueUV), because those
         * UVs only line up with their own baked material (e.g. M_Battleship_Hull_Unique). */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hull")
        TSoftObjectPtr<UMaterialInterface> HullMaterialOverride;

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

    // Cargo hold component (trading inventory)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trading")
    TObjectPtr<UCargoComponent> CargoComponent;

    // Drone bay: mining drones fly out, cut ore and bring it home (enabled for ships with a MiningRating).
    // Ships have no beams; mining, gas, towing and cargo transfer are drone jobs.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drones")
    TObjectPtr<UDroneBayComponent> DroneBay;

    // Player trader component (credits, buy/sell)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trading")
    TObjectPtr<UPlayerTraderComponent> PlayerTraderComponent;

    // Fitted upgrades (engines, weapons, shields, hull, cargo); bought at station outfitting modules
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Upgrades")
    TObjectPtr<UShipUpgradeComponent> UpgradeComponent;

    // Engine voice: size/agility-derived loops driven by throttle, speed and boost.
    // 2D for the player's ship (muffled on foot), 3D and very short range for everyone else.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
    TObjectPtr<UShipEngineAudioComponent> EngineAudio;

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

    // Converts the data asset's MaxSpeed (design units) to DefaultMaxSpeed (cm/s).
    // 2.5 keeps the starter Viper (MaxSpeed 1200) at the 3000 it flew before the wiring.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DataAssetSpeedScale = 2.5f;

    // Converts the data asset's Acceleration to DefaultAcceleration/DefaultDeceleration (cm/s^2).
    // Keeps the Viper (Acceleration 180) at the 1000 it flew before the wiring.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0"))
    float DataAssetAccelerationScale = 1000.0f / 180.0f;

    /**
     * Sets top speed, acceleration and deceleration from ShipDataAsset (scaled by the
     * DataAsset*Scale factors) and pushes them to the movement component. Called in
     * BeginPlay before upgrades capture their base stats; call RebaseUpgradeStats()
     * after it if the data asset changes later.
     */
    UFUNCTION(BlueprintCallable, Category="Movement")
    void ApplyDataAssetMobility();

    // Reference to the walkable interior
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interior")
            TWeakObjectPtr<ASpaceshipInterior> InteriorInstance;

            /** Interior shell mesh to show when walking the ship (e.g. cockpit/hold). */
            UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interior")
            TSoftObjectPtr<UStaticMesh> InteriorShellMesh;

            /** Which companion-part kit + material set the interior shell belongs to.
             * Leave at None to infer it from the shell mesh's asset name (legacy
             * behaviour); set explicitly for new ships instead of relying on naming. */
            UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interior")
            EShipInteriorFamily InteriorFamily = EShipInteriorFamily::None;

            /** Get the ship's interior instance (may be null if unset). */
            UFUNCTION(BlueprintPure, Category="Interior")
            ASpaceshipInterior* GetInteriorInstance() const { return InteriorInstance.Get(); }

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
        FText GetShipClass() const;

        /** Assign the imported M_*_Hull material to the ship mesh (called in BeginPlay)
         * so ships render textured. Maps hull by ship class. */
        void ApplyShipHullMaterial();

        /** The hull material a ship named ShipName flies with: Override if it loads, else
         * the class material picked from the name. Shared with the ship-select preview so
         * it shows the hull the ship actually wears. */
        static UMaterialInterface* ResolveHullMaterial(const FString& ShipName, const TSoftObjectPtr<UMaterialInterface>& Override);

        /** Attach the hull's separate window geometry (called in BeginPlay). Windows are
         * not part of the tiling hull texture: Tools/build_ship_windows.py exports
         * SM_Ship_<X>_01_Windows in the hull's object space, so it goes on the ship
         * mesh at identity. Found from the hull mesh name; nothing happens if absent. */
        void AttachShipWindows();

        /** Frame the chase camera on the hull (called in BeginPlay): pivot on the hull's
         * centre, distance scaled to its size, ship in the bottom middle of the screen.
         * Applies to every ship so the view behind each one is consistent. */
        void FitCameraToHull();

        /** All visible static meshes on the ship, in ShipRoot's (unscaled) frame. */
        FBox GetHullLocalBounds() const;

        /** Runtime-created by AttachShipWindows; null when the hull has no window mesh. */
        UPROPERTY(VisibleInstanceOnly, Transient, Category="Components")
        TObjectPtr<UStaticMeshComponent> WindowMeshComponent;

        /** Swing the chase camera once around the ship from further out, then settle back
         * behind it. Played after a ship-select swap so the new ship is seen from all sides. */
        void PlaySwapShowcase();

        /** Half the hull's longest side (cm), from the combined static mesh bounds. */
        float GetHullHalfLength() const;

    /**
     * Get the current hull integrity
     * @return The current hull integrity value
     *
     * @note POST-MVP: Deferred - damage/health system not in MVP (no combat)
     */
    float GetCurrentHullIntegrity() const;

    /**
         * Get the maximum hull integrity
         * @return The maximum hull integrity value
         *
         * @note POST-MVP: Deferred - damage/health system not in MVP (no combat)
         */
        float GetMaxHullIntegrity() const;

         // ==========================================
         // COCKPIT / LEAVE CANDIDATE CONDITIONS
         // ==========================================

         /** Current linear speed (units/sec) of the ship. */
         UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship|Control")
         float GetCurrentSpeed() const;

         /** Current world velocity vector of the ship. */
         UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship|Control")
         FVector GetCurrentVelocity() const { return CurrentVelocity; }

         /** Whether the ship is effectively stationary (moving slower than the threshold). */
         UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship|Control")
         bool IsStationary(float SpeedThreshold = 25.0f) const { return GetCurrentSpeed() <= SpeedThreshold; }

         /** Whether the ship is currently flagged as in combat. */
         UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship|Combat")
         bool IsInCombat() const { return bIsInCombat; }

         /** Set/clear the in-combat flag (used by future combat system). */
         UFUNCTION(BlueprintCallable, Category="Spaceship|Combat")
         void SetInCombat(bool bInCombat) { bIsInCombat = bInCombat; }

         /** Whether autopilot / flight to destination is engaged. */
         UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship|Autopilot")
         bool IsOnAutopilot() const { return bIsOnAutopilot; }

         /** Set/clear the autopilot flag. */
         UFUNCTION(BlueprintCallable, Category="Spaceship|Autopilot")
         void SetAutopilot(bool bEnabled) { bIsOnAutopilot = bEnabled; }

         /**
          * Whether the player is permitted to leave the cockpit right now.
          * Prevents leaving while moving, in combat, on autopilot, or mid-docking.
          * @param OutReason Rfilled with a human-readable reason if the player cannot leave.
          * @return True if the player may leave the cockpit.
          */
         UFUNCTION(BlueprintCallable, Category="Spaceship|Control")
         bool CanLeaveCockpit(FString& OutReason) const;

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
     * Show a HUD alert message to the player
     * @param Message The message to display
     * @param Duration How long to show the message in seconds (default: 3.0)
     * @param bIsWarning Whether this is a warning message (affects visual style)
     */
    UFUNCTION(BlueprintCallable, Category="HUD")
    void ShowHUDAlert(const FText& Message, float Duration = 3.0f, bool bIsWarning = true);

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

    /** Launch / recall the mining drones (L). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drones")
    class UInputAction* DroneAction;

    /** Lock the asteroid nearest the ship's nose (T). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mining")
    class UInputAction* LockAsteroidAction;

    /** Send the mining drones to the locked asteroid (or the one ahead). */
    UFUNCTION(Exec, BlueprintCallable, Category="Drones")
    void LaunchDrones();

    UFUNCTION(Exec, BlueprintCallable, Category="Drones")
    void RecallDrones();

    /** Launch if the drones are aboard, recall if they're out. */
    UFUNCTION(Exec, BlueprintCallable, Category="Drones")
    void ToggleDrones();

    UFUNCTION(Exec, BlueprintCallable, Category="Mining")
    void LockAsteroid();

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
    void ToggleFlightAssist();

    /**
     * Increase throttle by ThrottleStep percentage
     *
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    void ThrottleUp();

    /**
     * Decrease throttle by ThrottleStep percentage
     *
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    void ThrottleDown();

    /**
     * Set throttle to specific percentage (0-100)
     * @param Percentage Target throttle percentage
     *
     * @note POST-MVP: Deferred - use SpaceshipControlsComponent instead for MVP
     */
    void SetThrottle(float Percentage);

    /**
     * Activate boost mode for temporary speed increase
     *
     * @note POST-MVP: Deferred - boost mechanic not needed for basic MVP trading
     */
    void ActivateBoost();

    /**
     * Deactivate boost mode
     *
     * @note POST-MVP: Deferred - boost mechanic not needed for basic MVP trading
     */
    void DeactivateBoost();

    /**
     * Toggle travel mode for high-speed cruise
     *
     * @note POST-MVP: Deferred - travel mode not needed for basic MVP trading
     */
    void ToggleTravelMode();

    /**
     * Get current effective max speed with boost/travel mode multipliers
     * @return Current max speed considering all speed modifiers
     *
     * @note POST-MVP: Deferred - advanced speed calculation not needed for MVP
     */
    float GetEffectiveMaxSpeed() const;

    // ===== UPGRADES =====

    /**
     * Re-apply every installed upgrade to the ship's base stats: speed, acceleration, boost,
     * hull, shields, cargo capacity and mining power. Bound to UpgradeComponent->OnUpgradesChanged.
     * Hull keeps its current fraction when the maximum changes.
     */
    UFUNCTION(BlueprintCallable, Category="Upgrades")
    void RecalculateUpgradedStats();

    /**
     * Take the ship's current stats as the pre-upgrade base and re-apply upgrades on top.
     * Called at BeginPlay; call it again after replacing base stats in code (e.g. a new data
     * asset on a ship with no upgrades fitted - fitted upgrades would be counted twice).
     */
    UFUNCTION(BlueprintCallable, Category="Upgrades")
    void RebaseUpgradeStats();

    /** A stat before upgrades (see ShipUpgradeStats); 0 for unknown stats. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
    float GetBaseStat(FName StatName) const;

    /** A stat with every installed upgrade applied. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
    float GetUpgradedStat(FName StatName) const;

    /** Maximum shield strength: the data asset's ShieldStrength plus shield upgrades. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
    float GetMaxShieldStrength() const;

    /** Weapon damage multiplier from weapon upgrades (1 = stock), for weapon systems to scale by. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrades")
    float GetWeaponDamageMultiplier() const;

    /** Whether removing one stack of Upgrade leaves the ship valid (e.g. the hold still fits its cargo). */
    bool CanRemoveUpgrade(const UShipUpgradeDataAsset* Upgrade, FText& OutReason) const;

    /** The station the ship is docked at (the owner of its docking module), else null. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Docking")
    ASpaceStation* GetDockedStation() const;

    // ===== DOCKING FUNCTIONS =====

    /**
     * Called by docking module when ship enters interaction range
     * @param Station The nearby docking module (DockingBay or DockingPort)
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    void SetNearbyStation(ASpaceStationModule* Station);

    /** The docking module in range (or docked at while IsDocked()), else null. */
    UFUNCTION(BlueprintPure, Category="Docking")
    ASpaceStationModule* GetNearbyStation() const { return NearbyStation; }

    /**
     * True when RequestDocking() would currently succeed: a docking bay is in range,
     * has a free slot and docking point within the effective docking range.
     * @param OutDistance Distance to the docking point (or bay) in cm, if one exists
     * @param OutStationName Name of the nearby station module, if one exists
     */
    UFUNCTION(BlueprintCallable, Category="Docking")
    bool CanRequestDocking(float& OutDistance, FString& OutStationName) const;

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
    UFUNCTION(Exec, BlueprintCallable, Category="Docking")
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

    /** The docking prompt widget, if one is showing (the map hides it while open). */
    UUserWidget* GetDockingPromptWidget() const { return DockingPromptWidget; }

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
    virtual void UnPossessed() override;

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

        /** Whether the ship is currently flagged as in combat (cannot leave cockpit). */
        UPROPERTY(BlueprintReadOnly, Category="Combat")
        bool bIsInCombat = false;

        /** Whether autopilot / auto-flight-to-destination is engaged (cannot leave cockpit). */
        UPROPERTY(BlueprintReadOnly, Category="Autopilot")
        bool bIsOnAutopilot = false;

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
    /** Seconds into PlaySwapShowcase; negative when it isn't playing. */
    float SwapShowcaseTime = -1.0f;

    /** Arm length to return to when the showcase ends. */
    float SwapShowcaseArmLength = 0.0f;

    /** Advance the swap showcase camera (from Tick). */
    void TickSwapShowcase(float DeltaTime);

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

    /** Self-contained input mapping context built in C++ so the ship controls
    *  work regardless of which GameMode is active (no reliance on a content
    *  IMC being added at runtime). Created lazily in SetupPlayerInputComponent. */
    UPROPERTY(Transient)
    class UInputMappingContext* RuntimeInputMappingContext;

public:
    /** Enable/disable the ship's runtime flight input mapping context. */
    UFUNCTION(BlueprintCallable, Category="Input")
    void SetRuntimeInputEnabled(bool bEnabled);

private:


    /**
    * Creates the ship's input actions + mapping context if not already set,
    * and adds the mapping context to the local player. Ensures WASD, mouse-look,
    * R/F throttle, and F dock all work even without a content-side IMC.
    */
    void EnsureOwnInputActionsAndContext();

    /**
    * Normalizes look input by viewport aspect ratio
    * @param LookInput The raw 2D look input vector
    * @param PC The player controller to get viewport size from
    * @return Normalized look input with X component adjusted for aspect ratio
    */
    static FVector2D NormalizeLookInputByAspectRatio(const FVector2D& LookInput, APlayerController* PC);

    // ===== EVENT SFX (thrusters, collisions) =====

    /**
     * Player ship only: plays Thruster.Puff (light ships) or Thruster.HeavyGroan (heavy
     * ships) when a rotation or strafe burst starts. Rate limited by ship size.
     */
    void UpdateThrusterAudio(float DeltaTime);

    /** Rotation last frame, to measure the turn rate for thruster bursts. */
    FQuat ThrusterAudioLastRotation = FQuat::Identity;
    bool bThrusterAudioHasRotation = false;

    /** Seconds the ship has been below the manoeuvre thresholds (a burst needs a quiet gap first). */
    float ThrusterAudioQuietTime = 10.0f;

    /** Whether the previous frame counted as manoeuvring. */
    bool bThrusterAudioWasManeuvering = false;

    /**
     * World time of the last strafe/vertical input strong enough to fire thrusters, stamped in Move().
     * (MoveAction is bound with BindAction, not BindActionValue, so GetBoundActionValue can't be used.)
     */
    double ThrusterAudioLastStrafeTime = -1.0;

    /** Timer for Dock.AirlockHiss, shortly after the clamps engage. */
    FTimerHandle AirlockHissTimerHandle;

    /** Pre-upgrade stats keyed by ShipUpgradeStats name. */
    TMap<FName, float> UpgradeBaseStats;

public:
    /** Flight.CollisionBump on blocking hits (player ship only, rate limited). */
    virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
        FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
    };
