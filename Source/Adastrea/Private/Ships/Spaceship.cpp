#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Ships/SpaceshipControlsComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Stations/SpaceStationModule.h"
#include "Stations/DockingBayModule.h"
#include "Blueprint/UserWidget.h"

ASpaceship::ASpaceship()
{
    // Enable tick for X4-style flight physics
    PrimaryActorTick.bCanEverTick = true;
    InteriorInstance = nullptr;
    SavedExternalPawn = nullptr;
    ShipDataAsset = nullptr;

    // Initialize default movement properties
    DefaultMaxSpeed = 3000.0f;
    DefaultAcceleration = 1000.0f;
    DefaultDeceleration = 1000.0f;
    DefaultTurningBoost = 8.0f;
    TurnRate = 45.0f;

    // Initialize ship status
    CurrentHullIntegrity = 1000.0f;
    MaxHullIntegrity = 1000.0f;
    // Initialize X4-style flight control parameters
    bFlightAssistEnabled = true;              // Flight assist on by default
    RotationDampingFactor = 0.85f;            // Smooth rotation with high damping
    AutoLevelStrength = 0.5f;                 // Moderate auto-leveling
    FlightAssistResponsiveness = 2.0f;        // Responsive but not twitchy
    ThrottlePercentage = 0.0f;                // Start at zero throttle
    ThrottleStep = 10.0f;                     // 10% increments
    ThrottleAdjustmentCooldown = 0.1f;        // 10 adjustments per second max
    bBoostActive = false;
    BoostMultiplier = 2.0f;                   // Double speed when boosting
    bTravelModeActive = false;
    TravelModeMultiplier = 5.0f;              // 5x speed in travel mode
    StrafeIndependence = 0.8f;                // High strafe independence
    MouseFlightSensitivity = 1.0f;            // 1:1 mouse sensitivity

    // Initialize free look camera parameters
    bFreeLookActive = false;
    FreeLookSensitivity = 1.5f;               // Slightly higher sensitivity for free look
    CameraDistance = 800.0f;                  // Default camera distance
    CameraLagSpeed = 10.0f;                   // Smooth position following
    CameraRotationLagSpeed = 10.0f;           // Smooth rotation following
    DoubleClickThreshold = 0.3f;              // 300ms for double-click detection

    // Initialize physics state
    CurrentVelocity = FVector::ZeroVector;
    TargetVelocity = FVector::ZeroVector;
    RotationVelocity = FRotator::ZeroRotator;
    ForwardInput = 0.0f;
    RightInput = 0.0f;
    UpInput = 0.0f;
    YawInput = 0.0f;
    PitchInput = 0.0f;
    RollInput = 0.0f;
    FreeLookRotation = FRotator::ZeroRotator;
    LastFreeLookClickTime = 0.0f;
    LastThrottleAdjustmentTime = 0.0f;

    // Initialize docking system
    NearbyStation = nullptr;
    CurrentDockingPoint = nullptr;
    bIsDocked = false;
    bIsDocking = false;
    DockingRange = 2000.0f;
    DockingPromptWidget = nullptr;
    DockingPromptWidgetClass = nullptr;
    TradingInterfaceClass = nullptr;
    TradingWidget = nullptr;

    // Create and configure the floating pawn movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = DefaultMaxSpeed;
    MovementComponent->Acceleration = DefaultAcceleration;
    MovementComponent->Deceleration = DefaultDeceleration;
    MovementComponent->TurningBoost = DefaultTurningBoost;

    // Create and configure the particle component
    ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));

    // Create and configure camera components
    CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    CameraSpringArm->SetupAttachment(RootComponent);
    CameraSpringArm->TargetArmLength = CameraDistance;
    CameraSpringArm->bUsePawnControlRotation = false; // We'll control this manually for free look
    CameraSpringArm->bEnableCameraLag = true;
    CameraSpringArm->CameraLagSpeed = CameraLagSpeed;
    CameraSpringArm->bEnableCameraRotationLag = true; // Enable smooth rotation following
    CameraSpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
    CameraSpringArm->bDoCollisionTest = false; // Disable collision in space

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
}

