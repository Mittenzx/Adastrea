#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipInterior.generated.h"

class UBoxComponent;

/**
 * Represents the walkable interior space of a spaceship
 *
 * This actor defines the physical interior that players can explore when boarding
 * a ship. It manages entry points and interior layout.
 *
 * Usage:
 * - Place this actor in your level or spawn it at runtime
 * - Configure entry location and rotation
 * - Reference from parent ASpaceship
 * - Use Blueprint to add interior geometry and functionality
 */
UCLASS()
class ADASTREA_API ASpaceshipInterior : public AActor
{
    GENERATED_BODY()

public:
    ASpaceshipInterior();

    /**
     * Get the entry point location for player spawn
     * @return World location where player should spawn when entering
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FVector GetEntryLocation() const;

    /**
     * Get the entry point rotation for player spawn
     * @return Rotation the player should face when entering
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FRotator GetEntryRotation() const;

    /**
     * Get the exit point location for player when leaving ship control
     * @return World location where player should be placed when exiting control
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FVector GetExitLocation() const;

    /** Get the collision box that defines the walkable interior volume. */
    UFUNCTION(BlueprintPure, Category="Interior")
    UBoxComponent* GetInteriorVolume() const { return InteriorVolume; }

    /** Set the rect size of the walkable floor (X = forward depth, Y = width). */
    UFUNCTION(BlueprintCallable, Category="Interior")
    void SetFloorDimensions(float ForwardDepth, float Width);

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Called when the avatar overlaps the cockpit/seat trigger -> return to ship. */
    UFUNCTION()
    void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                              bool bFromSweep, const FHitResult& SweepResult);

    // World location where players spawn when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector EntryLocation;

    // Rotation players face when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FRotator EntryRotation;

    // World location where players spawn when exiting ship control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector ExitLocation;

    /** Root scene component so the volume can be positioned relative to the ship. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
    TObjectPtr<USceneComponent> SceneRoot;

    /** Box volume the player can walk within (floor plane). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
    TObjectPtr<UBoxComponent> InteriorVolume;

    /** Trigger volume at the cockpit/seat. Avatar walking into it returns to the ship. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
    TObjectPtr<UBoxComponent> ExitTrigger;

public:
    // Walkable floor dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    float FloorForwardDepth = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    float FloorWidth = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
        float CeilingHeight = 350.0f;

        /** Local offset of the cockpit/seat exit trigger from the interior origin. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
        FVector ExitTriggerOffset = FVector(500.0f, 0.0f, 175.0f);

        /** Size of the cockpit/seat exit trigger box. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
        FVector ExitTriggerSize = FVector(100.0f, 150.0f, 200.0f);
    };