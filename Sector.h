#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sector.generated.h"

class APlanet;
class ASpaceStation;

UCLASS()
class ADASTREA_API ASector : public AActor
{
    GENERATED_BODY()

public:
    ASector();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector")
    FString SectorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector")
    TArray<APlanet*> Planets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sector")
    TArray<ASpaceStation*> SpaceStations;
};