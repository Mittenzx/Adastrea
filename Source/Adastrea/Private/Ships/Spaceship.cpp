#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Ships/DockingSettingsDataAsset.h"
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

// Debug flag for docking system - can be disabled for shipping builds
#ifndef DOCKING_DEBUG_ENABLED
    #define DOCKING_DEBUG_ENABLED 1
#endif

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
    bUseMousePositionFlight = true;           // X4-style mouse position flight by default
    MouseDeadzoneRadius = 50.0f;              // 50 pixel deadzone from center
    MouseMaxRadius = 400.0f;                  // 400 pixels for 100% rotation speed

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

    // Update X4-style mouse position flight controls every frame (when enabled)
    // This allows continuous rotation based on cursor position, not mouse movement
    if (bUseMousePositionFlight && bFlightAssistEnabled)
    {
        UpdateMousePositionFlight(DeltaTime);
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

    // Setup Enhanced Input bindings for ASpaceship's input (throttle, free look, etc.)
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Check if ASpaceship has its own configured input actions
        bool bHasOwnActions = (MoveAction != nullptr || LookAction != nullptr);
        
        if (bHasOwnActions)
        {
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Using ASpaceship's own input actions (MoveAction=%s, LookAction=%s)"), 
                MoveAction ? TEXT("Valid") : TEXT("NULL"),
                LookAction ? TEXT("Valid") : TEXT("NULL"));
            
            // Use ASpaceship's sophisticated input handling
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
        }
        else
        {
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: No configured input actions, delegating to SpaceshipControlsComponent"));
            
            // Initialize SpaceshipControlsComponent if present (handles basic movement/look/fire)
            USpaceshipControlsComponent* ControlsComponent = FindComponentByClass<USpaceshipControlsComponent>();
            if (ControlsComponent)
            {
                UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Found SpaceshipControlsComponent, initializing input bindings"));
                ControlsComponent->InitializeInputBindings(PlayerInputComponent);
            }
            else
            {
                UE_LOG(LogAdastreaInput, Warning, TEXT("ASpaceship: No input actions configured and SpaceshipControlsComponent NOT FOUND on %s"), *GetName());
            }
        }

        // Always bind free look, throttle, and dock actions (these are ASpaceship-specific)
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
    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::Move - MovementVector: X=%.2f Y=%.2f Z=%.2f"), 
        MovementVector.X, MovementVector.Y, MovementVector.Z);

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
    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::Look - LookAxisVector: X=%.2f Y=%.2f"), 
        LookAxisVector.X, LookAxisVector.Y);

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
            // Get ship-specific rotation rate multiplier from data asset
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }
            
            // Mouse position flight is now handled in UpdateMousePositionFlight() called from Tick()
            // This function only handles mouse delta mode (when bUseMousePositionFlight is false)
            if (!bUseMousePositionFlight)
            {
                // Original X4-style smooth rotation with damping (mouse delta mode)
                // Apply mouse flight sensitivity and ship rotation multiplier
                float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
                
                UE_LOG(LogAdastreaInput, Verbose, TEXT("ASpaceship::Turn - YawInput=%.2f, RotationRate=%.2f"), 
                    Value, RotationRate);
                
                // Interpolate rotation velocity for smooth feel
                RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
                
                // Apply rotation directly to actor
                FRotator DeltaRotation = FRotator(0.0f, RotationVelocity.Yaw * DeltaSeconds, 0.0f);
                AddActorWorldRotation(DeltaRotation);
            }
            // else: Mouse position mode - rotation handled in UpdateMousePositionFlight()
        }
        else
        {
            // Without flight assist: direct rotation
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }
            
            FRotator DeltaRotation = FRotator(0.0f, Value * TurnRate * ShipRotationMultiplier * DeltaSeconds, 0.0f);
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
            // Get ship-specific rotation rate multiplier from data asset
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }
            
            // Mouse position flight is now handled in UpdateMousePositionFlight() called from Tick()
            // This function only handles mouse delta mode (when bUseMousePositionFlight is false)
            if (!bUseMousePositionFlight)
            {
                // Original X4-style smooth rotation with damping (mouse delta mode)
                float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
                
                UE_LOG(LogAdastreaInput, Verbose, TEXT("ASpaceship::LookUp - PitchInput=%.2f, RotationRate=%.2f"), 
                    Value, RotationRate);
                
                // Interpolate rotation velocity for smooth feel
                RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
                
                // Apply rotation directly to actor
                FRotator DeltaRotation = FRotator(RotationVelocity.Pitch * DeltaSeconds, 0.0f, 0.0f);
                AddActorWorldRotation(DeltaRotation);
            }
            // else: Mouse position mode - rotation handled in UpdateMousePositionFlight()
        }
        else
        {
            // Without flight assist: direct rotation
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }
            
            FRotator DeltaRotation = FRotator(Value * TurnRate * ShipRotationMultiplier * DeltaSeconds, 0.0f, 0.0f);
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
        
        // Get ship-specific rotation rate multiplier from data asset
        float ShipRotationMultiplier = 1.0f;
        if (ShipDataAsset)
        {
            ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
        }
        
        if (bFlightAssistEnabled)
        {
            // X4-style: smooth rotation with damping
            float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
            
            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Roll = FMath::FInterpTo(RotationVelocity.Roll, RotationRate, DeltaSeconds, FlightAssistResponsiveness);
            
            // Apply roll rotation to actor in local space to avoid gimbal lock
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, RotationVelocity.Roll * DeltaSeconds);
            AddActorLocalRotation(DeltaRotation);
        }
        else
        {
            // Without flight assist: direct rotation
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, Value * TurnRate * ShipRotationMultiplier * DeltaSeconds);
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

