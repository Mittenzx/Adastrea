#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipEngineModule.generated.h"

UCLASS()
class ADASTREA_API ASpaceshipEngineModule : public AActor
{
    GENERATED_BODY()

public:
    ASpaceshipEngineModule();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
    float Thrust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
    float BoostMultiplier;
};