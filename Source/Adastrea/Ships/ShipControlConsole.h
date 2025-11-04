#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "ShipControlConsole.generated.h"

// Forward declaration
class ASpaceship;

/**
 * Interactable control console inside a spaceship interior
 * 
 * This actor represents the pilot's control console that allows a player
 * to take control of the spaceship from within the interior. When the player
 * interacts with the console, their walking pawn is saved and they possess
 * the spaceship to control it directly.
 * 
 * Usage:
 * - Place this actor inside a spaceship interior Blueprint
 * - Set the OwningShip reference to the parent spaceship
 * - Configure InteractionRange for the proximity detection
 * - Call Interact() when player presses interact key
 * 
 * Integration:
 * - Works with ASpaceship's BeginControl/EndControl methods
 * - Player's Interact implementation should detect this actor
 * - Supports both enter and exit control functionality
 */
UCLASS(BlueprintType)
class ADASTREA_API AShipControlConsole : public AActor
{
    GENERATED_BODY()

public:
    AShipControlConsole();

    /**
     * The spaceship that this console controls
     * Must be set in editor or at spawn time
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control Console")
    ASpaceship* OwningShip;

    /**
     * Radius around the console where the player can interact
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control Console", meta=(ClampMin="50.0", ClampMax="500.0"))
    float InteractionRange;

    /**
     * Whether the console is currently being used (ship is controlled)
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Control Console")
    bool bIsControlActive;

    /**
     * Handle player interaction with the console
     * Toggles between taking control of the ship and releasing control
     * @param PlayerController The controller that is interacting
     */
    UFUNCTION(BlueprintCallable, Category="Control Console")
    void Interact(class APlayerController* PlayerController);

    /**
     * Check if the console can be interacted with
     * @param PlayerController The controller attempting to interact
     * @return True if interaction is possible
     */
    UFUNCTION(BlueprintCallable, Category="Control Console")
    bool CanInteract(class APlayerController* PlayerController) const;

protected:
    virtual void BeginPlay() override;

    // Collision component for interaction detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USphereComponent* InteractionSphere;
};