void ASpaceship::UpdateMousePositionFlight(float DeltaTime)
{
    /**
     * X4-style Mouse Position Flight Implementation:
     * 
     * This method is called every frame when mouse position flight is enabled.
     * Unlike mouse delta controls, this continuously checks the mouse cursor position
     * and rotates the ship based on where the cursor is on screen, not how the mouse moves.
     * 
     * Key Behavior:
     * - If mouse is right of center → Ship continuously yaws right
     * - If mouse is left of center → Ship continuously yaws left
     * - If mouse is above center → Ship continuously pitches up
     * - If mouse is below center → Ship continuously pitches down
     * - Distance from center controls rotation speed (0% at deadzone, 100% at max radius)
     * - Mouse can remain stationary and ship will continue rotating
     * 
     * This creates the intuitive feel of X4: Foundations where you "point" with the cursor.
     */

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // Get ship-specific rotation rate multiplier from data asset
    float ShipRotationMultiplier = 1.0f;
    if (ShipDataAsset)
    {
        ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
    }

    // Get mouse position and viewport size
    int32 ViewportSizeX, ViewportSizeY;
    float MouseX, MouseY;
    
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    PC->GetMousePosition(MouseX, MouseY);
    
    // Calculate center of screen
    float CenterX = ViewportSizeX * 0.5f;
    float CenterY = ViewportSizeY * 0.5f;
    
    // Calculate distance from center
    float DeltaX = MouseX - CenterX;
    float DeltaY = MouseY - CenterY;
    float DistanceFromCenter = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
    
    // Check if within deadzone (no rotation)
    if (DistanceFromCenter < MouseDeadzoneRadius)
    {
        // Within deadzone, smoothly stop rotation
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, DeltaTime, FlightAssistResponsiveness);
        RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, 0.0f, DeltaTime, FlightAssistResponsiveness);
        
        // No rotational input intent while inside deadzone
        YawInput = 0.0f;
        PitchInput = 0.0f;
        return;
    }
    
    // Calculate rotation speed based on distance from center (beyond deadzone)
    float EffectiveDistance = DistanceFromCenter - MouseDeadzoneRadius;
    float MaxEffectiveDistance = MouseMaxRadius - MouseDeadzoneRadius;
    
    // Protect against invalid configuration
    if (MaxEffectiveDistance <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogAdastreaInput, Warning, 
            TEXT("ASpaceship::UpdateMousePositionFlight - Invalid mouse radius configuration: MouseMaxRadius (%.2f) must be greater than MouseDeadzoneRadius (%.2f)."),
            MouseMaxRadius, MouseDeadzoneRadius);
        MaxEffectiveDistance = KINDA_SMALL_NUMBER;
    }
    
    float DistanceRatio = FMath::Clamp(EffectiveDistance / MaxEffectiveDistance, 0.0f, 1.0f);
    
    // Calculate rotation rates for yaw and pitch
    // Direction is normalized (-1 to 1) and multiplied by distance ratio for speed
    float DirectionX = DeltaX / FMath::Max(DistanceFromCenter, 0.1f);
    float DirectionY = -DeltaY / FMath::Max(DistanceFromCenter, 0.1f); // Inverted for natural pitch
    
    float YawRotationRate = DirectionX * DistanceRatio * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
    float PitchRotationRate = DirectionY * DistanceRatio * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
    
    UE_LOG(LogAdastreaInput, Verbose, 
        TEXT("ASpaceship::UpdateMousePositionFlight - MousePos=(%.0f,%.0f), Center=(%.0f,%.0f), Distance=%.0f, DistanceRatio=%.2f, YawRate=%.2f, PitchRate=%.2f"), 
        MouseX, MouseY, CenterX, CenterY, DistanceFromCenter, DistanceRatio, YawRotationRate, PitchRotationRate);
    
    // Interpolate rotation velocity for smooth feel
    RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, YawRotationRate, DeltaTime, FlightAssistResponsiveness);
    RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, PitchRotationRate, DeltaTime, FlightAssistResponsiveness);
    
    // Signal active rotation intent to prevent auto-leveling and damping interference
    YawInput = (FMath::Abs(YawRotationRate) > 0.01f) ? 1.0f : 0.0f;
    PitchInput = (FMath::Abs(PitchRotationRate) > 0.01f) ? 1.0f : 0.0f;
    
    // Apply rotation to ship
    FRotator DeltaRotation = FRotator(RotationVelocity.Pitch * DeltaTime, RotationVelocity.Yaw * DeltaTime, 0.0f);
    AddActorWorldRotation(DeltaRotation);
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

