#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Factions/FactionDataAsset.h"
#include "AdastreaLog.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

ASpaceship::ASpaceship()
{
    // Enable tick for X4-style flight physics
    PrimaryActorTick.bCanEverTick = true;
    InteriorInstance = nullptr;
    SavedExternalPawn = nullptr;
    ShipDataAsset = nullptr;
    ShipFaction = nullptr;

    // Initialize default movement properties
    DefaultMaxSpeed = 3000.0f;
    DefaultAcceleration = 1000.0f;
    DefaultDeceleration = 1000.0f;
    DefaultTurningBoost = 8.0f;
    TurnRate = 45.0f;

    // Initialize ship status
    CurrentHullIntegrity = 1000.0f;
    MaxHullIntegrity = 1000.0f;
    bIsDestroyed = false;
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
    
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** ASpaceship::BeginPlay on %s ***"), *GetName());

    // Initialize hull integrity from data asset if available
    if (ShipDataAsset)
    {
        MaxHullIntegrity = ShipDataAsset->HullStrength;
        CurrentHullIntegrity = MaxHullIntegrity; // Start at full health
    }

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

void ASpaceship::PossessedBy(AController* NewController)
{
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** POSSESS CALLED: %s current controller=%s, new controller=%s ***"),
        *GetName(),
        GetController() ? *GetController()->GetName() : TEXT("nullptr"),
        NewController ? *NewController->GetName() : TEXT("nullptr"));

    // Check if this controller is already possessing this pawn
    if (GetController() == NewController)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("*** DUPLICATE POSSESS PREVENTED: %s is already possessed by controller %s ***"),
            *GetName(),
            NewController ? *NewController->GetName() : TEXT("nullptr"));
        return;
    }

    Super::PossessedBy(NewController);
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** SHIP POSSESSED: %s by controller %s ***"),
        *GetName(),
        NewController ? *NewController->GetName() : TEXT("nullptr"));
}void ASpaceship::Tick(float DeltaTime)
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

    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::SetupPlayerInputComponent called on %s"), *GetName());

    // Setup Enhanced Input bindings for ASpaceship's input
    // Always use direct binding to MoveAction and LookAction UPROPERTYs for simplicity
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceship::Move);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound MoveAction"));
        }
        else
        {
            UE_LOG(LogAdastreaInput, Error, TEXT("ASpaceship: MoveAction is not assigned! Set it to IA_Move in the Blueprint"));
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceship::Look);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound LookAction"));
        }
        else
        {
            UE_LOG(LogAdastreaInput, Error, TEXT("ASpaceship: LookAction is not assigned! Set it to IA_Look in the Blueprint"));
        }

        if (ThrottleUpAction)
        {
            EnhancedInputComponent->BindAction(ThrottleUpAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleUp);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleUpAction"));
        }
        else
        {
            UE_LOG(LogAdastreaInput, Error, TEXT("ASpaceship: ThrottleUpAction is not assigned! Set it to IA_ThrottleUp in the Blueprint"));
        }

        if (ThrottleDownAction)
        {
            EnhancedInputComponent->BindAction(ThrottleDownAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleDown);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleDownAction"));
        }
        else
        {
            UE_LOG(LogAdastreaInput, Error, TEXT("ASpaceship: ThrottleDownAction is not assigned! Set it to IA_ThrottleDown in the Blueprint"));
        }
    }
    else
    {
        UE_LOG(LogAdastreaInput, Error, TEXT("ASpaceship: PlayerInputComponent is not an EnhancedInputComponent!"));
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
        // Flight assist mode: direct velocity control, scaled by throttle
        AddMovementInput(GetActorForwardVector(), Value * (ThrottlePercentage / 100.0f));
    }
    else
    {
        // No flight assist: apply Newtonian acceleration, scaled by throttle
        // Increment velocity in forward direction based on input
        if (GetWorld() && MovementComponent)
        {
            FVector AccelerationVector = GetActorForwardVector() * Value * (ThrottlePercentage / 100.0f) * DefaultAcceleration * GetWorld()->GetDeltaSeconds();
            MovementComponent->Velocity += AccelerationVector;
        }
    }

    // Debug drawing for forward movement
    if (Value != 0.0f)
    {
        FVector Start = GetActorLocation();
        FVector End = Start + (GetActorForwardVector() * Value * 100.0f); // Scale for visualization
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f, 0, 2.0f);
    }
}

