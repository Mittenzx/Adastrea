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

    // Entry location for player teleport
    UFUNCTION(BlueprintCallable, Category="Interior")
    FVector GetEntryLocation() const;

    UFUNCTION(BlueprintCallable, Category="Interior")
    FRotator GetEntryRotation() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector EntryLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FRotator EntryRotation;
};