void ASpaceship::SetNearbyStation(ASpaceStationModule* Station)
{
    NearbyStation = Station;
    
#if DOCKING_DEBUG_ENABLED
    // Debug print
    if (GEngine)
    {
        if (Station)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("[DOCKING] Ship entered docking range of station: %s"), *Station->GetName()));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("[DOCKING] Ship left docking range"));
        }
    }
#endif
}

void ASpaceship::ShowDockingPrompt(bool bShow)
{
    if (bShow)
    {
        // Get effective widget class (from settings or fallback)
        TSubclassOf<UUserWidget> EffectiveWidgetClass = GetEffectiveDockingPromptWidgetClass();
        
        // Create widget if it doesn't exist
        if (!DockingPromptWidget && EffectiveWidgetClass)
        {
            APlayerController* PC = Cast<APlayerController>(GetController());
            if (PC)
            {
                DockingPromptWidget = CreateWidget<UUserWidget>(PC, EffectiveWidgetClass);
                if (DockingPromptWidget)
                {
                    DockingPromptWidget->AddToViewport();
                    
#if DOCKING_DEBUG_ENABLED
                    // Debug print
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                            TEXT("[DOCKING] WBP_DockingPrompt created and added to viewport"));
                    }
#endif
                }
                else
                {
#if DOCKING_DEBUG_ENABLED
                    // Debug print - widget creation failed
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                            TEXT("[DOCKING] ERROR: Failed to create WBP_DockingPrompt widget"));
                    }
#endif
                }
            }
            else
            {
#if DOCKING_DEBUG_ENABLED
                // Debug print - no player controller
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                        TEXT("[DOCKING] ERROR: No player controller found for widget creation"));
                }