void ASpaceship::MoveRight(float Value)
{
    // Store input for smooth interpolation
    RightInput = Value;

    // X4-style: strafe is independent from forward motion and throttle
    // Apply strafe independence factor for realistic feel (no throttle scaling for thrusters)
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

    // Debug drawing for right movement
    if (Value != 0.0f)
    {
        FVector Start = GetActorLocation();
        FVector End = Start + (GetActorRightVector() * Value * 100.0f); // Scale for visualization
        DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.1f, 0, 2.0f);
    }
}

void ASpaceship::MoveUp(float Value)
{
    // Store input for smooth interpolation
    UpInput = Value;

    // X4-style: vertical strafe is independent from forward motion and throttle
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

FText ASpaceship::GetShipName() const
{
    // If we have a data asset, use its name
    if (ShipDataAsset)
    {
        return ShipDataAsset->ShipName;
    }
    
    // Otherwise, use the actor's label or name
    FString ActorName = GetActorLabel();
    if (ActorName.IsEmpty())
    {
        ActorName = GetName();
    }
    
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

void ASpaceship::ThrottleUp()
{
    ThrottlePercentage = FMath::Clamp(ThrottlePercentage + ThrottleStep, 0.0f, 100.0f);
    UE_LOG(LogAdastreaInput, Log, TEXT("Throttle UP: %f%%"), ThrottlePercentage);
}

void ASpaceship::ThrottleDown()
{
    ThrottlePercentage = FMath::Clamp(ThrottlePercentage - ThrottleStep, 0.0f, 100.0f);
    UE_LOG(LogAdastreaInput, Log, TEXT("Throttle DOWN: %f%%"), ThrottlePercentage);
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

float ASpaceship::GetForwardSpeed() const
{
    // Return 0 if no movement component
    if (!MovementComponent)
    {
        return 0.0f;
    }

    // Get the ship's forward vector
    FVector ForwardVector = GetActorForwardVector();
    
    // Project the velocity onto the forward vector to get forward speed
    // This gives us the speed in the direction the ship is facing
    float ForwardSpeed = FVector::DotProduct(MovementComponent->Velocity, ForwardVector);
    
    return ForwardSpeed;
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

// ====================
// IDamageable Interface Implementation
// ====================

float ASpaceship::ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser)
{
    if (!CanTakeDamage_Implementation())
    {
        return 0.0f;
    }

    // TODO: Apply damage type modifiers based on ship armor/shields
    // For now, apply damage directly to hull
    float ActualDamage = FMath::Min(Damage, CurrentHullIntegrity);
    CurrentHullIntegrity -= ActualDamage;

    UE_LOG(LogAdastreaShips, Log, TEXT("%s took %.1f damage from %s. Health: %.1f/%.1f"),
        *GetName(), ActualDamage,
        Instigator ? *Instigator->GetName() : TEXT("Unknown"),
        CurrentHullIntegrity, MaxHullIntegrity);

    // Check if ship is destroyed
    if (CurrentHullIntegrity <= 0.0f)
    {
        CurrentHullIntegrity = 0.0f;
        bIsDestroyed = true;
        UE_LOG(LogAdastreaShips, Warning, TEXT("%s has been destroyed!"), *GetName());
        // TODO: Trigger destruction effects, loot spawning, etc.
    }

    return ActualDamage;
}

bool ASpaceship::CanTakeDamage_Implementation() const
{
    // Cannot take damage if already destroyed
    if (bIsDestroyed)
    {
        return false;
    }

    // TODO: Add additional checks for:
    // - Invulnerability (quest protection, safe zones)
    // - Docked status (if docked at safe station)
    
    return true;
}

float ASpaceship::GetHealthPercentage_Implementation() const
{
    if (MaxHullIntegrity <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentHullIntegrity / MaxHullIntegrity;
}

bool ASpaceship::IsDestroyed_Implementation() const
{
    return bIsDestroyed;
}

float ASpaceship::GetMaxHealth_Implementation() const
{
    return MaxHullIntegrity;
}

float ASpaceship::GetCurrentHealth_Implementation() const
{
    return CurrentHullIntegrity;
}

// ====================
// ITargetable Interface Implementation
// ====================

bool ASpaceship::CanBeTargeted_Implementation() const
{
    // Cannot target destroyed ships
    if (bIsDestroyed)
    {
        return false;
    }

    // TODO: Add checks for:
    // - Stealth/cloaking
    // - Sensor range
    // - Safe zones

    return true;
}

int32 ASpaceship::GetTargetPriority_Implementation() const
{
    // Base priority on ship class from DataAsset
    // TODO: Implement more sophisticated priority based on threat level
    if (ShipDataAsset)
    {
        // Use combat rating as priority indicator (0-100)
        return FMath::RoundToInt(ShipDataAsset->GetCombatRating());
    }

    return 50; // Default medium priority
}

FText ASpaceship::GetTargetDisplayName_Implementation() const
{
    return GetShipName();
}

UTexture2D* ASpaceship::GetTargetIcon_Implementation() const
{
    // TODO: Return ship icon from DataAsset
    // For now return nullptr, UI can use default icon
    return nullptr;
}

FVector ASpaceship::GetAimPoint_Implementation() const
{
    // Return center of actor (aim for center mass)
    return GetActorLocation();
}

float ASpaceship::GetTargetSignature_Implementation() const
{
    // TODO: Implement signature based on:
    // - Ship size
    // - Power output
    // - Stealth systems
    // - Transponder status
    
    return 1.0f; // Default normal signature
}

float ASpaceship::GetDistanceFromLocation_Implementation(FVector FromLocation) const
{
    return FVector::Dist(GetActorLocation(), FromLocation);
}

bool ASpaceship::IsHostileToActor_Implementation(AActor* Observer) const
{
    if (!Observer)
    {
        return false;
    }

    // Check if observer implements IFactionMember
    if (Observer->Implements<UFactionMember>())
    {
        IFactionMember* ObserverFaction = Cast<IFactionMember>(Observer);
        if (ObserverFaction)
        {
            return IsHostileTo_Implementation(TScriptInterface<IFactionMember>(Observer));
        }
    }

    // Default to non-hostile if can't determine faction
    return false;
}

// ====================
// IFactionMember Interface Implementation
// ====================

UFactionDataAsset* ASpaceship::GetFaction_Implementation() const
{
    return ShipFaction;
}

bool ASpaceship::IsAlliedWith_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject())
    {
        return false;
    }

    // Same faction = always allied
    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (ShipFaction && OtherFaction && ShipFaction == OtherFaction)
    {
        return true;
    }

    // Check diplomatic relations
    int32 Relationship = GetRelationshipWith_Implementation(Other);
    return Relationship >= 26; // Friendly or allied
}

bool ASpaceship::IsHostileTo_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject())
    {
        return false;
    }

    // Same faction = never hostile
    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (ShipFaction && OtherFaction && ShipFaction == OtherFaction)
    {
        return false;
    }

    // Check diplomatic relations
    int32 Relationship = GetRelationshipWith_Implementation(Other);
    return Relationship <= -26; // Hostile or at war
}