void ASpaceship::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** ASpaceship::BeginPlay on %s ***"), *GetName());

    // Initialize hull integrity from data asset if available
    if (ShipDataAsset)
    {
        MaxHullIntegrity = ShipDataAsset->HullStrength;
        CurrentHullIntegrity = MaxHullIntegrity; // Start at full health
    }

    // Spawn the interior actor if needed
    if (!InteriorInstance.IsValid())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        ASpaceshipInterior* SpawnedInterior = GetWorld()->SpawnActor<ASpaceshipInterior>(ASpaceshipInterior::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (SpawnedInterior)
        {
            SpawnedInterior->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
            SpawnedInterior->SetActorHiddenInGame(true); // Hide until entered
            InteriorInstance = SpawnedInterior;
        }
    }
}

#if WITH_EDITOR
void ASpaceship::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Synchronize camera spring arm settings when properties change in editor
    if (PropertyChangedEvent.Property && CameraSpringArm)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();
        
        if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraDistance))
        {
            CameraSpringArm->TargetArmLength = CameraDistance;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraLagSpeed))
        {
            CameraSpringArm->CameraLagSpeed = CameraLagSpeed;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraRotationLagSpeed))
        {
            CameraSpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
        }
    }
}
#endif

void ASpaceship::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** SHIP POSSESSED: %s by controller %s ***"), 
        *GetName(), 
        NewController ? *NewController->GetName() : TEXT("nullptr"));
}

void ASpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only apply flight physics if ship is possessed by a controller
    // This avoids unnecessary CPU usage on unpossessed NPC ships
    if (!GetController())
    {
        return;
    }

    // Apply X4-style flight physics when flight assist is enabled
    if (bFlightAssistEnabled)
    {
        ApplyFlightAssist(DeltaTime);
    }

    // Update velocity based on throttle setting (only when flight assist is on)
    if (bFlightAssistEnabled)
    {
        UpdateThrottleVelocity(DeltaTime);
    }

    // Apply auto-leveling when enabled and no rotation input
    // Note: Currently we don't have explicit roll input, so we check for no yaw input
    // as a proxy for when the player is not actively rotating
    if (bFlightAssistEnabled && FMath::IsNearlyZero(YawInput, 0.01f))
    {
        ApplyAutoLeveling(DeltaTime);
    }
}

void ASpaceship::EnterInterior(APlayerController* PlayerController)
{
    if (InteriorInstance.IsValid() && PlayerController)
    {
        // Hide spaceship exterior, show interior
        InteriorInstance->SetActorHiddenInGame(false);

        // Teleport player to interior start location
        FVector InteriorEntry = InteriorInstance->GetEntryLocation();
        FRotator EntryRotation = InteriorInstance->GetEntryRotation();
        PlayerController->GetPawn()->SetActorLocation(InteriorEntry, false, nullptr, ETeleportType::TeleportPhysics);
        PlayerController->GetPawn()->SetActorRotation(EntryRotation);

        // Optionally, switch possession to a walkable character
        // (Not shown: implement a character class for walking inside)
    }
}