#endif
            }
        }
        else if (!EffectiveWidgetClass)
        {
            UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::ShowDockingPrompt - No DockingPromptWidgetClass set (neither in DockingSettings nor direct property) on '%s'. Docking prompt UI will not be shown."), *GetName());
            
#if DOCKING_DEBUG_ENABLED
            // Debug print - widget class not set
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                    TEXT("[DOCKING] ERROR: DockingPromptWidgetClass not set in Blueprint or Settings"));
            }
#endif
        }
        
        // Show existing widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Visible);
            
#if DOCKING_DEBUG_ENABLED
            // Debug print
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                    TEXT("[DOCKING] Docking prompt now visible"));
            }
#endif
        }
    }
    else
    {
        // Hide widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
            
#if DOCKING_DEBUG_ENABLED
            // Debug print
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                    TEXT("[DOCKING] Docking prompt hidden"));
            }
#endif
        }
    }
}

void ASpaceship::RequestDocking()
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] RequestDocking() called - Player pressed docking key"));
    }

    #endif
    
    // Validate nearby station exists
    if (!NearbyStation)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No station in range"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: No station in range"));
        }

        
        #endif
        
        // TODO: Show user feedback via HUD message
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - station found
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Station in range: %s"), *NearbyStation->GetName()));
    }

    
    #endif
    
    // If already docked, undock instead
    if (bIsDocked)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("[DOCKING] Already docked - calling Undock() instead"));
        }

        #endif
        
        Undock();
        return;
    }
    
    // Prevent rapid input during docking sequence
    if (bIsDocking)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("[DOCKING] Already docking - ignoring input"));
        }

        #endif
        
        return;
    }
    
    // Cast to docking bay module to check availability
    ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
    if (!DockingBay)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Station is not a docking module"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: Station is not a docking module"));
        }

        
        #endif
        
        // TODO: Show user feedback via HUD message
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - docking module found
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Station is a valid docking module"));
    }

    
    #endif
    
    // Check if docking is available
    if (!DockingBay->HasAvailableDocking())
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No docking slots available"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: No docking slots available (%d/%d occupied)"), 
                    DockingBay->MaxDockedShips - DockingBay->GetAvailableDockingSpots(),
                    DockingBay->MaxDockedShips));
        }

        
        #endif
        
        // TODO: Show user feedback via HUD message
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - slots available
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Docking slots available: %d/%d free"), 
                DockingBay->GetAvailableDockingSpots(),
                DockingBay->MaxDockedShips));
    }

    
    #endif
    
    // Get available docking point
    USceneComponent* DockingPoint = DockingBay->GetAvailableDockingPoint();
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Failed to get docking point"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: Failed to get docking point (null pointer)"));
        }

        
        #endif
        
        // TODO: Show user feedback via HUD message
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - docking point found
    if (GEngine)
    {
        FVector PointLocation = DockingPoint->GetComponentLocation();
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Docking point found at location: X=%.0f Y=%.0f Z=%.0f"), 
                PointLocation.X, PointLocation.Y, PointLocation.Z));
    }

    
    #endif
    
    // Check if ship is within docking range
    float DistanceToDockingPoint = FVector::Dist(GetActorLocation(), DockingPoint->GetComponentLocation());
    float EffectiveRange = GetEffectiveDockingRange();
    
    if (DistanceToDockingPoint > EffectiveRange)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Too far from docking point (%.0f > %.0f)"), DistanceToDockingPoint, EffectiveRange);
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: Too far from docking point (%.0f units > %.0f max)"), 
                    DistanceToDockingPoint, EffectiveRange));
        }

        
        #endif
        
        // TODO: Show user feedback via HUD message
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - distance check passed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Distance check passed: %.0f units (within %.0f max)"), 
                DistanceToDockingPoint, EffectiveRange));
    }

    
    #endif
    
    // Store docking point and begin docking sequence
    CurrentDockingPoint = DockingPoint;
    bIsDocking = true;
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - starting docking
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] All checks passed - starting docking sequence"));
    }

    
    #endif
    
    // Navigate to docking point (instant in simplified version)
    NavigateToDockingPoint(CurrentDockingPoint);
}

