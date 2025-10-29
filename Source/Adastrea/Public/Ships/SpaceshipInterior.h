#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipInterior.generated.h"

UCLASS()
class ADASTREA_API ASpaceshipInterior : public AActor
{
    GENERATED_BODY()

public:
    ASpaceshipInterior();

    FVector GetEntryLocation() const;
    FRotator GetEntryRotation() const;

protected:
    UPROPERTY()
    FVector EntryLocation;

    UPROPERTY()
    FRotator EntryRotation;
};
