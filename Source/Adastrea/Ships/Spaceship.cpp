#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASpaceship::ASpaceship()
{
    PrimaryActorTick.bCanEverTick = false;
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
    }
}

void ASpaceship::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
        
        // Update particle throttle based on forward movement
        if (ParticleComponent)
        {
            ParticleComponent->UpdateThrottle(FMath::Abs(Value));
            
            // Activate RCS thrusters based on direction
            if (Value > 0.0f)
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Backward, Value);
            }
            else
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, FMath::Abs(Value));
            }
        }
    }
}

void ASpaceship::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
        
        // Activate RCS thrusters for strafing
        if (ParticleComponent)
        {
            if (Value > 0.0f)
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Left, Value);
            }
            else
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Right, FMath::Abs(Value));
            }
        }
    }
}

void ASpaceship::MoveUp(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorUpVector(), Value);
        
        // Activate RCS thrusters for vertical movement
        if (ParticleComponent)
        {
            if (Value > 0.0f)
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Down, Value);
            }
            else
            {
                ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Up, FMath::Abs(Value));
            }
        }
    }
}

void ASpaceship::Turn(float Value)
{
    if (Value != 0.0f && GetWorld())
    {
        AddControllerYawInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
    }
}

void ASpaceship::LookUp(float Value)
{
    if (Value != 0.0f && GetWorld())
    {
        AddControllerPitchInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
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