void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::SetupPlayerInputComponent called on %s"), *GetName());

    // Initialize SpaceshipControlsComponent if present (handles basic movement/look/fire)
    USpaceshipControlsComponent* ControlsComponent = FindComponentByClass<USpaceshipControlsComponent>();
    if (ControlsComponent)
    {
        UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Found SpaceshipControlsComponent, initializing input bindings"));
        ControlsComponent->InitializeInputBindings(PlayerInputComponent);
    }
    else
    {
        UE_LOG(LogAdastreaInput, Warning, TEXT("ASpaceship: SpaceshipControlsComponent NOT FOUND on %s"), *GetName());
    }
    
    // Setup Enhanced Input bindings for ASpaceship's input (throttle, free look, etc.)
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceship::Move);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound MoveAction"));
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceship::Look);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound LookAction"));
        }

        if (FreeLookAction)
        {
            // Bind free look action with Started, Triggered, and Completed events
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Started, this, &ASpaceship::FreeLookStarted);
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Triggered, this, &ASpaceship::FreeLookCamera);
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Completed, this, &ASpaceship::FreeLookCompleted);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound FreeLookAction"));
        }

        if (ThrottleUpAction)
        {
            EnhancedInputComponent->BindAction(ThrottleUpAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleUp);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleUpAction"));
        }

        if (ThrottleDownAction)
        {
            EnhancedInputComponent->BindAction(ThrottleDownAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleDown);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleDownAction"));
        }

        if (DockAction)
        {
            EnhancedInputComponent->BindAction(DockAction, ETriggerEvent::Triggered, this, &ASpaceship::RequestDocking);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound DockAction to RequestDocking"));
        }
    }
}

void ASpaceship::Move(const FInputActionValue& Value)
{
    // Get the 3D vector input (WASD + QE for vertical)
    const FVector MovementVector = Value.Get<FVector>();

    // Forward/Backward (W/S)
    MoveForward(MovementVector.X);

    // Left/Right (A/D)
    MoveRight(MovementVector.Y);

    // Up/Down (Q/E)
    MoveUp(MovementVector.Z);
}

void ASpaceship::Look(const FInputActionValue& Value)
{
    // Skip normal look behavior when free look is active
    if (bFreeLookActive)
    {
        return;
    }

    // Get the 2D vector input (mouse X/Y)
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    // Yaw (mouse X)
    Turn(LookAxisVector.X);

    // Pitch (mouse Y)
    LookUp(LookAxisVector.Y);
}

// Legacy input functions - called by Enhanced Input Move/Look functions
void ASpaceship::MoveForward(float Value)
{
    // Store input for smooth interpolation
    ForwardInput = Value;

    // X4-style: With flight assist, WASD provides direct velocity control
    // Without flight assist, it applies Newtonian acceleration to velocity
    if (bFlightAssistEnabled)
    {
        // Flight assist mode: direct velocity control
        AddMovementInput(GetActorForwardVector(), Value);
    }
    else
    {
        // No flight assist: apply Newtonian acceleration
        // Increment velocity in forward direction based on input
        if (GetWorld() && MovementComponent)
        {
            FVector AccelerationVector = GetActorForwardVector() * Value * DefaultAcceleration * GetWorld()->GetDeltaSeconds();
            MovementComponent->Velocity += AccelerationVector;
        }
    }
    
    // Update particle throttle based on forward movement
    if (ParticleComponent)
    {
        ParticleComponent->UpdateThrottle(FMath::Abs(Value));
        
        // Activate RCS thrusters based on direction
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Backward, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, FMath::Abs(Value));
        }
    }
}

void ASpaceship::MoveRight(float Value)
{
    // Store input for smooth interpolation
    RightInput = Value;

    // X4-style: strafe is independent from forward motion
    // Apply strafe independence factor for realistic feel
    float StrafeValue = Value * StrafeIndependence;
    AddMovementInput(GetActorRightVector(), StrafeValue);
    
    // Activate RCS thrusters for strafing
    if (ParticleComponent)
    {
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Left, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Right, FMath::Abs(Value));
        }
    }
}

void ASpaceship::MoveUp(float Value)
{
    // Store input for smooth interpolation
    UpInput = Value;

    // X4-style: vertical strafe is independent from forward motion
    float StrafeValue = Value * StrafeIndependence;
    AddMovementInput(GetActorUpVector(), StrafeValue);
    
    // Activate RCS thrusters for vertical movement
    if (ParticleComponent)
    {
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Down, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Up, FMath::Abs(Value));
        }
    }
}