int32 ASpaceship::GetRelationshipWith_Implementation(const TScriptInterface<IFactionMember>& Other) const
{
    if (!Other.GetObject() || !ShipFaction)
    {
        return 0; // Neutral if no faction
    }

    UFactionDataAsset* OtherFaction = IFactionMember::Execute_GetFaction(Other.GetObject());
    if (!OtherFaction)
    {
        return 0; // Neutral if other has no faction
    }

    if (ShipFaction == OtherFaction)
    {
        return 100; // Same faction = maximum positive relationship
    }

    // TODO: Query faction relationship from FactionDiplomacyManager
    // For now return neutral
    return 0;
}

bool ASpaceship::IsNeutral_Implementation() const
{
    return ShipFaction == nullptr;
}

FText ASpaceship::GetFactionDisplayName_Implementation() const
{
    if (ShipFaction)
    {
        return ShipFaction->FactionName;
    }

    return FText::FromString(TEXT("Independent"));
}

bool ASpaceship::CanEngageInCombat_Implementation() const
{
    // Cannot engage if destroyed
    if (bIsDestroyed)
    {
        return false;
    }

    // TODO: Add checks for:
    // - Docked status
    // - Safe zones
    // - Quest protection

    return true;
}

float ASpaceship::GetTradePriceModifier_Implementation(UFactionDataAsset* TraderFaction) const
{
    if (!ShipFaction || !TraderFaction)
    {
        return 1.0f; // Base price if no faction
    }

    if (ShipFaction == TraderFaction)
    {
        return 0.9f; // 10% discount for same faction
    }

    // TODO: Calculate modifier based on faction relationship
    // Positive relationship = better prices
    // Negative relationship = worse prices
    
    return 1.0f; // Base price for now
}