void ASpaceship::NavigateToDockingPoint(USceneComponent* DockingPoint)
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] NavigateToDockingPoint() called - Moving ship to docking point"));
    }

    #endif
    
    // Validate docking point
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::NavigateToDockingPoint - Invalid docking point"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: Invalid docking point (null pointer)"));
        }

        
        #endif
        
        bIsDocking = false;
        return;
    }
    
    // Simple MVP approach: instantly teleport to docking point
    // Get target transform from docking point
    FVector TargetLocation = DockingPoint->GetComponentLocation();
    FRotator TargetRotation = DockingPoint->GetComponentRotation();
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - target position
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("[DOCKING] Target docking position: X=%.0f Y=%.0f Z=%.0f"), 
                TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
    }

    
    #endif
    
    // Instantly move ship to docking point
    SetActorLocationAndRotation(TargetLocation, TargetRotation);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - ship moved
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Ship teleported to docking point successfully"));
    }

    
    #endif
    
    // Immediately complete docking
    CompleteDocking();
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::NavigateToDockingPoint - Instantly docked at point"));
}

void ASpaceship::CompleteDocking()
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] CompleteDocking() called - Finalizing docking process"));
    }

    #endif
    
    // Update docking state
    bIsDocked = true;
    bIsDocking = false;
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - state updated
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Ship state updated: bIsDocked=true, bIsDocking=false"));
    }

    
    #endif
    
    // Notify station that ship has docked
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->DockShip();
            
            #if DOCKING_DEBUG_ENABLED

            
            // Debug print - station notified
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    TEXT("[DOCKING] Station notified of successful docking"));
            }

            
            #endif
        }
    }
    
    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print - no player controller
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: No player controller found - cannot disable input or create UI"));
        }

        #endif
        return;
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - player controller found
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Player controller found"));
    }

    
    #endif
    
    // Disable input
    DisableInput(PC);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - input disabled
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Ship input disabled"));
    }

    
    #endif
    
    // Hide ship
    SetActorHiddenInGame(true);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - ship hidden
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Ship mesh hidden (player is 'inside' station)"));
    }

    
    #endif
    
    // Get effective trading interface class (from settings or fallback)
    TSubclassOf<UUserWidget> EffectiveTradingClass = GetEffectiveTradingInterfaceClass();
    
    // Create and show trading widget
    if (EffectiveTradingClass)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print - creating widget
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
                TEXT("[DOCKING] Creating trading UI widget..."));
        }

        #endif
        
        TradingWidget = CreateWidget<UUserWidget>(PC, EffectiveTradingClass);
        if (TradingWidget)
        {
            TradingWidget->AddToViewport();
            
            #if DOCKING_DEBUG_ENABLED

            
            // Debug print - widget created successfully
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    TEXT("[DOCKING] Trading UI widget created and added to viewport"));
            }

            
            #endif
        }
        else
        {
            #if DOCKING_DEBUG_ENABLED

            // Debug print - widget creation failed
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                    TEXT("[DOCKING] ERROR: Failed to create trading UI widget"));
            }

            #endif
        }
    }
    else
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::CompleteDocking - No TradingInterfaceClass set (neither in DockingSettings nor direct property) on '%s'. Trading UI will not be created."), *GetName());
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print - widget class not set
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: TradingInterfaceClass not set in Blueprint or Settings"));
        }

        
        #endif
    }
    
    // Set input mode to UI only
    PC->bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
    if (TradingWidget)
    {
        InputMode.SetWidgetToFocus(TradingWidget->TakeWidget());
    }
    PC->SetInputMode(InputMode);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - input mode changed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[DOCKING] Input mode set to UI only, mouse cursor shown"));
    }

    
    #endif
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - complete success
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("==================================================="));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("[DOCKING] DOCKING COMPLETE - Trading UI should be visible"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("==================================================="));
    }

    
    #endif
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::CompleteDocking - Docking complete for '%s'"), *GetName());
}