void ASpaceship::Turn(float Value)
{
    // Store input for smooth rotation
    YawInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        
        if (bFlightAssistEnabled)
        {
            // X4-style: smooth rotation with damping
            // Apply mouse flight sensitivity for better control
            float RotationRate = Value * TurnRate * MouseFlightSensitivity;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            
            // Apply rotation directly to actor
            FRotator DeltaRotation = FRotator(0.0f, RotationVelocity.Yaw * DeltaSeconds, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
        else
        {
            // Without flight assist: direct rotation
            FRotator DeltaRotation = FRotator(0.0f, Value * TurnRate * DeltaSeconds, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
    }
}

void ASpaceship::LookUp(float Value)
{
    // Store input for smooth rotation
    PitchInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        
        if (bFlightAssistEnabled)
        {
            // X4-style: smooth rotation with damping
            float RotationRate = Value * TurnRate * MouseFlightSensitivity;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            
            // Apply rotation directly to actor
            FRotator DeltaRotation = FRotator(RotationVelocity.Pitch * DeltaSeconds, 0.0f, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
        else
        {
            // Without flight assist: direct rotation
            FRotator DeltaRotation = FRotator(Value * TurnRate * DeltaSeconds, 0.0f, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
    }
}

void ASpaceship::Roll(float Value)
{
    // Store input for smooth rotation
    RollInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        
        if (bFlightAssistEnabled)
        {
            // X4-style: smooth rotation with damping
            float RotationRate = Value * TurnRate * MouseFlightSensitivity;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Roll = FMath::FInterpTo(RotationVelocity.Roll, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            
            // Apply roll rotation to actor in local space to avoid gimbal lock
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, RotationVelocity.Roll * DeltaSeconds);
            AddActorLocalRotation(DeltaRotation);
        }
        else
        {
            // Without flight assist: direct rotation
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, Value * TurnRate * DeltaSeconds);
            AddActorLocalRotation(DeltaRotation);
        }
    }
}

void ASpaceship::BeginControl(APlayerController* PC, APawn* ExternalPawn)
{
    if (!PC || !ExternalPawn)
    {
        return;
    }

    // Check if already controlling - prevent overwriting saved pawn
    if (SavedExternalPawn != nullptr)
    {
        return;
    }

    // Store the walking pawn so we can re-possess later
    SavedExternalPawn = ExternalPawn;

    // Disable walking pawn input and hide it
    ExternalPawn->DisableInput(PC);
    ExternalPawn->SetActorHiddenInGame(true);
    ExternalPawn->SetActorEnableCollision(false);

    // Possess the ship
    PC->Possess(this);

    // Ensure movement component is active
    if (MovementComponent)
    {
        MovementComponent->Activate(true);
    }
}

void ASpaceship::EndControl(APlayerController* PC)
{
    if (!PC || !SavedExternalPawn)
    {
        return;
    }

    // Possess back the walking pawn
    PC->Possess(SavedExternalPawn);

    // Restore walking pawn
    SavedExternalPawn->SetActorHiddenInGame(false);
    SavedExternalPawn->SetActorEnableCollision(true);
    SavedExternalPawn->EnableInput(PC);

    // Optionally teleport player to an exit point
    if (InteriorInstance.IsValid())
    {
        FVector ExitLocation = InteriorInstance->GetExitLocation();
        SavedExternalPawn->SetActorLocation(ExitLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    SavedExternalPawn = nullptr;
}

FText ASpaceship::GetShipName() const
{
    // If we have a data asset, use its name
    if (ShipDataAsset)
    {
        return ShipDataAsset->ShipName;
    }
    
    // Otherwise, use the actor's label or name
    FString ActorName = GetName();
    
    return FText::FromString(ActorName);
}

FText ASpaceship::GetShipClass() const
{
    // If we have a data asset, use its class
    if (ShipDataAsset)
    {
        return ShipDataAsset->ShipClass;
    }
    
    // Default fallback
    return FText::FromString("Starship");
}

float ASpaceship::GetCurrentHullIntegrity() const
{
    return CurrentHullIntegrity;
}

float ASpaceship::GetMaxHullIntegrity() const
{
    return MaxHullIntegrity;
}

// ==========================================
// X4-STYLE FLIGHT CONTROL IMPLEMENTATION
// ==========================================

void ASpaceship::ToggleFlightAssist()
{
    bFlightAssistEnabled = !bFlightAssistEnabled;

    // When disabling flight assist, preserve current velocity for inertia
    if (!bFlightAssistEnabled && MovementComponent)
    {
        CurrentVelocity = MovementComponent->Velocity;
    }
}

bool ASpaceship::CanAdjustThrottle()
{
    // Rate limit throttle adjustments to prevent excessively fast changes when button is held
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastThrottleAdjustmentTime < ThrottleAdjustmentCooldown)
    {
        return false; // Too soon, skip this adjustment
    }
    
    LastThrottleAdjustmentTime = CurrentTime;
    return true;
}

void ASpaceship::ThrottleUp()
{
    if (!CanAdjustThrottle())
    {
        return;
    }
    
    ThrottlePercentage = FMath::Clamp(ThrottlePercentage + ThrottleStep, 0.0f, 100.0f);
}

void ASpaceship::ThrottleDown()
{
    if (!CanAdjustThrottle())
    {
        return;
    }
    
    ThrottlePercentage = FMath::Clamp(ThrottlePercentage - ThrottleStep, 0.0f, 100.0f);
}

void ASpaceship::SetThrottle(float Percentage)
{
    ThrottlePercentage = FMath::Clamp(Percentage, 0.0f, 100.0f);
}

void ASpaceship::ActivateBoost()
{
    // Cannot boost in travel mode
    if (!bTravelModeActive)
    {
        bBoostActive = true;
        
        // Update movement component max speed
        if (MovementComponent)
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed * BoostMultiplier;
        }
    }
}

void ASpaceship::DeactivateBoost()
{
    bBoostActive = false;
    
    // Restore normal max speed
    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = bTravelModeActive ? (DefaultMaxSpeed * TravelModeMultiplier) : DefaultMaxSpeed;
    }
}

void ASpaceship::ToggleTravelMode()
{
    bTravelModeActive = !bTravelModeActive;

    // Deactivate boost when entering travel mode
    if (bTravelModeActive && bBoostActive)
    {
        DeactivateBoost();
    }

    // Update movement component max speed
    if (MovementComponent)
    {
        if (bTravelModeActive)
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed * TravelModeMultiplier;
            // Set throttle to 100% for travel mode
            ThrottlePercentage = 100.0f;
        }
        else
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed;
        }
    }
}

float ASpaceship::GetEffectiveMaxSpeed() const
{
    float EffectiveSpeed = DefaultMaxSpeed;

    if (bBoostActive)
    {
        EffectiveSpeed *= BoostMultiplier;
    }
    else if (bTravelModeActive)
    {
        EffectiveSpeed *= TravelModeMultiplier;
    }

    return EffectiveSpeed;
}

void ASpaceship::ApplyFlightAssist(float DeltaTime)
{
    /**
     * X4-style Flight Assist Implementation:
     * 
     * When flight assist is enabled:
     * - Ship maintains orientation when no rotation input is given
     * - Rotation is smoothly damped for natural feel
     * - Velocity is maintained when no movement input is given
     * - Ship responds to throttle setting rather than direct acceleration
     * 
     * This creates the "fly-by-wire" feel of X4: Foundations where the ship
     * computer assists the pilot in maintaining stable flight.
     */

    if (!MovementComponent)
    {
        return;
    }

    // Apply rotation damping when no input
    if (FMath::IsNearlyZero(YawInput, 0.01f))
    {
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    if (FMath::IsNearlyZero(PitchInput, 0.01f))
    {
        RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    if (FMath::IsNearlyZero(RollInput, 0.01f))
    {
        RotationVelocity.Roll = FMath::FInterpTo(RotationVelocity.Roll, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    // Preserve velocity when no movement input (inertia in space)
    if (FMath::IsNearlyZero(ForwardInput, 0.01f) && 
        FMath::IsNearlyZero(RightInput, 0.01f) && 
        FMath::IsNearlyZero(UpInput, 0.01f))
    {
        // In X4, with flight assist, the ship maintains its velocity
        // This is different from atmosphere flight where you'd slow down
        // Preserve the current velocity to maintain inertia
        if (!CurrentVelocity.IsNearlyZero())
        {
            MovementComponent->Velocity = CurrentVelocity;
        }
    }
    else
    {
        // Update CurrentVelocity when there is input
        CurrentVelocity = MovementComponent->Velocity;
    }
}

void ASpaceship::ApplyAutoLeveling(float DeltaTime)
{
    /**
     * X4-style Auto-Leveling:
     * 
     * When no rotation input is given, the ship automatically levels its roll
     * to the ecliptic plane (assuming Z-up in Unreal). This makes it easier
     * to maintain orientation during exploration and combat.
     * 
     * Note: Currently triggered by no yaw input as we don't have explicit roll input.
     * The strength can be tuned via AutoLevelStrength (0 = off, 1 = instant).
     */

    if (AutoLevelStrength <= 0.0f)
    {
        return;
    }

    // Get current rotation
    FRotator CurrentRotation = GetActorRotation();

    // Calculate target rotation with roll = 0 (level to ecliptic)
    FRotator TargetRotation = CurrentRotation;
    TargetRotation.Roll = 0.0f;

    // Smoothly interpolate to level position
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AutoLevelStrength * 2.0f);
    SetActorRotation(NewRotation);
}

void ASpaceship::UpdateThrottleVelocity(float DeltaTime)
{
    /**
     * X4-style Throttle System:
     * 
     * The throttle (0-100%) controls the target velocity, not acceleration directly.
     * The ship will automatically accelerate or decelerate to match the throttle setting.
     * 
     * This creates intuitive speed control where:
     * - Throttle 0% = ship comes to a stop
     * - Throttle 50% = ship maintains half speed
     * - Throttle 100% = ship maintains max speed
     * 
     * The flight computer handles the acceleration curve automatically.
     */

    if (!MovementComponent)
    {
        return;
    }

    // Calculate target speed based on throttle percentage
    float EffectiveMaxSpeed = GetEffectiveMaxSpeed();
    float TargetSpeed = (ThrottlePercentage / 100.0f) * EffectiveMaxSpeed;

    // Get current forward velocity
    FVector ForwardVector = GetActorForwardVector();
    float CurrentForwardSpeed = FVector::DotProduct(MovementComponent->Velocity, ForwardVector);

    // Calculate desired velocity
    TargetVelocity = ForwardVector * TargetSpeed;

    // Smoothly interpolate current velocity towards target
    // Use different interpolation speed for acceleration vs deceleration
    float InterpSpeed = (CurrentForwardSpeed < TargetSpeed) ? 
        (DefaultAcceleration / EffectiveMaxSpeed) * FlightAssistResponsiveness :
        (DefaultDeceleration / EffectiveMaxSpeed) * FlightAssistResponsiveness;

    // Apply velocity interpolation only if flight assist is on
    if (bFlightAssistEnabled)
    {
        // Blend current velocity with target throttle velocity
        FVector BlendedVelocity = FMath::VInterpTo(MovementComponent->Velocity, TargetVelocity, DeltaTime, InterpSpeed);
        
        // Update movement component with new velocity
        // Note: This works in conjunction with AddMovementInput for strafe/vertical
        MovementComponent->Velocity = BlendedVelocity;
    }
}

void ASpaceship::FreeLookStarted()
{
    // Check for double-click to reset camera
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float TimeSinceLastClick = CurrentTime - LastFreeLookClickTime;
    
    // If this is a double-click (within threshold), reset camera and exit free look
    // Check if we're within the double-click window, regardless of free look state
    if (TimeSinceLastClick > 0.0f && TimeSinceLastClick <= DoubleClickThreshold)
    {
        UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look double-click detected - resetting camera"));
        
        // Immediately reset camera to ship forward
        if (CameraSpringArm)
        {
            CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
        }
        
        // Exit free look mode if active, or prevent activation if not yet active
        bFreeLookActive = false;
        FreeLookRotation = FRotator::ZeroRotator;
        LastFreeLookClickTime = 0.0f; // Reset to prevent triple-click issues
        return;
    }
    
    // Store the click time for double-click detection
    LastFreeLookClickTime = CurrentTime;
    
    // Normal free look activation
    bFreeLookActive = true;
    
    // Reset free look rotation for new activation
    FreeLookRotation = FRotator::ZeroRotator;
    
    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look started"));
}

void ASpaceship::FreeLookCompleted()
{
    bFreeLookActive = false;
    
    // Reset camera to follow ship rotation
    if (CameraSpringArm)
    {
        // Smoothly return camera to ship's forward direction
        CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
    }
    
    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look completed"));
}

void ASpaceship::FreeLookCamera(const FInputActionValue& Value)
{
    if (!bFreeLookActive || !CameraSpringArm)
    {
        return;
    }

    // Get the 2D vector input (mouse X/Y)
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        
        // Apply free look sensitivity to mouse input
        float YawDelta = LookAxisVector.X * FreeLookSensitivity * TurnRate * DeltaSeconds;
        float PitchDelta = LookAxisVector.Y * FreeLookSensitivity * TurnRate * DeltaSeconds;
        
        // Accumulate free look rotation
        FreeLookRotation.Yaw += YawDelta;
        FreeLookRotation.Pitch += PitchDelta;
        
        // Clamp pitch to prevent camera flipping
        FreeLookRotation.Pitch = FMath::Clamp(FreeLookRotation.Pitch, -89.0f, 89.0f);
        
        // Apply free look rotation relative to ship's current rotation
        FRotator NewCameraRotation = GetActorRotation() + FreeLookRotation;
        CameraSpringArm->SetWorldRotation(NewCameraRotation);
    }
}

// ==========================================
// DOCKING SYSTEM IMPLEMENTATION
// ==========================================

void ASpaceship::SetNearbyStation(USpaceStationModule* Station)
{
    NearbyStation = Station;
}

void ASpaceship::ShowDockingPrompt(bool bShow)
{
    if (bShow)
    {
        // Create widget if it doesn't exist
        if (!DockingPromptWidget && DockingPromptWidgetClass)
        {
            APlayerController* PC = Cast<APlayerController>(GetController());
            if (PC)
            {
                DockingPromptWidget = CreateWidget<UUserWidget>(PC, DockingPromptWidgetClass);
                if (DockingPromptWidget)
                {
                    DockingPromptWidget->AddToViewport();
                }
            }
        }
        else if (!DockingPromptWidgetClass)
        {
            UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::ShowDockingPrompt - DockingPromptWidgetClass is not set on '%s'. Docking prompt UI will not be shown."), *GetName());
        }
        
        // Show existing widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Hide widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ASpaceship::RequestDocking()
{
    // Validate nearby station exists
    if (!NearbyStation)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No station in range"));
        // TODO: Show user feedback via HUD message
        return;
    }
    
    // If already docked, undock instead
    if (bIsDocked)
    {
        Undock();
        return;
    }
    
    // Cast to docking bay module to check availability
    ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
    if (!DockingBay)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Station is not a docking module"));
        // TODO: Show user feedback via HUD message
        return;
    }
    
    // Check if docking is available
    if (!DockingBay->HasAvailableDocking())
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No docking slots available"));
        // TODO: Show user feedback via HUD message
        return;
    }
    
    // Get available docking point
    USceneComponent* DockingPoint = DockingBay->GetAvailableDockingPoint();
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Failed to get docking point"));
        // TODO: Show user feedback via HUD message
        return;
    }
    
    // Check if ship is within docking range
    float DistanceToDockingPoint = FVector::Dist(GetActorLocation(), DockingPoint->GetComponentLocation());
    if (DistanceToDockingPoint > DockingRange)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Too far from docking point (%.0f > %.0f)"), DistanceToDockingPoint, DockingRange);
        // TODO: Show user feedback via HUD message
        return;
    }
    
    // Store docking point and begin docking sequence
    CurrentDockingPoint = DockingPoint;
    bIsDocking = true;
    
    // Navigate to docking point (instant in simplified version)
    NavigateToDockingPoint(CurrentDockingPoint);
}

