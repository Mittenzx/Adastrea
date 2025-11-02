#include "Ships/Spaceship.h"
#include "Ships/SpaceshipInterior.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASpaceship::ASpaceship()
{
    PrimaryActorTick.bCanEverTick = false;
    InteriorInstance = nullptr;

    // Create and configure the floating pawn movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    if (MovementComponent)
    {
        // Configure default movement properties suitable for space flight
        MovementComponent->MaxSpeed = 3000.0f;
        MovementComponent->Acceleration = 1000.0f;
        MovementComponent->Deceleration = 1000.0f;
        MovementComponent->TurningBoost = 8.0f;
    }
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