#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipInterior.h"
#include "Spaceship.generated.h"

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
    UFloatingPawnMovement* MovementComponent;

    // Default maximum movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultMaxSpeed;

    // Default acceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultAcceleration;

    // Default deceleration rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultDeceleration;

    // Default turning boost multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0"))
    float DefaultTurningBoost;

    // Reference to the walkable interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    ASpaceshipInterior* InteriorInstance;

    /**
     * Transition player into the ship's interior space
     * @param PlayerController The controller to transition into the interior
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship")
    void EnterInterior(class APlayerController* PlayerController);

protected:
    virtual void BeginPlay() override;
};