void ASpaceship::NavigateToDockingPoint(USceneComponent* DockingPoint)
{
    // Validate docking point
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::NavigateToDockingPoint - Invalid docking point"));
        bIsDocking = false;
        return;
    }
    
    // Simple MVP approach: instantly teleport to docking point
    // Get target transform from docking point
    FVector TargetLocation = DockingPoint->GetComponentLocation();
    FRotator TargetRotation = DockingPoint->GetComponentRotation();
    
    // Instantly move ship to docking point
    SetActorLocationAndRotation(TargetLocation, TargetRotation);
    
    // Immediately complete docking
    CompleteDocking();
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::NavigateToDockingPoint - Instantly docked at point"));
}

void ASpaceship::CompleteDocking()
{
    // Update docking state
    bIsDocked = true;
    bIsDocking = false;
    
    // Notify station that ship has docked
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->DockShip();
        }
    }
    
    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }
    
    // Disable input
    DisableInput(PC);
    
    // Hide ship
    SetActorHiddenInGame(true);
    
    // Create and show trading widget
    if (TradingInterfaceClass)
    {
        TradingWidget = CreateWidget<UUserWidget>(PC, TradingInterfaceClass);
        if (TradingWidget)
        {
            TradingWidget->AddToViewport();
        }
    }
    else
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::CompleteDocking - TradingInterfaceClass is not set on '%s'. Trading UI will not be created."), *GetName());
    }
    
    // Set input mode to UI only
    PC->bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
    if (TradingWidget)
    {
        InputMode.SetWidgetToFocus(TradingWidget->TakeWidget());
    }
    PC->SetInputMode(InputMode);
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::CompleteDocking - Docking complete for '%s'"), *GetName());
}

void ASpaceship::Undock()
{
    // Check if actually docked
    if (!bIsDocked)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::Undock - Not currently docked"));
        return;
    }
    
    // Notify station that ship is undocking
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->UndockShip();
        }
    }
    
    // Update state
    bIsDocked = false;
    
    // Remove trading widget
    if (TradingWidget)
    {
        TradingWidget->RemoveFromParent();
        TradingWidget = nullptr;
    }
    
    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }
    
    // Enable input
    EnableInput(PC);
    
    // Show ship
    SetActorHiddenInGame(false);
    
    // Set input mode to game only
    PC->bShowMouseCursor = false;
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    
    // Apply forward impulse to move away from station
    FVector ForwardVector = GetActorForwardVector();
    if (MovementComponent)
    {
        // Add velocity in forward direction for smooth movement away
        MovementComponent->Velocity += ForwardVector * 500.0f;
    }
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::Undock - Undocked successfully from '%s'"), NearbyStation ? *NearbyStation->GetName() : TEXT("Unknown Station"));
}
