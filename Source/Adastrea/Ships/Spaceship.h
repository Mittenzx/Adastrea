#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipParticleComponent.h"
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

    // Particle system component for engine and thruster effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
    USpaceshipParticleComponent* ParticleComponent;

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

    // Turn rate for ship rotation (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Control", meta=(ClampMin="0.0"))
    float TurnRate;

    // Reference to the ship's data asset with detailed stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Data")
    class USpaceshipDataAsset* ShipDataAsset;

    // Current hull integrity (health)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Status", meta=(ClampMin="0.0"))
    float CurrentHullIntegrity;

    // Maximum hull integrity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Status", meta=(ClampMin="0.0"))
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
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Spaceship")
    FText GetShipClass() const;

    /**
     * Transition player into the ship's interior space
     * @param PlayerController The controller to transition into the interior
     */
    UFUNCTION(BlueprintCallable, Category="Spaceship")
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
     * Setup input component for ship control
     * Binds axis and action inputs for movement and rotation
     */
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    // Saved reference to the walking pawn when controlling the ship
    UPROPERTY()
    APawn* SavedExternalPawn;

    // Movement input handlers
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void Turn(float Value);
    void LookUp(float Value);
};