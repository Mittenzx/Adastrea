#include "Ships/ShipControlConsole.h"
#include "Ships/Spaceship.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"

AShipControlConsole::AShipControlConsole()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create interaction sphere component
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    RootComponent = InteractionSphere;
    InteractionSphere->InitSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Default settings
    InteractionRange = 150.0f;
    bIsControlActive = false;
    OwningShip = nullptr;
}

void AShipControlConsole::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius to match interaction range
    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(InteractionRange);
    }

    // Try to find owning ship if not set
    if (!OwningShip)
    {
        AActor* ParentActor = GetAttachParentActor();
        if (ParentActor)
        {
            OwningShip = Cast<ASpaceship>(ParentActor);
            if (!OwningShip)
            {
                // Try searching for spaceship in the level
                AActor* OwnerActor = GetOwner();
                if (OwnerActor)
                {
                    OwningShip = Cast<ASpaceship>(OwnerActor);
                }
            }
        }
    }
}

bool AShipControlConsole::CanInteract(APlayerController* PlayerController) const
{
    if (!PlayerController || !OwningShip)
    {
        return false;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    // Check if player is within interaction range
    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= InteractionRange;
}

void AShipControlConsole::Interact(APlayerController* PlayerController)
{
    if (!PlayerController || !OwningShip)
    {
        return;
    }

    APawn* CurrentPawn = PlayerController->GetPawn();
    if (!CurrentPawn)
    {
        return;
    }

    // Toggle control: if controlling ship, release control; otherwise take control
    if (bIsControlActive)
    {
        // Release control - return to walking pawn
        OwningShip->EndControl(PlayerController);
        bIsControlActive = false;
    }
    else
    {
        // Take control - possess the ship
        OwningShip->BeginControl(PlayerController, CurrentPawn);
        bIsControlActive = true;
    }
}
