#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipInterior.generated.h"

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

protected:
    // World location where players spawn when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector EntryLocation;

    // Rotation players face when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FRotator EntryRotation;
};