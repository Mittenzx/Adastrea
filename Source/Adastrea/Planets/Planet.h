#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UCLASS()
class ADASTREA_API APlanet : public AActor
{
    GENERATED_BODY()

public:
    APlanet();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet")
    FString PlanetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planet")
    float ResourceAmount;
};