void ASpaceship::Undock()
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[UNDOCKING] Undock() called - Beginning undock sequence"));
    }

    #endif
    
    // Check if actually docked
    if (!bIsDocked)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::Undock - Not currently docked"));
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("[UNDOCKING] WARNING: Not currently docked - aborting undock"));
        }

        
        #endif
        
        return;
    }
    
    // Notify station that ship is undocking
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->UndockShip();
            
            #if DOCKING_DEBUG_ENABLED

            
            // Debug print - station notified
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    FString::Printf(TEXT("[UNDOCKING] Station notified: %s"), *NearbyStation->GetName()));
            }

            
            #endif
        }
    }
    
    // Update state
    bIsDocked = false;
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - state updated
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[UNDOCKING] Ship state updated: bIsDocked=false"));
    }

    
    #endif
    
    // Remove trading widget
    if (TradingWidget)
    {
        TradingWidget->RemoveFromParent();
        TradingWidget = nullptr;
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print - widget removed
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                TEXT("[UNDOCKING] Trading UI widget removed from viewport"));
        }

        
        #endif
    }
    
    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print - no player controller
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[UNDOCKING] ERROR: No player controller found"));
        }

        #endif
        
        return;
    }
    
    // Enable input
    EnableInput(PC);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - input enabled
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[UNDOCKING] Ship input re-enabled"));
    }

    
    #endif
    
    // Show ship
    SetActorHiddenInGame(false);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - ship visible
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[UNDOCKING] Ship mesh shown"));
    }

    
    #endif
    
    // Set input mode to game only
    PC->bShowMouseCursor = false;
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - input mode changed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("[UNDOCKING] Input mode set to game only, mouse cursor hidden"));
    }

    
    #endif
    
    // Apply forward impulse to move away from station
    FVector ForwardVector = GetActorForwardVector();
    if (MovementComponent)
    {
        // Add velocity in forward direction for smooth movement away
        MovementComponent->Velocity += ForwardVector * 500.0f;
        
        #if DOCKING_DEBUG_ENABLED

        
        // Debug print - impulse applied
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                TEXT("[UNDOCKING] Forward impulse applied (500 units)"));
        }

        
        #endif
    }
    
    #if DOCKING_DEBUG_ENABLED

    
    // Debug print - complete success
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("==================================================="));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("[UNDOCKING] UNDOCKING COMPLETE - Player has control"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            TEXT("==================================================="));
    }

    
    #endif
    
    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::Undock - Undocked successfully from '%s'"), NearbyStation ? *NearbyStation->GetName() : TEXT("Unknown Station"));
}

// ===== DOCKING CONFIGURATION HELPERS =====

float ASpaceship::GetEffectiveDockingRange() const
{
    // Use DockingSettings if set and valid, otherwise fall back to DockingRange property
    if (DockingSettings)
    {
        const float SettingsRange = DockingSettings->DockingRange;
        if (SettingsRange > 0.0f)
        {
            return SettingsRange;
        }
    }
    return DockingRange;
}

TSubclassOf<UUserWidget> ASpaceship::GetEffectiveDockingPromptWidgetClass() const
{
    // Use DockingSettings if set, otherwise fall back to DockingPromptWidgetClass property
    if (DockingSettings && DockingSettings->DockingPromptWidgetClass)
    {
        return DockingSettings->DockingPromptWidgetClass;
    }
    return DockingPromptWidgetClass;
}

TSubclassOf<UUserWidget> ASpaceship::GetEffectiveTradingInterfaceClass() const
{
    // Use DockingSettings if set, otherwise fall back to TradingInterfaceClass property
    if (DockingSettings && DockingSettings->TradingInterfaceClass)
    {
        return DockingSettings->TradingInterfaceClass;
    }
    return TradingInterfaceClass;
}
