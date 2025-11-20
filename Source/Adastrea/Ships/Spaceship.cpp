#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASpaceship::ASpaceship()
{
    // Enable tick for X4-style flight physics
    PrimaryActorTick.bCanEverTick = true;
    InteriorInstance = nullptr;
    SavedExternalPawn = nullptr;

    // Initialize default movement properties
    DefaultMaxSpeed = 3000.0f;
    DefaultAcceleration = 1000.0f;
    DefaultDeceleration = 1000.0f;
    DefaultTurningBoost = 8.0f;
    TurnRate = 45.0f;

    // Initialize X4-style flight control parameters
    bFlightAssistEnabled = true;              // Flight assist on by default
    RotationDampingFactor = 0.85f;            // Smooth rotation with high damping
    AutoLevelStrength = 0.5f;                 // Moderate auto-leveling
    FlightAssistResponsiveness = 2.0f;        // Responsive but not twitchy
    ThrottlePercentage = 0.0f;                // Start at zero throttle
    ThrottleStep = 10.0f;                     // 10% increments
    bBoostActive = false;
    BoostMultiplier = 2.0f;                   // Double speed when boosting
    bTravelModeActive = false;
    TravelModeMultiplier = 5.0f;              // 5x speed in travel mode
    StrafeIndependence = 0.8f;                // High strafe independence
    MouseFlightSensitivity = 1.0f;            // 1:1 mouse sensitivity

    // Initialize physics state
    CurrentVelocity = FVector::ZeroVector;
    TargetVelocity = FVector::ZeroVector;
    RotationVelocity = FRotator::ZeroRotator;
    ForwardInput = 0.0f;
    RightInput = 0.0f;
    UpInput = 0.0f;
    YawInput = 0.0f;
    PitchInput = 0.0f;

    // Create and configure the floating pawn movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = DefaultMaxSpeed;
    MovementComponent->Acceleration = DefaultAcceleration;
    MovementComponent->Deceleration = DefaultDeceleration;
    MovementComponent->TurningBoost = DefaultTurningBoost;

    // Create and configure the particle component
    ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));
}

void ASpaceship::BeginPlay()
{
    Super::BeginPlay();

    // Spawn the interior actor if needed
    if (!InteriorInstance)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        InteriorInstance = GetWorld()->SpawnActor<ASpaceshipInterior>(ASpaceshipInterior::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (InteriorInstance)
        {
            InteriorInstance->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
            InteriorInstance->SetActorHiddenInGame(true); // Hide until entered
        }
    }
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
    if (InteriorInstance && PlayerController)
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

    if (PlayerInputComponent)
    {
        // Bind axis inputs for movement
        PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ASpaceship::MoveForward);
        PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ASpaceship::MoveRight);
        PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &ASpaceship::MoveUp);
        PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ASpaceship::Turn);
        PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ASpaceship::LookUp);

        // Bind X4-style flight control actions
        // Note: These use legacy input system for backward compatibility
        // For Enhanced Input, these should be bound in Blueprint or via InputConfigDataAsset
        PlayerInputComponent->BindAction(TEXT("ThrottleUp"), IE_Pressed, this, &ASpaceship::ThrottleUp);
        PlayerInputComponent->BindAction(TEXT("ThrottleDown"), IE_Pressed, this, &ASpaceship::ThrottleDown);
        PlayerInputComponent->BindAction(TEXT("ToggleFlightAssist"), IE_Pressed, this, &ASpaceship::ToggleFlightAssist);
        PlayerInputComponent->BindAction(TEXT("ActivateBoost"), IE_Pressed, this, &ASpaceship::ActivateBoost);
        PlayerInputComponent->BindAction(TEXT("DeactivateBoost"), IE_Released, this, &ASpaceship::DeactivateBoost);
        PlayerInputComponent->BindAction(TEXT("ToggleTravelMode"), IE_Pressed, this, &ASpaceship::ToggleTravelMode);
    }
}

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
            float RotationRate = Value * TurnRate * MouseFlightSensitivity * DeltaSeconds;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            AddControllerYawInput(RotationVelocity.Yaw);
        }
        else
        {
            // Without flight assist: direct rotation
            AddControllerYawInput(Value * TurnRate * DeltaSeconds);
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
            float RotationRate = Value * TurnRate * MouseFlightSensitivity * DeltaSeconds;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            AddControllerPitchInput(RotationVelocity.Pitch);
        }
        else
        {
            // Without flight assist: direct rotation
            AddControllerPitchInput(Value * TurnRate * DeltaSeconds);
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
    if (InteriorInstance)
    {
        FVector ExitLocation = InteriorInstance->GetExitLocation();
        SavedExternalPawn->SetActorLocation(ExitLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    SavedExternalPawn = nullptr;
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

void ASpaceship::ThrottleUp()
{
    ThrottlePercentage = FMath::Clamp(ThrottlePercentage + ThrottleStep, 0.0f, 100.0f);
}

void ASpaceship::ThrottleDown()
{
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